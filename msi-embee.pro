#-------------------------------------------------
#
# Project created by QtCreator 2018-08-21T10:28:21
#
#-------------------------------------------------
QT       += core
QT       -= gui

CONFIG += c++11

TARGET = msi-embee
TEMPLATE = lib

DEFINES += MSIEMBEE_LIBRARY
DEFINES += METERPLUGIN_FILETREE

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        msiembee.cpp\
    msiembeehelper.cpp \
    crc8polyb5.cpp \
    shared/meterpluginhelper.cpp


HEADERS += \
        msiembee.h \
    shared/meterplugin.h \
    shared/myucmmeterstypes.h \
    msiembeehelper.h \
    shared/moji_defy.h \
    shared/definedpollcodes.h \
    crc8polyb5.h \
    shared/meterpluginhelper.h

EXAMPLE_FILES = zbyralko.json

linux-beagleboard-g++:{
    target.path = /opt/matilda/plugin
    INSTALLS += target
}
