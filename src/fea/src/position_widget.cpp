#include <cmath>
#include <float.h>

#include <QGridLayout>
#include <QLabel>

#include "position_widget.h"
#include "application.h"

#define MINIMUM_VALUE (-DBL_MAX*0.1)
#define MAXIMUM_VALUE (DBL_MAX*0.1)

PositionWidget::PositionWidget(const QString &title, const RR3Vector &position, bool horizontalLayout, QWidget *parent)
    : QGroupBox(title, parent)
    , horizontalLayout(horizontalLayout)
    , showButtonBox(true)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);

    this->xSlider = new QSlider(Qt::Horizontal);
    this->xLineEdit = new ValueLineEdit(MINIMUM_VALUE,MAXIMUM_VALUE);
    this->ySlider = new QSlider(Qt::Horizontal);
    this->yLineEdit = new ValueLineEdit(MINIMUM_VALUE,MAXIMUM_VALUE);
    this->zSlider = new QSlider(Qt::Horizontal);
    this->zLineEdit = new ValueLineEdit(MINIMUM_VALUE,MAXIMUM_VALUE);

    this->setPosition(position);

    this->pickButton = new QPushButton(tr("Set from picked element/node"));
    this->pickButton->setDisabled(Application::instance()->getSession()->getPickList().isEmpty());

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);

    this->cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonsLayout->addWidget(this->cancelButton);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setDisabled(true);
    buttonsLayout->addWidget(this->okButton);

    if (horizontalLayout)
    {
        layout->addWidget(this->xLineEdit, 0, 0, 1, 1);
        layout->addWidget(this->yLineEdit, 0, 1, 1, 1);
        layout->addWidget(this->zLineEdit, 0, 2, 1, 1);
        layout->addWidget(this->pickButton, 0, 3, 1, 1);
        layout->addWidget(new QWidget,1,0,1,4);
        layout->setRowStretch(1,1);
        this->pickButton->setText(tr("Use picked"));
    }
    else
    {
        QLabel *xLabel = new QLabel("X:");
        QLabel *xUnitsLabel = new QLabel("[m]");
        QLabel *yLabel = new QLabel("Y:");
        QLabel *yUnitsLabel = new QLabel("[m]");
        QLabel *zLabel = new QLabel("Z:");
        QLabel *zUnitsLabel = new QLabel("[m]");

        layout->addWidget(xLabel, 0, 0, 1, 1);
        layout->addWidget(this->xSlider, 0, 1, 1, 1);
        layout->addWidget(this->xLineEdit, 0, 2, 1, 1);
        layout->addWidget(xUnitsLabel, 0, 3, 1, 1);
        layout->addWidget(yLabel, 1, 0, 1, 1);
        layout->addWidget(this->ySlider, 1, 1, 1, 1);
        layout->addWidget(this->yLineEdit, 1, 2, 1, 1);
        layout->addWidget(yUnitsLabel, 1, 3, 1, 1);
        layout->addWidget(zLabel, 2, 0, 1, 1);
        layout->addWidget(this->zSlider, 2, 1, 1, 1);
        layout->addWidget(this->zLineEdit, 2, 2, 1, 1);
        layout->addWidget(zUnitsLabel, 2, 3, 1, 1);
        layout->addWidget(this->pickButton, 3, 0, 1, 4);
        layout->addLayout(buttonsLayout,4,0,1,4);

        layout->addWidget(new QWidget,5,0,1,4);
        layout->setRowStretch(5,1);
    }

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QObject::connect(this->xSlider,
                     &QSlider::valueChanged,
                     this,
                     &PositionWidget::onXSliderValueChanged);
    QObject::connect(this->ySlider,
                     &QSlider::valueChanged,
                     this,
                     &PositionWidget::onYSliderValueChanged);
    QObject::connect(this->zSlider,
                     &QSlider::valueChanged,
                     this,
                     &PositionWidget::onZSliderValueChanged);

    QObject::connect(this->xLineEdit,
                     &ValueLineEdit::valueChanged,
                     this,
                     &PositionWidget::onXLineEditValueChanged);
    QObject::connect(this->yLineEdit,
                     &ValueLineEdit::valueChanged,
                     this,
                     &PositionWidget::onYLineEditValueChanged);
    QObject::connect(this->zLineEdit,
                     &ValueLineEdit::valueChanged,
                     this,
                     &PositionWidget::onZLineEditValueChanged);

    QObject::connect(this->pickButton,
                     &QPushButton::clicked,
                     this,
                     &PositionWidget::onPickButtonClicked);

    QObject::connect(this->cancelButton,
                     &QPushButton::clicked,
                     this,
                     &PositionWidget::onCancelButtonClicked);
    QObject::connect(this->okButton,
                     &QPushButton::clicked,
                     this,
                     &PositionWidget::onOkButtonClicked);

    QObject::connect(Application::instance()->getSession(),
                     &Session::pickListChanged,
                     this,
                     &PositionWidget::onPickListChanged);
}

