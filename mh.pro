

QT += core gui widgets

CONFIG += c++11
TEMPLATE = app
TARGET = mh

RC_ICONS = resource/ghost.ico


LIBS += gdi32.lib
LIBS += user32.lib
LIBS += shell32.lib
LIBS += shlwapi.lib


INCLUDEPATH += D:/mylibrary/lua-5.3.2/src
INCLUDEPATH += D:/mylibrary/boost_1_59_0
INCLUDEPATH += D:/mylibrary/opencv/build/include
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_filesystem-vc140-mt-1_59.lib
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_system-vc140-mt-1_59.lib
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_date_time-vc140-mt-1_59.lib
LIBS += D:/mylibrary/boost_1_59_0/lib64-msvc-14.0/libboost_regex-vc140-mt-1_59.lib
LIBS += D:/mylibrary/lua-5.3.2/src/build-Release/release/lualib.lib
LIBS += D:/mylibrary/opencv/build/x64/vc14/lib/opencv_world310.lib

SOURCES += main.cpp \
    gamescript.cpp \
    config.cpp \
    mainwindow.cpp \
    selecttask.cpp \
    helperfun.cpp \
    scriptmanager.cpp \
    optiondlg.cpp \
    mhnetwork.cpp \
    ../control/rwhandler.cpp \
    ../control/protocol.cpp

HEADERS += \
    config.h \
    define.h \
    gamescript.h \
    mainwindow.h \
    selecttask.h \
    helperfun.h \
    scriptmanager.h \
    optiondlg.h \
    mhnetwork.h \
    ../control/rwhandler.h \
    ../control/protocol.h

#拷贝到bin目录
QMAKE_POST_LINK += \
copy .\release\mh.exe ..\bin\mh.exe && \
type ..\bin\qt5core.dll >> ..\bin\mh.exe

RESOURCES += \
    icon.qrc

FORMS += \
    mainwindow.ui \
    selecttask.ui \
    optiondlg.ui

DISTFILES += \
    model.qmodel
