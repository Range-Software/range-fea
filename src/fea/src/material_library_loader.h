#ifndef MATERIAL_LIBRARY_LOADER_H
#define MATERIAL_LIBRARY_LOADER_H

#include <QMap>
#include <QStringList>
#include <QThread>

#include <rml_material.h>

//! Loads a snapshot without touching application settings or GUI objects.
//! Results may only be accessed after the thread has finished.
class MaterialLibraryLoader : public QThread
{
    Q_OBJECT

    public:
        explicit MaterialLibraryLoader(const QString &directory, QObject *parent = nullptr);
        ~MaterialLibraryLoader() override;

        QMap<QString,RMaterial> materials;
        QStringList errors;

        //! Preserve the legacy material conversion rules using a captured directory.
        static RMaterial read(QString &filePath, const QString &directory);

    protected:
        void run() override;

    private:
        const QString directory;

    signals:
        void progressChanged(int completed, int total);
};

#endif // MATERIAL_LIBRARY_LOADER_H
