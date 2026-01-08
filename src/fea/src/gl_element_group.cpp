#include "gl_functions.h"
#include "gl_element_group.h"
#include "gl_element.h"
#include "gl_state_cache.h"
#include "model.h"
#include "application.h"

void GLElementGroup::_init(const GLElementGroup *pGlElementGroup)
{
    if (pGlElementGroup)
    {
        this->pointVolume = pGlElementGroup->pointVolume;
        this->lineCrossArea = pGlElementGroup->lineCrossArea;
        this->surfaceThickness = pGlElementGroup->surfaceThickness;
        this->lightingEnabled = pGlElementGroup->lightingEnabled;
        this->normalize = pGlElementGroup->normalize;
        this->pointSize = pGlElementGroup->pointSize;
        this->lineWidth = pGlElementGroup->lineWidth;
    }
}

GLElementGroup::GLElementGroup(GLWidget *glWidget, const RElementGroup &elementGroup, const SessionEntityID &entityID)
    : GLEntity(glWidget,entityID)
    , RElementGroup(elementGroup)
    , pointVolume(0.0)
    , lineCrossArea(0.0)
    , surfaceThickness(0.0)
{
    this->_init();
}

GLElementGroup::GLElementGroup(const GLElementGroup &glElementGroup)
    : GLEntity(glElementGroup)
    , RElementGroup(glElementGroup)
{
    this->_init(&glElementGroup);
}

GLElementGroup::~GLElementGroup()
{
}

GLElementGroup &GLElementGroup::operator =(const GLElementGroup &glElementGroup)
{
    this->GLEntity::operator =(glElementGroup);
    this->RElementGroup::operator =(glElementGroup);
    this->_init(&glElementGroup);
    return (*this);
}

void GLElementGroup::setPointVolume(double pointVolume)
{
    this->pointVolume = pointVolume;
}

void GLElementGroup::setLineCrossArea(double lineCrossArea)
{
    this->lineCrossArea = lineCrossArea;
}

void GLElementGroup::setSurfaceThickness(double surfaceThickness)
{
    this->surfaceThickness = surfaceThickness;
}

void GLElementGroup::initialize()
{
    // Note: GL_NORMALIZE, point size (10.0f), and line width (1.0f) are now set once
    // in GLWidget::drawModel() to avoid per-entity state queries and changes.

    // Save lighting state only if we need to disable it for wire mode
    if (this->getData().getDrawWire())
    {
        // Use cache instead of GPU query (avoids CPU-GPU sync)
        this->lightingEnabled = GLStateCache::instance().getLighting();
        GLStateCache::instance().disableLighting();
    }

    const Model *pModel = this->pParentModel ? this->pParentModel : &Application::instance()->getSession()->getModel(this->getEntityID().getMid());

    RVariableType scalarVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR);
    unsigned int scalarVarPos = pModel->findVariable(scalarVariableType);
    if (scalarVarPos != RConstants::eod)
    {
        const RVariable &variable = pModel->getVariable(scalarVarPos);
        this->texture.load(variable.getVariableData().getValueRangeName());
    }
}

void GLElementGroup::finalize()
{
    // Only restore lighting state if it was modified (wire mode)
    if (this->getData().getDrawWire())
    {
        // Use cache to restore (only makes GL call if state actually changes)
        GLStateCache::instance().setLighting(this->lightingEnabled);
    }
    // Note: GL_NORMALIZE, point size, and line width don't need restoration
    // as they're set once in GLWidget::drawModel() and all entities use the same values.

    this->texture.unload();
}

