#include <algorithm>

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <rgl_message_box.h>

#include "results_variable_tree.h"
#include "application.h"
#include "graph_dialog.h"
#include "graph_object.h"
#include "model_action.h"
#include <rbl_job_manager.h>
#include "value_scale.h"
#include "session.h"
#include "model.h"
#include "application_settings.h"
#include "main_window.h"
#include <rgl_icon.h>

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
    Qt::CheckState smoothDisplayState = Qt::Unchecked;
    uint nApplyAsScalar = 0;
    uint nApplyAsDisplacement = 0;
    uint nApplyTotal = 0;
    QString appliesOnStr;
    QString units;

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
                units = rVariable.getUnits();

                displayRangeState = variableData.getDisplayValueRange() ? Qt::Checked : Qt::Unchecked;
                smoothDisplayState = model.getSmoothVariable(this->variableType) ? Qt::Checked : Qt::Unchecked;
            }
            else
            {
                if (model.getSmoothVariable(this->variableType) != (smoothDisplayState == Qt::Checked))
                {
                    smoothDisplayState = Qt::PartiallyChecked;
                }
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
        appliesOnStr = tr("On nodes and elements");
    }
    else if (appliesOnNode && !appliesOnElement)
    {
        appliesOnStr = tr("On nodes");
    }
    else if (!appliesOnNode && appliesOnElement)
    {
        appliesOnStr = tr("On elements");
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

    ApplicationSettings *applicationSettings = Application::instance()->getApplicationSettings();

    // -----------------------------------------------------------------
    // WHERE RESULTS ARE STORED

    QWidget *locationWidget = new QWidget;
    QHBoxLayout *locationLayout = new QHBoxLayout;
    locationLayout->setContentsMargins(4,2,4,2);
    locationWidget->setLayout(locationLayout);

    locationLayout->addWidget(new QLabel(appliesOnStr));
    locationLayout->addStretch(1);

    // Display only, results stored on elements stay unchanged.
    const bool canSmooth = appliesOnElement && Model::canSmoothVariable(this->variableType);
    if (canSmooth)
    {
        QCheckBox *smoothDisplayCheckBox = new QCheckBox(tr("Smooth display"));
        smoothDisplayCheckBox->setToolTip(tr("Averages element results at nodes for display only, results themselves stay unchanged.\nAveraging lowers peak values, compare them with the element range shown in the value range."));
        smoothDisplayCheckBox->setCheckState(smoothDisplayState);
        locationLayout->addWidget(smoothDisplayCheckBox);
        QObject::connect(smoothDisplayCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onSmoothDisplayStateChanged);
    }
    else if (appliesOnElement)
    {
        QPushButton *applyOnNodeButton = new QPushButton(tr("Display on nodes"));
        locationLayout->addWidget(applyOnNodeButton);
        QObject::connect(applyOnNodeButton,&QPushButton::clicked,this,&ResultsVariableTree::onApplyOnNodeClicked);
    }

    item = new QTreeWidgetItem(this);
    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,locationWidget);

    // -----------------------------------------------------------------
    // SHOW ON SELECTED ENTITIES

    const bool hasSelectedEntities = !selectedEntityIDs.isEmpty();
    const QString showToolTip = hasSelectedEntities ? QString() : tr("Select entities in the model tree first.");

    QVBoxLayout *showLayout = this->addSection(tr("Show on selected entities") + " (" + QLocale().toString(selectedEntityIDs.size()) + ")");
    QHBoxLayout *showRowLayout = new QHBoxLayout;
    showLayout->addLayout(showRowLayout);

    this->applyAsScalarCheckBox = new QCheckBox(tr("Color map"));
    this->applyAsScalarCheckBox->setCheckState(applyAsScalarState);
    this->applyAsScalarCheckBox->setEnabled(hasSelectedEntities);
    this->applyAsScalarCheckBox->setToolTip(showToolTip);
    showRowLayout->addWidget(this->applyAsScalarCheckBox);

    QObject::connect(this->applyAsScalarCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onApplyAsScalarStateChanged);

    this->applyAsDisplacementCheckBox = new QCheckBox(tr("Deformed shape"));
    this->applyAsDisplacementCheckBox->setCheckState(applyAsDisplacementState);
    this->applyAsDisplacementCheckBox->setEnabled(hasSelectedEntities);
    this->applyAsDisplacementCheckBox->setToolTip(showToolTip);
    // Widget shown before it has a parent would open as a separate window.
    showRowLayout->addWidget(this->applyAsDisplacementCheckBox);
    this->applyAsDisplacementCheckBox->setVisible(isVector);
    showRowLayout->addStretch(1);

    QObject::connect(this->applyAsDisplacementCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onApplyAsDisplacementStateChanged);

    // -----------------------------------------------------------------
    // COLOR RANGE

    QPushButton *rangeAutoFillButton = new QPushButton(tr("Reset to data"));
    rangeAutoFillButton->setToolTip(tr("Set the range back to the default computed from the results."));
    QObject::connect(rangeAutoFillButton,&QPushButton::clicked,this,&ResultsVariableTree::onRangeAutofillClicked);

    QVBoxLayout *rangeLayout = this->addSection(tr("Color range"),rangeAutoFillButton);

    const bool rangeFineAdjust = applicationSettings->getResultsRangeFineAdjust();

    // Maximum
    this->maxValueScale = new ValueScale(tr("Max"));
    this->maxValueScale->setValue(maxValue);
    this->maxValueScale->setUnits(units);
    this->maxValueScale->setFineAdjustVisible(rangeFineAdjust);
    rangeLayout->addWidget(this->maxValueScale);

    QObject::connect(this->maxValueScale,&ValueScale::valueChanged,this,&ResultsVariableTree::onRangeMaxValueChanged);

    // Minimum
    this->minValueScale = new ValueScale(tr("Min"));
    this->minValueScale->setValue(minValue);
    this->minValueScale->setUnits(units);
    this->minValueScale->setFineAdjustVisible(rangeFineAdjust);
    rangeLayout->addWidget(this->minValueScale);

    QObject::connect(this->minValueScale,&ValueScale::valueChanged,this,&ResultsVariableTree::onRangeMinValueChanged);

    const int rangeTitleWidth = std::max(this->maxValueScale->findTitleWidth(),this->minValueScale->findTitleWidth());
    this->maxValueScale->setTitleWidth(rangeTitleWidth);
    this->minValueScale->setTitleWidth(rangeTitleWidth);

    QToolButton *rangeFineAdjustButton = ResultsVariableTree::createFineAdjustButton(rangeFineAdjust);
    rangeLayout->addWidget(rangeFineAdjustButton);

    QObject::connect(rangeFineAdjustButton,&QToolButton::toggled,this,[this](bool checked)
    {
        Application::instance()->getApplicationSettings()->setResultsRangeFineAdjust(checked);
        this->maxValueScale->setFineAdjustVisible(checked);
        this->minValueScale->setFineAdjustVisible(checked);
        QTimer::singleShot(0,this,[this](){ this->doItemsLayout(); });
    });

    // Colors
    QHBoxLayout *colorsLayout = new QHBoxLayout;
    rangeLayout->addLayout(colorsLayout);

    colorsLayout->addWidget(new QLabel(tr("Colors")));

    this->colorScaleComboBox = new ColorScaleComboBox;
    colorsLayout->addWidget(this->colorScaleComboBox,1);

    this->colorScaleComboBox->setColorScaleName(rangeName);

    this->connect(this->colorScaleComboBox,SIGNAL(currentIndexChanged(int)),SLOT(onColorScaleChanged(int)));

    // Legend
    QCheckBox *displayRangeCheckBox = new QCheckBox(tr("Show legend"));
    displayRangeCheckBox->setCheckState(displayRangeState);
    rangeLayout->addWidget(displayRangeCheckBox);

    QObject::connect(displayRangeCheckBox,&QCheckBox::checkStateChanged,this,&ResultsVariableTree::onRangeDisplayStateChanged);

    // -----------------------------------------------------------------
    // DEFORMATION AND ARROW SCALE

    if (isVector)
    {
        QVBoxLayout *scaleLayout = this->addSection(tr("Deformation and arrow scale"));

        const bool scaleFineAdjust = applicationSettings->getResultsScaleFineAdjust();

        ValueScale *valueScale = new ValueScale(QString());
        valueScale->setValue(scaleValue);
        valueScale->setUnits(QStringLiteral("×"));
        valueScale->setLogSliderVisible(true);
        valueScale->setFineAdjustVisible(scaleFineAdjust);
        valueScale->setToolTip(tr("Multiplies the deformation of deformed shapes and the length of vector field arrows."));
        scaleLayout->addWidget(valueScale);

        QObject::connect(valueScale,&ValueScale::valueChanged,this,&ResultsVariableTree::onVariableScaleChanged);

        QToolButton *scaleFineAdjustButton = ResultsVariableTree::createFineAdjustButton(scaleFineAdjust);
        scaleLayout->addWidget(scaleFineAdjustButton);

        QObject::connect(scaleFineAdjustButton,&QToolButton::toggled,this,[this,valueScale](bool checked)
        {
            Application::instance()->getApplicationSettings()->setResultsScaleFineAdjust(checked);
            valueScale->setFineAdjustVisible(checked);
            QTimer::singleShot(0,this,[this](){ this->doItemsLayout(); });
        });
    }

    // -----------------------------------------------------------------
    // VARIABLE ACTIONS

    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout;
    actionsLayout->setContentsMargins(4,2,4,2);
    actionsWidget->setLayout(actionsLayout);

    QPushButton *rangeGraphButton = new QPushButton(tr("Graph") + QStringLiteral("…"));
    rangeGraphButton->setToolTip(tr("Show distribution of values and set the color range on a graph."));
    actionsLayout->addWidget(rangeGraphButton);
    QObject::connect(rangeGraphButton,&QPushButton::clicked,this,&ResultsVariableTree::onRangeGraphClicked);

    actionsLayout->addStretch(1);

    QPushButton *removeVariableButton = new QPushButton(RIcon::fromFile(":/icons/action/pixmaps/range-cancel.svg"),tr("Remove"));
    removeVariableButton->setToolTip(tr("Remove this variable from the model."));
    actionsLayout->addWidget(removeVariableButton);
    QObject::connect(removeVariableButton,&QPushButton::clicked,this,&ResultsVariableTree::onRemoveVariableClicked);

    item = new QTreeWidgetItem(this);
    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,actionsWidget);

    this->blockSignals(false);
}

