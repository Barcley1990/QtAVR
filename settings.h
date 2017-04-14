#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    // Constructor & Destructor
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    // Load function, called once at startup
    bool load();
    // Getters
    QString getAvrdudePath();
    QString getToolchainRootPath();
    void setGeometry(QByteArray ba);
    QByteArray getGeometry();
    void setWindowState(QByteArray ba);
    QByteArray getWindowState();

private slots:
    void on_toolButtonPathAvrdude_clicked();
    void on_toolButtonPathToolchainRoot_clicked();
    void on_pushButtonSaveSettings_clicked();

private:
    Ui::Settings *ui;
    QString settingsFile;
    QSettings* settings;
};

#endif // SETTINGS_H
