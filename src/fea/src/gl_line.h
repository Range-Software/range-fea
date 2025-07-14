#ifndef GL_LINE_H
#define GL_LINE_H

#include <rbl_r3vector.h>

#include "gl_object.h"

class GLLine : public GLObject
{

    private:

        // Environment settings
        GLboolean normalizeEnabled;
        GLboolean lightingEnabled;
        GLfloat lineWidth;

    protected:

        //! Line start.
        RR3Vector start;
        //! Line end.
        RR3Vector end;
        //! Width;
        GLfloat width;

    private:

        //! Internal initialization function.
        void _init (const GLLine *pGlLine = nullptr );

    public:

        //! Constructor.
        explicit GLLine(GLWidget *glWidget, const RR3Vector &start, const RR3Vector &end, GLfloat width);

        //! Copy constructor.
        GLLine(const GLLine &glLine);

        //! Destructor.
        ~GLLine();

        //! Assignment operator.
        GLLine & operator =(const GLLine &glLine);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif // GL_LINE_H
