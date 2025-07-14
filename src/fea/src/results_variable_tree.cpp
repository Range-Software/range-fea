#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>

#include "results_variable_tree.h"
#include "application.h"
#include "graph_dialog.h"
#include "graph_object.h"
#include "model_action.h"
#include <rbl_job_manager.h>
#include "value_scale.h"

typedef enum _ResultsVariableTreeColumn
{
    RESULTS_VARIABLE_TREE_COLUMN_1 = 0,
    RESULTS_VARIABLE_TREE_N_COLUMNS
} ResultsVariableTreeColumn;

ResultsVariableTree::ResultsVariableTree(QWidget *parent) :
    QTreeWidget(parent),
    variableType(R_VARIABLE_NONE)
{
    this->setColumnCount(RESULTS_VARIABLE_TREE_N_COLUMNS);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setRootIsDecorated(false);
    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(RESULTS_VARIABLE_TREE_COLUMN_1,QString(tr("Results display properties")));
    this->setHeaderItem(headerItem);
    this->setHeaderHidden(true);

    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    this->connect(this,
                  SIGNAL(variableDataChanged(uint,RVariableType)),
                  SLOT(onVariableDataChanged(uint,RVariableType)));
    this->connect(this,
                  SIGNAL(variableDataChanged(SessionEntityID,RVariableType)),
                  SLOT(onVariableDataChanged(SessionEntityID,RVariableType)));
}

