#include <QTime>

#include "templateparser.h"

TemplateParser::TemplateParser()
{

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
