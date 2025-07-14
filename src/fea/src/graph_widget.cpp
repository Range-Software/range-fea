#include <cmath>
#include <QPainter>
#include <QPaintEvent>

#include <rbl_utils.h>

#include "color.h"
#include "graph_widget.h"

GraphWidget::GraphWidget(GraphObject *graphObject, bool editLimits, QWidget *parent) :
    QWidget(parent),
    graphObject(graphObject),
    xMargin(5.0),
    yMargin(5.0),
    pointWidth(5.0),
    lineWidth(5.0),
    barWidth(0.0),
    spacing(2.0),
    titleFontSize(12),
    labelFontSize(10),
    labelSpace(this->labelFontSize*2.0),
    xAxisSpace(this->labelFontSize*2.0),
    yAxisSpace(this->labelFontSize*2.0),
    showPointer(true),
    snapPointerX(true),
    snapPointerY(true),
    editLimits(editLimits),
    paintGraphLimits(false),
    paintX0(false),
    paintY0(false)
{
    this->graphObject->setParent(this);

    QObject::connect(this->graphObject,
                     &GraphObject::dataChanged,
                     this,
                     &GraphWidget::onGraphDataChanged);

    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    this->setFocusPolicy(Qt::StrongFocus);
    if (this->showPointer)
    {
        this->setMouseTracking(true);
    }

    this->xMin = this->graphObject->getData().findXMin();
    this->xMax = this->graphObject->getData().findXMax();
    this->yMin = this->graphObject->getData().findYMin();
    this->yMax = this->graphObject->getData().findYMax();
}

void GraphWidget::setShowPointer(bool showPointer, bool snapPointerX, bool snapPointerY)
{
    this->showPointer = showPointer;
    this->snapPointerX = snapPointerX;
    this->snapPointerY = snapPointerY;
    this->update();
}

void GraphWidget::setPaintGraphLimits(bool paintGraphLimits)
{
    this->paintGraphLimits = paintGraphLimits;
    this->update();
}

void GraphWidget::setPaintX0(bool paintX0)
{
    this->paintX0 = paintX0;
    this->update();
}

void GraphWidget::setPaintY0(bool paintY0)
{
    this->paintY0 = paintY0;
    this->update();
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    this->paintSetup();

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(event->rect(), QBrush(Qt::white));

    if (this->paintX0)
    {
        this->paintXLine(painter,0.0);
    }
    if (this->paintY0)
    {
        this->paintYLine(painter,0.0);
    }

    uint nDataFields = this->graphObject->getData().getNDataFields();
    for (uint i=0;i<nDataFields;i++)
    {
        if (this->graphObject->getData().getTypeMask() & GRAPH_BARS)
        {
            this->paintBars(painter,i);
        }

        if (this->graphObject->getData().getTypeMask() & GRAPH_LINES)
        {
            this->paintLines(painter,i);
        }

        if (this->graphObject->getData().getTypeMask() & GRAPH_POINTS)
        {
            this->paintPoints(painter,i);
        }

        if (this->showPointer)
        {
            this->paintPointer(painter,i);
            this->paintPosition(painter,i);
        }
    }

    if (this->paintGraphLimits)
    {
        this->paintLimits(painter);
    }
    this->paintTitle(painter);
    this->paintXLabel(painter);
    this->paintYLabel(painter);
    this->paintXAxis(painter);
    this->paintYAxis(painter);

    painter.end();
}

void GraphWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
    this->mousePosition = mouseEvent->pos();


    if (mouseEvent->buttons() & Qt::LeftButton)
    {
        if (this->editLimits)
        {
            this->updateLimits();
        }
    }

    if (this->showPointer)
    {
        this->update();
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    this->mousePosition = mouseEvent->pos();

    if (mouseEvent->buttons() & Qt::LeftButton)
    {
        if (this->editLimits)
        {
            this->updateLimits();
        }
    }

    if (this->showPointer)
    {
        this->update();
    }
}

void GraphWidget::enterEvent(QEnterEvent *)
{
    this->setFocus();
}

void GraphWidget::leaveEvent(QEvent *)
{
}

