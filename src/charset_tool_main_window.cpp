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
#include <QtWidgets/QFileDialog>

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include "file_charset_detection_task.h"
#include "file_charset_encoding_task.h"
#include "composite_file_task.h"
#include "dir_iteration_task.h"
#include "select_charset_dialog.h"
#include "progress_dialog.h"
#include "ui_charset_tool_main_window.h"

CharsetToolMainWindow::CharsetToolMainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui_CharsetToolMainWindow),
      m_taskProgressDialog(nullptr),
      m_csDetector(new FileCharsetDetectionTask(this)),
      m_csEncoder(new FileCharsetEncodingTask(this)),
      m_dirIterator(new DirIterationTask(this)),
      m_listAndAnalyseTask(newCompositeFileTask(
                               m_dirIterator,
                               m_csDetector,
                               new CompositeFileTaskBridge_QStringList)),
      m_currentTaskId(CharsetToolMainWindow::NoTask)
{
    m_ui->setupUi(this);

    QObject::connect(m_ui->addFilesBtn, &QAbstractButton::clicked,
                     this, &CharsetToolMainWindow::addInputFiles);
    QObject::connect(m_ui->addFolderBtn, &QAbstractButton::clicked,
                     this, &CharsetToolMainWindow::addInputFolder);

    QObject::connect(m_ui->runAnalyseBtn, &QAbstractButton::clicked,
                     this, &CharsetToolMainWindow::runAnalyse);
    QObject::connect(m_ui->convertCharsetBtn, &QAbstractButton::clicked,
                     this, &CharsetToolMainWindow::runConversion);
    QObject::connect(m_ui->analyseTreeWidget, &QTreeWidget::itemSelectionChanged,
                     this, &CharsetToolMainWindow::updateTaskButtons);

    // Tasks
    m_listAndAnalyseTask->setObjectName(tr("Analyse"));
    m_csEncoder->setObjectName(tr("Conversion"));

    this->connectTask(m_listAndAnalyseTask);
    this->connectTask(m_csEncoder);

    QObject::connect(m_dirIterator, &BaseFileTask::taskStarted,
                     this, &CharsetToolMainWindow::onFileListingStarted);
    QObject::connect(m_csDetector, &BaseFileTask::taskStarted,
                     this, &CharsetToolMainWindow::onDetectionStarted);
    QObject::connect(m_csEncoder, &BaseFileTask::taskStarted,
                     this, &CharsetToolMainWindow::onConversionStarted);

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
    const QStringList fileList = QFileDialog::getOpenFileNames(
                this, tr("Select Input Files"), m_lastInputDir);
    foreach (const QString& file, fileList) {
        const QFileInfo fileInfo(file);
        auto fileItem = new QListWidgetItem(
                    m_fileIconProvider.icon(fileInfo),
                    fileInfo.absoluteFilePath());
        m_ui->inputListWidget->addItem(fileItem);
    }
    if (!fileList.isEmpty())
        m_lastInputDir = QFileInfo(fileList.front()).absolutePath();
}

void CharsetToolMainWindow::addInputFolder()
{
    const QString folder = QFileDialog::getExistingDirectory(
                this, tr("Select Input Folder"), m_lastInputDir);
    if (!folder.isEmpty()) {
        const QString absoluteFolder = QDir(folder).absolutePath();
        m_ui->inputListWidget->addItem(
                    new QListWidgetItem(
                        m_fileIconProvider.icon(QFileIconProvider::Folder),
                        absoluteFolder));
        m_lastInputDir = absoluteFolder;
    }
}

void CharsetToolMainWindow::runAnalyse()
{
    this->setCurrentTask(CharsetToolMainWindow::AnalyseTask);

    m_ui->analyseTreeWidget->clear();
    m_fileToItem.clear();
    m_fileItemBatch.clear();

    QStringList inputList;
    const int inputCount = m_ui->inputListWidget->count();
    for (int row = 0; row < inputCount; ++row)
        inputList += m_ui->inputListWidget->item(row)->text();

    m_dirIterator->setFilters(
                m_ui->filePatternCombo->currentText().split(
                    QLatin1Char(','), QString::SkipEmptyParts));
    m_dirIterator->setInput(inputList);

    m_listAndAnalyseTask->asyncExec();
}

void CharsetToolMainWindow::runConversion()
{
    SelectCharsetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const QByteArray charset = dialog.selectedCharset();
        this->setCurrentTask(CharsetToolMainWindow::ConversionTask);

        // Create vector of input files
        const auto selectedFileItems = m_ui->analyseTreeWidget->selectedItems();
        QVector<FileCharsetEncodingTask::InputFile> inputFileVec;
        inputFileVec.reserve(selectedFileItems.size());
        foreach (const QTreeWidgetItem* fileItem, selectedFileItems) {
            inputFileVec.append(FileCharsetEncodingTask::InputFile(
                                    fileItem->text(1),
                                    fileItem->text(0).toUtf8()));
        }

        m_csEncoder->setTargetCharset(charset);
        m_csEncoder->setInput(inputFileVec);
        m_csEncoder->asyncExec();
    }
}

void CharsetToolMainWindow::onTaskResultItem(
        const BaseFileTask::ResultItem &resultItem)
{
    if (!resultItem.hasError()) {
        switch (m_currentTaskId) {
        case CharsetToolMainWindow::NoTask:
            break;
        case CharsetToolMainWindow::AnalyseTask:
            this->handleAnalyseResultItem(resultItem);
            break;
        case CharsetToolMainWindow::ConversionTask:
            this->handleConversionResultItem(resultItem);
            break;
        } // end switch()
    }
    else {
        this->handleTaskError(resultItem.filePath, resultItem.errorText);
    }

    this->incrementTaskProgress();
}

