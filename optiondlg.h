#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include <QDialog>

namespace Ui {
class optiondlg;
}

class optiondlg : public QDialog
{
    Q_OBJECT

public:
    explicit optiondlg(QWidget *parent = 0);
    ~optiondlg();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::optiondlg *ui;
};

#endif // OPTIONDLG_H