void GraphWidget::paintSetup()
{
    if (this->graphObject->getData().getTypeMask() & GRAPH_BARS)
    {
        this->barWidth = (this->getDrawingWidth()/(this->graphObject->getData().size())) - this->spacing;
    }
    else
    {
        this->barWidth = 0.0;
    }
}

void GraphWidget::paintPoints(QPainter &painter, uint dataFieldID)
{
    QVector<QRect> rectangles;

    GraphData::const_iterator iter = this->graphObject->getData().constBegin();
    while (iter != this->graphObject->getData().constEnd())
    {
        int x = int(std::round(this->getX(iter.key(),this->xMin,this->xMax) - this->pointWidth/2.0));
        int y = int(std::round(this->getY(iter.value()[dataFieldID],this->yMin,this->yMax) - this->pointWidth/2.0));
        int pw  = int(std::round(this->pointWidth));
        int ph = int(std::round(this->pointWidth));

        rectangles.append(QRect(x,y,pw,ph));

        ++iter;
    }

    painter.save();

    painter.setBrush(QBrush(Qt::green));
    painter.setPen(QPen(Qt::red));

    painter.drawRects(rectangles);

    painter.restore();
}

void GraphWidget::paintLines(QPainter &painter, uint dataFieldID)
{
    QVector<QLine> lines;

    GraphData::const_iterator iter = this->graphObject->getData().constBegin();
    while (iter != this->graphObject->getData().constEnd())
    {
        int x1 = int(std::round(this->getX(iter.key(),this->xMin,this->xMax)));
        int y1 = int(std::round(this->getY(iter.value()[dataFieldID],this->yMin,this->yMax)));
        ++iter;
        if (iter == this->graphObject->getData().constEnd())
        {
            break;
        }
        int x2 = int(std::round(this->getX(iter.key(),this->xMin,this->xMax)));
        int y2 = int(std::round(this->getY(iter.value()[dataFieldID],this->yMin,this->yMax)));

        lines.append(QLine(x1,y1,x2,y2));
    }

    painter.save();

    painter.setBrush(QBrush(Qt::green));

    QList<Qt::GlobalColor> colorList = Color::getPaintColors();
    int colorIndex = int(dataFieldID);
    if (colorIndex > colorList.size())
    {
        colorIndex = int(std::floor((std::rand()/RAND_MAX)*double(colorList.size()-1)));
    }

    painter.setPen(QPen(colorList[colorIndex]));

    painter.drawLines(lines);

    painter.restore();
}

void GraphWidget::paintBars(QPainter &painter, uint dataFieldID)
{
    QVector<QRect> rectangles;

    GraphData::const_iterator iter = this->graphObject->getData().constBegin();
    while (iter != this->graphObject->getData().constEnd()) {
        int x = int(std::round(this->getX(iter.key(),this->xMin,this->xMax) - this->barWidth/2.0));
        int y = int(std::round(this->getY(iter.value()[dataFieldID],this->yMin,this->yMax)));
        int bh = int(std::round(this->getY(0,this->yMin,this->yMax)-y));

        rectangles.append(QRect(x,y,int(std::round(this->barWidth)),bh));

        ++iter;
    }

    painter.save();

    painter.setBrush(QBrush(QColor(50,100,200)));
    painter.setPen(QPen(QColor(50,100,200)));

    painter.drawRects(rectangles);

    painter.restore();
}

void GraphWidget::paintTitle(QPainter &painter)
{
    int fontSize = this->titleFontSize;

    QFont textFont;
    textFont.setPointSize(fontSize);

    painter.save();

    painter.setPen(QPen(Qt::black));
    painter.setFont(textFont);
    int x = int(std::round(this->getX(0.0,0.0,1.0)));
    int y = int(std::round(this->getY(1.0,0.0,1.0)));
    int w = int(std::round(this->getX(1.0,0.0,1.0))) - x;
    int h = int(std::round(2.5*fontSize));

    painter.drawText(QRect(x, y, w, h), Qt::AlignCenter, this->graphObject->getData().getTitle());

    painter.restore();
}

