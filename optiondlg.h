#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include <QtCore>
#include <QtGui>
#include <QWidget>

#include <QMessageBox>


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

    void on_checkBox_autostartgame_clicked();

    void on_checkBox_autorun_stateChanged(int arg1);

private:
    Ui::optiondlg *ui;
};

#endif // OPTIONDLG_H
