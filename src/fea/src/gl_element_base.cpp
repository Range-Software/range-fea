#include "gl_functions.h"
#include "gl_element_base.h"
#include "gl_state_cache.h"

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

    GLStateCache &cache = GLStateCache::instance();

    // Save current state from cache (no GPU query)
    this->lineSmoothEnabled = cache.getLineSmooth();
    this->lightingEnabled = cache.getLighting();
    this->normalize = cache.getNormalize();
    this->pointSize = cache.getPointSize();
    this->lineWidth = cache.getLineWidth();

    switch (this->getDrawMode())
    {
        case GL_ELEMENT_DRAW_TEXT:
        {
            cache.disableNormalize();
            cache.setPointSize(15.0f);
            cache.setLineWidth(2.0f);
            cache.disableLighting();
            break;
        }
        case GL_ELEMENT_DRAW_NORMAL:
        default:
        {
            cache.enableNormalize();
            cache.setPointSize(this->nodePointSize);
            cache.setLineWidth(this->edgeLineWidth);
            if (this->elementGroupData.getDrawWire())
            {
                cache.disableLighting();
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

    GLStateCache &cache = GLStateCache::instance();

    // Restore saved state via cache (only makes GL calls if state changed)
    cache.setLineSmooth(this->lineSmoothEnabled);
    cache.setLighting(this->lightingEnabled);
    cache.setNormalize(this->normalize);
    cache.setPointSize(this->pointSize);
    cache.setLineWidth(this->lineWidth);
}
