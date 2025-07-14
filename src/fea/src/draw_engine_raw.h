#ifndef DRAW_ENGINE_RAW_H
#define DRAW_ENGINE_RAW_H

#include "draw_engine_object.h"

class DrawEngineRaw : public DrawEngineObject
{

        Q_OBJECT

        public:

            //! Constructor.
            explicit DrawEngineRaw(QObject *parent = nullptr);

            //! Generate sphere.
            static RModelRaw generate(const QString &rawModelContent);

        protected:

            //! Generate RAW model from input parameters.
            virtual RModelRaw generate() const;

};

#endif // DRAW_ENGINE_RAW_H
