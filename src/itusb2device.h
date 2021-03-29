/* ITUSB2 device class for Qt - Version 1.0 for Debian Linux
   Copyright (c) 2021 Samuel Lourenço

   This library is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library.  If not, see <https://www.gnu.org/licenses/>.


   Please feel free to contact me via e-mail: samuel.fmlourenco@gmail.com */


#ifndef ITUSB2DEVICE_H
#define ITUSB2DEVICE_H

// Includes
#include <QStringList>  // Also includes QString
#include <libusb-1.0/libusb.h>

class ITUSB2Device
{
private:
    libusb_context *context_;
    libusb_device_handle *handle_;
    bool deviceOpen_, kernelAttached_;

public:
    //Class definitions
    static const uint8_t CFRQ1500K = 0x03;  // Value corresponding to a clock frequency of 1.5MHz, applicable to SPIMode/configureSPIMode()
    static const bool CPOL0 = false;    // Boolean corresponding to CPOL = 0, applicable to SPIMode/configureSPIMode()
    static const bool CPHA0 = false;    // Boolean corresponding to CPHA = 0, applicable to SPIMode/configureSPIMode()
    static const bool CSMODEPP = true;  // Boolean corresponding to chip select push-pull mode, applicable to SPIMode/configureSPIMode()

    struct SPIMode {
        bool csmode;
        uint8_t cfrq;
        bool cpol;
        bool cpha;
    };

    ITUSB2Device();
    ~ITUSB2Device();

    void configureSPIMode(uint8_t channel, const SPIMode &mode, int &errcnt, QString &errstr) const;
    void disableCS(uint8_t channel, int &errcnt, QString &errstr) const;
    void disableSPIDelays(uint8_t channel, int &errcnt, QString &errstr) const;
    uint16_t getCurrent(int &errcnt, QString &errstr) const;
    bool getGPIO1(int &errcnt, QString &errstr) const;
    bool getGPIO2(int &errcnt, QString &errstr) const;
    void reset(int &errcnt, QString &errstr) const;
    void selectCS(uint8_t channel, int &errcnt, QString &errstr) const;
    void setGPIO1(bool value, int &errcnt, QString &errstr) const;
    void setGPIO2(bool value, int &errcnt, QString &errstr) const;

    void close();
    int open(const QString &serial);

    static QStringList listDevices(int &errcnt, QString &errstr);
};

#endif // ITUSB2DEVICE_H