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

#include "win_imulti_language_charset_detector.h"

#ifdef Q_OS_WIN
# include <comdef.h> // For _com_error
# include <mlang.h> // For IMultiLanguage2
# include <Shlwapi.h> // For IMultiLanguage2
# include <windows.h>
#endif

#include <QtCore/QtDebug>

#include <algorithm>

#include <fougtools/cpptools/c_array_utils.h>
#include <fougtools/cpptools/memory_utils.h>

namespace Internal {

static bool confidenceLessThan(const DetectEncodingInfo& lhs, const DetectEncodingInfo& rhs)
{
    return lhs.nConfidence < rhs.nConfidence;
}

static QString hresultToQString(HRESULT hres)
{
  _com_error err(hres);
#ifdef UNICODE
  return QString::fromWCharArray(err.ErrorMessage());
#else
  return QString::fromLatin1(err.ErrorMessage());
#endif // UNICODE
}

static AbstractCharsetDetector::Error toDetectionError(HRESULT error)
{
    return AbstractCharsetDetector::Error(static_cast<int64_t>(error), hresultToQString(error));
}

} // namespace Internal

WinIMultiLanguageCharsetDetector::WinIMultiLanguageCharsetDetector()
    : m_multiLang(nullptr)
{
#ifdef Q_OS_WIN
    // Note that CoCreateInstance() requires ole32.lib
    CoInitialize(nullptr);
    const HRESULT createInstError = CoCreateInstance(
                CLSID_CMultiLanguage,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_IMultiLanguage2,
                reinterpret_cast<void**>(&m_multiLang));
    m_constructError = Internal::toDetectionError(createInstError);
#endif // Q_OS_WIN
}

WinIMultiLanguageCharsetDetector::~WinIMultiLanguageCharsetDetector()
{
    if (m_multiLang != nullptr)
        m_multiLang->Release();

    CoUninitialize();
}

QByteArray WinIMultiLanguageCharsetDetector::detectedEncodingName() const
{
    return m_detectedEncodingName;
}

void WinIMultiLanguageCharsetDetector::init()
{
    m_detectedEncodingName.clear();
}

bool WinIMultiLanguageCharsetDetector::handleData(const QByteArray &buffer, Error *error)
{
#ifdef Q_OS_WIN
    if (m_multiLang != nullptr) {
        IStream* streamBuffer = SHCreateMemStream(
                    reinterpret_cast<const BYTE*>(buffer.constData()), buffer.size());
        if (streamBuffer != nullptr) {
            DetectEncodingInfo encodingInfoArray[8];
            int encodingInfoCount =
                    static_cast<int>(cpp::cArraySize(encodingInfoArray));

            const HRESULT detectError = m_multiLang->DetectCodepageInIStream(
                        MLDETECTCP_NONE,
                        0,
                        streamBuffer,
                        encodingInfoArray,
                        &encodingInfoCount);
            cpp::checkedAssign(error, Internal::toDetectionError(detectError));
            streamBuffer->Release();
            if (detectError == S_OK) {
                // Find best confident encoding
                const auto encodingInfoArrayEnd = encodingInfoArray + encodingInfoCount;
                auto iBestEncInfo = std::max_element(
                            encodingInfoArray,
                            encodingInfoArrayEnd,
                            &Internal::confidenceLessThan);
                if (iBestEncInfo != encodingInfoArrayEnd) {
                    MIMECPINFO cpInfo;
                    const HRESULT getCpInfoError = m_multiLang->GetCodePageInfo(
                                (*iBestEncInfo).nCodePage,
                                (*iBestEncInfo).nLangID,
                                &cpInfo);
                    cpp::checkedAssign(
                                error, Internal::toDetectionError(getCpInfoError));
                    if (getCpInfoError == S_OK) {
                        m_detectedEncodingName =
                                QString::fromWCharArray(cpInfo.wszWebCharset).toUtf8();
//                        m_detectedEncodingName =
//                                QString("%1(%2%)")
//                                .arg(QString(m_detectedEncodingName))
//                                .arg((*iBestEncInfo).nConfidence)
//                                .toUtf8();
                        return true;
                    }
                }
            }
        }
    }
    else {
        cpp::checkedAssign(error, m_constructError);
    }
#endif // Q_OS_WIN

    return false;
}

void WinIMultiLanguageCharsetDetector::dataEnd()
{
}
