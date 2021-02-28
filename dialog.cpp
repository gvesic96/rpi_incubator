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

#define DHTPIN 3
#define SERVOPIN 1
#define SW_PIN_L 26
#define SW_PIN_R 27



#define ROT_PERIOD 6

#define changeHexToInt(hex) ((((hex)>>4)*10)+((hex)%16))
#define SEK 0x00
#define MIN 0x01
#define SAT 0x02
#define DAY 0x03
#define DATE  0x04
#define MONTH 0x05
#define YEAR  0x06


unsigned char ds3231_Store[7];
unsigned char init3231_Store[7]={0x01,0x00,0x05,0x01,0x01,0x01,0x01};
//unsigned char init3231_Store[7]={0x00,0x59,0x23,0x02,0x31,0x12,0x20};

void DS3231_Readtime(void);
unsigned char count_days(void);
void DS3231_init(void);

bool rot = 0;
bool start_sig = 0;
int d_target = 0;
int t_target = 0;
int h_target = 0;


long sense_temp(void);
int dht_read(void);
void open_hatch(void);
void close_hatch(void);
//void period_rotation(void);
void rotation_check(void);
void rotate_left(void);
void rotate_right(void);
void stall(void);

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{

    ui->setupUi(this);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(led_blink()));
    timer->start(1000);

    QTimer *timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(period_rotation()));
    timer1->start(200);

    pinMode(SERVOPIN, OUTPUT);

    pinMode(SW_PIN_L, INPUT);
    pinMode(SW_PIN_R, INPUT);

}


void Dialog::led_blink(){


    if(start_sig){
        long temp;
        temp = sense_temp();
        double t;
        t=0.001*temp;
        QString value_t;
                value_t.setNum(t);
        ui->label_15->setText(value_t+" C");

        int humi;
        humi = dht_read();

        double humi_val;
        humi_val = 0.1*humi;
        QString value_h;
                value_h.setNum(humi_val);
        ui->label_16->setText(value_h+" %");

        if(humi>h_target){
            open_hatch();
            }else {
            close_hatch();
        }

        unsigned char days;
        days = count_days();
        QString value_d;
                value_d.setNum(days);
        ui->label_14->setText(value_d+" D");

        rotation_check();

        ui->label_17->setNum(rot);
        ui->label_18->setNum(ds3231_Store[0]);

    }else {
        digitalWrite(25, LOW);

        }
}

void Dialog::period_rotation(){
    //called every 100ms in another timer

    int sw_l;
    int sw_r;

    if(start_sig){
    sw_l = digitalRead(SW_PIN_L);
    sw_r = digitalRead(SW_PIN_R);

        if(rot == 0){
            if(sw_l == 0){
            //rotate_left function
            rotate_left();
            }else{stall();}
        }else {if(sw_r == 0){
                //rotate_right function
                rotate_right();
                }else {stall();}
        }

    }

}

void rotate_left(void){
    //rotate left
    digitalWrite(28, HIGH);
    digitalWrite(29, LOW);
}

void rotate_right(void){
    //rotate right
    digitalWrite(28, LOW);
    digitalWrite(29, HIGH);
}

void stall(void){
    //do not rotate
    digitalWrite(28, LOW);
    digitalWrite(29, LOW);
}

long sense_temp(void){

    int fd = -1, ret;

        char *tmp1, tmp2[10], ch='t';
        char dev_name[100]="/sys/devices/w1_bus_master1/28-030079a25e9c/w1_slave";
        long t;
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
        t = atoi(tmp2);


        if(t>temp_treshold){
            digitalWrite(25, HIGH);
            }
            else
            digitalWrite(25, LOW);

        close(fd);
    return t;

}


int dht_read(void){

    //BITS[250] IS UNUSED !!!!!!!!!!!!!

    int bits[250], data[100];
    int bitidx = 0;

    int counter = 0;
    int laststate = HIGH;
    int j=0;

    //set GPIO pin to output
    pinMode(DHTPIN, OUTPUT);

    digitalWrite(DHTPIN, HIGH);
    delay(50);
    //usleep(500000); TOO LONG DELAY
    digitalWrite(DHTPIN, LOW);
    delay(10);
    //usleep(10000);
    pinMode(DHTPIN, INPUT);

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    //wait for pin to drop?
    while(digitalRead(DHTPIN) == 1){
      usleep(1);
    }

    //read data
    for (int i=0; i<100; i++){
      counter = 0;
      while(digitalRead(DHTPIN) == laststate){
      counter++;
      if(counter == 1000)
        break;
      }
    laststate = digitalRead(DHTPIN);
    if(counter == 1000) break;
    bits[bitidx++] = counter;

      if((i>3) && (i%2 == 0)){
        data[j/8] <<= 1;
          if(counter > 200)
          data[j/8] |= 1;
        j++;
      }
    }

    int h = 0;
  if ((j >= 39) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
     // yay!
    h = data[0] * 256 + data[1];    
  }

  return h;
}


