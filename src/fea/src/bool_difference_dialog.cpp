#include <QVBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "bool_difference_dialog.h"
#include "model_action.h"
#include "application.h"

BoolDifferenceDialog::BoolDifferenceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent) :
    QDialog(parent),
    modelID(modelID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Difference"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QBoxLayout::SetFixedSize);
    this->setLayout(mainLayout);

    QLabel *label = new QLabel(tr("Select entity which will be substracted."));
    mainLayout->addWidget(label);

    this->listWidget = new QListWidget;
    this->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(this->listWidget);

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getMid() != this->modelID)
        {
            continue;
        }
        uint groupID = rModel.getEntityGroupID(entityIDs[i].getType(),entityIDs[i].getEid());
        if (groupID == RConstants::eod)
        {
            continue;
        }
        const REntityGroup *pEntityGroup = rModel.getEntityGroupPtr(groupID,true);
        if (!pEntityGroup)
        {
            continue;
        }
        this->entityIDs.push_back(entityIDs[i]);

        QListWidgetItem *item = new QListWidgetItem(this->listWidget);
        item->setText(pEntityGroup->getName());
        item->setSelected(true);
        item->setData(Qt::UserRole,QVariant(this->entityIDs.size()-1));
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

int BoolDifferenceDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        QList<QListWidgetItem*> selectedItems = this->listWidget->selectedItems();
        if (selectedItems.size() > 0)
        {
            int position = selectedItems[0]->data(Qt::UserRole).toInt();
            if (position >= 0 && position < this->entityIDs.size())
            {
                qSwap(this->entityIDs[position],this->entityIDs[this->entityIDs.size()-1]);
            }
        }

        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setBoolDifference(this->entityIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}
