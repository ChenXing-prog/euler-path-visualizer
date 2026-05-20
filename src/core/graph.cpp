#include "graph.h"

Graph::Graph(int vertexCount)
    : vertexCount_(0) {
    setVertexCount(vertexCount);
}

void Graph::setVertexCount(int vertexCount) {
    vertexCount_ = vertexCount < 0 ? 0 : vertexCount;
    edges_.clear();
    adjacency_.assign(vertexCount_, {});
}

bool Graph::addEdge(int u, int v) {
    if (u < 0 || u >= vertexCount_ || v < 0 || v >= vertexCount_) {
        return false;
    }

    // 用当前 edges_ 的长度作为新边编号，天然保证 edgeId 唯一且从 0 连续递增。
    const int id = static_cast<int>(edges_.size());
    edges_.push_back({id, u, v});

    // 无向边要同时写入两个端点的邻接表。
    // 如果是自环 u == v，这里会写入两次，正好符合无向图自环度数加 2 的定义。
    adjacency_[u].push_back({v, id});
    adjacency_[v].push_back({u, id});
    return true;
}

void Graph::clear() {
    vertexCount_ = 0;
    edges_.clear();
    adjacency_.clear();
}

int Graph::vertexCount() const {
    return vertexCount_;
}

int Graph::edgeCount() const {
    return static_cast<int>(edges_.size());
}

int Graph::degree(int vertex) const {
    if (vertex < 0 || vertex >= vertexCount_) {
        return 0;
    }
    return static_cast<int>(adjacency_[vertex].size());
}

const std::vector<Edge>& Graph::edges() const {
    return edges_;
}

const std::vector<std::vector<AdjEdge>>& Graph::adjacency() const {
    return adjacency_;
}
