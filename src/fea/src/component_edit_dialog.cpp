#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "component_edit_dialog.h"
#include "application.h"

ComponentEditDialog::ComponentEditDialog(RConditionComponent &component, QWidget *parent) :
    QDialog(parent),
    pComponent(&component)
{
    this->setWindowTitle(QString("Component editor"));
    this->resize(400,500);

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    QSpinBox *spinNValues = new QSpinBox;
    spinNValues->setRange(R_TIME_STEP_MIN_NUMBER,R_TIME_STEP_MAX_NUMBER);
    spinNValues->setValue(component.size());
    formLayout->addRow(tr("Number of values") + ":",spinNValues);

    this->editTable = new ComponentEditTable(component, parent);
    mainLayout->addWidget(this->editTable);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(spinNValues,&QSpinBox::valueChanged,this,&ComponentEditDialog::onSpinNValuesChanged);
}

int ComponentEditDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted && this->pComponent)
    {
        this->pComponent->clear();
        for (int i=0;i<this->editTable->rowCount();i++)
        {
            this->pComponent->add(this->editTable->getKey(i),
                                  this->editTable->getValue(i));
        }
    }
    return retVal;
}

void ComponentEditDialog::onSpinNValuesChanged(int nValues)
{
    int oldRowCount = this->editTable->rowCount();

    if (oldRowCount > nValues)
    {
        this->editTable->setRowCount(nValues);
        return;
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    double timeStepSize = R_TIME_STEP_DEFAULT_SIZE;
    if (modelIDs.size() > 0)
    {
        timeStepSize = Application::instance()->getSession()->getModel(modelIDs[0]).getTimeSolver().getInputTimeStepSize();
    }

    double currTime = this->editTable->getKey(oldRowCount-1);
    double value = this->editTable->getValue(oldRowCount-1);

    for (int i=oldRowCount;i<nValues;i++)
    {
        currTime += timeStepSize;
        this->editTable->addValue(currTime,value);
    }
}
