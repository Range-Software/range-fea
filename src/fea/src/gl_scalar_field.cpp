#include <cmath>
#include <omp.h>

#include "gl_functions.h"
#include "gl_scalar_field.h"
#include "gl_point.h"
#include "gl_state_cache.h"
#include "application.h"

class ScalarFieldItem
{
    public:

        ScalarFieldItem (const RR3Vector &position,
                         double scalarRate,
                         bool validScaleValue,
                         double scaleValue)
            : position(position)
            , scalarRate(scalarRate)
            , validScaleValue(validScaleValue)
            , scaleValue(scaleValue)
        {

        }

        RR3Vector position;
        double scalarRate;
        bool validScaleValue;
        double scaleValue;
};

void GLScalarField::_init(const GLScalarField *pGlScalarField)
{
    if (pGlScalarField)
    {
        this->entityID = pGlScalarField->entityID;
        this->lightingEnabled = pGlScalarField->lightingEnabled;
    }
}

GLScalarField::GLScalarField(GLWidget *glWidget, const RScalarField &scalarField, const SessionEntityID &entityID)
    : GLObject(glWidget)
    , RScalarField(scalarField)
    , entityID(entityID)
{
    this->_init();
}

GLScalarField::GLScalarField(const GLScalarField &glScalarField)
    : GLObject(glScalarField)
    , RScalarField(glScalarField)
{
    this->_init(&glScalarField);
}

GLScalarField::~GLScalarField()
{

}

GLScalarField &GLScalarField::operator =(const GLScalarField &glScalarField)
{
    this->GLObject::operator =(glScalarField);
    this->RScalarField::operator =(glScalarField);
    this->_init(&glScalarField);
    return (*this);
}

const SessionEntityID &GLScalarField::getEntityID() const
{
    return this->entityID;
}

void GLScalarField::initialize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    this->lightingEnabled = GLStateCache::instance().getLighting();
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalize));

    GLStateCache::instance().disableLighting();
    GL_SAFE_CALL(glDisable(GL_NORMALIZE));
}

void GLScalarField::finalize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    GLStateCache::instance().setLighting(this->lightingEnabled);
    GL_SAFE_CALL(this->normalize ? glEnable(GL_NORMALIZE) : glDisable(GL_NORMALIZE));
}

void GLScalarField::draw()
{
    if (!this->getData().getVisible())
    {
        return;
    }

    const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());

    RVariableType scalarVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR);
    uint scalarVariablePosition = rModel.findVariable(scalarVariableType);
    const RVariable *pScalarVariable = nullptr;
    GLTexture texture;
    if (scalarVariablePosition != RConstants::eod)
    {
        pScalarVariable = &rModel.getVariable(scalarVariablePosition);
        texture.load(pScalarVariable->getVariableData().getValueRangeName());
    }

    RVariableType displacementVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT);
    uint displacementVariablePosition = rModel.findVariable(displacementVariableType);
    const RVariable *pDisplacementVariable = nullptr;
    if (displacementVariablePosition != RConstants::eod)
    {
        pDisplacementVariable = &rModel.getVariable(displacementVariablePosition);
    }

    GLEntityList *pGlEntityList = &this->getGLWidget()->getGLModelList().getGlScalarFieldList(this->getEntityID().getEid());

    if (!pGlEntityList->getListValid(GL_ENTITY_LIST_ITEM_NORMAL))
    {
        pGlEntityList->newList(GL_ENTITY_LIST_ITEM_NORMAL);

        std::vector<ScalarFieldItem> scalarField = this->calculateField(pScalarVariable,pDisplacementVariable);

        for (uint i=0;i<scalarField.size();i++)
        {
            float pointSize = float(scalarField[i].scalarRate * (this->maxPointSize - this->minPointSize) + this->minPointSize);

            if (pointSize <= 0.0f)
            {
                continue;
            }

            if (scalarField[i].validScaleValue)
            {
                GL_SAFE_CALL(glEnable(GL_TEXTURE_1D));
                this->getGLWidget()->qglColor(QColor(255,255,255,255));
                GLFunctions::texCoord1f(GLfloat(scalarField[i].scaleValue));
            }
            else
            {
                int r,g,b,a;
                this->getData().getColor(r,g,b,a);
                this->getGLWidget()->qglColor(QColor(r,g,b,a));
            }

            GLPoint point(this->glWidget,scalarField[i].position,pointSize);
            point.paint();

            if (scalarField[i].validScaleValue)
            {
                GL_SAFE_CALL(glDisable(GL_TEXTURE_1D));
            }
        }

        pGlEntityList->endList(GL_ENTITY_LIST_ITEM_NORMAL);
    }

    pGlEntityList->callList(GL_ENTITY_LIST_ITEM_NORMAL);

    if (scalarVariablePosition != RConstants::eod)
    {
        texture.unload();
    }
}

