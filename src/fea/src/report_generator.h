#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <QObject>
#include <QMap>
#include <QTextDocument>
#include <QPrinter>
#include <QTextCharFormat>

#include "document_generator.h"

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

        //! Take model screenshot.
        QImage takeModelScreenshot() const;

};

#endif // REPORT_GENERATOR_H
