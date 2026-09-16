#include <rml_file_utils.h>

#include <rgl_message_box.h>

#include "application.h"
#include "action.h"
#include "render_backend.h"
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

QList<RArgumentOption> Application::getAdditionalArgumentOptions() const
{
    QList<RArgumentOption> options;

    // The stored application settings provide the default; the command line
    // overrides it for a single run without changing what is stored.
    const ApplicationSettings *settings = this->getApplicationSettings();

    options.append(RArgumentOption("render-backend",
                                   RArgumentOption::String,
                                   QVariant(RenderBackend::toString(settings->getRenderBackend())),
                                   "Rendering backend to use (" + RenderBackend::getTypeNames().join("|") + ")",
                                   RArgumentOption::Optional,
                                   false));
    options.append(RArgumentOption("rhi-api",
                                   RArgumentOption::String,
                                   QVariant(RenderBackend::toString(settings->getRhiApi())),
                                   "Graphics API for the rhi rendering backend (" + RenderBackend::getRhiApiNames().join("|") + ")",
                                   RArgumentOption::Optional,
                                   false));

    return options;
}

void Application::processAdditionalArguments(const RArgumentsParser &argumentsParser)
{
    // Start from what the user configured in the application settings.
    RenderBackend::setType(this->getApplicationSettings()->getRenderBackend());
    RenderBackend::setRhiApi(this->getApplicationSettings()->getRhiApi());

    // A command line switch overrides the stored setting for this run only.
    if (argumentsParser.isSet("render-backend"))
    {
        bool isOk = false;
        const QString name(argumentsParser.getValue("render-backend").toString());
        RenderBackend::Type type = RenderBackend::typeFromString(name, &isOk);
        if (isOk)
        {
            RenderBackend::setType(type);
        }
        else
        {
            RLogger::warning("Unknown rendering backend \'%s\'. Falling back to \'%s\'.\n",
                             name.toUtf8().constData(),
                             RenderBackend::toString(RenderBackend::getType()).toUtf8().constData());
        }
    }

    if (argumentsParser.isSet("rhi-api"))
    {
        bool isOk = false;
        const QString name(argumentsParser.getValue("rhi-api").toString());
        RenderBackend::RhiApi api = RenderBackend::rhiApiFromString(name, &isOk);
        if (isOk)
        {
            RenderBackend::setRhiApi(api);
        }
        else
        {
            RLogger::warning("Unknown RHI graphics API \'%s\'. Falling back to \'auto\'.\n",
                             name.toUtf8().constData());
        }
    }

    QString backendInfo(RenderBackend::toString(RenderBackend::getType()));
    if (RenderBackend::isRhi())
    {
        backendInfo += " (api: " + RenderBackend::toString(RenderBackend::getRhiApi()) + ")";
    }
    RLogger::info("Rendering backend: %s\n", backendInfo.toUtf8().constData());
}

void Application::resetSettings()
{
    RApplication::resetSettings();

    // Re-apply the settings which have already been consumed by the constructor.
    RJobSettings defaultJobSettings = RJob::getDefaultJobSettings();
    defaultJobSettings.setNOmpThreads(this->getApplicationSettings()->getNThreads());
    RJob::setDefaultJobSettings(defaultJobSettings);
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
        if (!sessionFileName.isEmpty() && RFileUtils::fileExists(sessionFileName))
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
