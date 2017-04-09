#include "mainwindow.h"
#include "ui_mainwindow.h"


// TODO:
// Dateipfade ändern.
// Comboboxen schreiben
// flashen schreiben


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* create QProcess object */
    proc1 = new QProcess();

    // default file path at start
    d.setPath("/Users/tobias/Desktop/");
    ui->lWD->setText(d.path());

    /* show output */
    connect(proc1, SIGNAL(readyReadStandardOutput()),this, SLOT(rightMessage()) );
    connect(proc1, SIGNAL(readyReadStandardError()), this, SLOT(errorMessage()) );
    /* buttons */
    connect(ui->bAddFile, SIGNAL(clicked()), this, SLOT(AddCFile()));
    connect(ui->bBuild, SIGNAL(clicked()), this, SLOT(Build()));
    connect(ui->bFlash, SIGNAL(clicked()), this, SLOT(Flash()));
    connect(ui->bRun, SIGNAL(clicked()), this, SLOT(Run()));
    connect(ui->bNew, SIGNAL(clicked()), this, SLOT(NewProject()));
    connect(ui->bOpen, SIGNAL(clicked()), this, SLOT(OpenProject()));
    connect(ui->bFuses, SIGNAL(clicked()), this, SLOT(FlashFuses()));

    connect(ui->cbController, SIGNAL(currentIndexChanged(int)), this, SLOT(DefineUC()));
    connect(ui->cbFlashtool, SIGNAL(currentIndexChanged(int)), this, SLOT(DefineFD()));

    /*-------------------------------------------------------------------------------*/

}

MainWindow::~MainWindow()
{
    delete ui;
    // delete Bashfiles
    if(BuildFile.exists())
        BuildFile.remove();
    if(FlashFile.exists())
        FlashFile.remove();
    if(FFlashFile.exists())
        FFlashFile.remove();
}

void MainWindow::NewProject(){
    qDebug() << "Create new project" << endl;

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    d.path(),
                                                    "c-Files (*.c)"
                                                    );
    if(filename>1){
        QFile f(filename);
        // get actual working dir
        d = QFileInfo(filename).absoluteDir();
        Workingdir = d.absolutePath();
        ui->lWD->setText(Workingdir);
        f.open( QIODevice::WriteOnly );

        QTextStream stream( &f );
        stream << "#define F_CPU x000000UL"     << endl
               << endl
               << "#include <stdlib.h>"         << endl
               << "#include <avr/io.h>"         << endl
               << "#include <util/delay.h>"     << endl
               << endl
               << endl
               << "void setup(){"                << endl
               << endl
               << "}"                            << endl
               << endl
               << endl
               << "void main(){"                 << endl
               << endl
               << "}"                            << endl
               << endl;
        f.close();

        cFilePaths.clear();
        cFileNames.clear();
        // append filepath to list
        cFilePaths.append(filename);
        // get filname from filepath
        QFileInfo fi(filename);
        cFileNames.append(fi.fileName());

        qDebug() << "Filepath: " << filename << endl;
        qDebug() << "Number of Paths in List: " << cFilePaths.length() <<endl;
        for(uint8_t i=0; i<cFilePaths.length(); i++){
            qDebug() << i << ": " << cFilePaths[i] << endl;
        }
        for(uint8_t i=0; i<cFileNames.length(); i++){
            qDebug() << i << ": " << cFileNames[i] << endl;
        }
        // create o-files
        oFileNames.clear();
        for(volatile uint8_t i=0; i<cFileNames.length(); i++){
            QString fn = cFileNames[i];
            int dot = fn.indexOf(".")+1;
            oFileNames.append(fn.replace(dot,1,"o"));
            qDebug() << oFileNames[i] << endl;
        }
        // set on label
        ui->cCfiles->clear();
        for(uint8_t i=0; i<cFilePaths.length(); i++){
            ui->cCfiles->append(cFileNames[i]);
        }

     }
}

