#include "material_property_line_edit.h"

MaterialPropertyLineEdit::MaterialPropertyLineEdit(RMaterialProperty::Type type, QWidget *parent) :
    ValueLineEdit(parent),
    type(type)
{
    this->setDoubleValidator();
    QObject::connect(this,&ValueLineEdit::valueChanged,this,&MaterialPropertyLineEdit::onValueChaged);
}

void MaterialPropertyLineEdit::onValueChaged(double value)
{
    emit this->valueChanged(this->type,value);
}
