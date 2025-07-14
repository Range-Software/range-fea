#include <QDirIterator>

#include "color_scale.h"

ColorScale::ColorScale(const QString &scaleName, QObject *parent) :
    QObject(parent),
    scaleName(scaleName)
{
}

const QString &ColorScale::getName() const
{
    return this->scaleName;
}

void ColorScale::setName(const QString &scaleName)
{
    this->scaleName = scaleName;
    emit this->changed(this->scaleName);
}

bool ColorScale::validate(const QString &scaleName)
{
    QDirIterator it(":/icons/colorscale/", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        if (it.next() == scaleName)
        {
            return true;
        }
    }
    return false;
}

QList<QString> ColorScale::getColorScales()
{
    QList<QString> colorScales;
    QDirIterator it(":/icons/colorscale/pixmaps/", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        colorScales.append(it.next());
    }
    std::sort(colorScales.begin(),colorScales.end());
    return colorScales;
}

QString ColorScale::getDefaultColorScale()
{
    QList<QString> colorScales = ColorScale::getColorScales();

    for (int i=0;i<colorScales.size();i++)
    {
        if (colorScales[i].contains("01-RGB"))
        {
            return colorScales[i];
        }
    }

    if (colorScales.size() > 0)
    {
        return colorScales[0];
    }
    else
    {
        return QString();
    }
}
