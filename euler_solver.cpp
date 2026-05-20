#include "euler_solver.h"

#include <algorithm>
#include <queue>
#include <sstream>

// solve() 是算法总入口：
// 1. 先判断非孤立顶点是否连通；
// 2. 再统计奇度顶点数量；
// 3. 根据欧拉定理判断是否存在欧拉通路/回路；
// 4. 若存在，再调用 Hierholzer 算法构造具体路径。
EulerResult EulerSolver::solve(const Graph& graph) const {
    EulerResult result;
    std::ostringstream line;

    // 日志不是算法必需数据，但可以让界面显示完整推理过程。
    line << "图中共有 " << graph.vertexCount() << " 个顶点，"
         << graph.edgeCount() << " 条边。";
    result.logs.push_back(line.str());

    // 没有顶点时，连起点都不存在，因此直接返回失败结果。
    if (graph.vertexCount() == 0) {
        result.logs.push_back("顶点数为 0，无法构造一笔画路径。");
        return result;
    }

    int firstNonIsolated = -1;
    std::vector<int> visited;

    // 第一步：连通性检查。
    // 一笔画要一次画完所有边，如果有边的顶点分散在多个连通块中，就必须抬笔。
    // 孤立点没有边，不需要画，所以不参与连通性要求。
    if (!isConnectedIgnoringIsolated(graph, firstNonIsolated, visited, result.logs)) {
        result.logs.push_back("结论：所有非孤立顶点不连通，不存在一笔画。");
        return result;
    }

    // 第二步：统计奇度顶点。
    // 欧拉定理告诉我们：无向图存在欧拉通路，当且仅当奇度顶点数量为 0 或 2。
    for (int v = 0; v < graph.vertexCount(); ++v) {
        if (graph.degree(v) % 2 == 1) {
            result.oddVertices.push_back(v);
        }
    }

    // 把奇度顶点统计结果写入日志，便于在界面或报告中展示判定过程。
    std::ostringstream oddLine;
    oddLine << "奇度顶点数量为 " << result.oddVertices.size();
    if (!result.oddVertices.empty()) {
        oddLine << "：";
        for (std::size_t i = 0; i < result.oddVertices.size(); ++i) {
            if (i > 0) {
                oddLine << ", ";
            }
            oddLine << vertexName(result.oddVertices[i]);
        }
    }
    oddLine << "。";
    result.logs.push_back(oddLine.str());

    // 第三步：根据奇度顶点数量分类。
    if (result.oddVertices.empty()) {
        result.hasEulerPath = true;
        result.isCircuit = true;

        // 欧拉回路可以从任意非孤立顶点开始；如果图没有边，就从 1 号顶点开始显示空路径。
        result.startVertex = firstNonIsolated == -1 ? 0 : firstNonIsolated;
        result.logs.push_back("0 个奇度顶点：存在欧拉回路。");
    } else if (result.oddVertices.size() == 2) {
        result.hasEulerPath = true;
        result.isCircuit = false;

        // 欧拉通路必须从一个奇度顶点出发，到另一个奇度顶点结束。
        result.startVertex = result.oddVertices.front();
        result.logs.push_back("2 个奇度顶点：存在欧拉通路。");
    } else {
        // 奇度顶点如果超过 2 个，中间顶点的“进出配对”无法全部满足。
        result.logs.push_back("奇度顶点数量不是 0 或 2，不存在一笔画。");
        return result;
    }

    // 第四步：只在确定存在欧拉路径后，才真正构造路径。
    result.logs.push_back("起点选择为顶点 " + vertexName(result.startVertex) + "。");
    buildEulerPath(graph, result.startVertex, result);

    // 理论上前面的判定已经保证能覆盖所有边；这里再校验一次，方便发现实现或输入异常。
    if (static_cast<int>(result.edgePath.size()) != graph.edgeCount()) {
        result.hasEulerPath = false;
        result.logs.push_back("构造结果未覆盖所有边，判定失败。");
        return result;
    }

    // 把算法结果整理成面向用户的顶点路径日志。
    std::ostringstream pathLine;
    pathLine << "最终顶点路径：";
    for (std::size_t i = 0; i < result.vertexPath.size(); ++i) {
        if (i > 0) {
            pathLine << " -> ";
        }
        pathLine << vertexName(result.vertexPath[i]);
    }
    result.logs.push_back(pathLine.str());

    // 把算法结果整理成面向用户的边序列日志。边使用 e1、e2... 展示。
    std::ostringstream edgeLine;
    edgeLine << "最终边序列：";
    if (result.edgePath.empty()) {
        edgeLine << "空路径（图中没有边）";
    } else {
        for (std::size_t i = 0; i < result.edgePath.size(); ++i) {
            if (i > 0) {
                edgeLine << " -> ";
            }
            edgeLine << "e" << (result.edgePath[i] + 1);
        }
    }
    result.logs.push_back(edgeLine.str());

    return result;
}

