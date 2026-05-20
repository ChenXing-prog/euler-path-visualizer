#include "graph_widget.h"

#include <QFontMetrics>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <map>

GraphWidget::GraphWidget(QWidget* parent)
    : QWidget(parent),
      timer_(new QTimer(this)),
      currentStep_(-1) {
    setMinimumSize(520, 420);
    setAutoFillBackground(true);

    // 项目要求每 500ms 高亮一条路径边。
    timer_->setInterval(500);
    connect(timer_, &QTimer::timeout, this, [this]() { advanceAnimation(); });
}

void GraphWidget::setGraph(const Graph& graph) {
    timer_->stop();
    graph_ = graph;

    // 新图载入后，旧欧拉路径和动画状态都不再有效。
    edgePath_.clear();
    vertexPath_.clear();
    currentStep_ = -1;
    rebuildEdgeVisuals();
    update();
}

void GraphWidget::setEulerPath(const std::vector<int>& edgePath, const std::vector<int>& vertexPath) {
    timer_->stop();
    edgePath_ = edgePath;
    vertexPath_ = vertexPath;
    currentStep_ = -1;

    // 把“路径数组”转换成“每条边在第几步走过”，绘图时 O(1) 判断颜色和标签。
    edgeOrder_.assign(graph_.edgeCount(), -1);
    for (std::size_t i = 0; i < edgePath_.size(); ++i) {
        const int edgeId = edgePath_[i];
        if (edgeId >= 0 && edgeId < graph_.edgeCount()) {
            edgeOrder_[edgeId] = static_cast<int>(i);
        }
    }

    update();
}

void GraphWidget::clearAll() {
    timer_->stop();
    graph_.clear();
    edgePath_.clear();
    vertexPath_.clear();
    edgeOrder_.clear();
    edgeOffsetRank_.clear();
    currentStep_ = -1;
    update();
}

void GraphWidget::resetAnimation() {
    timer_->stop();
    currentStep_ = -1;
    update();
}

void GraphWidget::startAnimation() {
    if (edgePath_.empty()) {
        return;
    }

    // currentStep 表示当前橙色边的下标，从第 1 条路径边开始播放。
    currentStep_ = 0;
    timer_->start();
    update();
}

QSize GraphWidget::sizeHint() const {
    return QSize(680, 520);
}

void GraphWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(255, 255, 255));

    const int n = graph_.vertexCount();
    if (n == 0) {
        painter.setPen(QColor(120, 120, 120));
        painter.drawText(rect(), Qt::AlignCenter, "请在左侧输入并载入图");
        return;
    }

    const std::vector<QPointF> positions = computeVertexPositions();

    // 先画边，再画顶点，这样顶点圆形不会被边线压住。
    for (const Edge& edge : graph_.edges()) {
        QPointF labelPoint;
        const QPainterPath path = makeEdgePath(edge, positions, &labelPoint);

        QPen pen(edgeColor(edge.id), currentStep_ >= 0 ? 4 : 2);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);

        // e1/e2 是真实边编号；#1/#2 是欧拉路径中的行走顺序。
        QString label = QString("e%1").arg(edge.id + 1);
        if (edge.id >= 0 && edge.id < static_cast<int>(edgeOrder_.size()) && edgeOrder_[edge.id] >= 0) {
            label += QString(" #%1").arg(edgeOrder_[edge.id] + 1);
        }
        drawCenteredText(painter, labelPoint, label);
    }

    const int activeVertex = currentVertex();

    // 最后画顶点和当前顶点高亮。
    for (int v = 0; v < n; ++v) {
        const QPointF center = positions[v];
        const bool active = (v == activeVertex);
        const int radius = active ? 19 : 16;

        painter.setPen(QPen(active ? QColor(230, 126, 34) : QColor(80, 80, 80), active ? 3 : 2));
        painter.setBrush(active ? QColor(255, 232, 204) : QColor(245, 247, 255));
        painter.drawEllipse(center, radius, radius);

        painter.setPen(QColor(30, 30, 30));
        painter.drawText(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2),
                         Qt::AlignCenter,
                         QString::number(v + 1));
    }
}

void GraphWidget::rebuildEdgeVisuals() {
    edgeOrder_.assign(graph_.edgeCount(), -1);
    edgeOffsetRank_.assign(graph_.edgeCount(), 0.0);

    // 重边按两个端点分组，同组边使用不同弯曲偏移量显示。
    std::map<std::pair<int, int>, std::vector<int>> groups;
    for (const Edge& edge : graph_.edges()) {
        const int a = std::min(edge.u, edge.v);
        const int b = std::max(edge.u, edge.v);
        groups[{a, b}].push_back(edge.id);
    }

    for (const auto& item : groups) {
        const std::vector<int>& ids = item.second;
        const int count = static_cast<int>(ids.size());
        for (int i = 0; i < count; ++i) {
            const int edgeId = ids[i];
            edgeOffsetRank_[edgeId] = i - (count - 1) / 2.0;
        }
    }
}