void MainWindow::OpenProject(){
    qDebug() << "Open project" << endl;

    QString filename = QFileDialog::getOpenFileName(this,
                                                     tr("Open File"),
                                                     d.path(),
                                                     "c-Files (*.c)"
                                                     );
    if(filename>1){
        cFilePaths.clear();
        cFileNames.clear();
        // get actual working dir
        d = QFileInfo(filename).absoluteDir();
        Workingdir = d.absolutePath();
        ui->lWD->setText(Workingdir);

        // append filepath to list
        cFilePaths.append(filename);
        // get filname from filepath
        QFileInfo fi(filename);
        cFileNames.append(fi.fileName());

        // find other c-files and append
        QDirIterator it(d.path(), QStringList() << "*.c", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()){
            qDebug() << it.next();
            qDebug() << it.fileName();
            if(it.fileName()!=fi.fileName()){
                cFileNames.append(it.fileName());
            }
        }
        // output cFile List in debugger
        qDebug() << "Filepath: " << filename << endl;
        qDebug() << "Number of Paths in List: " << cFilePaths.length() <<endl;
        for(uint8_t i=0; i<cFilePaths.length(); i++){
            qDebug() << i << ": " << cFilePaths[i] << endl;
        }
        for(uint8_t i=0; i<cFileNames.length(); i++){
            qDebug() << i << ": " << cFileNames[i] << endl;
        }
        // create o-files
        oFileNames.clear();
        for(volatile uint8_t i=0; i<cFileNames.length(); i++){
            QString fn = cFileNames[i];
            int dot = fn.indexOf(".")+1;
            oFileNames.append(fn.replace(dot,1,"o"));
            qDebug() << oFileNames[i] << endl;
        }
        // set on label
        ui->cCfiles->clear();
        for(uint8_t i=0; i<cFileNames.length(); i++){
            //if(i==0)
            //   ui->cCfiles->setTextColor(QColor(220,220,220));
            //else
            //    ui->cCfiles->setTextColor(QColor(0,0,0));
            ui->cCfiles->append(cFileNames[i]);
        }



    }
}

void MainWindow::AddCFile()
{
    qDebug() << "Add File" << endl;
    QString filename = QFileDialog::getOpenFileName(this,
                                                     tr("Open File"),
                                                     d.path(),
                                                     "c-Files (*.c)"
                                                     );
    if(filename>1){
        // get actual working dir
        d = QFileInfo(filename).absoluteDir();
        Workingdir = d.absolutePath();
        ui->lWD->setText(Workingdir);

        // append filepath to list
        cFilePaths.append(filename);
        // get filname from filepath
        QFileInfo fi(filename);
        cFileNames.append(fi.fileName());

        qDebug() << "Filepath: " << filename << endl;
        qDebug() << "Number of Paths in List: " << cFilePaths.length() <<endl;
        for(uint8_t i=0; i<cFilePaths.length(); i++){
            qDebug() << i << ": " << cFilePaths[i] << endl;
        }
        for(uint8_t i=0; i<cFileNames.length(); i++){
            qDebug() << i << ": " << cFileNames[i] << endl;
        }
        // create o-files
        oFileNames.clear();
        for(volatile uint8_t i=0; i<cFileNames.length(); i++){
            QString fn = cFileNames[i];
            int dot = fn.indexOf(".")+1;
            oFileNames.append(fn.replace(dot,1,"o"));
            qDebug() << oFileNames[i] << endl;
        }
        // set on label
        ui->cCfiles->clear();
        for(uint8_t i=0; i<cFilePaths.length(); i++){
            ui->cCfiles->append(cFileNames[i]);
        }
    }
}

