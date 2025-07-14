#include <QIcon>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "merge_near_nodes_dialog.h"
#include "application.h"
#include "model_action.h"
#include <rbl_job_manager.h>

MergeNearNodesDialog::MergeNearNodesDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Merge near nodes"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *label = new QLabel(tr("Merge near nodes in model") + " <b>" + Application::instance()->getSession()->getModel(this->modelID).getName() + "</b>");
    mainLayout->addWidget(label);

    QFormLayout *toleranceLayout = new QFormLayout;
    mainLayout->addLayout(toleranceLayout);

    this->toleranceEdit = new ValueLineEdit(0.0,1e10);
    this->toleranceEdit->setValue(Application::instance()->getSession()->getModel(this->modelID).findMinimumNodeDistance());
    toleranceLayout->addRow(tr("Tolerance") + ":",this->toleranceEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int MergeNearNodesDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setMergeNearNodes(this->toleranceEdit->getValue());

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}
