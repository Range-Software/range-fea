#include <QDataStream>

#include <rbl_application_state.h>
#include <rbl_logger.h>

#include "command_processor.h"

CommandProcessor::CommandProcessor(const QString &taskServer, const QString &taskID, QCoreApplication *application)
    : QObject(application)
    , localSocket(new QLocalSocket(this))
    , taskServer(taskServer)
    , taskID(taskID)
{
    if (!this->taskServer.isEmpty() && !this->taskID.isEmpty())
    {
        this->start();
    }
}

void CommandProcessor::start(void)
{
    this->localSocket->connectToServer(taskServer);
    if (!this->localSocket->waitForConnected())
    {
        RLogger::error("Failed to connect to: %s\n",taskServer.toUtf8().constData());
    }
    RLogger::info("Connected to: %s\n",this->localSocket->serverName().toUtf8().constData());
    QObject::connect(this->localSocket, &QLocalSocket::readyRead, this, &CommandProcessor::readSocket);
}

void CommandProcessor::stop(void)
{
    this->localSocket->disconnectFromServer();
}

void CommandProcessor::readSocket(void)
{
    if (this->localSocket->bytesAvailable() < (int)sizeof(quint32))
    {
        return;
    }
    QDataStream in;
    in.setDevice(this->localSocket);
    in.setVersion(QDataStream::Qt_6_8);
    quint32 blockSize;
    in >> blockSize;
    if (this->localSocket->bytesAvailable() < blockSize || in.atEnd())
    {
        return;
    }
    QString message;
    in >> message;
    RLogger::info("Received command: \'%s\'\n",message.toUtf8().constData());
    if (message.contains(this->taskID + ":STOP"))
    {
        RApplicationState::getInstance().setStateType(R_APPLICATION_STATE_STOP);
    }
}
