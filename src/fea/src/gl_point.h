#ifndef GL_POINT_H
#define GL_POINT_H

#include <rbl_r3vector.h>

#include "gl_object.h"

class GLPoint : public GLObject
{

    private:

        // Environment settings
        GLfloat prevPointSize;
        GLboolean lightingEnabled;

    protected:

        //! Point position.
        RR3Vector position;
        //! Point size.
        float pointSize;

    private:

        //! Internal initialization function.
        void _init ( const GLPoint *pGlPoint = nullptr );

    public:

        //! Constructor.
        explicit GLPoint(GLWidget *glWidget, const RR3Vector &position, float pointSize);

        //! Copy constructor.
        GLPoint(const GLPoint &glPoint);

        //! Destructor.
        ~GLPoint();

        //! Assignment operator.
        GLPoint & operator = (const GLPoint &glPoint);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif // GL_POINT_H
