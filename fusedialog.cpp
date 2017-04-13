#include "fusedialog.h"
#include "ui_fusedialog.h"

FuseDialog::FuseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FuseDialog)
{
    ui->setupUi(this);
}

FuseDialog::~FuseDialog()
{
    delete ui;
}
