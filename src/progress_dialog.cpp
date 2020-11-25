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

    QObject::connect(m_ui->buttonBox, &QDialogButtonBox::clicked,
                     this, &ProgressDialog::onClicked);
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
        emit canceled();
    }
}
