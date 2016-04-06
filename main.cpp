#include "scriptmanager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>
#include <QFile>
#include <mutex>
#include <clocale>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //当前工作目录
    QString mhbot_dir = QCoreApplication::applicationDirPath();
    QDir::setCurrent(mhbot_dir);

    QFile file(":/resource/stylecss.css");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());


    MainWindow wnd;
    wnd.show();

    return app.exec();
}

