#ifndef DRAW_ENGINE_LINE_H
#define DRAW_ENGINE_LINE_H

#include "draw_engine_object.h"

class DrawEngineLine : public DrawEngineObject
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit DrawEngineLine(QObject *parent = nullptr);

    protected:

        //! Generate RAW model from input parameters.
        virtual RModelRaw generate() const;

};

#endif // DRAW_ENGINE_LINE_H
