#ifndef GL_SIMPLEX_POINT_H
#define GL_SIMPLEX_POINT_H

#include "gl_simplex.h"

class GLSimplexPoint : public GLSimplex
{

    protected:

        //! Volume.
        double volume;

    private:

        //! Internal initialization function.
        void _init(const GLSimplexPoint *pGlPoint = nullptr );

    public:

        //! Constructor.
        explicit GLSimplexPoint(GLWidget *glWidget, const std::vector<RR3Vector> &nodes, double volume = 0.0);

        //! Copy constructor.
        GLSimplexPoint(const GLSimplexPoint &glPoint);

        //! Destructor.
        ~GLSimplexPoint();

        //! Assignment operator.
        GLSimplexPoint &operator =(const GLSimplexPoint &glPoint);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

        //! Draw normal.
        void drawNormal(const RModelRaw &sphereModel,
                        bool volumeElement,
                        bool useTexture);

        //! Draw wired.
        void drawWired(const RModelRaw &sphereModel,
                       bool volumeElement,
                       bool useTexture);

        //! Draw nodes.
        void drawNodes(const RModelRaw &, bool);

};

#endif // GL_SIMPLEX_POINT_H
