#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>


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

    populateComboBoxes();

    // Settings Dialog
    userSettings = new Settings();
    if(userSettings->load()){
        // Settings successfully loaded
    }else{
        // TODO: There are no user settings, maybe show a welcome screen or a "first-steps" instruction
    }

    // Load last seleted programmer and processor
    int temp = userSettings->getDefaultProgrammer();
    if(ui->cbFlashtool->count() > temp){
        ui->cbFlashtool->setCurrentIndex(temp);
    }
    temp = userSettings->getDefaultProcessor();
    if(ui->cbController->count() > temp){
        ui->cbController->setCurrentIndex(temp);
    }

    // Load layout
    restoreGeometry(userSettings->getGeometry());
    restoreState(userSettings->getWindowState());

    // Fuses Dialog
    fuseSettings = new FuseDialog();

    // show default file path at start on statusbar
    p.Workingdir= QDir::homePath();
    ui->statusBar->showMessage(p.Workingdir);

    /* show output */
    connect(proc1, SIGNAL(readyReadStandardOutput()),this, SLOT(rightMessage()) );
    connect(proc1, SIGNAL(readyReadStandardError()), this, SLOT(errorMessage()) );
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
    // Save current layout
    userSettings->setGeometry(saveGeometry());
    userSettings->setWindowState(saveState());
    // Save seleted programmer and processor
    userSettings->setDefaultProgrammer(ui->cbFlashtool->currentIndex());
    userSettings->setDefaultProcessor(ui->cbController->currentIndex());

    event->ignore();

    // Pop up dialog if there are unsaved files
    if(unsavedFiles() == true){
        QMessageBox question;
        question.setText("There are unsaved Files \n\n Exit anyway?");
        question.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::SaveAll);
        question.show();
        int answer = question.exec();
        if(answer == QMessageBox::Yes) {
            event->accept();
        }
        else if(answer == QMessageBox::SaveAll) {
            on_actionSave_All_triggered();
            qtavr->setValue("project.wdir", p.Workingdir);
            qtavr->setValue("project.cfiles", p.cFileNames);
            qtavr->setValue("project.hfiles", p.hFileNames);
            event->accept();
        }
    }
    else
    {
        qtavr->setValue("project.wdir", p.Workingdir);
        qtavr->setValue("project.cfiles", p.cFileNames);
        qtavr->setValue("project.hfiles", p.hFileNames);
        event->accept();
    }
}

void MainWindow::populateComboBoxes()
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
}

/**
 * Checks if there are any unsaved files in the current project
 * @brief MainWindow::unsavedFiles
 * @return True if any file modifications are not saved
 */
bool MainWindow::unsavedFiles()
{
    int i;
    for(i=0; i<ui->twMainTab->count(); i++){
        Editor* e = (Editor*)ui->twMainTab->widget(i);
        if(e->isSaved() == false){
            return true;
        }
    }
    return false;
}

