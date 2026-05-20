#include "main_window.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      vertexSpin_(nullptr),
      edgeSpin_(nullptr),
      edgeInput_(nullptr),
      loadButton_(nullptr),
      solveButton_(nullptr),
      playButton_(nullptr),
      resetButton_(nullptr),
      graphWidget_(nullptr),
      statusLabel_(nullptr),
      pathOutput_(nullptr),
      logOutput_(nullptr),
      graphLoaded_(false),
      solved_(false) {
    setupUi();
    connectSignals();
}

void MainWindow::setupUi() {
    setWindowTitle("一笔画问题可视化求解系统");
    resize(1180, 720);

    // 主界面使用三栏结构：左侧输入，中间绘图，右侧结果和日志。
    auto* central = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    rootLayout->addWidget(splitter);

    auto* inputPanel = new QWidget(splitter);
    inputPanel->setMinimumWidth(270);
    inputPanel->setMaximumWidth(330);
    auto* inputLayout = new QVBoxLayout(inputPanel);

    auto* inputGroup = new QGroupBox("输入区", inputPanel);
    auto* formLayout = new QFormLayout(inputGroup);

    vertexSpin_ = new QSpinBox(inputGroup);
    vertexSpin_->setRange(1, 1000);
    vertexSpin_->setValue(5);
    formLayout->addRow("顶点数 P：", vertexSpin_);

    edgeSpin_ = new QSpinBox(inputGroup);
    edgeSpin_->setRange(0, 2000);
    edgeSpin_->setValue(5);
    formLayout->addRow("边数 Q：", edgeSpin_);

    edgeInput_ = new QTextEdit(inputGroup);
    edgeInput_->setPlaceholderText("每行一条边：u v\n顶点编号从 1 开始\n例如：\n1 2\n2 3\n3 1");
    edgeInput_->setPlainText("1 2\n2 3\n3 4\n4 5\n5 1");
    edgeInput_->setMinimumHeight(220);
    formLayout->addRow("边列表：", edgeInput_);

    inputLayout->addWidget(inputGroup);

    loadButton_ = new QPushButton("载入图", inputPanel);
    solveButton_ = new QPushButton("判断", inputPanel);
    playButton_ = new QPushButton("播放动画", inputPanel);
    resetButton_ = new QPushButton("重置", inputPanel);

    inputLayout->addWidget(loadButton_);
    inputLayout->addWidget(solveButton_);
    inputLayout->addWidget(playButton_);
    inputLayout->addWidget(resetButton_);
    inputLayout->addStretch();

    graphWidget_ = new GraphWidget(splitter);

    auto* resultPanel = new QWidget(splitter);
    resultPanel->setMinimumWidth(330);
    resultPanel->setMaximumWidth(420);
    auto* resultLayout = new QVBoxLayout(resultPanel);

    auto* resultGroup = new QGroupBox("结果区", resultPanel);
    auto* resultGroupLayout = new QVBoxLayout(resultGroup);

    statusLabel_ = new QLabel("请先载入图。", resultGroup);
    statusLabel_->setWordWrap(true);
    resultGroupLayout->addWidget(statusLabel_);

    pathOutput_ = new QTextEdit(resultGroup);
    pathOutput_->setReadOnly(true);
    pathOutput_->setMinimumHeight(120);
    resultGroupLayout->addWidget(new QLabel("路径输出：", resultGroup));
    resultGroupLayout->addWidget(pathOutput_);

    logOutput_ = new QTextEdit(resultGroup);
    logOutput_->setReadOnly(true);
    resultGroupLayout->addWidget(new QLabel("算法日志：", resultGroup));
    resultGroupLayout->addWidget(logOutput_, 1);

    resultLayout->addWidget(resultGroup);

    splitter->addWidget(inputPanel);
    splitter->addWidget(graphWidget_);
    splitter->addWidget(resultPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);

    setCentralWidget(central);
}

void MainWindow::connectSignals() {
    connect(loadButton_, &QPushButton::clicked, this, [this]() { loadGraphFromInput(); });
    connect(solveButton_, &QPushButton::clicked, this, [this]() { solveGraph(); });
    connect(playButton_, &QPushButton::clicked, this, [this]() {
        // 如果用户直接点“播放动画”，先自动运行一次判断。
        if (!solved_) {
            solveGraph();
        }
        if (!solved_) {
            return;
        }
        if (!result_.hasEulerPath) {
            QMessageBox::information(this, "提示", "当前图不存在一笔画路径，无法播放动画。");
            return;
        }
        if (result_.edgePath.empty()) {
            QMessageBox::information(this, "提示", "图中没有边，欧拉路径为空。");
            return;
        }
        graphWidget_->startAnimation();
    });
    connect(resetButton_, &QPushButton::clicked, this, [this]() { resetAll(); });
}

