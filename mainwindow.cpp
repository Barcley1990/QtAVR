#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projectsettings.h"
#include "templateparser.h"

#include <QSettings>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>

#include <QDirModel>
#include <QVariant>

#include <QFile>

// TODO:
// lernfaehiger Autocompleter
// Eventuell Snippets in den Completer einbauen (for(), while(), etc.)
// Bei AddExistingFile wird die Datei zwar in die Liste hinzugefuegt aber die entsprechende Datei nicht ins Projektverzeichnis kopiert.
// Dateien sollten aus dem Projektverzeichnis auch wieder entfernt werden können.
// Aendern von Dateinamen zulassen
// Die Ausgabe in der Konsole colorieren (Warnungen gelb oder Errors rot etc.)
// Neuer Tab mit "CMD + T"
// Dateien mit Doppelklick aus Filelist oeffnen
// Reverse Backspace ermöglichen "Shift + Backspace"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create QProcess object
    TerminalProcess = new QProcess();
    // Set Window Icon
    setWindowIcon(QIcon(":/images/images/chip03_small.png"));
    // Set Window Size in Startup
    this->setWindowState(Qt::WindowMaximized);
    // Load XML-Files
    loadXMLFiles();
    // Settings Dialog
    userSettings = new Settings();
    if(userSettings->load()){
        // Settings successfully loaded
    }else{
        // TODO: There are no user settings, maybe show a welcome screen or a "first-steps" instruction
    }
    // Load layout
    restoreGeometry(userSettings->getGeometry());
    restoreState(userSettings->getWindowState());
    // Create Dialog to set the uCs Fuses
    fuseSettings = new FuseDialog();
    // show default file path at start on statusbar
    p.Workingdir= QDir::homePath();
    ui->statusBar->showMessage(p.Workingdir);
    /* show output */
    connect(TerminalProcess, SIGNAL(readyReadStandardOutput()),this, SLOT(rightMessage()) );
    connect(TerminalProcess, SIGNAL(readyReadStandardError()), this, SLOT(errorMessage()) );
    /* buttons */
    connect(ui->bBuild, SIGNAL(clicked()), this, SLOT(Build()));
    connect(ui->bFlash, SIGNAL(clicked()), this, SLOT(Flash()));
    connect(ui->bRun, SIGNAL(clicked()), this, SLOT(Run()));
    // Get selected tab of main tabwidget
    connect(ui->twMainTab, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    // delete initialized tabs
    ui->twMainTab->removeTab(0);
    ui->twMainTab->removeTab(0);
    /*-------------------------------------------------------------------------------*/


    // Set Workingtree in TreeView Widget
    QStringList headers;
    headers << tr("Title");
    // New Tree Model with parent "Project" on Top Level (0,0)
    model = new TreeModel(headers, "Project");
    indexLevel0 = new QModelIndex();
    indexLevel1 = new QModelIndex();
    ui->treeView->setModel( model );
    //connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);

    // insert childitems for header and source files at parent "Project"
    // New items wil be insert from bottom to top. To insert a new item you first have to
    // select the parent item.
    // After this construction, the parent will be "index".
    *indexLevel0 = model->index(posTop,0);
    ui->treeView->selectionModel()->setCurrentIndex(*indexLevel0,QItemSelectionModel::Select);
    insertChild("Sources");
    *indexLevel0 = model->index(posTop,0);
    ui->treeView->selectionModel()->setCurrentIndex(*indexLevel0,QItemSelectionModel::Select);
    insertChild("Headers");


    // Accept Drag&Drop
    this->setAcceptDrops(true);
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
    // Save current layout
    userSettings->setGeometry(saveGeometry());
    userSettings->setWindowState(saveState());

    event->ignore();
    if(qtavr == NULL)
        event->accept();
    else{
        // check if there are any unsaved files
        // get number of opened tabs
        int numberOfTabs = ui->twMainTab->count()-1;
        bool unsavedFiles = false;
        while(numberOfTabs>=0){
            QString currentText = ui->twMainTab->tabBar()->tabText(numberOfTabs);
            if(currentText.contains("*", Qt::CaseSensitive) != 0){
                unsavedFiles = true;
            }
            numberOfTabs--;
        }
        if(unsavedFiles){
            QMessageBox question;
            question.setText("There are unsaved Files \n\n Exit anyway?");
            question.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::SaveAll);
            question.show();
            int answer = question.exec();
            if(answer == QMessageBox::Yes) {
                event->accept();
            }
            else if(answer == QMessageBox::SaveAll) {
                saveProject();
                event->accept();
            }
        }
        else
        {
            saveProject();
            event->accept();
        }
    }
}
// Drag and Drop Eventfilter
void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    qDebug() << "dragEnterEvent in MainWindow";
    if (event->mimeData()->hasText()){
        event->accept();
        qDebug() << "dragEnterEvent acceptet";
    }else {
        event->ignore();
        qDebug() << "dragEnterEvent not acceptet";
    }
}