void CharsetToolMainWindow::onFileListingStarted()
{
    this->createTaskProgressDialog(tr("Listing files"), 0);
}

void CharsetToolMainWindow::onDetectionStarted()
{
    const int fileCount = m_csDetector->inputSize();
    this->createTaskProgressDialog(tr("Analysing %n files", nullptr, fileCount),
                                   fileCount);
}

void CharsetToolMainWindow::onConversionStarted()
{
    const int fileCount = m_csEncoder->inputSize();
    const QString charsetStr = QString::fromUtf8(m_csEncoder->targetCharset());
    this->createTaskProgressDialog(
                tr("Converting %n file to %1 ...", nullptr, fileCount).arg(charsetStr),
                fileCount);
}

void CharsetToolMainWindow::handleTaskError(
        const QString &inputFile, const QString &errorText)
{
    m_ui->pageLog->appendLogError(QString("%1 : %2").arg(inputFile, errorText));
}

void CharsetToolMainWindow::onTaskAborted()
{
    m_ui->pageLog->appendLogInfo(tr("%1 aborted").arg(this->currentTaskName()));
    m_taskProgressDialog->resetCancelFlag();
    this->onTaskEnded();
}

void CharsetToolMainWindow::onTaskFinished()
{
    if (m_currentTaskId == CharsetToolMainWindow::AnalyseTask)
        m_ui->analyseTreeWidget->addTopLevelItems(m_fileItemBatch);

    // taskFinished() is emitted after taskAborted(), so we check if there is
    // any task running to avoid displaying any superfluous "<Task> ended" log
    // message
    if (m_currentTaskId != CharsetToolMainWindow::NoTask) {
        m_ui->pageLog->appendLogInfo(
                    tr("%1 finished").arg(this->currentTaskName()));
    }

    this->onTaskEnded();
}

void CharsetToolMainWindow::onProgressDialogCanceled()
{
    if (m_taskProgressDialog->wasCanceled())
        this->currentTask()->abortTask();
}

void CharsetToolMainWindow::handleAnalyseResultItem(
        const BaseFileTask::ResultItem &resultItem)
{
    const QString charsetStr = resultItem.payload.toString();
    auto item = new QTreeWidgetItem(QStringList(charsetStr) << resultItem.filePath);
    item->setIcon(1, m_fileIconProvider.icon(QFileInfo(resultItem.filePath)));
    m_fileToItem.insert(resultItem.filePath, item);
    m_fileItemBatch.append(item);
    if (m_fileItemBatch.size() == CharsetToolMainWindow::fileItemBatchSize) {
        m_ui->analyseTreeWidget->addTopLevelItems(m_fileItemBatch);
        m_fileItemBatch.clear();
    }
}

void CharsetToolMainWindow::handleConversionResultItem(
        const BaseFileTask::ResultItem &resultItem)
{
    const QString charset = resultItem.payload.toString();
    m_ui->pageLog->appendLogInfo(
                CharsetToolMainWindow::tr("Converted %1 to %2")
                .arg(resultItem.filePath, charset));
    // Update detected charset
    QTreeWidgetItem* item = m_fileToItem.value(resultItem.filePath);
    if (item != nullptr)
        item->setText(0, charset);
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
    QObject::connect(task, &BaseFileTask::taskResultItem,
                     this, &CharsetToolMainWindow::onTaskResultItem);
    QObject::connect(task, &BaseFileTask::taskAborted,
                     this, &CharsetToolMainWindow::onTaskAborted);
    QObject::connect(task, &BaseFileTask::taskFinished,
                     this, &CharsetToolMainWindow::onTaskFinished);
}

BaseFileTask *CharsetToolMainWindow::currentTask() const
{
    switch (m_currentTaskId) {
    case CharsetToolMainWindow::NoTask: return nullptr;
    case CharsetToolMainWindow::AnalyseTask: return m_listAndAnalyseTask;
    case CharsetToolMainWindow::ConversionTask: return m_csEncoder;
    default: return nullptr;
    }
}

QString CharsetToolMainWindow::currentTaskName() const
{
    if (m_currentTaskId != CharsetToolMainWindow::NoTask)
        return this->currentTask()->objectName();
    else
        return tr("Idle");
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
    m_ui->convertCharsetBtn->setEnabled(
                isIdle && !m_ui->analyseTreeWidget->selectedItems().isEmpty());
}

void CharsetToolMainWindow::createTaskProgressDialog(
        const QString &labelText, int fileCount)
{
    if (m_taskProgressDialog == nullptr) {
        m_taskProgressDialog = new ProgressDialog(this);
        QObject::connect(
                    m_taskProgressDialog, &ProgressDialog::canceled,
                    this, &CharsetToolMainWindow::onProgressDialogCanceled);
    }
    m_taskProgressDialog->setLabelText(labelText);
    m_taskProgressDialog->setValue(0);
    m_taskProgressDialog->setMaximumValue(fileCount);
    m_taskProgressDialog->resetCancelFlag();

    if (!m_taskProgressDialog->isVisible())
        m_taskProgressDialog->show();

    m_ui->pageLog->appendLogInfo(labelText);
}

void CharsetToolMainWindow::incrementTaskProgress(int amount)
{
    m_taskProgressDialog->setValue(m_taskProgressDialog->value() + amount);
}
