#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionTitleBar>

#include "sub_window_style.h"

static const int titleBarHeight = 26;
static const int frameWidth = 3;
static const int buttonSize = 18;
static const int buttonSpacing = 2;
static const int titleBarPadding = 8;
static const double cornerRadius = 8.0;
static const double highlightWidth = 2.0;

static QColor findSurfaceColor(const QPalette &palette)
{
    const QColor windowColor = palette.color(QPalette::Active,QPalette::Window);
    return windowColor.lightness() < 128 ? windowColor.lighter(125) : windowColor.lighter(105);
}

static QColor findOutlineColor(const QPalette &palette, bool active)
{
    return active ? palette.color(QPalette::Active,QPalette::Highlight) : palette.color(QPalette::Active,QPalette::Mid);
}

static double findCornerRadius(const QWidget *widget)
{
    return (widget && widget->isMaximized()) ? 0.0 : cornerRadius;
}

// Corners outside of the rounded window show the color of the area behind the window.
static void drawWindowShape(QPainter *painter, const QRectF &windowRect, double radius, const QPalette &palette, const QPen &outlinePen)
{
    QPainterPath cornersPath;
    cornersPath.setFillRule(Qt::OddEvenFill);
    cornersPath.addRect(windowRect);
    cornersPath.addRoundedRect(windowRect,radius,radius);
    painter->fillPath(cornersPath,palette.color(QPalette::Active,QPalette::Window));

    QPainterPath windowPath;
    windowPath.addRoundedRect(windowRect,radius,radius);
    painter->fillPath(windowPath,findSurfaceColor(palette));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(outlinePen);
    const double inset = outlinePen.widthF()/2.0;
    painter->drawRoundedRect(windowRect.adjusted(inset,inset,-inset,-inset),radius,radius);
}

static void drawButtonIcon(QPainter *painter, QStyle::SubControl subControl, const QRectF &buttonRect, const QColor &color)
{
    const QPointF center = buttonRect.center();
    const double size = 4.5;

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(color,1.2,Qt::SolidLine,Qt::RoundCap));

    switch (subControl)
    {
        case QStyle::SC_TitleBarMinButton:
        {
            painter->drawLine(QPointF(center.x()-size,center.y()),QPointF(center.x()+size,center.y()));
            break;
        }
        case QStyle::SC_TitleBarMaxButton:
        {
            painter->drawRect(QRectF(center.x()-size,center.y()-size,2.0*size,2.0*size));
            break;
        }
        case QStyle::SC_TitleBarNormalButton:
        {
            painter->drawRect(QRectF(center.x()-size,center.y()-size+2.0,2.0*size-2.0,2.0*size-2.0));
            painter->drawLine(QPointF(center.x()-size+2.0,center.y()-size),QPointF(center.x()+size,center.y()-size));
            painter->drawLine(QPointF(center.x()+size,center.y()-size),QPointF(center.x()+size,center.y()+size-2.0));
            break;
        }
        case QStyle::SC_TitleBarCloseButton:
        {
            painter->drawLine(QPointF(center.x()-size,center.y()-size),QPointF(center.x()+size,center.y()+size));
            painter->drawLine(QPointF(center.x()-size,center.y()+size),QPointF(center.x()+size,center.y()-size));
            break;
        }
        default:
        {
            break;
        }
    }
}

SubWindowStyle::SubWindowStyle(const QColor &modelColor)
    : modelColor(modelColor)
    , highlighted(false)
{
}

void SubWindowStyle::setHighlighted(bool highlighted)
{
    this->highlighted = highlighted;
}

QPen SubWindowStyle::findOutlinePen(const QPalette &palette, bool active) const
{
    return this->highlighted ? QPen(this->modelColor,highlightWidth) : QPen(findOutlineColor(palette,active),1.0);
}

int SubWindowStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
        case PM_TitleBarHeight:
        {
            return titleBarHeight;
        }
        case PM_MdiSubWindowFrameWidth:
        {
            return frameWidth;
        }
        default:
        {
            return QProxyStyle::pixelMetric(metric,option,widget);
        }
    }
}

int SubWindowStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint)
    {
        case SH_TitleBar_NoBorder:
        {
            return 1;
        }
        case SH_WindowFrame_Mask:
        {
            // Mask would make rounded corners jagged, they are painted instead.
            return 0;
        }
        default:
        {
            return QProxyStyle::styleHint(hint,option,widget,returnData);
        }
    }
}

void SubWindowStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element != PE_FrameWindow)
    {
        QProxyStyle::drawPrimitive(element,option,painter,widget);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    drawWindowShape(painter,QRectF(option->rect),findCornerRadius(widget),option->palette,this->findOutlinePen(option->palette,option->state & State_Active));
    painter->restore();
}

void SubWindowStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTitleBar *titleBarOption = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
    if (control != CC_TitleBar || !titleBarOption)
    {
        QProxyStyle::drawComplexControl(control,option,painter,widget);
        return;
    }

    const bool active = option->state & State_Active;
    const bool minimized = titleBarOption->titleBarState & Qt::WindowMinimized;
    const QPalette &palette = option->palette;
    QColor textColor = palette.color(QPalette::Active,QPalette::WindowText);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Without minimized window the lower part of the shape is covered by the frame.
    painter->setClipRect(option->rect);
    QRectF shapeRect(option->rect);
    if (!minimized)
    {
        shapeRect.setBottom(shapeRect.bottom() + 2.0*cornerRadius);
    }
    drawWindowShape(painter,shapeRect,findCornerRadius(widget),palette,this->findOutlinePen(palette,active));

    // Line distinguishing the model follows rounded corners of the window, 1px of padding separates it from the outline.
    QPainterPath shapePath;
    shapePath.addRoundedRect(shapeRect,findCornerRadius(widget),findCornerRadius(widget));
    QPainterPath linePath;
    linePath.addRect(QRectF(option->rect.left(),option->rect.top()+2.0,option->rect.width(),2.0));
    painter->fillPath(shapePath.intersected(linePath),this->modelColor);

    if (!minimized)
    {
        QColor lineColor(textColor);
        lineColor.setAlphaF(0.12f);
        painter->setPen(QPen(lineColor,1.0));
        painter->drawLine(QPointF(option->rect.left()+1.0,option->rect.bottom()+0.5),QPointF(option->rect.right(),option->rect.bottom()+0.5));
    }

    if (!active)
    {
        textColor.setAlphaF(0.55f);
    }

    // Title
    QFont titleFont(painter->font());
    titleFont.setWeight(active ? QFont::Medium : QFont::Normal);
    painter->setFont(titleFont);
    painter->setPen(textColor);
    const QRect labelRect = this->subControlRect(control,option,SC_TitleBarLabel,widget);
    painter->drawText(labelRect,Qt::AlignLeft|Qt::AlignVCenter,painter->fontMetrics().elidedText(titleBarOption->text,Qt::ElideRight,labelRect.width()));

    // Buttons
    const SubControl buttons[] = { SC_TitleBarMinButton, SC_TitleBarNormalButton, SC_TitleBarMaxButton, SC_TitleBarCloseButton };
    for (SubControl button : buttons)
    {
        const QRect buttonRect = this->subControlRect(control,option,button,widget);
        if (!buttonRect.isValid())
        {
            continue;
        }

        const bool hovered = option->activeSubControls & button;
        const bool sunken = hovered && (option->state & State_Sunken);
        QColor iconColor(textColor);

        if (hovered)
        {
            QColor hoverColor(palette.color(QPalette::Active,QPalette::WindowText));
            hoverColor.setAlphaF(sunken ? 0.24f : 0.14f);
            if (button == SC_TitleBarCloseButton)
            {
                hoverColor = sunken ? QColor(176,44,44) : QColor(210,60,60);
                iconColor = Qt::white;
            }
            QPainterPath hoverPath;
            hoverPath.addRoundedRect(QRectF(buttonRect),4.0,4.0);
            painter->fillPath(hoverPath,hoverColor);
        }

        drawButtonIcon(painter,button,QRectF(buttonRect),iconColor);
    }

    painter->restore();
}

QRect SubWindowStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    const QStyleOptionTitleBar *titleBarOption = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
    if (control != CC_TitleBar || !titleBarOption)
    {
        return QProxyStyle::subControlRect(control,option,subControl,widget);
    }

    const QRect &titleBarRect = option->rect;
    const bool hasClose = titleBarOption->titleBarFlags & Qt::WindowSystemMenuHint;
    const bool hasMax = titleBarOption->titleBarFlags & Qt::WindowMaximizeButtonHint;
    const bool hasMin = titleBarOption->titleBarFlags & Qt::WindowMinimizeButtonHint;
    const bool isMaximized = titleBarOption->titleBarState & Qt::WindowMaximized;
    const bool isMinimized = titleBarOption->titleBarState & Qt::WindowMinimized;

    auto buttonRect = [&](int slot)
    {
        return QRect(titleBarRect.right() + 1 - titleBarPadding/2 - buttonSize - slot*(buttonSize + buttonSpacing),
                     titleBarRect.top() + (titleBarRect.height() - buttonSize)/2,
                     buttonSize,
                     buttonSize);
    };

    // Buttons are placed from right to left.
    int maxSlot = hasClose ? 1 : 0;
    int minSlot = maxSlot + (hasMax ? 1 : 0);
    int nSlots = minSlot + (hasMin ? 1 : 0);

    switch (subControl)
    {
        case SC_TitleBarLabel:
        {
            QRect labelRect(titleBarRect);
            labelRect.setLeft(labelRect.left() + titleBarPadding);
            labelRect.setRight(labelRect.right() - titleBarPadding/2 - nSlots*(buttonSize + buttonSpacing));
            return labelRect;
        }
        case SC_TitleBarCloseButton:
        {
            return hasClose ? buttonRect(0) : QRect();
        }
        case SC_TitleBarMaxButton:
        {
            return (hasMax && !isMaximized) ? buttonRect(maxSlot) : QRect();
        }
        case SC_TitleBarMinButton:
        {
            return (hasMin && !isMinimized) ? buttonRect(minSlot) : QRect();
        }
        case SC_TitleBarNormalButton:
        {
            if (isMaximized && hasMax)
            {
                return buttonRect(maxSlot);
            }
            return (isMinimized && hasMin) ? buttonRect(minSlot) : QRect();
        }
        default:
        {
            return QRect();
        }
    }
}

QStyle::SubControl SubWindowStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
{
    if (control != CC_TitleBar)
    {
        return QProxyStyle::hitTestComplexControl(control,option,position,widget);
    }

    const SubControl subControls[] = { SC_TitleBarCloseButton, SC_TitleBarMaxButton, SC_TitleBarMinButton, SC_TitleBarNormalButton, SC_TitleBarLabel };
    for (SubControl subControl : subControls)
    {
        if (this->subControlRect(control,option,subControl,widget).contains(position))
        {
            return subControl;
        }
    }
    return option->rect.contains(position) ? SC_TitleBarLabel : SC_None;
}