void GraphWidget::paintPointer(QPainter &painter, uint dataFieldID)
{
    int xmin = int(std::round(this->getX(0.0,0.0,1.0) - this->barWidth));
    int xmax = int(std::round(this->getX(1.0,0.0,1.0) + this->barWidth));
    int ymin = int(std::round(this->getY(0.0,0.0,1.0)));
    int ymax = int(std::round(this->getY(1.0,0.0,1.0)));
    int xp = int(std::round(this->mousePosition.x()));
    int yp = int(std::round(this->mousePosition.y()));

    if (this->snapPointerX)
    {
        xp = int(std::round(this->getX(this->graphObject->getData().roundKey(this->getXValue(xp)),
                            this->xMin,
                            this->xMax)));
    }
    if (this->snapPointerY)
    {
        yp = int(std::round(this->getY(this->graphObject->getData().findValue(this->getXValue(xp))[dataFieldID],
                            this->yMin,
                            this->yMax)));
    }

    QVector<QLine> lines;

    if (yp <= ymin && yp >= ymax)
    {
        lines.append(QLine(xmin,yp,xmax,yp));
    }
    if (xp >= xmin && xp <= xmax)
    {
        lines.append(QLine(xp,ymin,xp,ymax));
    }

    painter.save();

    painter.setPen(QPen(Qt::gray));

    painter.drawLines(lines);

    painter.restore();
}

void GraphWidget::paintLimits(QPainter &painter)
{
    int xl = int(std::round(this->getX(this->graphObject->getData().getLLimit(),this->xMin,this->xMax)));
    int xu = int(std::round(this->getX(this->graphObject->getData().getULimit(),this->xMin,this->xMax)));
    int xmin = int(std::round(this->getX(0.0,0.0,1.0)));
    int xmax = int(std::round(this->getX(1.0,0.0,1.0)));
    int ymin = int(std::round(this->getY(0.0,0.0,1.0)));
    int ymax = int(std::round(this->getY(1.0,0.0,1.0)));

    QVector<QLine> lines;
    lines.append(QLine(xl,ymin,xl,ymax));
    lines.append(QLine(xu,ymin,xu,ymax));

    QPolygon pl;

    pl.append(QPoint(xmin,ymin));
    pl.append(QPoint(xl,ymin));
    pl.append(QPoint(xl,ymax));
    pl.append(QPoint(xmin,ymax));

    QPolygon pu;

    pu.append(QPoint(xmax,ymin));
    pu.append(QPoint(xu,ymin));
    pu.append(QPoint(xu,ymax));
    pu.append(QPoint(xmax,ymax));

    painter.save();

    painter.setPen(QPen(Qt::gray));
    painter.setBrush(QBrush(QColor(100,100,100,50)));

    painter.drawPolygon(pl);
    painter.drawPolygon(pu);

    painter.setPen(QPen(Qt::black,2.0));
    painter.drawLines(lines);

    painter.restore();
}

void GraphWidget::paintXLine(QPainter &painter, double value)
{
    int x = int(std::round(this->getX(value,this->xMin,this->xMax)));
    int ymin = int(std::round(this->getY(0.0,0.0,1.0)));
    int ymax = int(std::round(this->getY(1.0,0.0,1.0)));

    QVector<QLine> lines;
    lines.append(QLine(x,ymin,x,ymax));

    painter.save();

    painter.setPen(QPen(Qt::darkGray,1.0));
    painter.drawLines(lines);

    painter.restore();
}

void GraphWidget::paintYLine(QPainter &painter, double value)
{
    int y = int(std::round(this->getY(value,this->yMin,this->yMax)));
    int xmin = int(std::round(this->getX(0.0,0.0,1.0)));
    int xmax = int(std::round(this->getX(1.0,0.0,1.0)));

    QVector<QLine> lines;
    lines.append(QLine(xmin,y,xmax,y));

    painter.save();

    painter.setPen(QPen(Qt::darkGray,1.0));
    painter.drawLines(lines);

    painter.restore();
}

