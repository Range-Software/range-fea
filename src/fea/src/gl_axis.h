#ifndef GL_AXIS_H
#define GL_AXIS_H

#include "gl_object.h"

typedef enum _GLAxisType
{
    GL_AXIS_NONE = 0,
    GL_AXIS_GLOBAL,
    GL_AXIS_LOCAL,
    GL_AXIS_POSITION,
    GL_AXIS_N_TYPES
} GLAxisType;

class GLAxis : public GLObject
{

    private:

        // Environment settings.
        GLboolean depthTestEnabled;
        GLboolean lineSmoothEnabled;
        GLint lineSmoothHint;
        GLboolean normalizeEnabled;
        GLboolean lightingEnabled;
        GLfloat lineWidth;
        GLboolean cullFaceEnabled;

    protected:

        //! Axis type.
        GLAxisType type;
        //! Axis size.
        float size;
        //! Axis name.
        QString name;

    private:

        //! Internal initialization function.
        void _init ( const GLAxis *pGlAxis = nullptr );

    public:

        //! Constructor.
        explicit GLAxis(GLWidget *glWidget, GLAxisType type = GL_AXIS_NONE, const QString &name = QString());

        //! Copy constructor.
        GLAxis(const GLAxis &glAxis);

        //! Destructor.
        ~GLAxis();

        //! Assignment operator.
        GLAxis & operator = (const GLAxis &glAxis);

        //! Return axis type.
        GLAxisType getType() const;

        //! Set axis type.
        void setType(GLAxisType type);

        //! Return axis size.
        float getSize() const;

        //! Set axis size.
        void setSize(float size);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();
};

#endif /* GL_AXIS_H */
