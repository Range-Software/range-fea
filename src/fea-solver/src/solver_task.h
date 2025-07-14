#ifndef SOLVER_TASK_H
#define SOLVER_TASK_H

#include <QObject>
#include <QCoreApplication>

#include "solver_input.h"

class SolverTask : public QObject
{

    Q_OBJECT

    protected:

        //! Model file.
        QString modelFileName;
        //! Convergence file name.
        QString convergenceFileName;
        //! Monitoring file name.
        QString monitoringFileName;
        //! Number of threads to use.
        uint nThreads;
        //! Force to restart solver
        bool restart;
        //! Pointer to application object.
        QCoreApplication *app;

    public:

        //! Constructor.
        explicit SolverTask(const SolverInput &solverInput, QCoreApplication *app, QObject *parent = nullptr);

    signals:

        //! Signal that solver has finished.
        void finished(void);
        
    public slots:

        //! Run task.
        void run(void);

};

#endif // SOLVER_TASK_H