QVBoxLayout *ResultsVariableTree::addSection(const QString &title, QWidget *headerWidget)
{
    QGroupBox *groupBox = new QGroupBox;
    QVBoxLayout *sectionLayout = new QVBoxLayout;
    groupBox->setLayout(sectionLayout);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    sectionLayout->addLayout(headerLayout);

    QLabel *titleLabel = new QLabel(title);
    QFont titleFont(titleLabel->font());
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);
    if (headerWidget)
    {
        headerLayout->addWidget(headerWidget);
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    this->setItemWidget(item,RESULTS_VARIABLE_TREE_COLUMN_1,groupBox);

    return sectionLayout;
}

QToolButton *ResultsVariableTree::createFineAdjustButton(bool expanded)
{
    QToolButton *button = new QToolButton;
    button->setText(tr("Fine adjust"));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setAutoRaise(true);
    button->setCheckable(true);
    button->setChecked(expanded);
    button->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);

    QObject::connect(button,&QToolButton::toggled,button,[button](bool checked)
    {
        button->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    });

    return button;
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
        REntityGroupData &rEntityGroupData = pEntityGroup->getData();
        std::map<RVariableType,REntityGroupVariableDisplayTypeMask> &egVarData = rEntityGroupData.getVariableData();

        egVarData.erase(this->variableType);

        if (this->applyAsScalarCheckBox->checkState() == Qt::Checked)
        {
            // Only one variable at a time can be displayed as scalar - a left over
            // entry, e.g. results of a previously solved problem type, would keep
            // shadowing this one.
            rEntityGroupData.clearVariableDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR);
            egVarData[this->variableType] |= R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR;
        }
        if (this->applyAsDisplacementCheckBox->checkState() == Qt::Checked)
        {
            rEntityGroupData.clearVariableDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT);
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

