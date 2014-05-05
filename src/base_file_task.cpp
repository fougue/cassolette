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
    m_futureWatcher(NULL),
    m_batchSize(0)
{
  this->setBatchSize(256);
}

BaseFileTask::~BaseFileTask()
{
}

void BaseFileTask::abortTask()
{
  if (m_futureWatcher != NULL && !m_futureWatcher->isCanceled()) {
    m_futureWatcher->cancel();
    m_futureWatcher->waitForFinished();
  }
}

int BaseFileTask::batchSize() const
{
  return m_batchSize;
}

void BaseFileTask::setBatchSize(int size)
{
  m_batchSize = size;
  m_batchVec.reserve(size);
}

void BaseFileTask::onTaskResultReadyAt(int resultId)
{
  const BaseFileTask::ResultItem fileRes = m_futureWatcher->resultAt(resultId);
  m_batchVec.append(fileRes);
  if (m_batchVec.size() == this->batchSize()) {
    emit taskBatch(m_batchVec);
    m_batchVec.clear();
  }
}

void BaseFileTask::onFutureWatcherFinished()
{
  if (m_batchVec.size() < this->batchSize() && !m_futureWatcher->isCanceled()) {
    emit taskBatch(m_batchVec);
    m_batchVec.clear();
  }
  emit taskFinished();
}

void BaseFileTask::onFutureWatcherCanceled()
{
  m_batchVec.clear();
  emit taskAborted();
}

void BaseFileTask::createFutureWatcher()
{
  if (m_futureWatcher == NULL) {
    m_futureWatcher = new QFutureWatcher<ResultItem>(this);
    QObject::connect(m_futureWatcher, SIGNAL(finished()), this, SLOT(onFutureWatcherFinished()));
    QObject::connect(m_futureWatcher, SIGNAL(canceled()), this, SLOT(onFutureWatcherCanceled()));
    QObject::connect(m_futureWatcher, SIGNAL(resultReadyAt(int)),
                     this, SLOT(onTaskResultReadyAt(int)));
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
