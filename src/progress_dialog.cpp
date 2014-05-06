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

#include "progress_dialog.h"
#include "ui_progress_dialog.h"

#include <QtCore/QtDebug>
#include <QtWidgets/QPushButton>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_ProgressDialog),
      m_wasCanceled(false)
{
    m_ui->setupUi(this);
    m_ui->progressBar->setMinimum(0);
    m_ui->progressBar->setValue(0);
    m_ui->stackedWidget->setCurrentWidget(m_ui->progressPage);
    this->setModal(true);

    QObject::connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &ProgressDialog::onClicked);
}

ProgressDialog::~ProgressDialog()
{
    delete m_ui;
}

QString ProgressDialog::labelText() const
{
    return m_ui->progressLabel->text();
}

void ProgressDialog::setLabelText(const QString &text)
{
    m_ui->progressLabel->setText(text);
}

int ProgressDialog::minimumValue() const
{
    return m_ui->progressBar->minimum();
}

int ProgressDialog::maximumValue() const
{
    return m_ui->progressBar->maximum();
}

void ProgressDialog::setMinimumValue(int value)
{
    m_ui->progressBar->setMinimum(value);
}

void ProgressDialog::setMaximumValue(int value)
{
    m_ui->progressBar->setMaximum(value);
}

void ProgressDialog::setRangeValue(int min, int max)
{
    m_ui->progressBar->setRange(min, max);
}

int ProgressDialog::value() const
{
    return m_ui->progressBar->value();
}

void ProgressDialog::setValue(int value)
{
    m_ui->progressBar->setValue(value);
}

bool ProgressDialog::wasCanceled() const
{
    return m_wasCanceled;
}

void ProgressDialog::reset()
{
    this->close();
    m_ui->stackedWidget->setCurrentWidget(m_ui->progressPage);
}

void ProgressDialog::resetCancelFlag()
{
    m_wasCanceled = false;
}

void ProgressDialog::onClicked(QAbstractButton *btn)
{
    if (btn == m_ui->buttonBox->button(QDialogButtonBox::Abort)) {
        m_ui->stackedWidget->setCurrentWidget(m_ui->abortPage);
        m_wasCanceled = true;
    }
}
