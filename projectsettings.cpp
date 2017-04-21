#include <qdebug.h>
#include "QtXml/qdom.h"
#include <QFile>

#include "projectsettings.h"
#include "ui_projectsettings.h"

ProjectSettings::ProjectSettings(QWidget *parent, QSettings* settings) : QDialog(parent), ui(new Ui::ProjectSettings)
{
    ui->setupUi(this);
    // Set Window Icon
    setWindowIcon(QIcon(":/images/images/chip03_small.png"));

    populateComboBoxes();

    this->settings = settings;

    if(this->settings != NULL){
        ui->comboBoxOptimize->setCurrentIndex(this->settings->value("project.optimize").toInt());
        ui->cbController->setCurrentIndex(this->settings->value("project.controller").toInt());
        ui->cbFlashtool->setCurrentIndex(this->settings->value("project.programmer").toInt());
    }
}

ProjectSettings::~ProjectSettings()
{
    saveSettings();
    delete ui;
}


void ProjectSettings::populateComboBoxes()
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

void ProjectSettings::saveSettings()
{
    qDebug() << "saveSettings";
    if(this->settings != NULL){
        this->settings->setValue("project.optimize", ui->comboBoxOptimize->currentIndex());
        this->settings->setValue("project.optimizeCommand", ui->comboBoxOptimize->currentText());
        this->settings->setValue("project.controller", ui->cbController->currentIndex());
        this->settings->setValue("project.gcc_mcu", processorGccCommands.at(ui->cbController->currentIndex()));
        this->settings->setValue("project.avrdude_mcu", processorAvrdudeCommands.at(ui->cbController->currentIndex()));
        this->settings->setValue("project.programmer", ui->cbFlashtool->currentIndex());
        this->settings->setValue("project.programmerCommand", programmerAvrdudeCommands.at(ui->cbFlashtool->currentIndex()));
    }
}
