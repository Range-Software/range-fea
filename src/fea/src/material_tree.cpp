#include <QVBoxLayout>
#include <QHBoxLayout>

#include "material_tree.h"
#include "material_property_add_dialog.h"
#include "material_property_line_edit.h"
#include "material_property_edit_dialog.h"
#include "push_button.h"

MaterialTree::MaterialTree(QWidget *parent) :
    QWidget{parent}
{
    QIcon addIcon(":/icons/file/pixmaps/range-new.svg");
    QIcon removeIcon(":/icons/action/pixmaps/range-close.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    this->treeWidget = new QTreeWidget;
    mainLayout->addWidget(this->treeWidget);

    this->treeWidget->setColumnCount(ColumnType::NunberOfColumns);
    this->treeWidget->setRootIsDecorated(true);
    this->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    this->treeWidget->setSortingEnabled(true);
    this->treeWidget->sortByColumn(ColumnType::ColumnName,Qt::AscendingOrder);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem;
    headerItem->setText(ColumnType::ColumnPropertyType,tr("Property type"));
    headerItem->setText(ColumnType::ColumnName,tr("Property name"));
    headerItem->setText(ColumnType::ColumnValue,tr("Value"));
    headerItem->setText(ColumnType::ColumnUnits,tr("Units"));
    this->treeWidget->setHeaderItem(headerItem);

    this->treeWidget->setColumnHidden(ColumnType::ColumnPropertyType,true);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);

    this->addButton = new QPushButton(addIcon,tr("Add"));
    this->addButton->setToolTip(tr("Add new material property"));
    buttonLayout->addWidget(this->addButton);

    this->removeButton = new QPushButton(removeIcon,tr("Remove"));
    this->removeButton->setToolTip(tr("Remove selected material property"));
    this->removeButton->setDisabled(true);
    buttonLayout->addWidget(this->removeButton);

    QObject::connect(this->addButton,&QPushButton::clicked,this,&MaterialTree::onPropertyAdd);
    QObject::connect(this->removeButton,&QPushButton::clicked,this,&MaterialTree::onPropertyDelete);

    QObject::connect(this->treeWidget,&QTreeWidget::itemSelectionChanged,this,&MaterialTree::onItemSelectionChanged);
}

void MaterialTree::populate()
{
    bool signalsBlockedOld = this->treeWidget->signalsBlocked();
    this->treeWidget->blockSignals(true);

    this->treeWidget->clear();

    for (uint i=0;i<this->material.size();i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(this->treeWidget);
        item->setData(ColumnType::ColumnPropertyType,Qt::DisplayRole,QVariant(this->material.get(i).getType()));
        item->setText(ColumnType::ColumnName,this->material.get(i).getName());
        item->setText(ColumnType::ColumnUnits,this->material.get(i).getUnits());

        MaterialPropertyLineEdit *lineEdit = new MaterialPropertyLineEdit(this->material.get(i).getType());
        lineEdit->setValue(this->material.get(i).get(RVariable::getInitValue(this->material.get(i).getKeyType())));
        this->treeWidget->setItemWidget(item,ColumnType::ColumnValue,lineEdit);
        if (this->material.get(i).size() > 1)
        {
            lineEdit->setDisabled(true);
        }

        QTreeWidgetItem *child = new QTreeWidgetItem(item);
        child->setFirstColumnSpanned(true);
        child->setData(ColumnType::ColumnPropertyType,Qt::DisplayRole,QVariant(this->material.get(i).getType()));
        PushButton *buttonValues = new PushButton(int(this->material.get(i).getType()),QString("Edit temperature dependent values"));
        this->treeWidget->setItemWidget(child,ColumnType::ColumnName,buttonValues);

        QObject::connect(lineEdit,&MaterialPropertyLineEdit::valueChanged,this,&MaterialTree::onValueChanged);
        QObject::connect(buttonValues,&PushButton::clicked,this,&MaterialTree::onButtonValueClicked);
    }

    this->treeWidget->blockSignals(signalsBlockedOld);
}

void MaterialTree::onMaterialSelected(const RMaterial &material)
{
    this->material = material;
    this->populate();
}

void MaterialTree::onItemSelectionChanged()
{
    this->removeButton->setEnabled(this->treeWidget->selectedItems().count());
}

void MaterialTree::onValueChanged(RMaterialProperty::Type type, double value)
{
    uint propertyPosition = this->material.findPosition(type);
    this->material.get(propertyPosition).add(RVariable::getInitValue(this->material.get(propertyPosition).getKeyType()),value);
    emit this->materialChanged(this->material);
}

void MaterialTree::onButtonValueClicked(int id)
{
    uint propertyPosition = this->material.findPosition(RMaterialProperty::Type(id));

    MaterialPropertyEditDialog materialPropertyEditDialog(this->material.get(propertyPosition),this);

    if (materialPropertyEditDialog.exec() == QDialog::Accepted)
    {
//        for (int i=0;i<this->topLevelItemCount();i++)
//        {
//            RMaterialProperty::Type type = RMaterialProperty::Type(this->topLevelItem(i)->data(MATERIAL_TREE_PROPERTY_TYPE,Qt::DisplayRole).toInt());
//            if (type != RMaterialProperty::Type(id))
//            {
//                continue;
//            }
//            MaterialPropertyLineEdit *lineEdit = (MaterialPropertyLineEdit*)this->itemWidget(this->topLevelItem(i),MATERIAL_TREE_PROPERTY_VALUE);
//            lineEdit->setValue(this->material.get(propertyPosition).get(RVariable::getInitValue(this->material.get(propertyPosition).getKeyType())));
//            lineEdit->setEnabled(this->material.get(propertyPosition).size() == 1);
//        }
        this->populate();
        emit this->materialChanged(this->material);
    }
}

void MaterialTree::onPropertyAdd()
{
    MaterialPropertyAddDialog addPropertyDialog(this->material,this);
    if (addPropertyDialog.exec() == QDialog::Accepted)
    {
        emit this->materialChanged(this->material);
        this->populate();
    }
}

void MaterialTree::onPropertyDelete()
{
    QList<QTreeWidgetItem*> items = this->treeWidget->selectedItems();

    for (int i=0;i<items.size();i++)
    {
        RMaterialProperty::Type type = RMaterialProperty::Type(items[i]->data(ColumnType::ColumnPropertyType,Qt::DisplayRole).toInt());

        uint propertyPosition = this->material.findPosition(type);
        this->material.remove(propertyPosition);
    }
    this->populate();
    emit this->materialChanged(this->material);
}
