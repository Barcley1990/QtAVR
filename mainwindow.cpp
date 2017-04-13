#include "mainwindow.h"
#include "ui_mainwindow.h"


// TODO:
// Dateipfade aendern.
// Comboboxen schreiben
// Eigene Klasse fue Editor
// Checken ob ungespeicherte Dateien vorhanden sind und im closeHandle abfragen.



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // create QProcess object
    proc1 = new QProcess();
    // Set Window Icon
    setWindowIcon(QIcon(":/images/images/chip03_small.png"));
    // Set Window Size in Startup
    this->setWindowState(Qt::WindowMaximized);
    // Settings Dialog
    userSettings = new Settings();
    if(userSettings->load()){
        // Settings successfully loaded
    }else{
        // TODO: There are no user settings, maybe show a welcome screen or a "first-steps" instruction
    }
    // Fuses Dialog
    fuseSettings = new FuseDialog();

    // show default file path at start on statusbar
    p.Workingdir= "/Users/tobias/Desktop/";
    ui->statusBar->showMessage(p.Workingdir);

    /* show output */
    connect(proc1, SIGNAL(readyReadStandardOutput()),this, SLOT(rightMessage()) );
    connect(proc1, SIGNAL(readyReadStandardError()), this, SLOT(errorMessage()) );
    /* buttons */
    connect(ui->bBuild, SIGNAL(clicked()), this, SLOT(Build()));
    connect(ui->bFlash, SIGNAL(clicked()), this, SLOT(Flash()));
    connect(ui->bRun, SIGNAL(clicked()), this, SLOT(Run()));
    connect(ui->cbController, SIGNAL(currentIndexChanged(int)), this, SLOT(DefineUC()));
    connect(ui->cbFlashtool, SIGNAL(currentIndexChanged(int)), this, SLOT(DefineFD()));
    // Get selected tab of main tabwidget
    connect(ui->twMainTab, SIGNAL(currentChanged(curTabIndex)), this, SLOT());
    // delete initialized tabs
    ui->twMainTab->removeTab(0);
    ui->twMainTab->removeTab(0);
    /*-------------------------------------------------------------------------------*/

    QDomDocument doc;
    QFile processorsFile(":/xml/xml/processors.xml");
    if (!processorsFile.open(QIODevice::ReadOnly) || !doc.setContent(&processorsFile)){
        // TODO: ERROR!
    }
    QDomNodeList cpus = doc.elementsByTagName("processor");
    for (int i = 0; i < cpus.size(); i++) {
        QDomNode n = cpus.item(i);
        QDomElement name = n.firstChildElement("name");
        QDomElement avrdude = n.firstChildElement("avrdude");
        QDomElement gcc = n.firstChildElement("gcc");
        if(name.isNull() == false && avrdude.isNull() == false){
            QString cpuName = name.firstChild().nodeValue();
            QString cpuCommand = avrdude.firstChild().nodeValue();
            qDebug() << "Loaded CPU: " << cpuName << " with command: " << cpuCommand;
            processors += cpuName;
            processorAvrdudeCommands += cpuCommand;
        }
        // Add the AVR-GCC parameters
        if(gcc.isNull() == false && gcc.firstChild().nodeValue().length()>0){
            // Parameter available, processor supported
            processorGccCommands += gcc.firstChild().nodeValue();
        }else{
            // TODO: Processor currently not supported
            processorGccCommands += "";
        }
    }
    // Add all items to the ComboBox
    ui->cbController->addItems(processors);
    // Select ATmega32 as default
    if(ui->cbController->count() >= 46){
        ui->cbController->setCurrentIndex(45);
    }
    QFile programmersFile(":/xml/xml/programmers.xml");
    if (!programmersFile.open(QIODevice::ReadOnly) || !doc.setContent(&programmersFile)){
        // TODO: ERROR!
    }
    QDomNodeList progs = doc.elementsByTagName("programmer");
    for (int i = 0; i < progs.size(); i++) {
        QDomNode n = progs.item(i);
        QDomElement name = n.firstChildElement("name");
        QDomElement avrdude = n.firstChildElement("avrdude");
        if (name.isNull() == false && avrdude.isNull() == false){
            QString progName = name.firstChild().nodeValue();
            QString progCommand = avrdude.firstChild().nodeValue();
            qDebug() << "Loaded programmer: " << progName << " with command: " << progCommand;
            programmers += progName;
            programmerAvrdudeCommands += progCommand;
        }
    }
    // Add all items to the ComboBox
    ui->cbFlashtool->addItems(programmers);
    // Select AVR ISP mkII as default
    if(ui->cbFlashtool->count() >= 14){
        ui->cbFlashtool->setCurrentIndex(13);
    }


    // Set Workingtree in TreeView Widget
    model = new QDirModel();
    model->setReadOnly(false);
    model->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);
    ui->treeView->setModel( model );
    QModelIndex index = model->index(p.Workingdir);
    ui->treeView->expand(index);
    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
    ui->treeView->resizeColumnToContents(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    // delete Bashfiles
    if(BuildFile.exists())
        BuildFile.remove();
    if(FlashFile.exists())
        FlashFile.remove();
}

// show prompt when user wants to close app
void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    QMessageBox question;
    question.setText("There are unsaved Files \n\n Exit anyway?");
    question.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    question.show();
    if(question.exec() == QMessageBox::Yes) {
        event->accept();
    }
}