void PositionWidget::showSliders()
{
    this->xSlider->show();
    this->ySlider->show();
    this->zSlider->show();
}

void PositionWidget::hideSliders()
{
    this->xSlider->hide();
    this->ySlider->hide();
    this->zSlider->hide();
}

void PositionWidget::showPickButton()
{
    this->pickButton->show();
}

void PositionWidget::hidePickButton()
{
    this->pickButton->hide();
}

void PositionWidget::showButtons()
{
    this->showButtonBox = true;
    this->cancelButton->show();
    this->okButton->show();
}

void PositionWidget::hideButtons()
{
    this->showButtonBox = false;
    this->cancelButton->hide();
    this->okButton->hide();
}

void PositionWidget::setXRange(double xMin, double xMax)
{
    this->xLineEdit->setRange(xMin,xMax);
    this->setPosition(this->getPosition());
}

void PositionWidget::setYRange(double yMin, double yMax)
{
    this->yLineEdit->setRange(yMin,yMax);
    this->setPosition(this->getPosition());
}

void PositionWidget::setZRange(double zMin, double zMax)
{
    this->zLineEdit->setRange(zMin,zMax);
    this->setPosition(this->getPosition());
}

RR3Vector PositionWidget::getPosition() const
{
    RR3Vector position;

    position[0] = this->xLineEdit->getValue();
    position[1] = this->yLineEdit->getValue();
    position[2] = this->zLineEdit->getValue();

    return position;
}

void PositionWidget::setPosition(const RR3Vector &position)
{
    this->setSlidersPosition(position);

    this->xLineEdit->blockSignals(true);
    this->xLineEdit->setValue(position[0]);
    this->xLineEdit->blockSignals(false);

    this->yLineEdit->blockSignals(true);
    this->yLineEdit->setValue(position[1]);
    this->yLineEdit->blockSignals(false);

    this->zLineEdit->blockSignals(true);
    this->zLineEdit->setValue(position[2]);
    this->zLineEdit->blockSignals(false);

    if (!this->showButtonBox)
    {
        emit this->changed(position);
    }
}

void PositionWidget::setSlidersPosition(const RR3Vector &position)
{
    int xnsp = 0;
    int ynsp = 0;
    int znsp = 0;

    this->findSliderPositions(position,xnsp,ynsp,znsp);

    this->xSlider->blockSignals(true);
    this->xSlider->setValue(xnsp);
    this->xSlider->blockSignals(false);

    this->ySlider->blockSignals(true);
    this->ySlider->setValue(ynsp);
    this->ySlider->blockSignals(false);

    this->zSlider->blockSignals(true);
    this->zSlider->setValue(znsp);
    this->zSlider->blockSignals(false);

    if (this->horizontalLayout)
    {
        emit this->changed(position);
    }
}

void PositionWidget::findSliderPositions(const RR3Vector &position, int &x, int &y, int &z)
{
    int xMin = this->xSlider->minimum();
    int xMax = this->xSlider->maximum();
    int yMin = this->ySlider->minimum();
    int yMax = this->ySlider->maximum();
    int zMin = this->zSlider->minimum();
    int zMax = this->zSlider->maximum();

    double vxMin = this->xLineEdit->getMinimum();
    double vxMax = this->xLineEdit->getMaximum();
    double vyMin = this->yLineEdit->getMinimum();
    double vyMax = this->yLineEdit->getMaximum();
    double vzMin = this->zLineEdit->getMinimum();
    double vzMax = this->zLineEdit->getMaximum();

    double xRate = (position[0] - vxMin) / (vxMax - vxMin);
    double yRate = (position[1] - vyMin) / (vyMax - vyMin);
    double zRate = (position[2] - vzMin) / (vzMax - vzMin);

    x = qRound(xRate * double(xMax - xMin) + double(xMin));
    y = qRound(yRate * double(yMax - yMin) + double(yMin));
    z = qRound(zRate * double(zMax - zMin) + double(zMin));
}

