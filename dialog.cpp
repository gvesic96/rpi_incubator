#include "dialog.h"
#include "ui_dialog.h"

#include <QTimer>
#include <QTime>
#include <QDate>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>

#include <wiringPiI2C.h>
#include <wiringPi.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_checkBox_4_clicked(bool checked)
{
    if(checked){
        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
        ui->checkBox_3->setChecked(false);
    }
}
