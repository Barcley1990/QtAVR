#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

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

private slots:
    void on_toolButtonPathAvrdude_clicked();
    void on_pushButtonSaveSettings_clicked();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
