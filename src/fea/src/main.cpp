#include <rbl_arguments_parser.h>
#include <rbl_logger.h>

#include "application.h"

int main(int argc, char *argv[])
{
    RArgumentsParser::printHeader("GUI");

    QLocale::setDefault(QLocale::c());

    // Uncomment in case there are some issues with fractional scaling
//    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);

    int exitValue = 0;
    if ((exitValue = Application(argc, argv).exec()) != 0)
    {
        RLogger::info("Application has terminated with error code (%d).\n", exitValue);
    }

    RArgumentsParser::printFooter();
    return exitValue;
} /* main */
