QT -= core
QT -= gui



TARGET = mh
CONFIG += console
CONFIG -= app_bundle


LIBS += libgdi32
LIBS += libuser32
LIBS += libshell32


LIBS += D:\msys32\mingw32\lib\libopencv_imgproc.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_core.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_highgui.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_imgcodecs.dll.a

TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp \
    mh_function.cpp \
    mh_gamescript.cpp \
    mh_writebmp.cpp \
    mh_mousekeyboard.cpp \
    scriptapp.cpp

HEADERS += \
    mh_define.h \
    mh_function.h \
    mh_gamescript.h \
    mh_writebmp.h \
    mh_mousekeyboard.h \
    scriptapp.h

