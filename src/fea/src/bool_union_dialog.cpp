#include <QVBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include "bool_union_dialog.h"
#include "model_action.h"
#include <rbl_job_manager.h>

BoolUnionDialog::BoolUnionDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityIDs(entityIDs)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Union"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QBoxLayout::SetFixedSize);
    this->setLayout(mainLayout);

    QLabel *messageLabel = new QLabel(tr("Create union from selected surfaces?"));
    mainLayout->addWidget(messageLabel);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int BoolUnionDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setBoolUnion(this->entityIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}
