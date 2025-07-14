#ifndef DRAW_ENGINE_POINT_H
#define DRAW_ENGINE_POINT_H

#include "draw_engine_object.h"

class DrawEnginePoint : public DrawEngineObject
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit DrawEnginePoint(QObject *parent = nullptr);

    protected:

        //! Generate RAW model from input parameters.
        virtual RModelRaw generate() const;

};

#endif // DRAW_ENGINE_POINT_H
