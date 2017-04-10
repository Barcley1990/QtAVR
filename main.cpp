#include "mainwindow.h"
#include "mysyntaxhighlighter.h"

#include <QApplication>
#include <qprocess.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();



    return a.exec();
}