void MainWindow::Build()
{
    qDebug() << "Build" << endl;

    //********** Strings für avr-gcc vorbereiten **********/
    QString compile = "/usr/local/Cellar/avr-gcc/6.2.0/bin/avr-gcc -g -Os -mmcu=attiny2313 -c ";
    QString link    = "/usr/local/Cellar/avr-gcc/6.2.0/bin/avr-gcc -g -mmcu=attiny2313 -o ";
    QString hex     = "/usr/local/Cellar/avr-binutils/2.25/bin/avr-objcopy -j .text -j .data -O ihex ";

    // Compile
    for(uint8_t i=0; i<cFileNames.length(); i++){
        compile.append(cFileNames[i]);
        compile.append(" ");
    }
    //Link object files and create .ELF-file
    link.append("main.elf ");
    for(uint8_t i=0; i<oFileNames.length(); i++){
        link.append(oFileNames[i]);
        link.append(" ");
    }
    // create HEX from ELF file
    hex.append("main.elf main.hex");

    qDebug() << "compile: " << compile << endl;
    qDebug() << "link: "   << link    << endl;
    qDebug() << "hex: "    << hex     << endl;

    //********** Script-File erstellen **********/
    // Edit Build.sh
    // Create SHELL Files
    BuildFilePath = d.path().append("/Build.sh");
    BuildFile.setFileName(BuildFilePath);
    if (BuildFile.open(QIODevice::ReadWrite)){
        QTextStream stream( &BuildFile );
        stream << "#!/bin/bash \n" << endl;
        stream << "cd " << Workingdir << "\n" << endl;
        stream << compile << "\n" << endl;
        stream << link    << "\n" << endl;
        stream << hex     << "\n" << endl;
    }
    // set file permissions to executeable
    BuildFile.setPermissions(QFile::ReadGroup | QFile::ExeGroup  |
                             QFile::ReadUser  | QFile::WriteUser | QFile::ExeUser  |
                             QFile::ReadOther | QFile::ReadOther | QFile::ExeOther |
                             QFile::ExeOwner );

    // close file
    BuildFile.close();

    //********** Script-File ausfuehren **********/
    // Execute Scriptfile
    proc1->start(BuildFilePath);
    if (is_error == false){
        ui->cOutput->setTextColor(QColor(0,255,0));
        ui->cOutput->setText("Build OK!");
    }
    else
        is_error = false;

    //********** Script-File aufraeumen **********/
    //if(BuildFile.exists())
    //    BuildFile.remove();

}

void MainWindow::Flash()
{
    qDebug() << "Flash"<< endl;
    qDebug() << "uC: " << uc << " programer: " << fd << endl;
    QString avrdude = "/usr/local/Cellar/avrdude/6.3/bin/avrdude " ;
    QString write = "-U flash:w:main.hex";
    avrdude.append(fd).append(" ").append(uc).append(" ").append(write);
    qDebug() << avrdude << endl;

    //********** Script-File erstellen **********/
    // Edit Build.sh
    // Create SHELL Files
    FlashFilePath = d.path().append("/Flash.sh");
    FlashFile.setFileName(FlashFilePath);
    if (FlashFile.open(QIODevice::ReadWrite)){
        QTextStream stream( &FlashFile );
        stream << "#!/bin/bash \n" << endl;
        stream << "cd " << Workingdir << "\n" << endl;
        stream << avrdude << "\n" << endl;
    }
    // set file permissions to executeable
    FlashFile.setPermissions(QFile::ReadGroup | QFile::ExeGroup  |
                             QFile::ReadUser  | QFile::WriteUser | QFile::ExeUser  |
                             QFile::ReadOther | QFile::ReadOther | QFile::ExeOther |
                             QFile::ExeOwner );

    // close file
    FlashFile.close();

    //********** Script-File ausfuehren **********/
    // Execute Scriptfile
    proc1->start(FlashFilePath);
    if (is_error == false){
        ui->cOutput->setTextColor(QColor(150,200,150));
        ui->cOutput->append("Flash OK!");
    }
    else
        is_error = false;

    //********** Script-File aufraeumen **********/
    //if(FlashFile.exists())
    //    FlashFile.remove();

}

