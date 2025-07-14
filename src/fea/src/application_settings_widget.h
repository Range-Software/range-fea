#ifndef APPLICATION_SETTINGS_WIDGET_H
#define APPLICATION_SETTINGS_WIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>

#include <rgl_application_settings_widget.h>
#include <rgl_file_chooser_button.h>

#include "application_settings.h"

class ApplicationSettingsWidget : public RApplicationSettingsWidget
{

    Q_OBJECT

    protected:

        //! Application settings.
        ApplicationSettings *applicationSettings;

        //! Solver path file chooser.
        RFileChooserButton *solverFileChooserButton;
        //! Number of threads spin button.
        QSpinBox *nThreadsSpin;
        //! Number of history records spin button.
        QSpinBox *nHistoryRecordsSpin;

    public:

        //! Constructor.
        explicit ApplicationSettingsWidget(ApplicationSettings *applicationSettings, QWidget *parent = nullptr);

    private:

        //! Create tab containing general settings.
        QWidget *createGeneralTab();

    public slots:

        //! Set default values.
        void setDefaultValues();

    protected slots:

        //! Range solver path changed.
        void onSolverPathChanged(const QString &solverPath);

        //! Number of threads changed.
        void onNThreadsChanged(int nThreads);

        //! Number of history records changed.
        void onNHistoryRecordsChanged(int nHistoryRecords);

};

#endif // APPLICATION_SETTINGS_WIDGET_H
