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
#include "linkmodedetector.h"

// Definitions
const int DET_TIMEOUT = 1000;  // Assertion timeout in milliseconds

LinkModeDetector::LinkModeDetector() :
    cd_(false),
    hsCapable_(false),
    time_()
{
}

// Returns the link mode, represented by an int value as follows:
// - 0 if no device is detected
// - 1 if link mode is not asserted
// - 2 if full/low speed is asserted
// - 3 if high speed is asserted
// - 4 if device is in suspend mode (valid for high speed devices)
// Note that this function also updates the private members
int LinkModeDetector::detectedLinkMode(bool cd, bool hs)
{
    int retval;
    if (cd) {
        if (!cd_) {  // This, along with the previous condition, detects a rising edge on the UDCD signal
            time_.start();
        }
        if (hs) {
            hsCapable_ = true;
            retval = 3;  // High speed link mode  asserted
        } else if (hsCapable_) {
            retval = 4;  // Suspend mode asserted
        } else if (time_.elapsed() < DET_TIMEOUT) {  // Note that this QElapsedTimer should be valid at this point, so there is no need to check it with isValid() here!
            retval = 1;  // Link detected, but mode is not asserted
        } else {
            retval = 2;  // Full/low speed link mode asserted
        }
    } else {
        hsCapable_ = false;
        retval = 0;
    }
    cd_ = cd;  // Update device detection status
    return retval;
}
