#include "dialog.h"
#include "ui_dialog.h"

#include <QTimer>
#include <QTime>
#include <QDate>
#include <QMessageBox>

#include <wiringPiI2C.h>
#include <wiringPi.h>

#include <QPixmap>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include "softPwm.h"

#define DHTPIN 3
#define SERVOPIN 1
#define SW_PIN_L 26
#define SW_PIN_R 27
#define FLAG_PIN 25
#define HEATER_PIN 24

#define DRIVER_1 28
#define DRIVER_2 29


#define ROT_PERIOD 6
#define PWM_LEVEL 20


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

bool start_sig = 0;
int d_target = 0;
int t_target = 0;
int h_target = 0;

bool rot = 0;
bool rot_go = 1;
int counter = 0;

void DS3231_Readtime(void);
unsigned char count_days(void);
void DS3231_init(void);

long sense_temp(void);
int dht_read(void);
void open_hatch(void);
void close_hatch(void);

void period_rotation(void);
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
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    QTimer *timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(update_rotation()));
    timer2->start(10);

    pinMode(SERVOPIN, OUTPUT);

    pinMode(HEATER_PIN, OUTPUT);
    pinMode(FLAG_PIN, OUTPUT);

    pinMode(SW_PIN_L, INPUT);
    pinMode(SW_PIN_R, INPUT);

    softPwmCreate(DRIVER_1, 0, 100);
    softPwmCreate(DRIVER_2, 0, 100);

}


void Dialog::update_rotation(){

    //add rotation blockade for 3 days before stopping

    if(start_sig == 1 && rot_go == 1){
      period_rotation();

    }else {
      stall();
    }

}

void Dialog::update(){

    if(start_sig){

        long temp;
        temp = sense_temp();

        double temp_val;
        temp_val=0.001*temp;
        QString value_t;
                value_t.setNum(temp_val);
        ui->label_15->setText(value_t+" C");

        double t_threshold;
        t_threshold = 0.1* t_target;
        if(temp_val<t_threshold){
            digitalWrite(HEATER_PIN, HIGH);
        }else {
            digitalWrite(HEATER_PIN, LOW);
        }

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

        if(days >= d_target){start_sig = 0;}

        int rot_limit;
        rot_limit = d_target - 3;
        if(days >= rot_limit){rot_go = 0;}


    }else {

        digitalWrite(FLAG_PIN, LOW);
        digitalWrite(HEATER_PIN, LOW);
        counter=0;

        }

}


void period_rotation(void){

    int sw_l;
    int sw_r;

    sw_l = digitalRead(SW_PIN_L);
    sw_r = digitalRead(SW_PIN_R);

        if(rot == 0){
              if(sw_l == 0 && counter < 100){//changed to 20 because timer 2 100ms period
                //rotate_left function
                counter=counter+1;
                rotate_left();
              }else{
                  stall();
              }
            if(sw_l == 1 && sw_r == 0){counter = 0;}//resets safety counter

        }else {if(sw_r == 0 && counter < 100){
                  //rotate_right function
                  counter=counter+1;
                  rotate_right();
                }else{
                    stall();
                }
            if(sw_r == 1 && sw_l == 0){counter = 0;}//resets safety counter
        }

        if(counter >= 100){
            digitalWrite(FLAG_PIN, HIGH);//raise alarm rotation flag
        }
        else {
            digitalWrite(FLAG_PIN, LOW);
        }
}

void rotate_left(void){
    //rotate left
    softPwmWrite(DRIVER_1, PWM_LEVEL);//PWM
    softPwmWrite(DRIVER_2, 0);
}

void rotate_right(void){
    //rotate right
    softPwmWrite(DRIVER_1, 0);
    softPwmWrite(DRIVER_2, PWM_LEVEL); //PWM
}

void stall(void){
    //do not rotate    
    softPwmWrite(DRIVER_1, 0);
    softPwmWrite(DRIVER_2, 0);
}

