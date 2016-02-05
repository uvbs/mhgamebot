#-------------------------------------------------
#
# Project created by QtCreator 2016-01-27T16:31:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mhbot
TEMPLATE = app

CONFIG += c++11

LIBS += shlwapi.lib
LIBS += user32.lib
LIBS += gdi32.lib


LIBS += C:\library\lua\lua53.lib


INCLUDEPATH += C:\library\lua\include
INCLUDEPATH += C:\library\opencv\build\include
INCLUDEPATH += C:\library\boost_1_60_0


LIBS += C:\library\opencv\build\x64\vc12\lib\opencv_ts300d.lib
LIBS += C:\library\opencv\build\x64\vc12\lib\opencv_world300d.lib

SOURCES += main.cpp\
        mainwindow.cpp \
    ../mh_config.cpp \
    ../mh_gamescript.cpp \
    ../mh_mousekeyboard.cpp \
    ../scriptapp.cpp \
    playerinfo.cpp \
    dlgconfig.cpp

HEADERS  += mainwindow.h \
    ../mh_config.h \
    ../mh_define.h \
    ../mh_gamescript.h \
    ../mh_mousekeyboard.h \
    ../scriptapp.h \
    playerinfo.h \
    dlgconfig.h

FORMS    += mainwindow.ui \
    playerinfo.ui \
    dlgconfig.ui
