#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QColorDialog>

#include "color.h"
#include "color_combo_box.h"

ColorComboBox::ColorComboBox(QWidget *parent) :
    QComboBox (parent),
    whFactor(3)
{
    this->addItem("Custom Color..");
    QList<Qt::GlobalColor> colorList = Color::getGlobalColors();
    for (int i=0;i<colorList.size();i++)
    {
        this->addColor(QColor(colorList[i]));
    }

    this->setCurrentIndex(1);
    QSize iSize = this->iconSize();
    iSize.rwidth() *= this->whFactor;
    this->setIconSize(iSize);

    this->connect(this,SIGNAL(currentIndexChanged(int)),SLOT(onCurrentIndexChanged(int)));
}

QColor ColorComboBox::getColor()
{
    int index = this->currentIndex();
    if (index > 0 && index < this->count())
    {
        return this->itemData(index).value<QColor>();
    }

    return Color::random(true);
}

void ColorComboBox::setColor(const QColor &color)
{
    this->setColorName(color.name());
}

QString ColorComboBox::getColorName()
{
    return this->getColor().name();
}

void ColorComboBox::setColorName(const QString &colorName)
{
    if (!QColor(colorName).isValid())
    {
        return;
    }

    for (int i=1;i<this->count();i++)
    {
        if (this->itemText(i) == colorName)
        {
            this->setCurrentIndex(i);
            return;
        }
    }
    this->addColor(colorName);
}

QIcon ColorComboBox::createIcon(const QString &colorName)
{
    QPixmap iconPixmap(32*this->whFactor,32);
    iconPixmap.fill(QColor(colorName));
    QIcon itemIcon(iconPixmap);
    return itemIcon;
}

void ColorComboBox::addColor(const QColor &color)
{
    this->addColor(color.name());
}

void ColorComboBox::addColor(const QString &colorName)
{
    if (!QColor(colorName).isValid())
    {
        return;
    }

    this->addItem(this->createIcon(colorName),colorName,QColor(colorName));
    this->setCurrentIndex(this->count()-1);
}

void ColorComboBox::onCurrentIndexChanged(int index)
{
    if (index > 0 && index < this->count())
    {
        emit this->colorChanged(this->itemData(index).value<QColor>());
        emit this->colorChanged(this->itemData(index).value<QColor>(),this->id);
        this->color = this->itemData(index).value<QColor>();
        return;
    }
    if (index == 0)
    {
        // Select custom color.
        QColor color = QColorDialog::getColor(this->color,this->parentWidget(),"Choose entity color",QColorDialog::ShowAlphaChannel);
        if (color.isValid())
        {
            this->setColor(color);
        }
    }
}
