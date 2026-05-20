#include "main_window.h"

#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QThread>

#include <iostream>
#include <stdexcept>
#include <string>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

namespace {

void processEvents(int milliseconds = 80) {
    const int step = 20;
    for (int elapsed = 0; elapsed < milliseconds; elapsed += step) {
        QApplication::processEvents();
        QThread::msleep(step);
    }
    QApplication::processEvents();
}

QPushButton* findButton(MainWindow& window, const QString& text) {
    const QList<QPushButton*> buttons = window.findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }
    throw std::runtime_error(("找不到按钮：" + text).toStdString());
}

QSpinBox* findSpinBoxByMaximum(MainWindow& window, int maximum) {
    const QList<QSpinBox*> boxes = window.findChildren<QSpinBox*>();
    for (QSpinBox* box : boxes) {
        if (box->maximum() == maximum) {
            return box;
        }
    }
    throw std::runtime_error("找不到指定范围的 QSpinBox");
}

QTextEdit* findInputEditor(MainWindow& window) {
    const QList<QTextEdit*> editors = window.findChildren<QTextEdit*>();
    for (QTextEdit* editor : editors) {
        if (!editor->isReadOnly()) {
            return editor;
        }
    }
    throw std::runtime_error("找不到边列表输入框");
}

void setInput(MainWindow& window, int vertexCount, const QString& edgeText) {
    const int edgeCount = edgeText.trimmed().isEmpty()
                              ? 0
                              : edgeText.trimmed().split('\n', Qt::SkipEmptyParts).size();
    findSpinBoxByMaximum(window, 1000)->setValue(vertexCount);
    findSpinBoxByMaximum(window, 2000)->setValue(edgeCount);
    findInputEditor(window)->setPlainText(edgeText);
    processEvents();
}

void saveWindow(MainWindow& window, const QString& fileName) {
    processEvents(120);
    const QPixmap pixmap = window.grab();
    const QString path = QDir(QString::fromUtf8(PROJECT_SOURCE_DIR)).filePath("docs/screenshots/" + fileName);
    if (!pixmap.save(path)) {
        throw std::runtime_error(("截图保存失败：" + path).toStdString());
    }
    std::cout << "saved " << path.toStdString() << '\n';
}

} // namespace

int main(int argc, char* argv[]) {
    if (qgetenv("QT_QPA_PLATFORM").isEmpty()) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    QApplication app(argc, argv);
    MainWindow window;
    window.resize(1180, 720);
    window.show();
    processEvents();

    try {
        QPushButton* solveButton = findButton(window, "判断");
        QPushButton* playButton = findButton(window, "播放动画");

        setInput(window, 4, "1 2\n2 3\n1 3\n1 4\n3 4");
        solveButton->click();
        saveWindow(window, "case_euler_path.png");

        setInput(window, 4, "1 2\n1 3\n1 4");
        solveButton->click();
        saveWindow(window, "case_no_path.png");

        setInput(window, 3, "1 2\n2 3\n3 1");
        solveButton->click();
        playButton->click();
        processEvents(180);
        saveWindow(window, "case_animation.png");
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
