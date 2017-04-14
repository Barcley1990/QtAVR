#include <QTime>

#include "templateparser.h"

TemplateParser::TemplateParser(QString filename)
{
    this->filename = filename;
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
            return this->filename.mid(0, index-1).toUpper();
        }
        else
        {
            // There is no '.' character in the filename ?!
        }
    }
    return "";
}


