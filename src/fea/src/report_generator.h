#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <QTextCursor>

#include "document_generator.h"
#include "results_statistics.h"

class ReportGenerator : public DocumentGenerator
{
    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;

    public:

        //! Constructor.
        explicit ReportGenerator(uint modelID);

    protected:

        //! Generate title.
        void generateTitle();

        //! Generate body.
        void generateBody();

        //! Generate model chapter.
        void generateModelChapter();

        //! Generate problem chapter.
        void generateProblemChapter();

        //! Generate results chapter.
        void generateResultsChapter();

        //! Generate table of support reactions, if there are any.
        void generateSupportReactions(QTextCursor &cursor, const ResultsStatistics::SupportReactions &supportReactions);

        //! Take model screenshot.
        QImage takeModelScreenshot(QString &screenShotFile) const;

};

#endif // REPORT_GENERATOR_H
