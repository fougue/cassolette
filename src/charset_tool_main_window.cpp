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

#include "charset_tool_main_window.h"

#include <QtCore/QtDebug>
#include <QtCore/QFileInfo>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QProgressBar>

#include "charset_detector.h"
#include "input_filter_dialog.h"
#include "ui_charset_tool_main_window.h"

CharsetToolMainWindow::CharsetToolMainWindow(QWidget *parent)
  : QMainWindow(parent),
    m_ui(new Ui_CharsetToolMainWindow),
    m_analyseProgressBar(NULL),
    m_csDetector(new CharsetDetector(this))
{
  m_ui->setupUi(this);

  QObject::connect(m_ui->inputFilterBtn, SIGNAL(clicked()), this, SLOT(editFilters()));
  QObject::connect(m_ui->addFilesBtn, SIGNAL(clicked()), this, SLOT(addInputFiles()));
  QObject::connect(m_ui->addFolderBtn, SIGNAL(clicked()), this, SLOT(addInputFolder()));

  QObject::connect(m_ui->runAnalyseBtn, SIGNAL(clicked()), this, SLOT(runAnalyse()));
  QObject::connect(m_ui->stopAnalyseBtn, SIGNAL(clicked()), this, SLOT(stopAnalyse()));

  QObject::connect(m_csDetector, SIGNAL(detection(QString,QString)),
                   this, SLOT(onAnalyseDetection(QString,QString)));
  QObject::connect(m_csDetector, SIGNAL(error(QString,QString)),
                   this, SLOT(onAnalyseError(QString,QString)));
  QObject::connect(m_csDetector, SIGNAL(detectEnded()), this, SLOT(onAnalyseEnded()));

  this->statusBar()->showMessage(tr("Ready"));
  this->updateAnalyseControlButtons(false);
}

CharsetToolMainWindow::~CharsetToolMainWindow()
{
  delete m_ui;
}

void CharsetToolMainWindow::addInputFiles()
{
  const QStringList fileList = QFileDialog::getOpenFileNames(this,
                                                             tr("Select Input Files"),
                                                             m_lastInputDir);
  foreach (const QString& file, fileList)
    m_ui->inputListWidget->addItem(QFileInfo(file).absoluteFilePath());
  if (!fileList.isEmpty())
    m_lastInputDir = QFileInfo(fileList.front()).absolutePath();
}

void CharsetToolMainWindow::addInputFolder()
{
  const QString folder = QFileDialog::getExistingDirectory(this,
                                                           tr("Select Input Folder"),
                                                           m_lastInputDir);
  if (!folder.isEmpty()) {
    const QString absoluteFolder = QDir(folder).absolutePath();
    m_ui->inputListWidget->addItem(absoluteFolder);
    m_lastInputDir = absoluteFolder;
  }
}

void CharsetToolMainWindow::editFilters()
{
  InputFilterDialog dialog(this);
  dialog.installFilterPatterns(m_filterPatterns);
  dialog.installExcludePatterns(m_excludePatterns);
  if (dialog.exec() == QDialog::Accepted) {
    m_filterPatterns = dialog.filterPatterns();
    m_excludePatterns = dialog.excludePatterns();
  }
}

void CharsetToolMainWindow::runAnalyse()
{
  m_ui->analyseTreeWidget->clear();

  QStringList inputList;
  const int inputCount = m_ui->inputListWidget->count();
  for (int row = 0; row < inputCount; ++row)
    inputList += m_ui->inputListWidget->item(row)->text();

  const CharsetDetector::ListFilesResult listFilesRes = CharsetDetector::listFiles(inputList,
                                                                                   m_filterPatterns,
                                                                                   m_excludePatterns);
  foreach (const QString& err, listFilesRes.errors)
    qDebug() << err;

  if (!listFilesRes.files.isEmpty()) {
    this->updateAnalyseControlButtons(true);

    m_analyseProgressBar = new QProgressBar;
    m_analyseProgressBar->setMaximum(listFilesRes.files.size());
    this->statusBar()->addWidget(m_analyseProgressBar);

    m_csDetector->asyncDetect(listFilesRes.files);
  }
}

void CharsetToolMainWindow::stopAnalyse()
{
  m_csDetector->abortDetect();
}

void CharsetToolMainWindow::onAnalyseDetection(const QString &inputFile, const QString &charsetName)
{
  m_analyseProgressBar->setValue(m_analyseProgressBar->value() + 1);
  QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(charsetName) << inputFile);
  m_ui->analyseTreeWidget->addTopLevelItem(item);
}

void CharsetToolMainWindow::onAnalyseError(const QString &inputFile, const QString &errorText)
{
  m_analyseProgressBar->setValue(m_analyseProgressBar->value() + 1);
  qWarning() << QString("%1 : %2").arg(inputFile, errorText);
}

void CharsetToolMainWindow::onAnalyseEnded()
{
  this->updateAnalyseControlButtons(false);
  this->statusBar()->removeWidget(m_analyseProgressBar);
}

void CharsetToolMainWindow::updateAnalyseControlButtons(bool analyseIsRunning)
{
  m_ui->stopAnalyseBtn->setEnabled(analyseIsRunning);
  m_ui->runAnalyseBtn->setEnabled(!analyseIsRunning);
}
