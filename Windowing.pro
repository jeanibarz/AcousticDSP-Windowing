#-------------------------------------------------
#
# Project created by QtCreator 2014-05-10T16:11:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Windowing
TEMPLATE = app


SOURCES += main.cpp\
        windowing.cpp \
    qcustomplot-source/qcustomplot.cpp \
    plotting.cpp

HEADERS  += windowing.h \
    qcustomplot-source/qcustomplot.h \
    plotting.h

FORMS    += windowing.ui \
    plotting.ui

TRANSLATIONS = translations/windowing_fr.ts

RESOURCES += resources.qrc
