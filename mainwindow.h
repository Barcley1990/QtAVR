#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QCloseEvent>
#include <qprocess.h>
#include <qdebug.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdiriterator.h>
#include <qmessagebox.h>
#include <qsyntaxhighlighter.h>
#include <qtextedit.h>
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

    QString setPrgrammerName(void);
    QString setDeviceName(void);
    QString setWorkingDirectory(void);
    struct variables *setProjectVariables(void);

    QProcess *TerminalProcess;

private:
    Ui::MainWindow *ui;

    QSettings* qtavr = NULL;
    struct project{
        QString Workingdir;
        QString DeviceName;
        QString ProgrammerName;
        //QString Workingdir;
        QStringList cFileNames;
        QStringList hFileNames;
        QStringList oFileNames;
    }p;

    bool is_error=false;

    // Settings
    Settings* userSettings;
    // Fuse Set Dialog
    FuseDialog* fuseSettings;

    QFile BuildFile;
    QFile FlashFile;
    QString BuildFilePath;
    QString FlashFilePath;

    int curTabIndex;    // hold current Tab index of MainTabWidget

    QStringList processors;
    QStringList processorAvrdudeCommands;
    QStringList processorGccCommands;
    QString currentProcessorAvrdudeCommand;
    QString currentProcessorGccCommand;
    QStringList programmers;
    QStringList programmerAvrdudeCommands;
    QString currentProgrammerAvrdudeCommand;

    QDirModel *dirmodel;

    QSettings* projectFile;

    void loadXMLFiles();
    bool unsavedFiles();
    void reloadFileList();
    void saveAllFiles();
    void saveProject();
    void reloadProjectSetting();
    void generateMakefile();
    void addDroppedFile(QString file);

public slots:
    // Update Tree view
    void updateActions();

private slots:
    void rightMessage();
    void errorMessage();
    void Build();
    void Flash();
    void Run();
    void closeTab(int index);

    // Slots for ProjectTree
    void insertChild();
    bool insertColumn();
    void insertRow();
    bool removeColumn();
    void removeRow();

    // Action Slots
    void on_actionSave_triggered();
    void on_actionBuild_triggered();
    void on_actionFlash_triggered();
    void on_actionRun_triggered();
    void on_actionNew_Project_triggered();
    void on_actionAbout_triggered();
    void on_actionOpen_Settings_triggered();
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
	void on_actionDefault_View_triggered();
	void on_actionFile_triggered();
    void on_actionSave_as_triggered();
    void on_actionOpen_Project_triggered();
    void on_actionProject_Settings_triggered();

    void on_actionClose_Project_triggered();

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // MAINWINDOW_H
