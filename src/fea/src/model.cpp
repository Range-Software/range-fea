#include <QString>

#include <rbl_progress.h>

#include <rml_polygon.h>
#include <rml_file_manager.h>

#include "model.h"
#include "color.h"
#include "color_scale.h"
#include "gl_functions.h"
#include "gl_element.h"
#include "gl_element_group.h"
#include "gl_line.h"
#include "gl_interpolated_element.h"
#include "gl_interpolated_entity.h"
#include "gl_vector_field.h"
#include "gl_scalar_field.h"
#include "gl_widget.h"
#include "application.h"

const int Model::ConsolidateActionAll = Model::ConsolidateSurfaceNeighbors |
                                        Model::ConsolidateVolumeNeighbors |
                                        Model::ConsolidateEdgeNodes |
                                        Model::ConsolidateEdgeElements |
                                        Model::ConsolidateHoleElements |
                                        Model::ConsolidateSliverElements |
                                        Model::ConsolidateIntersectedElements |
                                        Model::ConsolidateMeshInput;

const double Model::SliverElementEdgeRatio = 30.0;

void Model::_init(const Model *pModel)
{
    R_LOG_TRACE;
    if (pModel)
    {
        this->edgeNodes = pModel->edgeNodes;
        this->edgeElements = pModel->edgeElements;
        this->holeElements = pModel->holeElements;
        this->sliverElements = pModel->sliverElements;
        this->intersectedElements = pModel->intersectedElements;
        this->surfaceNeigs = pModel->surfaceNeigs;
        this->volumeNeigs = pModel->volumeNeigs;
        this->fileName = pModel->fileName;
        this->meshInput = pModel->meshInput;
        this->viewFactorMatrix = pModel->viewFactorMatrix;
        this->patchColors = pModel->patchColors;
        this->actionDescription = pModel->actionDescription;
    }
}

Model::Model()
{
    R_LOG_TRACE;
    this->_init();
}

Model::Model(const Model &model) : RModel(model)
{
    R_LOG_TRACE;
    this->_init(&model);
}

Model::Model (const RModelMsh &modelMsh) : RModel(modelMsh)
{
    R_LOG_TRACE_IN;
    this->_init();
    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);
    R_LOG_TRACE_OUT;
}

Model::Model (const RModelStl &modelStl) : RModel(modelStl)
{
    R_LOG_TRACE_IN;
    this->_init();
    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);
    R_LOG_TRACE_OUT;
}

Model::Model (const RModelRaw &modelRaw,
              const QString &name,
              const QString &description, bool consolidate) : RModel(modelRaw,name,description)
{
    R_LOG_TRACE_IN;
    this->_init();
    if (consolidate)
    {
        this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);
    }
    R_LOG_TRACE_OUT;
}

Model & Model::operator = (const Model &model)
{
    R_LOG_TRACE_IN;
    this->RModel::operator =(model);
    this->_init(&model);
    R_LOG_TRACE_OUT;
    return (*this);
}

void Model::insertModel(const Model &model, bool mergeNearNodes, double tolerance, bool findNearest)
{
    R_LOG_TRACE_IN;
    uint nn = this->getNNodes();
    uint ne = this->getNElements();
    uint neg = this->getNElementGroups();

    QVector<uint> nodeBook;
    nodeBook.resize(int(model.getNNodes()));

    for (uint i=0;i<model.getNNodes();i++)
    {
        bool appendNode = false;
        if (mergeNearNodes)
        {
            uint nId = this->findNearNode(model.nodes[i],tolerance,findNearest);
            if (nId != RConstants::eod)
            {
                nodeBook[int(i)]=nId;
            }
            else
            {
                nodeBook[int(i)]=nn++;
                appendNode = true;
            }
        }
        else
        {
            nodeBook[int(i)]=nn++;
            appendNode = true;
        }
        if (appendNode)
        {
            this->nodes.push_back(model.nodes[i]);
            this->RResults::addNode();
        }
    }
    for (uint i=0;i<model.getNElements();i++)
    {
        this->elements.push_back(model.elements[i]);
        // Adjust node IDs.
        for (uint j=0;j<this->elements[ne+i].size();j++)
        {
            this->elements[ne+i].setNodeId(j,nodeBook[int(this->elements[ne+i].getNodeId(j))]);
        }
        this->RResults::addElement();
    }
    for (uint i=0;i<model.getNPoints();i++)
    {
        this->points.push_back(model.points[i]);
        // Adjust element IDs.
        RPoint &rPoint = this->getPoint(this->getNPoints()-1);
        for (uint j=0;j<rPoint.size();j++)
        {
            rPoint.set(j,rPoint.get(j)+ne);
        }
        // Set name.
        if (rPoint.getName().isEmpty())
        {
            rPoint.setName(this->generateNextEntityName(R_ENTITY_GROUP_POINT));
        }
    }
    for (uint i=0;i<model.getNLines();i++)
    {
        this->lines.push_back(model.lines[i]);
        // Adjust element IDs.
        RLine &rLine = this->getLine(this->getNLines()-1);
        for (uint j=0;j<rLine.size();j++)
        {
            rLine.set(j,rLine.get(j)+ne);
        }
        // Set name.
        if (rLine.getName().isEmpty())
        {
            rLine.setName(this->generateNextEntityName(R_ENTITY_GROUP_LINE));
        }
    }
    for (uint i=0;i<model.getNSurfaces();i++)
    {
        this->surfaces.push_back(model.surfaces[i]);
        // Adjust element IDs.
        RSurface &rSurface = this->getSurface(this->getNSurfaces()-1);
        for (uint j=0;j<rSurface.size();j++)
        {
            rSurface.set(j,rSurface.get(j)+ne);
        }
        // Set name.
        if (rSurface.getName().isEmpty())
        {
            rSurface.setName(this->generateNextEntityName(R_ENTITY_GROUP_SURFACE));
        }
    }
    for (uint i=0;i<model.getNVolumes();i++)
    {
        this->volumes.push_back(model.volumes[i]);
        // Adjust element IDs.
        RVolume &rVolume = this->getVolume(this->getNVolumes()-1);
        for (uint j=0;j<rVolume.size();j++)
        {
            rVolume.set(j,rVolume.get(j)+ne);
        }
        // Set name.
        if (rVolume.getName().isEmpty())
        {
            rVolume.setName(this->generateNextEntityName(R_ENTITY_GROUP_VOLUME));
        }
    }
    for (uint i=0;i<model.getNVectorFields();i++)
    {
        this->vectorFields.push_back(model.vectorFields[i]);
        // Adjust element group IDs.
        RVectorField &rVectorField = this->getVectorField(this->getNVectorFields()-1);
        std::vector<uint> &elementGroupIDs = rVectorField.getElementGroupIDs();
        for (uint j=0;j<elementGroupIDs.size();j++)
        {
            elementGroupIDs[j]+=neg;
        }
        // Set name.
        if (rVectorField.getName().isEmpty())
        {
            rVectorField.setName(this->generateNextEntityName(R_ENTITY_GROUP_VECTOR_FIELD));
        }
    }
    for (uint i=0;i<model.getNScalarFields();i++)
    {
        this->scalarFields.push_back(model.scalarFields[i]);
        RScalarField &rScalarField = this->getScalarField(this->getNScalarFields()-1);
        // Set name.
        if (rScalarField.getName().isEmpty())
        {
            rScalarField.setName(this->generateNextEntityName(R_ENTITY_GROUP_SCALAR_FIELD));
        }
    }
    for (uint i=0;i<model.getNStreamLines();i++)
    {
        this->streamLines.push_back(model.streamLines[i]);
        RStreamLine &rStreamLine = this->getStreamLine(this->getNStreamLines()-1);
        // Set name.
        if (rStreamLine.getName().isEmpty())
        {
            rStreamLine.setName(this->generateNextEntityName(R_ENTITY_GROUP_STREAM_LINE));
        }
    }
    for (uint i=0;i<model.getNCuts();i++)
    {
        this->cuts.push_back(model.cuts[i]);
        // Adjust element group IDs.
        RCut &rCut = this->getCut(this->getNCuts()-1);
        std::vector<uint> &elementGroupIDs = rCut.getElementGroupIDs();
        for (uint j=0;j<elementGroupIDs.size();j++)
        {
            elementGroupIDs[j]+=neg;
        }
        // Set name.
        if (rCut.getName().isEmpty())
        {
            rCut.setName(this->generateNextEntityName(R_ENTITY_GROUP_CUT));
        }
    }
    for (uint i=0;i<model.getNIsos();i++)
    {
        this->isos.push_back(model.isos[i]);
        // Adjust element group IDs.
        RIso &rIso = this->getIso(this->getNIsos()-1);
        std::vector<uint> &elementGroupIDs = rIso.getElementGroupIDs();
        for (uint j=0;j<elementGroupIDs.size();j++)
        {
            elementGroupIDs[j]+=neg;
        }
        // Set name.
        if (rIso.getName().isEmpty())
        {
            rIso.setName(this->generateNextEntityName(R_ENTITY_GROUP_ISO));
        }
    }
    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);
    R_LOG_TRACE_OUT;
}

const QString &Model::getFileName() const
{
    return this->fileName;
}

void Model::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

const RMeshInput &Model::getMeshInput() const
{
    return this->meshInput;
}

RMeshInput &Model::getMeshInput()
{
    return this->meshInput;
}

void Model::setMeshInput(const RMeshInput &meshInput)
{
    this->meshInput = meshInput;
}

void Model::initializeMeshInput()
{
    double xmin, xmax, ymin, ymax, zmin, zmax;

    this->findNodeLimits(xmin, xmax, ymin, ymax, zmin, zmax);

    double dx = xmax - xmin;
    double dy = ymax - ymin;
    double dz = zmax - zmin;

    this->meshInput.setVolumeConstraint(dx*dy*dz/1000.0);

    RMeshSetup &rMeshSetup = this->getProblemSetup().getMeshSetup();

    if (rMeshSetup.getMinEdgeLength() == 0.0 || rMeshSetup.getMaxEdgeLength() == 0.0)
    {
        double initEdgeLenght = std::cbrt(12.0 * this->meshInput.getVolumeConstraint() / std::sqrt(2.0));
        rMeshSetup.setMinEdgeLength(initEdgeLenght*0.1);
        rMeshSetup.setMaxEdgeLength(initEdgeLenght);
    }
}

const RViewFactorMatrix &Model::getViewFactorMatrix() const
{
    return this->viewFactorMatrix;
}

RViewFactorMatrix &Model::getViewFactorMatrix()
{
    return this->viewFactorMatrix;
}

bool Model::canColorByPatch() const
{
    return ((this->getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_RADIATIVE_HEAT) &&
            (this->getViewFactorMatrix().getPatchBook().getNPatches() > 0));
}

bool Model::canColorByViewFactor() const
{
    return ((this->getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_RADIATIVE_HEAT) &&
            (this->getViewFactorMatrix().getPatchBook().getNPatches() > 0) &&
            (this->getViewFactorMatrix().size() == this->getViewFactorMatrix().getPatchBook().getNPatches()));
}

const QList<QColor> &Model::getPatchColors() const
{
    return this->patchColors;
}

QList<QColor> &Model::getPatchColors()
{
    return this->patchColors;
}

void Model::generatePatchColors()
{
    this->patchColors.clear();
    for (uint i=0;i<this->viewFactorMatrix.getPatchBook().getNPatches();i++)
    {
        int r,g,b;
        r = qRound(255*QRandomGenerator::global()->generateDouble());
        g = qRound(255*QRandomGenerator::global()->generateDouble());
        b = qRound(255*QRandomGenerator::global()->generateDouble());
        this->patchColors.push_back(QColor(r,g,b,255));
    }
}

void Model::generateLineFromSurfaceEdges(double separationAngle)
{
    QVector<RElement> segments = this->findEdgeElements(separationAngle);

    if (segments.size() == 0)
    {
        return;
    }

    for (int i=0;i<segments.size();i++)
    {
        this->addElement(segments[i],true);
    }
}

const REntityGroupData *Model::getElementGroupData(REntityGroupType elementGroupType, uint position) const
{
    switch (elementGroupType)
    {
        case R_ENTITY_GROUP_POINT:
            return &this->getPoint(position).getData();
        case R_ENTITY_GROUP_LINE:
            return &this->getLine(position).getData();
        case R_ENTITY_GROUP_SURFACE:
            return &this->getSurface(position).getData();
        case R_ENTITY_GROUP_VOLUME:
            return &this->getVolume(position).getData();
        case R_ENTITY_GROUP_VECTOR_FIELD:
            return &this->getVectorField(position).getData();
        case R_ENTITY_GROUP_SCALAR_FIELD:
            return &this->getScalarField(position).getData();
        case R_ENTITY_GROUP_STREAM_LINE:
            return &this->getStreamLine(position).getData();
        case R_ENTITY_GROUP_CUT:
            return &this->getCut(position).getData();
        case R_ENTITY_GROUP_ISO:
            return &this->getIso(position).getData();
        default:
            return nullptr;
    }
}

