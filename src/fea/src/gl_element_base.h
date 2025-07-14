#ifndef GL_ELEMENT_BASE_H
#define GL_ELEMENT_BASE_H

#include <QtOpenGL>

#include "gl_object.h"
#include "model.h"

typedef enum _GLElementDrawMode
{
    GL_ELEMENT_DRAW_NORMAL = 0,
    GL_ELEMENT_DRAW_TEXT,
    GL_ELEMENT_DRAW_N_MODES
} GLElementDrawMode;

class GLElementBase : public GLObject
{

    protected:

        //! Draw mode.
        GLElementDrawMode drawMode;
        //! Const pointer to element group data object.
        REntityGroupData elementGroupData;
        //! Color.
        QColor color;
        //! Const pointer to model object.
        const Model *pModel;
        //! Element ID.
        uint elementID;
        //! Const pointer to scalar variable.
        const RVariable *pScalarVariable;
        //! Const pointer to displacement variable.
        const RVariable *pDisplacementVariable;
        //! Node point size.
        GLfloat nodePointSize;
        //! Edge line width.
        GLfloat edgeLineWidth;
        // Environment settings
        GLboolean lineSmoothEnabled;
        GLboolean lightingEnabled;
        GLboolean normalize;
        GLfloat pointSize;
        GLfloat lineWidth;

    private:

        //! Internal initialization function.
        void _init(const GLElementBase *pGlElement = nullptr);

    public:

        //! Constructor.
        GLElementBase(GLWidget *glWidget, const Model *pModel, uint elementID, const REntityGroupData &elementGroupData, const QColor &color, GLElementDrawMode drawMode = GL_ELEMENT_DRAW_NORMAL);

        //! Copy constructor.
        GLElementBase(const GLElementBase &glElement);

        //! Destructor.
        ~GLElementBase();

        //! Assignment operator.
        GLElementBase & operator = (const GLElementBase &glElement);

        //! Return draw mode.
        GLElementDrawMode getDrawMode() const;

        //! Return const reference to element group data.
        const REntityGroupData & getElementGroupData() const;

        //! Return const reference to element color.
        const QColor & getColor() const;

        //! Return const pointer to model.
        const Model * getModelPtr() const;

        //! Set pointer to model.
        void setModelPtr(const Model *pModel);

        //! Return element ID.
        uint getElementID() const;

        //! Set element ID.
        void setElementID(uint elementID);

        //! Set scalar variable position.
        void setScalarVariable(const RVariable *pScalarVariable);

        //! Set displacement variable position.
        void setDisplacementVariable(const RVariable *pDisplacementVariable);

        //! Set node point size.
        void setNodePointSize(GLfloat nodePointSize);

        //! Set edge line width.
        void setEdgeLineWidth(GLfloat edgeLineWidth);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

};

#endif /* GL_ELEMENT_BASE_H */