void open_hatch(void){

        digitalWrite(SERVOPIN, HIGH);
        usleep(800); //delay(1)
        digitalWrite(SERVOPIN, LOW);
        usleep(19200); //delay(19)
}

void close_hatch(void){

    digitalWrite(SERVOPIN, HIGH);
    usleep(1850); //delay(2)
    digitalWrite(SERVOPIN, LOW);
    usleep(18150); //delay(18)
}


void DS3231_settime(){

    int fd = wiringPiI2CSetup(0x68);
    for(int i=0; i<=6; i++){
        wiringPiI2CWriteReg8(fd, SEK+i, ds3231_Store[i]);
    }

}

void DS3231_init(){

    for(int i=0; i<=6; i++)
        ds3231_Store[i]=init3231_Store[i];

    DS3231_settime();
    //printf("Postavljeno pocetno tekuce vreme...\n");

    //inicijalizacija RTC-a ???????

}


void DS3231_Readtime(){

    unsigned char time[7];
    int fd =  wiringPiI2CSetup(0x68);

    for(int i=0; i<=6; i++){
        time[i] = wiringPiI2CReadReg8(fd, SEK+i);
    }

    ds3231_Store[0] = time[0] & 0x7f; //sec
    ds3231_Store[1] = time[1] & 0x7f; //min
    ds3231_Store[2] = time[2] & 0x3f; //hour

    ds3231_Store[3] = time[3] & 0x07; //day
    ds3231_Store[4] = time[4] & 0x3f; //date
    ds3231_Store[5] = time[5] & 0x1f; //month
    ds3231_Store[6] = time[6] & 0xff; //year

    for(int i=0; i<=6; i++){
        ds3231_Store[i] = changeHexToInt(ds3231_Store[i]);
    }
}

void rotation_check(void){
    //called every main timer period, in this case every second
    //executed every ROT_PERIOD hours
    DS3231_Readtime();

    int sw_l;
    int sw_r;

    unsigned char mod = 0;
    /*mod = ds3231_Store[2] % ROT_PERIOD;
    if(ds3231_Store[0] == 0 && ds3231_Store[1] == 0 && mod == 0){*/
    //isprobavanje ISPOD NE TREBA OVAKO
    mod = ds3231_Store[0] % 30;
    if(mod == 0){
        //rot=!rot;
        sw_l = digitalRead(SW_PIN_L);
        sw_r = digitalRead(SW_PIN_R);

        if(sw_l == 0 && sw_r == 1){
            rot = 0; //rotation permission for left
        }

        if(sw_l == 1 && sw_r == 0){
            rot = 1; //rotation permission for right
        }
    }
}


unsigned char count_days(void){
    DS3231_Readtime();

    unsigned char d=0;

    if(ds3231_Store[5]==1){
        d=ds3231_Store[4];
    } else if(ds3231_Store[5]==2){
                d=ds3231_Store[4] + 31;
    } else if(ds3231_Store[5]==3){
                d=ds3231_Store[4] + 31 + 28;
    }

    return d;
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

    DS3231_init();

    d_target = ui->horizontalSlider->value();
    t_target = ui->horizontalSlider_2->value();
    h_target = ui->horizontalSlider_3->value();

    ui->horizontalSlider->setDisabled(true);
    ui->horizontalSlider_2->setDisabled(true);
    ui->horizontalSlider_3->setDisabled(true);

    ui->checkBox->setDisabled(true);
    ui->checkBox_2->setDisabled(true);
    ui->checkBox_3->setDisabled(true);
    ui->checkBox_4->setDisabled(true);


}

void Dialog::on_pushButton_clicked()
{
    start_sig=0;


    ui->checkBox->setDisabled(false);
    ui->checkBox_2->setDisabled(false);
    ui->checkBox_3->setDisabled(false);
    ui->checkBox_4->setDisabled(false);

    if(ui->checkBox_4->isChecked() == true){
    ui->horizontalSlider->setDisabled(false);
    ui->horizontalSlider_2->setDisabled(false);
    ui->horizontalSlider_3->setDisabled(false);
    }/*else {
        ui->horizontalSlider->setDisabled(true);
        ui->horizontalSlider_2->setDisabled(true);
        ui->horizontalSlider_3->setDisabled(true);
        }*/

}

Dialog::~Dialog()
{
    delete ui;
}
