#include <QGridLayout>

#include <rbl_logger.h>

#include "local_direction_widget.h"

LocalDirectionWidget::LocalDirectionWidget(const QString &title, const RLocalDirection &localDirection, QWidget *parent)
    : QGroupBox(title, parent)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);

    this->positionWidget = new PositionWidget(tr("Position"),localDirection.getPosition(),false);
    this->positionWidget->hideSliders();
    this->positionWidget->hideButtons();
    layout->addWidget(this->positionWidget,0,0,1,1);

    this->directionWidget = new DirectionWidget(tr("Direction"),localDirection.getDirection());
    this->directionWidget->hideButtons();
    layout->addWidget(this->directionWidget,1,0,1,1);

    this->positionWidget->blockSignals(true);
    this->directionWidget->blockSignals(true);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);

    this->cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonsLayout->addWidget(this->cancelButton);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setDisabled(true);
    buttonsLayout->addWidget(this->okButton);

    layout->addLayout(buttonsLayout,2,0,1,1);

    layout->addWidget(new QWidget,3,0,1,4);
    layout->setRowStretch(3,1);

    QObject::connect(this->positionWidget,
                     &PositionWidget::changed,
                     this,
                     &LocalDirectionWidget::onPositionChanged);

    QObject::connect(this->directionWidget,
                     &DirectionWidget::changed,
                     this,
                     &LocalDirectionWidget::onDirectionChanged);

    QObject::connect(this->cancelButton,
                     &QPushButton::clicked,
                     this,
                     &LocalDirectionWidget::onCancelButtonClicked);
    QObject::connect(this->okButton,
                     &QPushButton::clicked,
                     this,
                     &LocalDirectionWidget::onOkButtonClicked);
}

RLocalDirection LocalDirectionWidget::getLocalDirection() const
{
    return RLocalDirection(this->positionWidget->getPosition(),this->directionWidget->getDirection());
}

void LocalDirectionWidget::setLocalDirection(const RLocalDirection &localDirection)
{
    this->positionWidget->setPosition(localDirection.getPosition());
    this->directionWidget->setDirection(localDirection.getDirection());

    emit this->changed(localDirection);
}

void LocalDirectionWidget::showEvent(QShowEvent *event)
{
    this->QGroupBox::showEvent(event);
    this->positionWidget->blockSignals(false);
    this->directionWidget->blockSignals(false);
}

void LocalDirectionWidget::onPositionChanged(const RR3Vector &)
{
    this->okButton->setEnabled(true);
    emit this->changed(this->getLocalDirection());
}

void LocalDirectionWidget::onDirectionChanged(const RR3Vector &)
{
    this->okButton->setEnabled(true);
    emit this->changed(this->getLocalDirection());
}

void LocalDirectionWidget::onCancelButtonClicked()
{
    emit this->closed();
}

void LocalDirectionWidget::onOkButtonClicked()
{
    emit this->changed(this->getLocalDirection());
    emit this->closed();
}