void MainWindow::FlashFuses() {
    qDebug() << "Flashing Fuses" << endl;
    QString LF = ui->lLF->text();
    uint8_t lf = LF.toInt();
    QString HF = ui->lHF->text();
    uint8_t hf = HF.toInt();
    QString EF = ui->lEF->text();
    uint8_t ef = EF.toInt();

    // open qmessage box (warning)
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Flash Fuses",
                                  "Flash?",
                                   QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qDebug() << "Yes was clicked";

        // create strings to send
        qDebug() << "uC: " << uc << " programer: " << fd << endl;
        QString avrdude = "/usr/local/Cellar/avrdude/6.3/bin/avrdude " ;
        QString s1 = "-U lfuse:w:";
        QString s2 = "-U hfuse:w:";
        QString s3 = "-U efuse:w:";
        QString s4 = ":m ";
        // append all together
        avrdude.append(fd).append(" ").append(uc).append(" ").
                append(s1).append(LF).append(s4).
                append(s2).append(HF).append(s4).
                append(s3).append(EF).append(s4);
        qDebug() << avrdude << endl;

        //********** Script-File erstellen **********/
        // Edit Build.sh
        // Create SHELL Files
        FFlashFilePath = d.path().append("/FFlash.sh");
        FFlashFile.setFileName(FFlashFilePath);
        if (FFlashFile.open(QIODevice::ReadWrite)){
            QTextStream stream( &FFlashFile );
            stream << "#!/bin/bash \n" << endl;
            stream << "cd " << Workingdir << "\n" << endl;
            stream << avrdude << "\n" << endl;
        }
        // set file permissions to executeable
        FFlashFile.setPermissions(QFile::ReadGroup | QFile::ExeGroup  |
                                 QFile::ReadUser  | QFile::WriteUser | QFile::ExeUser  |
                                 QFile::ReadOther | QFile::ReadOther | QFile::ExeOther |
                                 QFile::ExeOwner );

        // close file
        FFlashFile.close();

        //********** Script-File ausfuehren **********/
        // Execute Scriptfile
        proc1->start(FFlashFilePath);
        if (is_error == false){
            ui->cOutput->setTextColor(QColor(150,200,150));
            ui->cOutput->append("Fuse Flash OK!");
        }
        else
            is_error = false;

        //********** Script-File aufraeumen **********/
        //if(FlashFile.exists())
        //    FlashFile.remove();


      } else {
        qDebug() << "Yes was *not* clicked";
      }

}

void MainWindow::Run(){
    Build();
    Flash();
}

void MainWindow::DefineUC()
{
    uint8_t curInd = ui->cbController->currentIndex();
    switch (curInd){
        case 0: uc = "-p m32"; break;
        case 1: uc = "-p m328"; break;
        case 2: uc = "-p t2313"; break;
        case 3: uc = "-p t4313"; break;
        default: qDebug("something went wrong \n"); break;
    }
}

void MainWindow::DefineFD()
{
    uint8_t curInd = ui->cbFlashtool->currentIndex();
    switch (curInd){
        case 0: fd = "-c avrisp2"; break;
        default: qDebug("something went wrong \n"); break;
    }
}

void MainWindow::rightMessage()
{
    QByteArray message = proc1->readAllStandardOutput();
    qDebug() << "shell answer: " << message<< endl;
    ui->cOutput->setTextColor(QColor(0,0,0));
    ui->cOutput->append(message);
}

void MainWindow::errorMessage()
{
    is_error = true;
    QByteArray error = proc1->readAllStandardError();
    qDebug() << "shell error: "<< error << endl;
    if(error.startsWith("avrdude")||error.startsWith("\nReading")||error.startsWith("#")||error.startsWith("\navrdude"))
        ui->cOutput->setTextColor(QColor(0,0,0));
    else
        ui->cOutput->setTextColor(QColor(0,0,0));

    if(!error.endsWith("\n")){
        QTextCursor cursor = ui->cOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(error);
    }
    else
        ui->cOutput->append(error);
}
