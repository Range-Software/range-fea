#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include "stress_setup_widget.h"

StressSetupWidget::StressSetupWidget(const RStressSetup &stressSetup, QWidget *parent)
    : QWidget(parent)
    , stressSetup(stressSetup)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Transient stress analysis setup"));
    mainLayout->addWidget(groupBox);

    QGridLayout *groupLayout = new QGridLayout;
    groupBox->setLayout(groupLayout);

    int groupLayoutRow = 0;

    // Transient type
    QLabel *labelTransientType = new QLabel(tr("Analysis type"));
    groupLayout->addWidget(labelTransientType,groupLayoutRow,0);

    QComboBox *comboTransientType = new QComboBox();
    for (uint i=0;i<R_STRESS_TRANSIENT_N_TYPES;i++)
    {
        comboTransientType->addItem(RStressSetup::getTransientTypeName(RStressTransientType(i)));
    }
    comboTransientType->setCurrentIndex(this->stressSetup.getTransientType());
    comboTransientType->setToolTip(tr("Dynamic analysis solves M*u'' + C*u' + K*u = f(t) and captures inertia, vibrations and damping. "
                                      "Quasi-static analysis solves K*u = f(t) at every time step and neglects inertia."));
    groupLayout->addWidget(comboTransientType,groupLayoutRow++,1);

    this->connect(comboTransientType,SIGNAL(currentIndexChanged(int)),SLOT(onTransientTypeChanged(int)));

    // HHT alpha
    QLabel *labelHhtAlpha = new QLabel(tr("Numerical damping (HHT alpha)"));
    groupLayout->addWidget(labelHhtAlpha,groupLayoutRow,0);

    this->lineHhtAlpha = new ValueLineEdit(R_STRESS_HHT_ALPHA_MIN_VALUE,R_STRESS_HHT_ALPHA_MAX_VALUE);
    this->lineHhtAlpha->setValue(this->stressSetup.getHhtAlpha());
    this->lineHhtAlpha->setToolTip(tr("Hilber-Hughes-Taylor alpha from -1/3 to 0. "
                                      "Zero is the Newmark average acceleration scheme without numerical damping. "
                                      "Negative values damp spurious high frequency oscillations while keeping the low modes accurate; "
                                      "-0.05 to -0.1 is a common choice."));
    groupLayout->addWidget(this->lineHhtAlpha,groupLayoutRow++,1);

    QObject::connect(this->lineHhtAlpha,&ValueLineEdit::valueChanged,this,&StressSetupWidget::onHhtAlphaChanged);

    // Lumped mass
    this->checkLumpedMass = new QCheckBox(tr("Lumped mass matrix"));
    this->checkLumpedMass->setChecked(this->stressSetup.getLumpedMass());
    this->checkLumpedMass->setToolTip(tr("Replace the consistent mass matrix with a diagonal one of the same total mass. "
                                         "Lumped mass underestimates natural frequencies slightly and reduces oscillations behind sharp wave fronts."));
    groupLayout->addWidget(this->checkLumpedMass,groupLayoutRow++,0,1,2);

    QObject::connect(this->checkLumpedMass,&QCheckBox::checkStateChanged,this,&StressSetupWidget::onLumpedMassChanged);

    this->updateEnabled();
}

void StressSetupWidget::updateEnabled()
{
    bool dynamic = (this->stressSetup.getTransientType() == R_STRESS_TRANSIENT_DYNAMIC);
    this->lineHhtAlpha->setEnabled(dynamic);
    this->checkLumpedMass->setEnabled(dynamic);
}

void StressSetupWidget::onTransientTypeChanged(int index)
{
    this->stressSetup.setTransientType(RStressTransientType(index));
    this->updateEnabled();
    emit this->changed(this->stressSetup);
}

void StressSetupWidget::onHhtAlphaChanged(double hhtAlpha)
{
    this->stressSetup.setHhtAlpha(hhtAlpha);
    emit this->changed(this->stressSetup);
}

void StressSetupWidget::onLumpedMassChanged(Qt::CheckState state)
{
    this->stressSetup.setLumpedMass(state == Qt::Checked);
    emit this->changed(this->stressSetup);
}
