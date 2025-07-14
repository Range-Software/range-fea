#include <QVBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <float.h>

#include <rbl_utils.h>

#include "coarsen_surface_dialog.h"
#include "model_action.h"
#include <rbl_job_manager.h>
#include "application.h"

static uint mid = RConstants::eod;
static double edgeLength = RConstants::eps;
static double elementArea = RConstants::eps;

CoarsenSurfaceDialog::CoarsenSurfaceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityIDs(entityIDs)
{
    if (mid != modelID)
    {
        this->findGeometryLimits();
    }

    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Coarsen surface elements"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *messageLabel = new QLabel(tr("Coarsen selected surfaces."));
    mainLayout->addWidget(messageLabel);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->edgeLengthEdit = new ValueLineEdit(0.0,DBL_MAX);
    this->edgeLengthEdit->setValue(edgeLength);
    formLayout->addRow(tr("Edge length") + ":",this->edgeLengthEdit);

    this->elementAreaEdit = new ValueLineEdit(0.0,DBL_MAX);
    this->elementAreaEdit->setValue(elementArea);
    formLayout->addRow(tr("Element area") + ":",this->elementAreaEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int CoarsenSurfaceDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        mid = this->modelID;
        edgeLength = this->edgeLengthEdit->getValue();
        elementArea = this->elementAreaEdit->getValue();

        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setCoarsenSurface(this->entityIDs,this->edgeLengthEdit->getValue(),this->elementAreaEdit->getValue());

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}

void CoarsenSurfaceDialog::findGeometryLimits()
{
    edgeLength = elementArea = RConstants::eps;

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    bool firstTime = true;
    for (uint i=0;i<rModel.getNElements();i++)
    {
        const RElement &rElement = rModel.getElement(i);

        if (!R_ELEMENT_TYPE_IS_SURFACE(rElement.getType()))
        {
            continue;
        }

        uint n1, n2;
        double lenght, area;

        if (!rElement.findShortestEdge(rModel.getNodes(),lenght,n1,n2))
        {
            continue;
        }

        if (!rElement.findArea(rModel.getNodes(),area))
        {
            continue;
        }

        if (firstTime)
        {
            edgeLength = lenght;
            elementArea = area;
            firstTime = false;
        }
        else
        {
            edgeLength = std::min(edgeLength,lenght);
            elementArea = std::min(elementArea,area);
        }
    }
}
