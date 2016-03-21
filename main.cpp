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


//    QString path;
//    path=QDir::currentPath();
//    qDebug() << path;

//    QStringList list;
//    list.append(path.append("/plugins"));
//    app.setLibraryPaths(list);
//    qDebug() << path;


    QString workpath = QDir::currentPath();
    QDir::setCurrent(workpath);

    QFile file(":/resource/stylecss.css");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());


    MainWindow wnd;
    wnd.show();

    return app.exec();
}

