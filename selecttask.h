#ifndef SELECTTASK_H
#define SELECTTASK_H

#include <QDialog>

namespace Ui {
class selecttask;
}

class selecttask : public QDialog
{
    Q_OBJECT

public:
    explicit selecttask(QWidget *parent = 0);
    ~selecttask();

private:
    Ui::selecttask *ui;
};

#endif // SELECTTASK_H
