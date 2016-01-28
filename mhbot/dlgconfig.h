#ifndef DLGCONFIG_H
#define DLGCONFIG_H

#include <QDialog>

namespace Ui {
class dlgconfig;
}

class dlgconfig : public QDialog
{
    Q_OBJECT

public:
    explicit dlgconfig(QWidget *parent = 0);
    ~dlgconfig();

private:
    Ui::dlgconfig *ui;
};

#endif // DLGCONFIG_H
