#include <cmath>

#include <QGridLayout>
#include <QLabel>

#include "direction_widget.h"


DirectionWidget::DirectionWidget(const QString &title, const RR3Vector &direction, QWidget *parent)
    : QGroupBox(title, parent)
    , showButtonBox(true)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);

    QLabel *xLabel = new QLabel("X:");
    layout->addWidget(xLabel, 0, 0, 1, 1);

    this->xSlider = new QSlider(Qt::Horizontal);
    layout->addWidget(this->xSlider, 0, 1, 1, 1);

    this->xSpin = new QDoubleSpinBox;
    this->xSpin->setRange(-1.0,1.0);
    this->xSpin->setSingleStep(0.01);
    layout->addWidget(this->xSpin, 0, 2, 1, 1);

    QLabel *yLabel = new QLabel("Y:");
    layout->addWidget(yLabel, 1, 0, 1, 1);

    this->ySlider = new QSlider(Qt::Horizontal);
    layout->addWidget(this->ySlider, 1, 1, 1, 1);

    this->ySpin = new QDoubleSpinBox;
    this->ySpin->setRange(-1.0,1.0);
    this->ySpin->setSingleStep(0.01);
    layout->addWidget(this->ySpin, 1, 2, 1, 1);

    QLabel *zLabel = new QLabel("Z:");
    layout->addWidget(zLabel, 2, 0, 1, 1);

    this->zSlider = new QSlider(Qt::Horizontal);
    layout->addWidget(this->zSlider, 2, 1, 1, 1);

    this->zSpin = new QDoubleSpinBox;
    this->zSpin->setRange(-1.0,1.0);
    this->zSpin->setSingleStep(0.01);
    layout->addWidget(this->zSpin, 2, 2, 1, 1);

    this->setDirection(direction);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    layout->addLayout(buttonsLayout,3,0,1,3);

    this->cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonsLayout->addWidget(this->cancelButton);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setDisabled(true);
    buttonsLayout->addWidget(this->okButton);

    layout->addWidget(new QWidget,4,0,1,3);
    layout->setRowStretch(4,1);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QObject::connect(this->xSlider,&QSlider::valueChanged,this,&DirectionWidget::onXSliderValueChanged);
    QObject::connect(this->ySlider,&QSlider::valueChanged,this,&DirectionWidget::onYSliderValueChanged);
    QObject::connect(this->zSlider,&QSlider::valueChanged,this,&DirectionWidget::onZSliderValueChanged);

    this->connect(this->xSpin,SIGNAL(valueChanged(double)),SLOT(onXSpinValueChanged(double)));
    this->connect(this->ySpin,SIGNAL(valueChanged(double)),SLOT(onYSpinValueChanged(double)));
    this->connect(this->zSpin,SIGNAL(valueChanged(double)),SLOT(onZSpinValueChanged(double)));

    QObject::connect(this->cancelButton,
                     &QPushButton::clicked,
                     this,
                     &DirectionWidget::onCancelButtonClicked);
    QObject::connect(this->okButton,
                     &QPushButton::clicked,
                     this,
                     &DirectionWidget::onOkButtonClicked);
}

void DirectionWidget::showButtons()
{
    this->showButtonBox = true;
    this->okButton->show();
    this->cancelButton->show();
}

void DirectionWidget::hideButtons()
{
    this->showButtonBox = false;
    this->okButton->hide();
    this->cancelButton->hide();
}

RR3Vector DirectionWidget::getDirection() const
{
    RR3Vector direction;

    direction[0] = this->xSpin->value();
    direction[1] = this->ySpin->value();
    direction[2] = this->zSpin->value();

    return direction;
}

void DirectionWidget::setDirection(const RR3Vector &direction)
{
    int xnsp = 0;
    int ynsp = 0;
    int znsp = 0;

    this->findSliderPositions(direction,xnsp,ynsp,znsp);

    this->xSlider->blockSignals(true);
    this->xSlider->setValue(xnsp);
    this->xSlider->blockSignals(false);

    this->ySlider->blockSignals(true);
    this->ySlider->setValue(ynsp);
    this->ySlider->blockSignals(false);

    this->zSlider->blockSignals(true);
    this->zSlider->setValue(znsp);
    this->zSlider->blockSignals(false);

    this->xSpin->blockSignals(true);
    this->xSpin->setValue(direction[0]);
    this->xSpin->blockSignals(false);

    this->ySpin->blockSignals(true);
    this->ySpin->setValue(direction[1]);
    this->ySpin->blockSignals(false);

    this->zSpin->blockSignals(true);
    this->zSpin->setValue(direction[2]);
    this->zSpin->blockSignals(false);

    if (!this->showButtonBox)
    {
        emit this->changed(direction);
    }
}

