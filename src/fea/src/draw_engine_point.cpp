#include "draw_engine_point.h"

DrawEnginePoint::DrawEnginePoint(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("point");

    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,0.0),
                                                 tr("Position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));

    this->updateModel();
}

RModelRaw DrawEnginePoint::generate() const
{
    RModelRaw modelRaw;

    modelRaw.addPoint(RNode(this->inputParameters.at(0).toVector()));

    return modelRaw;
}
