#ifndef FUSEDIALOG_H
#define FUSEDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class FuseDialog;
}

class FuseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FuseDialog(QWidget *parent = 0);
    ~FuseDialog();

    void getWorkingDir(QString);

private:
    Ui::FuseDialog *ui;
    QString FuseFlashFilePath="/Users/tobias/Desktop/FFlash.sh";
    QString WorkingDir;


private slots:
    void SetFuses();

};

#endif // FUSEDIALOG_H