void PositionWidget::onXSliderValueChanged(int value)
{
    int min = this->xSlider->minimum();
    int max = this->xSlider->maximum();
    double vMin = this->xLineEdit->getMinimum();
    double vMax = this->xLineEdit->getMaximum();

    double rate = double(value - min) / double(max - min);

    this->xLineEdit->blockSignals(true);
    this->xLineEdit->setValue(rate*(vMax-vMin)+vMin);
    this->xLineEdit->blockSignals(false);

    if (!this->showButtonBox)
    {
        emit this->changed(this->getPosition());
    }
    this->okButton->setEnabled(true);
}

void PositionWidget::onYSliderValueChanged(int value)
{
    int min = this->ySlider->minimum();
    int max = this->ySlider->maximum();
    double vMin = this->yLineEdit->getMinimum();
    double vMax = this->yLineEdit->getMaximum();

    double rate = double(value - min) / double(max - min);

    this->yLineEdit->blockSignals(true);
    this->yLineEdit->setValue(rate*(vMax-vMin)+vMin);
    this->yLineEdit->blockSignals(false);

    if (!this->showButtonBox)
    {
        emit this->changed(this->getPosition());
    }
    this->okButton->setEnabled(true);
}

void PositionWidget::onZSliderValueChanged(int value)
{
    int min = this->zSlider->minimum();
    int max = this->zSlider->maximum();
    double vMin = this->zLineEdit->getMinimum();
    double vMax = this->zLineEdit->getMaximum();

    double rate = double(value - min) / double(max - min);

    this->zLineEdit->blockSignals(true);
    this->zLineEdit->setValue(rate*(vMax-vMin)+vMin);
    this->zLineEdit->blockSignals(false);

    if (!this->showButtonBox)
    {
        emit this->changed(this->getPosition());
    }
    this->okButton->setEnabled(true);
}

void PositionWidget::onXLineEditValueChanged(double)
{
    this->setSlidersPosition(this->getPosition());
    this->okButton->setEnabled(true);
    emit this->changed(this->getPosition());
}

void PositionWidget::onYLineEditValueChanged(double)
{
    this->setSlidersPosition(this->getPosition());
    this->okButton->setEnabled(true);
    emit this->changed(this->getPosition());
}

void PositionWidget::onZLineEditValueChanged(double)
{
    this->setSlidersPosition(this->getPosition());
    this->okButton->setEnabled(true);
    emit this->changed(this->getPosition());
}

void PositionWidget::onPickButtonClicked()
{
    const PickList &pickList = Application::instance()->getSession()->getPickList();

    const QVector<PickItem> &pickItems = pickList.getItems();

    for (int i=0;i<pickItems.size();i++)
    {
        const SessionEntityID &pickEntityID = pickItems[i].getEntityID();

        if (pickItems[i].getItemType() == PICK_ITEM_NODE)
        {
            this->setPosition(Application::instance()->getSession()->getModel(pickEntityID.getMid()).getNode(pickItems[i].getNodeID()).toVector());
            this->okButton->setEnabled(true);

            return;
        }
        else if (pickItems[i].getItemType() == PICK_ITEM_ELEMENT)
        {
            double cx, cy, cz;
            const Model &rModel = Application::instance()->getSession()->getModel(pickEntityID.getMid());
            rModel.getElement(pickItems[i].getElementID()).findCenter(rModel.getNodes(),cx,cy,cz);
            this->setPosition(RR3Vector(cx, cy, cz));
            this->okButton->setEnabled(true);

            return;
        }
    }
}

void PositionWidget::onOkButtonClicked()
{
    emit this->changed(this->getPosition());
    emit this->closed();
}

void PositionWidget::onCancelButtonClicked()
{
    emit this->closed();
}

void PositionWidget::onPickListChanged()
{
    this->pickButton->setDisabled(Application::instance()->getSession()->getPickList().isEmpty());
}
