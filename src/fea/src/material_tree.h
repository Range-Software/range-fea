#ifndef MATERIAL_TREE_H
#define MATERIAL_TREE_H

#include <QTreeWidget>
#include <QPushButton>

#include <rml_material.h>
#include <rml_material_property.h>

class MaterialTree : public QWidget
{

    Q_OBJECT

    protected:

        enum ColumnType
        {
            ColumnName = 0,
            ColumnValue,
            ColumnUnits,
            ColumnPropertyType,
            NunberOfColumns
        };

        //! Material name.
        RMaterial material;

        QTreeWidget *treeWidget;

        QPushButton *addButton;
        QPushButton *removeButton;

    public:

        //! Constructor.
        explicit MaterialTree(QWidget *parent = nullptr);

    protected:

        //! Populate model.
        void populate();

    public slots:

        //! Catch materialSelected signal.
        void onMaterialSelected(const RMaterial &material);

    protected slots:

        //! Catch itemSelectionChanged signal.
        void onItemSelectionChanged();

        //! Catch valueChanged signal from MaterialPropertyLineEditor.
        void onValueChanged(RMaterialProperty::Type type, double value);

        //! Edit values button was clicked.
        void onButtonValueClicked(int id);

        //! Add new property.
        void onPropertyAdd();

        //! Delete selected property.
        void onPropertyDelete();

    signals:

        void materialChanged(const RMaterial &material);

};

#endif // MATERIAL_TREE_H
