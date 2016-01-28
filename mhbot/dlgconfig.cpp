#include "dlgconfig.h"
#include "ui_dlgconfig.h"

dlgconfig::dlgconfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgconfig)
{
    ui->setupUi(this);
}

dlgconfig::~dlgconfig()
{
    delete ui;
}
