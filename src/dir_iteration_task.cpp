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

#include "dir_iteration_task.h"

#include <functional>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QDirIterator>
#include <QtCore/QtDebug>

DirIterationTask::DirIterationTask(QObject *parent)
    : BaseFileTask(parent),
      m_futureWatcher(new QFutureWatcher<void>(this))
{
    QObject::connect(m_futureWatcher, &QFutureWatcher<void>::started,
                     this, &DirIterationTask::taskStarted);
    QObject::connect(m_futureWatcher, &QFutureWatcher<void>::finished,
                     this, &DirIterationTask::onFutureFinished);
}

void DirIterationTask::setFilters(const QStringList &filters)
{
    m_filters = filters;
    m_filterRxVec.clear();
    foreach (const QString& filter, filters) {
        m_filterRxVec.append(QRegExp(
                                 filter.trimmed(),
                                 Qt::CaseSensitive,
                                 QRegExp::Wildcard));
    }
}

void DirIterationTask::setExcludes(const QStringList &excludes)
{
    m_excludes = excludes;
    m_excludeRxVec.clear();
    foreach (const QString& exclude, excludes) {
        m_excludeRxVec.append(QRegExp(
                                  exclude.trimmed(),
                                  Qt::CaseSensitive,
                                  QRegExp::Wildcard));
    }
}

void DirIterationTask::setInput(const QStringList &fileOrFolderList)
{
    m_fileOrFolderList = fileOrFolderList;
}

void DirIterationTask::asyncExec()
{
    m_futureWatcher->setFuture(QtConcurrent::run([=]{ this->iterate(); }));
}

bool DirIterationTask::isRunning() const
{
    return m_futureWatcher->isRunning();
}

void DirIterationTask::iterate()
{
    foreach (const QString& input, m_fileOrFolderList) {
        if (this->abortRequested())
            return;

        const QFileInfo inputInfo(input);
        const QString inputAbsPath = inputInfo.absoluteFilePath();
        if (inputInfo.exists()) {
            if (inputInfo.isDir()) {
                QDirIterator dirIt( inputInfo.absoluteFilePath(), QDirIterator::Subdirectories);
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

bool DirIterationTask::acceptsInputFile(const QString &file) const
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

void DirIterationTask::onFutureFinished()
{
    if (!this->abortRequested())
        emit taskFinished();
    else
        this->endAbortRequest();
}
