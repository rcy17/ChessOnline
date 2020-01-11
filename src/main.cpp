#include "mainwindow.h"
#include "starter.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     //MainWindow w;
    Starter w;
    w.show();

    return a.exec();
}
