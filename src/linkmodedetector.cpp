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

// Returns the link mode determined from the DUT (device under test) connection and link speed status
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
            retval = HIGH_SPEED;
        } else if (hsCapable_) {
            retval = SUSPEND;
        } else if (time_.elapsed() < DET_TIMEOUT) {  // Note that this QElapsedTimer should be valid at this point, so there is no need to check it with isValid() here!
            retval = DETECTED;
        } else {
            retval = FULL_SPEED;
        }
    } else {
        hsCapable_ = false;
        retval = NO_DEVICE;
    }
    cd_ = cd;  // Update device detection status
    return retval;
}
