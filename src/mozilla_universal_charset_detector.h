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

#include "abstract_charset_detector.h"

#include <ucsd/nscore.h>
#include <ucsd/nsUniversalDetector.h>

/*! Charset detector based on Mozilla's Universal CharSet Detector(UCSD) library
 *
 *  Class nsUniversalDetector has to be redefined because
 *  nsUniversalDetector::Report() is pure virtual
 *  (see http://www-archive.mozilla.org/projects/intl/detectorsrc.html
 *   Step 2, Write a wrapper class)
 */
class MozillaUniversalCharsetDetector :
        public AbstractCharsetDetector, protected nsUniversalDetector
{
public:
    MozillaUniversalCharsetDetector(PRUint32 langFilter);

    QByteArray detectedEncodingName() const override;

    void init() override;
    bool handleData(const QByteArray& buffer, Error* error) override;
    void dataEnd() override;

protected:
    void Report(const char* charset) override;
    void Reset() override;

private:
    QByteArray m_detectedEncodingName;
};