void MainWindow::dropEvent(QDropEvent *event){
    qDebug() << "dropEvent in MainWindow";
    foreach (const QUrl &url, event->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        qDebug() << "Dropped file:" << fileName;
        addDroppedFile(fileName);
    }
}
// Load XML-Files
void MainWindow::loadXMLFiles()
{
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
}

/**
 * Checks if there are any unsaved files in the current project
 * @brief MainWindow::unsavedFiles
 * @return True if any file modifications are not saved
 */
bool MainWindow::unsavedFiles(){
    int i;
    for(i=0; i<ui->twMainTab->count(); i++){
        Editor* e = (Editor*)ui->twMainTab->widget(i);
        if(e->isSaved() == false){
            return true;
        }
    }
    return false;
}

void MainWindow::reloadFileList()
{
    // TODO: Sort in groups of C and H or in alphabetic order...
    // set on label
    ui->cCfiles->clear();
    for(uint8_t i=0; i<p.cFileNames.length(); i++){
        ui->cCfiles->append(p.cFileNames[i]);
    }
    for(uint8_t i=0; i<p.hFileNames.length(); i++){
        ui->cCfiles->append(p.hFileNames[i]);
    }
}

void MainWindow::saveAllFiles()
{
   // for(int i=0; i<ui->twMainTab->count(); i++){
   //     Editor* e = (Editor*)ui->twMainTab->widget(i);
   //     e->saveContent();
   // }
    qDebug() << "Save all Files" << endl;
    int numberOfTabs = ui->twMainTab->count()-1;
    while(numberOfTabs>=0){
        Editor *editor = (Editor*)(ui->twMainTab->widget(numberOfTabs));
        if(editor->saveContent()){
            // Remove the '*' character after saving file
            QString currentText = ui->twMainTab->tabBar()->tabText(numberOfTabs);
            if(currentText.contains("*", Qt::CaseSensitive) != 0){
                currentText.remove('*');
                ui->twMainTab->tabBar()->setTabText(numberOfTabs, currentText);
            }
        }else{
            // TODO: Any error occurred while saving file!
        }
        numberOfTabs--;
    }
}

void MainWindow::saveProject()
{
    saveAllFiles();

    if(qtavr != NULL)
    {
        qtavr->setValue("project.cfiles", p.cFileNames);
        qtavr->setValue("project.hfiles", p.hFileNames);
        qtavr->setValue("project.wdir", p.Workingdir);
    }
}

void MainWindow::reloadProjectSetting()
{
    qDebug() << "reloadProjectSetting";
    if(qtavr != NULL){
        currentProcessorAvrdudeCommand = processorAvrdudeCommands.at(qtavr->value("project.controller").toInt());
        currentProcessorGccCommand = processorGccCommands.at(qtavr->value("project.controller").toInt());
        currentProgrammerAvrdudeCommand = programmerAvrdudeCommands.at(qtavr->value("project.programmer").toInt());
    }
    qDebug() << "   currentProcessorAvrdudeCommand: " << currentProcessorAvrdudeCommand;
    qDebug() << "   currentProcessorGccCommand: " << currentProcessorGccCommand;
    qDebug() << "   currentProgrammerAvrdudeCommand: " << currentProgrammerAvrdudeCommand;
}

