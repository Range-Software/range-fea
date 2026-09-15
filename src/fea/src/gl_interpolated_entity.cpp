#include "gl_functions.h"
#include "gl_interpolated_entity.h"
#include "gl_interpolated_element.h"
#include "gl_state_cache.h"
#include "gl_widget.h"
#include "application.h"


void GLInterpolatedEntity::_init(const GLInterpolatedEntity *pGLInterpolatedEntity)
{
    if (pGLInterpolatedEntity)
    {
        this->lightingEnabled = pGLInterpolatedEntity->lightingEnabled;
        this->normalize = pGLInterpolatedEntity->normalize;
        this->lineWidth = pGLInterpolatedEntity->lineWidth;
    }
}

GLInterpolatedEntity::GLInterpolatedEntity(GLWidget *glWidget, const RInterpolatedEntity &iEntity, const SessionEntityID &entityID)
    : GLEntity(glWidget,entityID)
    , RInterpolatedEntity(iEntity)

{
    this->_init();
}

GLInterpolatedEntity::GLInterpolatedEntity(const GLInterpolatedEntity &glInterpolatedEntity)
    : GLEntity(glInterpolatedEntity)
    , RInterpolatedEntity(glInterpolatedEntity)
{
    this->_init(&glInterpolatedEntity);
}

GLInterpolatedEntity::~GLInterpolatedEntity()
{
}

GLInterpolatedEntity &GLInterpolatedEntity::operator =(const GLInterpolatedEntity &glInterpolatedEntity)
{
    this->GLEntity::operator =(glInterpolatedEntity);
    this->RInterpolatedEntity::operator =(glInterpolatedEntity);
    this->REntityGroup::operator =(glInterpolatedEntity);
    this->_init(&glInterpolatedEntity);
    return (*this);
}

void GLInterpolatedEntity::initialize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    this->lightingEnabled = GLStateCache::instance().getLighting();
    this->normalize = GLStateCache::instance().getNormalize();
    this->pointSize = GLStateCache::instance().getPointSize();
    this->lineWidth = GLStateCache::instance().getLineWidth();

    GLStateCache::instance().setNormalize(GL_TRUE);
    GLStateCache::instance().enableLighting();
    GLStateCache::instance().setPointSize(10.0f);
    GLStateCache::instance().setLineWidth(1.0f);
}

void GLInterpolatedEntity::finalize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    GLStateCache::instance().setLighting(this->lightingEnabled);
    GLStateCache::instance().setNormalize(this->normalize);
    GLStateCache::instance().setPointSize(this->pointSize);
    GLStateCache::instance().setLineWidth(this->lineWidth);
}

void GLInterpolatedEntity::draw()
{
    if (!this->getData().getVisible())
    {
        return;
    }

    const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());

    uint scalarVariablePosition = rModel.findVariablePositionByDisplayType(this->getData(),R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR);
    const RVariable *pScalarVariable = nullptr;
    if (scalarVariablePosition != RConstants::eod)
    {
        pScalarVariable = &rModel.getVariable(scalarVariablePosition);
        this->texture.load(pScalarVariable->getVariableData().getValueRangeName());
    }

    uint displacementVarPosition = rModel.findVariablePositionByDisplayType(this->getData(),R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT);
    const RVariable *pDisplacementVariable = nullptr;
    if (displacementVarPosition != RConstants::eod)
    {
        pDisplacementVariable = &rModel.getVariable(displacementVarPosition);
    }

    GLEntityList *pGlEntityList = nullptr;

    switch (this->getEntityID().getType())
    {
        case R_ENTITY_GROUP_STREAM_LINE:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlStreamLineList(this->getEntityID().getEid());
            break;
        case R_ENTITY_GROUP_CUT:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlCutList(this->getEntityID().getEid());
            break;
        case R_ENTITY_GROUP_ISO:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlIsoList(this->getEntityID().getEid());
            break;
        default:
            return;
    }

    if (!pGlEntityList->getListValid(GL_ENTITY_LIST_ITEM_NORMAL))
    {
        pGlEntityList->newList(GL_ENTITY_LIST_ITEM_NORMAL);

        for (uint i=0;i<this->size();i++)
        {
            GLObject::PaintActionMask paintAction = GLObject::Draw;
            paintAction = (i == 0) ? paintAction | GLObject::Initialize : paintAction;
            paintAction = (i+1 == this->size()) ? paintAction | GLObject::Finalize : paintAction;

            GLInterpolatedElement glIElement(this->getGLWidget(),
                                             &rModel,
                                             this->at(i),
                                             i,
                                             this->getData(),
                                             GL_ELEMENT_DRAW_NORMAL);
            glIElement.setScalarVariable(pScalarVariable);
            glIElement.setDisplacementVariable(pDisplacementVariable);
            glIElement.setApplyEnvironmentSettings(false);
            glIElement.paint(paintAction);
        }

        pGlEntityList->endList(GL_ENTITY_LIST_ITEM_NORMAL);
    }

    // Cut/iso/stream entities are two-sided (no "inside" concept — don't render back faces as silver).
    GLStateCache::instance().setTwoSided(true);
    pGlEntityList->callList(GL_ENTITY_LIST_ITEM_NORMAL);
    GLStateCache::instance().setTwoSided(false);

    this->texture.unload();
}
