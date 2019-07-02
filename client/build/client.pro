TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
QT += network

SOURCES += \
        ../src/main.cpp \
        ../src/tcpsocket.cpp

HEADERS += \
    ../src/tcpsocket.h
