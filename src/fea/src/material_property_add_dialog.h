#ifndef MATERIAL_PROPERTY_ADD_DIALOG_H
#define MATERIAL_PROPERTY_ADD_DIALOG_H

#include <QDialog>
#include <QListWidget>

#include <rml_material.h>

class MaterialPropertyAddDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Pointer to material object.
        RMaterial *pMaterial;
        //! List of available material properties.
        QListWidget *propertyList;

    public:

        //! Constructor.
        explicit MaterialPropertyAddDialog(RMaterial &material,
                                           QWidget   *parent = nullptr);

        //! Overloaded exec function.
        int exec();

    protected:

        //! Find available properties.
        QList<RMaterialProperty::Type> findAvailableProperties(const RMaterial &material) const;

};

#endif /* MATERIAL_PROPERTY_ADD_DIALOG_H */
