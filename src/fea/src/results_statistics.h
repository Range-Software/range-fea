#ifndef RESULTS_STATISTICS_H
#define RESULTS_STATISTICS_H

#include <vector>

#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include <rbl_r3vector.h>
#include <rml_entity_group.h>
#include <rml_problem_type.h>
#include <rml_variable.h>

class Model;

//! Statistics of the results of a model, shared by the model statistics dialog
//! and the report. Labels are translated and values formatted, but nothing is
//! escaped - the caller decides how the text is presented.
class ResultsStatistics
{

    Q_DECLARE_TR_FUNCTIONS(ResultsStatistics)

    public:

        //! One line of statistics: label, value and where the value occurs.
        struct Row
        {
            QString label;
            QString value;
            QString location;
        };

        //! Statistics of one result variable.
        struct Variable
        {
            //! Name with units.
            QString title;
            std::vector<Row> rows;
        };

        //! One line of the support reaction table.
        struct ForceRow
        {
            QString label;
            //! A total line rather than a single support.
            bool total;
            RR3Vector force;
        };

        //! Support reaction table with the notes explaining it.
        struct SupportReactions
        {
            //! Column headers: support, Fx, Fy, Fz, |F|.
            QStringList headers;
            std::vector<ForceRow> rows;
            QStringList notes;
        };

        //! Results of one solved problem type, or of no problem type.
        struct Section
        {
            QString title;
            std::vector<Variable> variables;
            //! Empty when the section has no support reactions.
            SupportReactions supportReactions;
        };

    protected:

        //! Model has results.
        bool hasResultsFlag;
        //! Time step, time, mode or frequency of the results.
        std::vector<Row> solutionState;
        //! Sections in the order the problem types are solved.
        std::vector<Section> sections;

    public:

        //! Constructor.
        explicit ResultsStatistics(const Model &rModel);

        //! Return true if the model has any results.
        bool hasResults() const;

        //! Return time step and time, or mode and frequency, of the results.
        const std::vector<Row> &getSolutionState() const;

        //! Return sections, one for each solved problem type with results,
        //! followed by other results.
        const std::vector<Section> &getSections() const;

        //! Return real value in scientific format, e.g. 4,000000e-02.
        static QString realToString(double value);

        //! Return translated name of entity type.
        static QString entityTypeName(REntityGroupType entityType);

    protected:

        //! Return true if given element result belongs to the model boundary,
        //! like heat radiation, rather than to the body.
        static bool isBoundaryResult(RVariableType variableType);

        //! Return which elements carry the element results of given problem
        //! types. Results of the body are taken only on elements the solvers
        //! compute, results of the boundary on all but point elements.
        static std::vector<bool> findResultElements(const Model &rModel, RProblemTypeMask problemTypeMask, bool boundaryResult);

        //! Find statistics of one result variable. Element values are taken
        //! only on elements marked in resultElements. Return false if the
        //! variable has no valid value.
        static bool findVariableStatistics(const Model &rModel,
                                           const RVariable &rVariable,
                                           const std::vector<QString> &elementEntityNames,
                                           const std::vector<bool> &resultElements,
                                           Variable &variable);

        //! Find support reaction table.
        static SupportReactions findSupportReactions(const Model &rModel);

};

#endif // RESULTS_STATISTICS_H
