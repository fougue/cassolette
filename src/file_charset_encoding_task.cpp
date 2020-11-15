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

#include "file_charset_encoding_task.h"

#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include <functional>

FileCharsetEncodingTask::FileCharsetEncodingTask(QObject *parent)
    : BaseFileTask(parent),
      m_targetCodec(nullptr)
{
    this->createFutureWatcher();
}

QByteArray FileCharsetEncodingTask::targetCharset() const
{
    return m_targetCodec != nullptr ? m_targetCodec->name() : QByteArray();
}

void FileCharsetEncodingTask::setTargetCharset(const QByteArray &charset)
{
    m_targetCodec = QTextCodec::codecForName(charset);
}

void FileCharsetEncodingTask::setInput(const QVector<FileCharsetEncodingTask::InputFile> &fileVec)
{
    m_inputFileVec = fileVec;
    this->setInputSize(fileVec.size());
}

void FileCharsetEncodingTask::asyncExec()
{
    if (m_targetCodec != nullptr) {
        for (const auto& inputFile : m_inputFileVec) {
            const QByteArray& inputCharset = inputFile.charset;
            if (!m_codecCache.contains(inputCharset))
                m_codecCache.insert(inputCharset, QTextCodec::codecForName(inputCharset));
        }

        const std::function<ResultItem (const InputFile&)> func =
                [=](const InputFile &file) { return this->encodeFile(file); };
        auto future = QtConcurrent::mapped(m_inputFileVec, func);
        this->futureWatcher()->setFuture(future);
    }
    else {
        //emit taskError(QString(), tr("Null text encoder for %1").arg(QString::fromUtf8(charset)));
        emit taskFinished();
    }
}

BaseFileTask::ResultItem FileCharsetEncodingTask::encodeFile(
        const FileCharsetEncodingTask::InputFile &inputFile)
{
    BaseFileTask::ResultItem result;
    result.filePath = inputFile.filePath;
    result.payload = m_targetCodec->name();

    const QString inputFilePath = inputFile.filePath;
    QTextCodec *srcCodec = m_codecCache.value(inputFile.charset);
    if (srcCodec != nullptr) {
        QFile file(inputFilePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QString fileUnicodeContents = srcCodec->toUnicode(file.readAll());
            file.close();

            bool writeSuccess = false;
            if (file.open(QIODevice::WriteOnly)) {
                const QByteArray encodedContents = m_targetCodec->fromUnicode(fileUnicodeContents);
                if (file.write(encodedContents) != -1)
                    writeSuccess = true;
            }

            if (!writeSuccess)
                result.errorText = tr("Failed to write contents (%1)").arg(file.errorString());
        }
        else {
            result.errorText = tr("Failed to read file (%1)").arg(file.errorString());
        }
    }
    else {
        result.errorText = tr("Null text encoder for %1").arg(QString::fromUtf8(inputFile.charset));
    }

    return result;
}


FileCharsetEncodingTask::InputFile::InputFile()
{
}

FileCharsetEncodingTask::InputFile::InputFile(const QString &pFilePath, const QByteArray &pCharset)
    : filePath(pFilePath),
      charset(pCharset)
{
}