void GraphWidget::advanceAnimation() {
    if (edgePath_.empty()) {
        timer_->stop();
        currentStep_ = -1;
        update();
        return;
    }

    if (currentStep_ < static_cast<int>(edgePath_.size()) - 1) {
        ++currentStep_;
    } else {
        // 用 Q 表示动画已经完成，此时所有路径边都显示为绿色。
        currentStep_ = static_cast<int>(edgePath_.size());
        timer_->stop();
    }
    update();
}

int GraphWidget::currentVertex() const {
    if (vertexPath_.empty()) {
        return -1;
    }
    if (currentStep_ < 0) {
        return vertexPath_.front();
    }
    if (currentStep_ >= static_cast<int>(edgePath_.size())) {
        return vertexPath_.back();
    }

    // 正在走第 currentStep 条边时，当前顶点显示为这条边走完后的顶点。
    const int index = std::min(currentStep_ + 1, static_cast<int>(vertexPath_.size()) - 1);
    return vertexPath_[index];
}

QColor GraphWidget::edgeColor(int edgeId) const {
    const QColor gray(170, 170, 170);
    const QColor green(46, 160, 67);
    const QColor orange(245, 130, 32);

    if (edgeId < 0 || edgeId >= static_cast<int>(edgeOrder_.size())) {
        return gray;
    }

    const int order = edgeOrder_[edgeId];

    // 灰色：未访问；橙色：当前边；绿色：已经走过。
    if (order < 0 || currentStep_ < 0) {
        return gray;
    }
    if (currentStep_ >= static_cast<int>(edgePath_.size())) {
        return green;
    }
    if (order < currentStep_) {
        return green;
    }
    if (order == currentStep_) {
        return orange;
    }
    return gray;
}

std::vector<QPointF> GraphWidget::computeVertexPositions() const {
    std::vector<QPointF> positions(graph_.vertexCount());
    if (positions.empty()) {
        return positions;
    }

    const QRectF area = rect().adjusted(70, 60, -70, -60);
    const QPointF center = area.center();
    const double radius = std::max(80.0, std::min(area.width(), area.height()) * 0.42);

    if (positions.size() == 1) {
        positions[0] = center;
        return positions;
    }

    // 为了界面简单清晰，顶点固定摆在圆周上，不额外做复杂布局。
    constexpr double pi = 3.14159265358979323846;
    for (int i = 0; i < static_cast<int>(positions.size()); ++i) {
        const double angle = -pi / 2.0 + 2.0 * pi * i / positions.size();
        positions[i] = QPointF(center.x() + radius * std::cos(angle),
                               center.y() + radius * std::sin(angle));
    }
    return positions;
}

QPainterPath GraphWidget::makeEdgePath(const Edge& edge,
                                       const std::vector<QPointF>& positions,
                                       QPointF* labelPoint) const {
    QPainterPath path;
    if (edge.u < 0 || edge.u >= static_cast<int>(positions.size()) ||
        edge.v < 0 || edge.v >= static_cast<int>(positions.size())) {
        return path;
    }

    const QPointF a = positions[edge.u];
    const QPointF b = positions[edge.v];
    const double rank = (edge.id >= 0 && edge.id < static_cast<int>(edgeOffsetRank_.size()))
                            ? edgeOffsetRank_[edge.id]
                            : 0.0;

    if (edge.u == edge.v) {
        // 自环画成顶点上方的三次贝塞尔曲线。
        const double radius = 24.0 + std::abs(rank) * 10.0;
        const double shift = rank * 12.0;
        const QPointF start = a + QPointF(-10.0, -15.0);
        const QPointF end = a + QPointF(10.0, -15.0);
        const QPointF c1 = a + QPointF(-radius + shift, -radius * 1.9);
        const QPointF c2 = a + QPointF(radius + shift, -radius * 1.9);
        path.moveTo(start);
        path.cubicTo(c1, c2, end);
        if (labelPoint) {
            *labelPoint = a + QPointF(shift, -radius * 1.65);
        }
        return path;
    }

    const QLineF line(a, b);
    const double length = std::max(1.0, line.length());
    const QPointF normal(-(b.y() - a.y()) / length, (b.x() - a.x()) / length);
    const double offset = rank * 28.0;
    const QPointF mid = (a + b) / 2.0;
    const QPointF control = mid + normal * offset;

    path.moveTo(a);
    if (std::abs(offset) < 0.1) {
        // 没有重边偏移时，用直线即可。
        path.lineTo(b);
        if (labelPoint) {
            *labelPoint = mid;
        }
    } else {
        // 有重边时，用二次贝塞尔曲线错开显示。
        path.quadTo(control, b);
        if (labelPoint) {
            *labelPoint = (a + control * 2.0 + b) / 4.0;
        }
    }
    return path;
}

void GraphWidget::drawCenteredText(QPainter& painter, const QPointF& center, const QString& text) const {
    const QFontMetrics metrics(painter.font());
    QRect textRect = metrics.boundingRect(text).adjusted(-5, -3, 5, 3);
    textRect.moveCenter(center.toPoint());

    // 给边标签加一个浅色底，避免文字压在线上看不清。
    painter.setPen(QPen(QColor(210, 210, 210), 1));
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.drawRoundedRect(textRect, 3, 3);

    painter.setPen(QColor(50, 70, 100));
    painter.drawText(textRect, Qt::AlignCenter, text);
}
