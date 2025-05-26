// src/main.cpp
#include <QApplication>
#include "ui/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 设置应用信息
    QApplication::setApplicationName("山海之战"); // 应用名称
    QApplication::setApplicationVersion("1.0");    // 应用版本
    QApplication::setOrganizationName("山海工作室"); // 组织名称

    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();

    // 运行应用
    return app.exec();
}