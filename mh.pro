

QT += core gui widgets
QT += network


CONFIG += c++11
TEMPLATE = app
TARGET = mh

RC_ICONS = resource/ghost.ico


LIBS += gdi32.lib
LIBS += user32.lib
LIBS += shell32.lib
LIBS += shlwapi.lib
LIBS += advapi32.lib

INCLUDEPATH += D:/mylibrary/lua-5.3.2/src
INCLUDEPATH += D:/mylibrary/boost_1_60_0
INCLUDEPATH += D:/mylibrary/opencv/build/include

LIBS += -LD:/mylibrary/boost_1_60_0/lib64-msvc-12.0

debug {
    LIBS += libboost_filesystem-vc120-mt-gd-1_60.lib
    LIBS += libboost_system-vc120-mt-gd-1_60.lib
    LIBS += libboost_date_time-vc120-mt-gd-1_60.lib
    LIBS += libboost_regex-vc120-mt-gd-1_60.lib

}else
{
    LIBS += libboost_filesystem-vc120-mt-1_60.lib
    LIBS += libboost_system-vc120-mt-1_60.lib
    LIBS += libboost_date_time-vc120-mt-1_60.lib
    LIBS += libboost_regex-vc120-mt-1_60.lib
}




LIBS += D:/mylibrary/lua-5.3.2/src/build-Release/release/lualib.lib
LIBS += D:/mylibrary/opencv/build/x64/vc12/lib/opencv_world310.lib

SOURCES += main.cpp \
    gamescript.cpp \
    config.cpp \
    mainwindow.cpp \
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
    helperfun.h \
    scriptmanager.h \
    optiondlg.h \
    mhnetwork.h \
    ../control/rwhandler.h \
    ../control/protocol.h

target.path = ./bin
INSTALLS += target


RESOURCES += \
    icon.qrc

FORMS += \
    mainwindow.ui \
    selecttask.ui \
    optiondlg.ui

DISTFILES +=
