#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "config.h"
#include "scriptmanager.h"
#include "mhnetwork.h"


#include <QMainWindow>
#include <QListWidget>
#include <QCloseEvent>



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

private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_loadscript_clicked();

    void on_pushButton_option_clicked();

    void on_action_QT_triggered();

    void menu_debug_triggered();

    void on_action_dbg_triggered();

private:
    bool running = false;
    Ui::MainWindow *ui;
    ScriptManager _script_manager;
    QListWidget *script_listview;
    GameConfig _config;
    MHNetwork network;
    QListWidget* create_tab(QString title);
    void clear_tab();
    void update_window_title(QString title);
    void stop_script();
};

#endif // MAINWINDOW_H
