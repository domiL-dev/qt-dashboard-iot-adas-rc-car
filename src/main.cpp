#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Dashboard window;
    window.show();
    window.resize(400, 300);
    return app.exec();
}
