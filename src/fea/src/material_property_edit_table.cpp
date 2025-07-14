#include "material_property_edit_table.h"

MaterialPropertyEditTable::MaterialPropertyEditTable(RMaterialProperty &property, QWidget *parent) :
    ValueTable(parent)
{
    this->setKeyHeader(property.getKeyName());
    this->setValueHeader(property.getValueName());
    this->populate(property);
}

void MaterialPropertyEditTable::populate(RMaterialProperty &property)
{
    for (uint i=0;i<property.size();i++)
    {
        this->addValue(property.getKey(i),
                       property.getValue(i));
    }
    this->resizeColumnsToContents();
}
