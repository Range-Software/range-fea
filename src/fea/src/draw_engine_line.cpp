#include "draw_engine_line.h"

DrawEngineLine::DrawEngineLine(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("line");

    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,0.0),
                                                 tr("1-st node position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(RR3Vector(1.0,0.0,0.0),
                                                 tr("2-nd node position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));

    this->updateModel();
}

RModelRaw DrawEngineLine::generate() const
{
    RModelRaw modelRaw;

    modelRaw.addSegment(RNode(this->inputParameters.at(0).toVector()),
                        RNode(this->inputParameters.at(1).toVector()));

    return modelRaw;
}
