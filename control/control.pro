#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T17:52:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = control
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    scripttablemodel.cpp \
    scriptitem.cpp

HEADERS  += mainwindow.h \
    scripttablemodel.h \
    scriptitem.h

FORMS    += mainwindow.ui
