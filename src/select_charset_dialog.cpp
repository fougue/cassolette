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

#include "select_charset_dialog.h"
#include "ui_select_charset_dialog.h"

#include <algorithm>
#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>

namespace internal {

static const char SelectCharsetDialog_lastCodecNameIniKey[] = "SelectCharsetDialog_lastCodecName";
static const int  SelectCharsetDialog_codecDataRole = Qt::UserRole + 1;

} // namespace internal

SelectCharsetDialog::SelectCharsetDialog(QWidget *parent)
  : QDialog(parent),
    m_ui(new Ui_SelectCharsetDialog)
{
  m_ui->setupUi(this);

  QSettings appSettings;
  const QString iniLastCodecName = appSettings.value(internal::SelectCharsetDialog_lastCodecNameIniKey,
                                                     QLatin1String("UTF-8")).toString();

  // Build sorted map of charset names
  QMap<QByteArray, QTextCodec*> codecMap;
  foreach (const QByteArray& codecName, QTextCodec::availableCodecs())
    codecMap.insert(codecName, QTextCodec::codecForName(codecName));

  // Build list widget
  foreach (const QByteArray& codecName, codecMap.keys()) {
    const QTextCodec* codec = codecMap.value(codecName);

    // Build list of codec names
    const QString codecNameStr(codecName);
    QStringList codecAliasList(codecNameStr);
    foreach (const QByteArray& alias, codec->aliases()) {
      const QString aliasStr(alias);
      if (!codecAliasList.contains(aliasStr, Qt::CaseInsensitive))
        codecAliasList.append(aliasStr);
    }

    // Create list item for codec
    QListWidgetItem* codecItem = new QListWidgetItem(codecAliasList.join(QLatin1String(" / ")),
                                                     m_ui->charsetListWidget);
    codecItem->setData(internal::SelectCharsetDialog_codecDataRole, codecName);
    if (codecName == iniLastCodecName)
      codecItem->setSelected(true);
  }

  QObject::connect(m_ui->charsetListWidget, &QListWidget::itemDoubleClicked,
                   this, &SelectCharsetDialog::accept);
}

SelectCharsetDialog::~SelectCharsetDialog()
{
  delete m_ui;
}

QByteArray SelectCharsetDialog::selectedCharset() const
{
  const QList<QListWidgetItem*> selectedCodecItems = m_ui->charsetListWidget->selectedItems();
  if (selectedCodecItems.size() == 1) {
    const QListWidgetItem* codecItem = selectedCodecItems.first();
    return codecItem->data(internal::SelectCharsetDialog_codecDataRole).toByteArray();
  }
  return QByteArray();
}

void SelectCharsetDialog::accept()
{
  const QByteArray currentCharset = this->selectedCharset();
  if (!currentCharset.isEmpty()) {
    QSettings appSettings;
    appSettings.setValue(internal::SelectCharsetDialog_lastCodecNameIniKey, currentCharset);
  }

  QDialog::accept();
}
