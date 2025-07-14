#ifndef GL_SIMPLEX_SEGMENT_H
#define GL_SIMPLEX_SEGMENT_H

#include "gl_simplex.h"

class GLSimplexSegment : public GLSimplex
{

    protected:

        //! Cross area.
        double crossArea;

    private:

        //! Internal initialization function.
        void _init(const GLSimplexSegment *pGlSegment = nullptr );

    public:

        //! Constructor.
        explicit GLSimplexSegment(GLWidget *glWidget, const std::vector<RR3Vector> &nodes, double crossArea = 0.0);

        //! Copy constructor.
        GLSimplexSegment(const GLSimplexSegment &glSegment);

        //! Destructor.
        ~GLSimplexSegment();

        //! Assignment operator.
        GLSimplexSegment &operator =(const GLSimplexSegment &glSegment);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

        //! Draw normal.
        void drawNormal(const std::vector<RNode> &nodes1,
                        const std::vector<RNode> &nodes2,
                        bool volumeElement,
                        bool useTexture);

        //! Draw wired.
        void drawWired(const std::vector<RNode> &nodes1,
                       const std::vector<RNode> &nodes2,
                       bool volumeElement,
                       bool useTexture);

        //! Draw nodes.
        void drawNodes();

};

#endif // GL_SIMPLEX_SEGMENT_H
