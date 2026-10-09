#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include "particle_setup_widget.h"
#include "value_line_edit.h"

ParticleSetupWidget::ParticleSetupWidget(const RParticleSetup &particleSetup, QWidget *parent)
    : QWidget(parent)
    , particleSetup(particleSetup)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Contaminant dispersion setup"));
    mainLayout->addWidget(groupBox);

    QGridLayout *groupLayout = new QGridLayout;
    groupBox->setLayout(groupLayout);

    int groupLayoutRow = 0;

    // Maximum saturation
    QLabel *labelMaximumSaturation = new QLabel(tr("Maximum saturation") + " [kg/m^3]");
    groupLayout->addWidget(labelMaximumSaturation,groupLayoutRow,0);

    ValueLineEdit *lineMaximumSaturation = new ValueLineEdit(R_PARTICLE_MAXIMUM_SATURATION_MIN_VALUE,R_PARTICLE_MAXIMUM_SATURATION_MAX_VALUE);
    lineMaximumSaturation->setValue(this->particleSetup.getMaximumSaturation());
    lineMaximumSaturation->setToolTip(tr("Maximum possible concentration of the contaminant. "
                                         "Particle rate decreases as the concentration approaches this value and stops at saturation. "
                                         "Relative saturation is computed when set. "
                                         "Zero means unlimited."));
    groupLayout->addWidget(lineMaximumSaturation,groupLayoutRow++,1);

    QObject::connect(lineMaximumSaturation,&ValueLineEdit::valueChanged,this,&ParticleSetupWidget::onMaximumSaturationChanged);

    // Diffusion coefficient
    QLabel *labelDiffusionCoefficient = new QLabel(tr("Diffusion coefficient") + " [m^2/s]");
    groupLayout->addWidget(labelDiffusionCoefficient,groupLayoutRow,0);

    ValueLineEdit *lineDiffusionCoefficient = new ValueLineEdit(R_PARTICLE_DIFFUSION_COEFFICIENT_MIN_VALUE,R_PARTICLE_DIFFUSION_COEFFICIENT_MAX_VALUE);
    lineDiffusionCoefficient->setValue(this->particleSetup.getDiffusionCoefficient());
    lineDiffusionCoefficient->setToolTip(tr("Effective diffusion coefficient of the contaminant in the fluid. "
                                            "Molecular diffusion (about 1e-5 m^2/s in gases, 1e-9 m^2/s in liquids) is negligible in most flows; "
                                            "mixing is dominated by turbulence, which can be estimated as turbulent viscosity divided by 0.7. "
                                            "When diffusion dominates, use backward difference time march approximation to avoid oscillations. "
                                            "Zero means no diffusion."));
    groupLayout->addWidget(lineDiffusionCoefficient,groupLayoutRow++,1);

    QObject::connect(lineDiffusionCoefficient,&ValueLineEdit::valueChanged,this,&ParticleSetupWidget::onDiffusionCoefficientChanged);
}

void ParticleSetupWidget::onMaximumSaturationChanged(double maximumSaturation)
{
    this->particleSetup.setMaximumSaturation(maximumSaturation);
    emit this->changed(this->particleSetup);
}

void ParticleSetupWidget::onDiffusionCoefficientChanged(double diffusionCoefficient)
{
    this->particleSetup.setDiffusionCoefficient(diffusionCoefficient);
    emit this->changed(this->particleSetup);
}
