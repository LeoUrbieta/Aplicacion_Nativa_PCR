#-------------------------------------------------
#
# Project created by QtCreator 2016-09-08T11:00:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport serialport

TARGET = QTPCR
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui

DISTFILES +=

RESOURCES += \
    resources.qrc
