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
        settings = new QSettings(settingsFile, QSettings::NativeFormat);
        ui->lineEditPathAvrdude->setText(settings->value("path.avrdude", "").toString());
        ui->lineEditPathToolchainRoot->setText(settings->value("path.toolchain_root", "").toString());
        ui->spinBoxFontSize->setValue(settings->value("editor.fontsize", 14).toInt());
        ui->spinBoxTabWidth->setValue(settings->value("editor.tabwidth", 3).toInt());
        return true;
    }else{
        settings = new QSettings(settingsFile, QSettings::NativeFormat);
    }
    return false;
}

QString Settings::getAvrdudePath(){
    return ui->lineEditPathAvrdude->text();
}

QString Settings::getToolchainRootPath(){
    return ui->lineEditPathToolchainRoot->text();
}

void Settings::setGeometry(QByteArray ba)
{
    settings->setValue("window.geometry", ba);
}

QByteArray Settings::getGeometry()
{
    return settings->value("window.geometry", QVariant::Invalid).toByteArray();
}

void Settings::setWindowState(QByteArray ba)
{
    settings->setValue("window.state", ba);
}

QByteArray Settings::getWindowState()
{
    return settings->value("window.state", QVariant::Invalid).toByteArray();
}

void Settings::setDefaultProgrammer(int programmer)
{
    settings->setValue("default.programmer", programmer);
}

int Settings::getDefaultProgrammer()
{
    int i = 0;
    QVariant qv = settings->value("default.programmer", QVariant::Invalid);
    if(qv.isNull() == false)
    {
        return qv.toInt();
    }
    return i;
}

void Settings::setDefaultProcessor(int processor)
{
    settings->setValue("default.processor", processor);
}

int Settings::getDefaultProcessor()
{
    int i = 0;
    QVariant qv = settings->value("default.processor", QVariant::Invalid);
    if(qv.isNull() == false)
    {
        return qv.toInt();
    }
    return i;
}

void Settings::setFontSize(int size)
{
    ui->spinBoxFontSize->setValue(size);
}

int Settings::getFontSize()
{
    return ui->spinBoxFontSize->value();
}

void Settings::setTabWidth(int width)
{
    ui->spinBoxTabWidth->setValue(width);
}

int Settings::getTabWidth()
{
    return ui->spinBoxTabWidth->value();
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

    settings->setValue("path.avrdude", ui->lineEditPathAvrdude->text());
    settings->setValue("path.toolchain_root", ui->lineEditPathToolchainRoot->text());
    settings->setValue("editor.fontsize", ui->spinBoxFontSize->value());
    settings->setValue("editor.tabwidth", ui->spinBoxTabWidth->value());

    this->accept();
}
