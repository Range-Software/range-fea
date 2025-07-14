#include <rbl_error.h>
#include <rbl_logger.h>

#include "graph_object.h"

GraphObject::GraphObject(QObject *parent) :
    QObject(parent)
{
    this->sourceReadTimer = new QTimer(this);
    this->sourceReadTimer->setSingleShot(false);
    QObject::connect(this->sourceReadTimer,
                     &QTimer::timeout,
                     this,
                     &GraphObject::loadData);
}

const GraphData &GraphObject::getData() const
{
    return this->graphData;
}

GraphData &GraphObject::getData()
{
    return this->graphData;
}

void GraphObject::setData(const GraphData &graphData)
{
    this->graphData = graphData;
    this->setDataChanged();
}

void GraphObject::setDataChanged()
{
    emit this->dataChanged();
}

uint GraphObject::getSourceReadInterval() const
{
    return this->sourceReadInterval;
}

void GraphObject::setSourceReadInterval(uint sourceReadInterval)
{
    this->sourceReadInterval = sourceReadInterval;
    this->sourceReadTimer->stop();
    if (this->sourceReadInterval > 0)
    {
        this->sourceReadTimer->start(int(this->sourceReadInterval));
    }
}

void GraphObject::readSource()
{

}

void GraphObject::loadData()
{
    try
    {
        this->readSource();
    }
    catch (RError &error)
    {
        RLogger::error("Failed to read graph source. %s\n",error.getMessage().toUtf8().constData());
        return;
    }

    this->setDataChanged();
}
