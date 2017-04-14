#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <qprocess.h>
#include <qdebug.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdiriterator.h>
#include <qmessagebox.h>
#include <qsyntaxhighlighter.h>
#include <qtextedit.h>
#include <QDirModel>
#include "QtXml/qdom.h"


#include "mysyntaxhighlighter.h"
#include "settings.h"
#include "editor.h"
#include "fusedialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    QString setPrgrammerName(void);
    QString setDeviceName(void);
    QString setWorkingDirectory(void);
    struct variables *setProjectVariables(void);

    QProcess *proc1;

private:
    Ui::MainWindow *ui;

    struct project{
        QString Workingdir;
        QString DeviceName;
        QString ProgrammerName;
    }p;

    //QString Workingdir;
    QStringList cFilePaths;
    QStringList cFileNames;
    QStringList oFileNames;

    bool is_error=false;

    // Settings
    Settings* userSettings;
    // Fuse Set Dialog
    FuseDialog* fuseSettings;

    // uC and Programmer
    QString uc = "-p m32", fd = "-c avrisp2";

    QFile BuildFile;
    QFile FlashFile;
    QString BuildFilePath="/Users/tobias/Desktop/Build.sh";
    QString FlashFilePath="/Users/tobias/Desktop/Flash.sh";

    int curTabIndex;    // hold current Tab index of MainTabWidget

    QStringList processors;
    QStringList processorAvrdudeCommands;
    QStringList processorGccCommands;
    QString currentProcessorAvrdudeCommand;
    QString currentProcessorGccCommand;
    QStringList programmers;
    QStringList programmerAvrdudeCommands;
    QString currentProgrammerAvrdudeCommand;

    QDirModel *model;

    void populateComboBoxes();

private slots:
    void rightMessage();
    void errorMessage();
    void Build();
    void Flash();
    void Run();
    void closeTab(int index);

    void DefineUC();
    void DefineFD();
    void on_actionSave_triggered();
    void on_actionBuild_triggered();
    void on_actionFlash_triggered();
    void on_actionRun_triggered();
    void on_actionNew_Project_triggered();
    void on_actionAbout_triggered();
    void on_actionOpen_Settings_triggered();
    void on_cbController_currentIndexChanged(int index);
    void on_cbFlashtool_currentIndexChanged(int index);
    void on_actionNew_File_triggered();
    void on_actionFuses_triggered();
    void on_fileChanged(QString filename);
    void on_dockWidgetWorktree_visibilityChanged(bool visible);
    void on_actionViewWorktree_toggled(bool arg1);
    void on_actionViewFile_toggled(bool arg1);
    void on_actionViewFileList_toggled(bool arg1);
    void on_actionViewConsole_toggled(bool arg1);
    void on_dockWidgetFile_visibilityChanged(bool visible);
    void on_dockWidgetFileList_visibilityChanged(bool visible);
    void on_dockWidgetConsole_visibilityChanged(bool visible);
    void on_actionExisting_File_triggered();
    void on_actionSave_All_triggered();
};

#endif // MAINWINDOW_H
