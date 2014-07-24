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

#include "dir_iterator.h"

#include <functional>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QDirIterator>
#include <QtCore/QtDebug>

DirIterator::DirIterator(QObject *parent)
    : BaseFileTask(parent),
      m_futureWatcher(new QFutureWatcher<void>(this))
{
    QObject::connect(m_futureWatcher, &QFutureWatcher<void>::started,
                     this, &DirIterator::taskStarted);
    QObject::connect(m_futureWatcher, &QFutureWatcher<void>::finished,
                     this, &DirIterator::onFutureFinished);
}

void DirIterator::setFilters(const QStringList &filters)
{
    m_filters = filters;
    m_filterRxVec.clear();
    foreach (const QString& filter, filters)
        m_filterRxVec.append(QRegExp(filter.trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
}

void DirIterator::setExcludes(const QStringList &excludes)
{
    m_excludes = excludes;
    m_excludeRxVec.clear();
    foreach (const QString& exclude, excludes)
        m_excludeRxVec.append(QRegExp(exclude.trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
}

void DirIterator::setInput(const QStringList &fileOrFolderList)
{
    m_fileOrFolderList = fileOrFolderList;
}

void DirIterator::asyncExec()
{
    m_futureWatcher->setFuture(QtConcurrent::run(std::bind(&DirIterator::iterate, this)));
}

bool DirIterator::isRunning() const
{
    return m_futureWatcher->isRunning();
}

void DirIterator::iterate()
{
    foreach (const QString& input, m_fileOrFolderList) {
        if (this->abortRequested())
            return;

        const QFileInfo inputInfo(input);
        const QString inputAbsPath = inputInfo.absoluteFilePath();
        if (inputInfo.exists()) {
            if (inputInfo.isDir()) {
                QDirIterator dirIt(inputInfo.absoluteFilePath(), QDirIterator::Subdirectories);
                while (dirIt.hasNext()) {
                    if (this->abortRequested())
                        return;

                    dirIt.next();
                    const QFileInfo subFileInfo = dirIt.fileInfo();
                    const QString subFileAbsPath = subFileInfo.absoluteFilePath();
                    if (subFileInfo.isFile() && this->acceptsInputFile(subFileAbsPath))
                        emit taskResultItem(BaseFileTask::ResultItem::createPayload(subFileAbsPath));
                }
            }
            else if (inputInfo.isFile() && this->acceptsInputFile(inputAbsPath)) {
                emit taskResultItem(BaseFileTask::ResultItem::createPayload(inputAbsPath));
            }
        }
        else {
            const QString errorText = tr("'%1' does not exist").arg(inputAbsPath);
            emit taskResultItem(BaseFileTask::ResultItem::createError(inputAbsPath, errorText));
        }
    } // end foreach
}

bool DirIterator::acceptsInputFile(const QString &file) const
{
    bool passFilter = m_filterRxVec.isEmpty();
    foreach (const QRegExp& filterRx, m_filterRxVec) {
        if (filterRx.exactMatch(file)) {
            passFilter = true;
            break;
        }
    }

    if (passFilter) {
        foreach (const QRegExp& excludeRx, m_excludeRxVec) {
            if (excludeRx.indexIn(file) != -1)
                return false;
        }
    }

    return passFilter;
}

void DirIterator::onFutureFinished()
{
    if (!this->abortRequested())
        emit taskFinished();
    else
        this->endAbortRequest();
}
