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
class QSortFilterProxyModel;

class SelectCharsetDialog : public QDialog {
    Q_OBJECT

public:
    SelectCharsetDialog(QWidget *parent = nullptr);
    ~SelectCharsetDialog();

    QByteArray selectedCharset() const;

    void accept() override;

private:
    void onFilterChanged(const QString& filter);

    class Ui_SelectCharsetDialog *m_ui;
    QSortFilterProxyModel* m_filterCodecModel;
};
