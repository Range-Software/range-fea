#include <cmath>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSignalBlocker>

#include <rbl_utils.h>

#include "value_scale.h"

const double ValueScale::mantisScale = 10.0;
const uint ValueScale::mantisSize = 100;
const uint ValueScale::exponentSize = 100;

//! Range of logarithmic slider in powers of ten.
static constexpr int logSliderMinPower = -3;
static constexpr int logSliderMaxPower = 6;
//! Number of logarithmic slider steps in one power of ten.
static constexpr int logSliderStepsPerPower = 100;

ValueScale::ValueScale(const QString &title, QWidget *parent)
    : QWidget(parent)
    , mantis(1.0)
    , exponent(1.0)
    , setScaleAllowed(true)
    , logSliderChanging(false)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    this->setLayout(mainLayout);

    QHBoxLayout *editLayout = new QHBoxLayout;
    editLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(editLayout);

    this->titleLabel = new QLabel(title);
    this->titleLabel->setVisible(!title.isEmpty());
    editLayout->addWidget(this->titleLabel);

    this->scaleEdit = new ValueLineEdit(-ValueScale::mantisScale*std::pow(10,ValueScale::exponentSize),ValueScale::mantisScale*std::pow(10,ValueScale::exponentSize));
    this->scaleEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    editLayout->addWidget(this->scaleEdit,1);

    this->unitsLabel = new QLabel;
    this->unitsLabel->hide();
    editLayout->addWidget(this->unitsLabel);

    this->logSlider = new QSlider(Qt::Horizontal);
    this->logSlider->setRange(logSliderMinPower*logSliderStepsPerPower,logSliderMaxPower*logSliderStepsPerPower);
    this->logSlider->setPageStep(logSliderStepsPerPower);
    this->logSlider->setSingleStep(10);
    this->logSlider->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);
    this->logSlider->hide();
    editLayout->addWidget(this->logSlider,2);

    this->fineAdjustWidget = new QWidget;
    this->fineAdjustWidget->hide();
    mainLayout->addWidget(this->fineAdjustWidget);

    QGridLayout *fineAdjustLayout = new QGridLayout;
    fineAdjustLayout->setContentsMargins(0,0,0,0);
    this->fineAdjustWidget->setLayout(fineAdjustLayout);

    QLabel *mantisLabel = new QLabel(tr("Value") + ":");
    mantisLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    fineAdjustLayout->addWidget(mantisLabel,0,0,1,1);

    this->mantisSlider = new QSlider(Qt::Horizontal);
    this->mantisSlider->setRange(-100,100);
    this->mantisSlider->setPageStep(10);
    this->mantisSlider->setSingleStep(1);
    this->mantisSlider->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);
    fineAdjustLayout->addWidget(this->mantisSlider,0,1,1,2);

    this->mantisEdit = new ValueLineEdit(-ValueScale::mantisScale,ValueScale::mantisScale);
    this->mantisEdit->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    fineAdjustLayout->addWidget(this->mantisEdit,0,3,1,1);

    QLabel *exponentLabel = new QLabel(tr("Factor") + ":");
    exponentLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    fineAdjustLayout->addWidget(exponentLabel,1,0,1,1);

    QLabel *scaleExponentMantisLabel = new QLabel("1.0e");
    scaleExponentMantisLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    scaleExponentMantisLabel->setAlignment(Qt::AlignRight);
    fineAdjustLayout->addWidget(scaleExponentMantisLabel,1,1,1,1);

    this->exponentSpinBox = new QSpinBox;
    this->exponentSpinBox->setRange(-int(ValueScale::exponentSize),int(ValueScale::exponentSize));
    this->exponentSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    fineAdjustLayout->addWidget(this->exponentSpinBox,1,2,1,2);

    QObject::connect(this->scaleEdit,&ValueLineEdit::valueChanged,this,&ValueScale::onScaleEditChanged);

    QObject::connect(this->logSlider,&QSlider::valueChanged,this,&ValueScale::onLogSliderChanged);

    QObject::connect(this->mantisSlider,&QSlider::valueChanged,this,&ValueScale::onMantisSliderChanged);
    QObject::connect(this->mantisEdit,&ValueLineEdit::valueChanged,this,&ValueScale::onMantisEditChanged);

    this->connect(this->exponentSpinBox,SIGNAL(valueChanged(int)),SLOT(onExponentSpinBoxChanged(int)));

    this->setValue(1.0);
}