REntityGroupData *Model::getElementGroupData(REntityGroupType elementGroupType, uint position)
{
    switch (elementGroupType)
    {
        case R_ENTITY_GROUP_POINT:
            return &this->getPoint(position).getData();
        case R_ENTITY_GROUP_LINE:
            return &this->getLine(position).getData();
        case R_ENTITY_GROUP_SURFACE:
            return &this->getSurface(position).getData();
        case R_ENTITY_GROUP_VOLUME:
            return &this->getVolume(position).getData();
        case R_ENTITY_GROUP_VECTOR_FIELD:
            return &this->getVectorField(position).getData();
        case R_ENTITY_GROUP_SCALAR_FIELD:
            return &this->getScalarField(position).getData();
        case R_ENTITY_GROUP_STREAM_LINE:
            return &this->getStreamLine(position).getData();
        case R_ENTITY_GROUP_CUT:
            return &this->getCut(position).getData();
        case R_ENTITY_GROUP_ISO:
            return &this->getIso(position).getData();
        default:
            return nullptr;
    }
}

void Model::autoMarkSurfaces(double angle)
{
    RLogger::info("Auto-marking surface elements\n");
    RLogger::indent();
    RProgressInitialize("Auto-marking surface elements", true);

    std::vector<uint> marks;
    marks.resize(this->surfaceNeigs.size(),0);
    uint cMark = 0;

    for (uint i=0;i<this->getNElements();i++)
    {
        if (RElementGroup::getGroupType(this->getElement(i).getType()) != R_ENTITY_GROUP_SURFACE)
        {
            continue;
        }
        if (marks[i] != 0)
        {
            continue;
        }
        marks[i] = ++cMark;
        this->markSurfaceNeighbors(i,angle,this->surfaceNeigs,marks);
    }

    REntityGroupData data = this->getSurface(0).getData();
    RMaterial material = this->getSurface(0).getMaterial();
    double thickness = this->getSurface(0).getThickness();

    // Must be resized to 0 to loose all information about stored elements
    this->setNSurfaces(0);
    this->setNSurfaces(cMark);

    for (uint i=0;i<this->getNSurfaces();i++)
    {
        Color color = Color::random(true);
        data.setColor(color.red(),color.green(),color.blue());

        this->getSurface(i).setName("Surface " + QString::number(i+1));
        this->getSurface(i).setMaterial(material);
        this->getSurface(i).setThickness(thickness);
        this->getSurface(i).setData(data);
    }

    for (uint i=0;i<this->getNElements();i++)
    {
        if (RElementGroup::getGroupType(this->getElement(i).getType()) != R_ENTITY_GROUP_SURFACE)
        {
            continue;
        }
        if (marks[i] == 0)
        {
            RLogger::warning("Unmarked surface element \'%u\'\n",i);
            continue;
        }
        this->getSurface(marks[i]-1).add(i);
    }

    RProgressFinalize();
    RLogger::unindent();
}

void Model::markSurface(double angle, QList<uint> elementIDs)
{
    if (elementIDs.empty())
    {
        return;
    }

    RLogger::info("Marking surface elements\n");
    RLogger::indent();
    RProgressInitialize("Marking surface elements", true);

    std::vector<uint> marks;
    marks.resize(this->surfaceNeigs.size(),0);

    uint nMark = this->getNSurfaces();

    // Remember data, thickness and material for the first selected element.
    uint sourceSurfaceId = 0;
    for (uint i=0;i<this->getNSurfaces();i++)
    {
        uint position;
        if (this->getSurface(i).findPosition(elementIDs[0],&position))
        {
            sourceSurfaceId = i;
            break;
        }
    }

    for (int i=0;i<elementIDs.size();i++)
    {
        marks[elementIDs[i]] = nMark;
        this->markSurfaceNeighbors(elementIDs[i],angle,this->surfaceNeigs,marks);
    }

    for (uint i=0;i<this->getNSurfaces();i++)
    {
        for (uint j=0;j<marks.size();j++)
        {
            if (marks[j] == 0)
            {
                continue;
            }
            uint elementPosition;
            if (!this->getSurface(i).findPosition(j,&elementPosition))
            {
                continue;
            }
            this->getSurface(i).remove(elementPosition);
        }
    }

    this->setNSurfaces(this->getNSurfaces()+1);

    for (uint i=0;i<marks.size();i++)
    {
        if (marks[i] > 0)
        {
            this->getSurface(marks[i]).add(i);
        }
    }

    REntityGroupData data = this->getSurface(sourceSurfaceId).getData();
    RMaterial material = this->getSurface(sourceSurfaceId).getMaterial();
    double thickness = this->getSurface(sourceSurfaceId).getThickness();

    Color color = Color::random(true);
    data.setColor(color.red(),color.green(),color.blue());

    this->getSurface(nMark).setName("Surface " + QString::number(nMark+1));
    this->getSurface(nMark).setMaterial(material);
    this->getSurface(nMark).setThickness(thickness);
    this->getSurface(nMark).setData(data);

    for (uint i=0;i<this->getNSurfaces();i++)
    {
        if (this->getSurface(i).size() == 0)
        {
            this->removeSurface(i);
            i--;
        }
    }

    RProgressFinalize();
    RLogger::unindent();
}

void Model::closeSurfaceHole(QList<uint> edgeIDs)
{
    if (edgeIDs.empty())
    {
        return;
    }

    RLogger::info("Closing surface hole\n");
    RLogger::indent();

    QList<uint> elementIDs = this->sortLineElements(this->holeElements.toList(),edgeIDs[0]);

    QList<uint> nodeIDs;
    nodeIDs.reserve(elementIDs.size());

    for (int i=0;i<elementIDs.size();i++)
    {
        nodeIDs.append(this->holeElements[int(elementIDs[i])].getNodeId(0));
    }

    std::vector<RNode> edgeNodes;
    edgeNodes.resize(uint(nodeIDs.size()));
    for (uint i=0;i<edgeNodes.size();i++)
    {
        edgeNodes[i] = this->getNode(nodeIDs[int(i)]);
    }

    if (edgeNodes.size() > 0)
    {
        std::vector<RElement> patchElements = RPolygon::triangulate(edgeNodes,true);
        for (uint j=0;j<patchElements.size();j++)
        {
            patchElements[j].swapNodeIds(1,2);
            for (uint k=0;k<patchElements[j].size();k++)
            {
                patchElements[j].setNodeId(k,nodeIDs[int(patchElements[j].getNodeId(k))]);
            }
            this->addElement(patchElements[j]);
        }
    }

    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);

    RLogger::unindent();
}

void Model::transformGeometry(const GeometryTransformInput &geometryTransformInput, const QList<SessionEntityID> &entityIDs)
{
    this->consolidate(Model::ConsolidateActionAll);

    QSet<uint> nodeIDs;

    uint nSweepSteps = 1;

    if (geometryTransformInput.getApplyTo() != GeometryTransformInput::ApplyToAll &&
        geometryTransformInput.getIncludeSharedNodes() &&
        geometryTransformInput.getSplitSharedNodes() &&
        geometryTransformInput.getSweepSharedNodes())
    {
        nSweepSteps = geometryTransformInput.getNSweepSteps();
    }

    for (uint sweepStep=0;sweepStep<nSweepSteps;sweepStep++)
    {
        GeometryTransformInput input(geometryTransformInput);

        RLogger::info("Performing sweep step %u of %u\n",sweepStep+1,nSweepSteps);

        if (nSweepSteps > 1)
        {
            double scale = 1.0/double(nSweepSteps);
            if (input.isTranslateActive())
            {
                input.getTranslation().scale(scale);
            }
            if (input.isRotateActive())
            {
                input.getRotation().scale(scale);
            }
            if (input.isScaleActive())
            {

                double sx = input.getScale()[0];
                double sy = input.getScale()[1];
                double sz = input.getScale()[2];

                sx = (1.0+(double(sweepStep+1)/double(nSweepSteps))*(sx-1.0))/(1.0+(double(sweepStep)/double(nSweepSteps))*(sx-1.0));
                sy = (1.0+(double(sweepStep+1)/double(nSweepSteps))*(sy-1.0))/(1.0+(double(sweepStep)/double(nSweepSteps))*(sy-1.0));
                sz = (1.0+(double(sweepStep+1)/double(nSweepSteps))*(sz-1.0))/(1.0+(double(sweepStep)/double(nSweepSteps))*(sz-1.0));

                if (input.getScale()[0] != 1.0)
                {
                    input.getScale()[0] = sx;
                }
                if (input.getScale()[1] != 1.0)
                {
                    input.getScale()[1] = sy;
                }
                if (input.getScale()[2] != 1.0)
                {
                    input.getScale()[2] = sz;
                }
            }
        }

        if (input.getApplyTo() == GeometryTransformInput::ApplyToAll)
        {
            nodeIDs.reserve(int(this->getNNodes()));
            for (uint i=0;i<this->getNNodes();i++)
            {
                nodeIDs.insert(uint(i));
            }
        }
        else if (input.getApplyTo() == GeometryTransformInput::ApplyToSelected ||
                 input.getApplyTo() == GeometryTransformInput::ApplyToPicked ||
                 input.getApplyTo() == GeometryTransformInput::ApplyToVisible)
        {
            QSet<uint> elementIDs = this->getElementIDs(entityIDs);
            QSet<uint> edgeNodeIDs = this->findEdgeNodeIDs(entityIDs);

            nodeIDs = this->getNodeIDs(elementIDs);

            if (input.getIncludeSharedNodes())
            {
                if (input.getSplitSharedNodes())
                {
                    QMap<uint,uint> newEdgeNodeMap = this->splitNodes(edgeNodeIDs,elementIDs);
                    if (input.getSweepSharedNodes())
                    {
                        QList<RElement> edgeElements = this->generateEdgeElements(edgeNodeIDs,elementIDs);
                        this->createSweepEdgeElements(edgeElements,newEdgeNodeMap,sweepStep > 0,sweepStep+1 == nSweepSteps,sweepStep+1 == nSweepSteps);
                    }
                }
            }
            else
            {
                nodeIDs.subtract(edgeNodeIDs);
            }
        }
        else
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Unknown apply-to type \'%d\'",input.getApplyTo());
        }

        try
        {
            this->rotateGeometry(nodeIDs,input.getRotation(),input.getRotationCenter());
        }
        catch (const RError &error)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Failed to rotate the geometry. %s", error.getMessage().toUtf8().constData());
        }

        try
        {
            this->scaleGeometry(nodeIDs,input.getScale(),input.getScaleCenter());
        }
        catch (const RError &error)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Failed to scale the geometry. %s", error.getMessage().toUtf8().constData());
        }

        try
        {
            this->translateGeometry(nodeIDs,input.getTranslation());
        }
        catch (const RError &error)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Failed to translate the geometry. %s", error.getMessage().toUtf8().constData());
        }
    }

    this->mergeNearNodes();
    this->consolidate(Model::ConsolidateActionAll);
}

uint Model::coarsenSurfaceElements(const std::vector<uint> surfaceIDs, double edgeLength, double elementArea)
{
    uint nDeleted = this->RModel::coarsenSurfaceElements(surfaceIDs,edgeLength,elementArea);

    this->consolidate(Model::ConsolidateActionAll);

    return nDeleted;
}

uint Model::tetrahedralizeSurface(const std::vector<uint> surfaceIDs)
{
    uint nGenerated = this->RModel::tetrahedralizeSurface(surfaceIDs);

    this->consolidate(Model::ConsolidateActionAll);

    return nGenerated;
}

uint Model::mergeNearNodes(double tolerance)
{
    uint nMerged = this->RModel::mergeNearNodes(tolerance);

    this->consolidate(Model::ConsolidateActionAll);

    return nMerged;
}

uint Model::purgeUnusedNodes()
{
    uint nPurged = this->RModel::purgeUnusedNodes();

    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements);

    return nPurged;
}

uint Model::purgeUnusedElements()
{
    uint nPurged = this->RModel::purgeUnusedElements();

    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);

    return nPurged;
}

uint Model::removeDuplicateElements()
{
    uint nMerged = this->RModel::removeDuplicateElements();

    this->consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements);

    return nMerged;
}

uint Model::fixSliverElements(double edgeRatio)
{
    uint nAffected = this->RModel::fixSliverElements(edgeRatio);
    if (nAffected == 0)
    {
        RLogger::info("No sliver elements were found.\n");
    }
    else
    {
        RLogger::info("Total number of sliver elements that were affected = %d.\n", nAffected);

        RMeshInput tmpInput = this->getMeshInput();
        this->consolidate(Model::ConsolidateActionAll);
        this->setMeshInput(tmpInput);
    }

    return nAffected;
}

