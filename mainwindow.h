﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "config.h"
#include "scriptmanager.h"
#include "mhnetwork.h"


#include <QtCore>
#include <QtGui>
#include <QtWidgets>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);


signals:
    void append_msg_to_dbg(const QString& msg);


private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_loadscript_clicked();

    void on_pushButton_option_clicked();

    void on_action_QT_triggered();

    void menu_debug_triggered();

    void on_action_dbg_triggered();

private:
    QLabel *status_text;
    Ui::MainWindow *ui;
    ScriptManager script_manager;
    QListWidget *script_listview;
    GameConfig _config;
    MHNetwork network;
    QListWidget* create_tab(QString title);
    std::thread* _network_thread;
    void update_window_title(QString title);
    void app_text(QListWidget* widget, int type, const char* sz);
    void create_status_bar();
};

#endif // MAINWINDOW_H
