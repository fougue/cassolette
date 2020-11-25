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

#pragma once

#include <QtWidgets/QDialog>
class QAbstractButton;

/*! Similar to QProgressDialog
 *
 *  It is lighter than QProgressDialog that comes with an internal QTimer, app
 *  processing, ...
 *  But the main reason QProgressDialog is not used are the frequent crashs with
 *  Windows, even with recent Qt.
 */
class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    QString labelText() const;
    void setLabelText(const QString& text);

    int minimumValue() const;
    int maximumValue() const;
    void setMinimumValue(int value);
    void setMaximumValue(int value);
    void setRangeValue(int min, int max);

    int value() const;
    void setValue(int value);

    bool wasCanceled() const;

    void reset();
    void resetCancelFlag();

signals:
    void canceled();

private:
    void onClicked(QAbstractButton* btn);

    class Ui_ProgressDialog *m_ui;
    bool m_wasCanceled;
};
