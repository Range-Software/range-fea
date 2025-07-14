#ifndef COMPONENT_EDIT_DIALOG_H
#define COMPONENT_EDIT_DIALOG_H

#include <QDialog>

#include <rml_condition_component.h>

#include "component_edit_table.h"

class ComponentEditDialog : public QDialog
{
    Q_OBJECT

    private:

        //! Pointer to condition component.
        RConditionComponent *pComponent;

        //! Component table.
        ComponentEditTable *editTable;

    public:

        //! Constructor.
        explicit ComponentEditDialog(RConditionComponent &component, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    protected slots:

        //! Catch value changed signal and set new number of values.
        void onSpinNValuesChanged(int nValues);

};

#endif /* COMPONENT_VALUE_EDITOR_DIALOG_H */
