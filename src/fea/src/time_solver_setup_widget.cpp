#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>

#include <rml_variable.h>

#include "time_solver_setup_widget.h"
#include "value_line_edit.h"

TimeSolverSetupWidget::TimeSolverSetupWidget(const RTimeSolver &timeSolver, QWidget *parent)
    : QWidget(parent)
    , timeSolver(timeSolver)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Time-solver"));
    groupBox->setToolTip(tr("Enable/disable time-dependent solution."));
    groupBox->setCheckable(true);
    groupBox->setChecked(this->timeSolver.getEnabled());
    mainLayout->addWidget(groupBox);

    QObject::connect(groupBox,&QGroupBox::toggled,this,&TimeSolverSetupWidget::onTimeSolverEnabledChanged);

    QGridLayout *groupLayout = new QGridLayout;
    groupBox->setLayout(groupLayout);

    int groupLayoutRow = 0;

    // Time-march approximation
    QString toolTipTimeApprox(tr("Time-march approximation method."));

    QLabel *labelTimeApprox = new QLabel(tr("Approximation"));
    labelTimeApprox->setToolTip(toolTipTimeApprox);
    groupLayout->addWidget(labelTimeApprox,groupLayoutRow,0);

    QComboBox *comboTimeApprox = new QComboBox();
    comboTimeApprox->addItem(RTimeSolver::getTimeMarchApproximationName(R_TIME_MARCH_CENTRAL));
    comboTimeApprox->addItem(RTimeSolver::getTimeMarchApproximationName(R_TIME_MARCH_BACKWARD));
    comboTimeApprox->addItem(RTimeSolver::getTimeMarchApproximationName(R_TIME_MARCH_FORWARD));
    comboTimeApprox->setCurrentIndex(this->timeSolver.getTimeMarchApproximation());
    comboTimeApprox->setToolTip(toolTipTimeApprox);
    groupLayout->addWidget(comboTimeApprox,groupLayoutRow++,1,1,2);

    this->connect(comboTimeApprox,SIGNAL(currentIndexChanged(int)),SLOT(onTimeApproximationChanged(int)));

    // Start time
    QString toolTipStartTime(tr("Start time in seconds.")
                             + "\n"
                             + tr("In case of solver restart, entered value will be ignored and current computed time will be used.")
                             + "\n"
                             + tr("Current computed time") + " = " + QLocale().toString(this->timeSolver.getComputedTime()) + RVariable::getUnits(R_VARIABLE_TIME));

    QLabel *labelStartTime = new QLabel(tr("Start time"));
    labelStartTime->setToolTip(toolTipStartTime);
    groupLayout->addWidget(labelStartTime,groupLayoutRow,0);

    ValueLineEdit *lineStartTime = new ValueLineEdit;
    lineStartTime->setDoubleValidator();
    lineStartTime->setValue(this->timeSolver.getInputStartTime());
    lineStartTime->setToolTip(toolTipStartTime);
    groupLayout->addWidget(lineStartTime,groupLayoutRow,1);

    QObject::connect(lineStartTime,&ValueLineEdit::valueChanged,this,&TimeSolverSetupWidget::onStartTimeChanged);

    QLabel *unitsStartTime = new QLabel(RVariable::getUnits(R_VARIABLE_TIME));
    groupLayout->addWidget(unitsStartTime,groupLayoutRow++,2);

    // End time
    QString toolTipEndTime(tr("End time in seconds.")
                           + "\n"
                           + tr("End time = Start time + Time-step size * Number of time-steps"));

    QLabel *labelEndTime = new QLabel(tr("End time"));
    labelEndTime->setToolTip(toolTipEndTime);
    groupLayout->addWidget(labelEndTime,groupLayoutRow,0);

    this->valueEndTime = new QLabel(QLocale().toString(this->findEndTime()));
    this->valueEndTime->setToolTip(toolTipEndTime);
    groupLayout->addWidget(this->valueEndTime,groupLayoutRow,1,Qt::AlignRight);

    QLabel *unitsEndTime = new QLabel(RVariable::getUnits(R_VARIABLE_TIME));
    groupLayout->addWidget(unitsEndTime,groupLayoutRow++,2);

    // Time-step size
    QString toolTipTimeStepSize(tr("Time-step size in seconds."));

    QLabel *labelTimeStepSize = new QLabel(tr("Time-step size"));
    labelTimeStepSize->setToolTip(toolTipTimeStepSize);
    groupLayout->addWidget(labelTimeStepSize,groupLayoutRow,0);

    ValueLineEdit *lineTimeStepSize = new ValueLineEdit(1.0e-99,1.0e99);
    lineTimeStepSize->setValue(this->timeSolver.getInputTimeStepSize());
    lineTimeStepSize->setToolTip(toolTipTimeStepSize);
    groupLayout->addWidget(lineTimeStepSize,groupLayoutRow,1);

    QObject::connect(lineTimeStepSize,&ValueLineEdit::valueChanged,this,&TimeSolverSetupWidget::onTimeStepSizeChanged);

    QLabel *unitsTimeStepSize = new QLabel(RVariable::getUnits(R_VARIABLE_TIME));
    groupLayout->addWidget(unitsTimeStepSize,groupLayoutRow++,2);

    // Number of time-steps
    QString toolTipNTimeSteps(tr("Number of time-steps to solve."));

    QLabel *labelNTimeSteps = new QLabel(tr("Number of time-steps"));
    labelNTimeSteps->setToolTip(toolTipNTimeSteps);
    groupLayout->addWidget(labelNTimeSteps,groupLayoutRow,0);

    QSpinBox *spinNTimeSteps = new QSpinBox;
    spinNTimeSteps->setMinimum(R_TIME_STEP_MIN_NUMBER);
    spinNTimeSteps->setMaximum(R_TIME_STEP_MAX_NUMBER);
    spinNTimeSteps->setValue(this->timeSolver.getInputNTimeSteps());
    spinNTimeSteps->setToolTip(toolTipNTimeSteps);
    groupLayout->addWidget(spinNTimeSteps,groupLayoutRow++,1,1,2);

    this->connect(spinNTimeSteps,SIGNAL(valueChanged(int)),SLOT(onNTimeStepsChanged(int)));

    // Write output frequency
    QString toolTipOutputFrequency(tr("Number of time-steps to be computed for results to be written into a file."));

    QLabel *labelOutputFrequency = new QLabel(tr("Output frequency"));
    labelOutputFrequency->setToolTip(toolTipOutputFrequency);
    groupLayout->addWidget(labelOutputFrequency,groupLayoutRow,0);

    QSpinBox *spinOutputFrequency = new QSpinBox;
    spinOutputFrequency->setMinimum(R_TIME_MIN_OUTPUT_FREQUENCY);
    spinOutputFrequency->setMaximum(R_TIME_MAX_OUTPUT_FREQUENCY);
    spinOutputFrequency->setValue(this->timeSolver.getOutputFrequency());
    spinOutputFrequency->setToolTip(toolTipOutputFrequency);
    groupLayout->addWidget(spinOutputFrequency,groupLayoutRow++,1,1,2);

    this->connect(spinOutputFrequency,SIGNAL(valueChanged(int)),SLOT(onOutputFrequencyChanged(int)));
}

