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

#include "charset_encoder.h"

#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include <functional>

CharsetEncoder::CharsetEncoder(QObject *parent)
    : BaseFileTask(parent),
      m_targetCodec(nullptr)
{
    this->createFutureWatcher();
}

QByteArray CharsetEncoder::targetCharset() const
{
    return m_targetCodec != nullptr ? m_targetCodec->name() :
                                      QByteArray();
}

void CharsetEncoder::setTargetCharset(const QByteArray &charset)
{
    m_targetCodec = QTextCodec::codecForName(charset);
}

void CharsetEncoder::setInput(const QVector<CharsetEncoder::InputFile> &fileVec)
{
    m_inputFileVec = fileVec;
    this->setInputSize(fileVec.size());
}

void CharsetEncoder::asyncExec()
{
    if (m_targetCodec != nullptr) {
        foreach (const CharsetEncoder::InputFile& inputFile, m_inputFileVec) {
            const QByteArray& inputCharset = inputFile.charset;
            if (!m_codecCache.contains(inputCharset))
                m_codecCache.insert(inputCharset, QTextCodec::codecForName(inputCharset));
        }

        auto future = QtConcurrent::mapped(m_inputFileVec, std::bind(&CharsetEncoder::encodeFile,
                                                                     this,
                                                                     std::placeholders::_1));
        this->futureWatcher()->setFuture(future);
    }
    else {
        //emit taskError(QString(), tr("Null text encoder for %1").arg(QString::fromUtf8(charset)));
        emit taskFinished();
    }
}

BaseFileTask::ResultItem CharsetEncoder::encodeFile(const CharsetEncoder::InputFile &inputFile)
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


CharsetEncoder::InputFile::InputFile()
{
}

CharsetEncoder::InputFile::InputFile(const QString &pFilePath, const QByteArray &pCharset)
    : filePath(pFilePath),
      charset(pCharset)
{
}
