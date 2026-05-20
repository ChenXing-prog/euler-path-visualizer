#include "euler_solver.h"
#include "graph.h"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

namespace {

struct ValidationResult {
    bool ok = true;
    std::string reason = "OK";
};

struct FixedCase {
    int id = 0;
    std::string type;
    int vertexCount = 0;
    std::vector<std::pair<int, int>> edges; // 1 基输入，和界面保持一致。
    bool expectedHasEulerPath = false;
    bool expectedCircuit = false;
    int expectedEdgePathLength = 0;
};

struct FixedCaseResult {
    FixedCase testCase;
    EulerResult result;
    ValidationResult validation;
    bool passed = false;
};

struct RandomFailure {
    bool hasFailure = false;
    std::string reason;
    int vertexCount = 0;
    std::vector<std::pair<int, int>> edges; // 0 基保存，输出时转成 1 基。
    bool expected = false;
    bool actual = false;
    EulerResult result;
};

Graph buildGraph(int vertexCount, const std::vector<std::pair<int, int>>& oneBasedEdges) {
    Graph graph(vertexCount);
    for (const auto& edge : oneBasedEdges) {
        graph.addEdge(edge.first - 1, edge.second - 1);
    }
    return graph;
}

std::string yesNo(bool value) {
    return value ? "Yes" : "No";
}

std::string trueFalse(bool value) {
    return value ? "true" : "false";
}

std::string formatVertexPath(const std::vector<int>& vertexPath) {
    if (vertexPath.empty()) {
        return "空";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < vertexPath.size(); ++i) {
        if (i > 0) {
            out << "->";
        }
        out << vertexPath[i] + 1;
    }
    return out.str();
}

std::string formatEdgePath(const std::vector<int>& edgePath) {
    if (edgePath.empty()) {
        return "空";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < edgePath.size(); ++i) {
        if (i > 0) {
            out << "->";
        }
        out << "e" << edgePath[i] + 1;
    }
    return out.str();
}

std::string actualResultText(const EulerResult& result) {
    std::ostringstream out;
    out << "hasEulerPath=" << trueFalse(result.hasEulerPath)
        << "; isCircuit=" << trueFalse(result.isCircuit)
        << "; edgePath长度=" << result.edgePath.size();
    if (result.hasEulerPath) {
        out << "; 顶点路径=" << formatVertexPath(result.vertexPath)
            << "; 边序列=" << formatEdgePath(result.edgePath);
    }
    return out.str();
}

ValidationResult validateEulerPath(const Graph& graph, const EulerResult& result) {
    if (result.edgePath.size() != static_cast<std::size_t>(graph.edgeCount())) {
        return {false, "edgePath.size() 与 graph.edgeCount() 不一致"};
    }

    std::vector<char> used(graph.edgeCount(), 0);
    for (int edgeId : result.edgePath) {
        if (edgeId < 0 || edgeId >= graph.edgeCount()) {
            return {false, "edgePath 中存在越界 edgeId"};
        }
        if (used[edgeId]) {
            return {false, "edgePath 中存在重复 edgeId"};
        }
        used[edgeId] = 1;
    }

    if (graph.edgeCount() > 0 &&
        result.vertexPath.size() != result.edgePath.size() + 1) {
        return {false, "vertexPath.size() 不是 edgePath.size() + 1"};
    }

    for (int vertex : result.vertexPath) {
        if (vertex < 0 || vertex >= graph.vertexCount()) {
            return {false, "vertexPath 中存在越界顶点"};
        }
    }

    const std::vector<Edge>& edges = graph.edges();
    for (std::size_t i = 0; i < result.edgePath.size(); ++i) {
        const int edgeId = result.edgePath[i];
        const Edge& edge = edges[edgeId];
        const int a = result.vertexPath[i];
        const int b = result.vertexPath[i + 1];
        const bool matches = (edge.u == a && edge.v == b) ||
                             (edge.u == b && edge.v == a);
        if (!matches) {
            return {false, "edgePath 中的边没有连接 vertexPath 中相邻两个顶点"};
        }
    }

    if (result.isCircuit && graph.edgeCount() > 0 &&
        result.vertexPath.front() != result.vertexPath.back()) {
        return {false, "result.isCircuit=true，但 vertexPath 起点和终点不同"};
    }

    return {true, "OK"};
}

bool independentExpected(int vertexCount,
                         const std::vector<std::pair<int, int>>& zeroBasedEdges) {
    if (zeroBasedEdges.empty()) {
        return true;
    }

    std::vector<int> degree(vertexCount, 0);
    std::vector<std::vector<int>> adjacency(vertexCount);
    for (const auto& edge : zeroBasedEdges) {
        const int u = edge.first;
        const int v = edge.second;
        ++degree[u];
        ++degree[v];
        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
    }

    int start = -1;
    for (int v = 0; v < vertexCount; ++v) {
        if (degree[v] > 0) {
            start = v;
            break;
        }
    }

    std::vector<char> visited(vertexCount, 0);
    std::deque<int> queue;
    visited[start] = 1;
    queue.push_back(start);

    while (!queue.empty()) {
        const int u = queue.front();
        queue.pop_front();
        for (int v : adjacency[u]) {
            if (!visited[v]) {
                visited[v] = 1;
                queue.push_back(v);
            }
        }
    }

    for (int v = 0; v < vertexCount; ++v) {
        if (degree[v] > 0 && !visited[v]) {
            return false;
        }
    }

    int oddCount = 0;
    for (int value : degree) {
        if (value % 2 == 1) {
            ++oddCount;
        }
    }
    return oddCount == 0 || oddCount == 2;
}

std::vector<FixedCase> fixedCases() {
    return {
        {1, "欧拉回路，三角形图", 3, {{1, 2}, {2, 3}, {3, 1}}, true, true, 3},
        {2, "欧拉通路，两个奇度顶点", 4, {{1, 2}, {2, 3}, {1, 3}, {1, 4}, {3, 4}}, true, false, 5},
        {3, "不可一笔画，奇度顶点数量为 4", 4, {{1, 2}, {1, 3}, {1, 4}}, false, false, 0},
        {4, "不可一笔画，非孤立顶点不连通", 4, {{1, 2}, {3, 4}}, false, false, 0},
        {5, "含重边的欧拉回路", 2, {{1, 2}, {1, 2}}, true, true, 2},
        {6, "空边图", 5, {}, true, true, 0},
        {7, "单边图", 2, {{1, 2}}, true, false, 1},
        {8, "较复杂欧拉回路", 5, {{1, 2}, {2, 3}, {3, 1}, {3, 4}, {4, 5}, {5, 3}}, true, true, 6},
    };
}

std::vector<FixedCaseResult> runFixedTests() {
    EulerSolver solver;
    std::vector<FixedCaseResult> results;

    for (const FixedCase& testCase : fixedCases()) {
        Graph graph = buildGraph(testCase.vertexCount, testCase.edges);
        EulerResult result = solver.solve(graph);
        ValidationResult validation;
        if (result.hasEulerPath) {
            validation = validateEulerPath(graph, result);
        }

        bool passed = result.hasEulerPath == testCase.expectedHasEulerPath;
        if (passed && testCase.expectedHasEulerPath) {
            passed = result.isCircuit == testCase.expectedCircuit &&
                     static_cast<int>(result.edgePath.size()) == testCase.expectedEdgePathLength &&
                     validation.ok;
        }

        results.push_back({testCase, result, validation, passed});
    }

    return results;
}

RandomFailure randomTests(int testCount, std::uint32_t seed, int& passedCount) {
    EulerSolver solver;
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> vertexDist(1, 30);
    std::uniform_int_distribution<int> edgeDist(0, 80);

    passedCount = 0;

    for (int caseIndex = 0; caseIndex < testCount; ++caseIndex) {
        const int vertexCount = vertexDist(rng);
        const int edgeCount = edgeDist(rng);
        std::uniform_int_distribution<int> endpointDist(0, vertexCount - 1);

        std::vector<std::pair<int, int>> zeroBasedEdges;
        std::vector<std::pair<int, int>> oneBasedEdges;
        zeroBasedEdges.reserve(edgeCount);
        oneBasedEdges.reserve(edgeCount);

        for (int i = 0; i < edgeCount; ++i) {
            const int u = endpointDist(rng);
            const int v = endpointDist(rng);
            zeroBasedEdges.push_back({u, v});
            oneBasedEdges.push_back({u + 1, v + 1});
        }

        Graph graph = buildGraph(vertexCount, oneBasedEdges);
        EulerResult result = solver.solve(graph);
        const bool expected = independentExpected(vertexCount, zeroBasedEdges);

        if (result.hasEulerPath != expected) {
            return {true,
                    "判定结果与独立欧拉通路定理计算结果不一致",
                    vertexCount,
                    zeroBasedEdges,
                    expected,
                    result.hasEulerPath,
                    result};
        }

        if (result.hasEulerPath) {
            ValidationResult validation = validateEulerPath(graph, result);
            if (!validation.ok) {
                return {true,
                        "路径合法性检查失败：" + validation.reason,
                        vertexCount,
                        zeroBasedEdges,
                        expected,
                        result.hasEulerPath,
                        result};
            }
        }

        ++passedCount;
    }

    return {};
}

std::string expectedText(const FixedCase& testCase) {
    std::ostringstream out;
    out << yesNo(testCase.expectedHasEulerPath);
    if (testCase.expectedHasEulerPath) {
        out << (testCase.expectedCircuit ? "，欧拉回路" : "，欧拉通路")
            << "，edgePath 长度为 " << testCase.expectedEdgePathLength;
    }
    return out.str();
}

void writeResultsMarkdown(const std::vector<FixedCaseResult>& fixedResults,
                          int randomTotal,
                          int randomPassed,
                          std::uint32_t seed,
                          const RandomFailure& failure) {
    const std::string outputPath = std::string(PROJECT_SOURCE_DIR) + "/test_results.md";
    std::ofstream out(outputPath);

    out << "# 一笔画问题算法测试结果\n\n";
    out << "## 第一部分：固定测试用例结果表\n\n";
    out << "| 编号 | 测试类型 | 输入规模 | 预期结果 | 实际结果 | 是否通过 |\n";
    out << "|---|---|---|---|---|---|\n";

    for (const FixedCaseResult& item : fixedResults) {
        out << "| 用例" << item.testCase.id
            << " | " << item.testCase.type
            << " | P=" << item.testCase.vertexCount << ", Q=" << item.testCase.edges.size()
            << " | " << expectedText(item.testCase)
            << " | " << actualResultText(item.result)
            << " | " << (item.passed ? "通过" : "失败");
        if (!item.validation.ok) {
            out << "：" << item.validation.reason;
        }
        out << " |\n";
    }

    out << "\n## 第二部分：随机测试结果汇总\n\n";
    out << "随机测试组数：" << randomTotal << "  \n";
    out << "通过组数：" << randomPassed << "  \n";
    out << "失败组数：" << (randomTotal - randomPassed) << "  \n";
    out << "随机种子：" << seed << "  \n";

    if (!failure.hasFailure) {
        out << "结论：随机生成的图中，算法判定结果均与欧拉通路定理一致；"
            << "对于判定为可一笔画的图，输出路径均通过合法性检查。\n";
    } else {
        out << "结论：随机测试发现失败。  \n";
        out << "失败原因：" << failure.reason << "  \n";
        out << "失败图：P=" << failure.vertexCount
            << ", Q=" << failure.edges.size()
            << ", expected=" << trueFalse(failure.expected)
            << ", actual=" << trueFalse(failure.actual) << "  \n";
        out << "边列表：";
        for (const auto& edge : failure.edges) {
            out << " (" << edge.first + 1 << "," << edge.second + 1 << ")";
        }
        out << "  \n";
        out << "实际顶点路径：" << formatVertexPath(failure.result.vertexPath) << "  \n";
        out << "实际边序列：" << formatEdgePath(failure.result.edgePath) << "\n";
    }
}

} // namespace

