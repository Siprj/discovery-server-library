#-------------------------------------------------
#
# Project created by QtCreator 2013-09-11T20:36:21
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = DiscoveryServer
TEMPLATE = lib
CONFIG += staticlib

SOURCES += DiscoveryServer.cpp

HEADERS += DiscoveryServer.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
