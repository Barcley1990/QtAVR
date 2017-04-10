#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qprocess.h>
#include <qdebug.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdiriterator.h>
#include <qmessagebox.h>
#include <qsyntaxhighlighter.h>
#include <qtextedit.h>

#include "mysyntaxhighlighter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    QProcess *proc1;
private:
    Ui::MainWindow *ui;
    QStringList cFilePaths;
    QStringList cFileNames;
    QStringList oFileNames;
    QString Workingdir;
    bool is_error=false;

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

    QTabBar *TabBar;
    QTextEdit *mainEditor;

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
};

#endif // MAINWINDOW_H