void ResultsVariableTree::populate()
{
    if (this->variableType == R_VARIABLE_NONE)
    {
        return;
    }

    QString rangeName;
    double minValue = 0.0;
    double maxValue = 0.0;
    double scaleValue = 1.0;
    Qt::CheckState applyAsScalarState = Qt::Unchecked;
    Qt::CheckState applyAsDisplacementState = Qt::Unchecked;
    Qt::CheckState displayRangeState = Qt::Unchecked;
    uint nApplyAsScalar = 0;
    uint nApplyAsDisplacement = 0;
    uint nApplyTotal = 0;
    QString appliesOnStr;

    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<selectedEntityIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid());

        const REntityGroup *pEntityGroup = nullptr;

        switch (selectedEntityIDs[i].getType())
        {
            case R_ENTITY_GROUP_POINT:
                pEntityGroup = model.getPointPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_LINE:
                pEntityGroup = model.getLinePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_SURFACE:
                pEntityGroup = model.getSurfacePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_VOLUME:
                pEntityGroup = model.getVolumePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_VECTOR_FIELD:
                pEntityGroup = model.getVectorFieldPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_SCALAR_FIELD:
                pEntityGroup = model.getScalarFieldPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_STREAM_LINE:
                pEntityGroup = model.getStreamLinePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_CUT:
                pEntityGroup = model.getCutPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_ISO:
                pEntityGroup = model.getIsoPtr(selectedEntityIDs[i].getEid());
                break;
            default:
                break;
        }

        if (!pEntityGroup)
        {
            continue;
        }
        const std::map<RVariableType,REntityGroupVariableDisplayTypeMask> &egVarData = pEntityGroup->getData().getVariableData();
        const std::map<RVariableType,REntityGroupVariableDisplayTypeMask>::const_iterator it = egVarData.find(this->variableType);

        if (it == egVarData.end())
        {
            continue;
        }

        if (REntityGroupVariableDisplayTypeMask(it->second) & R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR)
        {
            nApplyAsScalar++;
        }
        if (REntityGroupVariableDisplayTypeMask(it->second) & R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT)
        {
            nApplyAsDisplacement++;
        }

        nApplyTotal++;
    }

    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    bool appliesOnNode = false;
    bool appliesOnElement = false;
    bool isVector = false;
    uint nActiveModels = 0;
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &rVariable = model.getVariable(j);
            if (rVariable.getType() != this->variableType)
            {
                continue;
            }

            if (rVariable.getNVectors() > 1)
            {
                isVector = true;
            }

            switch (rVariable.getApplyType())
            {
                case R_VARIABLE_APPLY_NODE:
                    appliesOnNode = true;
                    break;
                case R_VARIABLE_APPLY_ELEMENT:
                    appliesOnElement = true;
                    break;
                default:
                    break;
            }

            RVariableData &variableData = rVariable.getVariableData();

            if (nActiveModels == 0)
            {
                minValue = variableData.getMinDisplayValue();
                maxValue = variableData.getMaxDisplayValue();
                scaleValue = variableData.getScale();
                rangeName = variableData.getValueRangeName();

                displayRangeState = variableData.getDisplayValueRange() ? Qt::Checked : Qt::Unchecked;
            }
            else
            {
                minValue = std::min(minValue,variableData.getMinDisplayValue());
                maxValue = std::max(maxValue,variableData.getMaxDisplayValue());
                if ((variableData.getDisplayValueRange() && displayRangeState == Qt::Unchecked)
                    ||
                    (!variableData.getDisplayValueRange() && displayRangeState == Qt::Checked))
                {
                    displayRangeState = Qt::PartiallyChecked;
                }
            }

            nActiveModels ++;
        }
    }

    if (nApplyAsScalar > 0)
    {
        if (nApplyAsScalar == nApplyTotal)
        {
            applyAsScalarState = Qt::Checked;
        }
        else
        {
            applyAsScalarState = Qt::PartiallyChecked;
        }
    }
    if (nApplyAsDisplacement > 0 && isVector)
    {
        if (nApplyAsDisplacement == nApplyTotal)
        {
            applyAsDisplacementState = Qt::Checked;
        }
        else
        {
            applyAsDisplacementState = Qt::PartiallyChecked;
        }
    }

    if (appliesOnNode && appliesOnElement)
    {
        appliesOnStr = tr("Results are displayed on nodes and elements");
    }
    else if (appliesOnNode && !appliesOnElement)
    {
        appliesOnStr = tr("Results are displayed on nodes");
    }
    else if (!appliesOnNode && appliesOnElement)
    {
        appliesOnStr = tr("Results are displayed on elements");
    }
    else
    {
        appliesOnStr = tr("Results can not be displayed");
    }

    this->blockSignals(true);

    QTreeWidgetItem *item;

    if (nActiveModels > 1)
    {
        item = new QTreeWidgetItem(this);
        item->setText(RESULTS_VARIABLE_TREE_COLUMN_1,tr("Editing values for multiple models"));
        item->setForeground(RESULTS_VARIABLE_TREE_COLUMN_1,QBrush(Qt::red));
    }

    // -----------------------------------------------------------------
    // APPLY ON

    item = new QTreeWidgetItem(this);
    item->setText(RESULTS_VARIABLE_TREE_COLUMN_1,appliesOnStr);

    // -----------------------------------------------------------------
    // APPLY AS

    item = new QTreeWidgetItem(this);
    QGroupBox *applyAsGroupBox= new QGroupBox("Apply as ...");
    QGridLayout *applyAsLayout = new QGridLayout;

    uint crow = 0;

    this->applyAsScalarCheckBox = new QCheckBox(REntityGroupData::getVariableDisplayName(R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR));
    this->applyAsScalarCheckBox->setCheckState(applyAsScalarState);
    applyAsLayout->addWidget(this->applyAsScalarCheckBox,crow,0,1,1);

    QObject::connect(this->applyAsScalarCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onApplyAsScalarStateChanged);

    crow++;

    this->applyAsDisplacementCheckBox = new QCheckBox(REntityGroupData::getVariableDisplayName(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT));
    this->applyAsDisplacementCheckBox->setCheckState(applyAsDisplacementState);
    applyAsLayout->addWidget(this->applyAsDisplacementCheckBox,crow,0,1,1);

    this->applyAsDisplacementCheckBox->setEnabled(isVector);

    QObject::connect(this->applyAsDisplacementCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onApplyAsDisplacementStateChanged);

    crow++;

    applyAsGroupBox->setLayout(applyAsLayout);

    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,applyAsGroupBox);

    // -----------------------------------------------------------------
    // DISPLAY RANGE

    item = new QTreeWidgetItem(this);
    QGroupBox *rangeGroupBox= new QGroupBox("Display range");
    QGridLayout *rangeLayout = new QGridLayout;

    crow = 0;

    // Maximum
    this->maxValueScale = new ValueScale(tr("Maximum"));
    this->maxValueScale->setValue(maxValue);
    rangeLayout->addWidget(this->maxValueScale,crow,0,1,2);

    QObject::connect(this->maxValueScale,&ValueScale::valueChanged,this,&ResultsVariableTree::onRangeMaxValueChanged);

    crow++;

    // Minimum
    this->minValueScale = new ValueScale(tr("Minimum"));
    this->minValueScale->setValue(minValue);
    rangeLayout->addWidget(this->minValueScale,crow,0,1,2);

    QObject::connect(this->minValueScale,&ValueScale::valueChanged,this,&ResultsVariableTree::onRangeMinValueChanged);

    crow++;

    // Set default min max ranges.
    QPushButton *rangeAutoFillButton = new QPushButton(tr("Set default min/max values"));
    rangeLayout->addWidget(rangeAutoFillButton,crow,0,1,2);

    QObject::connect(rangeAutoFillButton,&QPushButton::clicked,this,&ResultsVariableTree::onRangeAutofillClicked);

    crow++;

    // Show graph of computed values.
    QPushButton *rangeGraphButton = new QPushButton(tr("Show graph values"));
    rangeLayout->addWidget(rangeGraphButton,crow,0,1,2);

    QObject::connect(rangeGraphButton,&QPushButton::clicked,this,&ResultsVariableTree::onRangeGraphClicked);

    crow++;

    // Color-Scale name
    QLabel *colorScaleLabelLabel = new QLabel(tr("Color scale") + ":");
    rangeLayout->addWidget(colorScaleLabelLabel,crow,0,1,1);

    this->colorScaleComboBox = new ColorScaleComboBox;
    rangeLayout->addWidget(this->colorScaleComboBox,crow,1,1,1);

    this->colorScaleComboBox->setColorScaleName(rangeName);

    this->connect(this->colorScaleComboBox,SIGNAL(currentIndexChanged(int)),SLOT(onColorScaleChanged(int)));

    crow++;

    // Display Value range.

    QCheckBox *displayRangeCheckBox = new QCheckBox("Display value range");
    displayRangeCheckBox->setCheckState(displayRangeState);
    rangeLayout->addWidget(displayRangeCheckBox,crow,0,1,2);

    QObject::connect(displayRangeCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onRangeDisplayStateChanged);

    crow++;

    rangeGroupBox->setLayout(rangeLayout);

    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,rangeGroupBox);

    // -----------------------------------------------------------------
    // SCALE

    item = new QTreeWidgetItem(this);

    ValueScale *valueScale = new ValueScale(tr("Scale"));
    valueScale->setValue(scaleValue);
    valueScale->setEnabled(isVector);
    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,valueScale);

    QObject::connect(valueScale,&ValueScale::valueChanged,this,&ResultsVariableTree::onVariableScaleChanged);

    // -----------------------------------------------------------------
    // VARIABLE ACTIONS

    item = new QTreeWidgetItem(this);
    QGroupBox *actionsGroupBox= new QGroupBox("Actions");
    QGridLayout *actionsLayout = new QGridLayout;

    if (appliesOnElement)
    {
        QPushButton *applyOnNodeButton = new QPushButton(tr("Display on nodes"));
        actionsLayout->addWidget(applyOnNodeButton);
        QObject::connect(applyOnNodeButton,&QPushButton::clicked,this,&ResultsVariableTree::onApplyOnNodeClicked);
    }

    QPushButton *removeVariableButton = new QPushButton(QIcon(":/icons/action/pixmaps/range-cancel.svg"),tr("Remove variable"));
    actionsLayout->addWidget(removeVariableButton);
    QObject::connect(removeVariableButton,&QPushButton::clicked,this,&ResultsVariableTree::onRemoveVariableClicked);

    actionsGroupBox->setLayout(actionsLayout);
    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,actionsGroupBox);

    this->blockSignals(false);
}

