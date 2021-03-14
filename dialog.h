#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

extern bool start_sig;
extern int d_target;
extern int t_target;
extern int h_target;
extern unsigned char ds3231_Store[7];
extern unsigned char init3231_Store[7];
extern bool rot;

extern int counter;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:

    void update_rotation();
    void update();

    void on_checkBox_4_clicked(bool checked);
    void on_checkBox_3_clicked(bool checked);
    void on_checkBox_2_clicked(bool checked);
    void on_checkBox_clicked(bool checked);

    void on_horizontalSlider_valueChanged(int value);
    void on_horizontalSlider_2_valueChanged(int value);
    void on_horizontalSlider_3_valueChanged(int value);

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();


private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
