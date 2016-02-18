

TEMPLATE = app


CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += libgdi32
LIBS += libuser32
LIBS += libshell32
LIBS += libshlwapi

LIBS += libboost_filesystem-mt
LIBS += libboost_system-mt

INCLUDEPATH += D:\msys32\usr\src\lua-5.3.2_fixCN\src
LIBS += D:\msys32\usr\src\lua-5.3.2_fixCN\src\liblua.a

LIBS += D:\msys32\mingw32\lib\libopencv_core.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_imgproc.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_imgcodecs.dll.a
LIBS += D:\msys32\mingw32\lib\libopencv_highgui.dll.a

SOURCES += main.cpp \
    scriptapp.cpp \
    gamescript.cpp \
    config.cpp

HEADERS += \
    scriptapp.h \
    config.h \
    define.h \
    gamescript.h

