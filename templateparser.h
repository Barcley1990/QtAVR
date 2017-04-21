#ifndef TEMPLATEPARSER_H
#define TEMPLATEPARSER_H

#include <QFile>
#include <QString>
#include <QSettings>

#include "settings.h"

class TemplateParser
{
public:
    TemplateParser(QString filename);
    TemplateParser(QSettings* project, Settings* userSettings);
    QString getParsedLine(QString line);

private:
    QSettings* project;
    Settings* userSettings;
    QString filename;
    QString getAuthorName();
    QString getTime();
    QString getDate();
    QString getFile();
    QString getGccMcu();
    QString getTarget();
    QString getOptimize();
    QString getAvrdudePath();
    QString getProgrammer();
    QString getAvrdudeMcu();
    QString getCFiles();
};

#endif // TEMPLATEPARSER_H
