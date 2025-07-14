#ifndef GL_ARROW_H
#define GL_ARROW_H

#include <QtOpenGL>

#include <rbl_r3vector.h>

#include "gl_object.h"

class GLArrow : public GLObject
{

    private:

        // Environment settings
        GLboolean normalizeEnabled;
        GLboolean cullFaceEnabled;
        GLfloat lineWidth;

    protected:

        //! Vector position.
        RR3Vector position;
        //! Vector direction.
        RR3Vector direction;
        //! Draw tip.
        bool showHead;
        //! Draw from.
        bool drawFrom;
        //! Scale.
        double scale;

    private:

        //! Internal initialization function.
        void _init ( const GLArrow *pGlArrow = nullptr );

    public:

        //! Constructor.
        explicit GLArrow(GLWidget *glWidget, const RR3Vector &position, const RR3Vector &direction, bool showHead, bool drawFrom, double scale = 1.0);

        //! Copy constructor.
        GLArrow(const GLArrow &glArrow);

        //! Destructor.
        virtual ~GLArrow() {}

        //! Assignment operator.
        GLArrow & operator = (const GLArrow &glArrow);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

        //! Draw arrow shaft.
        void drawShaft() const;

        //! Draw arrow head.
        void drawHead();

};

#endif // GL_ARROW_H
