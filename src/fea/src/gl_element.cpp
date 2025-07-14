#include "gl_element.h"
#include "gl_texture.h"
#include "gl_simplex_point.h"
#include "gl_simplex_segment.h"
#include "gl_simplex_polygon.h"
#include "gl_simplex_tetrahedra.h"
#include "application.h"

void GLElement::_init(const GLElement *pGlElement)
{
    if (pGlElement)
    {
        this->pointVolume = pGlElement->pointVolume;
        this->lineCrossArea = pGlElement->lineCrossArea;
        this->surfaceThickness = pGlElement->surfaceThickness;
    }
}

GLElement::GLElement(GLWidget *glWidget, const Model *pModel, uint elementID, const REntityGroupData &elementGroupData, const QColor &color, GLElementDrawMode drawMode)
    : GLElementBase(glWidget,pModel,elementID,elementGroupData,color,drawMode)
    , RElement(pModel->getElement(elementID))
    , pointVolume(0.0)
    , lineCrossArea(0.0)
    , surfaceThickness(0.0)
{
    this->_init();
}

GLElement::GLElement(const GLElement &glElement)
    : GLElementBase(glElement)
    , RElement(glElement)
{
    this->_init(&glElement);
}

GLElement::~GLElement()
{
}

GLElement &GLElement::operator =(const GLElement &glElement)
{
    this->GLElementBase::operator =(glElement);
    this->RElement::operator =(glElement);
    this->_init(&glElement);
    return (*this);
}

void GLElement::setPointVolume(double pointVolume)
{
    this->pointVolume = pointVolume;
}

void GLElement::setLineCrossArea(double lineCrossArea)
{
    this->lineCrossArea = lineCrossArea;
}

void GLElement::setSurfaceThickness(double surfaceThickness)
{
    this->surfaceThickness = surfaceThickness;
}

void GLElement::draw()
{
    if (!this->pModel)
    {
        return;
    }
    switch (this->getType())
    {
        case R_ELEMENT_POINT:
            this->drawPoint();
            break;
        case R_ELEMENT_TRUSS1:
            this->drawLine();
            break;
        case R_ELEMENT_TRI1:
            this->drawTriangle();
            break;
        case R_ELEMENT_QUAD1:
            this->drawQuadrilateral();
            break;
        case R_ELEMENT_TETRA1:
            this->drawTetrahedra();
            break;
        default:
            break;
    }
}

void GLElement::drawPoint()
{
    RNode node1 = this->pModel->getNode(this->getNodeId(0));

    std::vector<RR3Vector> displacementValues;
    bool validDisplacementValues = (this->pDisplacementVariable != 0);
    if (validDisplacementValues)
    {
        this->findDisplacementNodeValues(this->elementID,*this->pDisplacementVariable,displacementValues);
    }

    if (validDisplacementValues && displacementValues.size() == this->size())
    {
        node1.move(displacementValues[0]);
    }

    std::vector<RR3Vector> nodes;
    nodes.push_back(node1.toVector());

    GLSimplexPoint point(this->getGLWidget(),nodes,this->pointVolume);

    int drawMask = 0;

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
            if (this->elementGroupData.getDrawNodeNumbers())
            {
                drawMask |= GLSimplex::NodeIds;
                point.setNodeIds(this->nodeIDs);
            }
            if (this->elementGroupData.getDrawElementNumbers())
            {
                drawMask |= GLSimplex::ElementId;
                point.setId(this->getElementID());
            }
            break;
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            if (this->pScalarVariable != 0)
            {
                std::vector<double> scalarValues;
                this->findScalarNodeValues(this->elementID,*this->pScalarVariable,scalarValues);
                point.setNodeTextureCoordinates(scalarValues);
            }
            if (this->elementGroupData.getDrawWire())
            {
                drawMask |= GLSimplex::Wired;
            }
            else
            {
                drawMask |= GLSimplex::Normal;
            }
            if (this->elementGroupData.getDrawEdges())
            {
                drawMask |= GLSimplex::ElementEdges;
            }
            if (this->elementGroupData.getDrawNodes())
            {
                drawMask |= GLSimplex::ElementNodes;
            }
            break;
        }
    }
    point.setDrawType(drawMask);
    point.setColor(this->color);
    point.paint();
}

