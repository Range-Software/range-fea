#ifndef SOLVER_MANAGER_H
#define SOLVER_MANAGER_H

#include <QObject>

#include <rbl_job_manager.h>
#include "solver_task.h"

class SolverManager : public RJobManager
{

    Q_OBJECT

    protected:

        //! Last solver log file name.
        QString lastLogFileName;

    public:

        //! Constructor
        explicit SolverManager(QObject *parent = nullptr);

        //! Return static instance of the manager.
        static SolverManager & getInstance();

        //! Add new solver task.
        void submit(SolverTask *solverTask);

        //! Check if solver for given model ID is queued.
        bool isSolverQueued(uint modelID) const;

        //! Check if solver for given model ID is running.
        bool isSolverRunning(uint modelID) const;

        //! Stop current solvers.
        void stopRunningTasks();

        //! Kill current solvers.
        void killRunningTasks();

        //! Return const reference to last log file name.
        const QString & getLastLogFileName() const;

    signals:

        //! Message on standard output is availabe.
        void readyReadStandardOutput(const QString &message);

        //! Message on standard error is availabe.
        void readyReadStandardError(const QString &message);

    public slots:

        //! Message on standard output is availabe.
        void onReadyReadStandardOutput(const QString &message);

        //! Message on standard error is availabe.
        void onReadyReadStandardError(const QString &message);

};

#endif // SOLVER_MANAGER_H
