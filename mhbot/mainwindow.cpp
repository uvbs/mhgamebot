#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dlgconfig.h"
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <list>
#include <QTextCodec>
#include <QString>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_search_game_wnd()));
    timer->start(2000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_search_game_wnd()
{

    auto&& wndlist = app.Get_game_wnd_list();
    for(auto i = wndlist.begin(); i != wndlist.end(); i++)
    {
        char title[100];
        ::GetWindowTextA(i->wnd, title, 100);
        ui->listWidget_game_wnd->addItem(QString::fromLocal8Bit(title));
    }

    ui->label_game_counts->setText(QString("游戏窗口数量: %1 ").arg(wndlist.size()));
}

void MainWindow::on_pushButton_config_clicked()
{
    dlgconfig dlg;
    dlg.exec();
}

void MainWindow::on_pushButton_clicked()
{
    if(QMessageBox::Ok == QMessageBox::information(this, "提示", "你确定关闭所有游戏窗口?"))
    {
        app.Close_all_game();
    }

}
