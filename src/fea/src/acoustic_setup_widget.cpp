#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

#include "acoustic_setup_widget.h"

AcousticSetupWidget::AcousticSetupWidget(const RAcousticSetup &acousticSetup, QWidget *parent)
    : QWidget(parent)
    , acousticSetup(acousticSetup)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Acoustic analysis setup"));
    mainLayout->addWidget(groupBox);

    QGridLayout *groupLayout = new QGridLayout;
    groupBox->setLayout(groupLayout);

    int groupLayoutRow = 0;

    // Analysis type
    QLabel *labelAnalysisType = new QLabel(tr("Analysis type"));
    groupLayout->addWidget(labelAnalysisType,groupLayoutRow,0);

    QComboBox *comboAnalysisType = new QComboBox();
    for (uint i=0;i<R_ACOUSTIC_ANALYSIS_N_TYPES;i++)
    {
        comboAnalysisType->addItem(RAcousticSetup::getAnalysisTypeName(RAcousticAnalysisType(i)));
    }
    comboAnalysisType->setCurrentIndex(this->acousticSetup.getAnalysisType());
    comboAnalysisType->setToolTip(tr("Transient analysis marches the wave equation in time. "
                                     "Harmonic analysis solves the Helmholtz equation for each frequency of the sweep."));
    groupLayout->addWidget(comboAnalysisType,groupLayoutRow++,1);

    this->connect(comboAnalysisType,SIGNAL(currentIndexChanged(int)),SLOT(onAnalysisTypeChanged(int)));

    // First frequency
    QLabel *labelFrequencyStart = new QLabel(tr("First frequency") + " [Hz]");
    groupLayout->addWidget(labelFrequencyStart,groupLayoutRow,0);

    this->lineFrequencyStart = new ValueLineEdit(R_ACOUSTIC_FREQUENCY_MIN_VALUE,R_ACOUSTIC_FREQUENCY_MAX_VALUE);
    this->lineFrequencyStart->setValue(this->acousticSetup.getFrequencyStart());
    this->lineFrequencyStart->setToolTip(tr("First frequency of the harmonic sweep."));
    groupLayout->addWidget(this->lineFrequencyStart,groupLayoutRow++,1);

    QObject::connect(this->lineFrequencyStart,&ValueLineEdit::valueChanged,this,&AcousticSetupWidget::onFrequencyStartChanged);

    // Frequency step
    QLabel *labelFrequencyStep = new QLabel(tr("Frequency step") + " [Hz]");
    groupLayout->addWidget(labelFrequencyStep,groupLayoutRow,0);

    this->lineFrequencyStep = new ValueLineEdit(R_ACOUSTIC_FREQUENCY_MIN_VALUE,R_ACOUSTIC_FREQUENCY_MAX_VALUE);
    this->lineFrequencyStep->setValue(this->acousticSetup.getFrequencyStep());
    this->lineFrequencyStep->setToolTip(tr("Frequency increment between two consecutive solves."));
    groupLayout->addWidget(this->lineFrequencyStep,groupLayoutRow++,1);

    QObject::connect(this->lineFrequencyStep,&ValueLineEdit::valueChanged,this,&AcousticSetupWidget::onFrequencyStepChanged);

    // Number of frequencies
    QLabel *labelNFrequencies = new QLabel(tr("Number of frequencies"));
    groupLayout->addWidget(labelNFrequencies,groupLayoutRow,0);

    this->spinNFrequencies = new QSpinBox;
    this->spinNFrequencies->setMinimum(R_ACOUSTIC_FREQUENCIES_MIN_N);
    this->spinNFrequencies->setMaximum(R_ACOUSTIC_FREQUENCIES_MAX_N);
    this->spinNFrequencies->setValue(int(this->acousticSetup.getNFrequencies()));
    this->spinNFrequencies->setToolTip(tr("Number of frequencies to solve. Each frequency produces its own result record."));
    groupLayout->addWidget(this->spinNFrequencies,groupLayoutRow++,1);

    this->connect(this->spinNFrequencies,SIGNAL(valueChanged(int)),SLOT(onNFrequenciesChanged(int)));

    // Frequency range summary
    this->labelFrequencyRange = new QLabel;
    groupLayout->addWidget(this->labelFrequencyRange,groupLayoutRow++,0,1,2);

    // Reference pressure
    QLabel *labelReferencePressure = new QLabel(tr("Reference pressure") + " [Pa]");
    groupLayout->addWidget(labelReferencePressure,groupLayoutRow,0);

    ValueLineEdit *lineReferencePressure = new ValueLineEdit(1.0e-99,1.0e99);
    lineReferencePressure->setValue(this->acousticSetup.getReferencePressure());
    lineReferencePressure->setToolTip(tr("Reference pressure used to evaluate the sound pressure level. "
                                         "The threshold of hearing in air is 20 uPa."));
    groupLayout->addWidget(lineReferencePressure,groupLayoutRow++,1);

    QObject::connect(lineReferencePressure,&ValueLineEdit::valueChanged,this,&AcousticSetupWidget::onReferencePressureChanged);

    this->updateHarmonicControls();
}

void AcousticSetupWidget::updateHarmonicControls()
{
    bool harmonic = (this->acousticSetup.getAnalysisType() == R_ACOUSTIC_ANALYSIS_HARMONIC);

    this->lineFrequencyStart->setEnabled(harmonic);
    this->lineFrequencyStep->setEnabled(harmonic);
    this->spinNFrequencies->setEnabled(harmonic);

    if (harmonic)
    {
        uint nFrequencies = std::max(this->acousticSetup.getNFrequencies(),uint(1));
        this->labelFrequencyRange->setText(tr("Sweep") + ": "
                                           + QString::number(this->acousticSetup.getFrequency(0)) + " - "
                                           + QString::number(this->acousticSetup.getFrequency(nFrequencies-1)) + " [Hz]");
    }
    else
    {
        this->labelFrequencyRange->setText(tr("Frequencies apply to a harmonic analysis only."));
    }
}

void AcousticSetupWidget::onAnalysisTypeChanged(int index)
{
    this->acousticSetup.setAnalysisType(RAcousticAnalysisType(index));
    this->updateHarmonicControls();
    emit this->changed(this->acousticSetup);
}

void AcousticSetupWidget::onFrequencyStartChanged(double frequencyStart)
{
    this->acousticSetup.setFrequencyStart(frequencyStart);
    this->updateHarmonicControls();
    emit this->changed(this->acousticSetup);
}

void AcousticSetupWidget::onFrequencyStepChanged(double frequencyStep)
{
    this->acousticSetup.setFrequencyStep(frequencyStep);
    this->updateHarmonicControls();
    emit this->changed(this->acousticSetup);
}

void AcousticSetupWidget::onNFrequenciesChanged(int nFrequencies)
{
    this->acousticSetup.setNFrequencies(uint(nFrequencies));
    this->updateHarmonicControls();
    emit this->changed(this->acousticSetup);
}

void AcousticSetupWidget::onReferencePressureChanged(double referencePressure)
{
    this->acousticSetup.setReferencePressure(referencePressure);
    emit this->changed(this->acousticSetup);
}
