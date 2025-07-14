#ifndef COMPONENT_EDIT_TABLE_H
#define COMPONENT_EDIT_TABLE_H

#include <rml_condition_component.h>

#include "value_table.h"

class ComponentEditTable : public ValueTable
{
    Q_OBJECT

    protected:

    public:

        //! Constructor.
        explicit ComponentEditTable(const RConditionComponent &component, QWidget *parent = nullptr);

    protected:

        //! Populate tree.
        void populate(const RConditionComponent &component);

};

#endif /* COMPONENT_EDIT_TABLE_H */