bool MainWindow::loadGraphFromInput() {
    const int vertexCount = vertexSpin_->value();
    const int expectedEdgeCount = edgeSpin_->value();

    // 支持每行一条边，空行会被忽略；逗号、中文逗号和空白都可以作为分隔符。
    QStringList lines = edgeInput_->toPlainText().split(QRegularExpression("\\R"), Qt::SkipEmptyParts);
    for (QString& line : lines) {
        line = line.trimmed();
    }
    lines.removeAll(QString());

    if (lines.size() != expectedEdgeCount) {
        showInputError(QString("边数 Q=%1，但边列表中实际读取到 %2 条非空边。")
                           .arg(expectedEdgeCount)
                           .arg(lines.size()));
        return false;
    }

    Graph newGraph(vertexCount);
    for (int i = 0; i < lines.size(); ++i) {
        const QStringList parts = lines[i].split(QRegularExpression("[,，\\s]+"), Qt::SkipEmptyParts);
        if (parts.size() != 2) {
            showInputError(QString("第 %1 条边格式错误，应为两个顶点编号，例如：1 2。").arg(i + 1));
            return false;
        }

        bool okU = false;
        bool okV = false;
        const int u = parts[0].toInt(&okU);
        const int v = parts[1].toInt(&okV);
        if (!okU || !okV) {
            showInputError(QString("第 %1 条边包含非整数顶点编号。").arg(i + 1));
            return false;
        }
        if (u < 1 || u > vertexCount || v < 1 || v > vertexCount) {
            showInputError(QString("第 %1 条边顶点越界，顶点编号必须在 1 到 %2 之间。")
                               .arg(i + 1)
                               .arg(vertexCount));
            return false;
        }

        // 界面输入采用 1 基编号，Graph 内部采用 0 基编号。
        newGraph.addEdge(u - 1, v - 1);
    }

    // 输入完全合法后再替换当前图，避免半成品图进入界面。
    graph_ = newGraph;
    graphLoaded_ = true;
    solved_ = false;
    result_ = EulerResult();
    graphWidget_->setGraph(graph_);
    statusLabel_->setText(QString("已载入图：%1 个顶点，%2 条边。").arg(vertexCount).arg(expectedEdgeCount));
    pathOutput_->clear();
    logOutput_->setPlainText("图已载入。点击“判断”运行欧拉路径算法。");
    return true;
}

void MainWindow::solveGraph() {
    if (!loadGraphFromInput()) {
        return;
    }

    // 算法只在 EulerSolver 中执行，按钮槽函数不直接写 BFS/Hierholzer。
    EulerSolver solver;
    result_ = solver.solve(graph_);
    solved_ = true;
    graphWidget_->setEulerPath(result_.edgePath, result_.vertexPath);
    showResult();
}

void MainWindow::resetAll() {
    graph_.clear();
    result_ = EulerResult();
    graphLoaded_ = false;
    solved_ = false;
    edgeInput_->clear();
    edgeSpin_->setValue(0);
    graphWidget_->clearAll();
    statusLabel_->setText("已重置。");
    pathOutput_->clear();
    logOutput_->clear();
}

void MainWindow::showInputError(const QString& message) {
    graphLoaded_ = false;
    solved_ = false;
    result_ = EulerResult();
    graphWidget_->clearAll();
    statusLabel_->setText("输入错误：" + message);
    pathOutput_->clear();
    logOutput_->setPlainText(message);
    QMessageBox::warning(this, "输入错误", message);
}

void MainWindow::showResult() {
    QStringList logs;
    for (const std::string& log : result_.logs) {
        logs << QString::fromStdString(log);
    }
    logOutput_->setPlainText(logs.join('\n'));

    // 根据 EulerResult 更新结果区；界面不重新推导算法结论。
    if (!result_.hasEulerPath) {
        statusLabel_->setText("判断结果：不存在一笔画。");
        pathOutput_->setPlainText("不存在可覆盖所有边且每条边恰好走一次的一笔画路径。");
        return;
    }

    statusLabel_->setText(result_.isCircuit ? "判断结果：存在欧拉回路。" : "判断结果：存在欧拉通路。");
    QString output;
    output += result_.isCircuit ? "类型：欧拉回路\n" : "类型：欧拉通路\n";
    output += "顶点路径：\n" + formatVertexPath(result_.vertexPath) + "\n\n";
    output += "边序列：\n" + formatEdgePath(result_.edgePath);
    pathOutput_->setPlainText(output);
}

QString MainWindow::formatVertexPath(const std::vector<int>& vertexPath) const {
    if (vertexPath.empty()) {
        return "空路径";
    }

    QStringList parts;
    for (int vertex : vertexPath) {
        parts << QString::number(vertex + 1);
    }
    return parts.join(" -> ");
}

QString MainWindow::formatEdgePath(const std::vector<int>& edgePath) const {
    if (edgePath.empty()) {
        return "空路径（图中没有边）";
    }

    QStringList parts;
    for (int edgeId : edgePath) {
        parts << QString("e%1").arg(edgeId + 1);
    }
    return parts.join(" -> ");
}
