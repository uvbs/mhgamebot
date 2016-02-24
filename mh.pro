

TEMPLATE = app


CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += gdi32.lib
LIBS += user32.lib
LIBS += shell32.lib
LIBS += shlwapi.lib


INCLUDEPATH += D:/mylibrary/lua-5.3.2/src
INCLUDEPATH += D:/mylibrary/boost_1_59_0
INCLUDEPATH += D:/mylibrary/opencv/build/include
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_filesystem-vc140-mt-1_59.lib
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_system-vc140-mt-1_59.lib
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_system-vc140-mt-1_59.lib
LIBS += D:/mylibrary/lua-5.3.2/src/build-Release/release/lualib.lib


LIBS += D:\mylibrary\opencv\build\x64\vc14\lib\opencv_world310.lib

SOURCES += main.cpp \
    scriptapp.cpp \
    gamescript.cpp \
    config.cpp

HEADERS += \
    scriptapp.h \
    config.h \
    define.h \
    gamescript.h

#拷贝到bin目录
QMAKE_POST_LINK += "copy .\release\mh.exe ..\bin\mh.exe"