void ResultsVariableTree::processApplyAsStates()
{
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<selectedEntityIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid());

        REntityGroup *pEntityGroup = nullptr;

        switch (selectedEntityIDs[i].getType())
        {
            case R_ENTITY_GROUP_POINT:
                pEntityGroup = model.getPointPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_LINE:
                pEntityGroup = model.getLinePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_SURFACE:
                pEntityGroup = model.getSurfacePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_VOLUME:
                pEntityGroup = model.getVolumePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_VECTOR_FIELD:
                pEntityGroup = model.getVectorFieldPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_SCALAR_FIELD:
                pEntityGroup = model.getScalarFieldPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_STREAM_LINE:
                pEntityGroup = model.getStreamLinePtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_CUT:
                pEntityGroup = model.getCutPtr(selectedEntityIDs[i].getEid());
                break;
            case R_ENTITY_GROUP_ISO:
                pEntityGroup = model.getIsoPtr(selectedEntityIDs[i].getEid());
                break;
            default:
                break;
        }

        if (!pEntityGroup)
        {
            continue;
        }
        std::map<RVariableType,REntityGroupVariableDisplayTypeMask> &egVarData = pEntityGroup->getData().getVariableData();

        egVarData.erase(this->variableType);

        if (this->applyAsScalarCheckBox->checkState() == Qt::Checked)
        {
            egVarData[this->variableType] |= R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR;
        }
        if (this->applyAsDisplacementCheckBox->checkState() == Qt::Checked)
        {
            egVarData[this->variableType] |= R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT;
        }

        emit this->variableDataChanged(selectedEntityIDs[i],this->variableType);
    }
}

