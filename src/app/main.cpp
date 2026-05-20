#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    // Qt Widgets 程序入口：创建 QApplication，再显示主窗口。
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
