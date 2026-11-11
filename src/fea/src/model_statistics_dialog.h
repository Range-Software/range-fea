#ifndef MODEL_STATISTICS_DIALOG_H
#define MODEL_STATISTICS_DIALOG_H

#include <QDialog>
#include <QTextBrowser>

#include "results_statistics.h"

class Model;

class ModelStatisticsDialog : public QDialog
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ModelStatisticsDialog(uint modelID, QWidget *parent = nullptr);

        //! Overloaded exec function.
        int exec();

    private:

        //! Produce statistics.
        void produceStatistics(uint modelID, QTextBrowser *textBrowser);

        //! Produce results chapter.
        static QString produceResults(const Model &rModel);

        //! Produce table of support reactions.
        static QString produceSupportReactions(const ResultsStatistics::SupportReactions &supportReactions);

        //! Produce standard table row.
        static QString generateTableRow(const QString &label, const QString &value);

        //! Produce table row with a value and where it occurs.
        static QString generateTableRow(const QString &label, const QString &value, const QString &location);

};

#endif // MODEL_STATISTICS_DIALOG_H
