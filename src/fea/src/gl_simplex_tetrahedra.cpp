#include "gl_functions.h"
#include "gl_simplex_tetrahedra.h"
#include "gl_widget.h"

void GLSimplexTetrahedra::_init(const GLSimplexTetrahedra *pGlTetrahedra)
{
    if (pGlTetrahedra)
    {

    }
}

GLSimplexTetrahedra::GLSimplexTetrahedra(GLWidget *glWidget, const std::vector<RR3Vector> &nodes)
    : GLSimplex(glWidget,nodes)
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
        this->drawWired(useTexture);
    }
    if (drawTypeMask & GLSimplex::ElementNodes)
    {
        this->getGLWidget()->qglColor(Qt::black);
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
    if (!this->edgeNodes[0] && !this->edgeNodes[1] && !this->edgeNodes[2] && !this->edgeNodes[3])
    {
        return;
    }
    if (useTexture)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
    }

    GLboolean cullState;

    GL_SAFE_CALL(glGetBooleanv(GL_CULL_FACE,&cullState));
    GL_SAFE_CALL(glDisable(GL_CULL_FACE));

    GLFunctions::begin(GL_TRIANGLES);
    // 021
    if (this->edgeNodes[0] && this->edgeNodes[2] && this->edgeNodes[1])
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
    if (this->edgeNodes[0] && this->edgeNodes[1] && this->edgeNodes[3])
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
    if (this->edgeNodes[0] && this->edgeNodes[3] && this->edgeNodes[2])
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
    if (this->edgeNodes[1] && this->edgeNodes[2] && this->edgeNodes[3])
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
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }

    GL_SAFE_CALL(cullState ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
}

void GLSimplexTetrahedra::drawWired(bool useTexture)
{
    if (!this->edgeNodes[0] && !this->edgeNodes[1] && !this->edgeNodes[2] && !this->edgeNodes[3])
    {
        return;
    }
    if (useTexture)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
    }

    GLFunctions::begin(GL_LINES);
    // 01
    if (this->edgeNodes[0] && this->edgeNodes[1])
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
    }
    // 12
    if (this->edgeNodes[1] && this->edgeNodes[2])
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
    }
    // 20
    if (this->edgeNodes[2] && this->edgeNodes[0])
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
    }
    // 30
    if (this->edgeNodes[3] && this->edgeNodes[0])
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        GLObject::glVertexNode(this->nodes[0]);
    }
    // 31
    if (this->edgeNodes[3] && this->edgeNodes[1])
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        GLObject::glVertexNode(this->nodes[1]);
    }
    // 32
    if (this->edgeNodes[3] && this->edgeNodes[2])
    {
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[3]));
        GLObject::glVertexNode(this->nodes[3]);
        if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[2]));
        GLObject::glVertexNode(this->nodes[2]);
    }
    GLFunctions::end();

    if (useTexture)
    {
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }
}

void GLSimplexTetrahedra::drawNodes()
{
    uint nn = uint(this->nodes.size());

    for (uint i=0;i<nn;i++)
    {
        if (this->edgeNodes[i])
        {
            GLFunctions::begin(GL_POINTS);
            GLObject::glVertexNode(this->nodes[i]);
            GLFunctions::end();
        }
    }
}
