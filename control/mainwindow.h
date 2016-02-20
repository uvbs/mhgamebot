#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "scripttablemodel.h"
#include <QGridLayout>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_aboutQtAct_triggered();
    
    void on_pushButton_clicked();

    void on_action22_triggered();

    void on_action11_2_triggered();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    ScriptTableModel *model;
};

#endif // MAINWINDOW_H
