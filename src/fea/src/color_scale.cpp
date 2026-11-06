#include <QDirIterator>
#include <QImage>

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

int ColorScale::getBandCount(const QString &scaleName)
{
    // All stepped scale images (suffix S or RS, with O when outlined) consist of nine bands.
    const bool stepped = ColorScale::getBandOutline(scaleName)
                      || scaleName.endsWith("_S.png")
                      || scaleName.endsWith("_RS.png");
    return stepped ? 9 : 0;
}

bool ColorScale::getBandOutline(const QString &scaleName)
{
    return scaleName.endsWith("_SO.png") || scaleName.endsWith("_RSO.png");
}

QList<QColor> ColorScale::getBandColors(const QString &scaleName)
{
    QList<QColor> colors;

    const int nBands = ColorScale::getBandCount(scaleName);
    const QImage image(scaleName);
    if (nBands == 0 || image.isNull())
    {
        return colors;
    }

    // Lines in the image are at the band edges, center of a band always has its color.
    for (int i=0;i<nBands;i++)
    {
        colors.append(image.pixelColor((2*i+1)*image.width()/(2*nBands),0));
    }
    return colors;
}

QColor ColorScale::findBandLineColor(const QColor &lowerColor, const QColor &upperColor)
{
    // Keep in line with findBandColor() in fragment shaders.
    const double r = 0.5*(lowerColor.redF() + upperColor.redF());
    const double g = 0.5*(lowerColor.greenF() + upperColor.greenF());
    const double b = 0.5*(lowerColor.blueF() + upperColor.blueF());
    const double luminance = 0.299*r + 0.587*g + 0.114*b;

    if (luminance < 0.15)
    {
        return QColor::fromRgbF(float(r + 0.35*(1.0-r)),float(g + 0.35*(1.0-g)),float(b + 0.35*(1.0-b)));
    }
    return QColor::fromRgbF(float(0.5*r),float(0.5*g),float(0.5*b));
}
