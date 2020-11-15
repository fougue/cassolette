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

#include "base_file_task.h"
#include <QtCore/QStringList>

template<typename SECOND_TASK_INPUT> class CompositeFileTaskBridge;

//! \brief Composite of two BaseFileTask
template<typename FIRST_TASK, typename SECOND_TASK>
class CompositeFileTask : public BaseFileTask {
public:
    CompositeFileTask(FIRST_TASK* task1, SECOND_TASK* task2);
    ~CompositeFileTask();

    FIRST_TASK* firstTask() const;
    SECOND_TASK* secondTask() const;

    void asyncExec();
    void abortTask();
    bool isRunning() const;

    typedef typename SECOND_TASK::InputType SecondTaskInputType;
    typedef CompositeFileTaskBridge<SecondTaskInputType> TaskBridge;

    void setTaskBridge(TaskBridge* bridge);

private:
    void onFirstTaskResultItem(const BaseFileTask::ResultItem& resultItem);
    void onFirstTaskFinished();

private:
    FIRST_TASK* m_firstTask;
    SECOND_TASK* m_secondTask;
    TaskBridge* m_bridge;
};

//! \brief Utility function to ease creation of a CompositeFileTask<>
//! \relates CompositeFileTask
template<typename FIRST_TASK, typename SECOND_TASK>
BaseFileTask* newCompositeFileTask(
        FIRST_TASK* task1,
        SECOND_TASK* task2,
        CompositeFileTaskBridge<typename SECOND_TASK::InputType>* bridge = nullptr)
{
    auto* task = new CompositeFileTask<FIRST_TASK, SECOND_TASK>(task1, task2);
    task->setTaskBridge(bridge);
    return task;
}


//! Adapts CompositeFileTask first task output to be reinjected as second task
//! input
template<typename SECOND_TASK_INPUT>
class CompositeFileTaskBridge {
public:
    virtual ~CompositeFileTaskBridge();

    virtual void reset();
    virtual void onFirstTaskResultItem(const BaseFileTask::ResultItem&);
    virtual void onFirstTaskFinished();

    SECOND_TASK_INPUT  secondTaskInput() const;
    SECOND_TASK_INPUT* secondTaskInputPtr();

private:
    SECOND_TASK_INPUT m_secondTaskInput;
};

//! Default CompositeFileTaskBridge for task input being of type "QStringList"
class CompositeFileTaskBridge_QStringList : public CompositeFileTaskBridge<QStringList> {
public:
    void reset();
    void onFirstTaskResultItem(const BaseFileTask::ResultItem& resultItem);
};


// --
// -- Implementation
// --

// CompositeFileTask<>
template<typename FIRST_TASK, typename SECOND_TASK>
CompositeFileTask<FIRST_TASK, SECOND_TASK>::CompositeFileTask(FIRST_TASK* task1, SECOND_TASK* task2)
    : m_firstTask(task1),
      m_secondTask(task2),
      m_bridge(nullptr)
{
    QObject::connect(m_firstTask, &BaseFileTask::taskResultItem,
                     this, &CompositeFileTask::onFirstTaskResultItem);
    QObject::connect(m_secondTask, &BaseFileTask::taskResultItem,
                     this, &BaseFileTask::taskResultItem);

    QObject::connect(m_firstTask, &BaseFileTask::taskAborted,
                     this, &BaseFileTask::taskAborted);
    QObject::connect(m_secondTask, &BaseFileTask::taskAborted,
                     this, &BaseFileTask::taskAborted);

    QObject::connect(m_firstTask, &BaseFileTask::taskFinished,
                     this, &CompositeFileTask::onFirstTaskFinished);
    QObject::connect(m_secondTask, &BaseFileTask::taskFinished,
                     this, &BaseFileTask::taskFinished);
}

template<typename FIRST_TASK, typename SECOND_TASK>
CompositeFileTask<FIRST_TASK, SECOND_TASK>::~CompositeFileTask()
{
    delete m_bridge;
}

template<typename FIRST_TASK, typename SECOND_TASK>
FIRST_TASK* CompositeFileTask<FIRST_TASK, SECOND_TASK>::firstTask() const
{
    return m_firstTask;
}

template<typename FIRST_TASK, typename SECOND_TASK>
SECOND_TASK* CompositeFileTask<FIRST_TASK, SECOND_TASK>::secondTask() const
{
    return m_secondTask;
}

template<typename FIRST_TASK, typename SECOND_TASK>
void CompositeFileTask<FIRST_TASK, SECOND_TASK>::asyncExec()
{
    if (m_bridge != nullptr)
        m_bridge->reset();
    m_firstTask->asyncExec();
}

template<typename FIRST_TASK, typename SECOND_TASK>
void CompositeFileTask<FIRST_TASK, SECOND_TASK>::abortTask()
{
    m_firstTask->abortTask();
    m_secondTask->abortTask();
}

template<typename FIRST_TASK, typename SECOND_TASK>
bool CompositeFileTask<FIRST_TASK, SECOND_TASK>::isRunning() const
{
    return m_firstTask->isRunning() || m_secondTask->isRunning();
}

template<typename FIRST_TASK, typename SECOND_TASK>
void CompositeFileTask<FIRST_TASK, SECOND_TASK>::onFirstTaskResultItem(
        const BaseFileTask::ResultItem& resultItem)
{
    if (!resultItem.hasError()) {
        if (m_bridge != nullptr)
            m_bridge->onFirstTaskResultItem(resultItem);
    }
    else {
        emit taskResultItem(resultItem);
    }
}

template<typename FIRST_TASK, typename SECOND_TASK>
void CompositeFileTask<FIRST_TASK, SECOND_TASK>::onFirstTaskFinished()
{
    if (m_bridge != nullptr) {
        m_bridge->onFirstTaskFinished();
        m_secondTask->setInput(m_bridge->secondTaskInput());
    }
    m_secondTask->asyncExec();
}

template<typename FIRST_TASK, typename SECOND_TASK>
void CompositeFileTask<FIRST_TASK, SECOND_TASK>::setTaskBridge(TaskBridge* bridge)
{
    m_bridge = bridge;
}


// CompositeFileTaskBridge<>

template<typename SECOND_TASK_INPUT>
CompositeFileTaskBridge<SECOND_TASK_INPUT>::~CompositeFileTaskBridge()
{ }

template<typename SECOND_TASK_INPUT>
void CompositeFileTaskBridge<SECOND_TASK_INPUT>::reset()
{ }

template<typename SECOND_TASK_INPUT>
void CompositeFileTaskBridge<SECOND_TASK_INPUT>::onFirstTaskResultItem(
        const BaseFileTask::ResultItem&)
{ }

template<typename SECOND_TASK_INPUT>
void CompositeFileTaskBridge<SECOND_TASK_INPUT>::onFirstTaskFinished()
{ }

template<typename SECOND_TASK_INPUT>
SECOND_TASK_INPUT CompositeFileTaskBridge<SECOND_TASK_INPUT>::secondTaskInput() const
{
    return m_secondTaskInput;
}

template<typename SECOND_TASK_INPUT>
SECOND_TASK_INPUT* CompositeFileTaskBridge<SECOND_TASK_INPUT>::secondTaskInputPtr()
{
    return &m_secondTaskInput;
}
