#include <QApplication>
#include <QSplashScreen>
#include <QTimer>

#include "mainwindow.h"
#include "mysyntaxhighlighter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSplashScreen* splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/images/images/chip03_splash.png"));
    splash->show();

    MainWindow w;

    QTimer::singleShot(1500, splash, SLOT(close()));
    QTimer::singleShot(1500, &w, SLOT(show()));

    return a.exec();
}
