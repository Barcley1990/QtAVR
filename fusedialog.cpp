#include "fusedialog.h"
#include "ui_fusedialog.h"

FuseDialog::FuseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FuseDialog)
{
    ui->setupUi(this);
    connect(ui->bFuses, SIGNAL(clicked()), this, SLOT(SetFuses()));

}

FuseDialog::~FuseDialog()
{
    delete ui;
}

void FuseDialog::getWorkingDir(QString dir){
    WorkingDir = dir;
}

void FuseDialog::SetFuses(){
    // Get Fusevalues as ASCII
    QString LF = ui->LF_TextField->text();
    QString HF = ui->HF_TextField->text();
    QString EF = ui->EF_TextField->text();

    // open qmessage box (warning)
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Flash Fuses",
                                  "Flash?",
                                   QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qDebug() << "Yes was clicked";
        //QString workingdir = MainWindow.setWorkingDirectory();
/*
        // create strings to send
        qDebug() << "uC: " << uc << " programer: " << fd << endl;
        QString avrdude = userSettings->getAvrdudePath() + "/avrdude " ;
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


        // Edit Build.sh
        // Create SHELL Files
        FFlashFilePath = Workingdir.append("/FFlash.sh");
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


        // Execute Scriptfile
        proc1->start(FFlashFilePath);
        if (is_error == false){
            ui->cOutput->setTextColor(QColor(150,200,150));
            ui->cOutput->append("Fuse Flash OK!");
        }
        else
            is_error = false;


        //if(FlashFile.exists())
        //    FlashFile.remove();

*/
      } else {
        qDebug() << "Yes was *not* clicked";
      }


}
