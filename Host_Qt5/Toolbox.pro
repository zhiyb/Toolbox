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
    device.cpp \
    controller.cpp \
    structures.cpp \
    analog.cpp \
    analogwaveform.cpp \
    scalevalue.cpp \
    analogchannelctrl.cpp \
    conv.cpp \
    timebasectrl.cpp

HEADERS  += mainwindow.h \
    connection.h \
    device.h \
    ../Inc/instructions.h \
    structures.h \
    controller.h \
    analog.h \
    analogwaveform.h \
    scalevalue.h \
    analogchannelctrl.h \
    conv.h \
    timebasectrl.h

TRANSLATIONS = toolbox_en.ts

DISTFILES += \
    fragment.fsh \
    gridvertex.vsh \
    ytvertex.vsh \
    vertex.vsh