void ResultsVariableTree::onResultsVariableSelected(RVariableType variableType)
{
    this->variableType = variableType;
    this->clear();
    this->populate();
}

void ResultsVariableTree::onApplyAsScalarStateChanged(Qt::CheckState)
{
    this->processApplyAsStates();
}

void ResultsVariableTree::onApplyAsDisplacementStateChanged(Qt::CheckState)
{
    this->processApplyAsStates();
}

void ResultsVariableTree::onApplyOnNodeClicked()
{
    if (QMessageBox::question(this,
                              tr("Display on nodes?"),
                              tr("Are you sure you want to change display from elements to nodes?"),
                              QMessageBox::No,
                              QMessageBox::Yes) == QMessageBox::No)
    {
        return;
    }

    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        ModelActionInput modelActionInput(selectedModelIDs[i]);
        modelActionInput.setApplyVariableOnNode(this->variableType);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
}

void ResultsVariableTree::onRemoveVariableClicked()
{
    if (QMessageBox::question(this,
                              tr("Remove variable?"),
                              tr("Are you sure you want to remove selected variable?"),
                              QMessageBox::No,
                              QMessageBox::Yes) == QMessageBox::No)
    {
        return;
    }

    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        ModelActionInput modelActionInput(selectedModelIDs[i]);
        modelActionInput.setRemoveVariable(this->variableType);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
}

void ResultsVariableTree::onColorScaleChanged(int index)
{
    const QString &name = this->colorScaleComboBox->itemData(index-1).toString();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &variable = model.getVariable(j);
            if (variable.getType() != this->variableType)
            {
                continue;
            }
            variable.getVariableData().setValueRangeName(name);
            emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
        }
    }
}

void ResultsVariableTree::onRangeMinValueChanged(double minValue)
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &variable = model.getVariable(j);
            if (variable.getType() != this->variableType)
            {
                continue;
            }
            double minDisplayValue = minValue;
            double maxDisplayValue = std::max(minValue,variable.getVariableData().getMaxDisplayValue());
            variable.getVariableData().setMinMaxDisplayValue(minDisplayValue,maxDisplayValue);
            if (this->maxValueScale->getValue() != maxDisplayValue)
            {
                this->blockSignals(true);
                this->maxValueScale->setValue(maxDisplayValue);
                this->blockSignals(false);
            }
            emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
        }
    }
}

void ResultsVariableTree::onRangeMaxValueChanged(double maxValue)
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &variable = model.getVariable(j);
            if (variable.getType() != this->variableType)
            {
                continue;
            }
            double minDisplayValue = std::min(maxValue,variable.getVariableData().getMinDisplayValue());
            double maxDisplayValue = maxValue;
            variable.getVariableData().setMinMaxDisplayValue(minDisplayValue,maxDisplayValue);
            if (this->minValueScale->getValue() != minDisplayValue)
            {
                this->blockSignals(true);
                this->minValueScale->setValue(minDisplayValue);
                this->blockSignals(false);
            }
            emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
        }
    }
}

