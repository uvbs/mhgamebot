#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "scripttablemodel.h"



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
    
private:
    Ui::MainWindow *ui;
    ScriptTableModel *model;
};

#endif // MAINWINDOW_H
