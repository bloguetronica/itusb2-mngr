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


// Includes
#include <QMessageBox>
#include <QThread>
#include "devicewindow.h"
#include "ui_devicewindow.h"

DeviceWindow::DeviceWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DeviceWindow)
{
    ui->setupUi(this);
}

DeviceWindow::~DeviceWindow()
{
    delete ui;
}

// Opens the device and sets it up, while also preparing its window
void DeviceWindow::openDevice(const QString &serialstr)
{
    int err = device_.open(serialstr);
    if (err == 1) {  // Failed to initialize libusb
        QMessageBox::critical(this, tr("Critical Error"), tr("Could not initialize libusb.\n\nThis is a critical error and execution will be aborted."));
        exit(EXIT_FAILURE);  // This error is critical because libusb failed to initialize
    } else if (err == 2) {  // Failed to find device
        QMessageBox::critical(this, tr("Error"), tr("Could not find device."));
        this->deleteLater();  // Close window after the subsequent show() call
    } else if (err == 3) {  // Failed to claim interface
        QMessageBox::critical(this, tr("Error"), tr("Device is currently unavailable.\n\nPlease confirm that the device is not in use."));
        this->deleteLater();  // Close window after the subsequent show() call
    } else {
        serialstr_ = serialstr;  // Valid serial number
        setupDevice();  // Necessary in order to get correct readings
        this->setWindowTitle(tr("ITUSB2 USB Test Switch (S/N: %1)").arg(serialstr_));
        timer_ = new QTimer(this);  // Create a timer
        QObject::connect(timer_, SIGNAL(timeout()), this, SLOT(update()));
        timer_->start(200);
        time_.start();  // Start counting the elapsed time from this point
    }
}

// Prepares the device, performing basic configurations
void DeviceWindow::setupDevice()
{
    int errcnt = 0;
    QString errstr;
    ITUSB2Device::SPIMode mode;
    mode.csmode = ITUSB2Device::CSMODEPP;  // Chip select pin mode regarding channel 0 is push-pull
    mode.cfrq = ITUSB2Device::CFRQ1500K;  // SPI clock frequency set to 1.5MHz
    mode.cpol = ITUSB2Device::CPOL0;  // SPI clock polarity is active high (CPOL = 0)
    mode.cpha = ITUSB2Device::CPHA0;  // SPI data is valid on each rising edge (CPHA = 0)
    device_.configureSPIMode(0, mode, errcnt, errstr);  // Configure SPI mode for channel 0, using the above settings
    device_.disableSPIDelays(0, errcnt, errstr);  // Disable all SPI delays for channel 0
    device_.selectCS(0, errcnt, errstr);  // Enable the chip select corresponding to channel 0, and disable any others
    device_.getCurrent(errcnt, errstr);  // Discard this first reading - This also wakes up the LTC2312, if in nap or sleep mode!
    QThread::usleep(1100);  // Wait 1.1ms to ensure that the LTC2312 is awake, and also to prevent possible errors while disabling the chip select (workaround)
    device_.disableCS(0, errcnt, errstr);  // Disable the previously enabled chip select
    if (errcnt > 0) {
        errstr.chop(1);  // Remove the last character, which is always a newline
        QMessageBox::critical(this, tr("Error"), tr("Setup operation returned the following error(s):\n– %1\n\nPlease try accessing the device again.", "", errcnt).arg(errstr.replace("\n", "\n– ")));
        device_.reset(errcnt, errstr);  // Try to reset the device for sanity purposes, but don't check if it was successful
        this->deleteLater();  // In a context where the window is already visible, it has the same effect as this->close()
    }
}
