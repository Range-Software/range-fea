#include "draw_engine_hexahedron.h"

DrawEngineHexahedron::DrawEngineHexahedron(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("hexahedron");

    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,0.0),
                                                 tr("Position"),
                                                 tr("Position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(1.0,tr("X size"),tr("Size in X direction."),"m"));
    this->inputParameters.append(DrawEngineInput(1.0,tr("Y size"),tr("Size in Y direction."),"m"));
    this->inputParameters.append(DrawEngineInput(1.0,tr("Z size"),tr("Size in Z direction."),"m"));
    this->inputParameters.append(DrawEngineInput(uint(1),tr("X divisions"),tr("Number of divisions in X direction."),"#",uint(1)));
    this->inputParameters.append(DrawEngineInput(uint(1),tr("Y divisions"),tr("Number of divisions in Y direction."),"#",uint(1)));
    this->inputParameters.append(DrawEngineInput(uint(1),tr("Z divisions"),tr("Number of divisions in Z direction."),"#",uint(1)));

    this->updateModel();
}

RModelRaw DrawEngineHexahedron::generate() const
{
    RModelRaw modelRaw;

    RR3Vector origin = this->inputParameters.at(0).toVector();
    double sx = this->inputParameters.at(1).toDouble();
    double sy = this->inputParameters.at(2).toDouble();
    double sz = this->inputParameters.at(3).toDouble();
    uint nx = this->inputParameters.at(4).toUint();
    uint ny = this->inputParameters.at(5).toUint();
    uint nz = this->inputParameters.at(6).toUint();

    double dx = sx/nx;
    double dy = sy/ny;
    double dz = sz/nz;

    double x, y, z;

    // XY
    x = origin[0];
    z = origin[2];
    for (uint i=0;i<nx;i++)
    {
        y = origin[1];
        for (uint j=0;j<ny;j++)
        {
            modelRaw.addQuadrilateral(RNode(x,y,z+sz),RNode(x+dx,y,z+sz),RNode(x+dx,y+dy,z+sz),RNode(x,y+dy,z+sz));
            modelRaw.addQuadrilateral(RNode(x,y,z),RNode(x,y+dy,z),RNode(x+dx,y+dy,z),RNode(x+dx,y,z));
            y += dy;
        }
        x += dx;
    }

    // XZ
    x = origin[0];
    y = origin[1];
    for (uint i=0;i<nx;i++)
    {
        z = origin[2];
        for (uint j=0;j<nz;j++)
        {
            modelRaw.addQuadrilateral(RNode(x,y+sy,z),RNode(x,y+sy,z+dz),RNode(x+dx,y+sy,z+dz),RNode(x+dx,y+sy,z));
            modelRaw.addQuadrilateral(RNode(x,y,z),RNode(x+dx,y,z),RNode(x+dx,y,z+dz),RNode(x,y,z+dz));
            z += dz;
        }
        x += dx;
    }

    // YZ
    x = origin[0];
    y = origin[1];
    for (uint i=0;i<ny;i++)
    {
        z = origin[2];
        for (uint j=0;j<nz;j++)
        {
            modelRaw.addQuadrilateral(RNode(x+sx,y,z),RNode(x+sx,y+dy,z),RNode(x+sx,y+dy,z+dz),RNode(x+sx,y,z+dz));
            modelRaw.addQuadrilateral(RNode(x,y,z),RNode(x,y,z+dz),RNode(x,y+dy,z+dz),RNode(x,y+dy,z));
            z += dz;
        }
        y += dy;
    }

    return modelRaw;
}
