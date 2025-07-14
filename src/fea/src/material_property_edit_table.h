#ifndef MATERIAL_PROPERTY_EDIT_TABLE_H
#define MATERIAL_PROPERTY_EDIT_TABLE_H

#include <QTableWidget>

#include <rml_material_property.h>

#include "value_table.h"

class MaterialPropertyEditTable : public ValueTable
{
    Q_OBJECT

    public:

        //! Constructor.
        explicit MaterialPropertyEditTable(RMaterialProperty &property,
                                           QWidget *parent = nullptr);

    protected:

        //! Populate tree.
        void populate(RMaterialProperty &property);

};

#endif /* MATERIAL_PROPERTY_EDIT_TABLE_H */
