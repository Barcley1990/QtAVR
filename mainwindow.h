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


#include "mysyntaxhighlighter.h"
#include "settings.h"
#include "editor.h"

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


    QProcess *proc1;
private:
    Ui::MainWindow *ui;
    QStringList cFilePaths;
    QStringList cFileNames;
    QStringList oFileNames;
    QString Workingdir;
    bool is_error=false;

    // Settings
    Settings* userSettings;

    // uC and Programmer
    QString uc = "-p m32", fd = "-c avrisp2";

    QDir d;
    QFile BuildFile;
    QFile FlashFile;
    QFile FFlashFile;
    QFile *mainFile;
    QString BuildFilePath="/Users/tobias/Desktop/Build.sh";
    QString FlashFilePath="/Users/tobias/Desktop/Flash.sh";
    QString FFlashFilePath="/Users/tobias/Desktop/FFlash.sh";

    Editor *mainEditor;

private slots:
    void rightMessage();
    void errorMessage();
    void AddCFile();
    void Build();
    void Flash();
    void Run();
    void NewProject();
    void OpenProject();
    void FlashFuses();
    void DefineUC();
    void DefineFD();
    void SaveFile();
    void on_actionSave_triggered();
    void on_actionBuild_triggered();
    void on_actionFlash_triggered();
    void on_actionRun_triggered();
    void on_actionNew_Project_triggered();
    void on_actionAbout_triggered();
    void on_actionOpen_Settings_triggered();
    void on_actionNew_File_triggered();
};

#endif // MAINWINDOW_H
