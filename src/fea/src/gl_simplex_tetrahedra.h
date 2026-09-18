#ifndef GL_SIMPLEX_TETRAHEDRA_H
#define GL_SIMPLEX_TETRAHEDRA_H

#include <vector>

#include "gl_simplex.h"

class GLSimplexTetrahedra : public GLSimplex
{

    public:

        //! Mask with all four faces visible.
        static const uint AllFaces = 0xF;

    protected:

        //! Visible faces - bit i stands for the face opposite to node i.
        uint visibleFaces;

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

        //! Set visible faces - bit i stands for the face opposite to node i.
        void setVisibleFaces(uint visibleFaces);

        //! Return true if face opposite to given node is visible.
        bool faceIsVisible(uint oppositeNode) const;

        //! Return true if edge between given nodes is visible (at least one of its faces is).
        bool edgeIsVisible(uint node1, uint node2) const;

        //! Return true if node is visible (at least one of its faces is).
        bool nodeIsVisible(uint node) const;

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