void GLElement::drawLine()
{
    RNode node1 = this->pModel->getNode(this->getNodeId(0));
    RNode node2 = this->pModel->getNode(this->getNodeId(1));

    std::vector<RR3Vector> displacementValues;
    bool validDisplacementValues = (this->pDisplacementVariable != 0);
    if (validDisplacementValues)
    {
        this->findDisplacementNodeValues(this->elementID,*this->pDisplacementVariable,displacementValues);
    }

    if (validDisplacementValues && displacementValues.size() == this->size())
    {
        node1.move(displacementValues[0]);
        node2.move(displacementValues[1]);
    }

    std::vector<RR3Vector> nodes;
    nodes.push_back(node1.toVector());
    nodes.push_back(node2.toVector());

    GLSimplexSegment segment(this->getGLWidget(),nodes,this->lineCrossArea);

    int drawMask = 0;

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
            if (this->elementGroupData.getDrawNodeNumbers())
            {
                drawMask |= GLSimplex::NodeIds;
                segment.setNodeIds(this->nodeIDs);
            }
            if (this->elementGroupData.getDrawElementNumbers())
            {
                drawMask |= GLSimplex::ElementId;
                segment.setId(this->getElementID());
            }
            break;
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            if (this->pScalarVariable != 0)
            {
                std::vector<double> scalarValues;
                this->findScalarNodeValues(this->elementID,*this->pScalarVariable,scalarValues);
                segment.setNodeTextureCoordinates(scalarValues);
            }
            if (this->elementGroupData.getDrawWire())
            {
                drawMask |= GLSimplex::Wired;
            }
            else
            {
                drawMask |= GLSimplex::Normal;
            }
            if (this->elementGroupData.getDrawEdges())
            {
                drawMask |= GLSimplex::ElementEdges;
            }
            if (this->elementGroupData.getDrawNodes())
            {
                drawMask |= GLSimplex::ElementNodes;
            }
            break;
        }
    }
    segment.setDrawType(drawMask);
    segment.setColor(this->color);
    segment.paint();
}

void GLElement::drawTriangle()
{
    RNode node1 = this->pModel->getNode(this->getNodeId(0));
    RNode node2 = this->pModel->getNode(this->getNodeId(1));
    RNode node3 = this->pModel->getNode(this->getNodeId(2));

    std::vector<RR3Vector> displacementValues;
    bool validDisplacementValues = (this->pDisplacementVariable != 0);
    if (validDisplacementValues)
    {
        this->findDisplacementNodeValues(this->elementID,*this->pDisplacementVariable,displacementValues);
    }

    if (validDisplacementValues && displacementValues.size() == this->size())
    {
        node1.move(displacementValues[0]);
        node2.move(displacementValues[1]);
        node3.move(displacementValues[2]);
    }

    std::vector<RR3Vector> nodes;
    nodes.push_back(node1.toVector());
    nodes.push_back(node2.toVector());
    nodes.push_back(node3.toVector());

    GLSimplexPolygon polygon(this->getGLWidget(),nodes,this->surfaceThickness);
    polygon.setUseGlCullFace(this->getUseGlCullFace());

    int drawMask = 0;

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
            if (this->elementGroupData.getDrawNodeNumbers())
            {
                drawMask |= GLSimplex::NodeIds;
                polygon.setNodeIds(this->nodeIDs);
            }
            if (this->elementGroupData.getDrawElementNumbers())
            {
                drawMask |= GLSimplex::ElementId;
                polygon.setId(this->getElementID());
            }
            break;
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            if (this->pScalarVariable != 0)
            {
                std::vector<double> scalarValues;
                this->findScalarNodeValues(this->elementID,*this->pScalarVariable,scalarValues);
                polygon.setNodeTextureCoordinates(scalarValues);
            }
            if (this->elementGroupData.getDrawWire())
            {
                drawMask |= GLSimplex::Wired;
            }
            else
            {
                drawMask |= GLSimplex::Normal;
            }
            if (this->elementGroupData.getDrawEdges())
            {
                drawMask |= GLSimplex::ElementEdges;
            }
            if (this->elementGroupData.getDrawNodes())
            {
                drawMask |= GLSimplex::ElementNodes;
            }
            break;
        }
    }
    polygon.setDrawType(drawMask);
    polygon.setColor(this->color);
    polygon.paint();
}

