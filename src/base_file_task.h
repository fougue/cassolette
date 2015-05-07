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

#include <QtCore/QObject>
#include <QtCore/QVariant>
template<typename T> class QFutureWatcher;

class BaseFileTask : public QObject
{
    Q_OBJECT

public:
    typedef void InputType;

    struct ResultItem
    {
        QString  filePath;
        QVariant payload;
        QString  errorText;
        bool hasError() const;

        static ResultItem createPayload(const QString& pFilePath);
        static ResultItem createPayload(
                const QString& pFilePath, const QVariant& pPayload);
        static ResultItem createError(
                const QString& pFilePath, const QString& pErrorText);
    };

    BaseFileTask(QObject* parent = nullptr);
    ~BaseFileTask();

    int inputSize() const;

    virtual void asyncExec();
    virtual void abortTask();
    virtual bool isRunning() const;

signals:
    void taskStarted();

    void taskResultItem(const BaseFileTask::ResultItem& item);
    void taskProgressRangeChanged(int min, int max);
    void taskProgressValueChanged(int value);

    void taskFinished();
    void taskAborted();

protected:
    typedef QFutureWatcher<BaseFileTask::ResultItem> FutureWatcher;
    void createFutureWatcher();
    QFutureWatcher<BaseFileTask::ResultItem> *futureWatcher() const;

    virtual void onTaskResultReadyAt(int resultId);

    void setInputSize(int size);

    bool abortRequested() const;
    void endAbortRequest();

private:
    void onFutureFinished();

    QFutureWatcher<BaseFileTask::ResultItem>* m_futureWatcher;
    int m_inputSize;
    bool m_abortRequested;
};
