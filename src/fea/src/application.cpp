#include <rml_file_manager.h>

#include <rgl_message_box.h>

#include "application.h"
#include "action.h"
#include "solver_manager.h"

Application::Application(int &argc, char **argv)
    : RApplication{argc,argv,false}
{
    // Set translator names
    this->translatorNames << "fea";
    this->translatorNames << "range-gui-lib";
    this->translatorNames << "range-cloud-lib";

    this->applicationSettings = new ApplicationSettings(this);
    this->applicationSettings->setDirApplicationPath(this->applicationDirPath());

    RJobSettings defaultJobSettings = RJob::getDefaultJobSettings();
    defaultJobSettings.registerEmitMutex(Application::instance()->getSession()->getMutex());
    defaultJobSettings.setNOmpThreads(this->getApplicationSettings()->getNThreads());
    RJob::setDefaultJobSettings(defaultJobSettings);
}

Application *Application::instance() noexcept
{
    return qobject_cast<Application*>(RApplication::instance());
}

const ApplicationSettings *Application::getApplicationSettings() const
{
    return qobject_cast<ApplicationSettings*>(this->applicationSettings);
}

ApplicationSettings *Application::getApplicationSettings()
{
    return qobject_cast<ApplicationSettings*>(this->applicationSettings);
}

const ActionList *Application::getActionList() const
{
    return qobject_cast<ActionList*>(this->actionList);
}

ActionList *Application::getActionList()
{
    return qobject_cast<ActionList*>(this->actionList);
}

const Session *Application::getSession() const
{
    return this->session;
}

Session *Application::getSession()
{
    return this->session;
}

const MainWindow *Application::getMainWindow() const
{
    return qobject_cast<MainWindow*>(this->mainWindow);
}

MainWindow *Application::getMainWindow()
{
    return qobject_cast<MainWindow*>(this->mainWindow);
}

void Application::initialize()
{
    // Initialize action list
    this->actionList = new ActionList(Action::generateActionDefinitionList(),this);

    // Initialize session
    this->session = new Session(this);

    // Prepare main window
    this->mainWindow = new MainWindow(this->session,this->cloudSessionManager,this->applicationSettings,this->getActionList());
    this->cloudConnectionHandler->setMessageBoxParentWidget(this->mainWindow);
    this->mainWindow->show();

    this->userAppInfo.append(QString("Materials directory: \"%1\"").arg(this->getApplicationSettings()->getMaterialsDir()));

    this->updaterSourceDirs.append("materials");

    if (!this->filesToLoad.isEmpty())
    {
        // Load files
        try
        {
            Application::instance()->getSession()->readModels(this->filesToLoad);
        }
        catch (const RError &rError)
        {
            RLogger::warning("Failed to read model file(s). ERROR: %s\n",rError.getMessage().toUtf8().constData());
        }
    }
    else
    {
        // Load session
        QString sessionFileName = this->getApplicationSettings()->getSessionFileName();
        if (!sessionFileName.isEmpty() && RFileManager::fileExists(sessionFileName))
        {
            try
            {
                Application::instance()->getSession()->read(sessionFileName);
            }
            catch (const RError &rError)
            {
                RLogger::warning("Failed to read the session file \'%s\'. ERROR: %s\n",sessionFileName.toUtf8().constData(),rError.getMessage().toUtf8().constData());
            }
        }
    }

    // Check help directory
    if (this->applicationSettings->findHelpDir() != this->applicationSettings->getHelpDir())
    {
        QString messageStr = tr("Default and configured help directories differ") + ":"
                           + QString("<ul>")
                           + "<li><tt>" + this->applicationSettings->findHelpDir() + "</tt></li>"
                           + "<li><tt>" + this->applicationSettings->getHelpDir() + "</tt></li>"
                           + QString("</ul>")
                           + "<strong>" + tr("Would you like to set it to default?") + "</strong>";
        if (RMessageBox::question(Application::instance()->getMainWindow(),tr("Unusual configuration"),messageStr) == RMessageBox::Yes)
        {
            this->applicationSettings->setHelpDir(this->applicationSettings->findHelpDir());
        }
    }
    // Check solver path
    if (this->getApplicationSettings()->getDefaultSolverPath() != this->getApplicationSettings()->getSolverPath())
    {
        QString messageStr = tr("Default and configured paths to solver executable differ") + ":"
                           + QString("<ul>")
                           + "<li><tt>" + this->getApplicationSettings()->getDefaultSolverPath() + "</tt></li>"
                           + "<li><tt>" + this->getApplicationSettings()->getSolverPath() + "</tt></li>"
                           + QString("</ul>")
                           + "<strong>" + tr("Would you like to set it to default?") + "</strong>";
        if (RMessageBox::question(Application::instance()->getMainWindow(),tr("Unusual configuration"),messageStr) == RMessageBox::Yes)
        {
            this->getApplicationSettings()->setSolverPath(this->getApplicationSettings()->getDefaultSolverPath());
        }
    }

    this->getActionList()->processAvailability();
}

void Application::finalize()
{
    // Store session
    QString sessionFileName = Application::instance()->getSession()->getFileName();
    if (sessionFileName.isEmpty())
    {
        sessionFileName = Application::instance()->getSession()->getDefaultFileName();
    }
    try
    {
        Application::instance()->getSession()->write(sessionFileName);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to write the session file \'%s\'. ERROR: %s\n",sessionFileName.toUtf8().constData(),error.getMessage().toUtf8().constData());
    }

    // Stop solver manager server.
    RLogger::info("Stoping solver task server\n");
    SolverManager::getInstance().stopServer();
}