void GLElement::drawQuadrilateral()
{
    RNode node1 = this->pModel->getNode(this->getNodeId(0));
    RNode node2 = this->pModel->getNode(this->getNodeId(1));
    RNode node3 = this->pModel->getNode(this->getNodeId(2));
    RNode node4 = this->pModel->getNode(this->getNodeId(3));

    std::vector<RR3Vector> displacementValues;
    bool validDisplacementValues = (this->pDisplacementVariable != 0);
    if (validDisplacementValues)
    {
        this->findDisplacementNodeValues(this->elementID,*this->pDisplacementVariable,displacementValues);
    }

    if (validDisplacementValues && displacementValues.size() == this->size())
    {
        node1.move(displacementValues[0]);
        node2.move(displacementValues[1]);
        node3.move(displacementValues[2]);
        node4.move(displacementValues[3]);
    }

    std::vector<RR3Vector> nodes;
    nodes.push_back(node1.toVector());
    nodes.push_back(node2.toVector());
    nodes.push_back(node3.toVector());
    nodes.push_back(node4.toVector());

    GLSimplexPolygon polygon(this->getGLWidget(),nodes,this->surfaceThickness);
    polygon.setUseGlCullFace(this->getUseGlCullFace());

    int drawMask = 0;

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
            if (this->elementGroupData.getDrawNodeNumbers())
            {
                drawMask |= GLSimplex::NodeIds;
                polygon.setNodeIds(this->nodeIDs);
            }
            if (this->elementGroupData.getDrawElementNumbers())
            {
                drawMask |= GLSimplex::ElementId;
                polygon.setId(this->getElementID());
            }
            break;
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            if (this->pScalarVariable != 0)
            {
                std::vector<double> scalarValues;
                this->findScalarNodeValues(this->elementID,*this->pScalarVariable,scalarValues);
                polygon.setNodeTextureCoordinates(scalarValues);
            }
            if (this->elementGroupData.getDrawWire())
            {
                drawMask |= GLSimplex::Wired;
            }
            else
            {
                drawMask |= GLSimplex::Normal;
            }
            if (this->elementGroupData.getDrawEdges())
            {
                drawMask |= GLSimplex::ElementEdges;
            }
            if (this->elementGroupData.getDrawNodes())
            {
                drawMask |= GLSimplex::ElementNodes;
            }
            break;
        }
    }
    polygon.setDrawType(drawMask);
    polygon.setColor(this->color);
    polygon.paint();
}

void GLElement::drawTetrahedra()
{
    RNode node1 = this->pModel->getNode(this->getNodeId(0));
    RNode node2 = this->pModel->getNode(this->getNodeId(1));
    RNode node3 = this->pModel->getNode(this->getNodeId(2));
    RNode node4 = this->pModel->getNode(this->getNodeId(3));

    std::vector<RR3Vector> displacementValues;
    bool validDisplacementValues = (this->pDisplacementVariable != 0);
    if (validDisplacementValues)
    {
        this->findDisplacementNodeValues(this->elementID,*this->pDisplacementVariable,displacementValues);
    }

    if (validDisplacementValues && displacementValues.size() == this->size())
    {
        node1.move(displacementValues[0]);
        node2.move(displacementValues[1]);
        node3.move(displacementValues[2]);
        node4.move(displacementValues[3]);
    }

    std::vector<RR3Vector> nodes;
    nodes.push_back(node1.toVector());
    nodes.push_back(node2.toVector());
    nodes.push_back(node3.toVector());
    nodes.push_back(node4.toVector());

    GLSimplexTetrahedra tetrahedra(this->getGLWidget(),nodes);
    std::vector<bool> edgeNodes(this->size());
    for (uint i=0;i<this->size();i++)
    {
        edgeNodes[i] = this->pModel->nodeIsOnEdge(this->getNodeId(i));
    }
    tetrahedra.setEdgeNodes(edgeNodes);

    int drawMask = 0;

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
            if (this->elementGroupData.getDrawNodeNumbers())
            {
                drawMask |= GLSimplex::NodeIds;
                tetrahedra.setNodeIds(this->nodeIDs);
            }
            if (this->elementGroupData.getDrawElementNumbers())
            {
                drawMask |= GLSimplex::ElementId;
                tetrahedra.setId(this->getElementID());
            }
            break;
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            if (this->pScalarVariable != 0)
            {
                std::vector<double> scalarValues;
                this->findScalarNodeValues(this->elementID,*this->pScalarVariable,scalarValues);
                tetrahedra.setNodeTextureCoordinates(scalarValues);
            }
            if (this->elementGroupData.getDrawWire())
            {
                drawMask |= GLSimplex::Wired;
            }
            else
            {
                drawMask |= GLSimplex::Normal;
            }
            if (this->elementGroupData.getDrawEdges())
            {
                drawMask |= GLSimplex::ElementEdges;
            }
            if (this->elementGroupData.getDrawNodes())
            {
                drawMask |= GLSimplex::ElementNodes;
            }
            break;
        }
    }
    tetrahedra.setDrawType(drawMask);
    tetrahedra.setColor(this->color);
    tetrahedra.paint();
}
