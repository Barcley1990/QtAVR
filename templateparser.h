#ifndef TEMPLATEPARSER_H
#define TEMPLATEPARSER_H

#include <QFile>
#include <QString>

class TemplateParser
{
public:
    TemplateParser();
    QString getParsedLine(QString line);

private:
    QString getAuthorName();
    QString getTime();
    QString getDate();
};

#endif // TEMPLATEPARSER_H
