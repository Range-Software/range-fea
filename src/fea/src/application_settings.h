#ifndef APPLICATION_SETTINGS_H
#define APPLICATION_SETTINGS_H

#include <QSettings>
#include <QString>

#include <rbl_version.h>

#include <rgl_application_settings.h>

#include "render_backend.h"

class ApplicationSettings : public RApplicationSettings
{

    Q_OBJECT

    protected:

        static const QString sessionFileNameKey;
        static const QString nThreadsKey;
        static const QString nHistoryRecordsKey;
        static const QString solverPathKey;
        static const QString renderBackendKey;
        static const QString rhiApiKey;

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

        //! Return the rendering backend to be used for the 3D view.
        RenderBackend::Type getRenderBackend() const;

        //! Set the rendering backend to be used for the 3D view.
        //! Takes effect the next time the application starts.
        void setRenderBackend(RenderBackend::Type renderBackend);

        //! Return the graphics API the QRhi rendering backend should use.
        RenderBackend::RhiApi getRhiApi() const;

        //! Set the graphics API the QRhi rendering backend should use.
        //! Takes effect the next time the application starts.
        void setRhiApi(RenderBackend::RhiApi rhiApi);

        //! Return maximum number of Threads.
        static uint getMaxThreads();

        //! Return default range solver executable.
        static QString getDefaultSolverPath();

        //! Return default number of threads.
        static uint getDefaultNThreads();

        //! Return default number of history records.
        static uint getDefaultNHistoryRecords();

        //! Return default rendering backend.
        static RenderBackend::Type getDefaultRenderBackend();

        //! Return default graphics API for the QRhi rendering backend.
        static RenderBackend::RhiApi getDefaultRhiApi();

    signals:

        //! Number of history records has changed.
        void nHistoryRecordsChanged(uint nHistoryRecords);

        //! Rendering backend has changed.
        void renderBackendChanged(RenderBackend::Type renderBackend);

        //! Graphics API of the QRhi rendering backend has changed.
        void rhiApiChanged(RenderBackend::RhiApi rhiApi);

};

#endif /* APPLICATION_SETTINGS_H */
