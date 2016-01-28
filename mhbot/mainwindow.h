#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include "../scriptapp.h"


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
    void on_pushButton_config_clicked();
    void update_search_game_wnd();
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    ScriptApp app;
};

#endif // MAINWINDOW_H
