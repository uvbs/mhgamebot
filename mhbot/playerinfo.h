#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <QDialog>

namespace Ui {
class playerinfo;
}

class playerinfo : public QDialog
{
    Q_OBJECT

public:
    explicit playerinfo(QWidget *parent = 0);
    ~playerinfo();

private:
    Ui::playerinfo *ui;
};

#endif // PLAYERINFO_H