void Model::updateSliverElements(double edgeRatio)
{
    this->sliverElements = this->findSliverElements(edgeRatio);
    int ni = this->sliverElements.size();
    if (ni == 0)
    {
        RLogger::info("No sliver elements were found.\n");
    }
    else
    {
        RLogger::warning("Number of sliver elements found = %d.\n", ni);
    }
}

void Model::updateIntersectedElements()
{
    this->intersectedElements = this->findIntersectedElements();
    int ni = this->intersectedElements.size();
    if (ni == 0)
    {
        RLogger::info("No intersected elements were found.\n");
    }
    else
    {
        RLogger::warning("Number of intersected elements found = %d.\n", ni);
    }
}

uint Model::breakIntersectedElements(uint nIterations)
{
    uint ni = this->RModel::breakIntersectedElements(nIterations);
    if (ni == 0)
    {
        RLogger::info("No intersected elements were found.\n");
    }
    else
    {
        RLogger::info("Total number of elements that were intersected = %d.\n", ni);

        RMeshInput tmpInput = this->getMeshInput();
        this->consolidate(Model::ConsolidateActionAll);
        this->setMeshInput(tmpInput);
    }
    return ni;
}

bool Model::exportSliverElements() const
{
    if (this->sliverElements.size() == 0)
    {
        RLogger::warning("No intersecting elements were found.\n");
        return false;
    }

    std::vector<uint> nodeBook;
    nodeBook.resize(this->getNNodes(),RConstants::eod);

    for (int i=0;i<this->sliverElements.size();i++)
    {
        uint elementID = this->sliverElements[i];
        const RElement &rElement = this->getElement(elementID);
        for (uint j=0;j<rElement.size();j++)
        {
            nodeBook[rElement.getNodeId(j)] = 0;
        }
    }

    Model model;

    uint nn = 0;
    for (uint i=0;i<nodeBook.size();i++)
    {
        if (nodeBook[i] == 0)
        {
            nodeBook[i] = nn++;
            model.addNode(this->getNode(i));
        }
    }

    for (int i=0;i<this->sliverElements.size();i++)
    {
        uint elementID = this->sliverElements[i];
        RElement element = this->getElement(elementID);
        for (uint j=0;j<element.size();j++)
        {
            element.setNodeId(j,nodeBook[element.getNodeId(j)]);
        }
        model.addElement(element,true,0);
    }

    model.setName(this->getName() + " - sliver");
    model.setDescription("Sliver elements");
    model.consolidate(Model::ConsolidateActionAll);

    Application::instance()->getSession()->addModel(model);

    RLogger::warning("Sliver elements were found (%u).\n",model.getNElements());

    return true;
}

bool Model::exportIntersectedElements() const
{
    if (this->intersectedElements.size() == 0)
    {
        RLogger::warning("No intersecting elements were found.\n");
        return false;
    }

    std::vector<uint> nodeBook;
    nodeBook.resize(this->getNNodes(),RConstants::eod);

    for (int i=0;i<this->intersectedElements.size();i++)
    {
        uint elementID = this->intersectedElements[i];
        const RElement &rElement = this->getElement(elementID);
        for (uint j=0;j<rElement.size();j++)
        {
            nodeBook[rElement.getNodeId(j)] = 0;
        }
    }

    Model model;

    uint nn = 0;
    for (uint i=0;i<nodeBook.size();i++)
    {
        if (nodeBook[i] == 0)
        {
            nodeBook[i] = nn++;
            model.addNode(this->getNode(i));
        }
    }

    for (int i=0;i<this->intersectedElements.size();i++)
    {
        uint elementID = this->intersectedElements[i];
        RElement element = this->getElement(elementID);
        for (uint j=0;j<element.size();j++)
        {
            element.setNodeId(j,nodeBook[element.getNodeId(j)]);
        }
        model.addElement(element,true,0);
    }

    model.setName(this->getName() + " - intersected");
    model.setDescription("Reuslt of element intersections");
    model.consolidate(Model::ConsolidateActionAll);

    Application::instance()->getSession()->addModel(model);

    RLogger::warning("Intersecting elements were found (%u).\n",model.getNElements());

    return true;
}

bool Model::boolDifference(uint nIterations, QList<uint> surfaceEntityIDs, uint cuttingSurfaceEntityId)
{
    bool success = this->RModel::boolDifference(nIterations,surfaceEntityIDs,cuttingSurfaceEntityId);

    this->consolidate(Model::ConsolidateActionAll);

    return success;
}

bool Model::boolIntersection(uint nIterations, QList<uint> surfaceEntityIDs)
{
    bool success = this->RModel::boolIntersection(nIterations,surfaceEntityIDs);

    this->consolidate(Model::ConsolidateActionAll);

    return success;
}

bool Model::boolUnion(uint nIterations, QList<uint> surfaceEntityIDs)
{
    bool success = this->RModel::boolUnion(nIterations,surfaceEntityIDs);

    this->consolidate(Model::ConsolidateActionAll);

    return success;
}

bool Model::isSliver(uint elementID) const
{
    return (this->sliverElements.indexOf(elementID) >= 0);
}

bool Model::isIntersected(uint elementID) const
{
    return (this->intersectedElements.indexOf(elementID) >= 0);
}

bool Model::isSelected(bool selected) const
{
    if (isEmpty())
    {
        return (this->getSelected() == selected);
    }
    if (!this->isSelected(R_ENTITY_GROUP_POINT,selected) && this->getNPoints() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_LINE,selected) && this->getNLines() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_SURFACE,selected) && this->getNSurfaces() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_VOLUME,selected) && this->getNVolumes() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_VECTOR_FIELD,selected) && this->getNVectorFields() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_SCALAR_FIELD,selected) && this->getNScalarFields() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_STREAM_LINE,selected) && this->getNStreamLines() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_CUT,selected) && this->getNCuts() > 0)
    {
        return false;
    }
    if (!this->isSelected(R_ENTITY_GROUP_ISO,selected) && this->getNIsos() > 0)
    {
        return false;
    }
    return true;
}

bool Model::isSelected(REntityGroupType elementGroupType,
                       bool             selected) const
{
    uint nEntities;
    switch (elementGroupType)
    {
        case R_ENTITY_GROUP_POINT:
            nEntities = this->getNPoints();
            break;
        case R_ENTITY_GROUP_LINE:
            nEntities = this->getNLines();
            break;
        case R_ENTITY_GROUP_SURFACE:
            nEntities = this->getNSurfaces();
            break;
        case R_ENTITY_GROUP_VOLUME:
            nEntities = this->getNVolumes();
            break;
        case R_ENTITY_GROUP_VECTOR_FIELD:
            nEntities = this->getNVectorFields();
            break;
        case R_ENTITY_GROUP_SCALAR_FIELD:
            nEntities = this->getNScalarFields();
            break;
        case R_ENTITY_GROUP_STREAM_LINE:
            nEntities = this->getNStreamLines();
            break;
        case R_ENTITY_GROUP_CUT:
            nEntities = this->getNCuts();
            break;
        case R_ENTITY_GROUP_ISO:
            nEntities = this->getNIsos();
            break;
        default:
            return false;
    }
    if (nEntities == 0)
    {
        return false;
    }
    for (uint i=0;i<nEntities;i++)
    {
        if (this->getSelected(elementGroupType,i) != selected)
        {
            return false;
        }
    }
    return true;
}

bool Model::getSelected() const
{
    return this->getData().getSelected();
}

bool Model::getSelected(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getSelected();
    }
    return false;
}

void Model::setSelected(bool selected)
{
    if (this->isEmpty())
    {
        this->getData().setSelected(selected);
        return;
    }
    this->getData().setSelected(false);

    for (uint i=0;i<this->getNPoints();i++)
    {
        this->setSelected(R_ENTITY_GROUP_POINT,i,selected);
    }
    for (uint i=0;i<this->getNLines();i++)
    {
        this->setSelected(R_ENTITY_GROUP_LINE,i,selected);
    }
    for (uint i=0;i<this->getNSurfaces();i++)
    {
        this->setSelected(R_ENTITY_GROUP_SURFACE,i,selected);
    }
    for (uint i=0;i<this->getNVolumes();i++)
    {
        this->setSelected(R_ENTITY_GROUP_VOLUME,i,selected);
    }
    for (uint i=0;i<this->getNVectorFields();i++)
    {
        this->setSelected(R_ENTITY_GROUP_VECTOR_FIELD,i,selected);
    }
    for (uint i=0;i<this->getNScalarFields();i++)
    {
        this->setSelected(R_ENTITY_GROUP_SCALAR_FIELD,i,selected);
    }
    for (uint i=0;i<this->getNStreamLines();i++)
    {
        this->setSelected(R_ENTITY_GROUP_STREAM_LINE,i,selected);
    }
    for (uint i=0;i<this->getNCuts();i++)
    {
        this->setSelected(R_ENTITY_GROUP_CUT,i,selected);
    }
    for (uint i=0;i<this->getNIsos();i++)
    {
        this->setSelected(R_ENTITY_GROUP_ISO,i,selected);
    }
}

void Model::setSelected(REntityGroupType elementGroupType, uint position, bool selected)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setSelected(selected);
    }
    if (!this->isSelected())
    {
        this->getData().setSelected(false);
    }
} /* Model::setSelected */

QList<SessionEntityID> Model::getSelectedEntityIDs(uint modelID) const
{
    QList<SessionEntityID> selectedEntityIDs;
    SessionEntityID entityID;

    entityID.setMid(modelID);

    entityID.setType(R_ENTITY_GROUP_POINT);
    for (uint j=0;j<this->getNPoints();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_POINT,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_LINE);
    for (uint j=0;j<this->getNLines();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_LINE,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_SURFACE);
    for (uint j=0;j<this->getNSurfaces();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_SURFACE,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_VOLUME);
    for (uint j=0;j<this->getNVolumes();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_VOLUME,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_VECTOR_FIELD);
    for (uint j=0;j<this->getNVectorFields();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_VECTOR_FIELD,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_SCALAR_FIELD);
    for (uint j=0;j<this->getNScalarFields();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_SCALAR_FIELD,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_STREAM_LINE);
    for (uint j=0;j<this->getNStreamLines();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_STREAM_LINE,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_CUT);
    for (uint j=0;j<this->getNCuts();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_CUT,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_ISO);
    for (uint j=0;j<this->getNIsos();j++)
    {
        if (this->getSelected(R_ENTITY_GROUP_ISO,j))
        {
            entityID.setEid(j);
            selectedEntityIDs.push_back(entityID);
        }
    }

    return selectedEntityIDs;
} /* Model::getSelectedEntityIDs(uint modelID) */

QList<SessionEntityID> Model::getPickedEntityIDs(uint modelID) const
{
    QList<SessionEntityID> pickedEntityIDs;

    const PickList &rPickList = Application::instance()->getSession()->getPickList();
    if (!rPickList.isEmpty())
    {
        QVector<PickItem> pickItems = rPickList.getItems(modelID);
        for (int i=0;i<pickItems.size();i++)
        {
            if (!pickedEntityIDs.contains(pickItems.at(i).getEntityID()))
            {
                pickedEntityIDs.append(pickItems.at(i).getEntityID());
            }
        }
    }

    return pickedEntityIDs;
} /* Model::getPickedEntityIDs(uint modelID) */

