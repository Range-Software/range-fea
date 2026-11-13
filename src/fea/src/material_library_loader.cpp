#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <exception>

#include <rbl_error.h>

#include "material_library_loader.h"

namespace
{
RMaterial readMaterial(const QString &path)
{
    if (QFileInfo(path).suffix().compare("json",Qt::CaseInsensitive) != 0)
    {
        RMaterial material;
        material.read(path);
        return material;
    }

    // RMaterial::readJson() uses the global logger's scope/timer stack, which
    // cannot be shared with simultaneous model loading on another thread.
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,file.errorString());
    }
    const QByteArray bytes = file.readAll();
    if (file.error() != QFileDevice::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,file.errorString());
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(bytes,&error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
    {
        throw RError(RError::Type::InvalidFileFormat,R_ERROR_REF,"Invalid material JSON in %s: %s",
                     path.toUtf8().constData(),error.errorString().toUtf8().constData());
    }
    return RMaterial::fromJson(document.object());
}

void writeMaterial(const QString &path, const RMaterial &material)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,file.errorString());
    }
    const QByteArray bytes = QJsonDocument(material.toJson()).toJson();
    if (file.write(bytes) != bytes.size() || !file.commit())
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,file.errorString());
    }
}
}

MaterialLibraryLoader::MaterialLibraryLoader(const QString &directory, QObject *parent)
    : QThread(parent)
    , directory(directory)
{
}

MaterialLibraryLoader::~MaterialLibraryLoader()
{
    // A filesystem read cannot be interrupted safely. Finish that one read,
    // then stop before touching another file or destroying the worker state.
    this->requestInterruption();
    this->wait();
}

void MaterialLibraryLoader::run()
{
    try
    {
        const QFileInfoList files = QDir(this->directory).entryInfoList(QDir::Files | QDir::NoDotAndDotDot,QDir::Name);
        int completed = 0;
        emit this->progressChanged(0,int(files.size()));
        for (const QFileInfo &file : files)
        {
            if (this->isInterruptionRequested())
            {
                return;
            }
            QString path = file.filePath();
            try
            {
                // Conversion can make a second directory entry refer to a file
                // already loaded, or remove an obsolete entry in this snapshot.
                if (!this->materials.contains(path) && QFile::exists(path))
                {
                    const RMaterial material = MaterialLibraryLoader::read(path,this->directory);
                    this->materials.insert(path,material);
                }
            }
            catch (const RError &error)
            {
                this->errors.append(tr("Failed to read material file \"%1\". %2").arg(path,error.getMessage()));
            }
            catch (const std::exception &error)
            {
                this->errors.append(tr("Failed to read material file \"%1\". %2").arg(path,QString::fromUtf8(error.what())));
            }
            emit this->progressChanged(++completed,int(files.size()));
        }
    }
    catch (const RError &error)
    {
        this->errors.append(error.getMessage());
    }
    catch (const std::exception &error)
    {
        this->errors.append(QString::fromUtf8(error.what()));
    }
    catch (...)
    {
        this->errors.append(tr("Failed to load the material library."));
    }
}

RMaterial MaterialLibraryLoader::read(QString &filePath, const QString &directory)
{
    RMaterial material;
    material = readMaterial(filePath);

    QString suggestedFilePath = QDir(directory).filePath(material.getID().toString(QUuid::WithoutBraces) + "." + RMaterial::getDefaultFileExtension());
    if (filePath != suggestedFilePath)
    {
        if (QFileInfo(filePath) != QFileInfo(suggestedFilePath) && QFile::exists(suggestedFilePath))
        {
            // The material already exists in the default format, which is the
            // newer one - the former file is a leftover and must not overwrite it.
            material = readMaterial(suggestedFilePath);
        }
        else
        {
            // Write the material in the default format first so that the material
            // is never lost in case the conversion fails.
            writeMaterial(suggestedFilePath,material);
        }

        // Remove the former file only if it is not the file which has just been
        // written - the paths may differ while pointing to the same file on
        // case insensitive file systems.
        if (QFileInfo(filePath) != QFileInfo(suggestedFilePath))
        {
            if (!QFile::remove(filePath))
            {
                throw RError(RError::Type::RemoveFile,R_ERROR_REF,"Failed to remove material file %s",filePath.toUtf8().constData());
            }
        }

        filePath = suggestedFilePath;
    }

    return material;
}
