#ifndef RESULTS_VARIABLE_SELECTOR_H
#define RESULTS_VARIABLE_SELECTOR_H

#include <rml_variable.h>

#include "variable_selector.h"

class ResultsVariableSelector : public VariableSelector
{

    Q_OBJECT

    protected:

    public:

        //! Constructor.
        explicit ResultsVariableSelector(QWidget *parent = nullptr);

    protected:

        //! Populate combo.
        void populate(RVariableType selectedVariableType = R_VARIABLE_NONE);

    protected slots:

        //! Catch Session::resultsChanged signal
        void onResultsChanged(uint);

};

#endif // RESULTS_VARIABLE_SELECTOR_H