double TimeSolverSetupWidget::findEndTime() const
{
    return this->timeSolver.getInputStartTime() + this->timeSolver.getInputTimeStepSize() * this->timeSolver.getInputNTimeSteps();
}

void TimeSolverSetupWidget::onTimeSolverEnabledChanged(bool checked)
{
    this->timeSolver.setEnabled(checked);
    emit this->changed(this->timeSolver);
}

void TimeSolverSetupWidget::onTimeApproximationChanged(int index)
{
    RTimeMarchApproximation tma = RTimeMarchApproximation(index);
    if (R_TIME_MARCH_APPROXIMATION_TYPE_IS_VALID(tma))
    {
        this->timeSolver.setTimeMarchApproximation(tma);
        emit this->changed(this->timeSolver);
    }
}

void TimeSolverSetupWidget::onStartTimeChanged(double startTime)
{
    this->timeSolver.setInputStartTime(startTime);
    this->valueEndTime->setText(QLocale().toString(this->findEndTime()));
    emit this->changed(this->timeSolver);
}

void TimeSolverSetupWidget::onTimeStepSizeChanged(double timeStepSize)
{
    this->timeSolver.setInputTimeStepSize(timeStepSize);
    this->valueEndTime->setText(QLocale().toString(this->findEndTime()));
    emit this->changed(this->timeSolver);
}

void TimeSolverSetupWidget::onNTimeStepsChanged(int nTimeSteps)
{
    this->timeSolver.setInputNTimeSteps(nTimeSteps);
    this->valueEndTime->setText(QLocale().toString(this->findEndTime()));
    emit this->changed(this->timeSolver);
}

void TimeSolverSetupWidget::onOutputFrequencyChanged(int outputFrequency)
{
    this->timeSolver.setOutputFrequency(outputFrequency);
    this->valueEndTime->setText(QLocale().toString(this->findEndTime()));
    emit this->changed(this->timeSolver);
}
