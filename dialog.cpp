#include "dialog.h"
#include "ui_dialog.h"

#include <QTimer>
#include <QTime>
#include <QDate>
//#include <QMessageBox>
//#include <QFile>
//#include <QTextStream>
//#include <QScrollBar>

#include <wiringPiI2C.h>
#include <wiringPi.h>

#include <QPixmap>

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

void Dialog::on_checkBox_clicked(bool checked)
{
    if(checked){
        ui->checkBox_2->setChecked(false);
        ui->checkBox_3->setChecked(false);
        ui->checkBox_4->setChecked(false);

        ui->label_3->setPixmap(QPixmap(":/slike/goose128.png"));

        ui->horizontalSlider->setValue(28);
        ui->label_7->setText("28 D");
        ui->horizontalSlider_2->setValue(385);
        ui->label_8->setText("38.0 C");
        ui->horizontalSlider_3->setValue(850);
        ui->label_9->setText("85.0 %");
    }
}

void Dialog::on_checkBox_2_clicked(bool checked)
{
    if(checked){
        ui->checkBox->setChecked(false);
        ui->checkBox_3->setChecked(false);
        ui->checkBox_4->setChecked(false);

        ui->label_3->setPixmap(QPixmap(":/slike/duck128.png"));

        ui->horizontalSlider->setValue(28);
        ui->label_7->setText("28 D");
        ui->horizontalSlider_2->setValue(385);
        ui->label_8->setText("38.5 C");
        ui->horizontalSlider_3->setValue(850);
        ui->label_9->setText("85.0 %");
    }
}

void Dialog::on_checkBox_3_clicked(bool checked)
{
    if(checked){
        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
        ui->checkBox_4->setChecked(false);

        ui->label_3->setPixmap(QPixmap(":/slike/chicken128.png"));

        ui->horizontalSlider->setValue(21);
        ui->label_7->setText("21 D");
        ui->horizontalSlider_2->setValue(375);
        ui->label_8->setText("37.5 C");
        ui->horizontalSlider_3->setValue(850);
        ui->label_9->setText("85.0 %");
    }
}

void Dialog::on_checkBox_4_clicked(bool checked)
{
    if(checked){
        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
        ui->checkBox_3->setChecked(false);

        ui->label_3->setPixmap(QPixmap(":/slike/egg128.png"));

    }

}

void Dialog::on_horizontalSlider_valueChanged(int value)
{
    bool permission;
        permission=ui->checkBox_4->isChecked();
        if(permission){
            QString days_s;
                    days_s.setNum(value);
            ui->label_7->setText(days_s+" D");
        }

}

void Dialog::on_horizontalSlider_2_valueChanged(int value)
{
    bool permission;
        permission=ui->checkBox_4->isChecked();
        if(permission){
            double temp;
            temp=0.1*value;
            QString temp_s;
                    temp_s.setNum(temp);
            ui->label_8->setText(temp_s+" C");
        }

}

void Dialog::on_horizontalSlider_3_valueChanged(int value)
{
    bool permission;
        permission=ui->checkBox_4->isChecked();
        if(permission){
            double humi;
            humi=0.1*value;
            QString humi_s;
                    humi_s.setNum(humi);
            ui->label_9->setText(humi_s+" %");
        }

}
