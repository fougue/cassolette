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

#include <QtCore/QVector>
#include <QtCore/QRegExp>
class QFileInfo;

/*! \brief Provides recursive(directory) file iteration
 *
 *  BaseFileTask::ResultItem::filePath and payload contains the same value
 *
 *  \note DirIterationTask does not set BaseFileTask::inputSize() so task
 *        progress cannot be computed
 */
class DirIterationTask : public BaseFileTask
{
    Q_OBJECT

public:
    typedef QStringList InputType;

    DirIterationTask(QObject* parent = nullptr);

    void setFilters(const QStringList& filters);
    void setExcludes(const QStringList& excludes);
    void setInput(const QStringList& fileOrFolderList);

    void asyncExec() override;
    bool isRunning() const override;

private:
    void iterate();
    bool acceptsInputFile(const QString& file) const;

    void onFutureFinished();

    QFutureWatcher<void>* m_futureWatcher;

    QStringList m_filters;
    QVector<QRegExp> m_filterRxVec;

    QStringList m_excludes;
    QVector<QRegExp> m_excludeRxVec;

    QStringList m_fileOrFolderList;
};
