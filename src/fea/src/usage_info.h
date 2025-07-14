#ifndef USAGE_INFO_H
#define USAGE_INFO_H

#include <QString>

class UsageInfo
{

    private:

        //! Constructor.
        UsageInfo() = default;

    public:

        //! Return reference to static instance.
        static UsageInfo &getInstance();

        //! Get usage report.
        QString getReport() const;
};

#endif // USAGE_INFO_H
