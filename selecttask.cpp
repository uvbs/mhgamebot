#include "selecttask.h"
#include "ui_selecttask.h"

selecttask::selecttask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::selecttask)
{
    ui->setupUi(this);
}

selecttask::~selecttask()
{
    delete ui;
}
