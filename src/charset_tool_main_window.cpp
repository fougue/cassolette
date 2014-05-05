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
#include <QtWidgets/QScrollBar>

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include "charset_detector.h"
#include "charset_encoder.h"
#include "input_filter_dialog.h"
#include "progress_dialog.h"
#include "select_charset_dialog.h"
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
    m_taskProgressDialog(NULL),
    m_csDetector(new CharsetDetector(this)),
    m_csEncoder(new CharsetEncoder(this)),
    m_currentTaskId(CharsetToolMainWindow::NoTask)
{
  m_ui->setupUi(this);

  QObject::connect(m_ui->inputFilterBtn, SIGNAL(clicked()), this, SLOT(editFilters()));
  QObject::connect(m_ui->addFilesBtn, SIGNAL(clicked()), this, SLOT(addInputFiles()));
  QObject::connect(m_ui->addFolderBtn, SIGNAL(clicked()), this, SLOT(addInputFolder()));

  QObject::connect(m_ui->runAnalyseBtn, SIGNAL(clicked()), this, SLOT(runAnalyse()));
  QObject::connect(m_ui->convertCharsetBtn, SIGNAL(clicked()), this, SLOT(runConversion()));
  QObject::connect(m_ui->analyseTreeWidget, &QTreeWidget::itemSelectionChanged,
                   this, &CharsetToolMainWindow::updateTaskButtons);

  // Task "analyse"
  QObject::connect(m_csDetector, &BaseFileTask::taskResult,
                   this, &CharsetToolMainWindow::onAnalyseDetection);
  this->connectTask(m_csDetector);
  m_csDetector->setObjectName(tr("Analyse"));

  // Task "conversion"
  QObject::connect(m_csEncoder, &BaseFileTask::taskResult,
                   this, &CharsetToolMainWindow::onEncoded);
  this->connectTask(m_csEncoder);
  m_csEncoder->setObjectName(tr("Conversion"));

  // Init
  this->setCurrentTask(m_currentTaskId);
  m_ui->tabWidget->setCurrentWidget(m_ui->pageFiles);
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
  foreach (const QString& file, fileList) {
    const QFileInfo fileInfo(file);
    QListWidgetItem* fileItem = new QListWidgetItem(m_fileIconProvider.icon(fileInfo),
                                                    fileInfo.absoluteFilePath());
    m_ui->inputListWidget->addItem(fileItem);
  }
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
    m_ui->inputListWidget->addItem(new QListWidgetItem(m_fileIconProvider.icon(QFileIconProvider::Folder),
                                                       absoluteFolder));
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
  this->setCurrentTask(CharsetToolMainWindow::AnalyseTask);

  m_ui->analyseTreeWidget->clear();
  m_fileToItem.clear();

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
    // Notify user about new task
    this->createTaskProgressDialog(tr("Analysing %n file(s)", nullptr, listFilesRes.files.size()),
                                   listFilesRes.files.size());

    m_csDetector->asyncDetect(listFilesRes.files);
  }
  else {
    this->setCurrentTask(CharsetToolMainWindow::NoTask);
  }
}

void CharsetToolMainWindow::runConversion()
{
  SelectCharsetDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    const QByteArray charset = dialog.selectedCharset();
    const QString charsetStr = QString::fromUtf8(charset);
    this->setCurrentTask(CharsetToolMainWindow::ConversionTask);

    // Create vector of input files
    const QList<QTreeWidgetItem*> selectedFileItems = m_ui->analyseTreeWidget->selectedItems();
    QVector<CharsetEncoder::InputFile> inputFileVec;
    inputFileVec.reserve(selectedFileItems.size());
    foreach (const QTreeWidgetItem* fileItem, selectedFileItems) {
      inputFileVec.append(CharsetEncoder::InputFile(fileItem->text(1),
                                                    fileItem->text(0).toUtf8()));
    }

    // Notify user about new task
    this->createTaskProgressDialog(tr("Converting %n file(s) to %1 ...",
                                      NULL,
                                      inputFileVec.size()).arg(charsetStr),
                                   inputFileVec.size());

    m_csEncoder->asyncEncode(charset, inputFileVec);
  }
}

void CharsetToolMainWindow::onAnalyseDetection(const QString &inputFile, const QVariant &payload)
{
  const QString charset = payload.toString();
  QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(charset) << inputFile);
  item->setIcon(1, m_fileIconProvider.icon(QFileInfo(inputFile)));
  m_fileToItem.insert(inputFile, item);
  m_ui->analyseTreeWidget->addTopLevelItem(item);
  this->incrementTaskProgress();
}

void CharsetToolMainWindow::onEncoded(const QString &inputFile, const QVariant &payload)
{
  const QString charset = payload.toString();
  this->incrementTaskProgress();
  this->appendLogInfo(tr("Converted %1 to %2").arg(inputFile).arg(charset));
  // Update detected charset
  QTreeWidgetItem* item = m_fileToItem.value(inputFile);
  if (item != NULL)
    item->setText(0, charset);
}

