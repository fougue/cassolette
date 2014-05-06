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

#ifndef CHARSET_ENCODER_H
#define CHARSET_ENCODER_H

#include "base_file_task.h"
#include <QtCore/QHash>
class QTextCodec;

/*! \brief Provides encoding of files to a target character set
 *
 *  BaseFileTask::ResultItem::payload contains the target character set
 */
class CharsetEncoder : public BaseFileTask
{
    Q_OBJECT

public:
    struct InputFile
    {
        InputFile();
        InputFile(const QString& pFilePath, const QByteArray& pCharset);
        QString filePath;
        QByteArray charset;
    };

    CharsetEncoder(QObject *parent = NULL);

    void asyncEncode(const QByteArray& charset, const QVector<InputFile>& fileVec);

private:
    BaseFileTask::ResultItem encodeFile(const InputFile& inputFile);

    QHash<QByteArray, QTextCodec*> m_codecCache;
    QTextCodec *m_dstCodec;
};

#endif // CHARSET_ENCODER_H
