#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

// 一条无向边的完整信息。
// id 是边的唯一编号，用来区分重边；u、v 是 0 基顶点编号。
struct Edge {
    int id;
    int u;
    int v;
};

// 邻接表中的边记录。
// 对于无向边 (u, v)，会在 u 和 v 的邻接表中各放一条 AdjEdge，
// 但两条记录共享同一个 edgeId，因此 Hierholzer 算法能保证每条真实边只用一次。
struct AdjEdge {
    int to;
    int edgeId;
};

// Graph 只负责存图，不负责判断一笔画，也不依赖 Qt。
// 这样算法层可以单独测试，界面层只读取它提供的数据。
class Graph {
public:
    explicit Graph(int vertexCount = 0);

    void setVertexCount(int vertexCount);

    // 添加一条无向边。传入的 u、v 必须是 0 基编号。
    // 返回 false 表示顶点编号越界。
    bool addEdge(int u, int v);
    void clear();

    int vertexCount() const;
    int edgeCount() const;

    // 无向图中顶点的度数。自环会在邻接表中加入两次，因此度数贡献为 2。
    int degree(int vertex) const;

    // edges() 用于按 edgeId 找边；adjacency() 用于 BFS 和 Hierholzer 遍历。
    const std::vector<Edge>& edges() const;
    const std::vector<std::vector<AdjEdge>>& adjacency() const;

private:
    int vertexCount_;
    std::vector<Edge> edges_;
    std::vector<std::vector<AdjEdge>> adjacency_;
};

#endif
