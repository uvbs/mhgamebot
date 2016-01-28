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

LIBS += libshlwapi
LIBS += liblua


LIBS += D:\msys32\mingw32\lib\libopencv_imgproc.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_core.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_highgui.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_imgcodecs.dll.a


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
