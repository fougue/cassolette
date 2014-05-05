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
  this->setModal(true);

  QObject::connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &ProgressDialog::onClicked);
}

ProgressDialog::~ProgressDialog()
{
  delete m_ui;
}

QString ProgressDialog::labelText() const
{
  return m_ui->label->text();
}

void ProgressDialog::setLabelText(const QString &text)
{
  m_ui->label->setText(text);
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
}

void ProgressDialog::resetCancelFlag()
{
  m_wasCanceled = false;
}

void ProgressDialog::onClicked(QAbstractButton *btn)
{
  if (btn == m_ui->buttonBox->button(QDialogButtonBox::Abort))
    m_wasCanceled = true;
}