void MainWindow::addDroppedFile(QString file){
    qDebug() << "Add Dropped File" << endl;
    QString selectedFilter = "";

    if(selectedFilter.contains("*.c")){
        selectedFilter = "c";
    }else if(selectedFilter.contains("*.h")){
        selectedFilter = "h";
    }

    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        if(suffix.contains("c") || suffix.contains("C")){
            // Append a new C file to the list
            // append filepath/name to list
            p.cFileNames.append(filename);
        }else{
            // Append a new H file to the list
            // append filepath/name to list
            p.hFileNames.append(filename);
        }

        reloadFileList();

        // New File in tab-bar
        Editor* e = new Editor(this, filepath, filename);
        e->setSettings(userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, filename);
        ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionSave_as->setEnabled(true);

        saveProject();
    }
}


/**
 * Generates the project makefile according to the template and the qtavr project settings file
 * @brief MainWindow::generateMakefile
 */
void MainWindow::generateMakefile()
{
    qDebug() << "generateMakefile()" << endl;

    if(qtavr != NULL){
        QString makefilePath = p.Workingdir + "/Makefile";
        QFile makefile(makefilePath);

        if (makefile.open(QIODevice::ReadWrite)){

            QFile makefileTemplate(":/templates/templates/default_makefile.txt");
            if (makefileTemplate.open(QIODevice::ReadOnly)){
                TemplateParser* parser = new TemplateParser(this->qtavr, this->userSettings);

                QTextStream in(&makefileTemplate);
                while (!in.atEnd()){
                    makefile.write(parser->getParsedLine(in.readLine()).toLatin1());
                    makefile.write("\r\n");
                }
                makefileTemplate.close();

                delete parser;
            }

            // close file
            makefile.close();
        }
    }
}
// Close open Tabwindow
void MainWindow::closeTab(int index) {
    Editor* e = (Editor*)(ui->twMainTab->widget(index));

    if(e->isSaved()){
        // File is saved, close tab
        ui->twMainTab->removeTab(index);
        delete e;
    }else{
        // File unsaved, show dialog
        QMessageBox question;
        question.setText("There are unsaved Files \n\n Close anyway?");
        question.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Save);
        question.show();
        int answer = question.exec();
        if(answer == QMessageBox::Yes) {
            ui->twMainTab->removeTab(index);
            delete e;
        }else if(answer == QMessageBox::Save) {
            if(e->saveContent()){
                // Successfully saved file
                ui->twMainTab->removeTab(index);
                delete e;
            }else{
                // Error while saving file
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Error while saving file!");
                messageBox.setFixedSize(500,200);
            }
        }
        question.accept();
    }
}
// Build Project
void MainWindow::Build()
{
    qDebug() << "Build" << endl;

    // Clear Console
    ui->cOutput->clear();
    // Save all files
    saveAllFiles();
    // Generate a new makefile
    generateMakefile();

    TerminalProcess->start("make -C " + qtavr->value("project.wdir").toString());
    if (is_error == false){

    }else{
        is_error = false;
    }

    /*
    on_actionSave_All_triggered();
    // Strings für avr-gcc vorbereiten
    QString compile = userSettings->getToolchainRootPath() + "/bin/avr-gcc -g -Os -mmcu=" + currentProcessorGccCommand + " -c ";
    QString link    = userSettings->getToolchainRootPath() + "/bin/avr-gcc -g -mmcu=" + currentProcessorGccCommand + " -o ";
    QString hex     = userSettings->getToolchainRootPath() + "/bin/avr-objcopy -j .text -j .data -O ihex ";

    // create o-files
    p.oFileNames.clear();
    for(volatile uint8_t i=0; i<p.cFileNames.length(); i++){
        QString fn = p.cFileNames[i];
        int dot = fn.indexOf(".")+1;
        p.oFileNames.append(fn.replace(dot,1,"o"));
    }
    // Compile
    for(uint8_t i=0; i<p.cFileNames.length(); i++){
        compile.append(p.cFileNames[i]);
        compile.append(" ");
    }
    //Link object files and create .ELF-file
    link.append("main.elf ");
    for(uint8_t i=0; i<p.oFileNames.length(); i++){
        link.append(p.oFileNames[i]);
        link.append(" ");
    }
    // create HEX from ELF file
    hex.append("main.elf main.hex");

    qDebug() << "compile: " << compile << endl;
    qDebug() << "link: "   << link    << endl;
    qDebug() << "hex: "    << hex     << endl;

    // Script-File erstellen
    // Edit Build.sh
    // Create SHELL Files
    BuildFilePath = (p.Workingdir + "/Build.sh");
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

    // Script-File ausfuehren
    // Execute Scriptfile

    TerminalProcess->start(BuildFilePath);
    if (is_error == false){        
        ui->cOutput->setTextColor(QColor(0,255,0));
        ui->cOutput->setText("Build OK!");
    }
    else
        is_error = false;

    // Script-File aufraeumen
    //if(BuildFile.exists())
    //    BuildFile.remove();
    */
}
// Flash Project
void MainWindow::Flash()
{
    // Clear Console
    ui->cOutput->clear();
    // TODO: Add the right HEX file, instead of using 'main.hex'
    qDebug() << "Flash"<< endl;
    qDebug() << "currentProcessorAvrdudeCommand: " << currentProcessorAvrdudeCommand << " currentProgrammerAvrdudeCommand: " << currentProgrammerAvrdudeCommand << endl;
    qDebug() << "project.wdir: " << qtavr->value("project.wdir").toString();

    TerminalProcess->start("make flash -C " + qtavr->value("project.wdir").toString());
    if (is_error == false){

    }else{
        is_error = false;
    }

    /*
    QString avrdude = userSettings->getAvrdudePath() + "/bin/avrdude " ;
    QString write = " -U flash:w:main.hex";
    avrdude += "-p " + currentProcessorAvrdudeCommand + " -c " + currentProgrammerAvrdudeCommand + write;
    qDebug() << avrdude << endl;

    // Script-File erstellen
    // Edit Build.sh
    // Create SHELL Files
    FlashFilePath = (p.Workingdir + "/Flash.sh");
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

    // Script-File ausfuehren
    // Execute Scriptfile
    TerminalProcess->start(FlashFilePath);
    if (is_error == false){
        ui->cOutput->setTextColor(QColor(150,200,150));
        ui->cOutput->append("Flash OK!");
    }
    else
        is_error = false;

    // Script-File aufraeumen
    //if(FlashFile.exists())
    //    FlashFile.remove();
    */
}
// Run Project
void MainWindow::Run(){
    Build();
    Flash();
}