void ResultsVariableTree::onRangeDisplayStateChanged(Qt::CheckState state)
{
    bool displayRange;

    Qt::CheckState displayRangeState = Qt::CheckState(state);

    if (displayRangeState == Qt::Checked)
    {
        displayRange = true;
    }
    else if (displayRangeState == Qt::Unchecked)
    {
        displayRange = false;
    }
    else
    {
        return;
    }

    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &variable = model.getVariable(j);
            if (variable.getType() != this->variableType)
            {
                continue;
            }
            variable.getVariableData().setDisplayValueRange(displayRange);
            emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
        }
    }
}

void ResultsVariableTree::onRangeAutofillClicked()
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    double minDisplayValue = 0.0;
    double maxDisplayValue = 0.0;
    bool firstValue = true;

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);

        unsigned int variablePos = model.findVariable(this->variableType);
        if (variablePos == RConstants::eod)
        {
            continue;
        }
        RVariable &variable =  model.getVariable(variablePos);

        double minValue = variable.getMinValue();
        double maxValue = variable.getMaxValue();

        if (firstValue)
        {
            minDisplayValue = minValue;
            maxDisplayValue = maxValue;
            firstValue = false;
        }

        minDisplayValue = std::min(minValue,minDisplayValue);
        maxDisplayValue = std::max(maxValue,maxDisplayValue);
    }

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &rVariable = model.getVariable(j);
            if (rVariable.getType() != this->variableType)
            {
                continue;
            }
            rVariable.getVariableData().setMinMaxDisplayValue(minDisplayValue,maxDisplayValue);
            this->blockSignals(true);
            this->minValueScale->setValue(minDisplayValue);
            this->maxValueScale->setValue(maxDisplayValue);
            this->blockSignals(false);
            emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
        }
    }
}

void ResultsVariableTree::onRangeGraphClicked()
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    RRVector values;

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &variable = model.getVariable(j);
            if (variable.getType() != this->variableType)
            {
                continue;
            }

            values.append(variable.getValues());
        }
    }

    RRMatrix distValues = RStatistics::findDistributedValues(values);

    GraphObject *graphObject = new GraphObject;

    GraphData &rGraphData = graphObject->getData();

    rGraphData.setTitle(RVariable::getName(this->variableType));
    rGraphData.setXLabel("Value");
    rGraphData.setYLabel("Population");
    rGraphData.setTypeMask(GRAPH_BARS);

    for (uint i=0;i<distValues.getNColumns();i++)
    {
        RRVector &rValues = rGraphData[distValues[0][i]];
        rValues.resize(1);
        rValues[0] = distValues[1][i];
    }

    rGraphData.setLimits(this->minValueScale->getValue(),
                         this->maxValueScale->getValue());

    GraphDialog graphDialog(graphObject,Application::instance()->getMainWindow());
    graphDialog.getGraphWidget()->setPaintGraphLimits(true);
    graphDialog.getGraphWidget()->setShowPointer(true,true,true);
    graphDialog.exec();

    this->blockSignals(true);
    this->minValueScale->setValue(rGraphData.getLLimit());
    this->maxValueScale->setValue(rGraphData.getULimit());
    this->blockSignals(false);

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
    }
}

void ResultsVariableTree::onVariableScaleChanged(double scale)
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &rVariable = model.getVariable(j);
            if (rVariable.getType() != this->variableType)
            {
                continue;
            }
            rVariable.getVariableData().setScale(scale);
            emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
        }
    }
}

void ResultsVariableTree::onVariableDataChanged(uint mid, RVariableType variableType)
{
    Application::instance()->getSession()->setVariableDataChanged(mid,variableType);
}

void ResultsVariableTree::onVariableDataChanged(const SessionEntityID &entityID, RVariableType variableType)
{
    Application::instance()->getSession()->setVariableDataChanged(entityID,variableType);
}
