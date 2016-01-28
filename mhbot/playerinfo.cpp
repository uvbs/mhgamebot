#include "playerinfo.h"
#include "ui_playerinfo.h"

playerinfo::playerinfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::playerinfo)
{
    ui->setupUi(this);
}

playerinfo::~playerinfo()
{
    delete ui;
}
