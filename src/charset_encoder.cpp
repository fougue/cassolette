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
  : QObject(parent),
    m_dstCodec(NULL)
{
  QObject::connect(&m_encodeWatcher, SIGNAL(resultReadyAt(int)),
                   this, SLOT(onEncodingResultReadyAt(int)));
  QObject::connect(&m_encodeWatcher, SIGNAL(finished()), this, SIGNAL(encodingEnded()));
  QObject::connect(&m_encodeWatcher, SIGNAL(canceled()), this, SIGNAL(encodingEnded()));
}

void CharsetEncoder::asyncEncode(const QByteArray& charset,
                                 const QVector<CharsetEncoder::InputFile> &fileVec)
{
  emit encodingStarted();

  m_dstCodec = QTextCodec::codecForName(charset);
  if (m_dstCodec != NULL) {
    foreach (const CharsetEncoder::InputFile& inputFile, fileVec) {
      const QByteArray& inputCharset = inputFile.charset;
      if (!m_codecCache.contains(inputCharset))
        m_codecCache.insert(inputCharset, QTextCodec::codecForName(inputCharset));
    }

    QFuture<FileEncodingResult> future = QtConcurrent::mapped(fileVec,
                                                              std::bind(&CharsetEncoder::encodeFile,
                                                                        this,
                                                                        std::placeholders::_1));
    m_encodeWatcher.setFuture(future);
  }
  else {
    emit error(QString(), tr("Null text encoder for %1").arg(QString::fromUtf8(charset)));
    emit encodingEnded();
  }
}

void CharsetEncoder::abortEncoding()
{
  m_encodeWatcher.cancel();
}

void CharsetEncoder::onEncodingResultReadyAt(int index)
{
  const FileEncodingResult result = m_encodeWatcher.resultAt(index);
  if (result.error.isEmpty())
    emit encoded(result.inputFile.filePath, m_dstCodec->name());
  else
    emit error(result.inputFile.filePath, result.error);
}

CharsetEncoder::FileEncodingResult CharsetEncoder::encodeFile(const CharsetEncoder::InputFile &inputFile)
{
  CharsetEncoder::FileEncodingResult result;
  result.inputFile = inputFile;

  const QString inputFilePath = inputFile.filePath;
  QTextCodec *srcCodec = m_codecCache.value(inputFile.charset);
  if (srcCodec != NULL) {
    QFile file(inputFilePath);
    if (file.open(QIODevice::ReadOnly)) {
      const QString fileUnicodeContents = srcCodec->toUnicode(file.readAll());
      file.close();

      bool writeSuccess = false;
      if (file.open(QIODevice::WriteOnly)) {
        const QByteArray encodedContents = m_dstCodec->fromUnicode(fileUnicodeContents);
        if (file.write(encodedContents) != -1)
          writeSuccess = true;
      }
      if (!writeSuccess)
        result.error = tr("Failed to write contents (%1)").arg(file.errorString());
    }
    else {
      result.error = tr("Failed to read file (%1)").arg(file.errorString());
    }
  }
  else {
    result.error = tr("Null text encoder for %1").arg(QString::fromUtf8(inputFile.charset));
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
