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

#include "charset_detector.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVector>
#include <QtCore/QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include <functional>

#include "ucsd/nscore.h"
#include "ucsd/nsUniversalDetector.h"

namespace internal {

/*! \brief Charset detector based on Mozilla's Universal CharSet Detector(UCSD) library
 *
 *  Class nsUniversalDetector has to be redefined because nsUniversalDetector::Report() is pure
 *  virtual (see http://www-archive.mozilla.org/projects/intl/detectorsrc.html  Step 2, Write a
 *  wrapper class)
 */
class TextFileFormatDetector : public nsUniversalDetector
{
public:
    TextFileFormatDetector(PRUint32 langFilter);

    const char* detectedEncodingName() const;
    void init();

protected:
    void Report(const char* charset);
    void Reset();

private:
    const char* m_detectedEncodingName;
};

TextFileFormatDetector::TextFileFormatDetector(PRUint32 langFilter)
    : nsUniversalDetector(langFilter),
      m_detectedEncodingName(nullptr)
{
}

const char *TextFileFormatDetector::detectedEncodingName() const
{
    return m_detectedEncodingName;
}

void TextFileFormatDetector::init()
{
    this->Reset();
}

void TextFileFormatDetector::Report(const char *charset)
{
    m_detectedEncodingName = charset;
}

void TextFileFormatDetector::Reset()
{
    nsUniversalDetector::Reset();
    m_detectedEncodingName = nullptr;
}

} // namespace internal

CharsetDetector::CharsetDetector(QObject *parent)
    : BaseFileTask(parent)
{
    this->createFutureWatcher();
}

void CharsetDetector::setInput(const QStringList &filePathList)
{
    m_filePathList = filePathList;
    this->setInputSize(filePathList.size());
}

void CharsetDetector::asyncExec()
{
    auto future = QtConcurrent::mapped(m_filePathList, std::bind(&CharsetDetector::detectFile,
                                                                 this,
                                                                 std::placeholders::_1));
    this->futureWatcher()->setFuture(future);
}

BaseFileTask::ResultItem CharsetDetector::detectFile(const QString &filePath)
{
    BaseFileTask::ResultItem result;

    if (!m_detectorByThread.hasLocalData())
        m_detectorByThread.setLocalData(new internal::TextFileFormatDetector(NS_FILTER_ALL));
    internal::TextFileFormatDetector* formatDetector = m_detectorByThread.localData();

    const QFileInfo fileInfo(filePath);
    result.filePath = fileInfo.absoluteFilePath();
    if (fileInfo.isFile()) {
        QFile file(result.filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray fileContents = file.readAll();
            formatDetector->init();
            const nsresult handleRes = formatDetector->HandleData(fileContents.constData(),
                                                                  fileContents.size());
            formatDetector->DataEnd();
            if (handleRes == NS_OK && formatDetector->detectedEncodingName() != nullptr)
                result.payload = formatDetector->detectedEncodingName();
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
