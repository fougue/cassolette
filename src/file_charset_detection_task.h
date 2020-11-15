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

#include <QtCore/QStringList>
#include <QtCore/QThreadStorage>

class AbstractCharsetDetector;

/*! \brief Provides detection of the character set used to encode a file
 *
 *  BaseFileTask::ResultItem::payload contains the detected character set
 */
class FileCharsetDetectionTask : public BaseFileTask {
    Q_OBJECT

public:
    typedef QStringList InputType;

    FileCharsetDetectionTask(QObject* parent = nullptr);

    void setInput(const QStringList& filePathList);
    void asyncExec() override;

private:
    BaseFileTask::ResultItem detectFile(const QString& filePath);

    QStringList m_filePathList;
    QThreadStorage<AbstractCharsetDetector*> m_detectorByThread;
};
