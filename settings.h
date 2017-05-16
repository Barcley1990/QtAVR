#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QCompleter>
#include <QDirModel>

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
    QString getAvrgccPath();
    QString getAvrobjcopyPath();
    QString getToolchainRootPath();
    void setGeometry(QByteArray ba);
    QByteArray getGeometry();
    void setWindowState(QByteArray ba);
    QByteArray getWindowState();

    void setDefaultProgrammer(int programmer);
    int getDefaultProgrammer();
    void setDefaultProcessor(int processor);
    int getDefaultProcessor();

    // Editor settings
    void setFontSize(int size);
    int getFontSize();
    void setTabWidth(int width);
    int getTabWidth();

private slots:
    void on_toolButtonPathAvrdude_clicked();
    void on_toolButtonPathToolchainRoot_clicked();
    void on_pushButtonSaveSettings_clicked();

    void on_toolButtonPathAVROBJCPY_clicked();

    void on_toolButtonPathAVRGCC_clicked();

private:
    Ui::Settings *ui;
    QString settingsFile;
    QSettings* settings;
    QCompleter* modelCompleter;
};

#endif // SETTINGS_H
