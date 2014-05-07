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

#include "base_file_task.h"

#include <QtCore/QFutureWatcher>
#include <QtCore/QtDebug>

BaseFileTask::BaseFileTask(QObject *parent)
    : QObject(parent),
      m_futureWatcher(nullptr),
      m_inputSize(0)
{
    qRegisterMetaType<BaseFileTask::ResultItem>("BaseFileTask::ResultItem");
}

BaseFileTask::~BaseFileTask()
{
}

int BaseFileTask::inputSize() const
{
    return m_inputSize;
}

//! \brief Does nothing by default
void BaseFileTask::asyncExec()
{
}

void BaseFileTask::abortTask()
{
    if (m_futureWatcher != nullptr
            && m_futureWatcher->isRunning()
            && !m_futureWatcher->isCanceled())
    {
        m_futureWatcher->cancel();
        m_futureWatcher->waitForFinished();
    }
}

bool BaseFileTask::isRunning() const
{
    return m_futureWatcher != nullptr ? m_futureWatcher->isRunning() :
                                        false;
}

void BaseFileTask::onTaskResultReadyAt(int resultId)
{
    const BaseFileTask::ResultItem res = m_futureWatcher->resultAt(resultId);
    emit taskResultItem(res);
}

void BaseFileTask::setInputSize(int size)
{
    m_inputSize = size;
}

void BaseFileTask::createFutureWatcher()
{
    if (m_futureWatcher == nullptr) {
        m_futureWatcher = new QFutureWatcher<ResultItem>(this);
        QObject::connect(m_futureWatcher, SIGNAL(started()), this, SIGNAL(taskStarted()));
        QObject::connect(m_futureWatcher, SIGNAL(finished()), this, SIGNAL(taskFinished()));
        QObject::connect(m_futureWatcher, SIGNAL(canceled()), this, SIGNAL(taskAborted()));
        QObject::connect(m_futureWatcher, SIGNAL(resultReadyAt(int)),
                         this, SLOT(onTaskResultReadyAt(int)));

        QObject::connect(m_futureWatcher, &FutureWatcher::progressRangeChanged,
                         this, &BaseFileTask::taskProgressRangeChanged);
        QObject::connect(m_futureWatcher, &FutureWatcher::progressValueChanged,
                         this, &BaseFileTask::taskProgressValueChanged);
    }
}

QFutureWatcher<BaseFileTask::ResultItem> *BaseFileTask::futureWatcher() const
{
    return m_futureWatcher;
}


bool BaseFileTask::ResultItem::hasError() const
{
    return !errorText.isEmpty();
}

BaseFileTask::ResultItem BaseFileTask::ResultItem::createPayload(const QString &pFilePath)
{
    BaseFileTask::ResultItem item;
    item.filePath = pFilePath;
    item.payload = pFilePath;
    return item;
}

BaseFileTask::ResultItem BaseFileTask::ResultItem::createPayload(const QString &pFilePath,
                                                                 const QVariant &pPayload)
{
    BaseFileTask::ResultItem item;
    item.filePath = pFilePath;
    item.payload = pPayload;
    return item;
}

BaseFileTask::ResultItem BaseFileTask::ResultItem::createError(const QString &pFilePath,
                                                               const QString &pErrorText)
{
    BaseFileTask::ResultItem item;
    item.filePath = pFilePath;
    item.errorText = pErrorText;
    return item;
}
