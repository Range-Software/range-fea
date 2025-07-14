#include <QRandomGenerator>

#include <rbl_logger.h>

#include "color.h"

Color::Color()
{
    R_LOG_TRACE;
}

Color::Color(int r, int g, int b, int a) : QColor(r,g,b,a)
{
    R_LOG_TRACE;
}

Color::Color(QRgb color) : QColor(color)
{
    R_LOG_TRACE;
}

Color::Color(const QString &name) : QColor(name)
{
    R_LOG_TRACE;
}

Color::Color(const char *name) : QColor(name)
{
    R_LOG_TRACE;
}

Color::Color(const QColor &color) : QColor(color)
{
    R_LOG_TRACE;
}

Color::Color(const Color &color) : QColor(color)
{
    R_LOG_TRACE;
}

Color::Color(Qt::GlobalColor color) : QColor(color)
{
    R_LOG_TRACE;
}

Color &Color::operator=(const Color &color)
{
    R_LOG_TRACE_IN;
    this->QColor::operator=(color);
    R_LOG_TRACE_OUT;
    return (*this);
}

void Color::randomize(bool fromGlobal, bool excludeWhiteAndBlack)
{
    R_LOG_TRACE_IN;
    if (fromGlobal)
    {
        QList<Qt::GlobalColor> colorList = this->getGlobalColors(excludeWhiteAndBlack);
        int nColors = colorList.size();
        if (nColors == 0)
        {
            R_LOG_TRACE_OUT;
            return;
        }
        int colorPos = qRound((nColors-1)*QRandomGenerator::global()->generateDouble());
        this->operator = (colorList[colorPos]);
    }
    else
    {
        this->setRed(qRound(255.0*QRandomGenerator::global()->generateDouble()));
        this->setGreen(qRound(255.0*QRandomGenerator::global()->generateDouble()));
        this->setBlue(qRound(255.0*QRandomGenerator::global()->generateDouble()));
        this->setAlpha(255);
    }
    R_LOG_TRACE_OUT;
}

QColor Color::random(bool fromGlobal, bool excludeWhiteAndBlack)
{
    R_LOG_TRACE_IN;
    Color color;
    color.randomize(fromGlobal,excludeWhiteAndBlack);
    R_LOG_TRACE_OUT;
    return QColor(color.rgba());
}

QList<Qt::GlobalColor> Color::getGlobalColors(bool excludeWhiteAndBlack)
{
    R_LOG_TRACE_IN;
    QList<Qt::GlobalColor> colorList;
    if (!excludeWhiteAndBlack)
    {
        colorList.append(Qt::white);
        colorList.append(Qt::black);
    }
    colorList.append(Qt::red);
    colorList.append(Qt::darkRed);
    colorList.append(Qt::green);
    colorList.append(Qt::darkGreen);
    colorList.append(Qt::blue);
    colorList.append(Qt::darkBlue);
    colorList.append(Qt::cyan);
    colorList.append(Qt::darkCyan);
    colorList.append(Qt::magenta);
    colorList.append(Qt::darkMagenta);
    colorList.append(Qt::yellow);
    colorList.append(Qt::darkYellow);
    colorList.append(Qt::gray);
    colorList.append(Qt::darkGray);
    colorList.append(Qt::lightGray);
    R_LOG_TRACE_OUT;
    return colorList;
}

QList<Qt::GlobalColor> Color::getPaintColors()
{
    R_LOG_TRACE_IN;
    QList<Qt::GlobalColor> colorList;
    colorList.append(Qt::red);
    colorList.append(Qt::green);
    colorList.append(Qt::blue);
    colorList.append(Qt::cyan);
    colorList.append(Qt::magenta);
    colorList.append(Qt::yellow);
    colorList.append(Qt::darkRed);
    colorList.append(Qt::darkGreen);
    colorList.append(Qt::darkBlue);
    colorList.append(Qt::darkCyan);
    colorList.append(Qt::darkMagenta);
    colorList.append(Qt::darkYellow);
    R_LOG_TRACE_OUT;
    return colorList;
}
