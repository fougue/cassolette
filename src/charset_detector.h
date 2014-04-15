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

#ifndef CHARSET_DETECTOR_H
#define CHARSET_DETECTOR_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QThreadStorage>
#include <QtCore/QFutureWatcher>

namespace internal { class TextFileFormatDetector; }

class CharsetDetector : public QObject
{
  Q_OBJECT

public:
  CharsetDetector(QObject* parent = NULL);

  Q_SLOT void asyncDetect(const QStringList& filePathList);
  Q_SLOT void abortDetect();

  struct ListFilesResult
  {
    QStringList files;
    QStringList errors;
  };

  static ListFilesResult listFiles(const QStringList& inputs, // Files and folders
                                   const QStringList& filters = QStringList(),
                                   const QStringList& excludes = QStringList());

signals:
  void detectStarted();
  void detection(const QString& inputFile, const QString& charsetName);
  void error(const QString& inputFile, const QString& errorText);
  void detectEnded();

private slots:
  void onDetectionResultReadyAt(int index);

private:
  struct FileDetectionResult
  {
    QString filePath;
    QString encoding;
    QString error;
  };

  FileDetectionResult detectFile(const QString& filePath);

  QThreadStorage<internal::TextFileFormatDetector*> m_detectorByThread;
  QFutureWatcher<FileDetectionResult> m_detectWatcher;
};

#endif // CHARSET_DETECTOR_H
