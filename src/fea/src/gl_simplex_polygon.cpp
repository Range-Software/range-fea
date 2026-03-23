#include <rml_polygon.h>

#include "gl_functions.h"
#include "gl_simplex_polygon.h"
#include "gl_state_cache.h"
#include "gl_widget.h"

void GLSimplexPolygon::_init(const GLSimplexPolygon *pGlPolygon)
{
    if (pGlPolygon)
    {
        this->thickness = pGlPolygon->thickness;
    }
}

GLSimplexPolygon::GLSimplexPolygon(GLWidget *glWidget, const std::vector<RR3Vector> &nodes, double thickness)
    : GLSimplex(glWidget,nodes)
    , thickness(thickness)
{
    this->_init();
}

GLSimplexPolygon::GLSimplexPolygon(const GLSimplexPolygon &glPolygon)
    : GLSimplex(glPolygon)
{
    this->_init(&glPolygon);
}

GLSimplexPolygon::~GLSimplexPolygon()
{

}

GLSimplexPolygon &GLSimplexPolygon::operator =(const GLSimplexPolygon &glPolygon)
{
    this->GLSimplex::operator =(glPolygon);
    this->_init(&glPolygon);
    return (*this);
}

void GLSimplexPolygon::initialize()
{

}

void GLSimplexPolygon::finalize()
{

}

void GLSimplexPolygon::draw()
{
    uint nn = uint(this->nodes.size());

    if (nn < 3)
    {
        return;
    }

    bool volumeElement = (this->thickness > RConstants::eps);
    bool useTexture = (this->nodeTextureCoordinates.size() == nn);

    std::vector<RNode> nodes1;
    std::vector<RNode> nodes2;

    nodes1.resize(nn);
    nodes2.resize(nn);

    for (uint i=0;i<nn;i++)
    {
        nodes1[i] = RNode(this->nodes[i]);
        nodes2[i] = RNode(this->nodes[i]);
    }

    if (volumeElement)
    {
        RR3Vector offset(0.0,0.0,0.0);
        offset = RPolygon(nodes1).getNormal();
        offset *= this->thickness;

        for (uint i=0;i<nn;i++)
        {
            nodes1[i].move(offset);
        }

        offset *= -1.0;

        for (uint i=0;i<nn;i++)
        {
            nodes2[i].move(offset);
        }
    }

    if (drawTypeMask & GLSimplex::NodeIds)
    {
        this->getGLWidget()->qglColor(QColor(Qt::black));

        for (uint i=0;i<nn;i++)
        {
            this->getGLWidget()->renderText(nodes1[i].getX(),
                                            nodes1[i].getY(),
                                            nodes1[i].getZ(),
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
            x += nodes1[i].getX();
            y += nodes1[i].getY();
            z += nodes1[i].getZ();
        }

        x /= double(nn);
        y /= double(nn);
        z /= double(nn);

        this->getGLWidget()->renderText(x,y,z,QLocale().toString(this->id),QFont("Courier",8));
    }
    // Draw filled faces first so edges recorded afterwards render on top with GL_LEQUAL.
    if (drawTypeMask & GLSimplex::Wired)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawWired(nodes1,nodes2,volumeElement,useTexture);
    }
    if (drawTypeMask & GLSimplex::Normal)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawNormal(nodes1,nodes2,volumeElement,useTexture);
    }
    if (drawTypeMask & GLSimplex::ElementEdges)
    {
        this->getGLWidget()->qglColor(Qt::black);
        GLFunctions::texCoord1f(-1.0f); // sentinel: use vColor in shader, not the colormap
        this->drawWired(nodes1,nodes2,volumeElement,false); // never textured — edges are always black
    }
    if (drawTypeMask & GLSimplex::ElementNodes)
    {
        this->getGLWidget()->qglColor(Qt::black);
        GLFunctions::texCoord1f(-1.0f); // sentinel: use vColor in shader, not the colormap
        this->drawNodes(nodes1,nodes2,volumeElement);
    }
}

