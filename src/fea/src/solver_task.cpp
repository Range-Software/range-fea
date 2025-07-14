#include <QFileInfo>

#include <rml_file_manager.h>

#include "model_io.h"
#include "solver_manager.h"
#include "solver_task.h"
#include "application.h"

SolverTask::SolverTask(const ApplicationSettings *applicationSettings, uint modelID)
    : applicationSettings{applicationSettings}
    , modelID{modelID}
    , solverProcess{nullptr}
{
    this->taskID.generate();
    this->solverExecutable = applicationSettings->getSolverPath();

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    QFileInfo modelFileInfo(rModel.getFileName());

    this->modelFileName = modelFileInfo.absoluteFilePath();
    this->logFileName = rModel.buildTmpFileName("log",this->taskID.toString());
    this->convergenceFileName = rModel.buildTmpFileName("cvg",this->taskID.toString());
    this->monitoringFileName = rModel.buildTmpFileName("mon",this->taskID.toString());

    this->solverArguments.append("--file=" + this->modelFileName);
    this->solverArguments.append("--log-file=" + this->logFileName);
    this->solverArguments.append("--convergence-file=" + this->convergenceFileName);
    this->solverArguments.append("--monitoring-file=" + this->monitoringFileName);
    this->solverArguments.append("--nthreads=" + QString::number(this->applicationSettings->getNThreads()));
    this->solverArguments.append("--task-id=" + this->taskID.toString());
    this->solverArguments.append("--task-server=" + SolverManager::getInstance().getTaskServerName());
}

uint SolverTask::getModelID() const
{
    return this->modelID;
}

const SolverTaskID &SolverTask::getTaskID() const
{
    return this->taskID;
}

SolverTaskID &SolverTask::getTaskID()
{
    return this->taskID;
}

const QString SolverTask::getStopCommand() const
{
    return QString(this->taskID.toString() + ":STOP");
}

void SolverTask::kill()
{
    RLogger::info("Killing solver task (#%s).\n",this->taskID.toString().toUtf8().constData());
    this->solverProcess->kill();
}

const QString &SolverTask::getLogFileName() const
{
    return this->logFileName;
}

QString SolverTask::getCommandLine() const
{
    QString cmdLine = this->solverExecutable;
    for (int i=0;i<this->solverArguments.size();i++)
    {
        cmdLine += QString(" \"") + this->solverArguments.at(i) + QString("\"");
    }
    return cmdLine;
}

int SolverTask::perform()
{
    try
    {
        Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Execute solver task"));

        RLogger::info("Solver task (#%s) - Begin\n",this->taskID.toString().toUtf8().constData());
        RLogger::indent();

        Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
        QString modelFileName(RFileManager::getFileNameWithOutTimeStep(rModel.getFileName()));

        // Save model first.
        try
        {
            emit this->isBlocking(true);
            ModelIO::save(rModel,modelFileName,false);
            emit this->isBlocking(false);
        }
        catch (const RError &error)
        {
            RLogger::error("%s File: '%s'\n",error.getMessage().toUtf8().constData(),modelFileName.toUtf8().constData());
            emit this->isBlocking(false);
            throw RError(RError::Type::Application,R_ERROR_REF,"Failed to start the solver because model could not be saved.");
        }

        // Execute solver
        QString cmdLine = this->getCommandLine();

        RLogger::info("Executing \'%s\'\n",cmdLine.toUtf8().constData());

        this->solverProcess = new QProcess;

        QObject::connect(this->solverProcess,&QProcess::readyReadStandardOutput,this,&SolverTask::onProcessReadyReadStandardOutput,Qt::DirectConnection);
        QObject::connect(this->solverProcess,&QProcess::readyReadStandardError,this,&SolverTask::onProcessReadyReadStandardError,Qt::DirectConnection);

        this->solverProcess->start(this->solverExecutable,this->solverArguments);

        if (!this->solverProcess->waitForFinished(-1) || this->solverProcess->exitCode() != 0)
        {
            RLogger::warning("Command \'%s\' failed with exit code = %d.\n",cmdLine.toUtf8().constData(),this->solverProcess->exitCode());

            throw RError(RError::Type::Application,R_ERROR_REF,"Solver execution failed.");
        }

        this->solverProcess->deleteLater();

        RLogger::info("Command \'%s\' succesfully finished.\n",cmdLine.toUtf8().constData());

        // Update model.
        try
        {
            emit this->isBlocking(true);
            ModelIO::update(rModel,modelFileName);
            emit this->isBlocking(false);
        }
        catch (const RError &error)
        {
            RLogger::error("%s File: '%s'\n",error.getMessage().toUtf8().constData(),modelFileName.toUtf8().constData());
            emit this->isBlocking(false);
            throw RError(RError::Type::Application,R_ERROR_REF,"Failed to finish the solver because model could not be opened.");
        }

        RLogger::unindent();
        RLogger::info("Solver task (#%s) - End\n",this->taskID.toString().toUtf8().constData());

        Application::instance()->getSession()->setModelChanged(this->modelID);
    }
    catch (const RError &error)
    {
        RLogger::unindent();
        RLogger::error("Solver task (#%s) failed: %s\n",this->taskID.toString().toUtf8().constData(),error.getMessage().toUtf8().constData());
        return 1;
    }
    return 0;
}

void SolverTask::onProcessReadyReadStandardOutput()
{
    emit this->readyReadStandardOutput(this->solverProcess->readAllStandardOutput());
}

void SolverTask::onProcessReadyReadStandardError()
{
    emit this->readyReadStandardError(this->solverProcess->readAllStandardError());
}