/* Action Bar */
// Create New Project
void MainWindow::on_actionNew_Project_triggered(){
    qDebug() << "Create new project" << endl;

    QString file = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    p.Workingdir,
                                                    "Project (*.qtavr)"
                                                    );
    if(file.length() > 0){

        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString filenameExSuffix = QFileInfo(file).baseName();

        // New main C file in as Editor instance
        QString mainFilename = filenameExSuffix+".c";
        Editor* e = new Editor(this, filepath, mainFilename, true);
        e->setSettings(userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, mainFilename);
        // get actual working dir
        p.Workingdir = filepath;
        ui->statusBar->showMessage(p.Workingdir);

        //  Create Filelist
        p.cFileNames.clear();
        // append filepath/name to list
        p.cFileNames.append(mainFilename);

        reloadFileList();

        // new Project File
        // ToDo: Get filename exculuding suffix including filepath
        qtavr = new QSettings(filepath + "/" + filenameExSuffix + ".qtavr", QSettings::NativeFormat);
        qtavr->setValue("project.target", filenameExSuffix);

        saveProject();

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
// Open Existing Project
void MainWindow::on_actionOpen_Project_triggered()
{
    qDebug() << "Open Existing Project" << endl;
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                p.Workingdir,
                                                tr("Project (*.qtavr)")
                                                );

    QFile tempFile(file);
    if(tempFile.exists()){
        // TODO: Check if the file and suffix is correct
        //QString projectFile = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        filepath += "/";

        // Load project settings file and get string lists
        qtavr = new QSettings(file, QSettings::NativeFormat);
        // TODO: Check if it is correct not to set a default setting here
        //ui->cbController->setCurrentIndex(qtavr->value("project.controller").toInt());
        //ui->cbFlashtool->setCurrentIndex(qtavr->value("project.programmer").toInt());

        p.cFileNames = qtavr->value("project.cfiles").toStringList();
        p.hFileNames = qtavr->value("project.hfiles").toStringList();
        p.Workingdir = qtavr->value("project.wdir").toString();
        ui->statusBar->showMessage(p.Workingdir);
        // Insert all files
        // open c-files
        for(uint8_t i=0; i<p.cFileNames.length(); i++){
            ui->cCfiles->append(p.cFileNames[i]);

            // New File in tab-bar
            Editor* e = new Editor(this, filepath, p.cFileNames[i]);
            e->setSettings(this->userSettings);
            connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
            ui->twMainTab->addTab(e, p.cFileNames[i]);
            ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);

            // add Files to TreeView
            *indexLevel1 = model->index(posSources,0, *indexLevel0);
            ui->treeView->selectionModel()->setCurrentIndex(*indexLevel1,QItemSelectionModel::Select);
            insertChild(p.cFileNames[i]);
        }
        // open h-files
        for(uint8_t i=0; i<p.hFileNames.length(); i++){
            ui->cCfiles->append(p.hFileNames[i]);

            // New File in tab-bar
            Editor* e = new Editor(this, filepath, p.hFileNames[i]);
            e->setSettings(this->userSettings);
            connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
            ui->twMainTab->addTab(e, p.hFileNames[i]);
            ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);

            // add Files to TreeView
            *indexLevel1 = model->index(posHeaders,0, *indexLevel0);
            ui->treeView->selectionModel()->setCurrentIndex(*indexLevel1,QItemSelectionModel::Select);
            insertChild(p.hFileNames[i]);
        }

        // Enable Action- and other buttons (Add-File)
        ui->actionClose_Project->setEnabled(true);
        ui->actionNew_File->setEnabled(true);
        ui->actionExisting_File->setEnabled(true);
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionSave_as->setEnabled(true);
        ui->actionBuild->setEnabled(true);
        ui->actionFlash->setEnabled(true);
        ui->actionRun->setEnabled(true);
        ui->bBuild->setEnabled(true);
        ui->bFlash->setEnabled(true);
        ui->bRun->setEnabled(true);

        reloadProjectSetting();

        // Save all files and the project at the the end of open all files.
        saveAllFiles();

        // jump to first Tab
        ui->twMainTab->setCurrentIndex(0);



    }else{
        // Project file does not exist
        qDebug() << "ERROR WHILE LOADING PROJECT FILE!!!";
    }
}
void MainWindow::on_actionClose_Project_triggered()
{
    p.cFileNames.clear();
    p.hFileNames.clear();
    p.oFileNames.clear();
    p.DeviceName = "";
    p.ProgrammerName = "";
    p.Workingdir = "";
    ui->cCfiles->clear();
    ui->actionClose_Project->setEnabled(false);
}
// Save Active File
void MainWindow::on_actionSave_triggered(){
    qDebug() << "Save File" << endl;
    curTabIndex = ui->twMainTab->currentIndex();
    qDebug() << "current selected Tab: " << curTabIndex << endl;
    Editor *editor = (Editor*)(ui->twMainTab->widget(curTabIndex));
    if(editor->saveContent()){
        // Remove the '*' character after saving file
        QString currentText = ui->twMainTab->tabBar()->tabText(ui->twMainTab->currentIndex());
        if(currentText.contains("*", Qt::CaseSensitive) != 0){
            currentText.remove('*');
            ui->twMainTab->tabBar()->setTabText(ui->twMainTab->currentIndex(), currentText);
        }
    }else{
        // TODO: Any error occurred while saving file!
    }
}
// Save All Files
void MainWindow::on_actionSave_All_triggered()
{
    saveAllFiles();
}
// Save File at specified path
void MainWindow::on_actionSave_as_triggered()
{
    QString file = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    p.Workingdir,
                                                    "c-Files (*.c)"
                                                    );
    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();


    }
}
// Build Project
void MainWindow::on_actionBuild_triggered(){
    Build();
}
// Flash Project
void MainWindow::on_actionFlash_triggered(){
    Flash();
}
// Build and Flash Project
void MainWindow::on_actionRun_triggered()
{
    Run();
}
// Show About Prompt
void MainWindow::on_actionAbout_triggered(){
    QMessageBox::about(this, tr("About QtAVR"),
        tr("<h2>QtAVR</h2>"
        "<p>Copyright &copy; 2017 Tobias Nuss, Dennis Greguhn"
        "<p>Crude Build and Flash Tool for AVR Microcontroller"));
}
//
void MainWindow::on_actionOpen_Settings_triggered() {
    if(userSettings->exec()){
        int i;
        for(i=0; i<ui->twMainTab->count(); i++){
            Editor* e = (Editor*)ui->twMainTab->widget(i);
            e->reloadSettings();
        }
    }

}

