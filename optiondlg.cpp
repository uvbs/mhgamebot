#include "optiondlg.h"
#include "ui_optiondlg.h"
#include "config.h"


optiondlg::optiondlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optiondlg)
{
    ui->setupUi(this);

    ui->checkBox_autoskipgc->setChecked(GameConfig::instance()->auto_skipgc);
    ui->checkBox_autostartgame->setChecked(GameConfig::instance()->auto_startgame);
    ui->lineEdit_helperaddr->setText(GameConfig::instance()->helperaddr);
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

    GameConfig::instance()->save();
}
