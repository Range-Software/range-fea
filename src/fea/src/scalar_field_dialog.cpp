#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "application.h"
#include "scalar_field_dialog.h"

ScalarFieldDialog::ScalarFieldDialog(uint modelID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(RConstants::eod)
{
    this->createDialog();
}

ScalarFieldDialog::ScalarFieldDialog(uint modelID, uint entityID, QWidget *parent) :
    QDialog(parent),
    modelID(modelID),
    entityID(entityID)
{
    this->createDialog();
}

int ScalarFieldDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        RVariableType varType = this->getVariableType();
        QList<SessionEntityID> entities = this->modelTree->getSelected();

        if (varType != R_VARIABLE_NONE && entities.size() > 0)
        {
            Application::instance()->getSession()->storeCurentModelVersion(this->modelID,tr("Create scalar field"));

            if (this->entityID != RConstants::eod)
            {
                RScalarField &scalarField = Application::instance()->getSession()->getModel(this->modelID).getScalarField(this->entityID);

                scalarField.clearElementGroupIDs();

                scalarField.setVariableType(varType);

                for (int i=0;i<entities.size();i++)
                {
                    uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getEntityGroupID(entities[i].getType(),
                                                                                                          entities[i].getEid());
                    if (elementGroupId == RConstants::eod)
                    {
                        continue;
                    }
                    scalarField.addElementGroupID(elementGroupId);
                }
                scalarField.setMaxPointSize(double(this->pointSizeSpin->value()));

                RLogger::info("Modified Scalar field \'%s\'\n",scalarField.getName().toUtf8().constData());
            }
            else
            {
                RScalarField scalarField;

                scalarField.setName(RVariable::getName(varType));
                scalarField.setVariableType(varType);

                for (int i=0;i<entities.size();i++)
                {
                    uint elementGroupId = Application::instance()->getSession()->getModel(this->modelID).getElementGroupID(entities[i].getType(),
                                                                                                           entities[i].getEid());
                    scalarField.addElementGroupID(elementGroupId);
                }
                scalarField.setMaxPointSize(double(this->pointSizeSpin->value()));

                Application::instance()->getSession()->getModel(this->modelID).addScalarField(scalarField);
                RLogger::info("Created new Scalar field \'%s\'\n",scalarField.getName().toUtf8().constData());
            }

            Application::instance()->getSession()->setModelChanged(this->modelID);
        }
    }

    return retVal;
}

void ScalarFieldDialog::createDialog()
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    this->resize(500,400);

    QString windowTitleStr = tr("Scalar field editor");
    this->setWindowTitle(tr(windowTitleStr.toUtf8().constData()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QLabel *titleLabel = new QLabel;
    if (this->entityID == RConstants::eod)
    {
        titleLabel->setText(tr("Create new Scalar field."));
    }
    else
    {
        titleLabel->setText(tr("Modify Scalar field") + ": <b>" + Application::instance()->getSession()->getModel(modelID).getScalarField(entityID).getName() + "</b>");
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
        RScalarField &rScalarField = rModel.getScalarField(entityID);
        const std::vector<unsigned int> groupIDs = rScalarField.getElementGroupIDs();
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
                     &ScalarFieldDialog::onModelTreeSelectionChanged);

    this->variableTree = new QTreeWidget(this);
    this->variableTree->setSelectionMode(QAbstractItemView::SingleSelection);
    this->variableTree->setColumnCount(ScalarFieldDialog::NColumns);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(ScalarFieldDialog::Name,QString("Variables"));
    headerItem->setText(ScalarFieldDialog::Type,QString("type"));
    this->variableTree->setHeaderItem(headerItem);
    this->variableTree->setColumnHidden(ScalarFieldDialog::Type,true);

    Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    for (uint i=0;i<rModel.getNVariables();i++)
    {
        RVariable &rVariable = rModel.getVariable(i);

        QTreeWidgetItem *itemVariable = new QTreeWidgetItem(this->variableTree);
        itemVariable->setText(ScalarFieldDialog::Name, rVariable.getName());
        itemVariable->setData(ScalarFieldDialog::Type,Qt::DisplayRole,QVariant(rVariable.getType()));
        if (this->entityID != RConstants::eod)
        {
            RScalarField &rScalarField = Application::instance()->getSession()->getModel(modelID).getScalarField(entityID);
            itemVariable->setSelected(rVariable.getType() == rScalarField.getVariableType());
        }
    }

    hBoxLayout->addWidget(this->variableTree);

    QObject::connect(this->variableTree,
                     &QTreeWidget::itemSelectionChanged,
                     this,
                     &ScalarFieldDialog::onVariableTreeSelectionChanged);

    QFormLayout *pointSizeLayout = new QFormLayout;
    mainLayout->addLayout(pointSizeLayout);

    this->pointSizeSpin = new QSpinBox;
    this->pointSizeSpin->setMinimum(1);
    this->pointSizeSpin->setMaximum(100);
    if (this->entityID != RConstants::eod)
    {
        RScalarField &rScalarField = Application::instance()->getSession()->getModel(modelID).getScalarField(entityID);
        this->pointSizeSpin->setValue(rScalarField.getMaxPointSize());
    }
    else
    {
        this->pointSizeSpin->setValue(5);
    }
    pointSizeLayout->addRow(tr("Point size in pixels") + ":",this->pointSizeSpin);

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

RVariableType ScalarFieldDialog::getVariableType() const
{
    QList<QTreeWidgetItem*> items = this->variableTree->selectedItems();

    if (items.size() == 0)
    {
        return R_VARIABLE_NONE;
    }

    return RVariableType(items[0]->data(ScalarFieldDialog::Type,Qt::DisplayRole).toInt());
}

void ScalarFieldDialog::onVariableTreeSelectionChanged()
{
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
}

void ScalarFieldDialog::onModelTreeSelectionChanged()
{
    this->okButton->setEnabled(this->variableTree->selectedItems().size() && this->modelTree->selectedItems().size());
}
