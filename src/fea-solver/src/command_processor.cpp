#include <iostream>
#include <string>
#include <thread>

#include <rbl_application_state.h>
#include <rbl_logger.h>

#include "command_processor.h"

CommandProcessor::CommandProcessor(const QString &taskID, QCoreApplication *application)
    : QObject(application)
    , taskID(taskID)
{
    if (!this->taskID.isEmpty())
    {
        this->start();
    }
}

void CommandProcessor::start(void)
{
    // Commands arrive one per line on standard input, which is a pipe when the solver
    // is started by the application. No socket is used because its path inside the
    // App Sandbox container would exceed the length allowed by macOS.
    // Reading blocks and cannot be interrupted, therefore the thread is detached and
    // ends with the end of input or with the process.
    QString taskID(this->taskID);
    std::thread([taskID]()
    {
        std::string line;
        while (std::getline(std::cin,line))
        {
            CommandProcessor::processCommand(taskID,QString::fromStdString(line).trimmed());
        }
    }).detach();
    RLogger::info("Reading commands from standard input\n");
}

void CommandProcessor::processCommand(const QString &taskID, const QString &command)
{
    if (command.isEmpty())
    {
        return;
    }
    RLogger::info("Received command: \'%s\'\n",command.toUtf8().constData());
    if (command == taskID + ":STOP")
    {
        RApplicationState::getInstance().setStateType(R_APPLICATION_STATE_STOP);
    }
}
