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

#include "file_charset_detection_task.h"

#include "mozilla_universal_charset_detector.h"
#include "win_imulti_language_charset_detector.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVector>
#include <QtCore/QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include <functional>

FileCharsetDetectionTask::FileCharsetDetectionTask(QObject *parent)
    : BaseFileTask(parent)
{
    this->createFutureWatcher();
}

void FileCharsetDetectionTask::setInput(const QStringList &filePathList)
{
    m_filePathList = filePathList;
    this->setInputSize(filePathList.size());
}

void FileCharsetDetectionTask::asyncExec()
{
    const std::function<ResultItem (const QString&)> func =
            [=](const QString& fpath) { return this->detectFile(fpath); };
    auto future = QtConcurrent::mapped(m_filePathList, func);
    this->futureWatcher()->setFuture(future);
}

BaseFileTask::ResultItem FileCharsetDetectionTask::detectFile(const QString &filePath)
{
    BaseFileTask::ResultItem result;

    if (!m_detectorByThread.hasLocalData()) {
        m_detectorByThread.setLocalData(new MozillaUniversalCharsetDetector(NS_FILTER_ALL));
        //m_detectorByThread.setLocalData(new WinIMultiLanguageCharsetDetector);
    }
    AbstractCharsetDetector* formatDetector = m_detectorByThread.localData();

    const QFileInfo fileInfo(filePath);
    result.filePath = fileInfo.absoluteFilePath();
    if (fileInfo.isFile()) {
        QFile file(result.filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray fileContents = file.readAll();
            formatDetector->init();

            AbstractCharsetDetector::Error detectError;
            const bool handleSuccess = formatDetector->handleData(fileContents, &detectError);
            formatDetector->dataEnd();

            if (handleSuccess) {
                if (!formatDetector->detectedEncodingName().isEmpty())
                    result.payload = formatDetector->detectedEncodingName();
            }
            else {
                result.errorText = detectError.message;
            }
        }
        else {
            result.errorText = !file.errorString().isEmpty() ? file.errorString() : tr("Unknonw error");
        }
    }
    else {
        result.errorText = tr("Not a file");
    }

    return result;
}
