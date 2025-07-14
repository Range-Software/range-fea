#ifndef DRAW_ENGINE_OBJECT_H
#define DRAW_ENGINE_OBJECT_H

#include <QObject>

#include "draw_engine_input.h"
#include "model.h"

class GLWidget;

class DrawEngineObject : public QObject
{

    Q_OBJECT

    protected:

        //! Object name.
        QString name;
        //! List of input parameters.
        QList<DrawEngineInput> inputParameters;
        //! OpenGL color.
        QColor color;
        //! Generated model.
        Model model;

    public:

        //! Constructor.
        explicit DrawEngineObject(QObject *parent = nullptr);

        //! Return const reference object name.
        const QString &getName() const;

        //! Return number of input parameters.
        uint getNInputParameters() const;

        //! Return const reference to input parameter at given position.
        const DrawEngineInput &getInputParamater(uint position) const;

        //! Return reference to input parameter at given position.
        DrawEngineInput &getInputParamater(uint position);

        //! Update model.
        void updateModel();

        //! Return reference to generated model.
        Model &getModel();

        //! Return const reference to generated model.
        const Model &getModel() const;

        //! Draw object to OpenGL.
        void glDraw(GLWidget *glWidget) const;

    protected:

        //! Generate RAW model from input parameters.
        virtual RModelRaw generate() const = 0;

};

#endif // DRAW_ENGINE_OBJECT_H
