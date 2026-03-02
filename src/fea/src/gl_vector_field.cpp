#include <omp.h>

#include <rbl_r3vector.h>

#include "gl_functions.h"
#include "gl_vector_field.h"
#include "gl_arrow.h"
#include "gl_state_cache.h"
#include "gl_texture.h"
#include "application.h"

class VectorFieldItem
{
    public:

        VectorFieldItem (const RR3Vector &v1,
                         const RR3Vector &v2,
                         bool validScaleValue,
                         double scaleValue)
            : v1(v1)
            , v2(v2)
            , validScaleValue(validScaleValue)
            , scaleValue(scaleValue)
        {

        }

        RR3Vector v1;
        RR3Vector v2;
        bool validScaleValue;
        double scaleValue;
};

void GLVectorField::_init(const GLVectorField *pGlVectorField)
{
    if (pGlVectorField)
    {
        this->entityID = pGlVectorField->entityID;
        this->lightingEnabled = pGlVectorField->lightingEnabled;
        this->normalize = pGlVectorField->normalize;
        this->lineWidth = pGlVectorField->lineWidth;
    }
}

GLVectorField::GLVectorField(GLWidget *glWidget, const RVectorField &vectorFied, const SessionEntityID &entityID)
    : GLObject(glWidget)
    , RVectorField(vectorFied)
    , entityID(entityID)
{
    this->_init();
}

GLVectorField::GLVectorField(const GLVectorField &glVectorField)
    : GLObject(glVectorField)
    , RVectorField(glVectorField)
{
    this->_init(&glVectorField);
}

GLVectorField &GLVectorField::operator =(const GLVectorField &glVectorField)
{
    this->GLObject::operator =(glVectorField);
    this->RVectorField::operator =(glVectorField);
    this->_init(&glVectorField);
    return (*this);
}

const SessionEntityID &GLVectorField::getEntityID() const
{
    return this->entityID;
}

void GLVectorField::initialize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    this->lightingEnabled = GLStateCache::instance().getLighting();
    GL_SAFE_CALL(glGetBooleanv(GL_NORMALIZE, &this->normalize));
    GL_SAFE_CALL(glGetFloatv(GL_LINE_WIDTH, &this->lineWidth));

    GL_SAFE_CALL(glLineWidth(1.0f));
    if (this->getData().getDrawArrowHeads())
    {
        GL_SAFE_CALL(glEnable(GL_NORMALIZE));
    }
    else
    {
        GLStateCache::instance().disableLighting();
        GL_SAFE_CALL(glDisable(GL_NORMALIZE));
    }
}

void GLVectorField::finalize()
{
    if (this->getApplyEnvironmentSettings())
    {
        return;
    }

    GLStateCache::instance().setLighting(this->lightingEnabled);
    GL_SAFE_CALL(this->normalize ? glEnable(GL_NORMALIZE) : glDisable(GL_NORMALIZE));
    GL_SAFE_CALL(glLineWidth(this->lineWidth));
}

