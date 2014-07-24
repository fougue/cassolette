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

#include "input_filter_dialog.h"
#include "ui_input_filter_dialog.h"

namespace Internal {

static QStringList allTexts(const QListWidget* listWidget)
{
    QStringList textList;
    const int rowCount = listWidget->count();
    for (int row = 0; row < rowCount; ++row)
        textList += listWidget->item(row)->text();
    return textList;
}

static void installTexts(QListWidget* listWidget, const QStringList& textList)
{
    listWidget->clear();
    foreach (const QString& text, textList)
        listWidget->addItem(text);
}

} // namespace Internal

InputFilterDialog_FilePatterns::InputFilterDialog_FilePatterns()
    : checked(true)
{
}

QStringList InputFilterDialog_FilePatterns::appliablePatterns() const
{
    return this->checked ? this->patterns : QStringList();
}

InputFilterDialog::InputFilterDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::InputFilterDialog)
{
    m_ui->setupUi(this);

    QObject::connect(m_ui->addFilterBtn, SIGNAL(clicked()), this, SLOT(addFilterPattern()));
    QObject::connect(m_ui->addExcludeBtn, SIGNAL(clicked()), this, SLOT(addExcludePattern()));

    QObject::connect(m_ui->filterCheckBox, SIGNAL(toggled(bool)), m_ui->addFilterBtn, SLOT(setEnabled(bool)));
    QObject::connect(m_ui->filterCheckBox, SIGNAL(toggled(bool)), m_ui->filterListWidget, SLOT(setEnabled(bool)));

    QObject::connect(m_ui->excludeCheckBox, SIGNAL(toggled(bool)), m_ui->addExcludeBtn, SLOT(setEnabled(bool)));
    QObject::connect(m_ui->excludeCheckBox, SIGNAL(toggled(bool)), m_ui->excludeListWidget, SLOT(setEnabled(bool)));
}

InputFilterDialog::~InputFilterDialog()
{
    delete m_ui;
}

void InputFilterDialog::installFilterPatterns(const FilePatterns &fp)
{
    Internal::installTexts(m_ui->filterListWidget, fp.patterns);
    m_ui->filterCheckBox->setChecked(fp.checked);
}

InputFilterDialog::FilePatterns InputFilterDialog::filterPatterns() const
{
    FilePatterns fp;
    fp.checked = m_ui->filterCheckBox->isChecked();
    fp.patterns = Internal::allTexts(m_ui->filterListWidget);
    return fp;
}

void InputFilterDialog::installExcludePatterns(const FilePatterns &fp)
{
    Internal::installTexts(m_ui->excludeListWidget, fp.patterns);
    m_ui->excludeCheckBox->setChecked(fp.checked);
}

InputFilterDialog::FilePatterns InputFilterDialog::excludePatterns() const
{
    FilePatterns fp;
    fp.checked = m_ui->excludeCheckBox->isChecked();
    fp.patterns = Internal::allTexts(m_ui->excludeListWidget);
    return fp;
}

void InputFilterDialog::addFilterPattern()
{
    QListWidgetItem* item = new QListWidgetItem("<...>");
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    m_ui->filterListWidget->addItem(item);
    m_ui->filterListWidget->editItem(item);
}

void InputFilterDialog::addExcludePattern()
{
    QListWidgetItem* item = new QListWidgetItem("<...>");
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    m_ui->excludeListWidget->addItem(item);
    m_ui->excludeListWidget->editItem(item);
}