void GraphWidget::paintXLabel(QPainter &painter)
{
    int fontSize = this->labelFontSize;

    QFont textFont;
    textFont.setPointSize(fontSize);

    painter.save();

    painter.setPen(QPen(Qt::black));
    painter.setFont(textFont);
    int x = int(std::round(this->getX(0.0,0.0,1.0)));
    int y = int(std::round(this->height() - this->labelSpace));
    int h = int(std::round(this->labelSpace));
    int w = int(std::round(this->getX(1.0,0.0,1.0))) - x;

    painter.drawText(QRect(x, y, w, h), Qt::AlignCenter, this->graphObject->getData().getXLabel());

    painter.restore();
}

void GraphWidget::paintYLabel(QPainter &painter)
{
    int fontSize = this->labelFontSize;

    QFont textFont;
    textFont.setPointSize(fontSize);

    painter.save();

    painter.setPen(QPen(Qt::black));
    painter.setBrush(QBrush(Qt::red));
    painter.setFont(textFont);
    int x = int(std::round(-this->getY(0.0,0.0,1.0)));
    int y = 1;
    int w = int(std::round(this->getY(0.0,0.0,1.0) - this->getY(1.0,0.0,1.0)));
    int h = int(std::round(this->labelSpace));
    painter.rotate(-90);
    painter.drawText(QRect(x, y, w, h), Qt::AlignCenter, this->graphObject->getData().getYLabel());

    painter.restore();
}

void GraphWidget::paintXAxis(QPainter &painter)
{
    int fontSize = this->labelFontSize;

    QFont textFont;
    textFont.setPointSize(fontSize);

    painter.save();

    painter.setPen(QPen(Qt::black));
    painter.setFont(textFont);
    int x = int(std::round(this->getX(0.0,0.0,1.0)));
    int y = int(std::round(this->height() - (this->labelSpace + this->xAxisSpace)));
    int h = int(std::round(this->xAxisSpace));
    int w = int(std::round(this->getX(1.0,0.0,1.0) - x));

    painter.drawText(QRect(x, y, w, h), Qt::AlignLeft, QLocale().toString(this->xMin));
    painter.drawText(QRect(x, y, w, h), Qt::AlignRight, QLocale().toString(this->xMax));

    painter.restore();
}

void GraphWidget::paintYAxis(QPainter &painter)
{
    int fontSize = this->labelFontSize;

    QFont textFont;
    textFont.setPointSize(fontSize);

    painter.save();

    painter.setPen(QPen(Qt::black));
    painter.setBrush(QBrush(Qt::red));
    painter.setFont(textFont);
    int x = int(std::round(-this->getY(0.0,0.0,1.0)));
    int y = int(std::round(this->labelSpace));
    int w = int(std::round(this->getY(0.0,0.0,1.0) - this->getY(1.0,0.0,1.0)));
    int h = int(std::round(this->yAxisSpace));
    painter.rotate(-90);

    painter.drawText(QRect(x, y, w, h), Qt::AlignLeft, QLocale().toString(this->graphObject->getData().findYMin()));
    painter.drawText(QRect(x, y, w, h), Qt::AlignRight, QLocale().toString(this->graphObject->getData().findYMax()));

    painter.restore();
}

void GraphWidget::paintPosition(QPainter &painter,uint dataFieldID)
{
    double xValue = this->getXValue(this->mousePosition.x());
    double yValue = this->getYValue(this->mousePosition.y());

    if (this->snapPointerX)
    {
        xValue = this->graphObject->getData().roundKey(this->getXValue(this->mousePosition.x()));
    }
    if (this->snapPointerY)
    {
        yValue = this->graphObject->getData().findValue(xValue)[dataFieldID];
    }

    QString labelText = this->graphObject->getData().getXLabel() + QString(":\n")
                      + this->graphObject->getData().getYLabel() + QString(":");
    QString valueText = QLocale().toString(xValue) + QString("\n")
                      + QLocale().toString(yValue);

    int fontSize = this->labelFontSize;

    QFont textFont;
    textFont.setPointSize(fontSize);

    painter.save();

    painter.setPen(QPen(Qt::black));
    painter.setFont(textFont);

    QFontMetrics fm = painter.fontMetrics();
    double lw = std::max(fm.horizontalAdvance(this->graphObject->getData().getXLabel() + QString(": ")),
                         fm.horizontalAdvance(this->graphObject->getData().getYLabel() + QString(": ")));
    double vw = std::max(fm.horizontalAdvance(QLocale().toString(xValue)),
                         fm.horizontalAdvance(QLocale().toString(yValue)));

    int x = int(std::round(this->getX(1.0,0.0,1.0) - lw - vw));
    int y = int(std::round(this->getY(1.0,0.0,1.0)));
    int w = int(std::round(this->getX(1.0,0.0,1.0))) - x;
    int h = int(std::round(2.0*3.0*fontSize));

    painter.drawText(QRect(x, y, w, h), Qt::AlignLeft, labelText);
    painter.drawText(QRect(x + int(std::round(lw)), y, w - int(std::round(lw)), h), Qt::AlignLeft, valueText);

    painter.restore();
}

