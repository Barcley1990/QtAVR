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

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    //Editor(QWidget *parent = 0);
    Editor(QWidget *parent = 0, QString directory = 0);
    ~Editor();

    QFile getter();
    void setter();
    void saveContent();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
    mySyntaxHighLighter *highlighter;
    QFile *file;
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
