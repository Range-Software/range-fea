#ifndef VARIABLE_VALUE_EDIT_H
#define VARIABLE_VALUE_EDIT_H

#include <QCheckBox>

#include <rml_variable.h>

#include "value_line_edit.h"

class VariableValueEdit : public QWidget
{
    Q_OBJECT

    protected:

        //! Variable type.
        RVariableType type;
        //! Value line edit.
        ValueLineEdit *lineEdit;
        //! Boolean check box.
        QCheckBox *checkBox;

    public:

        //! Constructor.
        explicit VariableValueEdit(RVariableType variableType,
                                   QWidget *parent = nullptr);

        //! Set new value.
        void setValue(double value);

    signals:

        //! Value changed signal.
        void valueChanged(RVariableType variableType, double value);

    private slots:

        //! Catch signal value changed.
        void onValueChaged(double value);

        //! Catch signal state changed.
        void onStateChaged(Qt::CheckState state);
};

#endif /* VARIABLE_VALUE_EDIT_H */
