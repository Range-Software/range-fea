#ifndef DRAW_ENGINE_QUADRILATERAL_H
#define DRAW_ENGINE_QUADRILATERAL_H

#include "draw_engine_object.h"

class DrawEngineQuadrilateral : public DrawEngineObject
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit DrawEngineQuadrilateral(QObject *parent = nullptr);

    protected:

        //! Generate RAW model from input parameters.
        virtual RModelRaw generate() const;

};

#endif // DRAW_ENGINE_QUADRILATERAL_H