std::vector<ScalarFieldItem> GLScalarField::calculateField(const RVariable *pScalarVariable, const RVariable *pDisplacementVariable) const
{
    std::vector<ScalarFieldItem> field;

    const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());

    uint variablePosition = rModel.findVariable(this->variableType);

    if (variablePosition == RConstants::eod)
    {
        return field;
    }

    const RVariable &rVariable = rModel.getVariable(variablePosition);

    std::vector<bool> nodeBook;
    if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
    {
        nodeBook.resize(rModel.getNNodes(),false);
    }

//    double minValue = rVariable.getMinValue();
//    double maxValue = rVariable.getMaxValue();
    double minValue = rVariable.getVariableData().getMinDisplayValue();
    double maxValue = rVariable.getVariableData().getMaxDisplayValue();

#pragma omp parallel for default(shared)
    for (int64_t i=0;i<int64_t(this->elementGroupIDs.size());i++)
    {
        REntityGroupType entityType;
        uint entityID;

        if (!rModel.getEntityID(this->elementGroupIDs[uint(i)],entityType,entityID))
        {
            continue;
        }
        if (REntityGroup::typeIsElementGroup(entityType))
        {
            const RElementGroup *pElementGroup = rModel.getElementGroupPtr(this->elementGroupIDs[uint(i)]);
            if (!pElementGroup)
            {
                continue;
            }

            for (uint j=0;j<pElementGroup->size();j++)
            {
                uint elementID = pElementGroup->get(j);
                const RElement &rElement = rModel.getElement(elementID);

                std::vector<double> scalarValues = this->getScalarValues(elementID,rVariable.getApplyType(),pScalarVariable);
                std::vector<RR3Vector> displacementValues = this->getDisplacementValues(elementID,rVariable.getApplyType(),pDisplacementVariable);

                if (rVariable.getApplyType() == R_VARIABLE_APPLY_ELEMENT)
                {
                    RR3Vector position;
                    rElement.findCenter(rModel.getNodes(),position[0],position[1],position[2]);

                    if (displacementValues.size() == 1)
                    {
                        position[0] += displacementValues[0][0];
                        position[1] += displacementValues[0][1];
                        position[2] += displacementValues[0][2];
                    }

                    double scalarRate = 0.0;
                    if (std::abs(maxValue - minValue) > RConstants::eps)
                    {
                        scalarRate = (rVariable.getValue(elementID) - minValue)/(maxValue - minValue);
                    }

                    bool validScalarValue = (scalarValues.size() == 1);
                    double scalarValue = validScalarValue ? scalarValues[0] : 0.0;

#pragma omp critical
                    {
                        field.push_back(ScalarFieldItem(position,scalarRate,validScalarValue,scalarValue));
                    }
                }
                else if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
                {
                    for (uint k=0;k<rElement.size();k++)
                    {
                        uint nodeID = rElement.getNodeId(k);

                        bool nodeSet = false;
#pragma omp flush(nodeBook)
#pragma omp critical
                        {
                            if (nodeBook[nodeID])
                            {
                                nodeSet = true;
                            }
                            else
                            {
                                nodeBook[nodeID] = true;
                            }
                        }
                        if (nodeSet)
                        {
                            continue;
                        }

                        RR3Vector position;
                        position[0] = rModel.getNode(nodeID).getX();
                        position[1] = rModel.getNode(nodeID).getY();
                        position[2] = rModel.getNode(nodeID).getZ();

                        if (displacementValues.size() == rElement.size())
                        {
                            position[0] += displacementValues[k][0];
                            position[1] += displacementValues[k][1];
                            position[2] += displacementValues[k][2];
                        }

                        double scalarRate = 0.0;
                        if (std::abs(maxValue - minValue) > RConstants::eps)
                        {
                            scalarRate = (rVariable.getValue(nodeID) - minValue)/(maxValue - minValue);
                        }

                        bool validScalarValue = (scalarValues.size() == rElement.size());
                        double scalarValue = validScalarValue ? scalarValues[k] : 0.0;

#pragma omp critical
                        {
                            field.push_back(ScalarFieldItem(position,scalarRate,validScalarValue,scalarValue));
                        }
                    }
                }
            }
        }
        else
        {
            const RInterpolatedEntity *pIEntity = nullptr;
            switch (entityType)
            {
                case R_ENTITY_GROUP_CUT:
                    pIEntity = rModel.getCutPtr(entityID);
                    break;
                case R_ENTITY_GROUP_ISO:
                    pIEntity = rModel.getIsoPtr(entityID);
                    break;
                case R_ENTITY_GROUP_STREAM_LINE:
                    pIEntity = rModel.getStreamLinePtr(entityID);
                    break;
                default:
                    RLogger::warning("Non element-type entities are not implemented for scalar fields.\n");
                    continue;
            }

            if (!pIEntity)
            {
                continue;
            }

            for (uint j=0;j<pIEntity->size();j++)
            {
                const RInterpolatedElement &rIElement = pIEntity->at(j);
                if (rVariable.getApplyType() == R_VARIABLE_APPLY_ELEMENT)
                {
                    uint elementID = rIElement.at(0).getElementID();

                    RR3Vector position;
                    rIElement.findCenter(position[0],position[1],position[2]);

                    std::vector<RR3Vector> displacementValues = this->getDisplacementValues(elementID,rVariable.getApplyType(),pDisplacementVariable);
                    if (displacementValues.size() == 1)
                    {
                        position[0] += displacementValues[0][0];
                        position[1] += displacementValues[0][1];
                        position[2] += displacementValues[0][2];
                    }

                    double scalarRate = 0.0;
                    if (std::abs(maxValue - minValue) > RConstants::eps)
                    {
                        scalarRate = (rVariable.getValue(elementID) - minValue)/(maxValue - minValue);
                    }

                    std::vector<double> scalarValues = this->getScalarValues(elementID,rVariable.getApplyType(),pScalarVariable);

                    bool validScalarValue = (scalarValues.size() == 1);
                    double scalarValue = validScalarValue ? scalarValues[0] : 0.0;

#pragma omp critical
                    {
                        field.push_back(ScalarFieldItem(position,scalarRate,validScalarValue,scalarValue));
                    }

                }
                else if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
                {
                    for (uint k=0;k<rIElement.size();k++)
                    {
                        const RElement &rElement = rModel.getElement(rIElement[k].getElementID());

                        RR3Vector position = rIElement[k].toVector();

                        double scalarRate = 0.0;
                        if (std::abs(maxValue - minValue) > RConstants::eps)
                        {
                            std::vector<double> elementNodeValues;
                            elementNodeValues.resize(rElement.size(),0.0);

                            for (uint l=0;l<rElement.size();l++)
                            {
                                elementNodeValues[l] = rVariable.getValue(rElement.getNodeId(l));
                            }

                            scalarRate = rElement.interpolate(rModel.getNodes(),rIElement[k],elementNodeValues);
                            scalarRate = (scalarRate - minValue)/(maxValue - minValue);
                        }

                        std::vector<double> elementScalarValues = this->getScalarValues(rIElement[k].getElementID(),rVariable.getApplyType(),pScalarVariable);

                        bool validScalarValue = false;
                        double scalarValue = 0.0;

                        if (elementScalarValues.size() == rElement.size())
                        {
                            validScalarValue = true;
                            scalarValue = rElement.interpolate(rModel.getNodes(),rIElement[k],elementScalarValues);
                        }

                        std::vector<RR3Vector> displacementValues = this->getDisplacementValues(rIElement[k].getElementID(),rVariable.getApplyType(),pDisplacementVariable);
                        if (displacementValues.size() == rElement.size())
                        {
                            std::vector<double> xDisplacementValues;
                            std::vector<double> yDisplacementValues;
                            std::vector<double> zDisplacementValues;

                            xDisplacementValues.resize(displacementValues.size());
                            yDisplacementValues.resize(displacementValues.size());
                            zDisplacementValues.resize(displacementValues.size());

                            for (uint l=0;l<displacementValues.size();l++)
                            {
                                xDisplacementValues[l] = displacementValues[l][0];
                                yDisplacementValues[l] = displacementValues[l][1];
                                zDisplacementValues[l] = displacementValues[l][2];
                            }

                            position[0] += rElement.interpolate(rModel.getNodes(),rIElement[k],xDisplacementValues);
                            position[1] += rElement.interpolate(rModel.getNodes(),rIElement[k],yDisplacementValues);
                            position[2] += rElement.interpolate(rModel.getNodes(),rIElement[k],zDisplacementValues);
                        }

#pragma omp critical
                        {
                            field.push_back(ScalarFieldItem(position,scalarRate,validScalarValue,scalarValue));
                        }
                    }
                }
            }
        }
    }

    return field;
}

