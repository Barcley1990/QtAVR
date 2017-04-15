/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
****************************************************************************/
#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QFile>

#include "mysyntaxhighlighter.h"
#include "settings.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    Editor(QWidget *parent = 0, QString fileName="", bool addFile=true, bool newFile=true, uint8_t fileType=0, QString wdir = "", Settings* settings = NULL);
    ~Editor();

    bool saveContent();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    QFile *getFile() const;
    void setFile(QFile *value);

    bool isSaved() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void textHasChanged();

signals:
   void unsafed(QString filename);

private:
    QString filename;
    QWidget *lineNumberArea;
    mySyntaxHighLighter *highlighter;
    QFile *file;
    uint8_t fileType;
    bool saved;
    const int defaultFontSize = 14;
    const int defaultTabStop = 3;
    Settings* settings;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(Editor *editor) : QWidget(editor) {
        this->editor = editor;
    }

    QSize sizeHint() const override {
        return QSize(editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        editor->lineNumberAreaPaintEvent(event);
    }

private:
    Editor *editor;
};



#endif // EDITOR_H
