#-------------------------------------------------
#
# Project created by QtCreator 2014-12-07T18:16:59
#
#-------------------------------------------------

QT       += core gui
QT       += network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Toolbox
TEMPLATE = app

INCLUDEPATH += ../Inc

SOURCES += main.cpp\
        mainwindow.cpp \
    connection.cpp \
    device.cpp

HEADERS  += mainwindow.h \
    connection.h \
    device.h \
    ../Inc/instructions.h \
    structures.h
