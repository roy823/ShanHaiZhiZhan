#include "ui/scenes/mainwindow.h"
#include <QApplication>
#include <QDebug> // Required for qDebug, qWarning, etc.

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}