void DirectionWidget::normalize(RR3Vector &direction, int keepPosition)
{
    double x = direction[0];
    double y = direction[1];
    double z = direction[2];

    if (keepPosition == 0)
    {
        double yz2 = 1-direction[0]*direction[0];
        double y2 = direction[1]*direction[1];

        if (yz2 < y2)
        {
            y2 = yz2;
            bool negative = y < 0.0;
            y = std::sqrt(y2);
            if (negative) y *= -1;
        }
        z = std::sqrt(yz2 - y2);
    }
    else if (keepPosition == 1)
    {
        double xz2 = 1-direction[1]*direction[1];
        double x2 = direction[0]*direction[0];

        if (xz2 < x2)
        {
            x2 = xz2;
            bool negative = x < 0.0;
            x = std::sqrt(x2);
            if (negative) x *= -1;
        }
        z = std::sqrt(xz2 - x2);
    }
    else if (keepPosition == 2)
    {
        double xy2 = 1-direction[2]*direction[2];
        double x2 = direction[0]*direction[0];

        if (xy2 < x2)
        {
            x2 = xy2;
            bool negative = x < 0.0;
            x = std::sqrt(x2);
            if (negative) x *= -1;
        }
        y = std::sqrt(xy2 - x2);
    }

    direction[0] = x;
    direction[1] = y;
    direction[2] = z;
}

void DirectionWidget::findSliderPositions(const RR3Vector &direction, int &x, int &y, int &z)
{
    int xMin = this->xSlider->minimum();
    int xMax = this->xSlider->maximum();
    int yMin = this->ySlider->minimum();
    int yMax = this->ySlider->maximum();
    int zMin = this->zSlider->minimum();
    int zMax = this->zSlider->maximum();

    double vxMin = this->xSpin->minimum();
    double vxMax = this->xSpin->maximum();
    double vyMin = this->ySpin->minimum();
    double vyMax = this->ySpin->maximum();
    double vzMin = this->zSpin->minimum();
    double vzMax = this->zSpin->maximum();

    double xRate = (direction[0] - vxMin) / (vxMax - vxMin);
    double yRate = (direction[1] - vyMin) / (vyMax - vyMin);
    double zRate = (direction[2] - vzMin) / (vzMax - vzMin);

    x = qRound(xRate * double(xMax - xMin) + double(xMin));
    y = qRound(yRate * double(yMax - yMin) + double(yMin));
    z = qRound(zRate * double(zMax - zMin) + double(zMin));
}

void DirectionWidget::onXSliderValueChanged(int value)
{
    int min = this->xSlider->minimum();
    int max = this->xSlider->maximum();
    double vMin = this->xSpin->minimum();
    double vMax = this->xSpin->maximum();

    double rate = double(value - min) / double(max - min);

    this->xSpin->blockSignals(true);
    this->xSpin->setValue(rate*(vMax-vMin)+vMin);
    this->xSpin->blockSignals(false);

    RR3Vector direction = this->getDirection();
    this->normalize(direction,0);
    this->setDirection(direction);

    this->okButton->setEnabled(true);
}

void DirectionWidget::onYSliderValueChanged(int value)
{
    int min = this->ySlider->minimum();
    int max = this->ySlider->maximum();
    double vMin = this->ySpin->minimum();
    double vMax = this->ySpin->maximum();

    double rate = double(value - min) / double(max - min);

    this->ySpin->blockSignals(true);
    this->ySpin->setValue(rate*(vMax-vMin)+vMin);
    this->ySpin->blockSignals(false);

    RR3Vector direction = this->getDirection();
    this->normalize(direction,1);
    this->setDirection(direction);

    this->okButton->setEnabled(true);
}

void DirectionWidget::onZSliderValueChanged(int value)
{
    int min = this->zSlider->minimum();
    int max = this->zSlider->maximum();
    double vMin = this->zSpin->minimum();
    double vMax = this->zSpin->maximum();

    double rate = double(value - min) / double(max - min);

    this->zSpin->blockSignals(true);
    this->zSpin->setValue(rate*(vMax-vMin)+vMin);
    this->zSpin->blockSignals(false);

    RR3Vector direction = this->getDirection();
    this->normalize(direction,2);
    this->setDirection(direction);

    this->okButton->setEnabled(true);
}

void DirectionWidget::onXSpinValueChanged(double)
{
    RR3Vector direction = this->getDirection();
    this->normalize(direction,0);
    this->setDirection(direction);

    this->okButton->setEnabled(true);
}

void DirectionWidget::onYSpinValueChanged(double)
{
    RR3Vector direction = this->getDirection();
    this->normalize(direction,1);
    this->setDirection(direction);

    this->okButton->setEnabled(true);
}

void DirectionWidget::onZSpinValueChanged(double)
{
    RR3Vector direction = this->getDirection();
    this->normalize(direction,2);
    this->setDirection(direction);

    this->okButton->setEnabled(true);
}

void DirectionWidget::onOkButtonClicked()
{
    emit this->changed(this->getDirection());
    emit this->closed();
}

void DirectionWidget::onCancelButtonClicked()
{
    emit this->closed();
}
