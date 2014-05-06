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
