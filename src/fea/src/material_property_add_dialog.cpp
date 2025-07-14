#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "material_property_add_dialog.h"

MaterialPropertyAddDialog::MaterialPropertyAddDialog(RMaterial &material, QWidget *parent) :
    QDialog(parent),
    pMaterial(&material)
{
    this->setWindowTitle(QString("Material property editor"));
    this->resize(300,300);

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *label = new QLabel(tr("Select one or more material properties from the list below."));
    mainLayout->addWidget(label);

    this->propertyList = new QListWidget;
    this->propertyList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mainLayout->addWidget(this->propertyList);

    QList<RMaterialProperty::Type> properties = this->findAvailableProperties(material);
    for (int i=0;i<properties.size();i++)
    {
        QListWidgetItem *item = new QListWidgetItem(this->propertyList);
        item->setText(RMaterialProperty::getName(properties[i]));
        item->setData(Qt::UserRole,QVariant(int(properties[i])));
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int MaterialPropertyAddDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted && this->pMaterial)
    {
        QList<QListWidgetItem*> items = this->propertyList->selectedItems();
        for (int i=0;i<items.size();i++)
        {
            RMaterialProperty::Type type = RMaterialProperty::Type(items[i]->data(Qt::UserRole).toInt());
            this->pMaterial->add(RMaterialProperty(type));
        }
    }
    return retVal;
}

QList<RMaterialProperty::Type> MaterialPropertyAddDialog::findAvailableProperties(const RMaterial &material) const
{
    QList<RMaterialProperty::Type> propertyTypes;

    for (int i=int(RMaterialProperty::None);
         i<int(RMaterialProperty::nTypes);
         i++)
    {
        if (RMaterialProperty::Type(i) == RMaterialProperty::None ||
            RMaterialProperty::Type(i) == RMaterialProperty::Custom)
        {
            continue;
        }
        if (material.findPosition(RMaterialProperty::Type(i)) >= material.size())
        {
            propertyTypes.push_back(RMaterialProperty::Type(i));
        }
    }

    return propertyTypes;
}
