#include <cmath>
#include <algorithm>

#include "draw_engine_cylinder.h"

DrawEngineCylinder::DrawEngineCylinder(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("cylinder");
    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,0.0),
                                                 tr("Position (bottom)"),
                                                 tr("Position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,1.0),
                                                 tr("Position (top)"),
                                                 tr("Position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(0.25,
                                                 tr("Radius (bottom)"),
                                                 tr("Radius of bottom circle."),
                                                 "m",
                                                 0.0));
    this->inputParameters.append(DrawEngineInput(0.25,
                                                 tr("Radius (top)"),
                                                 tr("Radius of top circle."),
                                                 "m",
                                                 0.0));
    this->inputParameters.append(DrawEngineInput(0.0,
                                                 tr("Start angle"),
                                                 tr("Opening angle of the cylinder section."),
                                                 "deg",
                                                 0.0,
                                                 360.0));
    this->inputParameters.append(DrawEngineInput(360.0,
                                                 tr("End angle"),
                                                 tr("Closing angle of the cylinder section."),
                                                 "deg",
                                                 0.0,
                                                 360.0));
    this->inputParameters.append(DrawEngineInput(uint(20),
                                                 tr("Anglular divisions"),
                                                 tr("Number of angular divisions."),
                                                 "#",
                                                 uint(3)));
    this->inputParameters.append(DrawEngineInput(uint(10),
                                                 tr("Axial divisions"),
                                                 tr("Number of divisions along rotation axis."),
                                                 "#",
                                                 uint(1)));
    this->updateModel();
}

RModelRaw DrawEngineCylinder::generate() const
{
    RModelRaw modelRaw;

    RR3Vector bc = this->inputParameters.at(0).toVector();
    RR3Vector tc = this->inputParameters.at(1).toVector();
    double br = this->inputParameters.at(2).toDouble();
    double tr = this->inputParameters.at(3).toDouble();
    double sa = this->inputParameters.at(4).toDouble();
    double ea = this->inputParameters.at(5).toDouble();
    double nai = this->inputParameters.at(6).toUint();
    double nxi = this->inputParameters.at(7).toUint();

    double da = 0.0;
    if (sa > ea)
    {
        da = 360.0 - (sa-ea);
    }
    else if (sa == ea)
    {
        da = 360.0;
    }
    else
    {
        da = ea - sa;
    }
    da /= double(nai);

    bool bCone = std::fabs(br) < RConstants::eps;
    bool tCone = std::fabs(tr) < RConstants::eps;

    RR3Vector direction(tc[0]-bc[0],tc[1]-bc[1],tc[2]-bc[2]);
    double length = direction.length();

    RNode bCenterNode(0.0,0.0,0.0);
    RNode tCenterNode(length,0.0,0.0);

    for (uint i=0;i<nai;i++)
    {
        double a1 = sa + i*da;
        double a2 = a1 + da;

        if (a1 > 360.0)
        {
            a1 -= 360.0;
        }

        if (a2 > 360.0)
        {
            a2 -= 360.0;
        }

        a1 = R_DEG_TO_RAD(a1);
        a2 = R_DEG_TO_RAD(a2);

        RNode bNode1(0.0,br*cos(a1),br*sin(a1));
        RNode bNode2(0.0,br*cos(a2),br*sin(a2));

        RNode tNode1(length,tr*cos(a1),tr*sin(a1));
        RNode tNode2(length,tr*cos(a2),tr*sin(a2));

        if (!bCone)
        {
            modelRaw.addTriangle(bCenterNode,bNode1,bNode2);
        }

        if (!tCone)
        {
            modelRaw.addTriangle(tCenterNode,tNode2,tNode1);
        }

        double dx = length / nxi;
        double dr = (tr-br) / nxi;
        for (uint j=0;j<nxi;j++)
        {
            double r1 = j*dr+br;
            double r2 = r1+dr;
            double x1 = j*dx;
            double x2 = x1 + dx;

            RNode bNode1(x1,r1*cos(a1),r1*sin(a1));
            RNode bNode2(x1,r1*cos(a2),r1*sin(a2));

            RNode tNode1(x2,r2*cos(a1),r2*sin(a1));
            RNode tNode2(x2,r2*cos(a2),r2*sin(a2));

            if (j == 0 && bCone)
            {
                modelRaw.addTriangle(bNode1,tNode1,tNode2);
            }
            else if (j == nxi - 1 && tCone)
            {
                modelRaw.addTriangle(tNode1,bNode2,bNode1);
            }
            else
            {
                modelRaw.addQuadrilateral(bNode2,bNode1,tNode1,tNode2);
            }
        }
    }

    RRMatrix R;
    direction.findRotationMatrix(R);

    for (uint i=0;i<modelRaw.getNNodes();i++)
    {
        modelRaw.getNode(i).transform(R,bc);
    }

    return modelRaw;
}
