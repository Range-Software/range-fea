#include "application.h"
#include "usage_info.h"

UsageInfo &UsageInfo::getInstance()
{
    static UsageInfo usageInfo;
    return usageInfo;
}

QString UsageInfo::getReport() const
{
    QString report;

    report += "{ ";
    report +=   "\"session:\" \"" + Application::instance()->getSession()->getID() + "\", ";
    report +=   "\"version:\" \"" + Application::instance()->getApplicationSettings()->getStoredVersion().toString() + "\", ";
    report +=   "\"system:\" { ";
    report +=     "\"buildAbi:\" \"" + QSysInfo::buildAbi() + "\", ";
    report +=     "\"buildCpuArchitecture:\" \"" + QSysInfo::buildCpuArchitecture() + "\", ";
    report +=     "\"currentCpuArchitecture:\" \"" + QSysInfo::currentCpuArchitecture() + "\", ";
    report +=     "\"kernelType:\" \"" + QSysInfo::kernelType() + "\", ";
    report +=     "\"kernelVersion:\" \"" + QSysInfo::kernelVersion() + "\", ";
    report +=     "\"machineHostName:\" \"" + QSysInfo::machineHostName() + "\", ";
    report +=     "\"prettyProductName:\" \"" + QSysInfo::prettyProductName() + "\", ";
    report +=     "\"productType:\" \"" + QSysInfo::productType() + "\", ";
    report +=     "\"productVersion:\" \"" + QSysInfo::productVersion() + "\" ";
    report +=   "}";
    report += "}";

    return report;
}
