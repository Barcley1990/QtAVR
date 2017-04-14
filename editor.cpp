/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
****************************************************************************/
#include "editor.h"
#include <QtWidgets>

#include "templateparser.h"

Editor::Editor(QWidget *parent, QString directory, QString filename, uint8_t fileType) : QPlainTextEdit(parent)
{
    // Set Monospace font and smaller TAB stop
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    this->setFont(font);
    QFontMetrics metrics(font);
    this->setTabStopWidth(tabStop * metrics.width(' '));

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(textChanged()), this, SLOT(textHasChanged()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    this->filename = filename;

    // create new file from template
    QString templatePath = "";
    if(fileType==0){
        templatePath = ":/templates/templates/default.txt";
    }
    else if(fileType==1){
        templatePath = ":/templates/templates/default_c.txt";
    }
    else if(fileType==2){
        templatePath = ":/templates/templates/default_h.txt";
    }
    else
        qDebug() << "Error: Unknows Filetype" << endl;

    file = new QFile(this);
    file->setFileName(directory);
    file->open(QIODevice::WriteOnly);
    // Copy the default ressource file to the new generated source file
    TemplateParser* parser = new TemplateParser(this->filename);
    QFile defaultTemplate(templatePath);
    if (defaultTemplate.open(QIODevice::ReadOnly)){
       QTextStream in(&defaultTemplate);
       while (!in.atEnd()){
           file->write(parser->getParsedLine(in.readLine()).toLatin1());
          //file->write(in.readLine().toLatin1());
          file->write("\r\n");
       }
       defaultTemplate.close();
    }


    file->close();

    // start syntaxhighlithning
    highlighter = new mySyntaxHighLighter(this->document());

    // Open File in Editor
    file->open(QFile::ReadOnly | QFile::Text);
    QTextStream ReadFile(file);
    this->document()->setPlainText(ReadFile.readAll());
    file->close();
}

Editor::~Editor(){
    highlighter->deleteLater();
    file->deleteLater();
}



bool Editor::saveContent()
{
    if(file->exists()){
        if(file->open(QFile::WriteOnly | QFile::Text)){
            QTextStream stream(file);
            stream << this->toPlainText();
            file->close();
            this->saved = true;
            return true;
        }else{
            qDebug() << "Error: Cannot open file!" << endl;
        }
    }
    else {
        qDebug() << "Error: File not found!" << endl;
    }
    return false;
}

int Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}



void Editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}



void Editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void Editor::textHasChanged()
{
    this->saved = false;
    emit unsafed(file->fileName());
}

QFile *Editor::getFile() const
{
    return file;
}

void Editor::setFile(QFile *value)
{
    file = value;
}

bool Editor::isSaved() const
{
    return this->saved;
}



void Editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}



void Editor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}



void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
