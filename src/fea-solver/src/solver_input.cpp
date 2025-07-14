#include "solver_input.h"

void SolverInput::_init(const SolverInput *pSolverInput)
{
    if (pSolverInput)
    {
        this->modelFileName = pSolverInput->modelFileName;
        this->convergenceFileName = pSolverInput->convergenceFileName;
        this->restart = pSolverInput->restart;
    }
}

SolverInput::SolverInput(const QString &modelFileName)
    : modelFileName(modelFileName)
    , restart(false)
{
    this->_init();
}

SolverInput::SolverInput(const SolverInput &solverInput)
{
    this->_init(&solverInput);
}

SolverInput::~SolverInput()
{

}

SolverInput &SolverInput::operator =(const SolverInput &solverInput)
{
    this->_init(&solverInput);
    return (*this);
}

void SolverInput::setConvergenceFileName(const QString &convergenceFileName)
{
    this->convergenceFileName = convergenceFileName;
}

void SolverInput::setMonitoringFileName(const QString &monitoringFileName)
{
    this->monitoringFileName = monitoringFileName;
}

void SolverInput::setNThreads(uint nThreads)
{
    this->nThreads = nThreads;
}

void SolverInput::setRestart(bool restart)
{
    this->restart = restart;
}
