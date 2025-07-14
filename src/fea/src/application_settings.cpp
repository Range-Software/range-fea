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
