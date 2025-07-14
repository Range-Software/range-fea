#include <locale.h>
#include <QTimer>

#include "application.h"
#include "main_task.h"

Application::Application(int &argc, char **argv) :
    QCoreApplication(argc,argv)
{
    // Needed for printf functions family to work correctly.
    setlocale(LC_ALL,"C");

    QObject::connect(this,&Application::started,this,&Application::onStarted);

    QTimer::singleShot(0, this, SIGNAL(started()));
}

void Application::onStarted(void)
{
    MainTask *mainTask = new MainTask(this);
    QTimer::singleShot(0, mainTask, SLOT(run()));
}
