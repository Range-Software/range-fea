#include "gl_functions.h"
#include "gl_element_base.h"

void GLElementBase::_init(const GLElementBase *pGlElement)
{
    if (pGlElement)
    {
        this->drawMode = pGlElement->drawMode;
        this->elementGroupData = pGlElement->elementGroupData;
        this->color = pGlElement->color;
        this->setModelPtr(pGlElement->getModelPtr());
        this->setElementID(pGlElement->getElementID());
        this->setApplyEnvironmentSettings(pGlElement->getApplyEnvironmentSettings());
        this->pScalarVariable = pGlElement->pScalarVariable;
        this->pDisplacementVariable = pGlElement->pDisplacementVariable;
        this->nodePointSize = pGlElement->nodePointSize;
        this->edgeLineWidth = pGlElement->edgeLineWidth;
    }
}

GLElementBase::GLElementBase(GLWidget *glWidget, const Model *pModel, uint elementID, const REntityGroupData &elementGroupData, const QColor &color, GLElementDrawMode drawMode)
    : GLObject(glWidget)
    , drawMode(drawMode)
    , elementGroupData(elementGroupData)
    , color(color)
    , pModel(pModel)
    , elementID(elementID)
    , pScalarVariable(nullptr)
    , pDisplacementVariable(nullptr)
    , nodePointSize(10.0f)
    , edgeLineWidth(1.0f)
{
    this->_init();
}

GLElementBase::GLElementBase(const GLElementBase &glElement)
    : GLObject(glElement)
{
    this->_init(&glElement);
}

GLElementBase::~GLElementBase()
{
}

GLElementBase &GLElementBase::operator =(const GLElementBase &glElement)
{
    this->GLObject::operator =(glElement);
    this->_init(&glElement);
    return (*this);
}

GLElementDrawMode GLElementBase::getDrawMode() const
{
    return this->drawMode;
}

const REntityGroupData &GLElementBase::getElementGroupData() const
{
    return this->elementGroupData;
}

const QColor &GLElementBase::getColor() const
{
    return color;
}

const Model *GLElementBase::getModelPtr() const
{
    return this->pModel;
}

void GLElementBase::setModelPtr(const Model *pModel)
{
    this->pModel = pModel;
}

uint GLElementBase::getElementID() const
{
    return this->elementID;
}

void GLElementBase::setElementID(uint elementID)
{
    this->elementID = elementID;
}

void GLElementBase::setScalarVariable(const RVariable *pScalarVariable)
{
    this->pScalarVariable = pScalarVariable;
}

void GLElementBase::setDisplacementVariable(const RVariable *pDisplacementVariable)
{
    this->pDisplacementVariable = pDisplacementVariable;
}

void GLElementBase::setNodePointSize(GLfloat nodePointSize)
{
    this->nodePointSize = nodePointSize;
}

void GLElementBase::setEdgeLineWidth(GLfloat edgeLineWidth)
{
    this->edgeLineWidth = edgeLineWidth;
}

void GLElementBase::initialize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    GL_SAFE_CALL(glGetBooleanv(GL_LINE_SMOOTH, &this->lineSmoothEnabled));
    GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &this->lightingEnabled));
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalize));
    GL_SAFE_CALL(glGetFloatv(GL_POINT_SIZE, &this->pointSize));
    GL_SAFE_CALL(glGetFloatv(GL_LINE_WIDTH, &this->lineWidth));

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
        {
            GL_SAFE_CALL(glDisable(GL_NORMALIZE));
            GL_SAFE_CALL(glPointSize(15.0f));
            GL_SAFE_CALL(glLineWidth(2.0f));
            GL_SAFE_CALL(glDisable(GL_LIGHTING));
            break;
        }
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            GL_SAFE_CALL(glEnable(GL_NORMALIZE));
            GL_SAFE_CALL(glPointSize(this->nodePointSize));
            GL_SAFE_CALL(glLineWidth(this->edgeLineWidth));
            if (this->elementGroupData.getDrawWire())
            {
                GL_SAFE_CALL(glDisable(GL_LIGHTING));
            }
            break;
        }
    }
}

void GLElementBase::finalize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    GL_SAFE_CALL(this->lineSmoothEnabled ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH));
    GL_SAFE_CALL(this->lightingEnabled ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING));
    GL_SAFE_CALL(this->normalize ? glEnable(GL_NORMALIZE) : glDisable(GL_NORMALIZE));
    GL_SAFE_CALL(glPointSize(this->pointSize));
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
}
