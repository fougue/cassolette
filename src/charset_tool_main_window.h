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

#pragma once

#include "base_file_task.h"
class FileCharsetDetectionTask;
class FileCharsetEncodingTask;
class DirIterationTask;
class ProgressDialog;

#include <QtWidgets/QFileIconProvider>
#include <QtWidgets/QMainWindow>
class QTreeWidgetItem;

class CharsetToolMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CharsetToolMainWindow(QWidget *parent = nullptr);
    ~CharsetToolMainWindow();

private slots:
    void addInputFiles();
    void addInputFolder();

    void runAnalyse();
    void runConversion();

    void onTaskResultItem(const BaseFileTask::ResultItem& resultItem);
    void onFileListingStarted();
    void onDetectionStarted();
    void onConversionStarted();
    void onTaskAborted();
    void onTaskFinished();
    void onProgressDialogCanceled();

private:
    void handleAnalyseResultItem(const BaseFileTask::ResultItem& resultItem);
    void handleConversionResultItem(const BaseFileTask::ResultItem& resultItem);

    enum TaskId
    {
        NoTask,
        AnalyseTask,
        ConversionTask
    };

    void connectTask(const BaseFileTask* task);

    BaseFileTask* currentTask() const;
    QString currentTaskName() const;
    void setCurrentTask(TaskId taskId);
    void handleTaskError(const QString& inputFile, const QString& errorText);

    void updateTaskButtons();
    void createTaskProgressDialog(const QString& labelText, int fileCount);
    void incrementTaskProgress(int amount = 1);
    void onTaskEnded();

    class Ui_CharsetToolMainWindow *m_ui;
    QString m_lastInputDir;
    ProgressDialog* m_taskProgressDialog;

    FileCharsetDetectionTask* m_csDetector;
    FileCharsetEncodingTask* m_csEncoder;
    DirIterationTask* m_dirIterator;
    BaseFileTask* m_listAndAnalyseTask;

    TaskId m_currentTaskId;
    QHash<QString, QTreeWidgetItem*> m_fileToItem;
    QFileIconProvider m_fileIconProvider;

    static const int fileItemBatchSize = 128;
    QList<QTreeWidgetItem*> m_fileItemBatch;
};
