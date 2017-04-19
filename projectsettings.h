#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class ProjectSettings;
}

class ProjectSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSettings(QWidget *parent = 0, QSettings* settings = NULL);
    ~ProjectSettings();

private:
    Ui::ProjectSettings *ui;
    QSettings* settings;
    QStringList processors;
    QStringList processorAvrdudeCommands;
    QStringList processorGccCommands;
    QString currentProcessorAvrdudeCommand;
    QString currentProcessorGccCommand;
    QStringList programmers;
    QStringList programmerAvrdudeCommands;
    QString currentProgrammerAvrdudeCommand;

    void populateComboBoxes();
    void saveSettings();
};

#endif // PROJECTSETTINGS_H