void GLSimplexPolygon::drawNormal(const std::vector<RNode> &nodes1, const std::vector<RNode> &nodes2, bool volumeElement, bool useTexture)
{
    uint nn = uint(nodes1.size());

    RR3Vector normal(RPolygon(nodes1).getNormal());

    GLObject::glNormalVector(normal);

    GLStateCache &cache = GLStateCache::instance();

    if (useTexture)
    {
        cache.enableTexture1D();
    }

    // In immediate mode (not recording into a VBO) set uTwoSided so the shader
    // colours back faces correctly.  During VBO recording the uniform cannot be
    // changed per-element; GLElementGroup::paint() sets it at callList() time.
    if (!GLFunctions::isRecordingVBO())
    {
        this->getGLWidget()->getMainShaderProgram().setUniformBool("uTwoSided", this->twoSidedFace);
    }

    GLFunctions::begin(GL_TRIANGLE_FAN);
    for (uint i=0;i<nn;i++)
    {
        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[i]));
        }
        GLObject::glVertexNode(nodes1[i]);
    }
    GLFunctions::end();

    if (volumeElement)
    {
        GLFunctions::begin(GL_TRIANGLES);

        for (uint i=0;i<nn;i++)
        {
            uint n1 = i;
            uint n2 = (i + 1 == nn) ? 0 : i + 1;

            GLObject::glNormalVector(RTriangle(nodes1[n2],nodes1[n1],nodes2[n1]).getNormal());

            // Triangle 1: nodes1[n2], nodes1[n1], nodes2[n1]
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[n2]));
            GLObject::glVertexNode(nodes1[n2]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[n1]));
            GLObject::glVertexNode(nodes1[n1]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[n1]));
            GLObject::glVertexNode(nodes2[n1]);

            // Triangle 2: nodes1[n2], nodes2[n1], nodes2[n2]
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[n2]));
            GLObject::glVertexNode(nodes1[n2]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[n1]));
            GLObject::glVertexNode(nodes2[n1]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[n2]));
            GLObject::glVertexNode(nodes2[n2]);
        }

        GLFunctions::end();
    }

    if (useTexture)
    {
        cache.disableTexture1D();
    }

    if (!GLFunctions::isRecordingVBO())
    {
        this->getGLWidget()->getMainShaderProgram().setUniformBool("uTwoSided", false);
    }
}

void GLSimplexPolygon::drawWired(const std::vector<RNode> &nodes1, const std::vector<RNode> &nodes2, bool volumeElement, bool useTexture)
{
    uint nn = uint(nodes1.size());

    RR3Vector normal(RPolygon(nodes1).getNormal());

    GLObject::glNormalVector(normal);

    GLStateCache &cache = GLStateCache::instance();

    if (useTexture)
    {
        cache.enableTexture1D();
    }

    GLFunctions::begin(GL_LINE_LOOP);
    for (uint i=0;i<nn;i++)
    {
        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[i]));
        }
        GLObject::glVertexNode(nodes1[i]);
    }
    GLFunctions::end();

    if (volumeElement)
    {
        GLFunctions::begin(GL_LINE_LOOP);
        for (uint i=0;i<nn;i++)
        {
            if (useTexture)
            {
                GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[i]));
            }
            GLObject::glVertexNode(nodes2[i]);
        }
        GLFunctions::end();

        GLFunctions::begin(GL_LINES);
        for (uint i=0;i<nn;i++)
        {
            if (useTexture)
            {
                GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[i]));
            }
            GLObject::glVertexNode(nodes1[i]);
            GLObject::glVertexNode(nodes2[i]);
        }
        GLFunctions::end();
    }

    if (useTexture)
    {
        cache.disableTexture1D();
    }
}

void GLSimplexPolygon::drawNodes(const std::vector<RNode> &nodes1, const std::vector<RNode> &nodes2, bool volumeElement)
{
    uint nn = uint(nodes1.size());

    for (uint i=0;i<nn;i++)
    {
        GLFunctions::begin(GL_POINTS);
        GLObject::glVertexNode(nodes1[i]);
        if (volumeElement)
        {
            GLObject::glVertexNode(nodes2[i]);
        }
        GLFunctions::end();
    }
}
