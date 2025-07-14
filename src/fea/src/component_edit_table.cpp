#include "component_edit_table.h"

ComponentEditTable::ComponentEditTable(const RConditionComponent &component, QWidget *parent) :
    ValueTable(parent)
{
    this->setKeyHeader(component.getKeyName());
    this->setValueHeader(component.getValueName());
    this->setDataType(RVariable::getDataType(component.getType()));
    this->populate(component);
}

void ComponentEditTable::populate(const RConditionComponent &component)
{
    for (uint i=0;i<component.size();i++)
    {
        this->addValue(component.getKey(i),
                       component.getValue(i));
    }
    this->resizeColumnsToContents();
}
