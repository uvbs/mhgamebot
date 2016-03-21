#ifndef MAINWINDOW_H
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
    void sign_append_msg_to_dbg(const QString& msg);
    void sign_append_msg_to_edit(const QString& msg);
    void sign_status_msg(const QString& msg, int time);


private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_loadscript_clicked();

    void on_pushButton_option_clicked();

    void on_action_QT_triggered();

    void menu_debug_triggered();

    void on_action_dbg_triggered();

    void on_pushButton_stop_clicked();

    void on_action_close_all_game_triggered();

private:
    QLabel *status_text;
    Ui::MainWindow *ui;
    ScriptManager script_manager;
    GameConfig _config;
    MHNetwork network;
    std::thread* _network_thread;
    void update_window_title(const QString& title);
    void create_status_bar();
};

#endif // MAINWINDOW_H
