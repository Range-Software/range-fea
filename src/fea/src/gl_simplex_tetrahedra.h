#ifndef GL_SIMPLEX_TETRAHEDRA_H
#define GL_SIMPLEX_TETRAHEDRA_H

#include <vector>

#include "gl_simplex.h"

class GLSimplexTetrahedra : public GLSimplex
{

    private:

        //! Internal initialization function.
        void _init(const GLSimplexTetrahedra *pGlTetrahedra = nullptr );

    public:

        //! Constructor.
        explicit GLSimplexTetrahedra(GLWidget *glWidget, const std::vector<RR3Vector> &nodes);

        //! Copy constructor.
        GLSimplexTetrahedra(const GLSimplexTetrahedra &glTetrahedra);

        //! Destructor.
        ~GLSimplexTetrahedra();

        //! Assignment operator.
        GLSimplexTetrahedra &operator =(const GLSimplexTetrahedra &glTetrahedra);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

        //! Draw normal.
        void drawNormal(bool useTexture);

        //! Draw wired.
        void drawWired(bool useTexture);

        //! Draw nodes.
        void drawNodes();

};

#endif // GL_SIMPLEX_TETRAHEDRA_H
