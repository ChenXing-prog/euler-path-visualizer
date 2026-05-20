#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "euler_solver.h"
#include "graph.h"
#include "graph_widget.h"

#include <QMainWindow>

class QLabel;
class QPushButton;
class QSpinBox;
class QTextEdit;

// MainWindow 是界面协调层：
// - 从输入框读取 P、Q 和边列表；
// - 调用 Graph 构图，调用 EulerSolver 求解；
// - 把结果交给 GraphWidget、日志框和路径框展示。
// 注意：这里不实现欧拉路径算法，只负责组织 UI 流程。
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QSpinBox* vertexSpin_;
    QSpinBox* edgeSpin_;
    QTextEdit* edgeInput_;
    QPushButton* loadButton_;
    QPushButton* solveButton_;
    QPushButton* playButton_;
    QPushButton* resetButton_;
    GraphWidget* graphWidget_;
    QLabel* statusLabel_;
    QTextEdit* pathOutput_;
    QTextEdit* logOutput_;

    Graph graph_;
    EulerResult result_;
    bool graphLoaded_;
    bool solved_;

    void setupUi();
    void connectSignals();

    // 读取并校验输入，成功后更新 graph_ 和图形显示区。
    bool loadGraphFromInput();

    // 调用 EulerSolver，保存 EulerResult，再刷新结果区。
    void solveGraph();
    void resetAll();
    void showInputError(const QString& message);
    void showResult();
    QString formatVertexPath(const std::vector<int>& vertexPath) const;
    QString formatEdgePath(const std::vector<int>& edgePath) const;
};

#endif
