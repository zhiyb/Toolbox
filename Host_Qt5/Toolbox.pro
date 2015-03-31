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

INCLUDEPATH += ../inc . ./common ./analog

SOURCES += main.cpp\
        mainwindow.cpp \
    connection.cpp \
    device.cpp \
    controller.cpp \
    structures.cpp \
    analog/analog.cpp \
    analog/analogchannelctrl.cpp \
    analog/analogtriggerctrl.cpp \
    analog/analogwaveform.cpp \
    common/colourselection.cpp \
    common/scalevalue.cpp \
    analog/timebasectrl.cpp \
    common/conv.cpp \
    common/dial.cpp

HEADERS  += mainwindow.h \
    connection.h \
    device.h \
    ../Inc/instructions.h \
    structures.h \
    controller.h \
    analog/analog.h \
    analog/analogchannelctrl.h \
    analog/analogtriggerctrl.h \
    analog/analogwaveform.h \
    common/colourselection.h \
    common/scalevalue.h \
    analog/timebasectrl.h \
    common/conv.h \
    common/dial.h

TRANSLATIONS = toolbox_en.ts

DISTFILES += \
    shader/fragment.fsh \
    shader/gridvertex.vsh \
    shader/vertex.vsh \
    shader/ytvertex.vsh
