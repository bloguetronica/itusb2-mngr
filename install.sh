#!/bin/sh

echo Obtaining required packages...
apt-get -qq update
apt-get -qq install build-essential
apt-get -qq install libusb-1.0-0-dev
apt-get -qq install qt5-default
apt-get -qq install qtbase5-dev
echo Copying source code files...
mkdir -p /usr/local/src/itusb2-mngr/icons
mkdir -p /usr/local/src/itusb2-mngr/misc
mkdir -p /usr/local/src/itusb2-mngr/translations
cp -f src/aboutdialog.cpp /usr/local/src/itusb2-mngr/.
cp -f src/aboutdialog.h /usr/local/src/itusb2-mngr/.
cp -f src/aboutdialog.ui /usr/local/src/itusb2-mngr/.
cp -f src/cp2130.cpp /usr/local/src/itusb2-mngr/.
cp -f src/cp2130.h /usr/local/src/itusb2-mngr/.
cp -f src/datalog.cpp /usr/local/src/itusb2-mngr/.
cp -f src/datalog.h /usr/local/src/itusb2-mngr/.
cp -f src/datapoint.h /usr/local/src/itusb2-mngr/.
cp -f src/devicewindow.cpp /usr/local/src/itusb2-mngr/.
cp -f src/devicewindow.h /usr/local/src/itusb2-mngr/.
cp -f src/devicewindow.ui /usr/local/src/itusb2-mngr/.
cp -f src/GPL.txt /usr/local/src/itusb2-mngr/.
cp -f src/icons/active64.png /usr/local/src/itusb2-mngr/icons/.
cp -f src/icons/greyed64.png /usr/local/src/itusb2-mngr/icons/.
cp -f src/icons/icon.svg /usr/local/src/itusb2-mngr/icons/.
cp -f src/icons/itusb2-mngr.png /usr/local/src/itusb2-mngr/icons/.
cp -f src/icons/selected64.png /usr/local/src/itusb2-mngr/icons/.
cp -f src/informationdialog.cpp /usr/local/src/itusb2-mngr/.
cp -f src/informationdialog.h /usr/local/src/itusb2-mngr/.
cp -f src/informationdialog.ui /usr/local/src/itusb2-mngr/.
cp -f src/itusb2device.cpp /usr/local/src/itusb2-mngr/.
cp -f src/itusb2device.h /usr/local/src/itusb2-mngr/.
cp -f src/itusb2-mngr.pro /usr/local/src/itusb2-mngr/.
cp -f src/LGPL.txt /usr/local/src/itusb2-mngr/.
cp -f src/libusb-extra.c /usr/local/src/itusb2-mngr/.
cp -f src/libusb-extra.h /usr/local/src/itusb2-mngr/.
cp -f src/linkmodedetector.cpp /usr/local/src/itusb2-mngr/.
cp -f src/linkmodedetector.h /usr/local/src/itusb2-mngr/.
cp -f src/main.cpp /usr/local/src/itusb2-mngr/.
cp -f src/mainwindow.cpp /usr/local/src/itusb2-mngr/.
cp -f src/mainwindow.h /usr/local/src/itusb2-mngr/.
cp -f src/mainwindow.ui /usr/local/src/itusb2-mngr/.
cp -f src/metrics.cpp /usr/local/src/itusb2-mngr/.
cp -f src/metrics.h /usr/local/src/itusb2-mngr/.
cp -f src/misc/itusb2-mngr.desktop /usr/local/src/itusb2-mngr/misc/.
cp -f src/README.txt /usr/local/src/itusb2-mngr/.
cp -f src/resources.qrc /usr/local/src/itusb2-mngr/.
cp -f src/translations/itusb2-mngr_en.qm /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_en.ts /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_en_US.qm /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_en_US.ts /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_pt.qm /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_pt.ts /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_pt_PT.qm /usr/local/src/itusb2-mngr/translations/.
cp -f src/translations/itusb2-mngr_pt_PT.ts /usr/local/src/itusb2-mngr/translations/.
echo Building and installing application...
cd /usr/local/src/itusb2-mngr
qmake
make install clean
rm -f itusb2-mngr
echo Applying configurations...
cat > /etc/udev/rules.d/71-bgtn-itusb2.rules << EOF
SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="8cdf", MODE="0666"
SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="8cdf", MODE="0666"
EOF
service udev restart
echo Done!
