#include <QVBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "break_intersected_elements_dialog.h"
#include "model_action.h"

BreakIntersectedElementsDialog::BreakIntersectedElementsDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Break intersected elements"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QBoxLayout::SetFixedSize);
    this->setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->nIterationsSpin = new QSpinBox;
    this->nIterationsSpin->setRange(1,100);
    this->nIterationsSpin->setValue(3);
    formLayout->addRow(tr("Number of iterations") + ":",this->nIterationsSpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

}

int BreakIntersectedElementsDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setBreakIntersectedElements(uint(this->nIterationsSpin->value()));

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}
