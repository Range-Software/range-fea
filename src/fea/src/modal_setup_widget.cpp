#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>

#include "modal_setup_widget.h"
#include "value_line_edit.h"

ModalSetupWidget::ModalSetupWidget(const RModalSetup &modalSetup, QWidget *parent)
    : QWidget(parent)
    , modalSetup(modalSetup)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Modal analysis setup"));
    mainLayout->addWidget(groupBox);

    QGridLayout *groupLayout = new QGridLayout;
    groupBox->setLayout(groupLayout);

    int groupLayoutRow = 0;

    // Modal method
    QLabel *labelModalMethod = new QLabel(tr("Modal method"));
    groupLayout->addWidget(labelModalMethod,groupLayoutRow,0);

    QComboBox *comboModalMethod = new QComboBox();
    for (uint i=0;i<R_MODAL_N_TYPES;i++)
    {
        comboModalMethod->addItem(RModalSetup::getMethodName(RModalMethod(i)));
    }
    comboModalMethod->setCurrentIndex(this->modalSetup.getMethod());
    groupLayout->addWidget(comboModalMethod,groupLayoutRow++,1);

    this->connect(comboModalMethod,SIGNAL(currentIndexChanged(int)),SLOT(onModalMethodChanged(int)));

    // Number of iterations
    QLabel *labelNIterations = new QLabel(tr("Iterations"));
    groupLayout->addWidget(labelNIterations,groupLayoutRow,0);

    this->spinNIterations = new QSpinBox;
    this->spinNIterations->setMinimum(R_MODAL_ITERATIONS_MIN_NUMBER);
    this->spinNIterations->setMaximum(R_MODAL_ITERATIONS_MAX_NUMBER);
    this->spinNIterations->setValue(this->modalSetup.getNIterations());
    this->spinNIterations->setToolTip(tr("Number of modal iterations to be performed."));
    this->spinNIterations->setEnabled(this->modalSetup.getMethod() == R_MODAL_MULTIPLE_MODES);
    groupLayout->addWidget(this->spinNIterations,groupLayoutRow++,1);

    this->connect(this->spinNIterations,SIGNAL(valueChanged(int)),SLOT(onNIterationsChanged(int)));

    // Number of modes to extract
    QLabel *labelNModes = new QLabel(tr("Extract modes"));
    groupLayout->addWidget(labelNModes,groupLayoutRow,0);

    this->spinNModesExtract = new QSpinBox;
    this->spinNModesExtract->setMinimum(R_MODAL_ITERATIONS_MIN_NUMBER);
    this->spinNModesExtract->setMaximum(spinNIterations->value());
    this->spinNModesExtract->setValue(this->modalSetup.getNModesToExtract());
    this->spinNModesExtract->setToolTip(tr("Number of modes to extract."));
    this->spinNModesExtract->setEnabled(this->modalSetup.getMethod() == R_MODAL_MULTIPLE_MODES);
    groupLayout->addWidget(this->spinNModesExtract,groupLayoutRow++,1);

    this->connect(this->spinNModesExtract,SIGNAL(valueChanged(int)),SLOT(onNModesExtractChanged(int)));

    // Convergence value
    QLabel *labelConvergenceValue = new QLabel(tr("Convergence value"));
    groupLayout->addWidget(labelConvergenceValue,groupLayoutRow,0);

    ValueLineEdit *lineConvergenceValue = new ValueLineEdit(1.0e-99,1.0e99);
    lineConvergenceValue->setValue(this->modalSetup.getConvergenceValue());
    lineConvergenceValue->setToolTip(tr("Value at which it solver will assume that solution has converged."));
    groupLayout->addWidget(lineConvergenceValue,groupLayoutRow++,1);

    QObject::connect(lineConvergenceValue,&ValueLineEdit::valueChanged,this,&ModalSetupWidget::onConvergenceValueChanged);
}

void ModalSetupWidget::onModalMethodChanged(int index)
{
    RModalMethod method = RModalMethod(index);
    this->modalSetup.setMethod(method);
    this->spinNIterations->setEnabled(method == R_MODAL_MULTIPLE_MODES);
    this->spinNModesExtract->setEnabled(method == R_MODAL_MULTIPLE_MODES);
    emit this->changed(this->modalSetup);
}

void ModalSetupWidget::onNIterationsChanged(int nIterations)
{
    this->modalSetup.setNIterations(uint(nIterations));
    this->spinNModesExtract->blockSignals(true);
    this->spinNModesExtract->setMaximum(nIterations);
    this->spinNModesExtract->blockSignals(false);
    if (this->modalSetup.getNModesToExtract() > this->modalSetup.getNIterations())
    {
        this->modalSetup.setNModesToExtract(this->modalSetup.getNIterations());
    }
    emit this->changed(this->modalSetup);
}

void ModalSetupWidget::onNModesExtractChanged(int nModes)
{
    this->modalSetup.setNModesToExtract(uint(nModes));
    emit this->changed(this->modalSetup);
}

void ModalSetupWidget::onConvergenceValueChanged(double convergenceValue)
{
    this->modalSetup.setConvergenceValue(convergenceValue);
    emit this->changed(this->modalSetup);
}