long sense_temp(void){

    int fd = -1, ret;

        char *tmp1, tmp2[10], ch='t';
        char dev_name[100]="/sys/devices/w1_bus_master1/28-030079a25e9c/w1_slave";
        long t;
        char buffer[100];

    if((fd = open(dev_name, O_RDONLY))<0)
        {
            return 0;
        }

        ret = read(fd, buffer, sizeof(buffer));

        if(ret<0)
        {
            return 0;
        }

        tmp1 = strchr(buffer, ch);
        sscanf(tmp1, "t=%s", tmp2);
        t = atoi(tmp2);

        close(fd);
    return t;

}


int dht_read(void){

    int data[100];

    int counter = 0;
    int laststate = HIGH;
    int j=0;

    //set GPIO pin to output
    pinMode(DHTPIN, OUTPUT);

    //waking up sensor
    digitalWrite(DHTPIN, HIGH);
    delay(50); //longer better?
    digitalWrite(DHTPIN, LOW);//host sending start signal
    delay(10);//1-10ms

    pinMode(DHTPIN, INPUT);

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    //wait for pin to drop
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

      if((i>3) && (i%2 == 0)){
        data[j/8] <<= 1;
          if(counter > 200)
          data[j/8] |= 1;
        j++;
      }
    }

    int h = 0;
  if ((j >= 39) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {

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
    //change permission every ROT_PERIOD hours
    DS3231_Readtime();

    int sw_l;
    int sw_r;

    unsigned char mod = 0;
    /*mod = ds3231_Store[2] % ROT_PERIOD;
    if(ds3231_Store[0] == 0 && ds3231_Store[1] == 0 && mod == 0){*/
    //isprobavanje ISPOD NE TREBA OVAKO
    mod = ds3231_Store[0] % 20;
    if(mod == 0){
        //rot=!rot;
        sw_l = digitalRead(SW_PIN_L);
        sw_r = digitalRead(SW_PIN_R);

        if(sw_l == 0 && sw_r == 1){
            rot = 0; //rotation permission for left
        }else if(sw_l == 1 && sw_r == 0){
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

        ui->horizontalSlider->setValue(21);
        ui->label_7->setText("21 D");
        ui->horizontalSlider_2->setValue(377);
        ui->label_8->setText("37.8 C");
        ui->horizontalSlider_3->setValue(600);
        ui->label_9->setText("60.0 %");

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
        ui->horizontalSlider_2->setValue(377);
        ui->label_8->setText("37.8 C");
        ui->horizontalSlider_3->setValue(550);
        ui->label_9->setText("55.0 %");

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

        ui->horizontalSlider->setValue(28);
        ui->label_7->setText("28 D");
        ui->horizontalSlider_2->setValue(372);
        ui->label_8->setText("37.2 C");
        ui->horizontalSlider_3->setValue(600);
        ui->label_9->setText("60.0 %");

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

    ui->label_17->setPixmap(QPixmap(":/slike/switch-on.png"));

    ui->pushButton_3->setDisabled(true);

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

    ui->pushButton_3->setDisabled(false);

    ui->checkBox->setDisabled(false);
    ui->checkBox_2->setDisabled(false);
    ui->checkBox_3->setDisabled(false);
    ui->checkBox_4->setDisabled(false);

    ui->label_17->setPixmap(QPixmap(":/slike/switch-off.png"));

    if(ui->checkBox_4->isChecked() == true){
    ui->horizontalSlider->setDisabled(false);
    ui->horizontalSlider_2->setDisabled(false);
    ui->horizontalSlider_3->setDisabled(false);
    }

}

void Dialog::on_pushButton_3_clicked()
{
    if(start_sig == 0){
    QMessageBox::information(this, "SUGGESTED CUSTOM", "Suggested custom settings...\n"
                                                       "Turkey      Days-28 Temp-37.2 Humi-55\n"
                                                       "MuscovyDuck Days-36 Temp-37.8 Humi-55\n"
                                                       "GuineaFowl  Days-28 Temp-37.8 Humi-55\n"
                                                       "Pheasant    Days-23 Temp-37.8 Humi-60\n"
                                                       "Peafowl     Days-28 Temp-37.2 Humi-55\n"
                                                       "Chukar      Days-23 Temp-37.8 Humi-45\n"
                                                       "Grouse      Days-25 Temp-37.8 Humi-55\n"
                                                       "Pigeon      Days-17 Temp-37.8 Humi-55\n");

    }
}

Dialog::~Dialog()
{
    delete ui;
}
