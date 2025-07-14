#include "color_scale.h"
#include "color_scale_combo_box.h"

ColorScaleComboBox::ColorScaleComboBox(QWidget *parent) :
    QComboBox(parent),
    whFactor(10)
{
    this->addItem("None");

    static QRegularExpression re("[-.]");

    QList<QString> colorScales = ColorScale::getColorScales();
    for (int i=0;i<colorScales.size();i++)
    {
        this->addItem(this->createIcon(colorScales[i]),
                      colorScales[i].section(re,3,3),
                      QColor(colorScales[i]));
        this->setItemData(i,QVariant(colorScales[i]));
    }

    this->setCurrentIndex(0);

    QSize iSize = this->iconSize();
    iSize.rwidth() *= this->whFactor;
    this->setIconSize(iSize);
}

void ColorScaleComboBox::setColorScaleName(const QString &colorScaleName)
{
    for (int i=0;i<this->count();i++)
    {
        if (this->itemData(i).toString() == colorScaleName)
        {
            this->setCurrentIndex(i+1);
            return;
        }
    }
}

QIcon ColorScaleComboBox::createIcon(const QString &colorScaleName)
{
    QPixmap iconPixmap;
    iconPixmap.load(colorScaleName);
    QTransform rotate_disc;
    rotate_disc.translate(iconPixmap.width()/2.0 , iconPixmap.height()/2.0);
    rotate_disc.translate(-iconPixmap.width()/2.0 , -iconPixmap.height()/2.0);
    QIcon itemIcon(iconPixmap.transformed(rotate_disc).scaled(32*this->whFactor,32));
    return itemIcon;
}