QList<SessionEntityID> Model::getVisibleEntityIDs(uint modelID) const
{
    QList<SessionEntityID> visibleEntityIDs;
    SessionEntityID entityID;

    entityID.setMid(modelID);

    entityID.setType(R_ENTITY_GROUP_POINT);
    for (uint j=0;j<this->getNPoints();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_POINT,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_LINE);
    for (uint j=0;j<this->getNLines();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_LINE,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_SURFACE);
    for (uint j=0;j<this->getNSurfaces();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_SURFACE,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_VOLUME);
    for (uint j=0;j<this->getNVolumes();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_VOLUME,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_VECTOR_FIELD);
    for (uint j=0;j<this->getNVectorFields();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_VECTOR_FIELD,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_SCALAR_FIELD);
    for (uint j=0;j<this->getNScalarFields();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_SCALAR_FIELD,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_STREAM_LINE);
    for (uint j=0;j<this->getNStreamLines();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_STREAM_LINE,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_CUT);
    for (uint j=0;j<this->getNCuts();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_CUT,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    entityID.setType(R_ENTITY_GROUP_ISO);
    for (uint j=0;j<this->getNIsos();j++)
    {
        if (this->getVisible(R_ENTITY_GROUP_ISO,j))
        {
            entityID.setEid(j);
            visibleEntityIDs.push_back(entityID);
        }
    }

    return visibleEntityIDs;
} /* Model::getVisibleEntityIDs */

QSet<uint> Model::getElementIDs(const QList<SessionEntityID> &entityIDs) const
{
    QSet<uint> elementIDs;

    elementIDs.reserve(int(this->getNElements()));
    for (int i=0;i<entityIDs.size();i++)
    {
        uint entityGroupId = this->getEntityGroupID(entityIDs[i].getType(),entityIDs[i].getEid(),true);
        if (entityGroupId == RConstants::eod)
        {
            continue;
        }
        const RElementGroup *pEelementGroup = static_cast<const RElementGroup*>(this->getEntityGroupPtr(entityGroupId,true));
        if (!pEelementGroup)
        {
            continue;
        }
        for (uint j=0;j<pEelementGroup->size();j++)
        {
            elementIDs.insert(pEelementGroup->get(j));
        }
    }

    return elementIDs;
} /* Model::getElementIDs */

QSet<uint> Model::getNodeIDs(const QSet<uint> &elementIDs) const
{
    QSet<uint> nodeIDs;

    QVector<bool> nodeBook;
    nodeBook.resize(int(this->getNNodes()));
    nodeBook.fill(false);

    foreach (uint elementID, elementIDs)
    {
        const RElement &rElement = this->getElement(elementID);
        for (uint i=0;i<rElement.size();i++)
        {
            nodeBook[int(rElement.getNodeId(i))] = true;
        }
    }

    uint nn = 0;
    for (int i=0;i<nodeBook.size();i++)
    {
        if (nodeBook[i])
        {
            nn++;
        }
    }

    nodeIDs.reserve(int(nn));

    for (int i=0;i<nodeBook.size();i++)
    {
        if (nodeBook[i])
        {
            nodeIDs.insert(uint(i));
        }
    }

    return nodeIDs;
} /* Model::getNodeIDs */


uint Model::getNSlivers() const
{
    return uint(this->sliverElements.size());
} /* Model::getNSlivers */


uint Model::getNIntersected() const
{
    return uint(this->intersectedElements.size());
} /* Model::getNIntersected */


uint Model::getNHoleElements() const
{
    return uint(this->holeElements.size());
} /* Model::getNHoleEdges */


bool Model::isVisible(bool visible) const
{
    if (!this->isVisible(R_ENTITY_GROUP_POINT,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_LINE,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_SURFACE,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_VOLUME,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_VECTOR_FIELD,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_SCALAR_FIELD,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_STREAM_LINE,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_CUT,visible))
    {
        return false;
    }
    if (!this->isVisible(R_ENTITY_GROUP_ISO,visible))
    {
        return false;
    }
    return true;
}

bool Model::isVisible(REntityGroupType elementGroupType,
                      bool              visible) const
{
    uint nEntities;
    switch (elementGroupType)
    {
        case R_ENTITY_GROUP_POINT:
            nEntities = this->getNPoints();
            break;
        case R_ENTITY_GROUP_LINE:
            nEntities = this->getNLines();
            break;
        case R_ENTITY_GROUP_SURFACE:
            nEntities = this->getNSurfaces();
            break;
        case R_ENTITY_GROUP_VOLUME:
            nEntities = this->getNVolumes();
            break;
        case R_ENTITY_GROUP_VECTOR_FIELD:
            nEntities = this->getNVectorFields();
            break;
        case R_ENTITY_GROUP_SCALAR_FIELD:
            nEntities = this->getNScalarFields();
            break;
        case R_ENTITY_GROUP_STREAM_LINE:
            nEntities = this->getNStreamLines();
            break;
        case R_ENTITY_GROUP_CUT:
            nEntities = this->getNCuts();
            break;
        case R_ENTITY_GROUP_ISO:
            nEntities = this->getNIsos();
            break;
        default:
            return false;
    }
    for (uint i=0;i<nEntities;i++)
    {
        if (this->getVisible(elementGroupType,i) != visible)
        {
            return false;
        }
    }
    return true;
}

bool Model::getVisible(REntityGroupType elementGroupType,
                       uint             position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getVisible();
    }
    return false;
}

void Model::setVisible(REntityGroupType elementGroupType,
                       uint             position,
                       bool             visible)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setVisible(visible);
    }
}

bool Model::getDrawWire(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawWire();
    }
    return false;
}

void Model::setDrawWire(REntityGroupType elementGroupType, uint position, bool drawWire)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawWire(drawWire);
    }
}

bool Model::getDrawEdges(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawEdges();
    }
    return false;
}

void Model::setDrawEdges(REntityGroupType elementGroupType, uint position, bool drawEdges)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawEdges(drawEdges);
    }
}

bool Model::getDrawNodes(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawNodes();
    }
    return false;
}

void Model::setDrawNodes(REntityGroupType elementGroupType, uint position, bool drawNodes)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawNodes(drawNodes);
    }
}

bool Model::getDrawElementNumbers(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawElementNumbers();
    }
    return false;
}

void Model::setDrawElementNumbers(REntityGroupType elementGroupType, uint position, bool drawElementNumbers)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawElementNumbers(drawElementNumbers);
    }
}

bool Model::getDrawNodeNumbers(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawNodeNumbers();
    }
    return false;
}

void Model::setDrawNodeNumbers(REntityGroupType elementGroupType, uint position, bool drawNodeNumbers)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawNodeNumbers(drawNodeNumbers);
    }
}

bool Model::getDrawArrowHeads(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawArrowHeads();
    }
    return false;
}

void Model::setDrawArrowHeads(REntityGroupType elementGroupType, uint position, bool drawArrowHeads)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawArrowHeads(drawArrowHeads);
    }
}

bool Model::getDrawEqualArrowLengths(REntityGroupType entityGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawEqualArrowLength();
    }
    return false;
}

void Model::setDrawEqualArrowLengths(REntityGroupType entityGroupType, uint position, bool drawEqualArrowLengths)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawEqualArrowLength(drawEqualArrowLengths);
    }
}

bool Model::getDrawArrowFrom(REntityGroupType entityGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getDrawArrowFrom();
    }
    return false;
}

void Model::setDrawArrowFrom(REntityGroupType entityGroupType, uint position, bool drawArrowFrom)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setDrawArrowFrom(drawArrowFrom);
    }
}

bool Model::getColorByPatch(REntityGroupType entityGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getColorByPatch();
    }
    return false;
}

void Model::setColorByPatch(REntityGroupType entityGroupType, uint position, bool colorByPatch)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setColorByPatch(colorByPatch);
    }
}

bool Model::getColorByViewFactor(REntityGroupType entityGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        return pElementGroupData->getColorByViewFactor();
    }
    return false;
}

void Model::setColorByViewFactor(REntityGroupType entityGroupType, uint position, bool colorByViewFactor)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(entityGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setColorByViewFactor(colorByViewFactor);
    }
}

QColor Model::getColor(REntityGroupType elementGroupType, uint position) const
{
    const REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        int r, g, b, a;
        pElementGroupData->getColor(r,g,b,a);
        return QColor(r,g,b,a);
    }
    else
    {
        return Color::random(true);
    }
}

void Model::setColor(REntityGroupType elementGroupType, uint position, const QColor &color)
{
    REntityGroupData *pElementGroupData = this->getElementGroupData(elementGroupType,position);
    if (pElementGroupData)
    {
        pElementGroupData->setColor(color.red(),color.green(),color.blue(),color.alpha());
    }
}

void Model::glDrawLock()
{
    R_LOG_TRACE_IN;
    this->drawLock.lock();
    R_LOG_TRACE_OUT;
}

bool Model::glDrawTrylock()
{
    R_LOG_TRACE;
    return this->drawLock.tryLock();
}

void Model::glDrawUnlock()
{
    R_LOG_TRACE_IN;
    this->drawLock.unlock();
    R_LOG_TRACE_OUT;
}

void Model::glDraw(GLWidget *glWidget) const
{
    R_LOG_TRACE_IN;
    try
    {
        // Reset OpenGL list sizes.
        glWidget->getGLModelList().setNGlPointLists(this->getNPoints());
        glWidget->getGLModelList().setNGlLineLists(this->getNLines());
        glWidget->getGLModelList().setNGlSurfaceLists(this->getNSurfaces());
        glWidget->getGLModelList().setNGlVolumeLists(this->getNVolumes());
        glWidget->getGLModelList().setNGlVectorFieldLists(this->getNVectorFields());
        glWidget->getGLModelList().setNGlScalarFieldLists(this->getNScalarFields());
        glWidget->getGLModelList().setNGlStreamLineLists(this->getNStreamLines());
        glWidget->getGLModelList().setNGlCutLists(this->getNCuts());
        glWidget->getGLModelList().setNGlIsoLists(this->getNIsos());

        GLint depthFunc;
        GL_SAFE_CALL(glGetIntegerv(GL_DEPTH_FUNC, &depthFunc));
        GL_SAFE_CALL(glDepthFunc(GL_LEQUAL));

        uint modelID = Application::instance()->getSession()->findModelByPtr(this);

        // Draw volume entities.
        for (uint i=0;i<this->getNVolumes();i++)
        {
            GLElementGroup glElementGroup(glWidget,this->getVolume(i),SessionEntityID(modelID,R_ENTITY_GROUP_VOLUME,i));
            glElementGroup.setParentModel(this);
            glElementGroup.setUseGlList(true);
            glElementGroup.paint();
        }

        // Draw surface entities.
        for (uint i=0;i<this->getNSurfaces();i++)
        {
            GLElementGroup glElementGroup(glWidget,this->getSurface(i),SessionEntityID(modelID,R_ENTITY_GROUP_SURFACE,i));
            glElementGroup.setParentModel(this);
            glElementGroup.setSurfaceThickness(this->getSurface(i).getThickness());
            glElementGroup.setUseGlList(true);
            glElementGroup.setUseGlCullFace(glWidget->getUseGlCullFace());
            glElementGroup.paint();
        }

        // Draw line entities.
        for (uint i=0;i<this->getNLines();i++)
        {
            GLElementGroup glElementGroup(glWidget,this->getLine(i),SessionEntityID(modelID,R_ENTITY_GROUP_LINE,i));
            glElementGroup.setParentModel(this);
            glElementGroup.setLineCrossArea(this->getLine(i).getCrossArea());
            glElementGroup.setUseGlList(true);
            glElementGroup.paint();
        }

        // Draw point entities.
        for (uint i=0;i<this->getNPoints();i++)
        {
            GLElementGroup glElementGroup(glWidget,this->getPoint(i),SessionEntityID(modelID,R_ENTITY_GROUP_POINT,i));
            glElementGroup.setParentModel(this);
            glElementGroup.setPointVolume(this->getPoint(i).getVolume());
            glElementGroup.setUseGlList(true);
            glElementGroup.paint();
        }

        // Draw vector field entities.
        for (uint i=0;i<this->getNVectorFields();i++)
        {
            GLVectorField glVectorField(glWidget,this->getVectorField(i),SessionEntityID(modelID,R_ENTITY_GROUP_VECTOR_FIELD,i));
            glVectorField.setApplyEnvironmentSettings(false);
            glVectorField.setParentModel(this);
            glVectorField.setUseGlList(true);
            glVectorField.paint();
        }

        // Draw scalar field entities.
        for (uint i=0;i<this->getNScalarFields();i++)
        {
            GLScalarField glScalarField(glWidget,this->getScalarField(i),SessionEntityID(modelID,R_ENTITY_GROUP_SCALAR_FIELD,i));
            glScalarField.setApplyEnvironmentSettings(false);
            glScalarField.setParentModel(this);
            glScalarField.setUseGlList(true);
            glScalarField.paint();
        }

        // Draw stream line entities.
        for (uint i=0;i<this->getNStreamLines();i++)
        {
            GLInterpolatedEntity glStreamLine(glWidget,this->getStreamLine(i),SessionEntityID(modelID,R_ENTITY_GROUP_STREAM_LINE,i));
            glStreamLine.setApplyEnvironmentSettings(false);
            glStreamLine.setParentModel(this);
            glStreamLine.setUseGlList(true);
            glStreamLine.paint();
        }

        // Draw cuts entities.
        for (uint i=0;i<this->getNCuts();i++)
        {
            GLInterpolatedEntity glCut(glWidget,this->getCut(i),SessionEntityID(modelID,R_ENTITY_GROUP_CUT,i));
            glCut.setApplyEnvironmentSettings(false);
            glCut.setParentModel(this);
            glCut.setUseGlList(true);
            glCut.paint();
        }

        // Draw isos entities.
        for (uint i=0;i<this->getNIsos();i++)
        {
            GLInterpolatedEntity glIso(glWidget,this->getIso(i),SessionEntityID(modelID,R_ENTITY_GROUP_ISO,i));
            glIso.setApplyEnvironmentSettings(false);
            glIso.setParentModel(this);
            glIso.setUseGlList(true);
            glIso.paint();
        }

        if (glWidget->getGLDisplayProperties().getShowModelEdges())
        {
            // Draw edge elements
            int cValue = qGray(glWidget->getGLDisplayProperties().getBgColor().rgb()) < 128 ? 255 : 0;
            QColor edgeColor(cValue,cValue,cValue);

            // Draw elements only in normal mode
            for (int i=0;i<this->edgeElements.size();i++)
            {
                const RNode &node1 = this->getNode(this->edgeElements[i].getNodeId(0));
                const RNode &node2 = this->getNode(this->edgeElements[i].getNodeId(1));

                glWidget->qglColor(edgeColor);

                GLLine line(glWidget,node1.toVector(),node2.toVector(),2.0);
                line.paint();
            }
        }

        if (glWidget->getGLDisplayProperties().getShowErrors())
        {
            // Draw hole edges.
            for (int i=0;i<this->holeElements.size();i++)
            {
                const RNode &node1 = this->getNode(this->holeElements[i].getNodeId(0));
                const RNode &node2 = this->getNode(this->holeElements[i].getNodeId(1));
                glWidget->qglColor(QColor(255,0,0));
                GLLine(glWidget,node1.toVector(),node2.toVector(),2.0).paint();
            }

            //Draw sliver elements.
            REntityGroupData sliverGroupData;
            sliverGroupData.setDrawWire(true);
            for (int i=0;i<this->sliverElements.size();i++)
            {
                GLElement glElement(glWidget,
                                    this,
                                    this->sliverElements[i],
                                    sliverGroupData,
                                    QColor(255,100,0),
                                    GL_ELEMENT_DRAW_NORMAL);
                glElement.setApplyEnvironmentSettings(false);
                glElement.setEdgeLineWidth(3.0f);
                glElement.paint();
            }

            //Draw intersected elements.
            REntityGroupData intersectedGroupData;
            intersectedGroupData.setDrawWire(true);
            for (int i=0;i<this->intersectedElements.size();i++)
            {
                GLElement glElement(glWidget,
                                    this,
                                    this->intersectedElements[i],
                                    intersectedGroupData,
                                    Qt::red,
                                    GL_ELEMENT_DRAW_NORMAL);
                glElement.setApplyEnvironmentSettings(false);
                glElement.setEdgeLineWidth(3.0f);
                glElement.paint();
            }
        }

        GL_SAFE_CALL(glDepthFunc(GLenum(depthFunc)));
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to draw model. %s\n",error.getMessage().toUtf8().constData());
    }
    catch (const std::exception &error)
    {
        RLogger::error("Failed to draw model. %s\n",error.what());
    }
    catch (...)
    {
        RLogger::error("Failed to draw model. Unknown exception\n");
    }
    R_LOG_TRACE_OUT;
}

