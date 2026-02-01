#ifndef MAIN_TASK_H
#define MAIN_TASK_H

#include <QObject>
#include <QThread>

class MainTask : public QObject
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit MainTask(QObject *parent = nullptr);

    protected slots:

        //! Run task.
        void run();

};

#endif // MAIN_TASK_H
