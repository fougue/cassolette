/****************************************************************************
** CharSet Tool
** Copyright Fougue Ltd. (15 Apr. 2014)
** contact@fougsys.fr
**
** This software is a computer program whose purpose is to analyse and convert
** the encoding of text files.
**
** This software is governed by the CeCILL-B license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-B
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html".
****************************************************************************/

#include "log_widget.h"
#include "ui_log_widget.h"

#include <QtCore/QTime>
#include <QtWidgets/QScrollBar>

namespace Internal {

static QColor mixColors(const QColor& a, const QColor& b)
{
    return QColor((a.red()   + 2 * b.red())   / 3,
                  (a.green() + 2 * b.green()) / 3,
                  (a.blue()  + 2 * b.blue())  / 3,
                  (a.alpha() + 2 * b.alpha()) / 3);
}

static QString currentTimeLogText()
{
    return QTime::currentTime().toString(Qt::DefaultLocaleShortDate);
}

} // namespace Internal

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui_LogWidget)
{
    m_ui->setupUi(this);
}

LogWidget::~LogWidget()
{
    delete m_ui;
}

void LogWidget::clearLog()
{
    m_ui->textEdit->clear();
}

void LogWidget::appendLogInfo(const QString &msg)
{
    this->appendLog(msg, LogWidget::InfoLog);
}

void LogWidget::appendLogWarning(const QString &msg)
{
    this->appendLog(msg, LogWidget::WarningLog);
}

void LogWidget::appendLogError(const QString &msg)
{
    this->appendLog(msg, LogWidget::ErrorLog);
}

void LogWidget::appendLog(const QString &msg, LogFormat format)
{
    // Code taken from QtCreator src/plugins/coreplugin/outputwindow.cpp

    const QPalette pal = m_ui->textEdit->palette();
    QTextCharFormat textFormat;

    switch (format) {
    case LogWidget::InfoLog:
        textFormat.setForeground(Internal::mixColors(pal.color(QPalette::Text), QColor(Qt::blue)));
        textFormat.setFontWeight(QFont::Normal);
        break;
    case LogWidget::WarningLog:
        textFormat.setForeground(Internal::mixColors(pal.color(QPalette::Text), QColor(255, 165 ,0))); // Orange
        textFormat.setFontWeight(QFont::Bold);
        break;
    case LogWidget::ErrorLog:
        textFormat.setForeground(Internal::mixColors(pal.color(QPalette::Text), QColor(Qt::red)));
        textFormat.setFontWeight(QFont::Bold);
        break;
    }

    QScrollBar* logTextEditVertScrollBar = m_ui->textEdit->verticalScrollBar();
    const bool atBottom = logTextEditVertScrollBar->value() == logTextEditVertScrollBar->maximum();
    QTextCursor cursor = QTextCursor(m_ui->textEdit->document());
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    //: %1 current time of log message    %2 log message
    cursor.insertText(
                tr("%1: %2").arg(Internal::currentTimeLogText(), msg)
                + QLatin1Char('\n'),
                textFormat);
    cursor.endEditBlock();

    if (atBottom) {
        logTextEditVertScrollBar->setValue(logTextEditVertScrollBar->maximum());
        // QPlainTextEdit destroys the first calls value in case of multiline
        // text, so make sure that the scroll bar actually gets the value set.
        // Is a noop if the first call succeeded.
        logTextEditVertScrollBar->setValue(logTextEditVertScrollBar->maximum());
    }
}