std::vector<double> GLScalarField::getScalarValues(uint elementID, RVariableApplyType variableApplyType, const RVariable *pVariable) const
{
    std::vector<double> scalarValues;

    if (!pVariable)
    {
        return scalarValues;
    }

    double minValue = pVariable->getVariableData().getMinDisplayValue();
    double maxValue = pVariable->getVariableData().getMaxDisplayValue();

    std::vector<double> values;

    if (pVariable->getApplyType() == R_VARIABLE_APPLY_NODE)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());
        const RElement &rElement = rModel.getElement(elementID);

        values.resize(rElement.size());
        for (uint i=0;i<rElement.size();i++)
        {
            values[i] = pVariable->getValue(rElement.getNodeId(i));
        }
    }
    else if (pVariable->getApplyType() == R_VARIABLE_APPLY_ELEMENT)
    {
        values.resize(1);
        values[0] = pVariable->getValue(elementID);
    }

    if (variableApplyType == R_VARIABLE_APPLY_NODE)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());
        scalarValues.resize(rModel.getElement(elementID).size());
        if (pVariable->getApplyType() == R_VARIABLE_APPLY_NODE)
        {
            for (uint i=0;i<values.size();i++)
            {
                scalarValues[i] = values[i];
            }
        }
        else if (pVariable->getApplyType() == R_VARIABLE_APPLY_ELEMENT)
        {
            for (uint i=0;i<values.size();i++)
            {
                scalarValues[i] = values[0];
            }
        }
    }
    else if (variableApplyType == R_VARIABLE_APPLY_ELEMENT)
    {
        scalarValues.resize(1,0.0);
        for (uint i=0;i<values.size();i++)
        {
            scalarValues[0] += values[i];
        }
        if (values.size() > 0)
        {
            scalarValues[0] /= values.size();
        }
    }

    for (uint i=0;i<scalarValues.size();i++)
    {
        scalarValues[i] = (scalarValues[i] - minValue) / (maxValue - minValue);

        scalarValues[i] = std::min(scalarValues[i],0.99);
        scalarValues[i] = std::max(scalarValues[i],0.01);
    }

    return scalarValues;
}