void MainWindow::Build()
{
    qDebug() << "Build" << endl;

    //********** Strings für avr-gcc vorbereiten **********/
    QString compile = userSettings->getToolchainRootPath() + "/bin/avr-gcc -g -Os -mmcu=" + currentProcessorGccCommand + " -c ";
    QString link    = userSettings->getToolchainRootPath() + "/bin/avr-gcc -g -mmcu=" + currentProcessorGccCommand + " -o ";
    QString hex     = userSettings->getToolchainRootPath() + "/bin/avr-objcopy -j .text -j .data -O ihex ";

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
    BuildFilePath = p.Workingdir.append("/Build.sh");
    BuildFile.setFileName(BuildFilePath);
    if (BuildFile.open(QIODevice::ReadWrite)){
        QTextStream stream( &BuildFile );
        stream << "#!/bin/bash \n" << endl;
        stream << "cd " << p.Workingdir << "\n" << endl;
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

    QString avrdude = userSettings->getAvrdudePath() + "/avrdude " ;
    QString write = "-U flash:w:main.hex";
    avrdude.append(fd).append(" ").append(uc).append(" ").append(write);
    qDebug() << avrdude << endl;

    //********** Script-File erstellen **********/
    // Edit Build.sh
    // Create SHELL Files
    FlashFilePath = p.Workingdir.append("/Flash.sh");
    FlashFile.setFileName(FlashFilePath);
    if (FlashFile.open(QIODevice::ReadWrite)){
        QTextStream stream( &FlashFile );
        stream << "#!/bin/bash \n" << endl;
        stream << "cd " << p.Workingdir << "\n" << endl;
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
        case 0:
            fd = "-c " + currentProgrammerAvrdudeCommand;
        break;
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


// Action Bar
void MainWindow::on_actionNew_Project_triggered(){
    qDebug() << "Create new project" << endl;

    QString file = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    p.Workingdir,
                                                    "c-Files (*.c)"
                                                    );
    if(file.length() > 0){

        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();

        // New File in tab-bar
        ui->twMainTab->addTab( new Editor(this, filepathname, 0), filename );
        // get actual working dir
        p.Workingdir = filepath;
        ui->statusBar->showMessage(p.Workingdir);

        //  Create Filelist (TODO: auslagern)
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


        // Enable Action- and other buttons (Add-File)
        ui->actionNew_File->setEnabled(true);
        ui->actionExisting_File->setEnabled(true);
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionBuild->setEnabled(true);
        ui->actionFlash->setEnabled(true);
        ui->actionRun->setEnabled(true);
        ui->bBuild->setEnabled(true);
        ui->bFlash->setEnabled(true);
        ui->bRun->setEnabled(true);
     }
}

void MainWindow::on_actionSave_triggered(){
    qDebug() << "Save File" << endl;
    curTabIndex = ui->twMainTab->currentIndex();
    qDebug() << "current selected Tab: " << curTabIndex << endl;
    Editor *editor = (Editor*)(ui->twMainTab->widget(curTabIndex));
    editor->saveContent();
}

void MainWindow::on_actionBuild_triggered(){
    Build();
}

void MainWindow::on_actionFlash_triggered(){
    Flash();
}

void MainWindow::on_actionRun_triggered()
{
    Run();
}

void MainWindow::on_actionAbout_triggered(){
    QMessageBox::about(this, tr("About QtAVR"),
        tr("<h2>QtAVR</h2>"
        "<p>Copyright &copy; 2017 Tobias Nuss, Dennis Greguhn"
        "<p>Crude Build and Flash Tool for AVR Microcontroller"));
}

void MainWindow::on_actionOpen_Settings_triggered() {
    if(userSettings->exec()){
        // TODO: Reload the server settings
    }
}

void MainWindow::on_cbController_currentIndexChanged(int index)
{
    qDebug() << QString::number(index) << "Selected: " << processors.at(index) << " with command: " << processorAvrdudeCommands.at(index);
    currentProcessorAvrdudeCommand = processorAvrdudeCommands.at(index);
    currentProcessorGccCommand = processorGccCommands.at(index);
    if(currentProcessorGccCommand.length() == 0){
        // TODO: Warning, this processor is currently not supported!
    }
}

void MainWindow::on_cbFlashtool_currentIndexChanged(int index)
{
    qDebug() << QString::number(index) << "Selected: " << programmers.at(index) << " with command: " << programmerAvrdudeCommands.at(index);
    currentProgrammerAvrdudeCommand = programmerAvrdudeCommands.at(index);
}

void MainWindow::on_actionNew_File_triggered()
{
    qDebug() << "Add File" << endl;
    QString file = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    p.Workingdir,
                                                    tr("source (*.c);;header (*.h)")
                                                    );
    if(file.length() > 0){

            QString filename        = QFileInfo(file).fileName();
            QString filepathname    = QFileInfo(file).filePath();
            QString filepath        = QFileInfo(file).path();

            // New File in tab-bar
            if(QFileInfo(file).suffix().compare("c", Qt::CaseInsensitive) == 0){
                ui->twMainTab->addTab( new Editor(this, filepathname, 1), filename );
                ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
            }else if(QFileInfo(file).suffix().compare("h", Qt::CaseInsensitive) == 0){
                ui->twMainTab->addTab( new Editor(this, filepathname, 2), filename );
                ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
            }else{
                qDebug() << "Error: Unknows Filetype" << endl;
            }
    }

}

void MainWindow::on_actionFuses_triggered()
{
    qDebug() << "Set Fuses" << endl;

    if(fuseSettings->exec()){

    }

}


