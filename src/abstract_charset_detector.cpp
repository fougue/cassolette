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

#include "abstract_charset_detector.h"

AbstractCharsetDetector::~AbstractCharsetDetector()
{
}

AbstractCharsetDetector::Error::Error()
    : code(-1)
{
}

AbstractCharsetDetector::Error::Error(int64_t pCode, const QString &msg)
    : code(pCode),
      message(msg)
{
}
