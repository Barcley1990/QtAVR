#include "QFileDialog"
#include "QSettings"
#include "QDebug"
#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings){
    ui->setupUi(this);

    settingsFile = QDir::homePath() + "/QtAVR/user_settings.ini";
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
        return true;
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
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select AVRDUDE directory"), QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty() == false) {
        ui->lineEditPathAvrdude->setText(dir);
    }
}

void Settings::on_toolButtonPathToolchainRoot_clicked(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select AVR Toolchain root directory"), QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

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
