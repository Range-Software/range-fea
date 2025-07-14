#include <algorithm>

#include <rbl_utils.h>

#include "graph_data.h"

void GraphData::_init(const GraphData *pGraphData)
{
    if (pGraphData)
    {
        this->typeMask = pGraphData->typeMask;
        this->title = pGraphData->title;
        this->xlabel = pGraphData->xlabel;
        this->ylabel = pGraphData->ylabel;
        this->lLimit = pGraphData->lLimit;
        this->uLimit = pGraphData->uLimit;
    }
}

GraphData::GraphData()
    : lLimit(0.0)
    , uLimit(0.0)
{
    this->_init();
}

GraphData::GraphData(const GraphData &graphData) : QMap<double,RRVector>(graphData)
{
    this->_init(&graphData);
}

GraphData &GraphData::operator =(const GraphData &graphData)
{
    QMap::operator =(graphData);
    this->_init(&graphData);
    return (*this);
}

GraphTypeMask GraphData::getTypeMask() const
{
    return this->typeMask;
}

void GraphData::setTypeMask(GraphTypeMask typeMask)
{
    this->typeMask = typeMask;
}

const QString &GraphData::getTitle() const
{
    return this->title;
}

void GraphData::setTitle(const QString &title)
{
    this->title = title;
}

const QString &GraphData::getXLabel() const
{
    return this->xlabel;
}

void GraphData::setXLabel(const QString &xlabel)
{
    this->xlabel = xlabel;
}

const QString &GraphData::getYLabel() const
{
    return this->ylabel;
}

void GraphData::setYLabel(const QString &ylabel)
{
    this->ylabel = ylabel;
}

double GraphData::getLLimit() const
{
    return this->lLimit;
}

double GraphData::getULimit() const
{
    return this->uLimit;
}

void GraphData::setLimits(double lLimit, double uLimit)
{
    this->lLimit = lLimit;
    this->uLimit = uLimit;
}

double GraphData::findXMin() const
{
    double value = 0.0;
    bool firstValue = true;

    GraphData::const_iterator iter = this->constBegin();
    while (iter != this->constEnd()) {
        if (firstValue)
        {
            value = iter.key();
            firstValue = false;
        }
        else
        {
            value = std::min(value,iter.key());
        }
        ++iter;
    }

    return value;
}

double GraphData::findXMax() const
{
    double value = 0.0;
    bool firstValue = true;

    GraphData::const_iterator iter = this->constBegin();
    while (iter != this->constEnd()) {
        if (firstValue)
        {
            value = iter.key();
            firstValue = false;
        }
        else
        {
            value = std::max(value,iter.key());
        }
        ++iter;
    }

    return value;
}

double GraphData::findYMin() const
{
    double value = 0.0;
    bool firstValue = true;

    GraphData::const_iterator iter = this->constBegin();
    while (iter != this->constEnd())
    {
        const RRVector &rValues = iter.value();
        for (unsigned int i=0;i<rValues.size();i++)
        {
            if (firstValue)
            {
                value = rValues[i];
                firstValue = false;
            }
            else
            {
                value = std::min(value,rValues[i]);
            }
        }
        ++iter;
    }

    return value;
}

double GraphData::findYMax() const
{
    double value = 0.0;
    bool firstValue = true;

    GraphData::const_iterator iter = this->constBegin();
    while (iter != this->constEnd())
    {
        const RRVector &rValues = iter.value();
        for (unsigned int i=0;i<rValues.size();i++)
        {
            if (firstValue)
            {
                value = rValues[i];
                firstValue = false;
            }
            else
            {
                value = std::max(value,rValues[i]);
            }
        }
        ++iter;
    }

    return value;
}

double GraphData::roundKey(double x) const
{
    double x1 = 0.0;
    double x2 = 0.0;
    bool firstValue = true;

    double xmin = this->findXMin();
    double xmax = this->findXMax();

    x = (x < xmin) ? xmin : x;
    x = (x > xmax) ? xmax : x;

    GraphData::const_iterator iter = this->constBegin();
    while (iter != this->constEnd())
    {
        if (firstValue)
        {
            x1 = iter.key();
            x2 = x1;
            firstValue = false;
        }
        else
        {
            x2 = iter.key();
        }
        if (x1 <= x && x <= x2)
        {
            if (x-x1 < x2-x)
            {
                return x1;
            }
            else
            {
                return x2;
            }
        }
        x1 = x2;
        ++iter;
    }
    return x;
}

RRVector GraphData::findValue(double x) const
{
    double x1 = 0.0;
    double x2 = 0.0;
    RRVector y1;
    RRVector y2;
    double ratio = 0.0;
    bool firstValue = true;

    double xmin = this->findXMin();
    double xmax = this->findXMax();

    x = (x < xmin) ? xmin : x;
    x = (x > xmax) ? xmax : x;

    GraphData::const_iterator iter = this->constBegin();
    while (iter != this->constEnd())
    {
        if (firstValue)
        {
            x1 = iter.key();
            y1 = iter.value();
            x2 = x1;
            y2 = y1;
            firstValue = false;
        }
        else
        {
            x2 = iter.key();
            y2 = iter.value();
        }
        if (x1 <= x && x <= x2)
        {
            if (std::abs(x1-x2) < RConstants::eps)
            {
                ratio = 0.0;
            }
            else
            {
                ratio = (x-x1)/(x2-x1);
            }
            break;
        }
        x1 = x2;
        y1 = y2;
        ++iter;
    }

    ratio = ratio < 0.5 ? 0.0 : 1.0;

    RRVector values(uint(y1.size() > y2.size() ? y1.size() : y2.size()));
    values.fill(0.0);
    for (unsigned int i=0;i<values.size();i++)
    {
        if (i < y1.size() && i < y2.size())
        {
            values[i] = ratio*(y2[i]-y1[i])+y1[i];
        }
    }

    return values;
}

uint GraphData::getNDataFields() const
{
    GraphData::const_iterator iter = this->constBegin();
    uint nData = 0;
    while (iter != this->constEnd())
    {
        nData = std::max(nData,uint(iter.value().size()));
        ++iter;
    }
    return nData;
}
