#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QCloseEvent>

#include "config.h"
#include "scriptmanager.h"

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

    void on_pushButton_pause_clicked();

    void on_pushButton_loadscript_clicked();

    void on_pushButton_option_clicked();

private:
    bool running = false;
    Ui::MainWindow *ui;
    ScriptManager _script_manager;
    QListWidget *script_listview;
    GameConfig _config;
    QListWidget* create_tab(QString title);
    void clear_tab();
    void update_window_title(QString title);
    void stop_script();
};

#endif // MAINWINDOW_H