// Close open Tabwindow
void MainWindow::closeTab(int index) {
    qDebug() << "Remove Tab: " << index << endl;
    ui->twMainTab->removeTab(index);
}
// Build Project
void MainWindow::Build()
{
    qDebug() << "Build" << endl;
    on_actionSave_All_triggered();
    //********** Strings für avr-gcc vorbereiten **********/
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
// Flash Project
void MainWindow::Flash()
{
    // TODO: Add the right HEX file, instead of using 'main.hex'
    qDebug() << "Flash"<< endl;
    qDebug() << "currentProcessorAvrdudeCommand: " << currentProcessorAvrdudeCommand << " currentProgrammerAvrdudeCommand: " << currentProgrammerAvrdudeCommand << endl;

    QString avrdude = userSettings->getAvrdudePath() + "/avrdude " ;
    QString write = " -U flash:w:main.hex";
    avrdude += "-p " + currentProcessorAvrdudeCommand + " -c " + currentProgrammerAvrdudeCommand + write;
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
// Run Project
void MainWindow::Run(){
    Build();
    Flash();
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


/* Action Bar */
// Create New Project
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
        QString filenameExSuffix = QFileInfo(file).baseName();

        // New File in tab-bar
        Editor* e = new Editor(this, file, true, true, 0);
        e->setSettings(userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, filename);
        // get actual working dir
        p.Workingdir = filepath;
        ui->statusBar->showMessage(p.Workingdir);

        //  Create Filelist
        p.cFileNames.clear();
        // append filepath/name to list
        p.cFileNames.append(filepathname);
        // set on label
        ui->cCfiles->clear();
        ui->cCfiles->append(QFileInfo(p.cFileNames[0]).fileName());

        // new Project File
        // ToDo: Get filename exculuding suffix including filepath
        qtavr = new QSettings(filepath + "/" + filenameExSuffix + ".qtavr", QSettings::NativeFormat);
        qtavr->setValue("project.cfiles", p.cFileNames);

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
                                                tr("project (*.qtavr)")
                                                );

    QString projectFile = QFileInfo(file).filePath();

    if(QFile(projectFile).exists()){
        qtavr = new QSettings(projectFile, QSettings::NativeFormat);
        p.Workingdir = qtavr->value("project.wdir").toString();
        ui->statusBar->showMessage(p.Workingdir);
        p.cFileNames.clear();
        p.cFileNames = qtavr->value("project.cfiles").toStringList();
        p.hFileNames.clear();
        p.hFileNames = qtavr->value("project.hfiles").toStringList();
        qDebug() << p.cFileNames << endl;
        qDebug() << p.hFileNames << endl;

        // open c-files
        for(uint8_t i=0; i<p.cFileNames.length(); i++){
            QString filename = QFileInfo(p.cFileNames[i]).fileName();
            QString filePathName = p.cFileNames[i];
            ui->cCfiles->append(filename);

            // New File in tab-bar
            Editor* e = new Editor(this, filePathName, false, false);
            connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
            ui->twMainTab->addTab(e, filename);
            ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
        }
        // open h-files
        for(uint8_t i=0; i<p.hFileNames.length(); i++){
            QString filename = QFileInfo(p.hFileNames[i]).fileName();
            QString filePathName = p.hFileNames[i];
            ui->cCfiles->append(filename);

            // New File in tab-bar
            Editor* e = new Editor(this, filePathName, false, false);
            connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
            ui->twMainTab->addTab(e, filename);
            ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
        }

        // Enable Action- and other buttons (Add-File)
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
    }
    else{
        qDebug() << "Ups..., Something went wrong" << endl;
    }
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
// Change uC
void MainWindow::on_cbController_currentIndexChanged(int index)
{
    qDebug() << QString::number(index) << "Selected: " << processors.at(index) << " with command: " << processorAvrdudeCommands.at(index);
    currentProcessorAvrdudeCommand = processorAvrdudeCommands.at(index);
    currentProcessorGccCommand = processorGccCommands.at(index);
    if(currentProcessorGccCommand.length() == 0){
        // TODO: Warning, this processor is currently not supported!
    }
}
// Change Programmer Device
void MainWindow::on_cbFlashtool_currentIndexChanged(int index)
{
    qDebug() << QString::number(index) << "Selected: " << programmers.at(index) << " with command: " << programmerAvrdudeCommands.at(index);
    currentProgrammerAvrdudeCommand = programmerAvrdudeCommands.at(index);
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

            // New File in tab-bar
            if(suffix.compare("c", Qt::CaseInsensitive) == 0){
                Editor* e = new Editor(this, file, true, true, 1);
                e->setSettings(userSettings);
                connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
                ui->twMainTab->addTab(e, filename);
                ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);

                // append filepath/name to list
                p.cFileNames.append(filepathname);
                // set on label
                ui->cCfiles->clear();
                for(uint8_t i=0; i<p.cFileNames.length(); i++){
                    ui->cCfiles->append(p.cFileNames[i]);
                }

            }else if(suffix.compare("h", Qt::CaseInsensitive) == 0){
                Editor* e = new Editor(this, file, true, true, 2);
                e->setSettings(userSettings);
                connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
                ui->twMainTab->addTab(e, filename);
                ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);

                // append filepath/name to list
                p.hFileNames.append(filepathname);
            }else{
                qDebug() << "Error: Unknows Filetype" << endl;
            }
            ui->actionSave->setEnabled(true);
            ui->actionSave_All->setEnabled(true);
            ui->actionSave_as->setEnabled(true);
    }
}
// Add Existing File
void MainWindow::on_actionExisting_File_triggered()
{
    qDebug() << "Add Existing File" << endl;
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                p.Workingdir,
                                                tr("source (*.c);;header (*.h)")
                                                );
    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        // append filepath/name to list
        p.cFileNames.append(filepathname);
        // set on label
        ui->cCfiles->clear();
        for(uint8_t i=0; i<p.cFileNames.length(); i++){
            ui->cCfiles->append(p.cFileNames[i]);
        }

        // New File in tab-bar
        Editor* e = new Editor(this, file, true, false, 3, p.Workingdir);
        e->setSettings(userSettings);
        connect(e, SIGNAL(unsafed(QString)), this, SLOT(on_fileChanged(QString)));
        ui->twMainTab->addTab(e, filename);
        ui->twMainTab->setCurrentIndex(ui->twMainTab->count()-1);
        ui->actionSave->setEnabled(true);
        ui->actionSave_All->setEnabled(true);
        ui->actionSave_as->setEnabled(true);

    }
}
// Just open an existing file
void MainWindow::on_actionFile_triggered()
{
    qDebug() << "Open Existing File" << endl;
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                p.Workingdir,
                                                tr("source (*.c);;header (*.h);;text (*.txt)")
                                                );
    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        // New File in tab-bar
        Editor* e = new Editor(this, file, false, false);
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

<<<<<<< HEAD
void MainWindow::on_actionOpen_Project_triggered()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Select QtAVR Project File"),
                                                p.Workingdir,
                                                tr("QtAVR Project File (*.avr)")
                                                );
    if(file.length() > 0){
        QString filename        = QFileInfo(file).fileName();
        QString filepathname    = QFileInfo(file).filePath();
        QString filepath        = QFileInfo(file).path();
        QString suffix          = QFileInfo(file).suffix();

        projectFile = new QSettings(file, QSettings::NativeFormat);
    }
}
=======




>>>>>>> tobias
