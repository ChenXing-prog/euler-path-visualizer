# 一笔画问题可视化求解系统

这是一个基于 **C++17 + Qt Widgets** 的一笔画问题可视化求解系统。项目重点是算法设计，Qt 主要用于输入、绘图、日志展示和动画演示。

## 功能特点

- 支持输入无向图的顶点数、边数和边列表
- 支持重边，每条边使用唯一 `edgeId` 区分
- 使用 BFS 判断所有非孤立顶点是否连通
- 根据奇度顶点数量判断欧拉通路或欧拉回路
- 使用 Hierholzer 算法构造欧拉路径
- 使用自定义 `QWidget` 绘制图形
- 使用 `QTimer` 每 500ms 高亮一条路径边
- 提供固定测试用例和随机批量测试

## 算法说明

无向图存在一笔画路径需要满足：

1. 所有非孤立顶点连通；
2. 奇度顶点数量为 0 或 2。

其中：

- 0 个奇度顶点：存在欧拉回路；
- 2 个奇度顶点：存在欧拉通路；
- 其他情况：不存在一笔画。

若满足条件，系统使用 Hierholzer 算法构造具体路径。算法整体时间复杂度为 `O(P + Q)`。

## 项目结构

```text
.
├── CMakeLists.txt
├── main.cpp
├── graph.h / graph.cpp
├── euler_solver.h / euler_solver.cpp
├── graph_widget.h / graph_widget.cpp
├── main_window.h / main_window.cpp
├── tests/
│   ├── test_euler_solver.cpp
│   └── capture_gui_screenshots.cpp
├── TEST_CASES.md
└── test_results.md
```

## 构建运行

```bash
cmake -S . -B build
cmake --build build
./build/EulerPathVisualizer
```

## 运行测试

```bash
./build/euler_tests
ctest --test-dir build --output-on-failure
```

## 测试结果

项目包含两层测试：

- 固定代表性测试用例，用于课程论文展示；
- 1000 组随机图批量测试，用于验证算法稳定性。

测试结果见 [test_results.md](test_results.md)。

