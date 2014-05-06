/****************************************************************************
**
** CharSet Tool
** Copyright FougSys Ltd. (15 Apr. 2014)
** contact@fougsys.fr
**
** This software is a computer program whose purpose is to analyse and convert
** the encoding of text files.
**
** This software is governed by the CeCILL-B license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-B
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info".
**
** As a counterpart to the access to the source code and  rights to copy,
** modify and redistribute granted by the license, users are provided only
** with a limited warranty  and the software's author,  the holder of the
** economic rights,  and the successive licensors  have only  limited
** liability.
**
** In this respect, the user's attention is drawn to the risks associated
** with loading,  using,  modifying and/or developing or reproducing the
** software by the user in light of its specific status of free software,
** that may mean  that it is complicated to manipulate,  and  that  also
** therefore means  that it is reserved for developers  and  experienced
** professionals having in-depth computer knowledge. Users are therefore
** encouraged to load and test the software's suitability as regards their
** requirements in conditions enabling the security of their systems and/or
** data to be ensured and,  more generally, to use and operate it in the
** same conditions as regards security.
**
** The fact that you are presently reading this means that you have had
** knowledge of the CeCILL-B license and that you accept its terms.
**
****************************************************************************/

#include "log_widget.h"
#include "ui_log_widget.h"

#include <QtCore/QTime>
#include <QtWidgets/QScrollBar>

namespace internal {

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

} // namespace internal

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
        textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text), QColor(Qt::blue)));
        textFormat.setFontWeight(QFont::Normal);
        break;
    case LogWidget::WarningLog:
        textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text),
                                                     QColor(255, 165 ,0))); // Orange
        textFormat.setFontWeight(QFont::Bold);
        break;
    case LogWidget::ErrorLog:
        textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text), QColor(Qt::red)));
        textFormat.setFontWeight(QFont::Bold);
        break;
    }

    QScrollBar* logTextEditVertScrollBar = m_ui->textEdit->verticalScrollBar();
    const bool atBottom = logTextEditVertScrollBar->value() == logTextEditVertScrollBar->maximum();
    QTextCursor cursor = QTextCursor(m_ui->textEdit->document());
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    //: %1 current time of log message    %2 log message
    cursor.insertText(tr("%1: %2").arg(internal::currentTimeLogText(), msg) + QLatin1Char('\n'),
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
