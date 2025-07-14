#ifndef MATERIAL_PROPERTY_LINE_EDIT_H
#define MATERIAL_PROPERTY_LINE_EDIT_H

#include <rml_material_property.h>

#include "value_line_edit.h"

class MaterialPropertyLineEdit : public ValueLineEdit
{
    Q_OBJECT

    protected:

        //! Material property type.
        RMaterialProperty::Type type;

    public:

        //! Constructor.
        explicit MaterialPropertyLineEdit(RMaterialProperty::Type type,
                                          QWidget *parent = nullptr);

    signals:

        //! Value changed signal.
        void valueChanged(RMaterialProperty::Type type, double value);

    private slots:

        //! Catch signal textChanged
        void onValueChaged(double value);

};

#endif /* MATERIAL_PROPERTY_LINE_EDIT_H */
