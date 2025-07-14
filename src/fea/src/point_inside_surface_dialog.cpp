#include <QVBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_job_manager.h>

#include "point_inside_surface_dialog.h"
#include "model_action.h"

PointInsideSurfaceDialog::PointInsideSurfaceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityIDs(entityIDs)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Point inside surface"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    this->position = new PositionWidget(tr("Point position"),RR3Vector(0.0,0.0,0.0));
    this->position->hideButtons();
    this->position->hideSliders();
    mainLayout->addWidget(this->position);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int PointInsideSurfaceDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setCheckPointInsideSurface(this->position->getPosition(),this->entityIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}