void ResultsVariableTree::onSmoothDisplayStateChanged(Qt::CheckState state)
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    double minDisplayValue = 0.0;
    double maxDisplayValue = 0.0;
    bool firstValue = true;

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);

        const uint variablePosition = model.findVariable(this->variableType);
        if (variablePosition == RConstants::eod)
        {
            continue;
        }

        model.setSmoothVariable(this->variableType,state == Qt::Checked);

        const RVariableData &rVariableData = model.getVariable(variablePosition).getVariableData();
        minDisplayValue = firstValue ? rVariableData.getMinDisplayValue() : std::min(minDisplayValue,rVariableData.getMinDisplayValue());
        maxDisplayValue = firstValue ? rVariableData.getMaxDisplayValue() : std::max(maxDisplayValue,rVariableData.getMaxDisplayValue());
        firstValue = false;

        emit this->variableDataChanged(selectedModelIDs[i],this->variableType);
    }

    if (!firstValue)
    {
        this->minValueScale->blockSignals(true);
        this->maxValueScale->blockSignals(true);
        this->minValueScale->setValue(minDisplayValue);
        this->maxValueScale->setValue(maxDisplayValue);
        this->minValueScale->blockSignals(false);
        this->maxValueScale->blockSignals(false);
    }
}

void ResultsVariableTree::onApplyOnNodeClicked()
{
    if (RMessageBox::question(this,
                              tr("Display on nodes?"),
                              tr("Are you sure you want to change display from elements to nodes?")) == RMessageBox::No)
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
    if (RMessageBox::question(this,
                              tr("Remove variable?"),
                              tr("Are you sure you want to remove selected variable?")) == RMessageBox::No)
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

        double minValue = 0.0;
        double maxValue = 0.0;
        model.findDefaultDisplayRange(this->variableType,minValue,maxValue);

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
