#include "draw_engine_triangle.h"

DrawEngineTriangle::DrawEngineTriangle(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("triangle");

    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,0.0),
                                                 tr("1-st node position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(RR3Vector(1.0,0.0,0.0),
                                                 tr("2-nd node position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,1.0,0.0),
                                                 tr("3-rd node position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));

    this->updateModel();
}

RModelRaw DrawEngineTriangle::generate() const
{
    RModelRaw modelRaw;

    modelRaw.addTriangle(RNode(this->inputParameters.at(0).toVector()),
                         RNode(this->inputParameters.at(1).toVector()),
                         RNode(this->inputParameters.at(2).toVector()));

    return modelRaw;
}
