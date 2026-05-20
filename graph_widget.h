#ifndef GRAPH_WIDGET_H
#define GRAPH_WIDGET_H

#include "graph.h"

#include <QColor>
#include <QPainterPath>
#include <QTimer>
#include <QWidget>

#include <vector>

// GraphWidget 是纯展示控件：
// 1. 负责画顶点、边、路径顺序；
// 2. 负责用 QTimer 播放高亮动画；
// 3. 不判断图能不能一笔画，也不构造欧拉路径。
class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget* parent = nullptr);

    // 载入新图时清空旧路径和动画状态。
    void setGraph(const Graph& graph);

    // 传入 EulerSolver 计算出的路径，控件只根据 edgePath/vertexPath 展示。
    void setEulerPath(const std::vector<int>& edgePath, const std::vector<int>& vertexPath);
    void clearAll();
    void resetAnimation();
    void startAnimation();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Graph graph_;
    std::vector<int> edgePath_;         // 动画要依次高亮的 edgeId 序列。
    std::vector<int> vertexPath_;       // 当前顶点高亮所需的顶点序列。
    std::vector<int> edgeOrder_;        // edgeOrder_[edgeId] = 这条边在路径中的第几步。
    std::vector<double> edgeOffsetRank_; // 重边绘制偏移量，避免多条边完全重叠。
    QTimer* timer_;
    int currentStep_;                   // -1 表示未播放；0..Q-1 表示当前正在走的边。

    void rebuildEdgeVisuals();
    void advanceAnimation();
    int currentVertex() const;
    QColor edgeColor(int edgeId) const;
    std::vector<QPointF> computeVertexPositions() const;
    QPainterPath makeEdgePath(const Edge& edge,
                              const std::vector<QPointF>& positions,
                              QPointF* labelPoint) const;
    void drawCenteredText(QPainter& painter, const QPointF& center, const QString& text) const;
};

#endif
