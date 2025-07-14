#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QScrollArea>

#include <rbl_job_manager.h>

#include "geometry_transform_widget.h"
#include "model_action.h"
#include "application.h"

GeometryTransformWidget::GeometryTransformWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    this->setLayout(vBoxLayout);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    vBoxLayout->addWidget(scrollArea);

    QWidget *containerWidget = new QWidget;
    scrollArea->setWidget(containerWidget);

    QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);

    QTabWidget *tabWidget = new QTabWidget;
    containerLayout->addWidget(tabWidget);

    this->scaleWidget = new GeometryScaleWidget(this->input.getScaleCenter(),this->input.getScale());
    tabWidget->addTab(this->scaleWidget,tr("Scale"));

    this->rotateWidget = new GeometryRotateWidget(this->input.getRotationCenter(),this->input.getRotation());
    tabWidget->addTab(this->rotateWidget,tr("Rotate"));

    this->translateWidget = new GeometryTranslateWidget(this->input.getTranslation());
    tabWidget->addTab(this->translateWidget,tr("Translate"));

    QGroupBox *applyToGroupBox = new QGroupBox(tr("Apply to"));
    containerLayout->addWidget(applyToGroupBox);

    QVBoxLayout *applyToLayout = new QVBoxLayout;
    applyToGroupBox->setLayout(applyToLayout);

    this->applyToAllRadio = new QRadioButton(tr("All"));
    this->applyToAllRadio->setChecked(this->input.getApplyTo() == GeometryTransformInput::ApplyToAll);
    applyToLayout->addWidget(this->applyToAllRadio);

    this->applyToSelectedRadio = new QRadioButton(tr("Selected entities"));
    this->applyToSelectedRadio->setChecked(this->input.getApplyTo() == GeometryTransformInput::ApplyToSelected);
    this->applyToSelectedRadio->setEnabled(Application::instance()->getSession()->getSelectedEntityIDs().size() > 0);
    applyToLayout->addWidget(this->applyToSelectedRadio);

    this->applyToPickedRadio = new QRadioButton(tr("Picked entities"));
    this->applyToPickedRadio->setChecked(this->input.getApplyTo() == GeometryTransformInput::ApplyToPicked);
    this->applyToPickedRadio->setEnabled(!Application::instance()->getSession()->getPickList().isEmpty());
    applyToLayout->addWidget(this->applyToPickedRadio);

    this->applyToVisibleRadio = new QRadioButton(tr("Visible entities"));
    this->applyToVisibleRadio->setChecked(this->input.getApplyTo() == GeometryTransformInput::ApplyToVisible);
    applyToLayout->addWidget(this->applyToVisibleRadio);

    this->includeSharedNodesCheck = new QGroupBox(tr("Include shared nodes"));
    this->includeSharedNodesCheck->setCheckable(true);
    this->includeSharedNodesCheck->setChecked(this->input.getIncludeSharedNodes());
    containerLayout->addWidget(this->includeSharedNodesCheck);
    this->includeSharedNodesCheck->setDisabled(this->input.getApplyTo() == GeometryTransformInput::ApplyToAll);

    QVBoxLayout *includeSharedNodesLayout = new QVBoxLayout;
    this->includeSharedNodesCheck->setLayout(includeSharedNodesLayout);

    this->splitSharedNodesCheck = new QGroupBox(tr("Detach/split shared nodes"));
    this->splitSharedNodesCheck->setCheckable(true);
    this->splitSharedNodesCheck->setChecked(this->input.getSplitSharedNodes());
    includeSharedNodesLayout->addWidget(this->splitSharedNodesCheck);

    QVBoxLayout *splitSharedNodesLayout = new QVBoxLayout;
    this->splitSharedNodesCheck->setLayout(splitSharedNodesLayout);

    this->sweepNodesCheck = new QGroupBox(tr("Sweep nodes"));
    this->sweepNodesCheck->setCheckable(true);
    this->sweepNodesCheck->setChecked(this->input.getSweepSharedNodes());
    splitSharedNodesLayout->addWidget(this->sweepNodesCheck);

    QVBoxLayout *sweepNodesLayout = new QVBoxLayout;
    this->sweepNodesCheck->setLayout(sweepNodesLayout);

    this->sweepNStepsSpin = new QSpinBox;
    this->sweepNStepsSpin->setPrefix(tr("Sweep steps") + ": ");
    this->sweepNStepsSpin->setMinimum(1);
    this->sweepNStepsSpin->setValue(this->input.getNSweepSteps());
    sweepNodesLayout->addWidget(this->sweepNStepsSpin);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    vBoxLayout->addLayout(buttonsLayout);

    this->cancelButton = new QPushButton(QIcon(":/icons/action/pixmaps/range-cancel.svg"),tr("Cancel"));
    buttonsLayout->addWidget(this->cancelButton);

    this->okButton = new QPushButton(QIcon(":/icons/action/pixmaps/range-ok.svg"),tr("Ok"));
    buttonsLayout->addWidget(this->okButton);

    this->enableOkButton();

    QObject::connect(Application::instance()->getSession(),
                     &Session::modelSelectionChanged,
                     this,
                     &GeometryTransformWidget::onModelSelectionChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::pickListChanged,
                     this,
                     &GeometryTransformWidget::onPickListChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelVisibilityChanged,
                     this,
                     &GeometryTransformWidget::onModelVisibilityChanged);

    QObject::connect(scaleWidget,
                     &GeometryScaleWidget::scaleChanged,
                     this,
                     &GeometryTransformWidget::onScaleChanged);
    QObject::connect(rotateWidget,
                     &GeometryRotateWidget::rotationChanged,
                     this,
                     &GeometryTransformWidget::onRotationChanged);
    QObject::connect(translateWidget,
                     &GeometryTranslateWidget::translationChanged,
                     this,
                     &GeometryTransformWidget::onTranslationChanged);

    QObject::connect(this->applyToAllRadio,
                     &QRadioButton::clicked,
                     this,
                     &GeometryTransformWidget::onApplyToAllClicked);
    QObject::connect(this->applyToSelectedRadio,
                     &QRadioButton::clicked,
                     this,
                     &GeometryTransformWidget::onApplyToSelectedClicked);
    QObject::connect(this->applyToPickedRadio,
                     &QRadioButton::clicked,
                     this,
                     &GeometryTransformWidget::onApplyToPickedClicked);
    QObject::connect(this->applyToVisibleRadio,
                     &QRadioButton::clicked,
                     this,
                     &GeometryTransformWidget::onApplyToVisibleClicked);

    QObject::connect(this->includeSharedNodesCheck,
                     &QGroupBox::clicked,
                     this,
                     &GeometryTransformWidget::onIncludeSharedNodesClicked);
    QObject::connect(this->splitSharedNodesCheck,
                     &QGroupBox::clicked,
                     this,
                     &GeometryTransformWidget::onSplitSharedNodesClicked);
    QObject::connect(this->sweepNodesCheck,
                     &QGroupBox::clicked,
                     this,
                     &GeometryTransformWidget::onSweepSharedNodesClicked);
    this->connect(this->sweepNStepsSpin,SIGNAL(valueChanged(int)),SLOT(onSweepNStepsSpinChanged(int)));

    QObject::connect(this->cancelButton,
                     &QPushButton::clicked,
                     this,
                     &GeometryTransformWidget::onCancelClicked);
    QObject::connect(this->okButton,
                     &QPushButton::clicked,
                     this,
                     &GeometryTransformWidget::onOkClicked);
}

