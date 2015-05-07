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