void Model::glDraw(GLWidget *glWidget, const QVector<PickItem> &pickedItems) const
{
    R_LOG_TRACE_IN;
    try
    {
        GLint depthFunc;
        GLboolean lightingEnabled;
        GLfloat pointSize;

        GL_SAFE_CALL(glGetBooleanv(GL_LIGHTING, &lightingEnabled));
        GL_SAFE_CALL(glGetIntegerv(GL_DEPTH_FUNC, &depthFunc));
        GL_SAFE_CALL(glGetFloatv(GL_POINT_SIZE, &pointSize));

        GL_SAFE_CALL(glDisable(GL_LIGHTING));
        GL_SAFE_CALL(glDepthFunc(GL_LEQUAL));
        GL_SAFE_CALL(glPointSize(10.0));

        for (int i=0;i<pickedItems.size();i++)
        {
            PickItemType itemType = pickedItems[i].getItemType();
            REntityGroupType entityType = pickedItems[i].getEntityID().getType();
            uint entityID = pickedItems[i].getEntityID().getEid();
            uint elementPosition = pickedItems[i].getElementPosition();
            uint elementID = pickedItems[i].getElementID();
            uint nodePosition = pickedItems[i].getNodePosition();
            uint nodeID = pickedItems[i].getNodeID();

            if (itemType == PICK_ITEM_HOLE_ELEMENT)
            {
                glWidget->qglColor(QColor(Qt::white));

                const RNode &node1 = this->getNode(this->holeElements[int(elementPosition)].getNodeId(0));
                const RNode &node2 = this->getNode(this->holeElements[int(elementPosition)].getNodeId(1));

                GLLine line(glWidget,node1.toVector(),node2.toVector(),4.0);

                line.paint();

                continue;
            }

            uint groupID = this->getEntityGroupID(entityType,entityID);

            const REntityGroup *pEntity = this->getEntityGroupPtr(groupID);

            if (!pEntity)
            {
                continue;
            }

            uint displacementVarPosition = this->findVariable(pEntity->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT));
            const RVariable *pDisplacementVariable = nullptr;
            if (displacementVarPosition != RConstants::eod)
            {
                pDisplacementVariable = &this->getVariable(displacementVarPosition);
            }

            if (itemType == PICK_ITEM_ELEMENT)
            {
                REntityGroupData groupData;
                groupData.setColor(255,255,255,200);
                groupData.setDrawEdges(false);
                groupData.setDrawElementNumbers(false);
                groupData.setDrawNodeNumbers(false);
                groupData.setDrawNodes(false);
                groupData.setDrawWire(false);


                if (REntityGroup::typeIsElementGroup(entityType))
                {
                    int r,g,b,a;
                    groupData.getColor(r,g,b,a);
                    GLElement glElement(glWidget,
                                        this,
                                        elementID,
                                        groupData,
                                        QColor(r,g,b,a),
                                        GL_ELEMENT_DRAW_NORMAL);
                    glElement.setDisplacementVariable(pDisplacementVariable);
                    glElement.setApplyEnvironmentSettings(false);
                    switch (entityType)
                    {
                        case R_ENTITY_GROUP_POINT:
                        {
                            glElement.setPointVolume(this->getPoint(entityID).getVolume());
                            break;
                        }
                        case R_ENTITY_GROUP_LINE:
                        {
                            glElement.setLineCrossArea(this->getLine(entityID).getCrossArea());
                            break;
                        }
                        case R_ENTITY_GROUP_SURFACE:
                        {
                            glElement.setSurfaceThickness(this->getSurface(entityID).getThickness());
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }

                    glElement.paint();
                }
                else if (REntityGroup::typeIsInterpolatedElementGroup(entityType))
                {
                    const RInterpolatedEntity *pIEntity = nullptr;

                    if (entityType == R_ENTITY_GROUP_CUT || entityType == R_ENTITY_GROUP_ISO)
                    {
                        pIEntity = static_cast<const RInterpolatedEntity*>(pEntity);
                    }
                    else
                    {
                        continue;
                    }

                    GLInterpolatedElement glElement(glWidget,
                                                    this,
                                                    pIEntity->at(elementPosition),
                                                    elementPosition,
                                                    groupData,
                                                    GL_ELEMENT_DRAW_NORMAL);
                    glElement.setDisplacementVariable(pDisplacementVariable);
                    glElement.setApplyEnvironmentSettings(false);
                    glElement.paint();
                }
            }
            else if (itemType == PICK_ITEM_NODE)
            {
                glWidget->qglColor(QColor(Qt::white));
                bool nodeFound = false;
                RNode node;

                if (REntityGroup::typeIsElementGroup(entityType))
                {
                    const RElement &rElement = this->getElement(elementID);
                    node = this->getNode(nodeID);
                    nodeFound = true;
                    if (pDisplacementVariable)
                    {
                        std::vector<RR3Vector> displacementValues;
                        rElement.findDisplacementNodeValues(elementID,*pDisplacementVariable,displacementValues);
                        node.move(displacementValues[nodePosition]);
                    }
                }
                else if (REntityGroup::typeIsInterpolatedElementGroup(entityType))
                {
                    const RInterpolatedElement *pIElement = nullptr;
                    switch (entityType)
                    {
                        case R_ENTITY_GROUP_CUT:
                        {
                            pIElement = &this->getCut(entityID).at(elementPosition);
                            break;
                        }
                        case R_ENTITY_GROUP_ISO:
                        {
                            pIElement = &this->getIso(entityID).at(elementPosition);
                            break;
                        }
                        default:
                        {
                            continue;
                        }
                    }
                    if (pIElement)
                    {
                        node = pIElement->at(nodePosition);
                        nodeFound = true;
                        if (pDisplacementVariable)
                        {
                            std::vector<RR3Vector> displacementValues;
                            pIElement->findDisplacementNodeValues(this->getNodes(),this->getElements(),*pDisplacementVariable,displacementValues);
                            node.move(displacementValues[nodePosition]);
                        }
                    }
                }

                if (nodeFound)
                {
                    GLFunctions::begin(GL_POINTS);
                    GLObject::glVertexNode(node);
                    GLFunctions::end();
                }
            }
        }

        if (lightingEnabled)
        {
            GL_SAFE_CALL(glEnable(GL_LIGHTING));
        }
        GL_SAFE_CALL(glDepthFunc(GLenum(depthFunc)));
        GL_SAFE_CALL(glPointSize(pointSize));
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to draw model. %s\n",error.getMessage().toUtf8().constData());
    }
    catch (const std::exception &error)
    {
        RLogger::error("Failed to draw model. %s\n",error.what());
    }
    catch (...)
    {
        RLogger::error("Failed to draw model. Unknown exception.\n");
    }
    R_LOG_TRACE_OUT;
}

