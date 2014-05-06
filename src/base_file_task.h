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

#ifndef BASE_FILE_TASK_H
#define BASE_FILE_TASK_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QVector>
template<typename T> class QFutureWatcher;

class BaseFileTask : public QObject
{
    Q_OBJECT

public:
    struct ResultItem
    {
        QString  filePath;
        QVariant payload;
        QString  errorText;
        bool hasError() const;
    };
    typedef QVector<BaseFileTask::ResultItem> ResultBatch;

    BaseFileTask(QObject* parent = nullptr);
    ~BaseFileTask();

    Q_SLOT virtual void abortTask();

    int batchSize() const;
    void setBatchSize(int size);

signals:
    void taskStarted();
    void taskBatch(const BaseFileTask::ResultBatch& batch);
    void taskFinished();
    void taskAborted();

protected:
    void createFutureWatcher();
    QFutureWatcher<BaseFileTask::ResultItem> *futureWatcher() const;

    Q_SLOT virtual void onTaskResultReadyAt(int resultId);

private slots:
    void onFutureWatcherFinished();
    void onFutureWatcherCanceled();

private:
    QFutureWatcher<BaseFileTask::ResultItem>* m_futureWatcher;
    int m_batchSize;
    QVector<ResultItem> m_batchVec;
};

#endif // BASE_FILE_TASK_H
