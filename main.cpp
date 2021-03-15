#include "dialog.h"
#include <QApplication>
#include <wiringPiI2C.h>
#include <wiringPi.h>

int main(int argc, char *argv[])
{
    wiringPiSetup();
    if(wiringPiSetup() == -1) exit(1);

    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
