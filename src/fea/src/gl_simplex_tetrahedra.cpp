#include "gl_functions.h"
#include "gl_simplex_tetrahedra.h"
#include "gl_state_cache.h"
#include "gl_widget.h"

void GLSimplexTetrahedra::_init(const GLSimplexTetrahedra *pGlTetrahedra)
{
    if (pGlTetrahedra)
    {
        this->visibleFaces = pGlTetrahedra->visibleFaces;
    }
}

GLSimplexTetrahedra::GLSimplexTetrahedra(GLWidget *glWidget, const std::vector<RR3Vector> &nodes)
    : GLSimplex(glWidget,nodes)
    , visibleFaces(GLSimplexTetrahedra::AllFaces)
{
    this->_init();
}

GLSimplexTetrahedra::GLSimplexTetrahedra(const GLSimplexTetrahedra &glTetrahedra)
    : GLSimplex(glTetrahedra)
{
    this->_init(&glTetrahedra);
}

GLSimplexTetrahedra::~GLSimplexTetrahedra()
{

}

GLSimplexTetrahedra &GLSimplexTetrahedra::operator =(const GLSimplexTetrahedra &glTetrahedra)
{
    this->GLSimplex::operator =(glTetrahedra);
    this->_init(&glTetrahedra);
    return (*this);
}

void GLSimplexTetrahedra::setVisibleFaces(uint visibleFaces)
{
    this->visibleFaces = visibleFaces & GLSimplexTetrahedra::AllFaces;
}

bool GLSimplexTetrahedra::faceIsVisible(uint oppositeNode) const
{
    return (this->visibleFaces & (1u << oppositeNode)) != 0;
}

bool GLSimplexTetrahedra::edgeIsVisible(uint node1, uint node2) const
{
    // Edge is shared by the two faces opposite to the remaining two nodes.
    return (this->visibleFaces & ~((1u << node1) | (1u << node2))) != 0;
}

bool GLSimplexTetrahedra::nodeIsVisible(uint node) const
{
    // Node is shared by all faces except the one opposite to it.
    return (this->visibleFaces & ~(1u << node)) != 0;
}

namespace
{
    //! Corner nodes of each face, the face is opposite to the node missing in it.
    const uint faceNodes[4][3] = {{1,2,3},{0,3,2},{0,1,3},{0,2,1}};

    //! Position of the node in the middle of an edge of a second order tetrahedron
    //! (edges 0-1, 1-2, 2-0, 0-3, 1-3, 2-3).
    uint findMidNode(uint node1, uint node2)
    {
        static const uint edges[6][2] = {{0,1},{1,2},{2,0},{0,3},{1,3},{2,3}};
        for (uint i=0;i<6;i++)
        {
            if ((edges[i][0] == node1 && edges[i][1] == node2) || (edges[i][0] == node2 && edges[i][1] == node1))
            {
                return 4 + i;
            }
        }
        return 0;
    }
}

void GLSimplexTetrahedra::initialize()
{

}

void GLSimplexTetrahedra::finalize()
{

}

void GLSimplexTetrahedra::draw()
{
    uint nn = uint(this->nodes.size());

    if (nn < 4)
    {
        return;
    }

    bool useTexture = (this->nodeTextureCoordinates.size() == nn);

    if (drawTypeMask & GLSimplex::NodeIds)
    {
        this->getGLWidget()->qglColor(QColor(Qt::black));

        for (uint i=0;i<nn;i++)
        {
            this->getGLWidget()->renderText(this->nodes[i][0],
                                            this->nodes[i][1],
                                            this->nodes[i][2],
                                            QLocale().toString(this->nodeIds[i]),
                                            QFont("Courier",8));
        }
    }
    if (drawTypeMask & GLSimplex::ElementId)
    {
        this->getGLWidget()->qglColor(QColor(Qt::black));

        double x = 0.0, y = 0.0, z = 0.0;

        for (uint i=0;i<nn;i++)
        {
            x += this->nodes[i][0];
            y += this->nodes[i][1];
            z += this->nodes[i][2];
        }

        x /= double(nn);
        y /= double(nn);
        z /= double(nn);

        this->getGLWidget()->renderText(x,y,z,QLocale().toString(this->id),QFont("Courier",8));
    }
    if (drawTypeMask & GLSimplex::ElementEdges)
    {
        this->getGLWidget()->qglColor(Qt::black);
        GLFunctions::texCoord1f(-1.0f); // sentinel: use vColor in shader, not the colormap
        this->drawWired(false); // never textured - edges are always black
    }
    if (drawTypeMask & GLSimplex::ElementNodes)
    {
        this->getGLWidget()->qglColor(Qt::black);
        GLFunctions::texCoord1f(-1.0f); // sentinel: use vColor in shader, not the colormap
        this->drawNodes();
    }
    if (drawTypeMask & GLSimplex::Wired)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawWired(useTexture);
    }
    if (drawTypeMask & GLSimplex::Normal)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawNormal(useTexture);
    }
}

