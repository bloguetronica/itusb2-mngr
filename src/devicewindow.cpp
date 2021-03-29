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
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include "aboutdialog.h"
#include "devicewindow.h"
#include "ui_devicewindow.h"

DeviceWindow::DeviceWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DeviceWindow)
{
    ui->setupUi(this);
    labelTime_ = new QLabel(tr("Time: 0s"));
    this->statusBar()->addPermanentWidget(labelTime_);
    labelLog_ = new QLabel(tr("Log: 0"));
    this->statusBar()->addPermanentWidget(labelLog_);
    labelMeas_ = new QLabel(tr("Meas.: 0"));
    this->statusBar()->addPermanentWidget(labelMeas_);
    ui->pushButtonClear->setFocus();
    filepath_ = QDir::homePath();
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

void DeviceWindow::on_actionAbout_triggered()
{
    AboutDialog about;
    about.exec();
}

void DeviceWindow::on_pushButtonAttach_clicked()
{
    int errcnt = 0;
    QString errstr;
    if (device_.getGPIO1(errcnt, errstr) != device_.getGPIO2(errcnt, errstr)) {  // If GPIO.1 and GPIO.2 pins do not match in value, indicating an unusual state
        device_.setGPIO1(true, errcnt, errstr);  // Set GPIO.1 to a logical high to switch off VBUS first
        device_.setGPIO2(true, errcnt, errstr);  // Set GPIO.2 to a logical high to disconnect the data lines
        QThread::msleep(100);  // Wait 100ms to allow for device shutdown
    }
    if (device_.getGPIO1(errcnt, errstr) && device_.getGPIO2(errcnt, errstr)) {  // If GPIO.1 and GPIO.2 are both set to a logical high
        device_.setGPIO1(false, errcnt, errstr);  // Set GPIO.1 to a logical low to switch VBUS on
        QThread::msleep(100);  // Wait 100ms in order to emulate a manual attachment of the device
        device_.setGPIO2(false, errcnt, errstr);  // Set GPIO.2 to a logical low to connect the data lines
        QThread::msleep(100);  // Wait 100ms so that device enumeration process can, at least, start (this is not enough to guarantee enumeration, though)
    }
    if (opCheck(tr("attach-op"), errcnt, errstr)) {  // If error check passes  (the string "attach-op" should be translated to "Attach")
        // Needed for improved responsiveness
        ui->checkBoxPower->setChecked(true);
        ui->checkBoxData->setChecked(true);
        // Note that update() will always confirm the true status of the lines
    }
}

void DeviceWindow::on_pushButtonDetach_clicked()
{
    int errcnt = 0;
    QString errstr;
    if (!device_.getGPIO1(errcnt, errstr) || !device_.getGPIO2(errcnt, errstr)) {  // If GPIO.1 or GPIO.2, or both, are set to a logical low
        device_.setGPIO2(true, errcnt, errstr);  // Set GPIO.2 to a logical high so that the data lines are disconnected
        QThread::msleep(100);  // Wait 100ms in order to emulate a manual detachment of the device
        device_.setGPIO1(true, errcnt, errstr);  // Set GPIO.1 to a logical high to switch VBUS off
        QThread::msleep(100);  // Wait 100ms to allow for device shutdown
    }
    if (opCheck(tr("detach-op"), errcnt, errstr)) {  // If error check passes (the string "detach-op" should be translated to "Detach")
        // Needed for improved responsiveness
        ui->checkBoxPower->setChecked(false);
        ui->checkBoxData->setChecked(false);
        // Note that, as before, update() will always confirm the true status of the lines
    }
}

// Partially disables device window
void::DeviceWindow::disableView()
{
    ui->actionInformation->setEnabled(false);
    ui->menuOptions->setEnabled(false);
    ui->centralWidget->setEnabled(false);
    ui->lcdNumberCurrent->setStyleSheet("");  // Just to ensure that any previously applied styles are removed
    ui->labelOCFault->setStyleSheet("");
    ui->statusBar->setEnabled(false);
}

// Checks for errors and validates (or ultimately halts) device operations
bool DeviceWindow::opCheck(const QString &op, int errcnt, QString errstr)
{
    bool retval = true;
    if (errcnt > 0) {
        errstr.chop(1);  // Remove the last character, which is always a newline
        QMessageBox::critical(this, tr("Error"), tr("%1 operation returned the following error(s):\n– %2", "", errcnt).arg(op, errstr.replace("\n", "\n– ")));
        erracc_ += errcnt;
        if (erracc_ > 10) {  // If the session accumulated more than 10 errors
            timer_->stop();  // This prevents a segmentation fault
            QMessageBox::critical(this, tr("Error"), tr("Detected too many errors. Device may not be properly connected.\n\nThe device window will be disabled."));
            disableView();  // Disable device window
            device_.reset(errcnt, errstr);  // Try to reset the device for sanity purposes, but don't check if it was successful
            device_.close();  // Ensure that the device is freed, even if the previous device reset is not effective (device_.reset() also frees the device interface, as an effect of re-enumeration)
            // It is essential that device_.close() is called, since some important checks rely on device_.isOpen() to retrieve a proper status
        }
        retval = false;  // Failed check
    }
    return retval;
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
