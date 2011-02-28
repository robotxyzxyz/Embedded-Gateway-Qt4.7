#-------------------------------------------------
#
# Project created by QtCreator 2011-02-12T01:10:36
#
#-------------------------------------------------

QT       += core gui

TARGET = Gateway
TEMPLATE = app


SOURCES += main.cpp\
        Window.cpp \
    MainController.cpp \
    BaseNode.cpp \
    StatusView.cpp \
    FMacPacketParser.cpp \
    GsmModule.cpp \
    GsmModuleController.cpp \
    MainView.cpp \
    SettingsView.cpp \
    Preferences.cpp \
    AbstractSerialDevice.cpp \
    WeatherModule.cpp

HEADERS  += Window.h \
    MainController.h \
    BaseNode.h \
    StatusView.h \
    Packets.h \
    FMacPacketParser.h \
    PacketSlots.h \
    GsmModule.h \
    GsmModuleController.h \
    MainView.h \
    SettingsView.h \
    Preferences.h \
    GlobalErrorCodes.h \
    AbstractSerialDevice.h \
    WeatherModule.h

FORMS    +=