double ValueScale::getValue() const
{
    return this->mantis*std::pow(10.0,this->exponent);
}

void ValueScale::setValue(double value)
{
    this->mantis = RUtil::frexp10(value,&this->exponent);
    this->mantis *= 10;
    this->exponent -= 1;
    this->mantisEdit->setValue(this->mantis);
    this->exponentSpinBox->setValue(this->exponent);
    this->scaleEdit->setValue(value);
    this->syncLogSlider(value);
}

void ValueScale::setUnits(const QString &units)
{
    this->unitsLabel->setText(units);
    this->unitsLabel->setVisible(!units.isEmpty());
}

void ValueScale::setLogSliderVisible(bool visible)
{
    this->logSlider->setVisible(visible);
}

void ValueScale::setFineAdjustVisible(bool visible)
{
    this->fineAdjustWidget->setVisible(visible);
    this->layout()->invalidate();
    this->updateGeometry();
}

int ValueScale::findTitleWidth() const
{
    return this->titleLabel->sizeHint().width();
}

void ValueScale::setTitleWidth(int width)
{
    this->titleLabel->setFixedWidth(width);
}

void ValueScale::syncLogSlider(double value)
{
    if (this->logSliderChanging)
    {
        return;
    }

    const QSignalBlocker blocker(this->logSlider);

    if (value > 0.0)
    {
        this->logSlider->setValue(int(std::lround(std::log10(value)*logSliderStepsPerPower)));
    }
    else
    {
        this->logSlider->setValue(this->logSlider->minimum());
    }
}

void ValueScale::onScaleEditChanged(double value)
{
    this->setScaleAllowed = false;

    this->mantis = RUtil::frexp10(value,&this->exponent);
    this->mantis *= 10;
    this->exponent -= 1;
    this->blockSignals(true);
    this->mantisEdit->setValue(this->mantis);
    this->exponentSpinBox->setValue(this->exponent);
    this->blockSignals(false);

    this->setScaleAllowed = true;

    this->syncLogSlider(value);

    emit this->valueChanged(value);
}

void ValueScale::onLogSliderChanged(int position)
{
    // Two significant digits give values which are easy to read and enter again.
    double value = std::pow(10.0,double(position)/logSliderStepsPerPower);
    const double magnitude = std::pow(10.0,std::floor(std::log10(value))-1.0);
    value = std::round(value/magnitude)*magnitude;

    this->logSliderChanging = true;
    {
        const QSignalBlocker blocker(this);
        this->setValue(value);
    }
    this->logSliderChanging = false;

    emit this->valueChanged(value);
}

void ValueScale::onMantisSliderChanged(int value)
{
    this->mantis = ValueScale::mantisScale * value / double(ValueScale::mantisSize);
    double scale = this->getValue();
    this->blockSignals(true);
    if (this->setScaleAllowed)
    {
        this->scaleEdit->blockSignals(true);
        this->scaleEdit->setValue(scale);
        this->scaleEdit->blockSignals(false);
    }
    this->mantisEdit->setValue(this->mantis);
    this->blockSignals(false);

    this->syncLogSlider(scale);

    emit this->valueChanged(scale);
}

void ValueScale::onMantisEditChanged(double value)
{
    this->mantis = value;
    double scale = this->getValue();
    int mantisPositon = ceil(ValueScale::mantisSize * value / ValueScale::mantisScale);
    this->blockSignals(true);
    if (this->setScaleAllowed)
    {
        this->scaleEdit->blockSignals(true);
        this->scaleEdit->setValue(scale);
        this->scaleEdit->blockSignals(false);
    }
    this->mantisSlider->setValue(mantisPositon);
    this->blockSignals(false);

    this->syncLogSlider(scale);

    emit this->valueChanged(scale);
}

void ValueScale::onExponentSpinBoxChanged(int value)
{
    this->exponent = value;
    double scale = this->getValue();
    this->blockSignals(true);
    if (this->setScaleAllowed)
    {
        this->scaleEdit->blockSignals(true);
        this->scaleEdit->setValue(scale);
        this->scaleEdit->blockSignals(false);
    }
    this->blockSignals(false);

    this->syncLogSlider(scale);

    emit this->valueChanged(scale);
}