std::vector<RR3Vector> GLScalarField::getDisplacementValues(uint elementID, RVariableApplyType variableApplyType, const RVariable *pVariable) const
{
    std::vector<RR3Vector> displacementValues;

    if (!pVariable)
    {
        return displacementValues;
    }

    std::vector<RR3Vector> values;

    if (pVariable->getApplyType() == R_VARIABLE_APPLY_NODE)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());
        const RElement &rElement = rModel.getElement(elementID);

        values.resize(rElement.size());
        for (uint i=0;i<rElement.size();i++)
        {
            values[i][0] = pVariable->getValue(0,rElement.getNodeId(i));
            values[i][1] = pVariable->getValue(1,rElement.getNodeId(i));
            values[i][2] = pVariable->getValue(2,rElement.getNodeId(i));
        }
    }
    else if (pVariable->getApplyType() == R_VARIABLE_APPLY_ELEMENT)
    {
        values.resize(1);
        values[0][0] = pVariable->getValue(0,elementID);
        values[0][1] = pVariable->getValue(1,elementID);
        values[0][2] = pVariable->getValue(2,elementID);
    }

    if (variableApplyType == R_VARIABLE_APPLY_NODE)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());
        displacementValues.resize(rModel.getElement(elementID).size());
        if (pVariable->getApplyType() == R_VARIABLE_APPLY_NODE)
        {
            for (uint i=0;i<values.size();i++)
            {
                displacementValues[i][0] = values[i][0];
                displacementValues[i][1] = values[i][1];
                displacementValues[i][2] = values[i][2];
            }
        }
        else if (pVariable->getApplyType() == R_VARIABLE_APPLY_ELEMENT)
        {
            for (uint i=0;i<values.size();i++)
            {
                displacementValues[i][0] = values[0][0];
                displacementValues[i][1] = values[0][1];
                displacementValues[i][2] = values[0][2];
            }
        }
    }
    else if (variableApplyType == R_VARIABLE_APPLY_ELEMENT)
    {
        displacementValues.resize(1,RR3Vector(0.0,0.0,0.0));
        for (uint i=0;i<values.size();i++)
        {
            displacementValues[0][0] += values[i][0];
            displacementValues[0][1] += values[i][1];
            displacementValues[0][2] += values[i][2];
        }
        if (values.size() > 0)
        {
            displacementValues[0][0] /= values.size();
            displacementValues[0][1] /= values.size();
            displacementValues[0][2] /= values.size();
        }
    }

    double scale = pVariable->getVariableData().getScale();

    for (uint i=0;i<displacementValues.size();i++)
    {
        displacementValues[i] *= scale;
    }

    return displacementValues;
}
