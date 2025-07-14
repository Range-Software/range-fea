#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "vector_field_dialog.h"

VectorFieldDialog::VectorFieldDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(RConstants::eod)
{
    this->createDialog();
}

VectorFieldDialog::VectorFieldDialog(uint modelID, uint entityID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(entityID)
{
    this->createDialog();
}

int VectorFieldDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        RVariableType varType = this->getVariableType();
        QList<SessionEntityID> entities = this->modelTree->getSelected();

        if (varType != R_VARIABLE_NONE && entities.size() > 0)
        {
            Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Create vector field"));

            if (this->entityID != RConstants::eod)
            {
                RVectorField &vectorField = Application::instance()->getSession()->getModel(this->modelID).getVectorField(this->entityID);

                vectorField.clearElementGroupIDs();

                vectorField.setVariableType(varType);
                vectorField.setType3D(this->type3DCheckbox->checkState() != Qt::Unchecked);

                for (int i=0;i<entities.size();i++)
                {
                    uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getEntityGroupID(entities[i].getType(),
                                                                                                          entities[i].getEid());
                    if (elementGroupId == RConstants::eod)
                    {
                        continue;
                    }
                    vectorField.addElementGroupID(elementGroupId);
                }

                RLogger::info("Modified vector field \'%s\'\n",vectorField.getName().toUtf8().constData());
            }
            else
            {
                RVectorField vectorField;

                vectorField.setName(RVariable::getName(varType));
                vectorField.setVariableType(varType);
                Qt::CheckState type3DCheckState = this->type3DCheckbox->checkState();
                if (type3DCheckState != Qt::PartiallyChecked)
                {
                    vectorField.setType3D(type3DCheckState == Qt::Checked);
                }

                for (int i=0;i<entities.size();i++)
                {
                    uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getEntityGroupID(entities[i].getType(),
                                                                                                          entities[i].getEid());
                    if (elementGroupId == RConstants::eod)
                    {
                        continue;
                    }
                    vectorField.addElementGroupID(elementGroupId);
                }

                Application::instance()->getSession()->getModel(this->modelID).addVectorField(vectorField);
                RLogger::info("Created new vector field \'%s\'\n",vectorField.getName().toUtf8().constData());
            }

            Application::instance()->getSession()->setModelChanged(this->modelID);
        }
    }

    return retVal;
}

void VectorFieldDialog::createDialog()
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->resize(500,400);

    QString windowTitleStr = tr("Vector field editor");
    this->setWindowTitle(tr(windowTitleStr.toUtf8().constData()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *titleLabel = new QLabel;
    if (this->entityID == RConstants::eod)
    {
        titleLabel->setText(tr("Create new vector field."));
    }
    else
    {
        titleLabel->setText(tr("Modify vector field")  + ": <b>" + Application::instance()->getSession()->getModel(modelID).getVectorField(entityID).getName() + "</b>");
    }
    mainLayout->addWidget(titleLabel);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    mainLayout->addLayout(hBoxLayout);

    REntityGroupTypeMask typeMask = R_ENTITY_GROUP_POINT
                                  | R_ENTITY_GROUP_LINE
                                  | R_ENTITY_GROUP_SURFACE
                                  | R_ENTITY_GROUP_VOLUME
                                  | R_ENTITY_GROUP_CUT
                                  | R_ENTITY_GROUP_ISO
                                  | R_ENTITY_GROUP_STREAM_LINE;

    this->modelTree = new ModelTreeSimple(this->modelID,typeMask,this);
    if (this->entityID != RConstants::eod)
    {
        Model &rModel = Application::instance()->getSession()->getModel(modelID);
        RVectorField &rVectorField = rModel.getVectorField(entityID);
        const std::vector<unsigned int> groupIDs = rVectorField.getElementGroupIDs();
        this->modelTree->clearSelection();
        for (uint i=0;i<groupIDs.size();i++)
        {
            REntityGroupType entityGroupType;
            uint entityIDNum;
            if (rModel.getEntityID(groupIDs[i],entityGroupType,entityIDNum))
            {
                this->modelTree->selectEntity(this->modelID,entityGroupType,entityIDNum);
            }
        }
    }
    hBoxLayout->addWidget(this->modelTree);

    QObject::connect(this->modelTree,
                     &QTreeWidget::itemSelectionChanged,
                     this,
                     &VectorFieldDialog::onModelTreeSelectionChanged);

    this->variableTree = new QTreeWidget(this);
    this->variableTree->setSelectionMode(QAbstractItemView::SingleSelection);
    this->variableTree->setColumnCount(VectorFieldDialog::NColumns);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(VectorFieldDialog::Name,QString("Variables"));
    headerItem->setText(VectorFieldDialog::Type,QString("type"));
    this->variableTree->setHeaderItem(headerItem);
    this->variableTree->setColumnHidden(VectorFieldDialog::Type,true);

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    uint nType3D = 0;
    uint nVariables = 0;
    for (uint i=0;i<rModel.getNVariables();i++)
    {
        RVariable &rVariable = rModel.getVariable(i);
        if (rVariable.getNVectors() > 1)
        {
            QTreeWidgetItem *itemVariable = new QTreeWidgetItem(this->variableTree);
            itemVariable->setText(VectorFieldDialog::Name, rVariable.getName());
            itemVariable->setData(VectorFieldDialog::Type,Qt::DisplayRole,QVariant(rVariable.getType()));
            if (this->entityID != RConstants::eod)
            {
                RVectorField &rVectorField = Application::instance()->getSession()->getModel(modelID).getVectorField(entityID);
                itemVariable->setSelected(rVariable.getType() == rVectorField.getVariableType());
                if (rVectorField.getType3D())
                {
                    nType3D++;
                }
                nVariables++;
            }
        }
    }

    hBoxLayout->addWidget(this->variableTree);

    QObject::connect(this->variableTree,
                     &QTreeWidget::itemSelectionChanged,
                     this,
                     &VectorFieldDialog::onVariableTreeSelectionChanged);

    Qt::CheckState type3DCheckState = (this->entityID == RConstants::eod) ? Qt::Checked : Qt::Unchecked;
    if (nType3D > 0)
    {
        type3DCheckState = (nType3D == nVariables) ? Qt::Checked : Qt::PartiallyChecked;
    }
    this->type3DCheckbox = new QCheckBox("3D vector field");
    this->type3DCheckbox->setCheckState(type3DCheckState);
    mainLayout->addWidget(this->type3DCheckbox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
    buttonBox->addButton(this->okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

RVariableType VectorFieldDialog::getVariableType() const
{
    QList<QTreeWidgetItem*> items = this->variableTree->selectedItems();

    if (items.size() == 0)
    {
        return R_VARIABLE_NONE;
    }

    return RVariableType(items[0]->data(VectorFieldDialog::Type,Qt::DisplayRole).toInt());
}

void VectorFieldDialog::onVariableTreeSelectionChanged()
{
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
}

void VectorFieldDialog::onModelTreeSelectionChanged()
{
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
}
