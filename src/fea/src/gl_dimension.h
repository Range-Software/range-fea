#ifndef GL_DIMENSION_H
#define GL_DIMENSION_H

#include "gl_object.h"

class GLDimension : public GLObject
{

    private:

        // Environment settings.

    public:

        static const double arrowScale;

    protected:

        double scale;
        double xMin;
        double xMax;
        double yMin;
        double yMax;
        double zMin;
        double zMax;

    private:

        //! Internal initialization function.
        void _init(const GLDimension *pGlDimension = nullptr);

    public:

        //! Constructor.
        explicit GLDimension(GLWidget *glWidget,
                             double scale,
                             double xMin,
                             double xMax,
                             double yMin,
                             double yMax,
                             double zMin,
                             double zMax);

        //! Copy constructor.
        GLDimension(const GLDimension &glDimension);

        //! Destructor.
        virtual ~GLDimension() {}

        //! Assignment operator.
        GLDimension &operator=(const GLDimension &glDimension);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif // GL_DIMENSION_H
