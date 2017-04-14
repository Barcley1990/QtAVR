#ifndef TEMPLATEPARSER_H
#define TEMPLATEPARSER_H

#include <QFile>
#include <QString>

class TemplateParser
{
public:
    TemplateParser(QString filename);
    QString getParsedLine(QString line);

private:
    QString filename;
    QString getAuthorName();
    QString getTime();
    QString getDate();
    QString getFile();
};

#endif // TEMPLATEPARSER_H
