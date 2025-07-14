#include "gl_functions.h"
#include "gl_interpolated_element.h"
#include "gl_widget.h"

void GLInterpolatedElement::_init(const GLInterpolatedElement *pGlElement)
{
    if (pGlElement)
    {

    }
}

GLInterpolatedElement::GLInterpolatedElement(GLWidget *glWidget,
                                             const Model *pModel,
                                             const RInterpolatedElement &iElement,
                                             uint elementID,
                                             const REntityGroupData &elementGroupData,
                                             GLElementDrawMode drawMode)
    : GLElementBase(glWidget,pModel,elementID,elementGroupData,Qt::white,drawMode)
    , RInterpolatedElement(iElement)
{
    this->_init();
}

GLInterpolatedElement::GLInterpolatedElement(const GLInterpolatedElement &glElement)
    : GLElementBase(glElement)
    , RInterpolatedElement(glElement)
{
    this->_init(&glElement);
}

GLInterpolatedElement::~GLInterpolatedElement()
{
}

GLInterpolatedElement &GLInterpolatedElement::operator =(const GLInterpolatedElement &glElement)
{
    this->GLElementBase::operator =(glElement);
    this->RInterpolatedElement::operator =(glElement);
    this->_init(&glElement);
    return (*this);
}

void GLInterpolatedElement::draw()
{
    if (!this->pModel)
    {
        return;
    }
    switch (this->drawMode)
    {
        case GL_ELEMENT_DRAW_NORMAL:
            this->drawNormal();
            break;
        case GL_ELEMENT_DRAW_TEXT:
            break;
        default:
            break;
    }
}

void GLInterpolatedElement::drawNormal()
{
    std::vector<double> scalarValues;
    std::vector<RR3Vector> displacementValues;

    bool validScalarValues = (this->pScalarVariable != 0);
    bool validDisplacementValues = (this->pDisplacementVariable != 0);

    if (validScalarValues)
    {
        this->findScalarNodeValues(this->pModel->getNodes(),this->pModel->getElements(),*this->pScalarVariable,scalarValues);
    }

    if (validDisplacementValues)
    {
        this->findDisplacementNodeValues(this->pModel->getNodes(),this->pModel->getElements(),*this->pDisplacementVariable,displacementValues);
    }

    std::vector<RInterpolatedNode> nodes;
    nodes.resize(this->size());

    for (uint i=0;i<this->size();i++)
    {
        nodes[i] = this->at(i);
    }

    if (validDisplacementValues && this->size() == displacementValues.size())
    {
        for (uint i=0;i<this->size();i++)
        {
            nodes[i].move(displacementValues[i]);
        }
    }

    int r = 255, g = 255, b = 255, a = 255;
    if (validScalarValues)
    {
        GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
    }
    else
    {
        this->getElementGroupData().getColor(r,g,b,a);
    }
    this->getGLWidget()->qglColor(QColor(r,g,b,a));

    if (this->size() == 1)
    {
        // Points.
        GLboolean lightingEnabled;
        GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));
        GL_SAFE_CALL(glDisable(GL_LIGHTING));
        GLFunctions::begin(GL_LINE_LOOP);
        for (uint j=0;j<this->size();j++)
        {
            if (validScalarValues)
            {
                GL_SAFE_CALL(glTexCoord1d(scalarValues[j]));
            }
            GL_SAFE_CALL(glVertex3d(nodes[j].getX(),nodes[j].getY(),nodes[j].getZ()));
        }
        GLFunctions::end();
        GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));

    }
    else if (this->size() == 2)
    {
        // Lines.
        GLboolean lightingEnabled;
        GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));
        GL_SAFE_CALL(glDisable(GL_LIGHTING));
        GLFunctions::begin(GL_LINE_STRIP);
        for (uint j=0;j<this->size();j++)
        {
            if (validScalarValues)
            {
                GL_SAFE_CALL(glTexCoord1d(scalarValues[j]));
            }
            GL_SAFE_CALL(glVertex3d(nodes[j].getX(),nodes[j].getY(),nodes[j].getZ()));
        }
        GLFunctions::end();
        GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    }
    else if (this->size() > 2)
    {
        // Polygons.
        if (this->getElementGroupData().getDrawWire())
        {
            GLboolean lightingEnabled;
            GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));
            GL_SAFE_CALL(glDisable(GL_LIGHTING));
            GLFunctions::begin(GL_LINE_LOOP);
            for (uint j=0;j<this->size();j++)
            {
                if (validScalarValues)
                {
                    GL_SAFE_CALL(glTexCoord1d(scalarValues[j]));
                }
                GL_SAFE_CALL(glVertex3d(nodes[j].getX(),nodes[j].getY(),nodes[j].getZ()));
            }
            GLFunctions::end();
            GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
        }
        else
        {
            RTriangle triangle(this->at(0),this->at(1),this->at(2));
            GL_SAFE_CALL(glNormal3d(triangle.getNormal()[0],triangle.getNormal()[1],triangle.getNormal()[2]));

            GLFunctions::begin(GL_POLYGON);
            for (uint j=0;j<this->size();j++)
            {
                if (validScalarValues)
                {
                    GL_SAFE_CALL(glTexCoord1d(scalarValues[j]));
                }
                GL_SAFE_CALL(glVertex3d(nodes[j].getX(),nodes[j].getY(),nodes[j].getZ()));
            }
            GLFunctions::end();
        }
    }

    if (validScalarValues)
    {
        GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
    }

    if (this->size() > 2)
    {
        if (this->getElementGroupData().getDrawEdges() && !this->getElementGroupData().getDrawWire())
        {
            GLboolean lightingEnabled;
            GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));
            GL_SAFE_CALL(glDisable(GL_LIGHTING));
            this->getGLWidget()->qglColor(QColor(Qt::black));
            GLFunctions::begin(GL_LINE_LOOP);
            for (uint j=0;j<this->size();j++)
            {
                GL_SAFE_CALL(glVertex3d(nodes[j].getX(),nodes[j].getY(),nodes[j].getZ()));
            }
            GLFunctions::end();
            GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
        }
    }

    if (this->getElementGroupData().getDrawNodes())
    {
        GLboolean lightingEnabled;
        GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));
        GL_SAFE_CALL(glDisable(GL_LIGHTING));
        this->getGLWidget()->qglColor(QColor(Qt::black));
        GLFunctions::begin(GL_POINTS);
        for (uint j=0;j<this->size();j++)
        {
            GL_SAFE_CALL(glVertex3d(nodes[j].getX(),nodes[j].getY(),nodes[j].getZ()));
        }
        GLFunctions::end();
        GL_SAFE_CALL(lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    }
}
