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

#pragma once

#include <cstdint>
#include <QtCore/QString>

class AbstractCharsetDetector {
public:
    struct Error {
        Error();
        Error(int64_t pCode, const QString& msg = QString());
        int64_t code;
        QString message;
    };

    virtual ~AbstractCharsetDetector();

    virtual QByteArray detectedEncodingName() const = 0;

    virtual void init() = 0;
    virtual bool handleData(const QByteArray& buffer, Error* error = nullptr) = 0;
    virtual void dataEnd() = 0;
};