QMultiMap <RVariableType, PickValue> Model::getPickedResultsValues(const PickItem &rPickItem) const
{
    R_LOG_TRACE_IN;
    QMultiMap<RVariableType, PickValue> resultsValues;

    for (uint i=0;i<this->getNVariables();i++)
    {
        const RVariable &rVariable = this->getVariable(i);
        const RVariable *pDisplacementVariable = this->findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT,rPickItem.getEntityID().getType(),rPickItem.getEntityID().getEid());

        if (rPickItem.getItemType() == PICK_ITEM_ELEMENT)
        {
            std::vector<RRVector> resultsValuesVector;
            std::vector<RR3Vector> positionVector;
            if (REntityGroup::typeIsElementGroup(rPickItem.getEntityID().getType()))
            {
                uint elementID = rPickItem.getElementID();
                if (elementID == RConstants::eod)
                {
                    continue;
                }

                resultsValuesVector = this->getElementResultsValues(rVariable.getType(),elementID);

                const RElement &rElement = this->getElement(elementID);
                std::vector<RR3Vector> displacementValues;
                if (pDisplacementVariable)
                {
                    rElement.findDisplacementNodeValues(elementID,*pDisplacementVariable,displacementValues);
                }
                else
                {
                    displacementValues.resize(rElement.size(),RR3Vector(0.0,0.0,0.0));
                }
                if (rVariable.getApplyType() == R_VARIABLE_APPLY_ELEMENT)
                {
                    positionVector.resize(1);
                    positionVector[0] = RR3Vector(0.0,0.0,0.0);
                    for (uint j=0;j<rElement.size();j++)
                    {
                        positionVector[0][0] += this->getNode(rElement.getNodeId(j)).getX() + displacementValues[j][0];
                        positionVector[0][1] += this->getNode(rElement.getNodeId(j)).getY() + displacementValues[j][1];
                        positionVector[0][2] += this->getNode(rElement.getNodeId(j)).getZ() + displacementValues[j][2];
                    }
                    positionVector[0] *= 1.0 / double(rElement.size());
                }
                else if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
                {
                    positionVector.resize(rElement.size());
                    for (uint j=0;j<rElement.size();j++)
                    {
                        positionVector[j][0] = this->getNode(rElement.getNodeId(j)).getX() + displacementValues[j][0];
                        positionVector[j][1] = this->getNode(rElement.getNodeId(j)).getY() + displacementValues[j][1];
                        positionVector[j][2] = this->getNode(rElement.getNodeId(j)).getZ() + displacementValues[j][2];
                    }
                }
                else
                {
                    positionVector.resize(resultsValuesVector.size());
                }
            }
            else if (REntityGroup::typeIsInterpolatedElementGroup(rPickItem.getEntityID().getType()))
            {
                resultsValuesVector = this->getInterpolatedElementResultsValues(rVariable.getType(),
                                                                                rPickItem.getEntityID().getType(),
                                                                                rPickItem.getEntityID().getEid(),
                                                                                rPickItem.getElementPosition());
                const RInterpolatedElement *pIElement = this->getInterpolatedElement(rPickItem.getEntityID().getType(),
                                                                                     rPickItem.getEntityID().getEid(),
                                                                                     rPickItem.getElementPosition());
                std::vector<RR3Vector> displacementValues;
                if (pDisplacementVariable)
                {
                    pIElement->findDisplacementNodeValues(this->getNodes(),this->getElements(),*pDisplacementVariable,displacementValues);
                }
                else
                {
                    displacementValues.resize(pIElement->size(),RR3Vector(0.0,0.0,0.0));
                }
                if (rVariable.getApplyType() == R_VARIABLE_APPLY_ELEMENT)
                {
                    positionVector.resize(1);
                    positionVector[0] = RR3Vector(0.0,0.0,0.0);
                    for (uint j=0;j<pIElement->size();j++)
                    {
                        positionVector[0][0] += pIElement->at(j).getX() + displacementValues[j][0];
                        positionVector[0][1] += pIElement->at(j).getY() + displacementValues[j][1];
                        positionVector[0][2] += pIElement->at(j).getZ() + displacementValues[j][2];
                    }
                    positionVector[0] *= 1.0 / double(pIElement->size());
                }
                else if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
                {
                    positionVector.resize(pIElement->size());
                    for (uint j=0;j<pIElement->size();j++)
                    {
                        positionVector[j][0] = pIElement->at(j).getX() + displacementValues[j][0];
                        positionVector[j][1] = pIElement->at(j).getY() + displacementValues[j][1];
                        positionVector[j][2] = pIElement->at(j).getZ() + displacementValues[j][2];
                    }
                }
                else
                {
                    positionVector.resize(resultsValuesVector.size());
                }
            }
            for (uint j=0;j<resultsValuesVector.size();j++)
            {
                resultsValues.insert(rVariable.getType(),PickValue(positionVector[j],resultsValuesVector[j]));
            }
        }
        else if (rPickItem.getItemType() == PICK_ITEM_NODE)
        {
            RRVector resultsValuesVector;
            RR3Vector position;
            if (REntityGroup::typeIsElementGroup(rPickItem.getEntityID().getType()))
            {
                RR3Vector displacement(0.0,0.0,0.0);
                if (pDisplacementVariable)
                {
                    uint elementID = rPickItem.getElementID();
                    if (elementID != RConstants::eod)
                    {
                        const RElement &rElement = this->getElement(elementID);
                        std::vector<RR3Vector> displacementNodeValues;
                        rElement.findDisplacementNodeValues(elementID,*pDisplacementVariable,displacementNodeValues);
                        displacement = displacementNodeValues[rPickItem.getNodePosition()];
                    }
                }

                uint nodeID = rPickItem.getNodeID();
                if (nodeID == RConstants::eod)
                {
                    continue;
                }
                resultsValuesVector = this->getNodeResultsValues(rVariable.getType(),nodeID);
                position[0] = this->getNode(nodeID).getX() + displacement[0];
                position[1] = this->getNode(nodeID).getY() + displacement[1];
                position[2] = this->getNode(nodeID).getZ() + displacement[2];
            }
            else if (REntityGroup::typeIsInterpolatedElementGroup(rPickItem.getEntityID().getType()))
            {
                RR3Vector displacement(0.0,0.0,0.0);
                if (pDisplacementVariable)
                {
                    const RInterpolatedElement *pIElement = this->getInterpolatedElement(rPickItem.getEntityID().getType(),
                                                                                         rPickItem.getEntityID().getEid(),
                                                                                         rPickItem.getElementPosition());
                    if (pIElement)
                    {
                        std::vector<RR3Vector> displacementNodeValues;
                        pIElement->findDisplacementNodeValues(this->getNodes(),this->getElements(),*pDisplacementVariable,displacementNodeValues);
                        displacement = displacementNodeValues[rPickItem.getNodePosition()];
                    }
                }

                resultsValuesVector = this->getInterpolatedNodeResultsValues(rVariable.getType(),
                                                                             rPickItem.getEntityID().getType(),
                                                                             rPickItem.getEntityID().getEid(),
                                                                             rPickItem.getElementPosition(),
                                                                             rPickItem.getNodePosition());
                const RInterpolatedElement *pIElement = this->getInterpolatedElement(rPickItem.getEntityID().getType(),
                                                                                     rPickItem.getEntityID().getEid(),
                                                                                     rPickItem.getElementPosition());
                position[0] = pIElement->at(rPickItem.getNodePosition()).getX() + displacement[0];
                position[1] = pIElement->at(rPickItem.getNodePosition()).getY() + displacement[1];
                position[2] = pIElement->at(rPickItem.getNodePosition()).getZ() + displacement[2];
            }
            resultsValues.insert(rVariable.getType(),PickValue(position,resultsValuesVector));
        }
    }

    R_LOG_TRACE_OUT;
    return resultsValues;
}

bool Model::nodeIsOnEdge(uint nodeID) const
{
    R_LOG_TRACE;
    return (this->edgeNodes[int(nodeID)]);
}

bool Model::elementIsOnEdge(uint elementID) const
{
    const RElement &element = this->getElement(elementID);
    REntityGroupType grpType = RElementGroup::getGroupType(element.getType());
    unsigned int nEdgeNodes = 0;

    switch (grpType)
    {
        case R_ENTITY_GROUP_POINT:
            return true;
        case R_ENTITY_GROUP_LINE:
            return true;
        case R_ENTITY_GROUP_SURFACE:
            return true;
        case R_ENTITY_GROUP_VOLUME:
            nEdgeNodes = 3;
            break;
        default:
            return false;
    }

    unsigned nFoundEdgeNodes = 0;
    for (unsigned int i=0;i<element.size();i++)
    {
        if (this->edgeNodes[int(element.getNodeId(i))])
        {
            nFoundEdgeNodes++;
        }
        if (nFoundEdgeNodes >= nEdgeNodes)
        {
            return true;
        }
    }
    return false;
}

