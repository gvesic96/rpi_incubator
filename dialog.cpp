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

long sense_temp(void);
double dht_read(void);

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

        ui->horizontalSlider->setDisabled(true);
        ui->horizontalSlider_2->setDisabled(true);
        ui->horizontalSlider_3->setDisabled(true);
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

        ui->horizontalSlider->setDisabled(true);
        ui->horizontalSlider_2->setDisabled(true);
        ui->horizontalSlider_3->setDisabled(true);
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

        ui->horizontalSlider->setDisabled(true);
        ui->horizontalSlider_2->setDisabled(true);
        ui->horizontalSlider_3->setDisabled(true);
    }
}

void Dialog::on_checkBox_4_clicked(bool checked)
{
    if(checked){
        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
        ui->checkBox_3->setChecked(false);

        ui->label_3->setPixmap(QPixmap(":/slike/egg128.png"));

        ui->horizontalSlider->setDisabled(false);
        ui->horizontalSlider_2->setDisabled(false);
        ui->horizontalSlider_3->setDisabled(false);
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

    ui->horizontalSlider->setDisabled(true);
    ui->horizontalSlider_2->setDisabled(true);
    ui->horizontalSlider_3->setDisabled(true);


}

void Dialog::on_pushButton_clicked()
{
    start_sig=0;


    ui->horizontalSlider->setDisabled(false);
    ui->horizontalSlider_2->setDisabled(false);
    ui->horizontalSlider_3->setDisabled(false);
}

void Dialog::led_blink(){


    if(start_sig){
        long temp;
        temp = sense_temp();
        double t;
        t=0.001*temp;
        QString value_str;
                value_str.setNum(t);
        ui->label_15->setText(value_str+" C");

        double humi;
        humi = dht_read();
        QString value_h;
                value_h.setNum(humi);
        ui->label_16->setText(value_h+" %");

    }else {
        digitalWrite(28, LOW);
        }
}

long sense_temp(void){

    int fd = -1, ret;

        char *tmp1, tmp2[10], ch='t';
        char dev_name[100]="/sys/devices/w1_bus_master1/28-030079a25e9c/w1_slave";
        long temp;
        char buffer[100];
        long temp_treshold;

        temp_treshold = 100*t_target;

    if((fd = open(dev_name, O_RDONLY))<0)
        {
            perror("Greska pri otvaranju");
            exit(1);
        }

        ret = read(fd, buffer, sizeof(buffer));

        if(ret<0)
        {
            perror("Greska pri citanju!");
            exit(1);
        }

        tmp1 = strchr(buffer, ch);
        sscanf(tmp1, "t=%s", tmp2);
        temp = atoi(tmp2);


        if(temp>temp_treshold){
            digitalWrite(28, HIGH);
            }
            else
            digitalWrite(28, LOW);

        close(fd);
    return temp;

}


double dht_read(void){

    int bits[250], data[100];
    int bitidx = 0;

    int counter = 0;
    int laststate = HIGH;
    int j=0;

    //set GPIO pin to output
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);
    delay(500);
    //usleep(500000);
    digitalWrite(3, LOW);
    delay(10);
    //usleep(10000);

    pinMode(3, INPUT);

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    //wait for pin to drop?
    while(digitalRead(3) == 1){
      usleep(1);
    }

    //read data
    for (int i=0; i<100; i++){
      counter = 0;
      while(digitalRead(3) == laststate){
      counter++;
      if(counter == 1000)
        break;
      }
    laststate = digitalRead(3);
    if(counter == 1000) break;
    bits[bitidx++] = counter;

      if((i>3) && (i%2 == 0)){
        data[j/8] <<= 1;
          if(counter > 200)
          data[j/8] |= 1;
        j++;
      }
    }


  //printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

  double hum=0;

  if ((j >= 39) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
     // yay!
    //float f, h;
    double h;
    h = data[0] * 256 + data[1];
    h /= 10;

    hum=h;
    /*f = (data[2] & 0x7F)* 256 + data[3];
        f /= 10.0;
        if (data[2] & 0x80)  f *= -1;
    printf("Temp =  %.1f *C, Hum = %.1f \%\n", f, h);

    return 1;*/
  }

  return hum;
}

