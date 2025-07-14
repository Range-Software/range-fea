#ifndef SOLVER_INPUT_H
#define SOLVER_INPUT_H

#include <QString>

class SolverTask;

class SolverInput
{

    protected:

        //! Model file.
        QString modelFileName;
        //! Convergence file.
        QString convergenceFileName;
        //! Monitoring file.
        QString monitoringFileName;
        //! Number of threads to use.
        uint nThreads;
        //! Force to restart solver
        bool restart;

    private:

        //! Internal initialization function.
        void _init(const SolverInput *pSolverInput = nullptr);

    public:

        //! Constructor.
        SolverInput(const QString &modelFileName);

        //! Copy constructor.
        SolverInput(const SolverInput &solverInput);

        //! Destructor.
        ~SolverInput();

        //! Assignment operator.
        SolverInput & operator =(const SolverInput &solverInput);

        //! Set convergence file name.
        void setConvergenceFileName(const QString &convergenceFileName);

        //! Set monitoring file name.
        void setMonitoringFileName(const QString &monitoringFileName);

        //! Set number of threads to use.
        void setNThreads(uint nThreads);

        //! Set force restart solver.
        void setRestart(bool restart);

        friend class SolverTask;

};

#endif // SOLVER_INPUT_H
