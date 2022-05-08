/* ITUSB2 Manager - Version 1.4 for Debian Linux
   Copyright (c) 2021-2022 Samuel Lourenço

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


#ifndef LINKMODEDETECTOR_H
#define LINKMODEDETECTOR_H

// Includes
#include <QElapsedTimer>

class LinkModeDetector
{
private:
    bool cd_, hsCapable_;
    QElapsedTimer time_;  // QTime::start() and QTime::elapsed() are now obsolete (version 1.1)

public:
    // Class definitions
    static const int NO_DEVICE = 0;   // Returned by detectedLinkMode() if no device is detected
    static const int DETECTED = 1;    // Returned by detectedLinkMode() if a device is detected, but the link mode is not asserted
    static const int FULL_SPEED = 2;  // Returned by detectedLinkMode() if full/low speed is asserted
    static const int HIGH_SPEED = 3;  // Returned by detectedLinkMode() if high speed is asserted
    static const int SUSPEND = 4;     // Returned by detectedLinkMode() if device is in suspend mode (only valid for high speed devices)

    LinkModeDetector();

    int detectedLinkMode(bool cd, bool hs);
};

#endif  // LINKMODEDETECTOR_H
