#include "application.h"
#include "solver_manager.h"

SolverManager::SolverManager(QObject *parent)
    : RJobManager(parent)
{
}

SolverManager &SolverManager::getInstance()
{
    static SolverManager solverManager;
    return solverManager;
}

void SolverManager::submit(SolverTask *solverTask)
{
    if (!this->isSolverQueued(solverTask->getModelID()))
    {
        QObject::connect(solverTask,
                         &SolverTask::readyReadStandardOutput,
                         this,
                         &SolverManager::onReadyReadStandardOutput);
        QObject::connect(solverTask,
                         &SolverTask::readyReadStandardError,
                         this,
                         &SolverManager::onReadyReadStandardError);

        this->lastLogFileName = solverTask->getLogFileName();

        this->RJobManager::submit(solverTask);
    }
}

bool SolverManager::isSolverQueued(uint modelID) const
{
    for (int i=0;i<this->waitingJobs.size();i++)
    {
        SolverTask *solverTask = static_cast<SolverTask*>(this->waitingJobs[i]);
        if (solverTask->getModelID() == modelID)
        {
            return true;
        }
    }
    return false;
}

bool SolverManager::isSolverRunning(uint modelID) const
{
    QMutexLocker<QRecursiveMutex> locker(&this->jobsMutex);
    for (int i=0;i<this->runningJobs.size();i++)
    {
        SolverTask *solverTask = static_cast<SolverTask*>(this->runningJobs[i]);
        if (!solverTask->isFinished() && solverTask->getModelID() == modelID)
        {
            return true;
        }
    }
    return false;
}

void SolverManager::stopRunningTasks()
{
    if (!this->runningJobs.isEmpty())
    {
        foreach (RJob *task, this->runningJobs)
        {
            SolverTask *solverTask = dynamic_cast<SolverTask*>(task);
            RLogger::info("Stopping solver task (#%s).\n",solverTask->getTaskID().toString().toUtf8().constData());
            solverTask->stop();
        }
    }
}

void SolverManager::killRunningTasks()
{
    if (!this->runningJobs.isEmpty())
    {
        foreach (RJob *task, this->runningJobs)
        {
            SolverTask *solverTask = dynamic_cast<SolverTask*>(task);
            RLogger::info("Killing solver task (#%s).\n",solverTask->getTaskID().toString().toUtf8().constData());
            solverTask->kill();
        }
    }
}

const QString &SolverManager::getLastLogFileName() const
{
    return this->lastLogFileName;
}

void SolverManager::onReadyReadStandardOutput(const QString &message)
{
    emit this->readyReadStandardOutput(message);
}

void SolverManager::onReadyReadStandardError(const QString &message)
{
    emit this->readyReadStandardError(message);
}
