#ifndef GL_ROTATION_SPHERE_H
#define GL_ROTATION_SPHERE_H

#include "gl_object.h"

class GLRotationSphere : public GLObject
{

    private:

        // Environment settings
        GLboolean depthTestEnabled;
        GLboolean lineSmoothEnabled;
        GLint lineSmoothHint;
        GLboolean normalizeEnabled;
        GLboolean lightingEnabled;
        GLfloat lineWidth;
        GLboolean cullFaceEnabled;

    protected:

        //! Position.
        RR3Vector position;
        //! Scale.
        double scale;

    private:

        //! Internal initialization function.
        void _init(const GLRotationSphere *pGlRotationSphere = nullptr);

    public:

        //! Constructor.
        explicit GLRotationSphere(GLWidget *glWidget, const RR3Vector &position, double scale = 1.0);

        //! Copy constructor.
        GLRotationSphere(const GLRotationSphere &glRotationSphere);

        //! Destructor.
        ~GLRotationSphere();

        //! Assignment operator.
        GLRotationSphere &operator =(const GLRotationSphere &glRotationSphere);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif // GL_ROTATION_SPHERE_H