void GLElementGroup::draw()
{
    if (!this->getData().getVisible())
    {
        return;
    }

    GLEntityList *pGlEntityList = nullptr;

    switch (this->getEntityID().getType())
    {
        case R_ENTITY_GROUP_POINT:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlPointList(this->getEntityID().getEid());
            break;
        case R_ENTITY_GROUP_LINE:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlLineList(this->getEntityID().getEid());
            break;
        case R_ENTITY_GROUP_SURFACE:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlSurfaceList(this->getEntityID().getEid());
            break;
        case R_ENTITY_GROUP_VOLUME:
            pGlEntityList = &this->getGLWidget()->getGLModelList().getGlVolumeList(this->getEntityID().getEid());
            break;
        default:
            return;
    }

    // Check if we need to rebuild (either display list or VBO is invalid)
    bool needsRebuild = pGlEntityList->getUseVBO()
        ? !pGlEntityList->getVBOValid(GL_ENTITY_LIST_ITEM_NORMAL)
        : !pGlEntityList->getListValid(GL_ENTITY_LIST_ITEM_NORMAL);

    if (needsRebuild)
    {
        const Model *pModel = this->pParentModel ? this->pParentModel : &Application::instance()->getSession()->getModel(this->getEntityID().getMid());

        RVariableType scalarVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_SCALAR);
        uint scalarVariablePosition = pModel->findVariable(scalarVariableType);
        const RVariable *pScalarVariable = nullptr;
        if (scalarVariablePosition != RConstants::eod)
        {
            pScalarVariable = &pModel->getVariable(scalarVariablePosition);
        }

        RVariableType displacementVariableType = this->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT);
        uint displacementVarPosition = pModel->findVariable(displacementVariableType);
        const RVariable *pDisplacementVariable = nullptr;
        if (displacementVarPosition != RConstants::eod)
        {
            pDisplacementVariable = &pModel->getVariable(displacementVarPosition);
        }

        bool colorByViewFactor = false;
        bool colorByPatch = false;
        QList<QColor> patchColors = pModel->getPatchColors();

        RViewFactorMatrixHeader viewFactorMatrixHeader;
        pModel->generateViewFactorMatrixHeader(viewFactorMatrixHeader);

        if (pModel->getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_RADIATIVE_HEAT)
        {
            const RPatchBook &rPatchBook = pModel->getViewFactorMatrix().getPatchBook();

            if (uint(patchColors.size()) != rPatchBook.getNPatches())
            {
                Application::instance()->getSession()->getModel(this->entityID.getMid()).generatePatchColors();
                patchColors = pModel->getPatchColors();
            }

            if (this->getData().getColorByPatch() && pModel->canColorByPatch())
            {
                colorByPatch = true;
            }
            if (this->getData().getColorByViewFactor() && pModel->canColorByViewFactor())
            {
                colorByViewFactor = true;
            }

            if (colorByViewFactor)
            {
                QVector<PickItem> pickList = Application::instance()->getSession()->getPickList().getItems(this->getEntityID().getMid());

                RRVector patchViewFactors(rPatchBook.getNPatches());
                patchViewFactors.fill(0.0);

                std::set<uint> pickedPatches;

                for (int i=0;i<pickList.size();i++)
                {
                    if (pickList[i].getEntityID().getType() != R_ENTITY_GROUP_SURFACE)
                    {
                        continue;
                    }

                    uint patchID = rPatchBook.findPatchID(pickList[i].getElementID());
                    if (patchID == RConstants::eod)
                    {
                        RLogger::error("Reffering to a nonexistent patch ID #%u\n",patchID);
                        continue;
                    }
                    pickedPatches.insert(patchID);
                }

                std::set<uint>::const_iterator it;
                for (it=pickedPatches.begin();it!=pickedPatches.end();++it)
                {
                    uint patchID = uint(*it);
                    const RSparseVector<double> &viewFactorRow = pModel->getViewFactorMatrix().getRow(patchID).getViewFactors();
                    std::vector<uint> indexes = viewFactorRow.getIndexes();
                    for (uint i=0;i<indexes.size();i++)
                    {
                        patchViewFactors[indexes[i]] += viewFactorRow.getValue(i);
                    }
                }

                double maxViewFactor = RStatistics::findMaximumValue(patchViewFactors);
                maxViewFactor = std::min(1.0,maxViewFactor);
                maxViewFactor = std::max(0.0,maxViewFactor);

                int r,g,b,a;

                this->getData().getColor(r,g,b,a);

                for (uint i=0;i<rPatchBook.getNPatches();i++)
                {
                    if (colorByPatch)
                    {
                        r = patchColors[int(i)].red();
                        g = patchColors[int(i)].green();
                        b = patchColors[int(i)].blue();
                        a = patchColors[int(i)].alpha();
                    }
                    double viewFactor = patchViewFactors[i];
                    if (maxViewFactor > RConstants::eps)
                    {
                        viewFactor /= maxViewFactor;
                    }
                    patchColors[int(i)] = QColor(qRound(r*viewFactor),
                                                 qRound(g*viewFactor),
                                                 qRound(b*viewFactor),a);
                }
            }
        } // R_PROBLEM_RADIATIVE_HEAT

        RUVector edgeElements;
        edgeElements.reserve(this->size());

        for (uint i=0;i<this->size();i++)
        {
            uint elementID = this->get(i);
            if (pModel->elementIsOnEdge(elementID))
            {
                edgeElements.push_back(elementID);
            }
        }

        // Start recording to VBO or display list
        if (this->getUseGlList())
        {
            if (pGlEntityList->getUseVBO())
            {
                // Use VBO recording for better performance
                GLFunctions::beginVBORecording(&pGlEntityList->getVBO(GL_ENTITY_LIST_ITEM_NORMAL));
            }
            else
            {
                // Fall back to display list
                pGlEntityList->newList(GL_ENTITY_LIST_ITEM_NORMAL);
            }
        }

        for (uint i=0;i<edgeElements.size();i++)
        {
            GLObject::PaintActionMask paintAction = GLObject::Draw;
            paintAction = (i == 0) ? paintAction | GLObject::Initialize : paintAction;
            paintAction = (i+1 == edgeElements.size()) ? paintAction | GLObject::Finalize : paintAction;

            if (i+1 < edgeElements.size())
            {
                if (i > 0)
                {
                    if (pModel->getElement(edgeElements[i-1]).getType() != pModel->getElement(edgeElements[i]).getType())
                    {
                        paintAction |= GLObject::Initialize;
                    }
                }
                if (pModel->getElement(edgeElements[i]).getType() != pModel->getElement(edgeElements[i+1]).getType())
                {
                    paintAction |= GLObject::Finalize;
                }
            }

            uint elementID = edgeElements[i];

            QColor color;
            if (colorByPatch || colorByViewFactor)
            {
                const RPatchBook &rPatchBook = pModel->getViewFactorMatrix().getPatchBook();
                uint patchID = rPatchBook.findPatchID(elementID);
                if (patchID == RConstants::eod)
                {
                    continue;
                }
                color = patchColors[int(patchID)];
            }
            else
            {
                int r,g,b,a;
                this->getData().getColor(r,g,b,a);
                color.setRgb(r,g,b,a);
            }
            GLElement glElement(this->getGLWidget(),
                                pModel,
                                elementID,
                                this->getData(),
                                color,
                                GL_ELEMENT_DRAW_NORMAL);
            glElement.setApplyEnvironmentSettings(false);
            glElement.setScalarVariable(pScalarVariable);
            glElement.setDisplacementVariable(pDisplacementVariable);
            glElement.setPointVolume(this->pointVolume);
            glElement.setLineCrossArea(this->lineCrossArea);
            glElement.setSurfaceThickness(this->surfaceThickness);
            glElement.setUseGlCullFace(this->getUseGlCullFace());
            glElement.paint(paintAction);
        }

        // End recording
        if (this->getUseGlList())
        {
            if (pGlEntityList->getUseVBO())
            {
                GLFunctions::endVBORecording();
            }
            else
            {
                pGlEntityList->endList(GL_ENTITY_LIST_ITEM_NORMAL);
            }
        }
    }

    // Render cached geometry
    if (this->getUseGlList())
    {
        if (pGlEntityList->getUseVBO())
        {
            // Render VBO
            pGlEntityList->getVBO(GL_ENTITY_LIST_ITEM_NORMAL).render();
        }
        else
        {
            // Render display list
            pGlEntityList->callList(GL_ENTITY_LIST_ITEM_NORMAL);
        }
    }

    if (this->getData().getDrawElementNumbers() || this->getData().getDrawNodeNumbers())
    {
        const Model *pModel = this->pParentModel ? this->pParentModel : &Application::instance()->getSession()->getModel(this->getEntityID().getMid());

        for (uint i=0;i<this->size();i++)
        {
            uint elementID = this->get(i);
            if (!pModel->elementIsOnEdge(elementID))
            {
                continue;
            }
            int r,g,b,a;
            this->getData().getColor(r,g,b,a);
            GLElement glElement(this->getGLWidget(),
                                pModel,
                                elementID,
                                this->getData(),
                                QColor(r,g,b,a),
                                GL_ELEMENT_DRAW_TEXT);
            glElement.setApplyEnvironmentSettings(false);
            glElement.setPointVolume(this->pointVolume);
            glElement.setLineCrossArea(this->lineCrossArea);
            glElement.setSurfaceThickness(this->surfaceThickness);
            glElement.paint();
        }
    }
}