void GeometryTransformWidget::enableOkButton()
{
    bool entityIsSelected = true;

    if (this->input.getApplyTo() == GeometryTransformInput::ApplyToSelected)
    {
        entityIsSelected = !Application::instance()->getSession()->getSelectedModelIDs().isEmpty();
    }
    else if (this->input.getApplyTo() == GeometryTransformInput::ApplyToPicked)
    {
        entityIsSelected = !Application::instance()->getSession()->getPickList().getModelIDs().isEmpty();
    }
    else if (this->input.getApplyTo() == GeometryTransformInput::ApplyToVisible)
    {
        entityIsSelected = !Application::instance()->getSession()->getVisibleEntityIDs().isEmpty();
    }

    this->okButton->setEnabled(entityIsSelected && (this->input.isRotateActive() || this->input.isScaleActive() || this->input.isTranslateActive()));
}

void GeometryTransformWidget::onModelSelectionChanged(uint)
{
    this->applyToSelectedRadio->setEnabled(!Application::instance()->getSession()->getSelectedEntityIDs().isEmpty());
    this->enableOkButton();
}

void GeometryTransformWidget::onModelVisibilityChanged(uint)
{
    this->applyToVisibleRadio->setEnabled(!Application::instance()->getSession()->getVisibleEntityIDs().isEmpty());
    this->enableOkButton();
}

