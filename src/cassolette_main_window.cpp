/****************************************************************************
** Cassolette
** Copyright Fougue Ltd. (15 Apr. 2014)
** contact@fougue.pro
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

#include "cassolette_main_window.h"

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
#include "ui_cassolette_main_window.h"

CassoletteMainWindow::CassoletteMainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui_CassoletteMainWindow),
      m_taskProgressDialog(nullptr),
      m_csDetector(new FileCharsetDetectionTask(this)),
      m_csEncoder(new FileCharsetEncodingTask(this)),
      m_dirIterator(new DirIterationTask(this)),
      m_listAndAnalyseTask(newCompositeFileTask(
                               m_dirIterator,
                               m_csDetector,
                               new CompositeFileTaskBridge_QStringList)),
      m_currentTaskId(CassoletteMainWindow::NoTask)
{
    m_ui->setupUi(this);

    QObject::connect(m_ui->addFilesBtn, &QAbstractButton::clicked,
                     this, &CassoletteMainWindow::addInputFiles);
    QObject::connect(m_ui->addFolderBtn, &QAbstractButton::clicked,
                     this, &CassoletteMainWindow::addInputFolder);

    QObject::connect(m_ui->runAnalyseBtn, &QAbstractButton::clicked,
                     this, &CassoletteMainWindow::runAnalyse);
    QObject::connect(m_ui->convertCharsetBtn, &QAbstractButton::clicked,
                     this, &CassoletteMainWindow::runConversion);
    QObject::connect(m_ui->analyseTreeWidget, &QTreeWidget::itemSelectionChanged,
                     this, &CassoletteMainWindow::updateTaskButtons);

    // Tasks
    m_listAndAnalyseTask->setObjectName(tr("Analyse"));
    m_csEncoder->setObjectName(tr("Conversion"));

    this->connectTask(m_listAndAnalyseTask);
    this->connectTask(m_csEncoder);

    QObject::connect(m_dirIterator, &BaseFileTask::taskStarted,
                     this, &CassoletteMainWindow::onFileListingStarted);
    QObject::connect(m_csDetector, &BaseFileTask::taskStarted,
                     this, &CassoletteMainWindow::onDetectionStarted);
    QObject::connect(m_csEncoder, &BaseFileTask::taskStarted,
                     this, &CassoletteMainWindow::onConversionStarted);

    // Init
    this->setCurrentTask(m_currentTaskId);
    m_ui->tabWidget->setCurrentWidget(m_ui->pageFiles);
}

CassoletteMainWindow::~CassoletteMainWindow()
{
    delete m_ui;
}

void CassoletteMainWindow::addInputFiles()
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

void CassoletteMainWindow::addInputFolder()
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

void CassoletteMainWindow::runAnalyse()
{
    this->setCurrentTask(CassoletteMainWindow::AnalyseTask);

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

void CassoletteMainWindow::runConversion()
{
    SelectCharsetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const QByteArray charset = dialog.selectedCharset();
        this->setCurrentTask(CassoletteMainWindow::ConversionTask);

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

void CassoletteMainWindow::onTaskResultItem(const BaseFileTask::ResultItem& resultItem)
{
    if (!resultItem.hasError()) {
        switch (m_currentTaskId) {
        case CassoletteMainWindow::NoTask:
            break;
        case CassoletteMainWindow::AnalyseTask:
            this->handleAnalyseResultItem(resultItem);
            break;
        case CassoletteMainWindow::ConversionTask:
            this->handleConversionResultItem(resultItem);
            break;
        } // end switch()
    }
    else {
        this->handleTaskError(resultItem.filePath, resultItem.errorText);
    }

    this->incrementTaskProgress();
}

void CassoletteMainWindow::onFileListingStarted()
{
    this->createTaskProgressDialog(tr("Listing files"), 0);
}

void CassoletteMainWindow::onDetectionStarted()
{
    const int fileCount = m_csDetector->inputSize();
    this->createTaskProgressDialog(tr("Analysing %n files", nullptr, fileCount), fileCount);
}

void CassoletteMainWindow::onConversionStarted()
{
    const int fileCount = m_csEncoder->inputSize();
    const QString charsetStr = QString::fromUtf8(m_csEncoder->targetCharset());
    this->createTaskProgressDialog(
                tr("Converting %n file to %1 ...", nullptr, fileCount).arg(charsetStr),
                fileCount);
}

void CassoletteMainWindow::handleTaskError(const QString &inputFile, const QString &errorText)
{
    m_ui->pageLog->appendLogError(QString("%1 : %2").arg(inputFile, errorText));
}

void CassoletteMainWindow::onTaskAborted()
{
    m_ui->pageLog->appendLogInfo(tr("%1 aborted").arg(this->currentTaskName()));
    m_taskProgressDialog->resetCancelFlag();
    this->onTaskEnded();
}

void CassoletteMainWindow::onTaskFinished()
{
    if (m_currentTaskId == CassoletteMainWindow::AnalyseTask)
        m_ui->analyseTreeWidget->addTopLevelItems(m_fileItemBatch);

    // taskFinished() is emitted after taskAborted(), so we check if there is
    // any task running to avoid displaying any superfluous "<Task> ended" log
    // message
    if (m_currentTaskId != CassoletteMainWindow::NoTask)
        m_ui->pageLog->appendLogInfo(tr("%1 finished").arg(this->currentTaskName()));

    this->onTaskEnded();
}

void CassoletteMainWindow::onProgressDialogCanceled()
{
    if (m_taskProgressDialog->wasCanceled())
        this->currentTask()->abortTask();
}

void CassoletteMainWindow::handleAnalyseResultItem(const BaseFileTask::ResultItem& resultItem)
{
    const QString charsetStr = resultItem.payload.toString();
    auto item = new QTreeWidgetItem(QStringList(charsetStr) << resultItem.filePath);
    item->setIcon(1, m_fileIconProvider.icon(QFileInfo(resultItem.filePath)));
    m_fileToItem.insert(resultItem.filePath, item);
    m_fileItemBatch.append(item);
    if (m_fileItemBatch.size() == CassoletteMainWindow::fileItemBatchSize) {
        m_ui->analyseTreeWidget->addTopLevelItems(m_fileItemBatch);
        m_fileItemBatch.clear();
    }
}

void CassoletteMainWindow::handleConversionResultItem(const BaseFileTask::ResultItem& resultItem)
{
    const QString charset = resultItem.payload.toString();
    m_ui->pageLog->appendLogInfo(
                CassoletteMainWindow::tr("Converted %1 to %2")
                .arg(resultItem.filePath, charset));
    // Update detected charset
    QTreeWidgetItem* item = m_fileToItem.value(resultItem.filePath);
    if (item != nullptr)
        item->setText(0, charset);
}

void CassoletteMainWindow::onTaskEnded()
{
    if (m_currentTaskId != CassoletteMainWindow::NoTask) {
        m_taskProgressDialog->reset();
        this->setCurrentTask(CassoletteMainWindow::NoTask);
    }
}

void CassoletteMainWindow::connectTask(const BaseFileTask *task)
{
    QObject::connect(task, &BaseFileTask::taskResultItem,
                     this, &CassoletteMainWindow::onTaskResultItem);
    QObject::connect(task, &BaseFileTask::taskAborted,
                     this, &CassoletteMainWindow::onTaskAborted);
    QObject::connect(task, &BaseFileTask::taskFinished,
                     this, &CassoletteMainWindow::onTaskFinished);
}

BaseFileTask *CassoletteMainWindow::currentTask() const
{
    switch (m_currentTaskId) {
    case CassoletteMainWindow::NoTask: return nullptr;
    case CassoletteMainWindow::AnalyseTask: return m_listAndAnalyseTask;
    case CassoletteMainWindow::ConversionTask: return m_csEncoder;
    default: return nullptr;
    }
}

QString CassoletteMainWindow::currentTaskName() const
{
    if (m_currentTaskId != CassoletteMainWindow::NoTask)
        return this->currentTask()->objectName();
    else
        return tr("Idle");
}

void CassoletteMainWindow::setCurrentTask(CassoletteMainWindow::TaskId taskId)
{
    m_currentTaskId = taskId;
    this->updateTaskButtons();
}

void CassoletteMainWindow::updateTaskButtons()
{
    const bool isIdle = m_currentTaskId == CassoletteMainWindow::NoTask;
    m_ui->runAnalyseBtn->setEnabled(isIdle);
    m_ui->convertCharsetBtn->setEnabled(
                isIdle && !m_ui->analyseTreeWidget->selectedItems().isEmpty());
}

void CassoletteMainWindow::createTaskProgressDialog(const QString &labelText, int fileCount)
{
    if (m_taskProgressDialog == nullptr) {
        m_taskProgressDialog = new ProgressDialog(this);
        QObject::connect(
                    m_taskProgressDialog, &ProgressDialog::canceled,
                    this, &CassoletteMainWindow::onProgressDialogCanceled);
    }

    m_taskProgressDialog->setLabelText(labelText);
    m_taskProgressDialog->setValue(0);
    m_taskProgressDialog->setMaximumValue(fileCount);
    m_taskProgressDialog->resetCancelFlag();

    if (!m_taskProgressDialog->isVisible())
        m_taskProgressDialog->show();

    m_ui->pageLog->appendLogInfo(labelText);
}

void CassoletteMainWindow::incrementTaskProgress(int amount)
{
    m_taskProgressDialog->setValue(m_taskProgressDialog->value() + amount);
}
