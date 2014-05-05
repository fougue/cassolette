#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include <QtWidgets/QDialog>

/*! \brief Similar to QProgressDialog
 *
 * It is lighter than QProgressDialog that comes with an internal QTimer, app processing, ...
 * But the main reason QProgressDialog is not used are the frequent crashs with Windows, even
 * with recent Qt.
 */
class ProgressDialog : public QDialog
{
  Q_OBJECT

public:
  ProgressDialog(QWidget *parent = NULL);
  ~ProgressDialog();

  QString labelText() const;
  void setLabelText(const QString& text);

  int maximumValue() const;
  void setMaximumValue(int value);

  int value() const;
  void setValue(int value);

public slots:
  void reset();

signals:
  void canceled();

private:
  class Ui_ProgressDialog *m_ui;
};

#endif // PROGRESS_DIALOG_H