void CharsetToolMainWindow::onTaskError(const QString &inputFile, const QString &errorText)
{
  this->incrementTaskProgress();
  this->appendLogError(QString("%1 : %2").arg(inputFile, errorText));
}

void CharsetToolMainWindow::onTaskAborted()
{
  this->appendLogInfo(tr("%1 aborted").arg(this->currentTaskName()));
  this->onTaskEnded();
}

void CharsetToolMainWindow::onTaskFinished()
{
  // taskFinished() is emitted after taskAborted(), so we check if there is any task running to
  //   avoid displaying any superfluous "<Task> ended" log message
  if (m_currentTaskId != CharsetToolMainWindow::NoTask)
    this->appendLogInfo(tr("%1 finished").arg(this->currentTaskName()));
  this->onTaskEnded();
}

void CharsetToolMainWindow::onTaskEnded()
{
  if (m_currentTaskId != CharsetToolMainWindow::NoTask) {
    m_taskProgressDialog->reset();
    this->setCurrentTask(CharsetToolMainWindow::NoTask);
  }
}

void CharsetToolMainWindow::connectTask(const BaseFileTask *task)
{
  QObject::connect(task, &BaseFileTask::taskError,
                   this, &CharsetToolMainWindow::onTaskError);
  QObject::connect(task, &BaseFileTask::taskAborted,
                   this, &CharsetToolMainWindow::onTaskAborted);
  QObject::connect(task, &BaseFileTask::taskFinished,
                   this, &CharsetToolMainWindow::onTaskFinished);
}

BaseFileTask *CharsetToolMainWindow::currentTask() const
{
  switch (m_currentTaskId) {
  case CharsetToolMainWindow::NoTask: return NULL;
  case CharsetToolMainWindow::AnalyseTask: return m_csDetector;
  case CharsetToolMainWindow::ConversionTask: return m_csEncoder;
  default: return NULL;
  }
}

QString CharsetToolMainWindow::currentTaskName() const
{
  if (m_currentTaskId != CharsetToolMainWindow::NoTask)
    return this->currentTask()->objectName();
  else
    return tr("Idle");
}


void CharsetToolMainWindow::onTaskStarted()
{
  //this->appendLogInfo(tr("%1 started").arg(this->currentTaskName()));
}

void CharsetToolMainWindow::setCurrentTask(CharsetToolMainWindow::TaskId taskId)
{
  m_currentTaskId = taskId;
  this->updateTaskButtons();
}

void CharsetToolMainWindow::updateTaskButtons()
{
  const bool isIdle = m_currentTaskId == CharsetToolMainWindow::NoTask;
  m_ui->runAnalyseBtn->setEnabled(isIdle);
  m_ui->convertCharsetBtn->setEnabled(isIdle && !m_ui->analyseTreeWidget->selectedItems().isEmpty());
}

void CharsetToolMainWindow::createTaskProgressDialog(const QString &labelText, int fileCount)
{
  if (m_taskProgressDialog == NULL)
    m_taskProgressDialog = new ProgressDialog(this);
  else
    QObject::disconnect(m_taskProgressDialog, &ProgressDialog::canceled, NULL, NULL);
  m_taskProgressDialog->setLabelText(labelText);
  m_taskProgressDialog->setValue(0);
  m_taskProgressDialog->setMaximumValue(fileCount);

  QObject::connect(m_taskProgressDialog, &ProgressDialog::canceled,
                   this->currentTask(), &BaseFileTask::abortTask);

  m_taskProgressDialog->show();

  this->appendLogInfo(labelText);
}

void CharsetToolMainWindow::incrementTaskProgress()
{
  m_taskProgressDialog->setValue(m_taskProgressDialog->value() + 1);
}

void CharsetToolMainWindow::clearLog()
{
  m_ui->logTextEdit->clear();
}

void CharsetToolMainWindow::appendLogInfo(const QString &msg)
{
  this->appendLog(msg, CharsetToolMainWindow::InfoLog);
}

void CharsetToolMainWindow::appendLogWarning(const QString &msg)
{
  this->appendLog(msg, CharsetToolMainWindow::WarningLog);
}

void CharsetToolMainWindow::appendLogError(const QString &msg)
{
  this->appendLog(msg, CharsetToolMainWindow::ErrorLog);
}

void CharsetToolMainWindow::appendLog(const QString &msg, LogFormat format)
{
  // Code taken from QtCreator src/plugins/coreplugin/outputwindow.cpp

  const QPalette pal = m_ui->logTextEdit->palette();
  QTextCharFormat textFormat;

  switch (format) {
  case CharsetToolMainWindow::InfoLog:
    textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text), QColor(Qt::blue)));
    textFormat.setFontWeight(QFont::Normal);
    break;
  case CharsetToolMainWindow::WarningLog:
    textFormat.setForeground(internal::mixColors(pal.color(QPalette::Text),
                                                 QColor(255, 165 ,0))); // Orange
    textFormat.setFontWeight(QFont::Bold);
    break;
  case CharsetToolMainWindow::ErrorLog:
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
