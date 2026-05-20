#ifndef EULER_SOLVER_H
#define EULER_SOLVER_H

#include "graph.h"

#include <string>
#include <vector>

// EulerResult 是算法层交给界面层的唯一结果对象。
// 界面只读取这些字段并展示，不需要知道 BFS 或 Hierholzer 的内部细节。
struct EulerResult {
    bool hasEulerPath = false;
    bool isCircuit = false;
    int startVertex = -1;
    std::vector<int> oddVertices;       // 0 基编号的奇度顶点列表。
    std::vector<int> vertexPath;        // 欧拉路径经过的顶点序列。
    std::vector<int> edgePath;          // 欧拉路径经过的边 id 序列。
    std::vector<std::string> logs;      // 给界面显示的算法过程日志。
};

// EulerSolver 只处理“一笔画”算法，不包含任何 Qt 控件或绘图逻辑。
class EulerSolver {
public:
    EulerResult solve(const Graph& graph) const;

private:
    // 判断所有非孤立顶点是否连通。孤立点没有边，不影响一笔画是否存在。
    bool isConnectedIgnoringIsolated(const Graph& graph,
                                     int& firstNonIsolated,
                                     std::vector<int>& visited,
                                     std::vector<std::string>& logs) const;

    // 在已经确认存在欧拉路径的前提下，用 Hierholzer 算法构造具体路径。
    void buildEulerPath(const Graph& graph, int startVertex, EulerResult& result) const;

    // 内部统一使用 0 基编号，展示给用户时转换为 1 基编号。
    static std::string vertexName(int zeroBasedVertex);
};

#endif
