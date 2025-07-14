#include "draw_engine_object.h"
#include "color.h"
#include "gl_widget.h"

DrawEngineObject::DrawEngineObject(QObject *parent) :
    QObject(parent),
    color(QColor(255,255,255,255))
{
    R_LOG_TRACE_IN;
    this->color = Color::random(true);
    this->color.setAlpha(100);
    R_LOG_TRACE_OUT;
}

const QString &DrawEngineObject::getName() const
{
    R_LOG_TRACE;
    return this->name;
}

uint DrawEngineObject::getNInputParameters() const
{
    R_LOG_TRACE;
    return uint(this->inputParameters.size());
}

const DrawEngineInput &DrawEngineObject::getInputParamater(uint position) const
{
    R_LOG_TRACE;
    return this->inputParameters[int(position)];
}

DrawEngineInput &DrawEngineObject::getInputParamater(uint position)
{
    R_LOG_TRACE;
    return this->inputParameters[int(position)];
}

void DrawEngineObject::updateModel()
{
    R_LOG_TRACE_IN;
    this->model = Model(this->generate(),this->getName(),tr("Model to be drawn"),false);

    const bool onlyElements = true;

    for (uint i=0;i<this->model.getNEntityGroups(onlyElements);i++)
    {
        REntityGroupType entityGroupType = this->model.getEntityGroupType(i,onlyElements);
        uint entityGroupPosition = this->model.getEntityGroupPosition(i,onlyElements);
        this->model.setVisible(entityGroupType,entityGroupPosition,true);
        this->model.setColor(entityGroupType,entityGroupPosition,this->color);
        this->model.setDrawEdges(entityGroupType,entityGroupPosition,true);
    }
    R_LOG_TRACE_OUT;
}

Model &DrawEngineObject::getModel()
{
    R_LOG_TRACE;
    return this->model;
}

const Model &DrawEngineObject::getModel() const
{
    R_LOG_TRACE;
    return this->model;
}

void DrawEngineObject::glDraw(GLWidget *glWidget) const
{
    R_LOG_TRACE_IN;
    bool useGlModelList = glWidget->getUseGLVoidModelList();
    glWidget->setUseGLVoidModelList(true);

    glWidget->getGLModelList().clear();
    this->model.glDraw(glWidget);
    glWidget->getGLModelList().clear();

    glWidget->setUseGLVoidModelList(useGlModelList);
    R_LOG_TRACE_OUT;
}
