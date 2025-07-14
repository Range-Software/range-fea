#include <QCoreApplication>
#include <QSettings>

#include <rbl_arguments_parser.h>
#include <rbl_error.h>
#include <rbl_logger.h>
#include <rbl_utils.h>

#include "main_task.h"
#include "solver_task.h"
#include "command_processor.h"

MainTask::MainTask(QObject *parent) :
    QObject(parent)
{
}

void MainTask::run(void)
{
    QCoreApplication *application = (QCoreApplication*)this->parent();

    QSettings settings(RVendor::name(), RVendor::shortName());

    try
    {
//        QString rangeAccount = settings.value("application/rangeAccount").toString();
//        QString rangePassword = settings.value("application/rangePassword").toString();

        // Process command line arguments.
        QList<RArgumentOption> validOptions;
        validOptions.append(RArgumentOption("file",RArgumentOption::Path,QVariant(),"File name",RArgumentOption::Category::Mandatory,false));
        validOptions.append(RArgumentOption("log-file",RArgumentOption::Path,QVariant(),"Log file name",RArgumentOption::Category::Logger,false));
        validOptions.append(RArgumentOption("convergence-file",RArgumentOption::Path,QVariant(),"Convergence file name",RArgumentOption::Category::Optional,false));
        validOptions.append(RArgumentOption("monitoring-file",RArgumentOption::Path,QVariant(),"Monitoring file name",RArgumentOption::Category::Optional,false));
        validOptions.append(RArgumentOption("nthreads",RArgumentOption::Integer,QVariant(1),"Number of threads to use",RArgumentOption::Category::Optional,false));
        validOptions.append(RArgumentOption("restart",RArgumentOption::Switch,QVariant(),"Restart solver",RArgumentOption::Category::Optional,false));
        validOptions.append(RArgumentOption("task-id",RArgumentOption::Path,QVariant(),"Task ID for inter process communication",RArgumentOption::Category::Optional,false));
        validOptions.append(RArgumentOption("task-server",RArgumentOption::Path,QVariant(),"Task server for inter process communication",RArgumentOption::Category::Optional,false));
        validOptions.append(RArgumentOption("log-debug",RArgumentOption::Switch,QVariant(),"Switch on debug log level",RArgumentOption::Category::Logger,false));
        validOptions.append(RArgumentOption("log-trace",RArgumentOption::Switch,QVariant(),"Switch on trace log level",RArgumentOption::Category::Logger,false));

        RArgumentsParser argumentsParser(QCoreApplication::arguments(),validOptions,false);

        if (argumentsParser.isSet("help"))
        {
            argumentsParser.printHelp();
            application->exit(0);
            return;
        }

        if (argumentsParser.isSet("version"))
        {
            argumentsParser.printVersion();
            application->exit(0);
            return;
        }

        if (argumentsParser.isSet("log-debug"))
        {
            RLogger::getInstance().setLevel(R_LOG_LEVEL_DEBUG);
        }
        if (argumentsParser.isSet("log-trace"))
        {
            RLogger::getInstance().setLevel(R_LOG_LEVEL_TRACE);
        }
        if (argumentsParser.isSet("log-file"))
        {
            RLogger::getInstance().setFile(argumentsParser.getValue("log-file").toString());
        }

        // Command processor.
        new CommandProcessor(argumentsParser.getValue("task-server").toString(),
                             argumentsParser.getValue("task-id").toString(),
                             application);

        SolverInput solverInput(argumentsParser.getValue("file").toString());
        if (argumentsParser.isSet("convergence-file"))
        {
            solverInput.setConvergenceFileName(argumentsParser.getValue("convergence-file").toString());
        }
        if (argumentsParser.isSet("monitoring-file"))
        {
            solverInput.setMonitoringFileName(argumentsParser.getValue("monitoring-file").toString());
        }
        if (argumentsParser.isSet("nthreads"))
        {
            solverInput.setNThreads(argumentsParser.getValue("nthreads").toUInt());
        }
        if (argumentsParser.isSet("restart"))
        {
            solverInput.setRestart(true);
        }

        // Start solver.
        QThread* thread = new QThread;
        SolverTask *solverTask = new SolverTask(solverInput, application);

        solverTask->moveToThread(thread);

        QObject::connect(thread, &QThread::started, solverTask, &SolverTask::run);
        QObject::connect(solverTask, &SolverTask::finished, solverTask, &SolverTask::deleteLater);
        QObject::connect(solverTask, &SolverTask::destroyed, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        QObject::connect(thread, &QThread::finished, application, &QCoreApplication::quit);

        thread->start();
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to start solver. %s\n",error.getMessage().toUtf8().constData());
        application->exit(1);
    }
}
