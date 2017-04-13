#ifndef FUSEDIALOG_H
#define FUSEDIALOG_H

#include <QDialog>

namespace Ui {
class FuseDialog;
}

class FuseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FuseDialog(QWidget *parent = 0);
    ~FuseDialog();

private:
    Ui::FuseDialog *ui;
};

#endif // FUSEDIALOG_H
