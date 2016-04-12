#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostInfo>

#include "optiondlg.h"


extern std::mutex topwnd_mutex;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, SIGNAL(sign_append_msg_to_dbg(QString)), ui->textEdit_log, SLOT(append(QString)));
    connect(this, SIGNAL(sign_status_msg(QString, int)), ui->statusbar, SLOT(showMessage(QString,int)));

    //加载配置
    _config.load();

    create_status_bar();

    //设置图标
    setWindowIcon(QIcon(":/resource/icon.png"));


    ui->statusbar->showMessage(QString::fromLocal8Bit("待命"), 2000);

    //设置服务器
    network.set_server(
                GameConfig::instance()->helperaddr.toStdString(),
                GameConfig::instance()->helperport.toShort());

    //连接回调
    network.set_connect_callback([this](int id, const boost::system::error_code& err){
        if(err){
            sign_status_msg(QString::fromLocal8Bit("连接服务器失败"), 2000);

            //
            status_text->setText(QString::fromLocal8Bit("未连接服务器"));

            sign_append_msg_to_dbg(QString::fromLocal8Bit("连接服务器失败"));
            sign_append_msg_to_dbg(err.message().c_str());

        }
        else
        {
            sign_status_msg(QString::fromLocal8Bit("连接服务器成功"), 2000);
            status_text->setText(QString::fromLocal8Bit("已连接服务器"));


            //发送登录包到服务器
            LOGIN_PARAM login_param;
            QString machineName = QHostInfo::localHostName();
            strncpy(login_param.pcname, machineName.toLocal8Bit().data(), MAX_PCNAME);
            login_param.status = SCRIPT_STATUS::stop;
            login_param.game_wnd_counts = 0;
            login_param.start_time = 0;

            network.send(CMD_ID::login, (char*)&login_param, sizeof(LOGIN_PARAM));
        }

    });

    //读取回调
    network.set_process_callback([this](int id, int cmdid, const char* sz, int len){
        qDebug() << "process callback";

        switch(cmdid)
        {
            case dama:
            {

                DAMA_RE* re = (DAMA_RE*)sz;

                qDebug() << "script process dama";
                qDebug() << "x:" << re->x;
                qDebug() << "y:" << re->y;
                qDebug() << "script id:" << re->script_id;


                //让脚本处理
                GameScript* script = script_manager.get_script(re->script_id);

                try
                {
                    if(script)
                    {
                        std::lock_guard<std::mutex> locker(topwnd_mutex);
                        script->top_wnd();
                        if(re->rightclick == true)
                        {
                            script->click(re->x, re->y, 0);
                        }
                        else
                        {
                            script->click(re->x, re->y);
                        }

                        script->until_stop_run();
                    }
                    else
                    {
                        qDebug() << "err script id";
                    }

                }
                catch(std::exception& e)
                {
                    qDebug() << QString::fromLocal8Bit(e.what());
                }
                catch(...)
                {
                    qDebug() << "dama exception!";
                }

                //停止脚本的等待
                script->set_help_ok();

                break;
            }

        }


    });

    network.set_error_callback([this](int id, const boost::system::error_code& err){
        status_text->setText(QString::fromLocal8Bit("断开与服务器的连接"));
    });


    network.start();



    QListWidget* widget = create_widget(QString::fromLocal8Bit("脚本"));

    //设置脚本管理器的输出回调
    script_manager.set_output_callback([=](int type, const char *sz){
        widget->addItem(QString::fromLocal8Bit(sz));
    });

    ui->stackedWidget->setCurrentIndex(0);
    ui->action_dbg->setChecked(false);


    //自动设置上次使用脚本
    if(_config.get_last_script().isEmpty() == false){
        update_window_title(_config.get_last_script());
        script_manager.set_script(_config.get_last_script().toLocal8Bit().toStdString());
    }

}

MainWindow::~MainWindow()
{
    delete status_text;


    _config.save();
    script_manager.stop();
    delete ui;
}

void MainWindow::delete_widget()
{
    for(int i = ui->tabWidget->count(); i > 0 ; i--){
        QListWidget* widget = (QListWidget*)ui->tabWidget->widget(1);
        if(widget)
        {
            widget->deleteLater();
        }

        ui->tabWidget->removeTab(i);
    }

}

