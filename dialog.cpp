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

#include <sys/fcntl.h>

bool start_sig = 0;
int d_target = 0;
int t_target = 0;
int h_target = 0;
//int mode = 0;

int sense_temp();


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{

    ui->setupUi(this);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(led_blink()));
    timer->start(1000);

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

        ui->label_3->setPixmap(QPixmap(":/slike/chicken128.png"));

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

        ui->label_3->setPixmap(QPixmap(":/slike/goose128.png"));

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

void Dialog::on_pushButton_2_clicked()
{
    start_sig=1;

    d_target = ui->horizontalSlider->value();
    t_target = ui->horizontalSlider_2->value();
    h_target = ui->horizontalSlider_3->value();



    /*if(ui->checkBox->isChecked())  {mode = 1;}
    if(ui->checkBox_2->isChecked()){mode = 2;}
    if(ui->checkBox_3->isChecked()){mode = 3;}
    if(ui->checkBox_4->isChecked()){mode = 4;}

    switch(mode){
        case 1 :
            //days = ui->horizontalSlider->value();
            //temp = ui->horizontalSlider_2->value();
            //humi = ui->horizontalSlider_3->value();
            break;
        case 2 :
            //days = ui->horizontalSlider->value();
            //temp = ui->horizontalSlider_2->value();
            //humi = ui->horizontalSlider_3->value();
            break;
        case 3 :
            //days = ui->horizontalSlider->value();
            //temp = ui->horizontalSlider_2->value();
            //humi = ui->horizontalSlider_3->value();
            break;
        case 4 :
            //days = ui->horizontalSlider->value();
            //temp = ui->horizontalSlider_2->value();
            //humi = ui->horizontalSlider_3->value();
            break;
    }*/
}

void Dialog::on_pushButton_clicked()
{
    start_sig=0;
}

void Dialog::led_blink(){


    if(start_sig){
        int a;
        a = sense_temp();
        QString value_1_s;
                value_1_s.setNum(a);
        ui->label_15->setText(value_1_s+" C");

    }else {
        digitalWrite(28, LOW);
        }
}

int sense_temp(){
    //int temp = 0;
    int fd = -1, ret;

        char *tmp1, tmp2[10], ch='t';
        char devname_head[50]="/sys/devices/w1_bus_master1/28-030079a25e9c";
        char devname_end[10] = "/w1_slave";
        char dev_name[100];
        int value;
        char buffer[100];
        //int i,j;

        strcpy(dev_name, devname_head);
        strcat(dev_name, devname_end);

    if((fd = open(dev_name, O_RDONLY))<0)
        {
            //perror("Greska pri otvaranju");
            exit(1);
        }

        ret = read(fd, buffer, sizeof(buffer));

        if(ret<0)
        {
            //perror("Greska pri citanju!");
            exit(1);
        }

        tmp1 = strchr(buffer, ch);
        sscanf(tmp1, "t=%s", tmp2);
        value = atoi(tmp2);

        double temp_treshold;
        temp_treshold=0.1*t_target;

        double value_1;
        value_1 = 0.001*value;

        if(value_1>temp_treshold){
            digitalWrite(28, HIGH);
            }
            else
            digitalWrite(28, LOW);

        close(fd);


    return value;
}