void GeometryTransformWidget::onPickListChanged()
{
    this->applyToPickedRadio->setEnabled(!Application::instance()->getSession()->getPickList().isEmpty());
    this->enableOkButton();
}

void GeometryTransformWidget::onScaleChanged(const RR3Vector &center, const RR3Vector &scales)
{
    this->input.setScale(scales);
    this->input.setScaleCenter(center);
    this->enableOkButton();
}

void GeometryTransformWidget::onRotationChanged(const RR3Vector &center, const RR3Vector &rotations)
{
    this->input.setRotation(rotations);
    this->input.setRotationCenter(center);
    this->enableOkButton();
}

void GeometryTransformWidget::onTranslationChanged(const RR3Vector &distances)
{
    this->input.setTranslation(distances);
    this->enableOkButton();
}

void GeometryTransformWidget::onApplyToAllClicked(bool checked)
{
    if (checked)
    {
        this->input.setApplyTo(GeometryTransformInput::ApplyToAll);
    }
    this->includeSharedNodesCheck->setDisabled(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onApplyToSelectedClicked(bool checked)
{
    if (checked)
    {
        this->input.setApplyTo(GeometryTransformInput::ApplyToSelected);
    }
    this->includeSharedNodesCheck->setEnabled(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onApplyToPickedClicked(bool checked)
{
    if (checked)
    {
        this->input.setApplyTo(GeometryTransformInput::ApplyToPicked);
    }
    this->includeSharedNodesCheck->setEnabled(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onApplyToVisibleClicked(bool checked)
{
    if (checked)
    {
        this->input.setApplyTo(GeometryTransformInput::ApplyToVisible);
    }
    this->includeSharedNodesCheck->setEnabled(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onIncludeSharedNodesClicked(bool checked)
{
    this->input.setIncludeSharedNodes(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onSplitSharedNodesClicked(bool checked)
{
    this->input.setSplitSharedNodes(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onSweepSharedNodesClicked(bool checked)
{
    this->input.setSweepSharedNodes(checked);
    this->enableOkButton();
}

void GeometryTransformWidget::onSweepNStepsSpinChanged(int nSweepSteps)
{
    this->input.setNSweepSteps(uint(nSweepSteps));
    this->enableOkButton();
}

void GeometryTransformWidget::onCancelClicked()
{
    emit this->finalize();
}

void GeometryTransformWidget::onOkClicked()
{
    QList<uint> modelIDs;

    if (this->input.getApplyTo() == GeometryTransformInput::ApplyToAll)
    {
        for (uint i=0;i<Application::instance()->getSession()->getNModels();i++)
        {
            modelIDs.push_back(i);
        }
    }
    else if (this->input.getApplyTo() == GeometryTransformInput::ApplyToSelected)
    {
        modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    }
    else if (this->input.getApplyTo() == GeometryTransformInput::ApplyToPicked)
    {
        modelIDs = Application::instance()->getSession()->getPickList().getModelIDs();
    }
    else if (this->input.getApplyTo() == GeometryTransformInput::ApplyToVisible)
    {
        modelIDs = Application::instance()->getSession()->getVisibleModelIDs();
    }

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelActionInput modelActionInput(modelIDs.at(i));
        modelActionInput.setGeometryTransform(this->input);
        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }

    emit this->finalize();
}
