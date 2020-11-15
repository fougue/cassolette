/****************************************************************************
** CharSet Tool
** Copyright Fougue Ltd. (15 Apr. 2014)
** contact@fougsys.fr
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

#include <QtWidgets/QWidget>

class LogWidget : public QWidget {
    Q_OBJECT

public:
    enum LogFormat {
        InfoLog,
        WarningLog,
        ErrorLog
    };

    LogWidget(QWidget *parent = nullptr);
    ~LogWidget();

    void clearLog();
    void appendLogInfo(const QString& msg);
    void appendLogWarning(const QString& msg);
    void appendLogError(const QString& msg);
    void appendLog(const QString& msg, LogFormat format);

private:
    class Ui_LogWidget *m_ui;
};