int main() {
    const int randomTotal = 1000;
    const std::uint32_t seed = 20260518u;

    std::vector<FixedCaseResult> fixedResults = runFixedTests();
    bool allFixedPassed = true;

    std::cout << "固定代表性测试用例结果：\n";
    for (const FixedCaseResult& item : fixedResults) {
        allFixedPassed = allFixedPassed && item.passed;
        std::cout << "用例" << item.testCase.id << " [" << item.testCase.type << "] "
                  << (item.passed ? "PASS" : "FAIL") << "，"
                  << actualResultText(item.result);
        if (!item.validation.ok) {
            std::cout << "，路径验证：" << item.validation.reason;
        }
        std::cout << '\n';
    }

    int randomPassed = 0;
    RandomFailure failure = randomTests(randomTotal, seed, randomPassed);

    std::cout << "\n随机测试组数：" << randomTotal << '\n';
    std::cout << "通过组数：" << randomPassed << '\n';
    std::cout << "失败组数：" << (randomTotal - randomPassed) << '\n';
    std::cout << "随机种子：" << seed << '\n';

    if (failure.hasFailure) {
        std::cout << "随机测试失败原因：" << failure.reason << '\n';
        std::cout << "P=" << failure.vertexCount << ", Q=" << failure.edges.size() << '\n';
        std::cout << "边列表：";
        for (const auto& edge : failure.edges) {
            std::cout << " (" << edge.first + 1 << "," << edge.second + 1 << ")";
        }
        std::cout << '\n';
        std::cout << "expected=" << trueFalse(failure.expected)
                  << ", actual=" << trueFalse(failure.actual) << '\n';
    }

    writeResultsMarkdown(fixedResults, randomTotal, randomPassed, seed, failure);
    std::cout << "\n测试报告已写入：" << PROJECT_SOURCE_DIR << "/test_results.md\n";

    return allFixedPassed && !failure.hasFailure ? 0 : 1;
}
