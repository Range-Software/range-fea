#include "gl_functions.h"
#include "gl_interpolated_entity.h"
#include "gl_interpolated_element.h"
#include "gl_state_cache.h"
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
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalize));
    GL_SAFE_CALL(glGetFloatv(GL_POINT_SIZE, &this->pointSize));
    GL_SAFE_CALL(glGetFloatv(GL_LINE_WIDTH, &this->lineWidth));

    GL_SAFE_CALL(glEnable(GL_NORMALIZE));
    GLStateCache::instance().enableLighting();
    GL_SAFE_CALL(glPointSize(10.0f));
    GL_SAFE_CALL(glLineWidth(1.0f));
}

void GLInterpolatedEntity::finalize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    GLStateCache::instance().setLighting(this->lightingEnabled);
    GL_SAFE_CALL(this->normalize ? glEnable(GL_NORMALIZE) : glDisable(GL_NORMALIZE));
    GL_SAFE_CALL(glPointSize(this->pointSize));
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
}

void GLInterpolatedEntity::draw()
{
    if (!this->getData().getVisible())
    {
        return;
    }

    const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());

    RVariableType scalarVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR);
    uint scalarVariablePosition = rModel.findVariable(scalarVariableType);
    const RVariable *pScalarVariable = nullptr;
    if (scalarVariablePosition != RConstants::eod)
    {
        pScalarVariable = &rModel.getVariable(scalarVariablePosition);
        this->texture.load(pScalarVariable->getVariableData().getValueRangeName());
    }

    RVariableType displacementVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT);
    uint displacementVarPosition = rModel.findVariable(displacementVariableType);
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

    pGlEntityList->callList(GL_ENTITY_LIST_ITEM_NORMAL);

    this->texture.unload();
}
