#include "gl_simplex.h"

void GLSimplex::_init(const GLSimplex *pGlSimplex)
{
    if (pGlSimplex)
    {
        this->nodes = pGlSimplex->nodes;
        this->edgeNodes = pGlSimplex->edgeNodes;
        this->nodeTextureCoordinates = pGlSimplex->nodeTextureCoordinates;
        this->nodeIds = pGlSimplex->nodeIds;
        this->id = pGlSimplex->id;
        this->color = pGlSimplex->color;
        this->drawTypeMask = pGlSimplex->drawTypeMask;
    }
    else
    {
        this->edgeNodes.resize(this->nodes.size());
        std::fill(this->edgeNodes.begin(),this->edgeNodes.end(),true);
    }
}

GLSimplex::GLSimplex(GLWidget *glWidget, const std::vector<RR3Vector> &nodes)
    : GLObject(glWidget)
    , nodes(nodes)
    , color(Qt::white)
    , drawTypeMask(GLSimplex::Normal)
{
    this->_init();
}

GLSimplex::GLSimplex(const GLSimplex &glSimplex)
    : GLObject(glSimplex)
{
    this->_init(&glSimplex);
}

GLSimplex::~GLSimplex()
{

}

GLSimplex &GLSimplex::operator =(const GLSimplex &glSimplex)
{
    this->GLObject::operator =(glSimplex);
    this->_init(&glSimplex);
    return (*this);
}

void GLSimplex::setEdgeNodes(const std::vector<bool> &edgeNodes)
{
    this->edgeNodes = edgeNodes;
}

void GLSimplex::setNodeTextureCoordinates(const std::vector<double> &nodeTextureCoordinates)
{
    this->nodeTextureCoordinates = nodeTextureCoordinates;
}

void GLSimplex::setNodeIds(const std::vector<uint> &nodeIds)
{
    this->nodeIds = nodeIds;
}

void GLSimplex::setId(uint id)
{
    this->id = id;
}

void GLSimplex::setColor(const QColor &color)
{
    this->color = color;
}

void GLSimplex::setDrawType(int drawTypeMask)
{
    this->drawTypeMask = drawTypeMask;
}
