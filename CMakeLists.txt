cmake_minimum_required(VERSION 3.16)

project(EulerPathVisualizer LANGUAGES CXX)

# 按课程要求使用 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件里用了 Q_OBJECT，开启 AUTOMOC 后 CMake 会自动调用 Qt 的 moc。
set(CMAKE_AUTOMOC ON)

# 兼容 Qt6 和 Qt5，电脑上装了哪个版本就链接哪个版本的 Widgets 模块。
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(EulerPathVisualizer
    main.cpp
    graph.h
    graph.cpp
    euler_solver.h
    euler_solver.cpp
    graph_widget.h
    graph_widget.cpp
    main_window.h
    main_window.cpp
)

target_link_libraries(EulerPathVisualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

add_executable(euler_tests
    tests/test_euler_solver.cpp
    graph.cpp
    euler_solver.cpp
)

target_include_directories(euler_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(euler_tests PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

enable_testing()
add_test(NAME EulerSolverTests COMMAND euler_tests)
set_tests_properties(EulerSolverTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(gui_screenshots
    tests/capture_gui_screenshots.cpp
    graph.cpp
    euler_solver.cpp
    graph_widget.cpp
    main_window.cpp
)

target_include_directories(gui_screenshots PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(gui_screenshots PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(gui_screenshots PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
