#include "QFileDialog"
#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings){
    ui->setupUi(this);
}

Settings::~Settings(){
    delete ui;
}

bool Settings::load(){
    // TODO: Load all settings from the user settings file and return true if all settings loaded.
    // Otherwise return false
    return false;
}

QString Settings::getAvrdudePath(){
    return ui->lineEditPathAvrdude->text();
}

void Settings::on_toolButtonPathAvrdude_clicked(){
    // TODO: Get the current operating system and use default start folder instead of /home
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select AVRDUDE directory"), "~/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty() == false) {
        ui->lineEditPathAvrdude->setText(dir);
    }
}

void Settings::on_pushButtonSaveSettings_clicked(){
    // TODO: Save all settings from this dialog to a file
    this->accept();
}
