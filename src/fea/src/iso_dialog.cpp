#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "iso_dialog.h"

IsoDialog::IsoDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(RConstants::eod)
{
    this->createDialog();
}

IsoDialog::IsoDialog(uint modelID, uint entityID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(entityID)
{
    this->createDialog();
}

int IsoDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Create ISO"));

        RVariableType varType = this->getVariableType();
        QList<SessionEntityID> entities = this->modelTree->getSelected();

        if (varType != R_VARIABLE_NONE && entities.size() > 0)
        {
            if (this->entityID != RConstants::eod)
            {
                RIso &iso = Application::instance()->getSession()->getModel(this->modelID).getIso(this->entityID);

                iso.clearElementGroupIDs();

                iso.setVariableType(varType);
                iso.setVariableValue(this->variableValue->getValue());

                for (int i=0;i<entities.size();i++)
                {
                    uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getElementGroupID(entities[i].getType(),
                                                                                                           entities[i].getEid());
                    if (elementGroupId == RConstants::eod)
                    {
                        continue;
                    }
                    iso.addElementGroupID(elementGroupId);
                }

                RLogger::info("Modified ISO entity \'%s\'\n",iso.getName().toUtf8().constData());
            }
            else
            {
                RIso iso;

                iso.setName(RVariable::getName(varType));
                iso.setVariableType(varType);
                iso.setVariableValue(this->variableValue->getValue());

                for (int i=0;i<entities.size();i++)
                {
                    uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getElementGroupID(entities[i].getType(),
                                                                                                           entities[i].getEid());
                    iso.addElementGroupID(elementGroupId);
                }

                Application::instance()->getSession()->getModel(this->modelID).addIso(iso);
                RLogger::info("Created new ISO entity \'%s\'\n",iso.getName().toUtf8().constData());
            }

            Application::instance()->getSession()->setModelChanged(this->modelID);
        }
    }

    return retVal;
}

void IsoDialog::createDialog()
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->resize(500,400);

    QString windowTitleStr = tr("ISO entity editor");
    this->setWindowTitle(tr(windowTitleStr.toUtf8().constData()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *titleLabel = new QLabel;
    if (this->entityID == RConstants::eod)
    {
        titleLabel->setText(tr("Create new ISO entity."));
    }
    else
    {
        titleLabel->setText(tr("Modify ISO entity") + ": <b>" + Application::instance()->getSession()->getModel(modelID).getIso(entityID).getName() + "</b>");
    }
    mainLayout->addWidget(titleLabel);

    QHBoxLayout *entitiesLayout = new QHBoxLayout;
    mainLayout->addLayout(entitiesLayout);

    REntityGroupTypeMask typeMask = R_ENTITY_GROUP_POINT
                                  | R_ENTITY_GROUP_LINE
                                  | R_ENTITY_GROUP_SURFACE
                                  | R_ENTITY_GROUP_VOLUME;

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    this->modelTree = new ModelTreeSimple(this->modelID,typeMask,this);
    if (this->entityID != RConstants::eod)
    {
        RIso &rIso = rModel.getIso(entityID);
        const std::vector<unsigned int> groupIDs = rIso.getElementGroupIDs();
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
    entitiesLayout->addWidget(this->modelTree);

    QObject::connect(this->modelTree,
                     &QTreeWidget::itemSelectionChanged,
                     this,
                     &IsoDialog::onModelTreeSelectionChanged);

    this->variableTree = new QTreeWidget(this);
    this->variableTree->setSelectionMode(QAbstractItemView::SingleSelection);
    this->variableTree->setColumnCount(IsoDialog::NColumns);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(IsoDialog::Name,QString("Variables"));
    headerItem->setText(IsoDialog::Type,QString("type"));
    this->variableTree->setHeaderItem(headerItem);
    this->variableTree->setColumnHidden(IsoDialog::Type,true);

    double variableValue = 0.0;
    double minVariableValue = 0.0;
    double maxVariableValue = 0.0;

    for (uint i=0;i<rModel.getNVariables();i++)
    {
        RVariable &rVariable = rModel.getVariable(i);
        if (rVariable.getApplyType() != R_VARIABLE_APPLY_NODE)
        {
            continue;
        }

        if (i == 0)
        {
            minVariableValue = rVariable.getMinValue();
            maxVariableValue = rVariable.getMaxValue();
        }

        QTreeWidgetItem *itemVariable = new QTreeWidgetItem(this->variableTree);
        itemVariable->setText(IsoDialog::Name, rVariable.getName());
        itemVariable->setData(IsoDialog::Type,Qt::DisplayRole,QVariant(rVariable.getType()));
        if (this->entityID != RConstants::eod)
        {
            RIso &rIso = Application::instance()->getSession()->getModel(this->modelID).getIso(entityID);
            if (rVariable.getType() == rIso.getVariableType())
            {
                itemVariable->setSelected(true);

                variableValue = rIso.getVariableValue();

                minVariableValue = rVariable.getMinValue();
                maxVariableValue = rVariable.getMaxValue();
            }
        }
    }

    entitiesLayout->addWidget(this->variableTree);

    QObject::connect(this->variableTree,
                     &QTreeWidget::itemSelectionChanged,
                     this,
                     &IsoDialog::onVariableTreeSelectionChanged);

    QHBoxLayout *valueLayout = new QHBoxLayout;
    mainLayout->addLayout(valueLayout);

    QLabel *valueLabel = new QLabel(tr("Value") + ":");
    valueLayout->addWidget(valueLabel);

    this->variableValue = new ValueLineEdit(minVariableValue, maxVariableValue);
    this->variableValue->setValue(variableValue);
    valueLayout->addWidget(this->variableValue);

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

RVariableType IsoDialog::getVariableType() const
{
    QList<QTreeWidgetItem*> items = this->variableTree->selectedItems();

    if (items.size() == 0)
    {
        return R_VARIABLE_NONE;
    }

    return RVariableType(items[0]->data(IsoDialog::Type,Qt::DisplayRole).toInt());
}

void IsoDialog::onVariableTreeSelectionChanged()
{
    RVariableType variableType = this->getVariableType();

    this->variableValue->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());

    if (!this->variableTree->selectedItems().size() || !this->modelTree->selectedItems().size())
    {
        return;
    }

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    double minVariableValue = 0.0;
    double maxVariableValue = 0.0;

    for (uint i=0;i<rModel.getNVariables();i++)
    {
        RVariable &rVariable = rModel.getVariable(i);

        if (rVariable.getType() == variableType)
        {
            minVariableValue = rVariable.getMinValue();
            maxVariableValue = rVariable.getMaxValue();
        }
    }

    this->variableValue->setRange(minVariableValue,maxVariableValue);
}

void IsoDialog::onModelTreeSelectionChanged()
{
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
}