// Add New File
void MainWindow::on_actionNew_File_triggered()
{
    qDebug() << "Add File" << endl;
    QString selectedFilter = "";
    QString file = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    p.Workingdir,
                                                    tr("source (*.c);;header (*.h)"),
                                                    &selectedFilter
                                                    );
    if(selectedFilter.contains("*.c")){
        selectedFilter = "c";
    }else if(selectedFilter.contains("*.h")){
        selectedFilter = "h";
    }

    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        // Add the selected file filter
        if(suffix.length() == 0){
            suffix = selectedFilter;
            filename += "."+suffix;
            filepathname += "."+suffix;
        }

        Editor* e = new Editor(this, filepath, filename);
        e->setSettings(userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, filename);
        ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);

        // New File in tab-bar
        if(suffix.compare("c", Qt::CaseInsensitive) == 0){
            // append filepath/name to list
            p.cFileNames.append(filename);
        }else if(suffix.compare("h", Qt::CaseInsensitive) == 0){
            // append filepath/name to list
            p.hFileNames.append(filename);
        }else{
            qDebug() << "Error: Unknows Filetype" << endl;
        }
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionSave_as->setEnabled(true);

        reloadFileList();
        saveProject();
    }
}
// Add Existing File
void MainWindow::on_actionExisting_File_triggered()
{
    qDebug() << "Add Existing File" << endl;
    QString selectedFilter = "";
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                p.Workingdir,
                                                tr("source (*.c);;header (*.h)"),
                                                &selectedFilter
                                                );

    if(selectedFilter.contains("*.c")){
        selectedFilter = "c";
    }else if(selectedFilter.contains("*.h")){
        selectedFilter = "h";
    }

    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        if(suffix.contains("c") || suffix.contains("C")){
            // Append a new C file to the list
            // append filepath/name to list
            p.cFileNames.append(filename);
        }else{
            // Append a new H file to the list
            // append filepath/name to list
            p.hFileNames.append(filename);
        }

        reloadFileList();

        // New File in tab-bar
        Editor* e = new Editor(this, filepath, filename);
        e->setSettings(userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, filename);
        ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionSave_as->setEnabled(true);

        saveProject();
    }
}

