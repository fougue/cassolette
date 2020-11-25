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

#pragma once

#include "base_file_task.h"
#include <QtCore/QHash>
#include <QtCore/QVector>
class QTextCodec;

/*! \brief Provides encoding of files to a target character set
 *
 *  BaseFileTask::ResultItem::payload contains the target character set
 */
class FileCharsetEncodingTask : public BaseFileTask {
    Q_OBJECT

public:
    struct InputFile {
        InputFile();
        InputFile(const QString& pFilePath, const QByteArray& pCharset);
        QString filePath;
        QByteArray charset;
    };
    typedef QVector<InputFile> InputType;

    FileCharsetEncodingTask(QObject *parent = nullptr);

    QByteArray targetCharset() const;
    void setTargetCharset(const QByteArray& charset);

    void setInput(const QVector<InputFile>& fileVec);

    void asyncExec() override;

private:
    BaseFileTask::ResultItem encodeFile(const InputFile& inputFile);

    QVector<InputFile> m_inputFileVec;
    QHash<QByteArray, QTextCodec*> m_codecCache;
    QTextCodec* m_targetCodec;
};