void GLSimplexTetrahedra::drawNormal(bool useTexture)
{
    if (this->visibleFaces == 0)
    {
        return;
    }
    if (useTexture)
    {
        GLStateCache::instance().enableTexture1D();
    }

    GLboolean cullState;

    cullState = GLStateCache::instance().getCullFace();
    GLStateCache::instance().setCullFace(GL_FALSE);

    if (this->nodes.size() == 10)
    {
        // Second order tetrahedron - each face is split into four triangles by the nodes in the
        // middle of its edges, so that values there are shown.
        GLFunctions::begin(GL_TRIANGLES);
        for (uint face=0;face<4;face++)
        {
            if (!this->faceIsVisible(face))
            {
                continue;
            }
            const uint a = faceNodes[face][0], b = faceNodes[face][1], c = faceNodes[face][2];
            const uint ab = findMidNode(a,b), bc = findMidNode(b,c), ca = findMidNode(c,a);
            const uint triangles[4][3] = {{a,ab,ca},{ab,b,bc},{ca,bc,c},{ab,bc,ca}};
            GLObject::glNormalVector(RR3Vector(RTriangle(RNode(this->nodes[a]),RNode(this->nodes[b]),RNode(this->nodes[c])).getNormal()));
            for (const auto &triangle : triangles)
            {
                for (uint node : triangle)
                {
                    if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[node]));
                    GLObject::glVertexNode(this->nodes[node]);
                }
            }
        }
        GLFunctions::end();

        if (useTexture)
        {
            GLStateCache::instance().disableTexture1D();
        }
        GLStateCache::instance().setCullFace(cullState);
        return;
    }

    GLFunctions::begin(GL_TRIANGLES);
    // 021
    if (this->faceIsVisible(3))
    {
        GLObject::glNormalVector(RR3Vector(RTriangle(RNode(this->nodes[0]),RNode(this->nodes[2]),RNode(this->nodes[1])).getNormal()));
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
    }
    // 013
    if (this->faceIsVisible(2))
    {
        GLObject::glNormalVector(RR3Vector(RTriangle(RNode(this->nodes[0]),RNode(this->nodes[1]),RNode(this->nodes[3])).getNormal()));
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
    }
    // 032
    if (this->faceIsVisible(1))
    {
        GLObject::glNormalVector(RR3Vector(RTriangle(RNode(this->nodes[0]),RNode(this->nodes[3]),RNode(this->nodes[2])).getNormal()));
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
    }
    // 123
    if (this->faceIsVisible(0))
    {
        GLObject::glNormalVector(RR3Vector(RTriangle(RNode(this->nodes[1]),RNode(this->nodes[2]),RNode(this->nodes[3])).getNormal()));
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
    }
    GLFunctions::end();

    if (useTexture)
    {
        GLStateCache::instance().disableTexture1D();
    }

    GLStateCache::instance().setCullFace(cullState);
}

void GLSimplexTetrahedra::drawWired(bool useTexture)
{
    if (this->visibleFaces == 0)
    {
        return;
    }
    if (useTexture)
    {
        GLStateCache::instance().enableTexture1D();
    }

    if (this->nodes.size() == 10)
    {
        // Second order tetrahedron - each edge goes through the node in its middle.
        static const uint edges[6][2] = {{0,1},{1,2},{2,0},{0,3},{1,3},{2,3}};
        GLFunctions::begin(GL_LINES);
        for (uint i=0;i<6;i++)
        {
            if (!this->edgeIsVisible(edges[i][0],edges[i][1]))
            {
                continue;
            }
            const uint path[3] = {edges[i][0],4+i,edges[i][1]};
            for (uint j=0;j<2;j++)
            {
                for (uint k=j;k<j+2;k++)
                {
                    if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[path[k]]));
                    GLObject::glVertexNode(this->nodes[path[k]]);
                }
            }
        }
        GLFunctions::end();

        if (useTexture)
        {
            GLStateCache::instance().disableTexture1D();
        }
        return;
    }

    GLFunctions::begin(GL_LINES);
    // 01
    if (this->edgeIsVisible(0,1))
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
    }
    // 12
    if (this->edgeIsVisible(1,2))
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
    }
    // 20
    if (this->edgeIsVisible(2,0))
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
    }
    // 30
    if (this->edgeIsVisible(3,0))
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
    }
    // 31
    if (this->edgeIsVisible(3,1))
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
    }
    // 32
    if (this->edgeIsVisible(3,2))
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
    }
    GLFunctions::end();

    if (useTexture)
    {
        GLStateCache::instance().disableTexture1D();
    }
}

void GLSimplexTetrahedra::drawNodes()
{
    uint nn = uint(this->nodes.size());

    static const uint edges[6][2] = {{0,1},{1,2},{2,0},{0,3},{1,3},{2,3}};
    for (uint i=0;i<nn;i++)
    {
        // A node in the middle of an edge is visible with its edge.
        const bool visible = i < 4 ? this->nodeIsVisible(i) : (i < 10 && this->edgeIsVisible(edges[i-4][0],edges[i-4][1]));
        if (visible)
        {
            GLFunctions::begin(GL_POINTS);
            GLObject::glVertexNode(this->nodes[i]);
            GLFunctions::end();
        }
    }
}