//运行脚本
void MainWindow::on_pushButton_start_clicked()
{
    try
    {
        delete_widget();

        //设置好回调
        std::vector<GameScript*> scripts = script_manager.create_all_script();
        if(scripts.size() == 0){
            script_manager.launcher_game(5);
            scripts = script_manager.create_all_script();
        }

        for(int i = 0; i < scripts.size(); i++)
        {
            QListWidget* widget = create_widget(QString::fromLocal8Bit("窗口%1").arg(scripts[i]->get_id()));

            //创建tab窗口
            scripts[i]->set_output_callback([this, widget](int type, const char* sz){
                if(widget)
                {
                    if(widget->count() > 50)
                    {
                        QListWidgetItem* item = widget->takeItem(0);
                        if(item){
                            delete item;
                        }
                    }
                }

                QListWidgetItem* item = new QListWidgetItem(sz);
                if(type == LOG_ERROR){
                    item->setTextColor(Qt::red);
                }
                else if(type == LOG_WARNING){
                    item->setTextColor(Qt::darkYellow);
                }
                else if(type == LOG_INFO){
                    item->setTextColor(Qt::blue);
                }

                widget->addItem(item);
            });


            scripts[i]->set_sendhelp_callback([this](const char* buf, int len)->bool{
                network.send(CMD_ID::dama, buf, len);
                return true;
            });

        }



        //启动所有脚本
        script_manager.start();

        ui->pushButton_start->setEnabled(false);
        ui->pushButton_stop->setEnabled(true);
        ui->pushButton_loadscript->setEnabled(false);

    }
    catch(std::runtime_error& e){
        QMessageBox::information(this, "information", e.what(), QMessageBox::Ok);
    }
    catch(...){

    }
}

void MainWindow::update_window_title(const QString& title)
{
    setWindowTitle(title.section("/", 1, -1) + " - mhbot");
}

void MainWindow::on_pushButton_loadscript_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("读取脚本"),
                                                     "",
                                                     QString::fromLocal8Bit("脚本 (*.lua)"));
    if(fileName.isEmpty() == false){
        //保存最近列表
        _config.add(fileName);

        //更新标题
        update_window_title(fileName);

        //设置脚本
        script_manager.set_script(fileName.toLocal8Bit().toStdString());
        ui->statusbar->showMessage(QString::fromLocal8Bit("脚本载入完成"), 2000);
    }
}


void MainWindow::on_pushButton_option_clicked()
{
    optiondlg dlg;
    dlg.exec();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(QMessageBox::information(this, QString::fromLocal8Bit("退出"),
                                QString::fromLocal8Bit("你确定要退出吗?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        ui->statusbar->showMessage(QString::fromLocal8Bit("等待脚本线程关闭.."), 2000);

        script_manager.stop();
        network.stop();


        e->accept();
    }
    else
    {
        e->ignore();
    }

}

void MainWindow::on_action_QT_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::menu_debug_triggered()
{

}

void MainWindow::on_action_dbg_triggered()
{
    if(ui->action_dbg->isChecked() == false){

        ui->stackedWidget->setCurrentIndex(0);
        ui->action_dbg->setChecked(false);
    }
    else{
        ui->stackedWidget->setCurrentIndex(1);
        ui->action_dbg->setChecked(true);

    }

}


void MainWindow::create_status_bar()
{
    status_text = new QLabel(QString::fromLocal8Bit("未连接服务器"));
    ui->statusbar->addPermanentWidget(status_text);
}

QListWidget* MainWindow::create_widget(const QString& wnd_name)
{
    QListWidget* widget = new QListWidget();
    widget->setFrameShape(QFrame::NoFrame);
    widget->setWordWrap(true);

    //自动滚动
    connect(widget->model(),
            SIGNAL(rowsInserted(const QModelIndex &, int, int ) ),
            widget,
            SLOT(scrollToBottom())
            );


    ui->tabWidget->addTab(widget,wnd_name);
    return widget;
}

void MainWindow::on_pushButton_stop_clicked()
{
    script_manager.stop();
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_loadscript->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
}



void MainWindow::on_action_close_all_game_triggered()
{
    script_manager.close_all_game();
}



void MainWindow::on_pushButton_reconnect_clicked()
{
    network.stop();
    network.start();
}

void MainWindow::on_action_create_game_triggered()
{
    script_manager.launcher_game(5);
}

void MainWindow::on_pushButton_testsrv_clicked()
{
    unsigned int all = 0;
    for(int i = 0; i < 50; i++)
    {
        int len = (rand()%100000) * i;
        all += len;
        char *buf = new char[len];
        network.send(rand()%5, buf, len);
        delete []buf;
    }

    QMessageBox::information(nullptr, "warning", QString("send size %1").arg(QString::number(all)), QMessageBox::Ok);
}
