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
#include <QTextStream>
#include <QThread>
#include "aboutdialog.h"
#include "informationdialog.h"
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

// Allows user to save pending data points
void DeviceWindow::closeEvent(QCloseEvent *event)
{
    if (log_.hasNewData()) {  // If there are pending data points
        int qmret = QMessageBox::warning(this, tr("Unsaved Data Points"), tr("There are data points in memory that were not saved yet.\n\nDo you wish to save them?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (qmret == QMessageBox::Save) {  // If user clicked "Save"
            int result;
            do {
                result = saveDataPrompt();  // Prompt user where to save, and save if successful (i.e., returns zero)
            } while (result == 2);  // While errors occur (e.g., the user selected, or reselected a location that is write-protected)
            if (result != 0) {
                event->ignore();  // The user canceled the save, so, cancel window closing event
            }
        } else if (qmret == QMessageBox::Cancel) {  // If user clicked "Cancel"
            event->ignore();  // Cancel window closing event
        }  // If the user clicks "Discard", it is implied that the event will proceed and the window will be closed!
    }
}

void DeviceWindow::on_actionAbout_triggered()
{
    AboutDialog about;
    about.exec();
}

void DeviceWindow::on_actionInformation_triggered()
{
    int errcnt = 0;
    QString errstr;
    InformationDialog info;
    info.setManufacturerLabelText(device_.getManufacturer(errcnt, errstr));
    info.setProductLabelText(device_.getProduct(errcnt, errstr));
    info.setSerialLabelText(device_.getSerial(errcnt, errstr));  // It is important to read the serial number from the OTP ROM, instead of just passing the value of serialstr_
    info.setRevisionLabelText(device_.getMajorRelease(errcnt, errstr), device_.getMinorRelease(errcnt, errstr));
    info.setMaxPowerLabelText(device_.getMaxPower(errcnt, errstr));
    if (opCheck(tr("device-information-retrieval-op"), errcnt, errstr)) {  // If error check passes (the string "device-information-retrieval-op" should be translated to "Device information retrieval")
        info.exec();
    }
}

void DeviceWindow::on_actionRate50_triggered()
{
    timer_->setInterval(50);
    ui->actionRate50->setChecked(true);
    ui->actionRate100->setChecked(false);
    ui->actionRate200->setChecked(false);
    ui->actionRate300->setChecked(false);
    ui->actionRate500->setChecked(false);
}

void DeviceWindow::on_actionRate100_triggered()
{
    timer_->setInterval(100);
    ui->actionRate100->setChecked(true);
    ui->actionRate50->setChecked(false);
    ui->actionRate200->setChecked(false);
    ui->actionRate300->setChecked(false);
    ui->actionRate500->setChecked(false);
}

void DeviceWindow::on_actionRate200_triggered()
{
    timer_->setInterval(200);
    ui->actionRate200->setChecked(true);
    ui->actionRate50->setChecked(false);
    ui->actionRate100->setChecked(false);
    ui->actionRate300->setChecked(false);
    ui->actionRate500->setChecked(false);
}

void DeviceWindow::on_actionRate300_triggered()
{
    timer_->setInterval(300);
    ui->actionRate300->setChecked(true);
    ui->actionRate50->setChecked(false);
    ui->actionRate100->setChecked(false);
    ui->actionRate200->setChecked(false);
    ui->actionRate500->setChecked(false);
}

void DeviceWindow::on_actionRate500_triggered()
{
    timer_->setInterval(500);
    ui->actionRate500->setChecked(true);
    ui->actionRate50->setChecked(false);
    ui->actionRate100->setChecked(false);
    ui->actionRate200->setChecked(false);
    ui->actionRate300->setChecked(false);
}

void DeviceWindow::on_actionResetTime_triggered()
{
    if (log_.isEmpty()) {
        resetTimeCount();  // Reset time count
    } else {
        int qmret = QMessageBox::question(this, tr("Reset Time Count?"), tr("This action, besides resetting the elapsed time count, will also delete any previously acquired data points.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (qmret == QMessageBox::Yes) {  // If user clicked "Yes"
            deleteData();  // Delete acquired data points
            if (device_.isOpen()) {  // This condition is essential so the timer won't be restarted if, in the meantime, the device gets disconnected - In effect this prevents a segmentation fault!
                resetTimeCount();  // Reset time count - This will restart the timer too!
            }
        }
    }
}

void DeviceWindow::on_actionSaveData_triggered()
{
    saveDataPrompt();  // Unlike in closeEvent(), it is not required to obtain the result, since there will be no retries in any case (user can retry later)
}

void DeviceWindow::on_checkBoxData_clicked()
{
    int errcnt = 0;
    QString errstr;
    device_.setGPIO2(!ui->checkBoxData->isChecked(), errcnt, errstr);
    opCheck(tr("data-switch-op"), errcnt, errstr);  // The string "data-switch-op" should be translated to "Data switch"
}

void DeviceWindow::on_checkBoxPower_clicked()
{
    int errcnt = 0;
    QString errstr;
    device_.setGPIO1(!ui->checkBoxPower->isChecked(), errcnt, errstr);
    opCheck(tr("power-switch-op"), errcnt, errstr);  // The string "power-switch-op" should be translated to "Power switch"
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

// Deletes any acquired data points
void DeviceWindow::deleteData()
{
    log_.clear();
    setLogActionsEnabled(false);  // And also disable logging related actions
    labelLog_->setText(tr("Log: 0"));
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

// Resets (and restarts) the time count
void DeviceWindow::resetTimeCount()
{
    timer_->start();  // Restart the timer using user settings
    time_.start();  // Restart counting the elapsed time from zero
    labelTime_->setText(tr("Time: 0s"));
}

// Saves logged data with prompt to the user
int DeviceWindow::saveDataPrompt()
{
    int retval = 0;
    QString csvstr = log_.toCSVString();  // Compiles acquired data points to CSV formatted string (note that implicit sharing is implemented in the QString class which, in this case, avoids unnecessary copying and minimizes RAM usage)
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Logged Data to File"), filepath_, tr("CSV files (*.csv);;All files (*)"));
    if (filename.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        retval = 1;
    } else {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not write to %1.\n\nPlease verify that you have write access to this file.").arg(QDir::toNativeSeparators(filename)));
            retval = 2;
        } else {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << csvstr;
            file.close();
            filepath_ = filename;
            log_.noNewData();  // All data points were saved
        }
    }
    return retval;
}

// Enables or disables logging related actions
void DeviceWindow::setLogActionsEnabled(bool value)
{
    ui->actionSaveData->setEnabled(value);
    ui->actionDeleteData->setEnabled(value);
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
