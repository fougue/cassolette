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

#include "charset_tool_main_window.h"

#include <QtCore/QtDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QTime>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QScrollBar>

#include "charset_detector.h"
#include "input_filter_dialog.h"
#include "ui_charset_tool_main_window.h"

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

CharsetToolMainWindow::CharsetToolMainWindow(QWidget *parent)
  : QMainWindow(parent),
    m_ui(new Ui_CharsetToolMainWindow),
    m_analyseProgressBar(NULL),
    m_csDetector(new CharsetDetector(this))
{
  m_ui->setupUi(this);

  QObject::connect(m_ui->inputFilterBtn, SIGNAL(clicked()), this, SLOT(editFilters()));
  QObject::connect(m_ui->addFilesBtn, SIGNAL(clicked()), this, SLOT(addInputFiles()));
  QObject::connect(m_ui->addFolderBtn, SIGNAL(clicked()), this, SLOT(addInputFolder()));

  QObject::connect(m_ui->runAnalyseBtn, SIGNAL(clicked()), this, SLOT(runAnalyse()));
  QObject::connect(m_ui->stopAnalyseBtn, SIGNAL(clicked()), this, SLOT(stopAnalyse()));

  QObject::connect(m_csDetector, &CharsetDetector::detection,
                   this, &CharsetToolMainWindow::onAnalyseDetection);
  QObject::connect(m_csDetector, &CharsetDetector::error,
                   this, &CharsetToolMainWindow::onAnalyseError);
  QObject::connect(m_csDetector, &CharsetDetector::detectEnded,
                   this, &CharsetToolMainWindow::onAnalyseEnded);

  m_ui->tabWidget->setCurrentWidget(m_ui->pageFiles);
  this->statusBar()->showMessage(tr("Ready"));
  this->updateAnalyseControlButtons(false);
}

CharsetToolMainWindow::~CharsetToolMainWindow()
{
  delete m_ui;
}

void CharsetToolMainWindow::addInputFiles()
{
  const QStringList fileList = QFileDialog::getOpenFileNames(this,
                                                             tr("Select Input Files"),
                                                             m_lastInputDir);
  foreach (const QString& file, fileList)
    m_ui->inputListWidget->addItem(QFileInfo(file).absoluteFilePath());
  if (!fileList.isEmpty())
    m_lastInputDir = QFileInfo(fileList.front()).absolutePath();
}

void CharsetToolMainWindow::addInputFolder()
{
  const QString folder = QFileDialog::getExistingDirectory(this,
                                                           tr("Select Input Folder"),
                                                           m_lastInputDir);
  if (!folder.isEmpty()) {
    const QString absoluteFolder = QDir(folder).absolutePath();
    m_ui->inputListWidget->addItem(absoluteFolder);
    m_lastInputDir = absoluteFolder;
  }
}

void CharsetToolMainWindow::editFilters()
{
  InputFilterDialog dialog(this);
  dialog.installFilterPatterns(m_filterPatterns);
  dialog.installExcludePatterns(m_excludePatterns);
  if (dialog.exec() == QDialog::Accepted) {
    m_filterPatterns = dialog.filterPatterns();
    m_excludePatterns = dialog.excludePatterns();
  }
}

void CharsetToolMainWindow::runAnalyse()
{
  m_ui->analyseTreeWidget->clear();

  this->appendLogInfo(tr("Analysis started"));

  QStringList inputList;
  const int inputCount = m_ui->inputListWidget->count();
  for (int row = 0; row < inputCount; ++row)
    inputList += m_ui->inputListWidget->item(row)->text();

  const auto listFilesRes = CharsetDetector::listFiles(inputList,
                                                       m_filterPatterns.appliablePatterns(),
                                                       m_excludePatterns.appliablePatterns());
  foreach (const QString& err, listFilesRes.errors)
    this->appendLogError(err);

  if (!listFilesRes.files.isEmpty()) {
    this->updateAnalyseControlButtons(true);

    m_analyseProgressBar = new QProgressBar;
    m_analyseProgressBar->setMaximum(listFilesRes.files.size());
    this->statusBar()->addPermanentWidget(m_analyseProgressBar);
    m_analyseProgressBar->show();
    this->statusBar()->showMessage(tr("Analysing ..."));

    m_csDetector->asyncDetect(listFilesRes.files);
  }

  this->appendLogInfo(tr("Analysis ended (%n file(s))", nullptr, listFilesRes.files.size()));
}

void CharsetToolMainWindow::stopAnalyse()
{
  m_csDetector->abortDetect();
}

void CharsetToolMainWindow::onAnalyseDetection(const QString &inputFile, const QString &charsetName)
{
  m_analyseProgressBar->setValue(m_analyseProgressBar->value() + 1);
  QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(charsetName) << inputFile);
  m_ui->analyseTreeWidget->addTopLevelItem(item);
}

void CharsetToolMainWindow::onAnalyseError(const QString &inputFile, const QString &errorText)
{
  m_analyseProgressBar->setValue(m_analyseProgressBar->value() + 1);
  this->appendLogError(QString("%1 : %2").arg(inputFile, errorText));
}

void CharsetToolMainWindow::onAnalyseEnded()
{
  this->updateAnalyseControlButtons(false);
  this->statusBar()->removeWidget(m_analyseProgressBar);
  this->statusBar()->showMessage(tr("Ready"));
}

void CharsetToolMainWindow::updateAnalyseControlButtons(bool analyseIsRunning)
{
  m_ui->stopAnalyseBtn->setEnabled(analyseIsRunning);
  m_ui->runAnalyseBtn->setEnabled(!analyseIsRunning);
}

void CharsetToolMainWindow::clearLog()
{
  m_ui->logTextEdit->clear();
}

void CharsetToolMainWindow::appendLogInfo(const QString &msg)
{
  this->appendLog(msg, InfoLog);
}

void CharsetToolMainWindow::appendLogWarning(const QString &msg)
{
  this->appendLog(msg, WarningLog);
}

void CharsetToolMainWindow::appendLogError(const QString &msg)
{
  this->appendLog(msg, ErrorLog);
}

void CharsetToolMainWindow::appendLog(const QString &msg, LogFormat format)
{
  // Code taken from QtCreator src/plugins/coreplugin/outputwindow.cpp

  const QPalette pal = m_ui->logTextEdit->palette();
  QTextCharFormat textFormat;

  switch (format) {
  case InfoLog:
    textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text), QColor(Qt::blue)));
    textFormat.setFontWeight(QFont::Normal);
    break;
  case WarningLog:
    textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text),
                                                 QColor(255, 165 ,0))); // Orange
    textFormat.setFontWeight(QFont::Bold);
    break;
  case ErrorLog:
    textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text), QColor(Qt::red)));
    textFormat.setFontWeight(QFont::Bold);
    break;
  }

  QScrollBar* logTextEditVertScrollBar = m_ui->logTextEdit->verticalScrollBar();
  const bool atBottom = logTextEditVertScrollBar->value() == logTextEditVertScrollBar->maximum();
  QTextCursor cursor = QTextCursor(m_ui->logTextEdit->document());
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
