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

#ifndef CHARSET_TOOL_MAIN_WINDOW_H
#define CHARSET_TOOL_MAIN_WINDOW_H

#include "base_file_task.h"
class CharsetDetector;
class CharsetEncoder;
class DirIterator;
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

    CharsetDetector* m_csDetector;
    CharsetEncoder* m_csEncoder;
    DirIterator* m_dirIterator;
    BaseFileTask* m_listAndAnalyseTask;

    TaskId m_currentTaskId;
    QHash<QString, QTreeWidgetItem*> m_fileToItem;
    QFileIconProvider m_fileIconProvider;

    static const int fileItemBatchSize = 128;
    QList<QTreeWidgetItem*> m_fileItemBatch;
};

#endif // CHARSET_TOOL_MAIN_WINDOW_H
