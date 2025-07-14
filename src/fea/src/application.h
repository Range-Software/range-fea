#ifndef APPLICATION_H
#define APPLICATION_H

#include <rgl_application.h>
#include <rgl_application_settings.h>

#include "application_settings.h"
#include "action_list.h"
#include "main_window.h"
#include "session.h"

class Application : public RApplication
{

    Q_OBJECT

    protected:

        //! Session.
        Session *session;

    public:

        //! Constructor.
        explicit Application(int &argc, char **argv);

        //! Return application instance.
        static Application *instance() noexcept;

        //! Return const pointer to application settings.
        const ApplicationSettings *getApplicationSettings() const;

        //! Return pointer to application settings.
        ApplicationSettings *getApplicationSettings();

        //! Return const pointer to action list.
        const ActionList *getActionList() const;

        //! Return pointer to action list.
        ActionList *getActionList();

        //! Return const reference to session.
        const Session *getSession() const;

        //! Return reference to session.
        Session *getSession();

        //! Return const pointer to main window.
        const MainWindow *getMainWindow() const;

        //! Return pointer to main window.
        MainWindow *getMainWindow();

        //! Initialize main window and other user objects.
        void initialize();

        //! Initialize main window and other user objects.
        void finalize();

};

#endif // APPLICATION_H