bool EulerSolver::isConnectedIgnoringIsolated(const Graph& graph,
                                              int& firstNonIsolated,
                                              std::vector<int>& visited,
                                              std::vector<std::string>& logs) const {
    firstNonIsolated = -1;

    // visited[v] 表示 BFS 是否访问过顶点 v。
    visited.assign(graph.vertexCount(), 0);

    // 找一个真正参与边的顶点作为 BFS 起点。
    // 如果从孤立点开始 BFS，会误以为其他有边顶点不可达，所以必须找 degree > 0 的点。
    for (int v = 0; v < graph.vertexCount(); ++v) {
        if (graph.degree(v) > 0) {
            firstNonIsolated = v;
            break;
        }
    }

    if (firstNonIsolated == -1) {
        // 没有任何边时，不存在“有边顶点不连通”的问题。
        // 这个项目把空边图视作存在空的欧拉回路。
        logs.push_back("所有顶点都是孤立点，连通性条件按空边图成立。");
        return true;
    }

    logs.push_back("从第一个非孤立顶点 " + vertexName(firstNonIsolated) + " 开始 BFS 连通性判断。");

    std::queue<int> queue;
    visited[firstNonIsolated] = 1;
    queue.push(firstNonIsolated);

    // 从第一个非孤立顶点出发做 BFS。
    // 只要沿邻接表能到达的非孤立顶点，都会被标记 visited。
    // 每条邻接表记录最多被扫描一次，总复杂度 O(P + Q)。
    const auto& adjacency = graph.adjacency();
    while (!queue.empty()) {
        const int u = queue.front();
        queue.pop();

        // adjacency[u] 存放所有从 u 出发能走到的邻接顶点和对应 edgeId。
        for (const AdjEdge& edge : adjacency[u]) {
            if (!visited[edge.to]) {
                visited[edge.to] = 1;
                queue.push(edge.to);
            }
        }
    }

    // BFS 结束后，只检查非孤立顶点。孤立点不需要被一笔画路径经过。
    for (int v = 0; v < graph.vertexCount(); ++v) {
        if (graph.degree(v) > 0 && !visited[v]) {
            logs.push_back("顶点 " + vertexName(v) + " 是非孤立顶点，但无法从起点到达。");
            return false;
        }
    }

    logs.push_back("所有非孤立顶点均已访问，连通性条件成立。");
    return true;
}

void EulerSolver::buildEulerPath(const Graph& graph, int startVertex, EulerResult& result) const {
    // 栈里保存“当前路径上还没完成回退的顶点”。
    // incomingEdge 用来在回退时知道：这个顶点是通过哪条边进入的。
    struct StackItem {
        int vertex;        // 当前栈节点所在顶点。
        int incomingEdge;  // 从上一个顶点走到这里所使用的 edgeId，起点为 -1。
    };

    const auto& adjacency = graph.adjacency();

    // nextIndex[v] 记录顶点 v 的邻接表扫描到哪里，避免每次从头找未使用边。
    // 这保证 Hierholzer 构造路径时仍然是线性复杂度，而不是反复扫描导致变慢。
    std::vector<int> nextIndex(graph.vertexCount(), 0);

    // used[edgeId] 保证重边也能被正确区分，并且每条真实边只走一次。
    // 注意：重边端点可能完全相同，只有 edgeId 能区分它们是不是同一条边。
    std::vector<char> used(graph.edgeCount(), 0);
    std::vector<StackItem> stack;

    // Hierholzer 在回退时得到的是反向路径，最后统一 reverse。
    // reversedVertices 保存回退时确认完成的顶点；
    // reversedEdges 保存进入这些顶点所使用的边。
    std::vector<int> reversedVertices;
    std::vector<int> reversedEdges;

    // 从已经选好的合法起点开始。起点没有“进入它的边”，所以 incomingEdge = -1。
    stack.push_back({startVertex, -1});
    result.logs.push_back("开始使用 Hierholzer 算法构造欧拉路径。");

    while (!stack.empty()) {
        const int u = stack.back().vertex;

        // 当前栈顶顶点 u 是“现在所在的位置”。
        // 先跳过 u 的邻接表中已经走过的边，找到下一条还没使用的边。
        // 由于 nextIndex[u] 只会向后移动，所有邻接表总扫描次数是线性的。
        while (nextIndex[u] < static_cast<int>(adjacency[u].size()) &&
               used[adjacency[u][nextIndex[u]].edgeId]) {
            ++nextIndex[u];
        }

        // 当前顶点没有剩余可走边，说明它在当前欧拉路径片段中已经闭合，可以回退并记录。
        // “走不动才记录”是 Hierholzer 的关键：这样能把后来插入的小回路自然拼接进总路径。
        if (nextIndex[u] == static_cast<int>(adjacency[u].size())) {
            reversedVertices.push_back(u);
            reversedEdges.push_back(stack.back().incomingEdge);
            stack.pop_back();
            continue;
        }

        // 找到一条从 u 出发、尚未使用的边。
        const AdjEdge edge = adjacency[u][nextIndex[u]];
        ++nextIndex[u];

        // 理论上前面的 while 已经跳过 used=true 的边；
        // 这里再判断一次，是为了保持逻辑稳健。
        if (used[edge.edgeId]) {
            continue;
        }

        // 走这条边：标记真实 edgeId 已使用，并把到达顶点压栈继续搜索。
        // 这里不是立即写入答案，因为后面可能还会从 edge.to 继续绕出一个回路。
        used[edge.edgeId] = 1;
        stack.push_back({edge.to, edge.edgeId});
    }

    // 回退时记录的是终点到起点方向，反转后才是实际一笔画顺序。
    // 例如实际路径是 1->2->3->1，回退记录可能先得到 1,3,2,1。
    std::reverse(reversedVertices.begin(), reversedVertices.end());
    std::reverse(reversedEdges.begin(), reversedEdges.end());

    // 顶点路径直接使用反转后的顶点序列。
    result.vertexPath = reversedVertices;

    // 边路径要去掉起点的 incomingEdge=-1，因为起点不是通过某条边进入的。
    result.edgePath.clear();
    for (int edgeId : reversedEdges) {
        if (edgeId != -1) {
            result.edgePath.push_back(edgeId);
        }
    }
}

std::string EulerSolver::vertexName(int zeroBasedVertex) {
    return std::to_string(zeroBasedVertex + 1);
}