void GLVectorField::draw()
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

    GLEntityList *pGlEntityList = &this->getGLWidget()->getGLModelList().getGlVectorFieldList(this->getEntityID().getEid());

    if (!pGlEntityList->getListValid(GL_ENTITY_LIST_ITEM_NORMAL))
    {
        pGlEntityList->newList(GL_ENTITY_LIST_ITEM_NORMAL);

        std::vector<VectorFieldItem> vectorField = this->calculateField(pScalarVariable,pDisplacementVariable);

        for (uint i=0;i<vectorField.size();i++)
        {
            if (vectorField[i].validScaleValue)
            {
                GLStateCache::instance().enableTexture1D();
                this->getGLWidget()->qglColor(QColor(255,255,255,255));
                GLFunctions::texCoord1f(GLfloat(vectorField[i].scaleValue));
            }
            else
            {
                int r,g,b,a;
                this->getData().getColor(r,g,b,a);
                this->getGLWidget()->qglColor(QColor(r,g,b,a));
            }
            GLArrow arrow(this->glWidget,vectorField[i].v1,vectorField[i].v2,this->getData().getDrawArrowHeads(),this->getData().getDrawArrowFrom());
            arrow.paint();
            if (vectorField[i].validScaleValue)
            {
                GLStateCache::instance().disableTexture1D();
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

std::vector<VectorFieldItem> GLVectorField::calculateField(const RVariable *pScalarVariable, const RVariable *pDisplacementVariable) const
{
    std::vector<VectorFieldItem> field;
    const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());
    uint variablePosition = rModel.findVariable(this->variableType);

    if (variablePosition == RConstants::eod)
    {
        return field;
    }

    const RVariable &rVariable = rModel.getVariable(variablePosition);
    double scale = rVariable.getVariableData().getScale()*0.01;

    if (!this->getData().getDrawEqualArrowLength())
    {
        double maxValue = rVariable.getMaxValue();
        if (maxValue != 0.0)
        {
            scale /= maxValue*0.5;
        }
    }

    std::vector<bool> nodeBook;
    if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
    {
        nodeBook.resize(rModel.getNNodes(),false);
    }

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
                    // 1. vector
                    RR3Vector v1;
                    rElement.findCenter(rModel.getNodes(),v1[0],v1[1],v1[2]);
                    if (displacementValues.size() == 1)
                    {
                        v1[0] += displacementValues[0][0];
                        v1[1] += displacementValues[0][1];
                        v1[2] += displacementValues[0][2];
                    }

                    // 2. vector
                    RR3Vector v2;

                    if (rVariable.getNVectors() > 0)
                    {
                        v2[0] = rVariable.getValue(0,elementID);
                    }
                    if (rVariable.getNVectors() > 1)
                    {
                        v2[1] = rVariable.getValue(1,elementID);
                    }
                    if (rVariable.getNVectors() > 2)
                    {
                        v2[2] = rVariable.getValue(2,elementID);
                    }

                    if (!this->getType3D())
                    {
                        RR3Vector normal;
                        if (rElement.findNormal(rModel.getNodes(),normal[0],normal[1],normal[2]))
                        {
                            RR3Vector pv2;
                            RR3Vector::cross(normal,v2,pv2);
                            RR3Vector::cross(pv2,normal,v2);
                        }
                    }

                    if (this->getData().getDrawEqualArrowLength())
                    {
                        v2.normalize();
                    }
                    v2 *= scale;

                    bool validScalarValue = (scalarValues.size() == 1);
                    double scalarValue = validScalarValue ? scalarValues[0] : 0.0;

#pragma omp critical
                    {
                        field.push_back(VectorFieldItem(v1,v2,validScalarValue,scalarValue));
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

                        // 1. vector
                        RR3Vector v1;
                        v1[0] = rModel.getNode(nodeID).getX();
                        v1[1] = rModel.getNode(nodeID).getY();
                        v1[2] = rModel.getNode(nodeID).getZ();
                        if (displacementValues.size() == rElement.size())
                        {
                            v1[0] += displacementValues[k][0];
                            v1[1] += displacementValues[k][1];
                            v1[2] += displacementValues[k][2];
                        }

                        // 2. vector
                        RR3Vector v2;

                        if (rVariable.getNVectors() > 0)
                        {
                            v2[0] = rVariable.getValue(0,nodeID);
                        }
                        if (rVariable.getNVectors() > 1)
                        {
                            v2[1] = rVariable.getValue(1,nodeID);
                        }
                        if (rVariable.getNVectors() > 2)
                        {
                            v2[2] = rVariable.getValue(2,nodeID);
                        }

                        if (!this->getType3D())
                        {
                            RR3Vector normal;
                            if (rElement.findNormal(rModel.getNodes(),normal[0],normal[1],normal[2]))
                            {
                                RR3Vector pv2;
                                RR3Vector::cross(normal,v2,pv2);
                                RR3Vector::cross(pv2,normal,v2);
                            }
                        }

                        if (this->getData().getDrawEqualArrowLength())
                        {
                            v2.normalize();
                        }
                        v2 *= scale;

                        bool validScalarValue = (scalarValues.size() == rElement.size());
                        double scalarValue = validScalarValue ? scalarValues[k] : 0.0;

#pragma omp critical
                        {
                            field.push_back(VectorFieldItem(v1,v2,validScalarValue,scalarValue));
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
                    RLogger::warning("Non element-type entities '%s' (%d) are not implemented for vector fields.\n",REntityGroup::getTypeName(entityType).toUtf8().constData(),entityType);
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

                    // 1. vector
                    RR3Vector v1;
                    rIElement.findCenter(v1[0],v1[1],v1[2]);

                    std::vector<RR3Vector> displacementValues = this->getDisplacementValues(elementID,rVariable.getApplyType(),pDisplacementVariable);
                    if (displacementValues.size() == 1)
                    {
                        v1[0] += displacementValues[0][0];
                        v1[1] += displacementValues[0][1];
                        v1[2] += displacementValues[0][2];
                    }

                    // 2. vector
                    RR3Vector v2;

                    if (rVariable.getNVectors() > 0)
                    {
                        v2[0] = rVariable.getValue(0,elementID);
                    }
                    if (rVariable.getNVectors() > 1)
                    {
                        v2[1] = rVariable.getValue(1,elementID);
                    }
                    if (rVariable.getNVectors() > 2)
                    {
                        v2[2] = rVariable.getValue(2,elementID);
                    }

                    if (!this->getType3D())
                    {
                        RR3Vector normal;
                        if (rIElement.findNormal(normal[0],normal[1],normal[2]))
                        {
                            RR3Vector pv2;
                            RR3Vector::cross(normal,v2,pv2);
                            RR3Vector::cross(pv2,normal,v2);
                        }
                    }

                    if (this->getData().getDrawEqualArrowLength())
                    {
                        v2.normalize();
                    }
                    v2 *= scale;

                    std::vector<double> scalarValues = this->getScalarValues(elementID,rVariable.getApplyType(),pScalarVariable);

                    bool validScalarValue = (scalarValues.size() == 1);
                    double scalarValue = validScalarValue ? scalarValues[0] : 0.0;

#pragma omp critical
                    {
                        field.push_back(VectorFieldItem(v1,v2,validScalarValue,scalarValue));
                    }

                }
                else if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
                {
                    for (uint k=0;k<rIElement.size();k++)
                    {
                        const RElement &rElement = rModel.getElement(rIElement[k].getElementID());

                        // 1. vector
                        RR3Vector v1 = rIElement[k].toVector();

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

                            v1[0] += rElement.interpolate(rModel.getNodes(),rIElement[k],xDisplacementValues);
                            v1[1] += rElement.interpolate(rModel.getNodes(),rIElement[k],yDisplacementValues);
                            v1[2] += rElement.interpolate(rModel.getNodes(),rIElement[k],zDisplacementValues);
                        }

                        // 2. vector
                        RR3Vector v2;

                        std::vector<double> elementNodeValues;
                        elementNodeValues.resize(rElement.size(),0.0);

                        for (uint l=0;l<rElement.size();l++) elementNodeValues[l] = rVariable.getValue(0,rElement.getNodeId(l));
                        v2[0] = rElement.interpolate(rModel.getNodes(),rIElement[k],elementNodeValues);
                        for (uint l=0;l<rElement.size();l++) elementNodeValues[l] = rVariable.getValue(1,rElement.getNodeId(l));
                        v2[1] = rElement.interpolate(rModel.getNodes(),rIElement[k],elementNodeValues);
                        for (uint l=0;l<rElement.size();l++) elementNodeValues[l] = rVariable.getValue(2,rElement.getNodeId(l));
                        v2[2] = rElement.interpolate(rModel.getNodes(),rIElement[k],elementNodeValues);

                        std::vector<double> elementScalarValues = this->getScalarValues(rIElement[k].getElementID(),rVariable.getApplyType(),pScalarVariable);

                        bool validScalarValue = false;
                        double scalarValue = 0.0;

                        if (elementScalarValues.size() == rElement.size())
                        {
                            validScalarValue = true;
                            scalarValue = rElement.interpolate(rModel.getNodes(),rIElement[k],elementScalarValues);
                        }
                        if (!this->getType3D())
                        {
                            RR3Vector normal;
                            if (rIElement.findNormal(normal[0],normal[1],normal[2]))
                            {
                                RR3Vector pv2;
                                RR3Vector::cross(normal,v2,pv2);
                                RR3Vector::cross(pv2,normal,v2);
                            }
                        }

                        if (this->getData().getDrawEqualArrowLength())
                        {
                            v2.normalize();
                        }
                        v2 *= scale;

#pragma omp critical
                        {
                            field.push_back(VectorFieldItem(v1,v2,validScalarValue,scalarValue));
                        }
                    }
                }
            }
        }
    }

    return field;
}

std::vector<double> GLVectorField::getScalarValues(uint elementID, RVariableApplyType variableApplyType, const RVariable *pVariable) const
{
    std::vector<double> scalarValues;

    if (!pVariable)
    {
        return scalarValues;
    }

    double minValue = pVariable->getVariableData().getMinDisplayValue();
    double maxValue = pVariable->getVariableData().getMaxDisplayValue();

    std::vector<double> values;

    const Model &rModel = Application::instance()->getSession()->getModel(this->entityID.getMid());

    if (pVariable->getApplyType() == R_VARIABLE_APPLY_NODE)
    {
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

std::vector<RR3Vector> GLVectorField::getDisplacementValues(uint elementID, RVariableApplyType variableApplyType, const RVariable *pVariable) const
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
