#include <cmath>

#include "draw_engine_sphere.h"

DrawEngineSphere::DrawEngineSphere(QObject *parent) :
    DrawEngineObject(parent)
{
    this->name = tr("sphere");

    this->inputParameters.append(DrawEngineInput(RR3Vector(0.0,0.0,0.0),
                                                 tr("Position"),
                                                 tr("Position in X,Y,Z coordinate system."),
                                                 "m"));
    this->inputParameters.append(DrawEngineInput(0.25,tr("Radius"),tr("Sphere radius."),"m",RConstants::eps));
    this->inputParameters.append(DrawEngineInput(uint(3),tr("Refinement"),tr("Number of refinement iterations."),"#",uint(1),uint(5)));

    this->updateModel();
}

RModelRaw DrawEngineSphere::generate(const RR3Vector &center, double radius, uint nIter)
{
    double a = 1.0/sqrt(2.0);

    RNode n1(0.0,0.0,1.0);
    RNode n2(0.0,0.0,-1.0);
    RNode n3(-a,-a,0.0);
    RNode n4(a,-a,0.0);
    RNode n5(a,a,0.0);
    RNode n6(-a,a,0.0);

    std::vector<RTriangle*> s;

    s.push_back(new RTriangle(n1,n4,n5));
    s.push_back(new RTriangle(n1,n5,n6));
    s.push_back(new RTriangle(n1,n6,n3));
    s.push_back(new RTriangle(n1,n3,n4));
    s.push_back(new RTriangle(n2,n5,n4));
    s.push_back(new RTriangle(n2,n6,n5));
    s.push_back(new RTriangle(n2,n3,n6));
    s.push_back(new RTriangle(n2,n4,n3));

    RModelRaw modelRaw;

    // Perform bisection untill sone refinement is reached
    for (uint iter=0;iter<nIter;iter++)
    {
        uint nt = uint(s.size());
        for (uint i=0;i<nt;i++)
        {
            RTriangle *t = s.at(i);

            RNode n1 = t->getNode1();
            RNode n2 = t->getNode2();
            RNode n3 = t->getNode3();

            RR3Vector v1 = n1.toVector();
            RR3Vector v2 = n2.toVector();
            RR3Vector v3 = n3.toVector();

            RR3Vector v12((v1[0]+v2[0])/2.0,(v1[1]+v2[1])/2.0,(v1[2]+v2[2])/2.0);
            RR3Vector v23((v2[0]+v3[0])/2.0,(v2[1]+v3[1])/2.0,(v2[2]+v3[2])/2.0);
            RR3Vector v31((v3[0]+v1[0])/2.0,(v3[1]+v1[1])/2.0,(v3[2]+v1[2])/2.0);

            v12.normalize();
            v23.normalize();
            v31.normalize();

            RNode n12(v12);
            RNode n23(v23);
            RNode n31(v31);

            t->setNodes(n12,n23,n31);

            s.push_back(new RTriangle(n1,n12,n31));
            s.push_back(new RTriangle(n2,n23,n12));
            s.push_back(new RTriangle(n3,n31,n23));
        }
    }

    // Convert to RAW model
    for (uint i=0;i<s.size();i++)
    {
        RTriangle *t = s.at(i);

        RNode n1 = t->getNode1();
        RNode n2 = t->getNode2();
        RNode n3 = t->getNode3();

        n1.scale(radius);
        n1.translate(center);

        n2.scale(radius);
        n2.translate(center);

        n3.scale(radius);
        n3.translate(center);

        modelRaw.addTriangle(n1,n2,n3);
    }

    return modelRaw;
}

RModelRaw DrawEngineSphere::generate() const
{
    RR3Vector c = this->inputParameters.at(0).toVector();
    double r = this->inputParameters.at(1).toDouble();
    uint nIter = this->inputParameters.at(2).toUint();

    return DrawEngineSphere::generate(c,r,nIter);
}
