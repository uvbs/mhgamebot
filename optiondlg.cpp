#include "optiondlg.h"
#include "ui_optiondlg.h"
#include "config.h"
#include "helperfun.h"


optiondlg::optiondlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optiondlg)
{
    ui->setupUi(this);

    ui->checkBox_autoskipgc->setChecked(GameConfig::instance()->auto_skipgc);
    ui->checkBox_autostartgame->setChecked(GameConfig::instance()->auto_startgame);
    ui->lineEdit_helperaddr->setText(GameConfig::instance()->helperaddr);
    ui->lineEdit_port->setText(GameConfig::instance()->helperport);
    ui->checkBox_autohide->setChecked(GameConfig::instance()->auto_hide);
    ui->checkBox_autorun->setChecked(GameConfig::instance()->auto_run);
}

optiondlg::~optiondlg()
{
    delete ui;
}

void optiondlg::on_buttonBox_accepted()
{
    GameConfig::instance()->auto_skipgc = ui->checkBox_autoskipgc->isChecked();
    GameConfig::instance()->auto_startgame = ui->checkBox_autostartgame->isChecked();
    GameConfig::instance()->helperaddr = ui->lineEdit_helperaddr->text();
    GameConfig::instance()->helperport = ui->lineEdit_port->text();
    GameConfig::instance()->auto_run = ui->checkBox_autorun->isChecked();
    GameConfig::instance()->auto_hide = ui->checkBox_autohide->isChecked();
    GameConfig::instance()->save();

    if(ui->checkBox_autorun->isChecked())
    {
        install_autorun();
    }
    else{
        uninstall_autorun();
    }
}

void optiondlg::on_checkBox_autostartgame_clicked()
{

}

void optiondlg::on_checkBox_autorun_stateChanged(int arg1)
{

}
