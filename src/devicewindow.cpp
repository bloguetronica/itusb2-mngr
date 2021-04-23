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
#include "datapoint.h"
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

void DeviceWindow::on_actionDeleteData_triggered()
{
    int qmret = QMessageBox::question(this, tr("Delete Logged Data?"), tr("This action will delete any data points acquired until now.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (qmret == QMessageBox::Yes) {  // If user clicked "Yes"
        deleteData();  // Delete acquired data points
    }
}

void DeviceWindow::on_actionInformation_triggered()
{
    int errcnt = 0;
    QString errstr;
    InformationDialog info;
    info.setManufacturerLabelText(device_.getManufacturerDesc(errcnt, errstr));
    info.setProductLabelText(device_.getProductDesc(errcnt, errstr));
    info.setSerialLabelText(device_.getSerialDesc(errcnt, errstr));  // It is important to read the serial number from the OTP ROM, instead of just passing the value of serialstr_
    CP2130::USBConfig config = device_.getUSBConfig(errcnt, errstr);
    info.setRevisionLabelText(config.majrel, config.minrel);
    info.setMaxPowerLabelText(config.maxpow);
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
            if (device_.isOpen()) {  // This condition is essential so the timer won't be restarted if, in the meantime, the device gets disconnected - In effect this prevents further errors!
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
    device_.switchUSBData(ui->checkBoxData->isChecked(), errcnt, errstr);
    opCheck(tr("data-switch-op"), errcnt, errstr);  // The string "data-switch-op" should be translated to "Data switch"
}

void DeviceWindow::on_checkBoxPower_clicked()
{
    int errcnt = 0;
    QString errstr;
    device_.switchUSBPower(ui->checkBoxPower->isChecked(), errcnt, errstr);
    opCheck(tr("power-switch-op"), errcnt, errstr);  // The string "power-switch-op" should be translated to "Power switch"
}

void DeviceWindow::on_pushButtonAttach_clicked()
{
    int errcnt = 0;
    QString errstr;
    device_.attach(errcnt, errstr);
    if (opCheck(tr("attach-op"), errcnt, errstr)) {  // If error check passes  (the string "attach-op" should be translated to "Attach")
        // Needed for improved responsiveness
        ui->checkBoxPower->setChecked(true);
        ui->checkBoxData->setChecked(true);
        // Note that update() will always confirm the true status of the lines
    }
}

void DeviceWindow::on_pushButtonClear_clicked()
{
    clearMetrics();
    ui->labelOCFault->clear();  // Clear "OC fault!" warning, if applicable
}

void DeviceWindow::on_pushButtonDetach_clicked()
{
    int errcnt = 0;
    QString errstr;
    device_.detach(errcnt, errstr);
    if (opCheck(tr("detach-op"), errcnt, errstr)) {  // If error check passes (the string "detach-op" should be translated to "Detach")
        // Needed for improved responsiveness
        ui->checkBoxPower->setChecked(false);
        ui->checkBoxData->setChecked(false);
        // Note that, as before, update() will always confirm the true status of the lines
    }
}

void DeviceWindow::on_pushButtonReset_clicked()
{
    if (log_.isEmpty()) {
        resetDevice();
    } else {
        int qmret = QMessageBox::question(this, tr("Reset Device?"), tr("This action, besides resetting the device, will also delete any previously acquired data points.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (qmret == QMessageBox::Yes) {  // If user clicked "Yes"
            if (device_.isOpen()) {  // This condition is required to prevent multiple errors if, in the meantime, the device gets disconnected
                resetDevice();
            } else {
                deleteData();  // Delete all data points, as an alternative to resetDevice()
            }
        }
    }
}

// This is the main update routine
void DeviceWindow::update()
{
    int errcnt = 0;
    QString errstr;
    float current = device_.getCurrent(errcnt, errstr);
    bool up = device_.getUSBPowerStatus(errcnt, errstr);
    bool ud = device_.getUSBDataStatus(errcnt, errstr);
    bool oc = device_.getOverCurrentStatus(errcnt, errstr);
    bool cd = device_.getConnectionStatus(errcnt, errstr);
    bool hs = device_.getSpeedStatus(errcnt, errstr);
    if (opCheck(tr("update-op"), errcnt, errstr)) {  // Update values if no errors occur (the string "update-op" should be translated to "Update")
        if (ui->actionLogData->isChecked()) {
            logDataPoint(current, up, ud, cd, hs, oc);
        }
        metrics_.update(current);  // Update actual, average, minimum and maximum values
        updateView(up, ud, cd, hs, oc);
    }
}

// Wrapper function that also resets the "Meas" counter in the status bar
void DeviceWindow::clearMetrics()
{
    metrics_.clear();
    labelMeas_->setText(tr("Meas.: 0"));
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

// Logs an acquired data point
void DeviceWindow::logDataPoint(float current, bool up, bool ud, bool cd, bool hs, bool oc)
{
    DataPoint datapt;
    datapt.time = time_.elapsed() / 1000.0;  // Log time
    datapt.curr = current;  // Current
    datapt.up = up;  // USB power status
    datapt.ud = ud;  // USB data status
    datapt.cd = cd;  // Device detection status
    datapt.hs = hs;  // Link mode
    datapt.oc = oc;  // OC status
    log_.append(datapt);  // Add data point
    setLogActionsEnabled(true);  // And also enable logging related actions
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
            timer_->stop();  // This prevents further errors
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

// Resets the device
void DeviceWindow::resetDevice()
{
    timer_->stop();  // Stop the update timer momentarily, in order to avoid recurrent errors if the device gets disconnected during a reset, or other unexpected behavior
    clearMetrics();
    deleteData();  // All data points are deleted before a reset is issued to the device, so that the window may close if there is a problem - Effectively, this will bypass the inner workings of closeEvent()!
    ui->labelOCFault->clear();  // Clear "OC fault!" warning, if applicable
    int errcnt = 0;
    QString errstr;
    device_.reset(errcnt, errstr);
    opCheck(tr("reset-op"), errcnt, errstr);  // The string "reset-op" should be translated to "Reset"
    if (device_.isOpen()) {  // If opCheck() passes, thus, not closing the device
        device_.close();  // Important! - This should be done always, even if the previous reset operation shows an error, because an error doesn't mean that a device reset was not effected
        int err;
        for (int i = 0; i < 10; ++i) {  // Verify enumeration 10 times
            QThread::msleep(500);  // Wait 500ms each time
            err = device_.open(serialstr_);
            if (err != 2) {  // Retry only if the device was not found yet (as it may take some time to enumerate)
                break;
            }
        }
        if (err == 1) {  // Failed to initialize libusb
            QMessageBox::critical(this, tr("Critical Error"), tr("Could not reinitialize libusb.\n\nThis is a critical error and execution will be aborted."));
            exit(EXIT_FAILURE);  // This error is critical because libusb failed to initialize
        } else if (err == 2) {  // Failed to find device
            QMessageBox::critical(this, tr("Error"), tr("Device disconnected."));
            this->close();  // Close window
        } else if (err == 3) {  // Failed to claim interface
            QMessageBox::critical(this, tr("Error"), tr("Device ceased to be available.\n\nPlease verify that the device is not in use by another application."));
            this->close();  // Close window
        } else {
            setupDevice();  // Necessary in order to get correct readings after a device reset
            erracc_ = 0;  // Zero the error count accumulator, since a new session gets started once the reset is done
            resetTimeCount();  // Reset time count
        }
    }
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
    device_.setup(errcnt, errstr);
    if (errcnt > 0) {
        errstr.chop(1);  // Remove the last character, which is always a newline
        QMessageBox::critical(this, tr("Error"), tr("Setup operation returned the following error(s):\n– %1\n\nPlease try accessing the device again.", "", errcnt).arg(errstr.replace("\n", "\n– ")));
        device_.reset(errcnt, errstr);  // Try to reset the device for sanity purposes, but don't check if it was successful
        this->deleteLater();  // In a context where the window is already visible, it has the same effect as this->close()
    }
}

// Updates the view, including status and displayed value
void DeviceWindow::updateView(bool up, bool ud, bool cd, bool hs, bool oc)
{
    if (up && ud) {
        ui->labelStatus->setText(tr("Connection enabled"));
    } else {
        ui->labelStatus->setText(tr("Connection disabled"));
    }
    int linkMode = lmdetector_.detectedLinkMode(cd, hs);
    if (linkMode == 1) {
        ui->labelMode->setText(tr("Device detected"));
    } else if (linkMode == 2) {
        ui->labelMode->setText(tr("Full/low speed device"));
    } else if (linkMode == 3) {
        ui->labelMode->setText(tr("High speed device"));
    } else if (linkMode == 4) {
        ui->labelMode->setText(tr("Suspend mode"));
    } else {
        ui->labelMode->setText(tr("No device"));
    }
    float value;
    if (ui->radioButtonDisplayAvg->isChecked()) {
        value = metrics_.average();  // Pass the average to be displayed
    } else if (ui->radioButtonDisplayMin->isChecked()) {
        value = metrics_.minimum();  // Pass the minimum value to be displayed
    } else if (ui->radioButtonDisplayMax->isChecked()) {
        value = metrics_.maximum();  // Pass the maximum value to be displayed
    } else {
        value = metrics_.last();  // Pass the last input value to be displayed
    }
    if (value > 500) {
        ui->lcdNumberCurrent->setStyleSheet("color: darkred;");  // A dark red color indicates that the measured current exceeds the 500mA limit established by the USB 2.0 specification
    } else {
        ui->lcdNumberCurrent->setStyleSheet("");
    }
    if (value < 1000) {
        ui->lcdNumberCurrent->display(QString::number(value, 'f', 1));  // Display the value passed previously
    } else {
        ui->lcdNumberCurrent->display(QString("OL"));  // Display "OL"
    }
    if (oc) {
        ui->labelOCFault->setStyleSheet("color: red;");
        ui->labelOCFault->setText(tr("OC fault!"));
    } else {
        ui->labelOCFault->setStyleSheet("");  // Empty (default) style sheet
        if (!ui->actionPersistent->isChecked()) {
            ui->labelOCFault->clear();
        }
    }
    ui->checkBoxPower->setChecked(up);
    ui->checkBoxData->setChecked(ud);
    labelTime_->setText(tr("Time: %1s").arg(time_.elapsed() / 1000.0, 0, 'f', 1));
    labelLog_->setText(tr("Log: %1").arg(log_.size()));
    labelMeas_->setText(tr("Meas.: %1").arg(metrics_.numberOfMeasurements()));
}
