#include <omp.h>

#include <algorithm>

#include <rml_file_manager.h>

#include <rsolver.h>

#include "solver_task.h"

SolverTask::SolverTask(const SolverInput &solverInput, QCoreApplication *app, QObject *parent)
    : QObject(parent)
    , modelFileName(solverInput.modelFileName)
    , convergenceFileName(solverInput.convergenceFileName)
    , monitoringFileName(solverInput.monitoringFileName)
    , nThreads(solverInput.nThreads)
    , restart(solverInput.restart)
    , app(app)
{
    this->nThreads = std::max(this->nThreads,uint(1));
    this->nThreads = std::min(this->nThreads,uint(omp_get_max_threads()));
}

void SolverTask::run(void)
{
    RLogger::info("Threads = %u (max=%d)\n", this->nThreads, omp_get_max_threads());
    omp_set_num_threads(this->nThreads);

    RModel model;

    // Load model
    try
    {
        model.read(this->modelFileName);
        RLogger::info("Model has been successfully loaded from file \'%s\'.\n", this->modelFileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to load model from file \'%s\'. %s\n", this->modelFileName.toUtf8().constData(), error.getMessage().toUtf8().constData());

        emit this->finished();
        this->app->exit(1);
        return;
    }

    model.getMatrixSolverConf(RMatrixSolverConf::CG).setOutputFileName(RFileManager::getFileNameWithSuffix(this->convergenceFileName,RMatrixSolverConf::getId(RMatrixSolverConf::CG)));
    model.getMatrixSolverConf(RMatrixSolverConf::GMRES).setOutputFileName(RFileManager::getFileNameWithSuffix(this->convergenceFileName,RMatrixSolverConf::getId(RMatrixSolverConf::GMRES)));
    model.getMonitoringPointManager().setOutputFileName(this->monitoringFileName);
    if (this->restart)
    {
        model.getProblemSetup().setRestart(true);
    }

    // Solve model
    try
    {
        RLogger::info("\n");
        RLogger::info("********* SOLVER - BEGIN *********\n");
        RLogger::info("\n");
        RLogger::indent();
        RSolver solver(model,this->modelFileName,this->convergenceFileName);
        solver.run();
        RLogger::unindent();
        RLogger::info("\n");
        RLogger::info("********** SOLVER - END **********\n");
        RLogger::info("\n");
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Failed to solve model from file \'%s\'. %s\n", this->modelFileName.toUtf8().constData(), error.getMessage().toUtf8().constData());

        emit this->finished();
        this->app->exit(1);
        return;
    }

    // Save model
    try
    {
        this->modelFileName = model.write(this->modelFileName);
        RLogger::info("Model has been successfully saved to file \'%s\'.\n", this->modelFileName.toUtf8().constData());
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to save model to file \'%s\'. %s\n", this->modelFileName.toUtf8().constData(), error.getMessage().toUtf8().constData());

        emit this->finished();
        this->app->exit(1);
        return;
    }

    emit this->finished();
}
