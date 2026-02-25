#include "gl_functions.h"
#include "gl_simplex_segment.h"
#include "gl_state_cache.h"
#include "gl_widget.h"

void GLSimplexSegment::_init(const GLSimplexSegment *pGlSegment)
{
    if (pGlSegment)
    {
        this->crossArea = pGlSegment->crossArea;
    }
}

GLSimplexSegment::GLSimplexSegment(GLWidget *glWidget, const std::vector<RR3Vector> &nodes, double crossArea)
    : GLSimplex(glWidget,nodes)
    , crossArea(crossArea)
{
    this->_init();
}

GLSimplexSegment::GLSimplexSegment(const GLSimplexSegment &glSegment)
    : GLSimplex(glSegment)
{
    this->_init(&glSegment);
}

GLSimplexSegment::~GLSimplexSegment()
{

}

GLSimplexSegment &GLSimplexSegment::operator =(const GLSimplexSegment &glSegment)
{
    this->GLSimplex::operator =(glSegment);
    this->_init(&glSegment);
    return (*this);
}

void GLSimplexSegment::initialize()
{

}

void GLSimplexSegment::finalize()
{

}

void GLSimplexSegment::draw()
{
    uint nn = uint(this->nodes.size());
    uint nd = 0;

    // Currently only line with two nodes is supported
    if (nn != 2)
    {
        return;
    }

    bool volumeElement = (this->crossArea > RConstants::eps);
    bool useTexture = (this->nodeTextureCoordinates.size() == nn);

    if (volumeElement)
    {
        nd = 4*4;
    }

    std::vector<RNode> nodes1;
    std::vector<RNode> nodes2;

    if (volumeElement)
    {
        nodes1.resize(nd);
        nodes2.resize(nd);

        RR3Vector v1(this->nodes[0]);
        RR3Vector direction(this->nodes[1][0]-this->nodes[0][0],
                            this->nodes[1][1]-this->nodes[0][1],
                            this->nodes[1][2]-this->nodes[0][2]);
        double distance = direction.length();
        direction.normalize();

        RR3Vector orth1(direction.findOrthogonal());

        RR3Vector orth2;
        RR3Vector::cross(direction,orth1,orth2);

        double r = std::sqrt(this->crossArea / RConstants::pi);
        orth1 *= r;
        orth2 *= r;

        RR3Vector start, end;
        RRVector::add(v1,orth1,start);
        RRVector::add(v1,orth2,end);

        RRMatrix R;
        RTriangle(v1,start,end).findRotationMatrix(R);

        direction *= distance;

        double da = 2.0 * RConstants::pi / double(nd);
        for (uint i=0;i<nd;i++)
        {
            double x = std::sin(da*i);
            double y = std::cos(da*i);
            RR3Vector lp(x,y,0.0);
            lp *= r;

            RR3Vector gp1;
            RRMatrix::mlt(R,lp,gp1);
            RRVector::add(gp1,v1,gp1);

            RR3Vector gp2;
            RRVector::add(gp1,direction,gp2);

            nodes1[i] = RNode(gp1);
            nodes2[i] = RNode(gp2);
        }
    }

    if (this->drawTypeMask & GLSimplex::NodeIds)
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
    if (this->drawTypeMask & GLSimplex::ElementId)
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
//    if (drawTypeMask & GLSimplex::ElementEdges)
//    {
//        this->getGLWidget()->qglColor(Qt::black);
//        this->drawWired(nodes1,nodes2,volumeElement,useTexture);
//    }
    if (this->drawTypeMask & GLSimplex::ElementNodes)
    {
        this->getGLWidget()->qglColor(Qt::black);
        this->drawNodes();
    }
    if (this->drawTypeMask & GLSimplex::Wired)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawWired(nodes1,nodes2,volumeElement,useTexture);
    }
    if (this->drawTypeMask & GLSimplex::Normal)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawNormal(nodes1,nodes2,volumeElement,useTexture);
    }
}

