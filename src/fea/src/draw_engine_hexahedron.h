#ifndef DRAW_ENGINE_HEXAHEDRON_H
#define DRAW_ENGINE_HEXAHEDRON_H

#include "draw_engine_object.h"

class DrawEngineHexahedron : public DrawEngineObject
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit DrawEngineHexahedron(QObject *parent = nullptr);

    protected:

        //! Generate RAW model from input parameters.
        virtual RModelRaw generate() const;

};

#endif // DRAW_ENGINE_HEXAHEDRON_H
