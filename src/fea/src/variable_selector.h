#ifndef VARIABLE_SELECTOR_H
#define VARIABLE_SELECTOR_H

#include <QComboBox>

#include <rml_variable.h>

class VariableSelector : public QComboBox
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit VariableSelector(const std::vector<RVariableType> &variableTypes,QWidget *parent = nullptr);

        //! Return current variable type.
        RVariableType getCurrentVariableType() const;

        //! Set current variable type and return true on success.
        bool setCurrentVariableType(RVariableType variableType);

    protected:

        //! Populate combo.
        void populate(const std::vector<RVariableType> &variableTypes);

    signals:

        //! Signal that results variable selection has changed.
        void resultsVariableSelected(RVariableType variableType);

    public slots:

        //! Catch QComboBox::currentIndexChanged(int) signal
        void onCurrentIndexChanged(int index);

};

#endif // VARIABLE_SELECTOR_H