void GLSimplexSegment::drawNormal(const std::vector<RNode> &nodes1, const std::vector<RNode> &nodes2, bool volumeElement, bool useTexture)
{
    GLboolean lightingEnabled = GLStateCache::instance().getLighting();

    if (useTexture)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
    }

    if (volumeElement)
    {
        uint nn = uint(nodes1.size());

        GLObject::glNormalVector(RTriangle(this->nodes[0],nodes1[0],nodes1[1]).getNormal());

        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        }
        GLFunctions::begin(GL_TRIANGLE_FAN);
        for (uint i=0;i<nn;i++)
        {
            GLObject::glVertexNode(nodes1[i]);
        }
        GLFunctions::end();

        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        }
        GLFunctions::begin(GL_TRIANGLE_FAN);
        for (uint i=0;i<nn;i++)
        {
            GLObject::glVertexNode(nodes2[i]);
        }
        GLFunctions::end();

        GLFunctions::begin(GL_TRIANGLES);
        for (uint i=0;i<nn;i++)
        {
            uint n1 = i;
            uint n2 = (i==nn-1) ? 0 : (i+1);

            GLObject::glNormalVector(RTriangle(nodes1[n1],nodes1[n2],nodes2[n2]).getNormal());

            // Triangle 1: nodes1[n1], nodes1[n2], nodes2[n2]
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
            GLObject::glVertexNode(nodes1[n1]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
            GLObject::glVertexNode(nodes1[n2]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
            GLObject::glVertexNode(nodes2[n2]);

            // Triangle 2: nodes1[n1], nodes2[n2], nodes2[n1]
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
            GLObject::glVertexNode(nodes1[n1]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
            GLObject::glVertexNode(nodes2[n2]);
            if (useTexture) GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
            GLObject::glVertexNode(nodes2[n1]);
        }
        GLFunctions::end();
    }
    else
    {
        GLStateCache::instance().disableLighting();

        GLFunctions::begin(GL_LINES);
        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        }
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        }
        GLObject::glVertexNode(this->nodes[1]);
        GLFunctions::end();
    }

    if (useTexture)
    {
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }

    GLStateCache::instance().setLighting(lightingEnabled);
}

void GLSimplexSegment::drawWired(const std::vector<RNode> &nodes1, const std::vector<RNode> &nodes2, bool volumeElement, bool useTexture)
{
    GLboolean lightingEnabled = GLStateCache::instance().getLighting();
    GLStateCache::instance().disableLighting();

    if (useTexture)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
    }

    if (volumeElement)
    {
        uint nn = uint(nodes1.size());

        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        }
        GLFunctions::begin(GL_LINES);
        for (uint i=0;i<nn;i++)
        {
            uint n1 = i;
            uint n2 = (i==nn-1) ? 0 : (i+1);

            GLObject::glVertexNode(this->nodes[0]);
            GLObject::glVertexNode(nodes1[n1]);

            GLObject::glVertexNode(nodes1[n1]);
            GLObject::glVertexNode(nodes1[n2]);
        }
        GLFunctions::end();

        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        }
        GLFunctions::begin(GL_LINES);
        for (uint i=0;i<nn;i++)
        {
            uint n1 = i;
            uint n2 = (i==nn-1) ? 0 : (i+1);

            GLObject::glVertexNode(this->nodes[1]);
            GLObject::glVertexNode(nodes2[n1]);

            GLObject::glVertexNode(nodes2[n1]);
            GLObject::glVertexNode(nodes2[n2]);
        }
        GLFunctions::end();

        GLFunctions::begin(GL_LINES);
        for (uint i=0;i<nn;i++)
        {
            if (useTexture)
            {
                GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
            }
            GLObject::glVertexNode(nodes1[i]);
            if (useTexture)
            {
                GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
            }
            GLObject::glVertexNode(nodes2[i]);
        }
        GLFunctions::end();
    }
    else
    {
        GLFunctions::begin(GL_LINES);
        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[0]));
        }
        GLObject::glVertexNode(this->nodes[0]);
        if (useTexture)
        {
            GLFunctions::texCoord1f(GLfloat(this->nodeTextureCoordinates[1]));
        }
        GLObject::glVertexNode(this->nodes[1]);
        GLFunctions::end();
    }

    if (useTexture)
    {
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }

    GLStateCache::instance().setLighting(lightingEnabled);
}

void GLSimplexSegment::drawNodes()
{
    GLFunctions::begin(GL_POINTS);
    GLObject::glVertexNode(this->nodes[0]);
    GLFunctions::end();
    GLFunctions::begin(GL_POINTS);
    GLObject::glVertexNode(this->nodes[1]);
    GLFunctions::end();
}
