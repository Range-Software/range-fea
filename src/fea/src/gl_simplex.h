#ifndef GL_SIMPLEX_H
#define GL_SIMPLEX_H

#include <vector>

#include "gl_object.h"

class GLSimplex : public GLObject
{

    public:

        enum DrawType
        {
            NodeIds      = 1 << 0,
            ElementId    = 1 << 1,
            ElementEdges = 1 << 2,
            ElementNodes = 1 << 3,
            Wired        = 1 << 4,
            Normal       = 1 << 5
        };

    protected:

        //! Polygon nodes.
        std::vector<RR3Vector> nodes;
        //! Polygon nodes.
        std::vector<bool> edgeNodes;
        //! Polygon node texture coordinates.
        std::vector<double> nodeTextureCoordinates;
        //! Polygon node IDs.
        std::vector<uint> nodeIds;
        //! Polygon number.
        uint id;
        //! Polygon color.
        QColor color;
        //! Draw type.
        int drawTypeMask;

    private:

        //! Internal initialization function.
        void _init(const GLSimplex *pGlSimplex = nullptr );

    public:

        //! Constructor.
        explicit GLSimplex(GLWidget *glWidget, const std::vector<RR3Vector> &nodes);

        //! Copy constructor.
        GLSimplex(const GLSimplex &glSimplex);

        //! Destructor.
        ~GLSimplex();

        //! Assignment operator.
        GLSimplex &operator =(const GLSimplex &glSimplex);

        //! Set node texture coordinates.
        void setEdgeNodes(const std::vector<bool> &edgeNodes);

        //! Set node texture coordinates.
        void setNodeTextureCoordinates(const std::vector<double> &nodeTextureCoordinates);

        //! Set node texture coordinates.
        void setNodeIds(const std::vector<uint> &nodeIds);

        //! Set polygon ID.
        void setId(uint id);

        //! Set color.
        void setColor(const QColor &color);

        //! Set draw type.
        void setDrawType(int drawTypeMask);

};

#endif // GL_SIMPLEX_H
