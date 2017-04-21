#include <QTime>

#include "templateparser.h"

TemplateParser::TemplateParser(QString filename)
{
    this->filename = filename;
}

TemplateParser::TemplateParser(QSettings *project, Settings *userSettings)
{
    this->project = project;
    this->userSettings = userSettings;
}

QString TemplateParser::getParsedLine(QString line)
{
    if(line.contains("*AUTHOR*", Qt::CaseSensitive))
    {
        line.replace("*AUTHOR*", getAuthorName());
    }
    if(line.contains("*TIME*", Qt::CaseSensitive))
    {
        line.replace("*TIME*", getTime());
    }
    if(line.contains("*DATE*", Qt::CaseSensitive))
    {
        line.replace("*DATE*", getDate());
    }
    if(line.contains("*FILENAME*", Qt::CaseSensitive))
    {
        line.replace("*FILENAME*", this->filename);
    }
    if(line.contains("*FILE*", Qt::CaseSensitive))
    {
        line.replace("*FILE*", getFile());
    }
    if(line.contains("*GCC_MCU*", Qt::CaseSensitive))
    {
        line.replace("*GCC_MCU*", getGccMcu());
    }
    if(line.contains("*TARGET*", Qt::CaseSensitive))
    {
        line.replace("*TARGET*", getTarget());
    }
    if(line.contains("*OPTIMIZE*", Qt::CaseSensitive))
    {
        line.replace("*OPTIMIZE*", getOptimize());
    }
    if(line.contains("*AVRDUDE_PATH*", Qt::CaseSensitive))
    {
        line.replace("*AVRDUDE_PATH*", getAvrdudePath());
    }
    if(line.contains("*PROGRAMMER*", Qt::CaseSensitive))
    {
        line.replace("*PROGRAMMER*", getProgrammer());
    }
    if(line.contains("*AVRDUDE_MCU*", Qt::CaseSensitive))
    {
        line.replace("*AVRDUDE_MCU*", getAvrdudeMcu());
    }
    if(line.contains("*C_FILES*", Qt::CaseSensitive))
    {
        line.replace("*C_FILES*", getCFiles());
    }
    return line;
}

QString TemplateParser::getAuthorName()
{
    QString name = qgetenv("USER");
    if (name.isEmpty())
    {
        name = qgetenv("USERNAME");
    }
    return name;
}

QString TemplateParser::getTime()
{
    QDateTime dt = QDateTime::currentDateTime();
    return dt.toString("hh:mm:ss");
}

QString TemplateParser::getDate()
{
    QDateTime dt = QDateTime::currentDateTime();
    return dt.toString("dd.MM.yyyy");
}

QString TemplateParser::getFile()
{
    if(this->filename.length() > 0)
    {
        this->filename.replace("-", "_");
        int index = this->filename.indexOf(".", Qt::CaseSensitive);
        if(index >= 0)
        {
            if(index > 0)
            return this->filename.mid(0, index).toUpper();
        }
        else
        {
            // There is no '.' character in the filename ?!
        }
    }
    return "";
}

QString TemplateParser::getGccMcu()
{
    return "";
}

QString TemplateParser::getTarget()
{
    return "";
}

QString TemplateParser::getOptimize()
{
    return "";
}

QString TemplateParser::getAvrdudePath()
{
    QString avrdude = "";
    if(this->userSettings != NULL){
        avrdude = this->userSettings->getAvrdudePath();
        if(!avrdude.endsWith("/")){
            avrdude += "/";
        }
        avrdude += "avrdude";
    }
    return avrdude;
}

QString TemplateParser::getProgrammer()
{
    return "";
}

QString TemplateParser::getAvrdudeMcu()
{
    return "";
}

QString TemplateParser::getCFiles()
{
    return "";
}
