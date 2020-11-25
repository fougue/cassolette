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

#include "mozilla_universal_charset_detector.h"

#include <QtCore/QString>

#include <fougtools/cpptools/memory_utils.h>

namespace Internal {

static AbstractCharsetDetector::Error toDetectionError(nsresult error)
{
    QString errorMsg;
    if (error == NS_ERROR_OUT_OF_MEMORY)
        errorMsg = QLatin1String("NS_ERROR_OUT_OF_MEMORY");
    return AbstractCharsetDetector::Error(static_cast<int64_t>(error), errorMsg);
}

} // namespace Internal

MozillaUniversalCharsetDetector::MozillaUniversalCharsetDetector(PRUint32 langFilter)
    : nsUniversalDetector(langFilter)
{
}

QByteArray MozillaUniversalCharsetDetector::detectedEncodingName() const
{
    return m_detectedEncodingName;
}

void MozillaUniversalCharsetDetector::init()
{
    this->Reset();
}

bool MozillaUniversalCharsetDetector::handleData(const QByteArray &buffer, Error *error)
{
    const nsresult res = nsUniversalDetector::HandleData(buffer.constData(), buffer.size());
    cpp::checkedAssign(error, Internal::toDetectionError(res));
    return res == NS_OK;
}

void MozillaUniversalCharsetDetector::dataEnd()
{
    nsUniversalDetector::DataEnd();
}

void MozillaUniversalCharsetDetector::Report(const char *charset)
{
    m_detectedEncodingName = charset;
}

void MozillaUniversalCharsetDetector::Reset()
{
    nsUniversalDetector::Reset();
    m_detectedEncodingName.clear();
}
