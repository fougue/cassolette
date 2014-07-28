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
    auto future = QtConcurrent::mapped(m_filePathList,
                                       std::bind(&FileCharsetDetectionTask::detectFile,
                                                 this,
                                                 std::placeholders::_1));
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
            result.errorText = !file.errorString().isEmpty() ? file.errorString() :
                                                               tr("Unknonw error");
        }
    }
    else {
        result.errorText = tr("Not a file");
    }

    return result;
}