// Just open an existing file
void MainWindow::on_actionFile_triggered()
{
    qDebug() << "Open Existing File" << endl;
    QString selectedFilter = "";
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                p.Workingdir,
                                                tr("source (*.c);;header (*.h);;text (*.txt)"),
                                                &selectedFilter
                                                );
    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        // New File in tab-bar
        Editor* e = new Editor(this, filepath, filename);
        e->setSettings(this->userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, filename);
        ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionSave_as->setEnabled(true);

    }
}
// Open Prompt for setting the uC's Fuses
void MainWindow::on_actionFuses_triggered()
{
    qDebug() << "Set Fuses" << endl;

    if(fuseSettings->exec()){

    }

}

void MainWindow::on_fileChanged(QString filename)
{
    // TODO: Parameter currently not used, check if really needed or remove.
    QString currentText = ui->twMainTab->tabBar()->tabText(ui->twMainTab->currentIndex());
    // Only add a '*' to the end of the current file, if there is no '*' already appended
    if(currentText.contains("*", Qt::CaseSensitive) == 0){
        ui->twMainTab->tabBar()->setTabText(ui->twMainTab->currentIndex(), currentText+"*");
    }
}

void MainWindow::on_dockWidgetWorktree_visibilityChanged(bool visible)
{
    ui->actionViewWorktree->setChecked(visible);
}

void MainWindow::on_actionViewWorktree_toggled(bool arg1)
{
     ui->dockWidgetWorktree->setVisible(arg1);
}

