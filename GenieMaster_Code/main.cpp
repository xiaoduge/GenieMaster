#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QTranslator trans;
    if(trans.load("myapp_chinese.qm"))
    {
        a.installTranslator(&trans);
    }
    w.show();

    return a.exec();
}
