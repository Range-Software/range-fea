#ifndef APPLICATION_SETTINGS_H
#define APPLICATION_SETTINGS_H

#include <QSettings>
#include <QString>

#include <rbl_version.h>

#include <rgl_application_settings.h>

class ApplicationSettings : public RApplicationSettings
{

    Q_OBJECT

    protected:

        static const QString sessionFileNameKey;
        static const QString nThreadsKey;
        static const QString nHistoryRecordsKey;
        static const QString solverPathKey;

    public:

        //! Constructor.
        explicit ApplicationSettings(QObject *parent = nullptr);

        //! Return users materials directory path.
        QString getMaterialsDir() const;

        //! Return users session directory path.
        QString getSessionDir() const;

        //! Return session file name.
        QString getSessionFileName() const;

        //! Return const reference to Range solver path.
        QString getSolverPath() const;

        //! Set new Range solver path.
        void setSolverPath(const QString &solverPath);

        //! Return number of threads to be used in calculation.
        uint getNThreads() const;

        //! Set number of threads to be used in calculation.
        void setNThreads(uint nThreads);

        //! Return number of history records.
        uint getNHistoryRecords() const;

        //! Set number of history records.
        void setNHistoryRecords(uint nHistoryRecords);

        //! Return maximum number of Threads.
        static uint getMaxThreads();

        //! Return default range solver executable.
        static QString getDefaultSolverPath();

        //! Return default number of threads.
        static uint getDefaultNThreads();

        //! Return default number of history records.
        static uint getDefaultNHistoryRecords();

    signals:

        //! Number of history records has changed.
        void nHistoryRecordsChanged(uint nHistoryRecords);

};

#endif /* APPLICATION_SETTINGS_H */