void MainWindow::on_actionViewFile_toggled(bool arg1)
{
    ui->dockWidgetFile->setVisible(arg1);
}

void MainWindow::on_actionViewFileList_toggled(bool arg1)
{
    ui->dockWidgetFileList->setVisible(arg1);
}

void MainWindow::on_actionViewConsole_toggled(bool arg1)
{
    ui->dockWidgetConsole->setVisible(arg1);
}

void MainWindow::on_dockWidgetFile_visibilityChanged(bool visible)
{
    ui->actionViewFile->setChecked(visible);
}

void MainWindow::on_dockWidgetFileList_visibilityChanged(bool visible)
{
    ui->actionViewFileList->setChecked(visible);
}

void MainWindow::on_dockWidgetConsole_visibilityChanged(bool visible)
{
    ui->actionViewConsole->setChecked(visible);
}

void MainWindow::on_actionDefault_View_triggered()
{
    // TODO: Reset views and layout
    ui->dockWidgetConsole->setHidden(false);
    ui->dockWidgetFile->setHidden(false);
    ui->dockWidgetFileList->setHidden(false);
    ui->dockWidgetWorktree->setHidden(false);
    ui->dockWidgetConsole->setFloating(false);
    ui->dockWidgetFile->setFloating(false);
    ui->dockWidgetFileList->setFloating(false);
    ui->dockWidgetWorktree->setFloating(false);
}
// Open project specific settings
void MainWindow::on_actionProject_Settings_triggered()
{
    if(qtavr != NULL){
        ProjectSettings* settings = new ProjectSettings(this, qtavr);
        settings->exec();
        delete settings;
        reloadProjectSetting();
    }
}

// Console output
void MainWindow::rightMessage()
{
    QByteArray message = TerminalProcess->readAllStandardOutput();
    qDebug() << "shell answer: " << message<< endl;
    //ui->cOutput->setTextColor(QColor(0,0,0));
    ui->cOutput->append(message);
}

void MainWindow::errorMessage()
{
    is_error = true;
    QByteArray error = TerminalProcess->readAllStandardError();
    qDebug() << "shell error: "<< error << endl;
    //if(error.startsWith("avrdude")||error.startsWith("\nReading")||error.startsWith("#")||error.startsWith("\navrdude"))
        //ui->cOutput->setTextColor(QColor(0,0,0));
    //else
        //ui->cOutput->setTextColor(QColor(0,0,0));

    if(!error.endsWith("\n")){
        QTextCursor cursor = ui->cOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(error);
    }
    else
        ui->cOutput->append(error);
}

/**********************************/
// Treeview Functions
/**********************************/
void MainWindow::insertChild(const QString &data)
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant(data), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    ui->treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}

bool MainWindow::insertColumn()
{
    QAbstractItemModel *model = ui->treeView->model();
    int column = ui->treeView->selectionModel()->currentIndex().column();

    // Insert a column in the parent item.
    bool changed = model->insertColumn(column + 1);
    if (changed)
        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);

    updateActions();

    return changed;
}

void MainWindow::insertRow(const QString &data)
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant(data), Qt::EditRole);
    }
}

bool MainWindow::removeColumn()
{
    QAbstractItemModel *model = ui->treeView->model();
    int column = ui->treeView->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
    bool changed = model->removeColumn(column);

    if (changed)
        updateActions();

    return changed;
}

void MainWindow::removeRow()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}

void MainWindow::updateActions()
{
    bool hasSelection = !ui->treeView->selectionModel()->selection().isEmpty();

    //removeRowAction->setEnabled(hasSelection);
    //removeColumnAction->setEnabled(hasSelection);

    bool hasCurrent = ui->treeView->selectionModel()->currentIndex().isValid();
    //insertRowAction->setEnabled(hasCurrent);
    //insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent) {
        ui->treeView->closePersistentEditor(ui->treeView->selectionModel()->currentIndex());

        int row = ui->treeView->selectionModel()->currentIndex().row();
        int column = ui->treeView->selectionModel()->currentIndex().column();
        if (ui->treeView->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
}


void MainWindow::on_actionsetRow_triggered()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant("data"), Qt::EditRole);
    }
}

void MainWindow::on_actionsetChild_triggered()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("data"), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    ui->treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}
