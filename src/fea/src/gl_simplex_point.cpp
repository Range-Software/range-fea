#include "gl_functions.h"
#include "gl_simplex_point.h"
#include "draw_engine_sphere.h"
#include "gl_widget.h"

void GLSimplexPoint::_init(const GLSimplexPoint *pGlPoint)
{
    if (pGlPoint)
    {
        this->volume = pGlPoint->volume;
    }
}

GLSimplexPoint::GLSimplexPoint(GLWidget *glWidget, const std::vector<RR3Vector> &nodes, double volume)
    : GLSimplex(glWidget,nodes)
    , volume(volume)
{
    this->_init();
}

GLSimplexPoint::GLSimplexPoint(const GLSimplexPoint &glPoint)
    : GLSimplex(glPoint)
{
    this->_init(&glPoint);
}

GLSimplexPoint::~GLSimplexPoint()
{

}

GLSimplexPoint &GLSimplexPoint::operator =(const GLSimplexPoint &glPoint)
{
    this->GLSimplex::operator =(glPoint);
    this->_init(&glPoint);
    return (*this);
}

void GLSimplexPoint::initialize()
{

}

void GLSimplexPoint::finalize()
{

}

void GLSimplexPoint::draw()
{
    RModelRaw sphereModel;

    bool volumeElement = (this->volume > RConstants::eps);
    bool useTexture = (this->nodeTextureCoordinates.size() == 1);

    if (volumeElement)
    {
        double r = std::pow((3.0/4.0) * this->volume / RConstants::pi,1.0/3.0);
        sphereModel = DrawEngineSphere::generate(this->nodes[0],r,2);
    }

    if (drawTypeMask & GLSimplex::NodeIds)
    {
        this->getGLWidget()->qglColor(QColor(Qt::black));

        this->getGLWidget()->renderText(this->nodes[0][0],
                                        this->nodes[0][1],
                                        this->nodes[0][2],
                                        QLocale().toString(this->nodeIds[0]),
                                        QFont("Courier",8));
    }
    if (drawTypeMask & GLSimplex::ElementId)
    {
        this->getGLWidget()->qglColor(QColor(Qt::black));

        this->getGLWidget()->renderText(this->nodes[0][0],
                                        this->nodes[0][1],
                                        this->nodes[0][2],
                                        QLocale().toString(this->id),
                                        QFont("Courier",8));
    }
//    if (drawTypeMask & GLSimplex::ElementEdges)
//    {
//        this->getGLWidget()->qglColor(Qt::black);
//        this->drawWired(sphereModel,volumeElement,useTexture);
//    }
    if (drawTypeMask & GLSimplex::ElementNodes)
    {
        this->getGLWidget()->qglColor(Qt::black);
        this->drawNodes(sphereModel,volumeElement);
    }
    if (drawTypeMask & GLSimplex::Wired)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawWired(sphereModel,volumeElement,useTexture);
    }
    if (drawTypeMask & GLSimplex::Normal)
    {
        this->getGLWidget()->qglColor(useTexture ? Qt::white : this->color);
        this->drawNormal(sphereModel,volumeElement,useTexture);
    }
}

void GLSimplexPoint::drawNormal(const RModelRaw &sphereModel, bool volumeElement, bool useTexture)
{
    GLboolean lightingEnabled;
    GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));

    if (useTexture)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
        GL_SAFE_CALL(glTexCoord1d(this->nodeTextureCoordinates[0]));
    }

    if (volumeElement)
    {
        GLFunctions::begin(GL_TRIANGLES);
        for (uint i=0;i<sphereModel.getNElements();i++)
        {
            const RElement &rElement = sphereModel.getElement(i);
            if (rElement.getType() != R_ELEMENT_TRI1 && rElement.getType() != R_ELEMENT_TRI2)
            {
                continue;
            }
            RNode n1(sphereModel.getNode(rElement.getNodeId(0)));
            RNode n2(sphereModel.getNode(rElement.getNodeId(1)));
            RNode n3(sphereModel.getNode(rElement.getNodeId(2)));
            GLObject::glNormalVector(RTriangle(n1,n2,n3).getNormal());
            GLObject::glVertexNode(n1);
            GLObject::glVertexNode(n2);
            GLObject::glVertexNode(n3);
        }
        GLFunctions::end();
    }
    else
    {
        GL_SAFE_CALL(glDisable(GL_LIGHTING));

        GLFunctions::begin(GL_POINTS);
        GLObject::glVertexNode(this->nodes[0]);
        GLFunctions::end();
    }

    if (useTexture)
    {
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }

    GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
}

void GLSimplexPoint::drawWired(const RModelRaw &sphereModel, bool volumeElement, bool useTexture)
{
    GLboolean lightingEnabled;
    GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));

    GL_SAFE_CALL(glDisable(GL_LIGHTING));

    if (useTexture)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
        GL_SAFE_CALL(glTexCoord1d(this->nodeTextureCoordinates[0]));
    }

    if (volumeElement)
    {
        for (uint i=0;i<sphereModel.getNElements();i++)
        {
            const RElement &rElement = sphereModel.getElement(i);
            if (rElement.getType() != R_ELEMENT_TRI1 && rElement.getType() != R_ELEMENT_TRI2)
            {
                continue;
            }
            GLFunctions::begin(GL_LINE_LOOP);
            GLObject::glVertexNode(sphereModel.getNode(rElement.getNodeId(0)));
            GLObject::glVertexNode(sphereModel.getNode(rElement.getNodeId(1)));
            GLObject::glVertexNode(sphereModel.getNode(rElement.getNodeId(2)));
            GLFunctions::end();
        }
    }
    else
    {
        GL_SAFE_CALL(glDisable(GL_LIGHTING));

        GLFunctions::begin(GL_POINTS);
        GLObject::glVertexNode(this->nodes[0]);
        GLFunctions::end();
    }

    if (useTexture)
    {
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }

    GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
}

void GLSimplexPoint::drawNodes(const RModelRaw &, bool)
{
    GLFunctions::begin(GL_POINTS);
    GLObject::glVertexNode(this->nodes[0]);
    GLFunctions::end();
}
