QT += core gui widgets printsupport

CONFIG += c++11

TARGET = SuperCalculator
CONFIG -= app_bundle
DESTDIR = $$PWD/bin

TEMPLATE = app

SOURCES += main.cpp \
    qcustomplot.cpp \
    window.cpp \
    calculator.cpp

HEADERS += \
    qcustomplot.h \
    window.h \
    calculator.h

FORMS += \
    window.ui

RC_ICONS = Icon.ico
