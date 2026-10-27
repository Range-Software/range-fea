#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <QObject>
#include <QCoreApplication>

class CommandProcessor : public QObject
{

    Q_OBJECT

    protected:

        //! Task ID.
        QString taskID;

    public:

        //! Constructor.
        explicit CommandProcessor(const QString &taskID, QCoreApplication *application = nullptr);

    protected:

        //! Start reading and processing commands from standard input.
        void start(void);

        //! Process single command.
        static void processCommand(const QString &taskID, const QString &command);

};

#endif // COMMAND_PROCESSOR_H
