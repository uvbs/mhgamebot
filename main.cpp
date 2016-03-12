#include "scriptmanager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>
#include <QFile>
#include <mutex>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString path;
    QDir dir;
    path=dir.currentPath();
    qDebug() << path;

    QStringList list;
    list.append(path.append(u8"/plugins"));
    app.setLibraryPaths(list);
    qDebug() << path;

    QFile file(":/resource/stylecss.css");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());


    MainWindow wnd;
    wnd.show();

    return app.exec();
}

