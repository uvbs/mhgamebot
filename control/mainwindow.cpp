#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QGraphicsView>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    model = new ScriptTableModel(this);
    ui->tableView->setModel(model);
    ui->statusBar->showMessage("Ready");


    for(int i = 0; i < 50; i++){
        QGraphicsView * newview = new QGraphicsView(ui->widget);
        newview->setParent(ui->widget);
        QImage image("D:\\code\\mh\\pic\\传送门.png");

        QGraphicsScene *scene = new QGraphicsScene;
        scene->addPixmap(QPixmap::fromImage(image));
        newview->setScene(scene);
        newview->resize(image.width() + 10, image.height() + 10);
        newview->show();

        //ui->gridLayout->addWidget(newview);
    }

    ui->widget->setLayout(new QGridLayout);
    ui->stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete model;
}

void MainWindow::on_aboutQtAct_triggered()
{
    qApp->aboutQt();
}

void MainWindow::on_pushButton_clicked()
{
    QImage image;

    QString fileName = QFileDialog::getOpenFileName(
                this, "open image file",
                ".",
                "Image files (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;All files (*.*)");
    if(fileName != "")
    {
        if(image.load(fileName))
        {
            QGraphicsScene *scene = new QGraphicsScene;
            scene->addPixmap(QPixmap::fromImage(image));

        }
    }
}

//显示视图
void MainWindow::on_action22_triggered()
{
    //hide list
    ui->tableView->hide();
    ui->widget->show();
}

//显示列表
void MainWindow::on_action11_2_triggered()
{
    //hide game wnd
    ui->widget->hide();
    ui->tableView->show();

}

void MainWindow::on_pushButton_2_clicked()
{
    static int current_index = 1;
    if(current_index == 1){
     ui->stackedWidget->setCurrentIndex(current_index);
     current_index = 0;
    }
    else{
     ui->stackedWidget->setCurrentIndex(current_index);
     current_index = 1;
    }
}
