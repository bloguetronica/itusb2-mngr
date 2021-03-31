/* ITUSB2 Manager - Version 1.0 for Debian Linux
   Copyright (c) 2021 Samuel Lourenço

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation, either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <https://www.gnu.org/licenses/>.


   Please feel free to contact me via e-mail: samuel.fmlourenco@gmail.com */


#ifndef DEVICEWINDOW_H
#define DEVICEWINDOW_H

// Includes
#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "datalog.h"
#include "itusb2device.h"

namespace Ui {
class DeviceWindow;
}

class DeviceWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeviceWindow(QWidget *parent = nullptr);
    ~DeviceWindow();

    void openDevice(const QString &serialstr);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionAbout_triggered();
    void on_actionInformation_triggered();
    void on_actionRate50_triggered();
    void on_actionRate100_triggered();
    void on_actionRate200_triggered();
    void on_actionRate300_triggered();
    void on_actionRate500_triggered();
    void on_actionResetTime_triggered();
    void on_actionSaveData_triggered();
    void on_checkBoxData_clicked();
    void on_checkBoxPower_clicked();
    void on_pushButtonAttach_clicked();
    void on_pushButtonDetach_clicked();

private:
    Ui::DeviceWindow *ui;
    DataLog log_;
    ITUSB2Device device_;
    QLabel *labelLog_, *labelMeas_, *labelTime_;
    QString filepath_, serialstr_;
    QTime time_;
    QTimer *timer_;
    int erracc_ = 0;

    void deleteData();
    void disableView();
    bool opCheck(const QString &op, int errcnt, QString errstr);
    void resetTimeCount();
    int saveDataPrompt();
    void setLogActionsEnabled(bool value);
    void setupDevice();
};

#endif // DEVICEWINDOW_H