void GraphWidget::updateLimits()
{
    double xp = this->getXValue(this->mousePosition.x());

    xp = (xp < this->xMin) ? this->xMin : xp;
    xp = (xp > this->xMax) ? this->xMax : xp;

    double dxl = std::abs(xp - this->graphObject->getData().getLLimit());
    double dxu = std::abs(xp - this->graphObject->getData().getULimit());

    if (dxl < dxu)
    {
        this->graphObject->getData().setLimits(xp,this->graphObject->getData().getULimit());
    }
    else
    {
        this->graphObject->getData().setLimits(this->graphObject->getData().getLLimit(),xp);
    }
}

double GraphWidget::getDrawingWidth() const
{
    return this->width() - 2.0 * this->xMargin - this->labelSpace - this->yAxisSpace;
}

double GraphWidget::getDrawingHeight() const
{
    return this->height() - 2.0 * this->yMargin - this->labelSpace - this->xAxisSpace;
}

double GraphWidget::getX(double xValue, double xMin, double xMax) const
{
    if (std::fabs(xMax-xMin) < RConstants::eps)
    {
        return 0.0;
    }

    double x = 0.0;
    double offset = this->xMargin + this->barWidth/2.0;

    x = this->width() * (xValue - xMin) / (xMax - xMin);
    x = x * (1.0 - (2.0 * offset  + this->labelSpace + this->yAxisSpace) / this->width()) + offset + this->labelSpace + this->yAxisSpace;

    return x;
}

double GraphWidget::getY(double yValue, double yMin, double yMax) const
{
    double y = 0.0;
    if (std::fabs(yMax-yMin) > RConstants::eps)
    {
        y = this->height() * (1.0 - (yValue - yMin) / (yMax - yMin));
        y = y * (1.0 - (2.0 * this->yMargin + this->labelSpace + this->xAxisSpace) / this->height()) + this->yMargin;
    }
    return y;
}

double GraphWidget::getXValue(int x) const
{
    double xp = double(x);
    double x1 = this->xMargin + this->labelSpace + this->yAxisSpace + this->barWidth/2.0;
    double x2 = this->width() - this->xMargin - this->barWidth/2.0;

    if (xp < x1)
    {
        xp = x1;
    }
    if (xp > x2)
    {
        xp = x2;
    }

    double xr = (xp - x1)/(x2-x1);

    return this->xMin + xr * (this->xMax - this->xMin);
}

double GraphWidget::getYValue(int y) const
{
    double yp = double(y);
    double y1 = this->yMargin;
    double y2 = this->height() - this->yMargin - this->labelSpace - this->xAxisSpace;

    if (yp < y1)
    {
        yp = y1;
    }
    if (yp > y2)
    {
        yp = y2;
    }

    double yr = 1.0 - (yp - y1)/(y2-y1);

    return this->yMin + yr * (this->yMax - this->yMin);
}

void GraphWidget::onGraphDataChanged()
{
    this->xMin = this->graphObject->getData().findXMin();
    this->xMax = this->graphObject->getData().findXMax();
    this->yMin = this->graphObject->getData().findYMin();
    this->yMax = this->graphObject->getData().findYMax();

    this->update();
}
