#include "scriptmanager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>
#include <QFile>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");

    QFile file(":/resource/stylecss.css");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());


    MainWindow wnd;
    wnd.show();

    return app.exec();
}

