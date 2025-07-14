#ifndef DRAW_ENGINE_SPHERE_H
#define DRAW_ENGINE_SPHERE_H

#include "draw_engine_object.h"

class DrawEngineSphere : public DrawEngineObject
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit DrawEngineSphere(QObject *parent = nullptr);

        //! Generate sphere.
        static RModelRaw generate(const RR3Vector &center, double radius, uint nIter);

    protected:

        //! Generate RAW model from input parameters.
        virtual RModelRaw generate() const;

};

#endif // DRAW_ENGINE_SPHERE_H
