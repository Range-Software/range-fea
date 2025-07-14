#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>

#include <rbl_utils.h>

#include "value_scale.h"

const double ValueScale::mantisScale = 10.0;
const uint ValueScale::mantisSize = 100;
const uint ValueScale::exponentSize = 100;

ValueScale::ValueScale(const QString &title, QWidget *parent)
    : QWidget(parent)
    , expandable(false)
    , mantis(1.0)
    , exponent(1.0)
    , setScaleAllowed(true)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    this->setLayout(mainLayout);

    QGroupBox *scaleGroupBox= new QGroupBox(title);
    mainLayout->addWidget(scaleGroupBox);

    QVBoxLayout *scaleLayout = new QVBoxLayout;
    scaleGroupBox->setLayout(scaleLayout);

    QHBoxLayout *editLayout = new QHBoxLayout;
    editLayout->setContentsMargins(0,0,0,0);
    scaleLayout->addLayout(editLayout);

    this->scaleEdit = new ValueLineEdit(-ValueScale::mantisScale*std::pow(10,ValueScale::exponentSize),ValueScale::mantisScale*std::pow(10,ValueScale::exponentSize));
    this->scaleEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    editLayout->addWidget(this->scaleEdit);

    this->expandButton = new QPushButton(tr("more"));
    this->expandButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    editLayout->addWidget(this->expandButton);
    if (this->expandable)
    {
        this->expandButton->show();
    }
    else
    {
        this->expandButton->hide();
    }

    this->expandWidget = new QWidget;
    scaleLayout->addWidget(expandWidget);

    QGridLayout *expandLayout = new QGridLayout;
    expandLayout->setContentsMargins(0,0,0,0);
    expandWidget->setLayout(expandLayout);

    QLabel *mantisLabel = new QLabel(tr("Scale") + ":");
    mantisLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    expandLayout->addWidget(mantisLabel,0,0,1,1);

    this->mantisSlider = new QSlider(Qt::Horizontal);
    this->mantisSlider->setRange(-100,100);
    this->mantisSlider->setPageStep(10);
    this->mantisSlider->setSingleStep(1);
    this->mantisSlider->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);
    expandLayout->addWidget(this->mantisSlider,0,1,1,2);

    this->mantisEdit = new ValueLineEdit(-ValueScale::mantisScale,ValueScale::mantisScale);
    this->mantisEdit->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    expandLayout->addWidget(this->mantisEdit,0,3,1,1);

    QLabel *exponentLabel = new QLabel(tr("Factor") + ":");
    exponentLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    expandLayout->addWidget(exponentLabel,1,0,1,1);

    QLabel *scaleExponentMantisLabel = new QLabel("1.0e");
    scaleExponentMantisLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    scaleExponentMantisLabel->setAlignment(Qt::AlignRight);
    expandLayout->addWidget(scaleExponentMantisLabel,1,1,1,1);

    this->exponentSpinBox = new QSpinBox;
    this->exponentSpinBox->setRange(-int(ValueScale::exponentSize),int(ValueScale::exponentSize));
    this->exponentSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    expandLayout->addWidget(this->exponentSpinBox,1,2,1,2);

    if (this->expandWidget->isHidden())
    {
        this->expandButton->setText(tr("more"));
    }
    else
    {
        this->expandButton->setText(tr("less"));
    }

    QObject::connect(this->scaleEdit,&ValueLineEdit::valueChanged,this,&ValueScale::onScaleEditChanged);

    QObject::connect(this->expandButton,&QPushButton::clicked,this,&ValueScale::onExpandButtonClicked);

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

    emit this->valueChanged(value);
}

void ValueScale::onExpandButtonClicked()
{
    if (this->expandWidget->isHidden())
    {
        this->expandWidget->show();
        this->expandButton->setText(tr("less"));
    }
    else
    {
        this->expandWidget->hide();
        this->expandButton->setText(tr("more"));
    }
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

    emit this->valueChanged(scale);
}
