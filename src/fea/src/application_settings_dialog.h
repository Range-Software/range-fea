#ifndef APPLICATION_SETTINGS_DIALOG_H
#define APPLICATION_SETTINGS_DIALOG_H

#include <QDialog>

#include "application_settings.h"

class ApplicationSettingsDialog : public QDialog
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ApplicationSettingsDialog(ApplicationSettings *applicationSettings, QWidget *parent = nullptr);

};

#endif // APPLICATION_SETTINGS_DIALOG_H
