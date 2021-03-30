QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

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
    devicewindow.cpp \
    informationdialog.cpp \
    itusb2device.cpp \
    libusb-extra.c \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    aboutdialog.h \
    devicewindow.h \
    informationdialog.h \
    itusb2device.h \
    libusb-extra.h \
    mainwindow.h

FORMS += \
    aboutdialog.ui \
    devicewindow.ui \
    informationdialog.ui \
    mainwindow.ui

LIBS += -lusb-1.0

RESOURCES += \
    resources.qrc
