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

#ifndef COMPOSITE_FILE_TASK_H
#define COMPOSITE_FILE_TASK_H

#include "base_file_task.h"
#include <QtCore/QStringList>

template<typename SECOND_TASK_INPUT> class CompositeFileTaskBridge;

//! \brief Composite of two BaseFileTask
template<typename FIRST_TASK, typename SECOND_TASK>
class CompositeFileTask : public BaseFileTask
{
public:
    CompositeFileTask(FIRST_TASK* task1, SECOND_TASK* task2);
    ~CompositeFileTask();

    FIRST_TASK* firstTask() const;
    SECOND_TASK* secondTask() const;

    void asyncExec();
    void abortTask();

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
BaseFileTask* newCompositeFileTask(FIRST_TASK* task1,
                                   SECOND_TASK* task2,
                                   CompositeFileTaskBridge<typename SECOND_TASK::InputType>* bridge = nullptr)
{
    auto* task = new CompositeFileTask<FIRST_TASK, SECOND_TASK>(task1, task2);
    task->setTaskBridge(bridge);
    return task;
}


//! \brief Adapts CompositeFileTask first task output to be reinjected as second task input
template<typename SECOND_TASK_INPUT>
class CompositeFileTaskBridge
{
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

//! \brief Default CompositeFileTaskBridge for task input being of type "QStringList"
class CompositeFileTaskBridge_QStringList : public CompositeFileTaskBridge<QStringList>
{
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
void CompositeFileTask<FIRST_TASK, SECOND_TASK>::onFirstTaskResultItem(const BaseFileTask::ResultItem& resultItem)
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
void CompositeFileTaskBridge<SECOND_TASK_INPUT>::onFirstTaskResultItem(const BaseFileTask::ResultItem&)
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

#endif // COMPOSITE_FILE_TASK_H
