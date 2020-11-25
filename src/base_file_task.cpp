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

#include "base_file_task.h"

#include <QtCore/QFutureWatcher>
#include <QtCore/QtDebug>

BaseFileTask::BaseFileTask(QObject *parent)
    : QObject(parent),
      m_futureWatcher(nullptr),
      m_inputSize(0),
      m_abortRequested(false)
{
    qRegisterMetaType<BaseFileTask::ResultItem>("BaseFileTask::ResultItem");
}

BaseFileTask::~BaseFileTask()
{
}

int BaseFileTask::inputSize() const
{
    return m_inputSize;
}

//! \brief Does nothing by default
void BaseFileTask::asyncExec()
{
}

void BaseFileTask::abortTask()
{
    if (this->isRunning() && !m_abortRequested) {
        m_abortRequested = true;

        if (m_futureWatcher != nullptr
                && m_futureWatcher->isRunning()
                && !m_futureWatcher->isCanceled())
        {
            m_futureWatcher->cancel();
        }
    }
}

bool BaseFileTask::isRunning() const
{
    return m_futureWatcher != nullptr ? m_futureWatcher->isRunning() : false;
}

void BaseFileTask::onTaskResultReadyAt(int resultId)
{
    const BaseFileTask::ResultItem res = m_futureWatcher->resultAt(resultId);
    emit taskResultItem(res);
}

void BaseFileTask::setInputSize(int size)
{
    m_inputSize = size;
}

bool BaseFileTask::abortRequested() const
{
    return m_abortRequested;
}

void BaseFileTask::endAbortRequest()
{
    m_abortRequested = false;
    emit taskAborted();
}

void BaseFileTask::onFutureFinished()
{
    if (!m_abortRequested)
        emit taskFinished();
    else
        this->endAbortRequest();
}

void BaseFileTask::createFutureWatcher()
{
    if (m_futureWatcher == nullptr) {
        m_futureWatcher = new QFutureWatcher<ResultItem>(this);
        QObject::connect(
                    m_futureWatcher, &QFutureWatcher<ResultItem>::started,
                    this, &BaseFileTask::taskStarted);
        QObject::connect(
                    m_futureWatcher, &QFutureWatcher<ResultItem>::finished,
                    this, &BaseFileTask::onFutureFinished);
        QObject::connect(
                    m_futureWatcher, &QFutureWatcher<ResultItem>::resultReadyAt,
                    this, &BaseFileTask::onTaskResultReadyAt);

        QObject::connect(
                    m_futureWatcher, &FutureWatcher::progressRangeChanged,
                    this, &BaseFileTask::taskProgressRangeChanged);
        QObject::connect(
                    m_futureWatcher, &FutureWatcher::progressValueChanged,
                    this, &BaseFileTask::taskProgressValueChanged);
    }
}

QFutureWatcher<BaseFileTask::ResultItem>* BaseFileTask::futureWatcher() const
{
    return m_futureWatcher;
}


bool BaseFileTask::ResultItem::hasError() const
{
    return !errorText.isEmpty();
}

BaseFileTask::ResultItem BaseFileTask::ResultItem::createPayload(const QString &pFilePath)
{
    BaseFileTask::ResultItem item;
    item.filePath = pFilePath;
    item.payload = pFilePath;
    return item;
}

BaseFileTask::ResultItem BaseFileTask::ResultItem::createPayload(
        const QString &pFilePath, const QVariant &pPayload)
{
    BaseFileTask::ResultItem item;
    item.filePath = pFilePath;
    item.payload = pPayload;
    return item;
}

BaseFileTask::ResultItem BaseFileTask::ResultItem::createError(
        const QString &pFilePath, const QString &pErrorText)
{
    BaseFileTask::ResultItem item;
    item.filePath = pFilePath;
    item.errorText = pErrorText;
    return item;
}
