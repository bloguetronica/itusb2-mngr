QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Added to provide backwards compatibility (C++11 support)
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    QMAKE_CXXFLAGS += -std=c++11
}

TARGET = itusb2-mngr
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    cp2130.cpp \
    datalog.cpp \
    devicewindow.cpp \
    informationdialog.cpp \
    itusb2device.cpp \
    libusb-extra.c \
    linkmodedetector.cpp \
    main.cpp \
    mainwindow.cpp \
    metrics.cpp

HEADERS += \
    aboutdialog.h \
    cp2130.h \
    datalog.h \
    datapoint.h \
    devicewindow.h \
    informationdialog.h \
    itusb2device.h \
    libusb-extra.h \
    linkmodedetector.h \
    mainwindow.h \
    metrics.h

FORMS += \
    aboutdialog.ui \
    devicewindow.ui \
    informationdialog.ui \
    mainwindow.ui

TRANSLATIONS += \
    translations/itusb2-mngr_en.ts \
    translations/itusb2-mngr_en_US.ts \
    translations/itusb2-mngr_pt.ts \
    translations/itusb2-mngr_pt_BR.ts \
    translations/itusb2-mngr_pt_PT.ts

LIBS += -lusb-1.0

RESOURCES += \
    resources.qrc

# Added installation option
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/bin
    icon.files += icons/itusb2-mngr.png
    icon.path = $$PREFIX/share/icons/hicolor/128x128/apps
    shortcut.files = misc/itusb2-mngr.desktop
    shortcut.path = $$PREFIX/share/applications
    INSTALLS += icon
    INSTALLS += shortcut
}

!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icons/itusb2-mngr.png \
    misc/itusb2-mngr.desktop
