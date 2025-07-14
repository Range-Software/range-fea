#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "material_property_edit_dialog.h"

MaterialPropertyEditDialog::MaterialPropertyEditDialog(RMaterialProperty &property, QWidget *parent) :
    QDialog(parent),
    pProperty(&property)
{
    this->setWindowTitle(QString("Material property editor"));
    this->resize(400,500);

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->spinNValues = new QSpinBox;
    this->spinNValues->setMinimum(1);
    this->spinNValues->setValue(property.size());
    formLayout->addRow(tr("Number of values") + ":",this->spinNValues);

    this->editTable = new MaterialPropertyEditTable(property, parent);
    mainLayout->addWidget(this->editTable);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->spinNValues,&QSpinBox::valueChanged,this,&MaterialPropertyEditDialog::onSpinNValuesChanged);
    QObject::connect(this->editTable,&ValueTable::sizeChanged,this,&MaterialPropertyEditDialog::onTableSizeChanged);
}

int MaterialPropertyEditDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted && this->pProperty)
    {
        this->pProperty->clear();
        for (int i=0;i<this->editTable->rowCount();i++)
        {
            this->pProperty->add(this->editTable->getKey(i),
                                 this->editTable->getValue(i));
        }
    }
    return retVal;
}

void MaterialPropertyEditDialog::onTableSizeChanged(int nRows)
{
    this->spinNValues->setValue(nRows);
}

void MaterialPropertyEditDialog::onSpinNValuesChanged(int nValues)
{
     int oldRowCount = this->editTable->rowCount();

     if (oldRowCount > nValues)
     {
         this->editTable->setRowCount(nValues);
         return;
     }

     double keyIncrement = 1.0;

     double key = this->editTable->getKey(oldRowCount-1);
     double value = this->editTable->getValue(oldRowCount-1);

     for (int i=oldRowCount;i<nValues;i++)
     {
         key += keyIncrement;
         this->editTable->addValue(key,value);
     }
}
