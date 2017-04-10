#include "QFileDialog"
#include "QSettings"
#include "QDebug"
#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings){
    ui->setupUi(this);

    // TODO: Create a program directory in home folder!
    settingsFile = "~/QtAVR/user_settings.ini";
    qDebug() << "Settings file: " << settingsFile;
}

Settings::~Settings(){
    delete ui;
}

bool Settings::load(){
    qDebug() << "Load settings!";

    if(QFile(settingsFile).exists()){
        QSettings settings(settingsFile, QSettings::NativeFormat);
        ui->lineEditPathAvrdude->setText(settings.value("path.avrdude", "").toString());
        ui->lineEditPathToolchainRoot->setText(settings.value("path.toolchain_root", "").toString());
    }

    return false;
}

QString Settings::getAvrdudePath(){
    return ui->lineEditPathAvrdude->text();
}

QString Settings::getToolchainRootPath(){
    return ui->lineEditPathToolchainRoot->text();
}

void Settings::on_toolButtonPathAvrdude_clicked(){
    // TODO: Get the current operating system and use default start folder instead of /home
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select AVRDUDE directory"), "~/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty() == false) {
        ui->lineEditPathAvrdude->setText(dir);
    }
}

void Settings::on_toolButtonPathToolchainRoot_clicked(){
    // TODO: Get the current operating system and use default start folder instead of /home
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Toolchain root directory"), "~/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty() == false) {
        ui->lineEditPathToolchainRoot->setText(dir);
    }
}

void Settings::on_pushButtonSaveSettings_clicked(){
    qDebug() << "Save settings!";

    QSettings settings(settingsFile, QSettings::NativeFormat);
    settings.setValue("path.avrdude", ui->lineEditPathAvrdude->text());
    settings.setValue("path.toolchain_root", ui->lineEditPathToolchainRoot->text());

    this->accept();
}
