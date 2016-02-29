#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "selecttask.h"
#include <QHBoxLayout>
#include <QListWidget>
#include <QFileDialog>
#include <QIcon>

#include "optiondlg.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _config.load();

    setWindowIcon(QIcon(":/resource/icon.png"));

    if(_config.get_last_script().isEmpty() == false){
        update_window_title(_config.get_last_script());
        _script_manager.set_script(_config.get_last_script().toLocal8Bit().toStdString());
    }

    //回调
    script_listview = create_tab(QString::fromLocal8Bit("辅助"));
    _script_manager.set_output_callback([&](int type, char *sz){
        QString message = QString::fromLocal8Bit(sz);
        QListWidgetItem* item = new QListWidgetItem(message);
        if(type == LOG_DEBUG){
            item->setTextColor(Qt::yellow);
        }
        else if(type == LOG_WARNING){
            item->setTextColor(Qt::red);
        }
        else if(type == LOG_NORMAL){
            item->setTextColor(Qt::white);
        }
        script_listview->addItem(item);
    });

    ui->statusbar->showMessage(QString::fromLocal8Bit("待命"));
}

MainWindow::~MainWindow()
{
    _config.save();
    delete ui;
}

void MainWindow::clear_tab()
{
    for(int i = 0; i < ui->tabWidget->count(); i++){
        ui->tabWidget->removeTab(1);
    }

}

void MainWindow::on_pushButton_start_clicked()
{

    clear_tab();


    if(running)
    {

    }
    else{
        int counts = _script_manager.get_game_counts();

        //call back
        std::vector<std::function<void(int type, char*)>> callback_list;
        for(int i = 0; i < counts; i++){
            QListWidget* widget = create_tab(QString::fromLocal8Bit("窗口%1").arg(i));
            callback_list.push_back([=](int type, char* sz){
                QString message = QString::fromLocal8Bit(sz);
                QListWidgetItem* item = new QListWidgetItem(message);
                if(type == LOG_DEBUG){
                    item->setTextColor(Qt::yellow);
                }
                else if(type == LOG_WARNING){
                    item->setTextColor(Qt::red);
                }
                else if(type == LOG_NORMAL){
                    item->setTextColor(Qt::white);
                }
                widget->addItem(item);
            });
        }
        _script_manager.set_gamescript_callback_list(callback_list);
        _script_manager.run();

    }

}

void MainWindow::update_window_title(QString title)
{
    setWindowTitle(title.section("/", -1, -1) + QString(" - mhbot"));
}

void MainWindow::on_pushButton_pause_clicked()
{
    //ui->statusbar->showMessage(QString::fromLocal8Bit("开始停止.."));
    _script_manager.pause();
    //ui->statusbar->showMessage(QString::fromLocal8Bit("已停止"));
}

void MainWindow::on_pushButton_loadscript_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("读取脚本"),
                                                     "",
                                                     tr("脚本 (*.lua)"));
    if(fileName.isEmpty() == false){
        //保存最近列表
        _config.add(fileName);

        //更新标题
        update_window_title(fileName);

        //设置脚本
        _script_manager.delete_all_script();
        _script_manager.set_script(fileName.toLocal8Bit().toStdString());
        ui->statusbar->showMessage(QString::fromLocal8Bit("脚本载入完成"));
    }
}


QListWidget* MainWindow::create_tab(QString title){

    QListWidget *newlist = new QListWidget();
    ui->tabWidget->insertTab(ui->tabWidget->count(), newlist, title);
    newlist->setAutoScroll(true);

    //自动滚动
    connect(newlist->model(),
            SIGNAL(rowsInserted(const QModelIndex &, int, int ) ),
            newlist,
            SLOT(scrollToBottom())
            );

    return newlist;
}

void MainWindow::on_pushButton_option_clicked()
{
    optiondlg dlg;
    dlg.exec();
}

void MainWindow::stop_script()
{
    ui->statusbar->showMessage(QString::fromLocal8Bit("等待脚本线程关闭.."));
    _script_manager.stop();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    stop_script();
    e->accept();
}
