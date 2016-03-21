#include "mainwindow.h"
#include "ui_mainwindow.h"



#include "optiondlg.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, SIGNAL(sign_append_msg_to_dbg(QString)), ui->textEdit_log, SLOT(append(QString)));
    connect(this, SIGNAL(sign_append_msg_to_edit(QString)), ui->textEdit, SLOT(append(QString)));
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
            status_text->setText(QString::fromLocal8Bit("未连接服务器"));
        }
        else
        {
            sign_status_msg(QString::fromLocal8Bit("连接服务器成功"), 2000);
            status_text->setText(QString::fromLocal8Bit("已连接服务器"));


            //发送登录包到服务器
            LOGIN_PARAM login_param;
            strncpy(login_param.pcname, "pc name", MAX_PCNAME);
            strncpy(login_param.script_name,"无", MAX_SCRIPTNAME);
            strncpy(login_param.script_type, "无", MAX_SCRIPTNAME);
            login_param.status = SCRIPT_STATUS::stop;
            login_param.game_wnd_counts = 0;
            login_param.start_time = 0;

            network.send(CMD_ID::login, (char*)&login_param, sizeof(LOGIN_PARAM));

        }

    });

    //读取回调
    network.set_process_callback([this](int id, int cmdid, const char* sz, int len){
        qDebug() << "process callback";
    });

    network.set_error_callback([this](int id, const boost::system::error_code& err){
        status_text->setText("断开与服务器的连接");
    });

    //启动客户端网络
    _network_thread = new std::thread([this](){
        network.start();
    });



    //设置脚本管理器的输出回调
    script_manager.set_output_callback([&](int type, const char *sz){
        sign_append_msg_to_edit(sz);
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
    _config.save();
    script_manager.stop();
    delete ui;
}



//运行脚本
void MainWindow::on_pushButton_start_clicked()
{

    //设置好回调
    std::vector<GameScript*> scripts = script_manager.create_all_script();
    for(int i = 0; i < scripts.size(); i++)
    {
        //创建tab窗口
        scripts[i]->set_output_callback([=](int type, const char* sz){
            sign_append_msg_to_edit(sz);
        });

        //TODO:
        scripts[i]->set_sendhelp_callback([this](DAMA_PARAM* param, const char* data, int len)->bool{
            sign_append_msg_to_dbg("新的人工请求发出");
            sign_append_msg_to_dbg(QString::number(param->height));
            sign_append_msg_to_dbg(QString::number(param->width));
            sign_append_msg_to_dbg(QString::number(param->x));
            sign_append_msg_to_dbg(QString::number(param->y));
            sign_append_msg_to_dbg("done");

            int buflen = sizeof(DAMA_PARAM) + len;
            char* buf = new char[buflen];
            memcpy(buf, param, sizeof(DAMA_PARAM));
            memcpy(buf + sizeof(DAMA_PARAM), data, len);
            network.send(CMD_ID::dama, buf, buflen);
            delete buf;

            return true;
        });
    }

    //启动所有脚本
    script_manager.start();

    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);
    ui->pushButton_loadscript->setEnabled(false);

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
