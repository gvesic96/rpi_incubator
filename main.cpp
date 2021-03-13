#include "dialog.h"
#include <QApplication>
#include <wiringPiI2C.h>
#include <wiringPi.h>

int main(int argc, char *argv[])
{
    wiringPiSetup();

    //pinMode(28, OUTPUT);
    //pinMode(29, OUTPUT);


    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
