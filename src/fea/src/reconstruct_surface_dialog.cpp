#include <QVBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <cmath>
#include <float.h>

#include <rbl_utils.h>
#include <rbl_job_manager.h>
#include <rml_surface_reconstructor.h>

#include "reconstruct_surface_dialog.h"
#include "model_action.h"
#include "application.h"
#include "model.h"
#include "session.h"
#include <rgl_icon.h>

// Last used values (per model).
static uint lastModelID = RConstants::eod;
static bool lastValuesValid = false;
static ReconstructSurfaceDialog::Values lastValues;

ReconstructSurfaceDialog::ReconstructSurfaceDialog(uint modelID, const QList<SessionEntityID> &entityIDs, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityIDs(entityIDs)
{
    QIcon cancelIcon = RIcon::fromFile(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon = RIcon::fromFile(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Reconstruct surface"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *messageLabel = new QLabel(tr("Retriangulate selected surfaces: merge coplanar elements, remove nodes which do not define the shape "
                                         "(inside planar regions and on straight edges), tiny and sliver elements.\n"
                                         "Existing nodes are never moved, shared nodes and edges are preserved."));
    messageLabel->setWordWrap(true);
    mainLayout->addWidget(messageLabel);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    this->angleToleranceSpin = new QDoubleSpinBox;
    this->angleToleranceSpin->setRange(0.0,90.0);
    this->angleToleranceSpin->setDecimals(2);
    this->angleToleranceSpin->setSingleStep(0.5);
    this->angleToleranceSpin->setSuffix(QString(" %1").arg(QChar(0x00B0)));
    this->angleToleranceSpin->setToolTip(tr("Maximum angle between normals of coplanar elements and between directions of collinear edges."));
    formLayout->addRow(tr("Angle tolerance") + ":",this->angleToleranceSpin);

    this->distanceToleranceEdit = new ValueLineEdit(0.0,DBL_MAX);
    this->distanceToleranceEdit->setToolTip(tr("Maximum distance of a node from a plane (line) to be considered coplanar (collinear)."));
    formLayout->addRow(tr("Distance tolerance") + ":",this->distanceToleranceEdit);

    this->minEdgeLengthEdit = new ValueLineEdit(0.0,DBL_MAX);
    this->minEdgeLengthEdit->setToolTip(tr("Edges shorter than this are collapsed. Features smaller than this value may be removed."));
    formLayout->addRow(tr("Minimum edge length") + ":",this->minEdgeLengthEdit);

    this->sliverAngleSpin = new QDoubleSpinBox;
    this->sliverAngleSpin->setRange(0.0,60.0);
    this->sliverAngleSpin->setDecimals(2);
    this->sliverAngleSpin->setSingleStep(1.0);
    this->sliverAngleSpin->setSuffix(QString(" %1").arg(QChar(0x00B0)));
    this->sliverAngleSpin->setToolTip(tr("Elements with smallest angle below this value are considered slivers."));
    formLayout->addRow(tr("Sliver angle") + ":",this->sliverAngleSpin);

    this->delaunayCheck = new QCheckBox(tr("Delaunay triangulation"));
    this->delaunayCheck->setToolTip(tr("Finish with Delaunay triangulation of the surface - flip edges whose opposite angles sum to more than 180 degrees, "
                                       "provided the shape is preserved within distance tolerance."));
    formLayout->addRow(this->delaunayCheck);

    this->refineCheck = new QCheckBox(tr("Quality mesh (insert nodes)"));
    this->refineCheck->setToolTip(tr("Insert new nodes (Delaunay refinement) so that elements have smallest angle at least the quality angle "
                                     "and edges are not longer than the maximum edge length. New nodes are placed on the existing surface "
                                     "and on straight feature edges, so the shape is preserved. Edges shared with unselected entities are not split."));
    formLayout->addRow(this->refineCheck);

    this->qualityAngleSpin = new QDoubleSpinBox;
    this->qualityAngleSpin->setRange(0.0,34.0);
    this->qualityAngleSpin->setDecimals(1);
    this->qualityAngleSpin->setSingleStep(1.0);
    this->qualityAngleSpin->setSuffix(QString(" %1").arg(QChar(0x00B0)));
    this->qualityAngleSpin->setToolTip(tr("Target smallest angle of elements. Values above 30 degrees may lead to excessive number of elements."));
    formLayout->addRow(tr("Quality angle") + ":",this->qualityAngleSpin);

    this->maxEdgeLengthEdit = new ValueLineEdit(0.0,DBL_MAX);
    this->maxEdgeLengthEdit->setToolTip(tr("Maximum edge length of elements. Zero means no limit."));
    formLayout->addRow(tr("Maximum edge length") + ":",this->maxEdgeLengthEdit);

    if (lastValuesValid && lastModelID == modelID)
    {
        this->setValues(lastValues);
    }
    else
    {
        this->resetToDefaults();
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *defaultsButton = new QPushButton(tr("Defaults"));
    buttonBox->addButton(defaultsButton,QDialogButtonBox::ResetRole);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("OK"));
    okButton->setDefault(true);
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(defaultsButton,&QPushButton::clicked,this,&ReconstructSurfaceDialog::resetToDefaults);
    QObject::connect(this->refineCheck,&QCheckBox::toggled,this,&ReconstructSurfaceDialog::onRefineToggled);
    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int ReconstructSurfaceDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        lastModelID = this->modelID;
        lastValues = this->getValues();
        lastValuesValid = true;

        RSurfaceReconstructor::Options options;
        options.angleTolerance = R_DEG_TO_RAD(lastValues.angleTolerance);
        options.distanceTolerance = lastValues.distanceTolerance;
        options.minEdgeLength = lastValues.minEdgeLength;
        options.sliverAngle = R_DEG_TO_RAD(lastValues.sliverAngle);
        options.delaunay = lastValues.delaunay;
        options.refine = lastValues.refine;
        options.qualityAngle = R_DEG_TO_RAD(lastValues.qualityAngle);
        options.maxEdgeLength = lastValues.maxEdgeLength;

        ModelActionInput modelActionInput(this->modelID);
        modelActionInput.setReconstructSurface(this->entityIDs,options);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    return retVal;
}

ReconstructSurfaceDialog::Values ReconstructSurfaceDialog::findDefaultValues() const
{
    RSurfaceReconstructor::Options options;

    Values values;
    values.angleTolerance = R_RAD_TO_DEG(options.angleTolerance);
    values.sliverAngle = R_RAD_TO_DEG(options.sliverAngle);
    values.delaunay = options.delaunay;
    values.refine = options.refine;
    values.qualityAngle = R_RAD_TO_DEG(options.qualityAngle);
    values.maxEdgeLength = options.maxEdgeLength;

    // Same automatic values as the reconstructor uses - relative to the size of selected surfaces.
    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    bool firstTime = true;
    double lower[3] = {0.0,0.0,0.0};
    double upper[3] = {0.0,0.0,0.0};
    for (const SessionEntityID &entityID : this->entityIDs)
    {
        if (entityID.getType() != R_ENTITY_GROUP_SURFACE || entityID.getEid() >= rModel.getNSurfaces())
        {
            continue;
        }
        const RSurface &rSurface = rModel.getSurface(entityID.getEid());
        for (uint i=0;i<rSurface.size();i++)
        {
            const RElement &rElement = rModel.getElement(rSurface.get(i));
            for (uint j=0;j<rElement.size();j++)
            {
                const RNode &rNode = rModel.getNode(rElement.getNodeId(j));
                const double p[3] = {rNode.getX(),rNode.getY(),rNode.getZ()};
                for (uint k=0;k<3;k++)
                {
                    lower[k] = firstTime ? p[k] : std::min(lower[k],p[k]);
                    upper[k] = firstTime ? p[k] : std::max(upper[k],p[k]);
                }
                firstTime = false;
            }
        }
    }
    double diagonal = std::sqrt((upper[0]-lower[0])*(upper[0]-lower[0]) +
                                (upper[1]-lower[1])*(upper[1]-lower[1]) +
                                (upper[2]-lower[2])*(upper[2]-lower[2]));
    if (diagonal <= 0.0)
    {
        diagonal = 1.0;
    }

    values.distanceTolerance = 1.0e-5 * diagonal;
    values.minEdgeLength = 10.0 * values.distanceTolerance;

    return values;
}

void ReconstructSurfaceDialog::setValues(const Values &values)
{
    this->angleToleranceSpin->setValue(values.angleTolerance);
    this->distanceToleranceEdit->setValue(values.distanceTolerance);
    this->minEdgeLengthEdit->setValue(values.minEdgeLength);
    this->sliverAngleSpin->setValue(values.sliverAngle);
    this->delaunayCheck->setChecked(values.delaunay);
    this->refineCheck->setChecked(values.refine);
    this->qualityAngleSpin->setValue(values.qualityAngle);
    this->maxEdgeLengthEdit->setValue(values.maxEdgeLength);
    this->onRefineToggled(values.refine);
}

ReconstructSurfaceDialog::Values ReconstructSurfaceDialog::getValues() const
{
    Values values;
    values.angleTolerance = this->angleToleranceSpin->value();
    values.distanceTolerance = this->distanceToleranceEdit->getValue();
    values.minEdgeLength = this->minEdgeLengthEdit->getValue();
    values.sliverAngle = this->sliverAngleSpin->value();
    values.delaunay = this->delaunayCheck->isChecked();
    values.refine = this->refineCheck->isChecked();
    values.qualityAngle = this->qualityAngleSpin->value();
    values.maxEdgeLength = this->maxEdgeLengthEdit->getValue();
    return values;
}

void ReconstructSurfaceDialog::resetToDefaults()
{
    this->setValues(this->findDefaultValues());
}

void ReconstructSurfaceDialog::onRefineToggled(bool checked)
{
    this->qualityAngleSpin->setEnabled(checked);
    this->maxEdgeLengthEdit->setEnabled(checked);
}
