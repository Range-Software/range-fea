#ifndef SOLVER_TASK_H
#define SOLVER_TASK_H

#include <QProcess>

#include <rbl_job.h>

#include "application_settings.h"
#include "solver_task_id.h"

class SolverTask : public RJob
{

    Q_OBJECT

    protected:

        //! Application settings.
        const ApplicationSettings *applicationSettings;
        //! Model ID
        uint modelID;
        //! Solver task ID.
        SolverTaskID taskID;
        //! Solver process.
        QProcess *solverProcess;
        //! Solver executable.
        QString solverExecutable;
        //! Solver executable arguments.
        QStringList solverArguments;
        //! Model file name.
        QString modelFileName;
        //! Log file name.
        QString logFileName;
        //! Convergence file name.
        QString convergenceFileName;
        //! Monitoring file name.
        QString monitoringFileName;

    public:

        //! Constructor.
        explicit SolverTask(const ApplicationSettings *applicationSettings, uint modelID);

        //! Return model ID.
        uint getModelID() const;

        //! Return const reference to solver task ID.
        const SolverTaskID & getTaskID() const;

        //! Return reference to solver task ID.
        SolverTaskID & getTaskID();

        //! Return const reference to solver task ID.
        const QString getStopCommand() const;

        //! Kill task.
        void kill();

        //! Return log file name.
        const QString & getLogFileName() const;

        //! Return command line.
        QString getCommandLine() const;

    protected:

        //! Run job.
        virtual int perform() override final;

    private slots:

        //! Catch ready read standard output signal.
        void onProcessReadyReadStandardOutput();

        //! Catch ready read standard error signal.
        void onProcessReadyReadStandardError();

    signals:

        //! Message on standard output is availabe.
        void readyReadStandardOutput(QString message);

        //! Message on standard error is availabe.
        void readyReadStandardError(QString message);

};

#endif // SOLVER_TASK_H
