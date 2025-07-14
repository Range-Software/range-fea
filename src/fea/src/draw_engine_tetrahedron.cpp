#include "draw_engine_tetrahedron.h"

DrawEngineTetrahedron::DrawEngineTetrahedron(QObject *parent)
    : DrawEngineObject(parent)
{
    this->name = tr("tetrahedron");

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
    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,1.0),
                                                 tr("4-th node position"),
                                                 tr("Node position in X,Y,Z coordinate system."),
                                                 "m"));

    this->updateModel();
}

RModelRaw DrawEngineTetrahedron::generate() const
{
    RModelRaw modelRaw;

    RNode node1(this->inputParameters.at(0).toVector());
    RNode node2(this->inputParameters.at(1).toVector());
    RNode node3(this->inputParameters.at(2).toVector());
    RNode node4(this->inputParameters.at(3).toVector());

    modelRaw.addTriangle(node1,node3,node2);
    modelRaw.addTriangle(node1,node2,node4);
    modelRaw.addTriangle(node3,node1,node4);
    modelRaw.addTriangle(node2,node3,node4);

    return modelRaw;
}
