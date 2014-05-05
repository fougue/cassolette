#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include <QtWidgets/QDialog>
class QAbstractButton;

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

  int minimumValue() const;
  int maximumValue() const;
  Q_SLOT void setMinimumValue(int value);
  Q_SLOT void setMaximumValue(int value);
  Q_SLOT void setRangeValue(int min, int max);

  int value() const;
  Q_SLOT void setValue(int value);

  bool wasCanceled() const;

public slots:
  void reset();
  void resetCancelFlag();

signals:
  void canceled();

private slots:
  void onClicked(QAbstractButton* btn);

private:
  class Ui_ProgressDialog *m_ui;
  bool m_wasCanceled;
};

#endif // PROGRESS_DIALOG_H