bool Model::findPickedElement(const RR3Vector &position, const RR3Vector &direction, double tolerance, PickItem &pickItem)
{
    double minDistance = 0.0;
    bool found = false;

    std::vector<RNode> dispNodes(this->getNodes());

#pragma omp parallel default(shared)
    {
        for (uint i=0;i<this->getNEntityGroups();i++)
        {
            REntityGroupType entityType = this->getEntityGroupType(i);
            uint entityID = this->getEntityGroupPosition(i);

            const REntityGroup *pEntity = this->getEntityGroupPtr(i);
            if (!pEntity)
            {
                continue;
            }
            if (!pEntity->getData().getVisible())
            {
                continue;
            }

            uint displacementVarPosition = this->findVariable(pEntity->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT));
            const RVariable *pDisplacementVariable = nullptr;
            if (displacementVarPosition != RConstants::eod)
            {
                pDisplacementVariable = &this->getVariable(displacementVarPosition);
            }

            if (REntityGroup::typeIsElementGroup(entityType))
            {
                const RElementGroup *pElementGroup = static_cast<const RElementGroup*>(pEntity);

#pragma omp barrier
#pragma omp for
                for (int64_t j=0;j<int64_t(pElementGroup->size());j++)
                {
                    uint elementID = pElementGroup->get(uint(j));
                    const RElement &rElement = this->getElement(elementID);

                    if (pDisplacementVariable)
                    {
                        std::vector<RR3Vector> displacementValues;
                        rElement.findDisplacementNodeValues(elementID,*pDisplacementVariable,displacementValues);
                        for (uint k=0;k<rElement.size();k++)
                        {
                            uint nodeID = rElement.getNodeId(k);
                            dispNodes[nodeID].setX(this->getNode(nodeID).getX() + displacementValues[k][0]);
                            dispNodes[nodeID].setY(this->getNode(nodeID).getY() + displacementValues[k][1]);
                            dispNodes[nodeID].setZ(this->getNode(nodeID).getZ() + displacementValues[k][2]);
                        }
                    }

                    double distance;
                    if (rElement.findPickDistance(dispNodes,position,direction,tolerance,distance))
                    {
                        if (!found || minDistance > distance)
                        {
#pragma omp critical
                            {
                                minDistance = distance;
                                pickItem = PickItem(SessionEntityID(0,entityType,entityID),elementID,uint(j));
                                found = true;
                            }
                        }
                    }
                }
            }
            else if (REntityGroup::typeIsInterpolatedElementGroup(entityType))
            {
                const RInterpolatedEntity *pIEntity = static_cast<const RInterpolatedEntity*>(pEntity);

                std::vector<RR3Vector> nodeDisplacementValues;

#pragma omp barrier
#pragma omp for
                for (int64_t j=0;j<int64_t(pIEntity->size());j++)
                {
                    RInterpolatedElement iElement(pIEntity->at(uint(j)));

                    if (pDisplacementVariable)
                    {
                        iElement.findDisplacementNodeValues(this->getNodes(),this->getElements(),*pDisplacementVariable,nodeDisplacementValues);
                        for (uint k=0;k<iElement.size();k++)
                        {
                            iElement[k].move(nodeDisplacementValues[k]);
                        }
                    }

                    double distance;
                    if (iElement.findPickDistance(position,direction,tolerance,distance))
                    {
                        if (!found || minDistance > distance)
                        {
#pragma omp critical
                            {
                                minDistance = distance;
                                pickItem = PickItem(SessionEntityID(0,entityType,entityID),uint(j),uint(j));
                                found = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return found;
}

bool Model::findPickedNode(const RR3Vector &position, const RR3Vector &direction, double tolerance, PickItem &pickItem)
{
    double minDistance = 0.0;
    bool found = false;

    std::vector<RNode> dispNodes(this->getNodes());

    RSegment ray(RNode(position),RNode(position[0]+direction[0],position[1]+direction[1],position[2]+direction[2]));

#pragma omp parallel default(shared)
    {
        for (uint i=0;i<this->getNEntityGroups();i++)
        {
            REntityGroupType entityType = this->getEntityGroupType(i);
            uint entityID = this->getEntityGroupPosition(i);

            const REntityGroup *pEntity = this->getEntityGroupPtr(i);
            if (!pEntity)
            {
                continue;
            }
            if (!pEntity->getData().getVisible())
            {
                continue;
            }

            uint displacementVarPosition = this->findVariable(pEntity->getData().findVariableByDisplayType(R_ENTITY_GROUP_VARIABLE_DISPLAY_DISPLACEMENT));
            const RVariable *pDisplacementVariable = nullptr;
            if (displacementVarPosition != RConstants::eod)
            {
                pDisplacementVariable = &this->getVariable(displacementVarPosition);
            }

            if (REntityGroup::typeIsElementGroup(entityType))
            {
                const RElementGroup *pElementGroup = static_cast<const RElementGroup*>(pEntity);

#pragma omp barrier
#pragma omp for
                for (int64_t j=0;j<int64_t(pElementGroup->size());j++)
                {
                    uint elementID = pElementGroup->get(uint(j));
                    const RElement &rElement = this->getElement(elementID);

                    if (pDisplacementVariable)
                    {
                        std::vector<RR3Vector> displacementValues;
                        rElement.findDisplacementNodeValues(elementID,*pDisplacementVariable,displacementValues);
                        for (uint k=0;k<rElement.size();k++)
                        {
                            uint nodeID = rElement.getNodeId(k);
                            dispNodes[nodeID].setX(this->getNode(nodeID).getX() + displacementValues[k][0]);
                            dispNodes[nodeID].setY(this->getNode(nodeID).getY() + displacementValues[k][1]);
                            dispNodes[nodeID].setZ(this->getNode(nodeID).getZ() + displacementValues[k][2]);
                        }
                    }

                    for (uint k=0;k<rElement.size();k++)
                    {
                        uint nodeID = rElement.getNodeId(k);
                        const RNode &node = dispNodes[nodeID];

                        double u = ray.findPointDistance(node.toVector());
                        if (u <= tolerance)
                        {
                            double distance = node.getDistance(RNode(position));
                            if (!found || minDistance > distance)
                            {
#pragma omp critical
                                {
                                    minDistance = distance;
                                    pickItem = PickItem(SessionEntityID(0,entityType,entityID),elementID,uint(j),nodeID,k);
                                    found = true;
                                }
                            }
                        }
                    }
                }
            }
            else if (REntityGroup::typeIsInterpolatedElementGroup(entityType))
            {
                const RInterpolatedEntity *pIEntity = static_cast<const RInterpolatedEntity*>(pEntity);

                std::vector<RR3Vector> nodeDisplacementValues;

#pragma omp barrier
#pragma omp for
                for (int64_t j=0;j<int64_t(pIEntity->size());j++)
                {
                    RInterpolatedElement iElement(pIEntity->at(uint(j)));

                    if (pDisplacementVariable)
                    {
                        iElement.findDisplacementNodeValues(this->getNodes(),this->getElements(),*pDisplacementVariable,nodeDisplacementValues);
                        for (uint k=0;k<iElement.size();k++)
                        {
                            iElement[k].move(nodeDisplacementValues[k]);
                        }
                    }

                    for (uint k=0;k<iElement.size();k++)
                    {
                        double u = ray.findPointDistance(iElement[k].toVector());
                        if (u <= tolerance)
                        {
                            double distance = iElement[k].getDistance(RNode(position));
                            if (!found || minDistance > distance)
                            {
#pragma omp critical
                                {
                                    minDistance = distance;
                                    pickItem = PickItem(SessionEntityID(0,entityType,entityID),uint(j),uint(j),k,k);
                                    found = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return found;
}

bool Model::findPickedHoleElement(const RR3Vector &position, const RR3Vector &direction, double tolerance, PickItem &pickItem)
{
    bool found = false;
    double minDistance = 0.0;

    for (uint i=0;i<uint(this->holeElements.size());i++)
    {
        double distance;
        if (this->holeElements[int(i)].findPickDistance(this->getNodes(),position,direction,tolerance,distance))
        {
            if (!found || distance < minDistance)
            {
                minDistance = distance;
                pickItem = PickItem(SessionEntityID(0,R_ENTITY_GROUP_NONE,RConstants::eod),i,i);
                found = true;
            }
        }
    }
    return found;
}

void Model::update(const RModel &rModel)
{
    RModel::update(rModel);

    int consolidateActionMask = Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements;
    if (this->getNVolumes() == 0)
    {
        consolidateActionMask |= Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements;
    }
    this->consolidate(consolidateActionMask);
    // Fix missing color scales in variable data.
    for (uint i=0;i<this->getNVariables();i++)
    {
        if (this->getVariable(i).getVariableData().getValueRangeName().isEmpty())
        {
            this->getVariable(i).getVariableData().setValueRangeName(ColorScale::getDefaultColorScale());
        }
    }
}

void Model::read(const QString &fileName)
{
    RModel::read(fileName);
    this->setFileName(fileName);
    if (this->getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_RADIATIVE_HEAT)
    {
        try
        {
            this->loadViewFactorMatrix();
        }
        catch (const RError &error)
        {
            RLogger::error("Failed to load view-factor matrix file. %s\n",error.getMessage().toUtf8().constData());
        }
    }
    else
    {
        this->unloadViewFactorMatrix();
    }
    int consolidateActionMask = Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements;
    if (this->getNVolumes() == 0)
    {
        consolidateActionMask |= Model::ConsolidateSliverElements | Model::ConsolidateIntersectedElements;
    }
    this->consolidate(consolidateActionMask);
    // Fix missing color scales in variable data.
    for (uint i=0;i<this->getNVariables();i++)
    {
        if (this->getVariable(i).getVariableData().getValueRangeName().isEmpty())
        {
            this->getVariable(i).getVariableData().setValueRangeName(ColorScale::getDefaultColorScale());
        }
    }
}

void Model::write(const QString &fileName, bool writeLinkFile)
{
    this->setFileName(RModel::write(fileName,writeLinkFile));
}

void Model::loadViewFactorMatrix()
{
    this->viewFactorMatrix.clear();

    QString viewFactorMatrixFile = this->getProblemSetup().getRadiationSetup().getViewFactorMatrixFile();
    if (viewFactorMatrixFile.isEmpty())
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"Missing view-factor matrix file name.");
    }

    if (!RFileManager::fileExists(viewFactorMatrixFile))
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"View-factor matrix file \'%s\' does not exist.",viewFactorMatrixFile.toUtf8().constData());
    }

    QString recentViewFactorMatrixFile = RRadiationSetup::findRecentViewFactorMatrixFile(viewFactorMatrixFile,
                                                                                         this->getTimeSolver().getCurrentTimeStep());

    if (recentViewFactorMatrixFile.isEmpty())
    {
        this->viewFactorMatrix.getHeader().setHemicubeResolution(this->getProblemSetup().getRadiationSetup().getResolution());
        this->generatePatchInputVector(this->viewFactorMatrix.getHeader().getPatchInput());
    }
    else
    {
        RLogger::info("Reading view-factor matrix from file \'%s\'\n",recentViewFactorMatrixFile.toUtf8().constData());

        try
        {
            this->viewFactorMatrix.read(recentViewFactorMatrixFile);
        }
        catch (const RError &error)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Failed to read view factor matrix from file \'%s\'. %s",
                         recentViewFactorMatrixFile.toUtf8().constData(),
                         error.getMessage().toUtf8().constData());
        }
    }
}

void Model::unloadViewFactorMatrix()
{
    this->viewFactorMatrix.clear();
}

void Model::consolidate(int consolidateActionMask)
{

    RLogger::info("Consolidating model\n");
    RLogger::indent();
    try
    {
        if (consolidateActionMask & Model::ConsolidateSurfaceNeighbors || this->getNElements() != this->surfaceNeigs.size())
        {
            this->setSurfaceNeighbors(this->findSurfaceNeighbors());
            this->syncSurfaceNormals();
            consolidateActionMask |= Model::ConsolidateMeshInput;
            consolidateActionMask |= Model::ConsolidateEdgeElements;
            consolidateActionMask |= Model::ConsolidateHoleElements;
            consolidateActionMask |= Model::ConsolidateSliverElements;
        }
        if (consolidateActionMask & Model::ConsolidateVolumeNeighbors || this->getNElements() != this->volumeNeigs.size())
        {
            this->setVolumeNeighbors(this->findVolumeNeighbors());
            consolidateActionMask |= Model::ConsolidateMeshInput;
        }
        if (consolidateActionMask & Model::ConsolidateEdgeNodes || this->getNNodes() != uint(this->edgeNodes.size()))
        {
            this->edgeNodes = this->findEdgeNodes();
            consolidateActionMask |= Model::ConsolidateMeshInput;
        }
        if (consolidateActionMask & Model::ConsolidateEdgeElements)
        {
            this->edgeElements = this->findEdgeElements(30.0);
        }
        if (consolidateActionMask & Model::ConsolidateHoleElements)
        {
            this->holeElements = this->findHoleElements();
        }
        if (consolidateActionMask & Model::ConsolidateSliverElements)
        {
            this->updateSliverElements(Model::SliverElementEdgeRatio);
        }
        if (consolidateActionMask & Model::ConsolidateIntersectedElements)
        {
            this->updateIntersectedElements();
        }
        if (consolidateActionMask & Model::ConsolidateMeshInput)
        {
            this->initializeMeshInput();
        }
    }
    catch (const RError &rError)
    {
        RLogger::error("Failed to consolidate model. %s\n",rError.getMessage().toUtf8().constData());
    }

    RLogger::unindent();
}

QString Model::buildDataFileName(const QString &format, bool withTimeStep) const
{
    QString fileName;
    if (withTimeStep)
    {
        fileName = RFileManager::getFileNameWithTimeStep(this->getFileName(),this->getTimeSolver().getCurrentTimeStep()+1);
    }
    else
    {
        fileName = RFileManager::getFileNameWithOutTimeStep(this->getFileName());
    }
    QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());

    return QString(dataDir.filePath(QFileInfo(fileName).baseName()) + "." + format);
}

QString Model::buildDocFileName(const QString &format, bool withTimeStep) const
{
    QString fileName;
    if (withTimeStep)
    {
        fileName = RFileManager::getFileNameWithTimeStep(this->getFileName(),this->getTimeSolver().getCurrentTimeStep()+1);
    }
    else
    {
        fileName = RFileManager::getFileNameWithOutTimeStep(this->getFileName());
    }
    QDir docDir(Application::instance()->getApplicationSettings()->getDocDir());

    QString formatExt;
    if (!format.isEmpty())
    {
        formatExt = "." + format;
    }

    return QString(docDir.filePath(QFileInfo(fileName).baseName()) + formatExt);
}

QString Model::buildTmpFileName(const QString &format, bool withTimeStep) const
{
    QString fileName;
    if (withTimeStep)
    {
        fileName = RFileManager::getFileNameWithTimeStep(this->getFileName(),this->getTimeSolver().getCurrentTimeStep()+1);
    }
    else
    {
        fileName = RFileManager::getFileNameWithOutTimeStep(this->getFileName());
    }
    QDir docDir(Application::instance()->getApplicationSettings()->getTmpDir());

    return QString(docDir.filePath(QFileInfo(fileName).baseName()) + "." + format);
}

QString Model::buildTmpFileName(const QString &format, const QString &idString) const
{
    QString fileName = RFileManager::getFileNameWithOutTimeStep(this->getFileName());
    QDir docDir(Application::instance()->getApplicationSettings()->getTmpDir());

    if (idString.isEmpty())
    {
        return QString(docDir.filePath(QFileInfo(fileName).baseName()) + "." + format);
    }
    else
    {
        return QString(docDir.filePath(QFileInfo(fileName).baseName()) + "-" + idString + "." + format);
    }
}

QString Model::buildScreenShotFileName(const QString &format) const
{
    return this->buildDocFileName(format,true);
}

QString Model::buildAnimationFileName(const QString &format) const
{
    return this->buildDocFileName(format,false);
}

QList<QString> Model::getRecordFiles(bool onlyExistingFiles) const
{
    uint nRecords = 0;

    if (this->getTimeSolver().getEnabled())
    {
        nRecords = this->getTimeSolver().getNTimeSteps();
    }
    else
    {
        if (this->getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_STRESS_MODAL)
        {
            nRecords = this->getProblemSetup().getModalSetup().getNModesToExtract();
            if (this->getProblemSetup().getModalSetup().getMethod() == R_MODAL_DOMINANT_MODE)
            {
                nRecords = 1;
            }
        }
    }

    QList<QString> recordFiles;
    recordFiles.reserve(nRecords);

    for (uint j=0;j<nRecords;j++)
    {
        QString recordFileName(RFileManager::getFileNameWithTimeStep(this->getFileName(),j+1));
        if (!onlyExistingFiles || RFileManager::fileExists(recordFileName))
        {
            recordFiles.append(recordFileName);
        }
    }
    return recordFiles;
}

QList<QString> Model::getDocumentFiles() const
{
    QList<QString> documentFiles;

    if (this->getFileName().isEmpty())
    {
        return documentFiles;
    }

    QFileInfo fi(RFileManager::getFileNameWithOutTimeStep(this->getFileName()));
    QDir docDir(Application::instance()->getApplicationSettings()->getDocDir());

    QList<QString> documentFormats = ApplicationSettings::getSupportedDocumentFormats();
    QList<QString> imageFormats = ApplicationSettings::getSupportedImageFormats();
    QList<QString> videoFormats = ApplicationSettings::getSupportedVideoFormats();

    QString baseName(fi.baseName());

    QStringList filters;

    for (int i=0;i<documentFormats.size();i++)
    {
        filters << baseName + "*." + documentFormats[i];
    }
    for (int i=0;i<imageFormats.size();i++)
    {
        filters << baseName + "*." + imageFormats[i];
    }
    for (int i=0;i<videoFormats.size();i++)
    {
        filters << baseName + "*." + videoFormats[i];
    }

    foreach (QString file, docDir.entryList(filters, QDir::Files))
    {
        documentFiles.append(docDir.absoluteFilePath(file));
    }

    return documentFiles;
}


uint Model::getUndoStackSize() const
{
    return uint(this->undoStack.size());
}

void Model::undo(uint revision)
{
    for (uint i=0;i<revision;i++)
    {
        this->redoStack.push(this->undoStack.pop());
    }
    this->redoStack.push(*this);
    this->operator =(this->undoStack.pop());
}

QString Model::getUndoActionMessage() const
{
    if (this->undoStack.size() > 0)
    {
        return this->undoStack.top().actionDescription;
    }
    return QString();
}

uint Model::getRedoStackSize() const
{
    return uint(this->redoStack.size());
}

void Model::redo(uint revision)
{
    for (uint i=0;i<revision;i++)
    {
        this->undoStack.push(this->redoStack.pop());
    }
    this->undoStack.push(*this);
    this->operator =(this->redoStack.pop());
}

QString Model::getRedoActionMessage() const
{
    if (this->redoStack.size() > 0)
    {
        return this->redoStack.top().actionDescription;
    }
    return QString();
}

void Model::storeCurentVersion(uint maxDepth, const QString &actionDescription)
{
    this->actionDescription = actionDescription;
    this->redoStack.clear();
    if (maxDepth > 0)
    {
        this->undoStack.push(*this);
        while (this->undoStack.size() > int(maxDepth))
        {
            this->undoStack.pop_front();
        }
    }
    else
    {
        this->undoStack.clear();
    }
}

void Model::updateHistoryStackSize(uint maxDepth)
{
    if (this->undoStack.size() > int(maxDepth))
    {
        this->undoStack.resize(int(maxDepth));
    }
    if (this->redoStack.size() > int(maxDepth))
    {
        this->redoStack.resize(int(maxDepth));
    }
}

void Model::clearEdgeNodes()
{
    this->edgeNodes.clear();
}

QSet<uint> Model::findEdgeNodeIDs(const QList<SessionEntityID> &entityIDs) const
{
    QSet<uint> nodeIDs;

    RUVector nodeIsOnEdge(this->getNNodes(),0);
    RBVector elementIsInGroup(this->getNElements(),false);
    RUVector nodeIsInGroup(this->getNNodes(),RConstants::eod);

    for (int i=0;i<entityIDs.size();i++)
    {
        uint entityGroupId = this->getEntityGroupID(entityIDs[i].getType(),entityIDs[i].getEid(),true);
        if (entityGroupId == RConstants::eod)
        {
            continue;
        }
        const RElementGroup *pEelementGroup = static_cast<const RElementGroup*>(this->getEntityGroupPtr(entityGroupId,true));
        if (!pEelementGroup)
        {
            continue;
        }
        for (uint j=0;j<pEelementGroup->size();j++)
        {
            uint elementID = pEelementGroup->get(j);
            elementIsInGroup[elementID] = true;
            const RElement &rElement = this->getElement(elementID);
            for (uint k=0;k<rElement.size();k++)
            {
                nodeIsOnEdge[rElement.getNodeId(k)] = 1;
                if (entityIDs[i].getType() == R_ENTITY_GROUP_POINT)
                {
                    nodeIsInGroup[rElement.getNodeId(k)] = R_ENTITY_GROUP_POINT;
                }
                else if (entityIDs[i].getType() == R_ENTITY_GROUP_LINE)
                {
                    nodeIsInGroup[rElement.getNodeId(k)] = R_ENTITY_GROUP_LINE;
                }
                else if (entityIDs[i].getType() == R_ENTITY_GROUP_SURFACE)
                {
                    nodeIsInGroup[rElement.getNodeId(k)] = R_ENTITY_GROUP_SURFACE;
                }
                else if (entityIDs[i].getType() == R_ENTITY_GROUP_VOLUME)
                {
                    nodeIsInGroup[rElement.getNodeId(k)] = R_ENTITY_GROUP_VOLUME;
                }
            }
        }
    }

    for (uint i=0;i<this->getNElements();i++)
    {
        if (elementIsInGroup[i])
        {
            continue;
        }
        const RElement &rElement = this->getElement(i);
        for (uint j=0;j<rElement.size();j++)
        {
            uint nodeID = rElement.getNodeId(j);
            if (nodeIsOnEdge[nodeID] == 1)
            {
                nodeIsOnEdge[nodeID] = 2;
            }
        }
    }

    uint nn = 0;
    for (uint i=0;i<this->getNNodes();i++)
    {
        if (nodeIsOnEdge[i] == 2 || nodeIsInGroup[i] == R_ENTITY_GROUP_POINT || nodeIsInGroup[i] == R_ENTITY_GROUP_LINE)
        {
            nn++;
        }
    }
    nodeIDs.reserve(int(nn));
    for (uint i=0;i<this->getNNodes();i++)
    {
        if (nodeIsOnEdge[i] == 2 || nodeIsInGroup[i] == R_ENTITY_GROUP_POINT || nodeIsInGroup[i] == R_ENTITY_GROUP_LINE)
        {
            nodeIDs.insert(i);
        }
    }
    for (int i=0;i<this->holeElements.size();i++)
    {
        for (uint j=0;j<this->holeElements[i].size();j++)
        {
            nodeIDs.insert(this->holeElements[i].getNodeId(j));
        }
    }

    return nodeIDs;
} /* Model::findEdgeNodeIDs */

QMap<uint, uint> Model::splitNodes(const QSet<uint> &nodeIDs, const QSet<uint> &elementIDs)
{
    QMap<uint,uint> newNodeMap;

    foreach (uint nodeID, nodeIDs)
    {
        this->addNode(this->getNode(nodeID));
        uint newNodeID = this->getNNodes()-1;
        newNodeMap[nodeID] = newNodeID;
    }

    for (uint i=0;i<this->getNElements();i++)
    {
        if (elementIDs.contains(i))
        {
            continue;
        }
        RElement &rElement = this->getElement(i);
        for (uint j=0;j<rElement.size();j++)
        {
            uint nodeID = rElement.getNodeId(j);
            if (newNodeMap.contains(nodeID))
            {
                rElement.setNodeId(j,newNodeMap[nodeID]);
            }
        }
    }

    return newNodeMap;
} /* Model::splitNodes */

QList<RElement> Model::generateEdgeElements(const QSet<uint> &edgeNodeIDs, const QSet<uint> &elementIDs) const
{
    QList<RElement> edgeElements;

    foreach (uint elementID, elementIDs)
    {
        if (R_ELEMENT_TYPE_IS_POINT(this->getElement(elementID).getType()) || R_ELEMENT_TYPE_IS_LINE(this->getElement(elementID).getType()))
        {
            edgeElements.push_back(this->getElement(elementID));
        }
        else
        {
            std::vector<RElement> elementEdgeElements = this->getElement(elementID).generateEdgeElements();
            for (uint j=0;j<elementEdgeElements.size();j++)
            {
                const RElement &rElement = elementEdgeElements[j];
                bool allNodesOnEdge = true;
                for (uint k=0;k<rElement.size();k++)
                {
                    if (!edgeNodeIDs.contains(rElement.getNodeId(k)))
                    {
                        allNodesOnEdge = false;
                        break;
                    }
                }
                if (allNodesOnEdge)
                {
                    edgeElements.push_back(rElement);
                }
            }
        }
    }

    for (int i=edgeElements.size()-1;i>=0;i--)
    {
        bool keepElement = true;
        uint nElementsFound = 0;
        foreach (uint elementID, elementIDs)
        {
            uint nFound = 0;
            for (uint j=0;j<edgeElements[i].size();j++)
            {
                if (this->getElement(elementID).hasNodeId(edgeElements[i].getNodeId(j)))
                {
                    nFound ++;
                }
            }
            if (nFound == edgeElements[i].size())
            {
                nElementsFound++;
            }
            if (nElementsFound >= 2)
            {
                keepElement = false;
                break;
            }
        }
        if (!keepElement)
        {
            edgeElements.removeAt(i);
        }
    }

    return edgeElements;
} /* Model::generateEdgeElements */

void Model::createSweepEdgeElements(const QList<RElement> &edgeElements, const QMap<uint, uint> &edgeNodeMap, bool useLastGroupID, bool selectNewEntities, bool showNewEntities)
{
    QList<RElement> lineElements;
    QList<RElement> surfaceElements;

    for (int i=0;i<edgeElements.size();i++)
    {
        switch (edgeElements[i].getType())
        {
            case R_ELEMENT_POINT:
            {
                if (edgeNodeMap.contains(edgeElements[i].getNodeId(0)))
                {
                    RElement e;
                    e.setType(R_ELEMENT_TRUSS1);
                    e.setNodeId(0,edgeElements[i].getNodeId(0));
                    e.setNodeId(1,edgeNodeMap[edgeElements[i].getNodeId(0)]);
                    lineElements.push_back(e);
                }
                break;
            }
            case R_ELEMENT_TRUSS1:
            {
                if (edgeNodeMap.contains(edgeElements[i].getNodeId(0)) &&
                    edgeNodeMap.contains(edgeElements[i].getNodeId(1)))
                {
                    RElement e;
                    e.setType(R_ELEMENT_QUAD1);
                    e.setNodeId(0,edgeElements[i].getNodeId(1));
                    e.setNodeId(1,edgeElements[i].getNodeId(0));
                    e.setNodeId(2,edgeNodeMap[edgeElements[i].getNodeId(0)]);
                    e.setNodeId(3,edgeNodeMap[edgeElements[i].getNodeId(1)]);
                    surfaceElements.push_back(e);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    if (lineElements.size() > 0)
    {
        uint groupID = this->getNLines() - (useLastGroupID ? 1 : 0);
        this->lines.resize(this->getNLines() + (useLastGroupID ? 0 : 1));
        this->lines[groupID].setName("Line " + QString::number(groupID + 1));
        this->lines[groupID].getData().setVisible(showNewEntities);
        this->lines[groupID].getData().setSelected(selectNewEntities);
        for (int i=0;i<lineElements.size();i++)
        {
            this->addElement(lineElements[i],true,groupID);
        }
    }

    if (surfaceElements.size() > 0)
    {
        uint groupID = this->getNSurfaces() - (useLastGroupID ? 1 : 0);
        this->surfaces.resize(this->getNSurfaces() + (useLastGroupID ? 0 : 1));
        this->surfaces[groupID].setName("Surface " + QString::number(groupID + 1));
        this->surfaces[groupID].getData().setVisible(showNewEntities);
        this->surfaces[groupID].getData().setSelected(selectNewEntities);
        for (int i=0;i<surfaceElements.size();i++)
        {
            this->addElement(surfaceElements[i],true,groupID);
        }
    }
} /* Model::createSweepEdgeElements */


QVector<RElement> Model::findEdgeElements(double separationAngle) const
{
    RLogger::info("Finding hole elements\n");
    RLogger::indent();

    double radAngle = R_DEG_TO_RAD(separationAngle);

    QVector<RElement> segments;

    for (uint i=0;i<this->surfaceNeigs.size();i++)
    {
        uint ceID = i;

        if (!R_ELEMENT_TYPE_IS_SURFACE(this->getElement(ceID).getType()))
        {
            continue;
        }

        RR3Vector cn;

        const RElement &cElement = this->getElement(ceID);
        cElement.findNormal(this->getNodes(),cn[0],cn[1],cn[2]);

        // Inspect neighboring elements.
        for (uint j=0;j<surfaceNeigs[i].getNRows();j++)
        {
            uint neID = this->surfaceNeigs[i][j];

            if (!R_ELEMENT_TYPE_IS_SURFACE(this->getElement(neID).getType()))
            {
                continue;
            }

            RR3Vector nn;

            const RElement &nElement = this->getElement(neID);
            nElement.findNormal(this->getNodes(),nn[0],nn[1],nn[2]);

            if (RR3Vector::angle(cn,nn) >= radAngle)
            {
                std::vector<uint> sNodeIDs;
                for (uint k=0;k<cElement.size();k++)
                {
                    if (nElement.hasNodeId(cElement.getNodeId(k)))
                    {
                        sNodeIDs.push_back(cElement.getNodeId(k));
                    }
                }
                if (sNodeIDs.size() == 2)
                {
                    RElement e(R_ELEMENT_TRUSS1);
                    e.setNodeId(0,sNodeIDs[0]);
                    e.setNodeId(1,sNodeIDs[1]);
                    segments.append(e);
                }
            }
        }

        // Inspect nodes that have no neighbor
        std::vector<RElement> edges = cElement.generateEdgeElements();
        for (uint j=0;j<edges.size();j++)
        {
            bool edgeHasNoNeighbor = true;
            for (uint k=0;k<surfaceNeigs[i].getNRows();k++)
            {
                if (edges[j].size() < 2)
                {
                    continue;
                }
                uint neID = this->surfaceNeigs[i][k];
                const RElement &nElement = this->getElement(neID);
                if (nElement.hasNodeId(edges[j].getNodeId(0)) && nElement.hasNodeId(edges[j].getNodeId(1)))
                {
                    edgeHasNoNeighbor = false;
                }

            }
            if (edgeHasNoNeighbor)
            {
                segments.append(edges[j]);
            }
        }
    }

    RLogger::unindent();
    return segments;
} /* Model::findEdgeElements */


QVector<RElement> Model::findHoleElements() const
{
    RLogger::info("Finding hole elements\n");
    RLogger::indent();

    if (this->getNElements() != this->surfaceNeigs.size())
    {
        RLogger::info("Invalid surface neighbors\n");
        RLogger::unindent();
        return QVector<RElement>();
    }


    QVector<RElement> elementList;

    for (uint i=0;i<this->surfaceNeigs.size();i++)
    {
        if (RElementGroup::getGroupType(this->getElement(i).getType()) != R_ENTITY_GROUP_SURFACE)
        {
            continue;
        }
        if (this->surfaceNeigs[i].size() == this->getElement(i).size())
        {
            continue;
        }
        std::vector<RElement> edgeElements = this->getElement(i).generateEdgeElements();
        for (uint j=0;j<edgeElements.size();j++)
        {
            bool edgeFound = false;
            for (uint k=0;k<this->surfaceNeigs[i].size();k++)
            {
                std::vector<RElement> neighborEdgeElements = this->getElement(this->surfaceNeigs[i][k]).generateEdgeElements();
                for (uint l=0;l<neighborEdgeElements.size();l++)
                {
                    if (edgeElements[j] == neighborEdgeElements[l])
                    {
                        edgeFound = true;
                        break;
                    }
                }
                if (edgeFound)
                {
                    break;
                }
            }
            if (!edgeFound)
            {
                elementList.push_back(edgeElements[j]);
            }
        }
    }
    RLogger::unindent();

    return elementList;
} /* Model::findHoleElements */
