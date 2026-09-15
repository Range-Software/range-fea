#include <omp.h>

#include <QApplication>
#include <QDir>
#include <QDesktopServices>
#include <QImageWriter>
#include <QStandardPaths>

#include <rbl_logger.h>
#include <rbl_utils.h>
#include <rbl_job.h>
#include <rbl_job_settings.h>

#include "application_settings.h"

const QString ApplicationSettings::sessionFileNameKey = "session/fileName";
const QString ApplicationSettings::nThreadsKey = "application/nThreads";
const QString ApplicationSettings::nHistoryRecordsKey = "application/nHistoryRecords";
const QString ApplicationSettings::solverPathKey = "application/solverPath";
const QString ApplicationSettings::renderBackendKey = "application/renderBackend";
const QString ApplicationSettings::rhiApiKey = "application/rhiApi";

ApplicationSettings::ApplicationSettings(QObject *parent)
    : RApplicationSettings{parent}
{
}

QString ApplicationSettings::getMaterialsDir() const
{
    return RApplicationSettings::getDir(this->getAppHomeDir(),"materials");
}

QString ApplicationSettings::getSessionDir() const
{
    return RApplicationSettings::getDir(this->getAppHomeDir(),"session");
}

QString ApplicationSettings::getSessionFileName() const
{
    return this->value(ApplicationSettings::sessionFileNameKey).toString();
}

QString ApplicationSettings::getSolverPath() const
{
    return this->value(ApplicationSettings::solverPathKey,ApplicationSettings::getDefaultSolverPath()).toString();
}

void ApplicationSettings::setSolverPath(const QString &rangeSolverPath)
{
    this->setValue(ApplicationSettings::solverPathKey, rangeSolverPath);
}

uint ApplicationSettings::getNThreads() const
{
    return this->value(ApplicationSettings::nThreadsKey,ApplicationSettings::getDefaultNThreads()).toUInt();
}

void ApplicationSettings::setNThreads(uint nCPUs)
{
    uint defaultNCPUs = ApplicationSettings::getDefaultNThreads();
    uint nThreads = (nCPUs < 1 || nCPUs > defaultNCPUs) ? defaultNCPUs : nCPUs;

    this->setValue(ApplicationSettings::nThreadsKey, nThreads);

    RJobSettings defaultJobSettings = RJob::getDefaultJobSettings();
    defaultJobSettings.setNOmpThreads(nThreads);
    RJob::setDefaultJobSettings(defaultJobSettings);
}

uint ApplicationSettings::getNHistoryRecords() const
{
    return this->value(ApplicationSettings::nHistoryRecordsKey,ApplicationSettings::getDefaultNHistoryRecords()).toUInt();
}

void ApplicationSettings::setNHistoryRecords(uint nHistoryRecords)
{
    bool changed = false;
    nHistoryRecords = nHistoryRecords > 0 ? nHistoryRecords : 1;
    if (this->getNHistoryRecords() != nHistoryRecords)
    {
        changed = true;
    }

    this->setValue(ApplicationSettings::nHistoryRecordsKey, nHistoryRecords);

    if (changed)
    {
        emit this->nHistoryRecordsChanged(nHistoryRecords);
    }
}

RenderBackend::Type ApplicationSettings::getRenderBackend() const
{
    // Stored by name so the settings file stays readable and survives reordering
    // of the enum.  Anything unrecognised falls back to the default.
    const QString name(this->value(ApplicationSettings::renderBackendKey,
                                   RenderBackend::toString(ApplicationSettings::getDefaultRenderBackend())).toString());

    bool isOk = false;
    RenderBackend::Type renderBackend = RenderBackend::typeFromString(name, &isOk);

    return isOk ? renderBackend : ApplicationSettings::getDefaultRenderBackend();
}

void ApplicationSettings::setRenderBackend(RenderBackend::Type renderBackend)
{
    if (this->getRenderBackend() == renderBackend)
    {
        return;
    }

    this->setValue(ApplicationSettings::renderBackendKey, RenderBackend::toString(renderBackend));

    emit this->renderBackendChanged(renderBackend);
}

RenderBackend::RhiApi ApplicationSettings::getRhiApi() const
{
    const QString name(this->value(ApplicationSettings::rhiApiKey,
                                   RenderBackend::toString(ApplicationSettings::getDefaultRhiApi())).toString());

    bool isOk = false;
    RenderBackend::RhiApi rhiApi = RenderBackend::rhiApiFromString(name, &isOk);

    return isOk ? rhiApi : ApplicationSettings::getDefaultRhiApi();
}

void ApplicationSettings::setRhiApi(RenderBackend::RhiApi rhiApi)
{
    if (this->getRhiApi() == rhiApi)
    {
        return;
    }

    this->setValue(ApplicationSettings::rhiApiKey, RenderBackend::toString(rhiApi));

    emit this->rhiApiChanged(rhiApi);
}

uint ApplicationSettings::getMaxThreads()
{
    return uint(omp_get_num_procs());
}

QString ApplicationSettings::getDefaultSolverPath()
{
    QString baseName("fea-solver");
#ifdef DEBUG
    baseName += ("_debug");
#endif
    return QDir(QApplication::applicationDirPath()).filePath(baseName);
}

uint ApplicationSettings::getDefaultNThreads()
{
    uint maxThreads = ApplicationSettings::getMaxThreads();
    if (maxThreads > 1)
    {
        maxThreads--;
    }
    return maxThreads;
}

uint ApplicationSettings::getDefaultNHistoryRecords()
{
    return 3;
}

RenderBackend::Type ApplicationSettings::getDefaultRenderBackend()
{
    return RenderBackend::Type::OpenGL;
}

RenderBackend::RhiApi ApplicationSettings::getDefaultRhiApi()
{
    return RenderBackend::RhiApi::Auto;
}
