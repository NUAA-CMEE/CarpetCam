#include "mainwindow.h"
#include <QApplication>
#include"cv.h"
#include"highgui.h"

#include<showpic.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    MainWindow w;
    w.show();


    return a.exec();
}

