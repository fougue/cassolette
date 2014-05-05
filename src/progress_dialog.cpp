#include "progress_dialog.h"
#include "ui_progress_dialog.h"

ProgressDialog::ProgressDialog(QWidget *parent)
  : QDialog(parent),
    m_ui(new Ui_ProgressDialog)
{
  m_ui->setupUi(this);
  m_ui->progressBar->setMinimum(0);
  m_ui->progressBar->setValue(0);
  this->setModal(true);

  QObject::connect(this, &QDialog::rejected, this, &ProgressDialog::canceled);
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

int ProgressDialog::maximumValue() const
{
  return m_ui->progressBar->maximum();
}

void ProgressDialog::setMaximumValue(int value)
{
  m_ui->progressBar->setMaximum(value);
}

int ProgressDialog::value() const
{
  return m_ui->progressBar->value();
}

void ProgressDialog::setValue(int value)
{
  m_ui->progressBar->setValue(value);
}

void ProgressDialog::reset()
{
  this->close();
}
