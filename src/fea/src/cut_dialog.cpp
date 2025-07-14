#include <cmath>

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "cut_dialog.h"
#include "direction_widget.h"
#include "position_widget.h"

CutDialog::CutDialog(uint modelID, uint entityID, QWidget *parent)
    : QDialog(parent)
    , modelID(modelID)
    , entityID(entityID)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->setWindowTitle(tr("Cut editor"));
    this->resize(500,650);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *titleLabel = new QLabel;
    if (this->entityID == RConstants::eod)
    {
        titleLabel->setText(tr("Create new cut."));
    }
    else
    {
        titleLabel->setText(tr("Modify cut") + ": <b>" + Application::instance()->getSession()->getModel(modelID).getCut(entityID).getName() + "</b>");
    }
    mainLayout->addWidget(titleLabel);

    REntityGroupTypeMask typeMask = R_ENTITY_GROUP_POINT
                                  | R_ENTITY_GROUP_LINE
                                  | R_ENTITY_GROUP_SURFACE
                                  | R_ENTITY_GROUP_VOLUME;

    Model &rModel = Application::instance()->getSession()->getModel(modelID);

    this->modelTree = new ModelTreeSimple(this->modelID,typeMask,this);
    if (this->entityID != RConstants::eod)
    {
        const RCut &rCut = rModel.getCut(entityID);
        this->cutPlane = rCut.getPlane();

        const std::vector<unsigned int> groupIDs = rCut.getElementGroupIDs();
        this->modelTree->clearSelection();
        for (uint i=0;i<groupIDs.size();i++)
        {
            REntityGroupType entityGroupType;
            uint entityIDNum;
            if (rModel.getEntityID(groupIDs[i],entityGroupType,entityIDNum))
            {
                this->modelTree->selectEntity(modelID,entityGroupType,entityIDNum);
            }
        }
    }
    else
    {
        RR3Vector position;
        rModel.findNodeCenter(position[0],position[1],position[2]);
        this->cutPlane.setPosition(position);
    }
    mainLayout->addWidget(this->modelTree);

    QObject::connect(this->modelTree,
                     &ModelTreeSimple::itemSelectionChanged,
                     this,
                     &CutDialog::onModelTreeSelectionChanged);

    double xMin = 0.0;
    double xMax = 0.0;
    double yMin = 0.0;
    double yMax = 0.0;
    double zMin = 0.0;
    double zMax = 0.0;

    rModel.findNodeLimits(xMin,xMax,yMin,yMax,zMin,zMax);

    double dx = xMax - xMin;
    double dy = yMax - yMin;
    double dz = zMax - zMin;

    xMin -= dx;
    xMax += dx;

    yMin -= dy;
    yMax += dy;

    zMin -= dz;
    zMax += dz;

    PositionWidget *positionWidget = new PositionWidget(tr("Plane position"),this->cutPlane.getPosition());
    positionWidget->hideButtons();
    positionWidget->hidePickButton();
    positionWidget->setXRange(xMin,xMax);
    positionWidget->setYRange(yMin,yMax);
    positionWidget->setZRange(zMin,zMax);
    mainLayout->addWidget(positionWidget);

    DirectionWidget *directionWidget = new DirectionWidget(tr("Plane normal"),this->cutPlane.getNormal());
    directionWidget->hideButtons();
    mainLayout->addWidget(directionWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setEnabled(this->modelTree->selectedItems().size());
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&CutDialog::onReject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&CutDialog::onAccept);

    QObject::connect(positionWidget,&PositionWidget::changed,this,&CutDialog::onPositionChanged);
    QObject::connect(directionWidget,&DirectionWidget::changed,this,&CutDialog::onDirectionChanged);

    Application::instance()->getSession()->setBeginDrawCutPlane(this->cutPlane);
}

void CutDialog::onAccept()
{
    Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Create a cut"));

    QList<SessionEntityID> entities = this->modelTree->getSelected();

    if (this->entityID != RConstants::eod)
    {
        RCut &rCut = Application::instance()->getSession()->getModel(this->modelID).getCut(this->entityID);

        rCut.setPlane(this->cutPlane);

        rCut.clearElementGroupIDs();

        for (int i=0;i<entities.size();i++)
        {
            uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getElementGroupID(entities[i].getType(),
                                                                                                   entities[i].getEid());
            if (elementGroupId == RConstants::eod)
            {
                continue;
            }
            rCut.addElementGroupID(elementGroupId);
        }

        RLogger::info("Modified cut \'%s\'\n",rCut.getName().toUtf8().constData());
    }
    else
    {
        RCut cut;

        cut.setName("Cut " + QString::number(Application::instance()->getSession()->getModel(this->modelID).getNCuts() + 1));
        cut.setPlane(this->cutPlane);

        for (int i=0;i<entities.size();i++)
        {
            uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getElementGroupID(entities[i].getType(),
                                                                                                   entities[i].getEid());
            cut.addElementGroupID(elementGroupId);
        }

        Application::instance()->getSession()->getModel(this->modelID).addCut(cut);
        RLogger::info("Created new cut \'%s\'\n",cut.getName().toUtf8().constData());
    }

    Application::instance()->getSession()->setModelChanged(this->modelID);

    // this->close();
    Application::instance()->getSession()->setEndDrawCutPlane();

    this->accept();
}

void CutDialog::onReject()
{
    // this->close();
    Application::instance()->getSession()->setEndDrawCutPlane();
    this->reject();
}

void CutDialog::onPositionChanged(const RR3Vector &position)
{
    this->cutPlane.setPosition(position);
    Application::instance()->getSession()->setBeginDrawCutPlane(this->cutPlane);
}

void CutDialog::onDirectionChanged(const RR3Vector &direction)
{
    this->cutPlane.setNormal(direction);
    Application::instance()->getSession()->setBeginDrawCutPlane(this->cutPlane);
}

void CutDialog::onModelTreeSelectionChanged()
{
    this->okButton->setEnabled(this->modelTree->selectedItems().size());
}
