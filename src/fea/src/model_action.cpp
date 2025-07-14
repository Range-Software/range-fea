#include <QtAlgorithms>

#include <rhemicube.h>

#include "model_action.h"
#include "application.h"

ModelAction::ModelAction()
{
}

void ModelAction::addAction(const ModelActionInput &modelActionInput)
{
    this->actions.append(modelActionInput);
    std::sort(this->actions.begin(),this->actions.end());
}

int ModelAction::perform()
{
    RLogger::info("Processing model actions\n");
    for (int i=0;i<this->actions.size();i++)
    {
        RLogger::indent();
        this->executeAction(this->actions[i]);
        RLogger::unindent();
    }
    return 0;
}

void ModelAction::executeAction(const ModelActionInput &modelActionInput)
{
    Application::instance()->getSession()->getModel(modelActionInput.getModelID()).glDrawLock();

    try
    {
        switch (modelActionInput.getType())
        {
            case MODEL_ACTION_AUTOMARK_SURFACES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Automark surface entities"));
                this->autoMarkSurfaces(modelActionInput);
                break;
            case MODEL_ACTION_MARK_SURFACES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Mark surface entities"));
                this->markSurfaces(modelActionInput);
                break;
            case MODEL_ACTION_CLOSE_SURFACE_HOLE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Close surface hole"));
                this->closeSurfaceHole(modelActionInput);
                break;
            case MODEL_ACTION_MERGE_NEAR_NODES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Merge near nodes"));
                this->mergeNearNodes(modelActionInput);
                break;
            case MODEL_ACTION_MERGE_ENTITIES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Merge entities"));
                this->mergeEntities(modelActionInput);
                break;
            case MODEL_ACTION_REMOVE_ENTITIES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Remove entities"));
                this->removeEntities(modelActionInput);
                break;
            case MODEL_ACTION_CREATE_ELEMENT:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Create element"));
                this->createElement(modelActionInput);
                break;
            case MODEL_ACTION_REMOVE_NODES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Remove nodes"));
                this->removeNodes(modelActionInput);
                break;
            case MODEL_ACTION_PURGE_UNUSED_NODES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Purge unused nodes"));
                this->purgeUnusedNodes(modelActionInput);
                break;
            case MODEL_ACTION_PURGE_UNUSED_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Purge unused elements"));
                this->purgeUnusedElements(modelActionInput);
                break;
            case MODEL_ACTION_REMOVE_DUPLICATE_NODES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Remove duplicate nodes"));
                this->removeDuplicateNodes(modelActionInput);
                break;
            case MODEL_ACTION_REMOVE_DUPLICATE_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Remove duplicate elements"));
                this->removeDuplicateElements(modelActionInput);
                break;
            case MODEL_ACTION_REMOVE_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Remove elements"));
                this->removeElements(modelActionInput);
                break;
            case MODEL_ACTION_GENERATE_LINE_FROM_EDGES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Generate line entities from model edges"));
                this->generateLineFromEdges(modelActionInput);
                break;
            case MODEL_ACTION_FIND_SLIVER_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Find sliver elements"));
                this->findSliverElements(modelActionInput);
                break;
            case MODEL_ACTION_FIX_SLIVER_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Fix sliver elements"));
                this->fixSliverElements(modelActionInput);
                break;
            case MODEL_ACTION_FIND_INTERSECTED_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Find intersected elements"));
                this->findIntersectedElements(modelActionInput);
                break;
            case MODEL_ACTION_BREAK_INTERSECTED_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Break intersected elements"));
                this->breakIntersectedElements(modelActionInput);
                break;
            case MODEL_ACTION_EXPORT_SLIVER_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Export intersected elements"));
                this->exportSliverElements(modelActionInput);
                break;
            case MODEL_ACTION_EXPORT_INTERSECTED_ELEMENTS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Export intersected elements"));
                this->exportIntersectedElements(modelActionInput);
                break;
            case MODEL_ACTION_BOOL_DIFFERENCE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Boolean difference"));
                this->boolDifference(modelActionInput);
                break;
            case MODEL_ACTION_BOOL_INTERSECTION:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Boolean intersection"));
                this->boolIntersection(modelActionInput);
                break;
            case MODEL_ACTION_BOOL_UNION:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Boolean union"));
                this->boolUnion(modelActionInput);
                break;
            case MODEL_ACTION_CHECK_POINT_INSIDE_SURFACE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Check point inside surface"));
                this->checkPointInsideSurface(modelActionInput);
                break;
            case MODEL_ACTION_GENERATE_PATCHES:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Generate patches"));
                this->generatePatches(modelActionInput);
                break;
            case MODEL_ACTION_CALCULATE_VIEWFACTORS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Calculate view-factors"));
                this->calculateViewFactors(modelActionInput);
                break;
            case MODEL_ACTION_SWAP_SURFACE_ELEMENT_NORMAL:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Swap surface element normal"));
                this->swapSurfaceElementNormal(modelActionInput);
                break;
            case MODEL_ACTION_SWAP_SURFACE_NORMALS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Swap surface normals"));
                this->swapSurfaceNormals(modelActionInput);
                break;
            case MODEL_ACTION_SYNC_SURFACE_NORMALS:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Synchronize surface normals"));
                this->syncSurfaceNormals(modelActionInput);
                break;
            case MODEL_ACTION_COARSEN_SURFACE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Coarsen surface"));
                this->coarsenSurface(modelActionInput);
                break;
            case MODEL_ACTION_TETRAHEDRALIZE_SURFACE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Tetrahedralize surface"));
                this->tetrahedralizeSurface(modelActionInput);
                break;
            case MODEL_ACTION_CONSOLIDATE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Consolidate geometry"));
                this->consolidate(modelActionInput);
                break;
            case MODEL_ACTION_GEOMETRY_TRANSFORM:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Transform geometry"));
                this->geometryTransform(modelActionInput);
                break;
            case MODEL_ACTION_APPLY_VARIABLE_ON_NODE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Apply variable on nodes"));
                this->applyVariableOnNode(modelActionInput);
                break;
            case MODEL_ACTION_REMOVE_VARIABLE:
                Application::instance()->getSession()->storeCurentModelVersion(modelActionInput.getModelID(),tr("Remove variable"));
                this->removeVariable(modelActionInput);
                break;
            default:
                //! Nothing to be performed.
                break;
        }
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to perform model action \'%d\'. %s\n",modelActionInput.getType(),error.getMessage().toUtf8().constData());
    }
    Application::instance()->getSession()->getModel(modelActionInput.getModelID()).glDrawUnlock();
}

void ModelAction::autoMarkSurfaces(const ModelActionInput &modelActionInput)
{
    Application::instance()->getSession()->getModel(modelActionInput.getModelID()).autoMarkSurfaces(modelActionInput.getSeparationAngle());
    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::markSurfaces(const ModelActionInput &modelActionInput)
{
    Application::instance()->getSession()->getModel(modelActionInput.getModelID()).markSurface(modelActionInput.getSeparationAngle(),modelActionInput.getElementIDs());
    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::closeSurfaceHole(const ModelActionInput &modelActionInput)
{
    Application::instance()->getSession()->getModel(modelActionInput.getModelID()).closeSurfaceHole(modelActionInput.getElementIDs());
    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::mergeNearNodes(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Merging near nodes\n");
    RLogger::indent();
    uint nMerged = rModel.mergeNearNodes(modelActionInput.getTolerance());
    RLogger::info("Number of merged nodes = %u\n",nMerged);
    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::mergeEntities(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Merging entities\n");
    RLogger::indent();
    rModel.mergeEntities(R_ENTITY_GROUP_POINT,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_POINT));
    rModel.mergeEntities(R_ENTITY_GROUP_LINE,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_LINE));
    rModel.mergeEntities(R_ENTITY_GROUP_SURFACE,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_SURFACE));
    rModel.mergeEntities(R_ENTITY_GROUP_VOLUME,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_VOLUME));
    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::removeEntities(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Removing entities\n");
    RLogger::indent();

    uint nSurfaceEntities = 0;
    // uint nVolumeEntities = 0;
    foreach (SessionEntityID sessionEntityID, modelActionInput.getEntityIDs())
    {
        if (sessionEntityID.getType() == R_ENTITY_GROUP_SURFACE)
        {
            nSurfaceEntities++;
        }
        // if (sessionEntityID.getType() == R_ENTITY_GROUP_VOLUME)
        // {
        //     nVolumeEntities++;
        // }
    }

    rModel.removeEntities(R_ENTITY_GROUP_POINT,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_POINT));
    rModel.removeEntities(R_ENTITY_GROUP_LINE,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_LINE));
    rModel.removeEntities(R_ENTITY_GROUP_SURFACE,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_SURFACE));
    rModel.removeEntities(R_ENTITY_GROUP_VOLUME,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_VOLUME));
    rModel.removeEntities(R_ENTITY_GROUP_CUT,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_CUT));
    rModel.removeEntities(R_ENTITY_GROUP_ISO,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_ISO));
    rModel.removeEntities(R_ENTITY_GROUP_STREAM_LINE,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_STREAM_LINE));
    rModel.removeEntities(R_ENTITY_GROUP_VECTOR_FIELD,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_VECTOR_FIELD));
    rModel.removeEntities(R_ENTITY_GROUP_SCALAR_FIELD,SessionEntityID::getEntityIDs(modelActionInput.getEntityIDs(),R_ENTITY_GROUP_SCALAR_FIELD));

    rModel.RModel::purgeUnusedElements();
    rModel.RModel::purgeUnusedNodes();

    int consolidateActionMask = Model::ConsolidateEdgeNodes | Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateMeshInput | Model::ConsolidateSliverElements;

    if (nSurfaceEntities > 0)
    {
        consolidateActionMask |= Model::ConsolidateSurfaceNeighbors;
        if (rModel.getNIntersected() > 0)
        {
            consolidateActionMask |= Model::ConsolidateIntersectedElements;
        }
    }
    if (nSurfaceEntities > 0)
    {
        consolidateActionMask |= Model::ConsolidateVolumeNeighbors;
    }

    rModel.consolidate(consolidateActionMask);

    Application::instance()->getSession()->getPickList().clear();

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::createElement(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Creating element\n");
    RLogger::indent();

    RElementType elementType = modelActionInput.getElementType();
    QList<uint> nodeIDs = modelActionInput.getNodeIDs();

    RElement e(elementType);
    for (uint i=0;i<e.size();i++)
    {
        e.setNodeId(i,nodeIDs[int(i)]);
    }

    if (elementType == R_ELEMENT_QUAD1)
    {
        uint n1 = e.getNodeId(0);
        uint n2 = e.getNodeId(1);
        uint n3 = e.getNodeId(2);
        uint n4 = e.getNodeId(3);
        RTriangle t1(rModel.getNode(n1),rModel.getNode(n2),rModel.getNode(n3));
        RTriangle t2(rModel.getNode(n2),rModel.getNode(n3),rModel.getNode(n4));
        if (RR3Vector::angle(t1.getNormal(),t2.getNormal()) > RConstants::pi / 2.0)
        {
            e.swapNodeIds(2,3);
        }
    }

    rModel.addElement(e);

    rModel.consolidate(Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateSliverElements);
    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::removeNodes(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Removing nodes\n");
    RLogger::indent();

    rModel.removeNodes(modelActionInput.getNodeIDs(),modelActionInput.getCloseHole());
    rModel.consolidate(Model::ConsolidateActionAll);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::purgeUnusedNodes(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Purging unused nodes\n");
    RLogger::indent();

    uint nPurged = rModel.purgeUnusedNodes();
    RLogger::info("Number of purged nodes = %u\n",nPurged);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::purgeUnusedElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Purging unused elements\n");
    RLogger::indent();

    uint nPurged = rModel.purgeUnusedElements();
    RLogger::info("Number of purged elements = %u\n",nPurged);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::removeDuplicateNodes(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Removing duplicate nodes\n");
    RLogger::indent();
    uint nMerged = rModel.mergeNearNodes(RConstants::eps);
    RLogger::info("Number of removed nodes = %u\n",nMerged);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::removeDuplicateElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Removing duplicate elements\n");
    RLogger::indent();
    uint nMerged = rModel.removeDuplicateElements();
    RLogger::info("Number of removed elements = %u\n",nMerged);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::removeElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Removing elements\n");
    RLogger::indent();

 /*
    uint nSurfaceElements = 0;
    uint nVolumeElements = 0;
    foreach (uint elementID, modelActionInput.getElementIDs())
    {
        if (R_ELEMENT_TYPE_IS_SURFACE(rModel.getElement(elementID).getType()))
        {
            nSurfaceElements++;
        }
        if (R_ELEMENT_TYPE_IS_VOLUME(rModel.getElement(elementID).getType()))
        {
            nVolumeElements++;
        }
    }

    int consolidateActionMask = Model::ConsolidateEdgeNodes | Model::ConsolidateEdgeElements | Model::ConsolidateHoleElements | Model::ConsolidateMeshInput | Model::ConsolidateSliverElements;
    if (nSurfaceElements > 0)
    {
        consolidateActionMask |= Model::ConsolidateSurfaceNeighbors;
        if (rModel.getNIntersected() > 0)
        {
            consolidateActionMask |= Model::ConsolidateIntersectedElements;
        }
    }
    if (nVolumeElements > 0)
    {
        consolidateActionMask |= Model::ConsolidateVolumeNeighbors;
    }
*/

    rModel.removeElements(modelActionInput.getElementIDs(),modelActionInput.getCloseHole());
    rModel.consolidate(Model::ConsolidateActionAll);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::generateLineFromEdges(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Generating line(s) from surface edges\n");
    RLogger::indent();

    rModel.generateLineFromSurfaceEdges(modelActionInput.getSeparationAngle());

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::findSliverElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Finding sliver elements\n");
    RLogger::indent();

    rModel.updateSliverElements(modelActionInput.getEdgeRatio());

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::fixSliverElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Fixing sliver elements\n");
    RLogger::indent();

    rModel.fixSliverElements(modelActionInput.getEdgeRatio());

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::exportSliverElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Exporting sliver elements\n");
    RLogger::indent();

    rModel.exportSliverElements();

    RLogger::unindent();
}

void ModelAction::findIntersectedElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Finding intersected elements\n");
    RLogger::indent();

    rModel.updateIntersectedElements();

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::breakIntersectedElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Breaking intersected elements\n");
    RLogger::indent();

//    // Better to remove volume emelements before breaking the triangles.
//    if (rModel.getNVolumes() > 0)
//    {
//        QList<uint> entityIDs;
//        for (uint i=0;i<rModel.getNVolumes();i++)
//        {
//            entityIDs.append(i);
//        }
//        rModel.removeEntities(R_ENTITY_GROUP_VOLUME,entityIDs);
//    }
    rModel.breakIntersectedElements(modelActionInput.getNIterations());

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::exportIntersectedElements(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Exporting intersected elements\n");
    RLogger::indent();

    rModel.exportIntersectedElements();

    RLogger::unindent();
}

void ModelAction::boolDifference(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());
    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();
    QList<uint> surfaceIDs;

    for (int i=0;i<entityIDs.size();i++)
    {
        surfaceIDs.append(entityIDs.at(i).getEid());
    }

    RLogger::info("Performing difference geometry operation\n");
    RLogger::indent();

    try
    {
        uint cuttingSurfaceID = surfaceIDs.last();
        surfaceIDs.pop_back();
        rModel.boolDifference(modelActionInput.getNIterations(),surfaceIDs,cuttingSurfaceID);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to perform difference boolean operation. %s", error.getMessage().toUtf8().constData());
    }

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::boolIntersection(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());
    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();
    QList<uint> surfaceIDs;

    for (int i=0;i<entityIDs.size();i++)
    {
        surfaceIDs.append(entityIDs.at(i).getEid());
    }

    RLogger::info("Performing intersection geometry operation\n");
    RLogger::indent();

    try
    {
        rModel.boolIntersection(modelActionInput.getNIterations(),surfaceIDs);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to perform intersection boolean operation. %s", error.getMessage().toUtf8().constData());
    }

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::boolUnion(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());
    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();
    QList<uint> surfaceIDs;

    for (int i=0;i<entityIDs.size();i++)
    {
        surfaceIDs.append(entityIDs.at(i).getEid());
    }

    RLogger::info("Performing union geometry operation\n");
    RLogger::indent();

    try
    {
        rModel.boolUnion(modelActionInput.getNIterations(),surfaceIDs);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to perform union boolean operation. %s", error.getMessage().toUtf8().constData());
    }

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::checkPointInsideSurface(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());
    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();
    const RR3Vector &point = modelActionInput.getPoint();

    RLogger::info("Checking if point is inside selected surfaces.\n");
    RLogger::indent();

    for (int i=0;i<entityIDs.size();i++)
    {
        const RSurface &rSurface = rModel.getSurface(entityIDs.at(i).getEid());
        bool isInside = rSurface.pointInside(rModel.getNodes(),rModel.getElements(),point,false);
        RLogger::info("Point [%g %g %g] is inside the surface %s: %s\n",point[0],point[1],point[2],rSurface.getName().toUtf8().constData(),isInside?"TRUE":"FALSE");
    }

    RLogger::unindent();
}

void ModelAction::generatePatches(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    // Clear view-factor matrix
    rModel.getViewFactorMatrix().clear();
    // Generate new header
    rModel.generateViewFactorMatrixHeader(rModel.getViewFactorMatrix().getHeader());
    // Generate patches
    rModel.generatePatchSurface(rModel.getViewFactorMatrix().getHeader().getPatchInput(),
                                rModel.getViewFactorMatrix().getPatchBook());

    // Write view factors.
    QString viewFactorMatrixFile = rModel.getProblemSetup().getRadiationSetup().getViewFactorMatrixFile();
    if (viewFactorMatrixFile.isEmpty())
    {
        viewFactorMatrixFile = rModel.buildDataFileName(RViewFactorMatrix::getDefaultFileExtension(true),rModel.getTimeSolver().getEnabled());
    }
    viewFactorMatrixFile = rModel.writeViewFactorMatrix(rModel.getViewFactorMatrix(),viewFactorMatrixFile);
    rModel.getProblemSetup().getRadiationSetup().setViewFactorMatrixFile(viewFactorMatrixFile);

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::calculateViewFactors(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RViewFactorMatrixHeader viewFactorMatrixHeader;

    // Generate new header
    rModel.generateViewFactorMatrixHeader(viewFactorMatrixHeader);

    // Check if patches need to be regenerated
    if (viewFactorMatrixHeader.getPatchInput() != rModel.getViewFactorMatrix().getHeader().getPatchInput() ||
        viewFactorMatrixHeader.getPatchInput().size() != rModel.getNSurfaces() ||
        viewFactorMatrixHeader.getNElements() != rModel.getNElements())
    {
        // Clear view-factor matrix
        rModel.getViewFactorMatrix().clear();
        // Set new header
        rModel.getViewFactorMatrix().setHeader(viewFactorMatrixHeader);
        // Generate patches
        rModel.generatePatchSurface(rModel.getViewFactorMatrix().getHeader().getPatchInput(),
                                    rModel.getViewFactorMatrix().getPatchBook());
    }

    // Calculate view factors
    RHemiCube::calculateViewFactors(rModel,rModel.getViewFactorMatrix());

    // Write view factors.
    QString viewFactorMatrixFile = rModel.getProblemSetup().getRadiationSetup().getViewFactorMatrixFile();
    viewFactorMatrixFile = rModel.writeViewFactorMatrix(rModel.getViewFactorMatrix(),viewFactorMatrixFile);
    rModel.getProblemSetup().getRadiationSetup().setViewFactorMatrixFile(viewFactorMatrixFile);

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::swapSurfaceElementNormal(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Swapping normals of selected surface elements\n");
    RLogger::indent();

    const QList<uint> &elementIDs = modelActionInput.getElementIDs();

    for (int i=0;i<elementIDs.size();i++)
    {
        RElement &rElement = rModel.getElement(elementIDs[i]);
        rElement.swapNormal();
    }

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::swapSurfaceNormals(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Swapping normals of selected surface entities\n");
    RLogger::indent();

    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();

    std::vector<bool> elementIDs;
    elementIDs.resize(rModel.getNElements(),false);

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs.at(i).getType() != R_ENTITY_GROUP_SURFACE)
        {
            continue;
        }
        RSurface &rSurface = rModel.getSurface(entityIDs.at(i).getEid());

        for (uint j=0;j<rSurface.size();j++)
        {
            elementIDs[rSurface.get(j)] = true;

        }
    }

    for (uint i=0;i<elementIDs.size();i++)
    {
        if (elementIDs[i])
        {
            RElement &rElement = rModel.getElement(i);
            rElement.swapNormal();
        }
    }
    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::syncSurfaceNormals(const ModelActionInput &modelActionInput)
{
    Model &rModel = Application::instance()->getSession()->getModel(modelActionInput.getModelID());

    RLogger::info("Synchronizing surface entities normals\n");
    RLogger::indent();

    rModel.syncSurfaceNormals();
    rModel.consolidate(Model::ConsolidateHoleElements | Model::ConsolidateEdgeElements | Model::ConsolidateSliverElements);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::coarsenSurface(const ModelActionInput &modelActionInput)
{
    uint modelID = modelActionInput.getModelID();

    Model &rModel = Application::instance()->getSession()->getModel(modelID);

    RLogger::info("Coarsening surface\n");
    RLogger::indent();

    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();

    std::vector<uint> surfaceIDs;
    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs.at(i).getType() == R_ENTITY_GROUP_SURFACE)
        {
            surfaceIDs.push_back(entityIDs.at(i).getEid());
        }
    }

    uint nDeleted = rModel.coarsenSurfaceElements(surfaceIDs,
                                                  modelActionInput.getEdgeLength(),
                                                  modelActionInput.getElementArea());
    RLogger::info("Number of deleted surface elements = %u.\n",nDeleted);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::tetrahedralizeSurface(const ModelActionInput &modelActionInput)
{
    uint modelID = modelActionInput.getModelID();

    Model &rModel = Application::instance()->getSession()->getModel(modelID);

    RLogger::info("Tetrahedralizing surface\n");
    RLogger::indent();

    const QList<SessionEntityID> &entityIDs = modelActionInput.getEntityIDs();

    std::vector<uint> surfaceIDs;
    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs.at(i).getType() == R_ENTITY_GROUP_SURFACE)
        {
            surfaceIDs.push_back(entityIDs.at(i).getEid());
        }
    }

    uint nCreated = rModel.tetrahedralizeSurface(surfaceIDs);
    RLogger::info("Number of created volume elements = %u.\n",nCreated);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::consolidate(const ModelActionInput &modelActionInput)
{
    uint modelID = modelActionInput.getModelID();

    Model &rModel = Application::instance()->getSession()->getModel(modelID);

    RLogger::info("Consolidating geometry\n");
    RLogger::indent();

    rModel.consolidate(Model::ConsolidateActionAll);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelActionInput.getModelID());
}

void ModelAction::geometryTransform(const ModelActionInput &modelActionInput)
{
    uint modelID = modelActionInput.getModelID();

    Model &rModel = Application::instance()->getSession()->getModel(modelID);

    RLogger::info("Transforming model geometry\n");
    RLogger::indent();

    QList<SessionEntityID> entityIDs;

    switch (modelActionInput.getGeometryTransformInput().getApplyTo())
    {
        case GeometryTransformInput::ApplyToAll:
        {
            entityIDs = Application::instance()->getSession()->getAllEntityIDs(modelID);
            break;
        }
        case GeometryTransformInput::ApplyToSelected:
        {
            entityIDs = Application::instance()->getSession()->getModel(modelID).getSelectedEntityIDs(modelID);
            break;
        }
        case GeometryTransformInput::ApplyToPicked:
        {
            entityIDs = Application::instance()->getSession()->getModel(modelID).getPickedEntityIDs(modelID);
            break;
        }
        case GeometryTransformInput::ApplyToVisible:
        {
            entityIDs = Application::instance()->getSession()->getModel(modelID).getVisibleEntityIDs(modelID);
            break;
        }
        default:
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Invalid \'ApplyTo\' type.");
        }
    }

    // Model transformation
    rModel.transformGeometry(modelActionInput.getGeometryTransformInput(),entityIDs);

    RLogger::unindent();

    Application::instance()->getSession()->setModelChanged(modelID);
}

void ModelAction::applyVariableOnNode(const ModelActionInput &modelActionInput)
{
    uint modelID = modelActionInput.getModelID();

    Model &rModel = Application::instance()->getSession()->getModel(modelID);
    RVariableType variableType = modelActionInput.getVariableType();

    RLogger::info("Applying variable %s on node\n",RVariable::getName(variableType).toUtf8().constData());
    RLogger::indent();

    uint nVariables = rModel.getNVariables();

    for (uint i=0;i<nVariables;i++)
    {
        RVariable &rVariable = rModel.getVariable(i);
        if (rVariable.getType() != variableType)
        {
            continue;

        }
        if (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE)
        {
            continue;
        }

        RVariable newVariable(rVariable);
        newVariable.setApplyType(R_VARIABLE_APPLY_NODE);
        newVariable.resize(rVariable.getNVectors(),rModel.getNNodes());
        for (uint j=0;j<rVariable.getNVectors();j++)
        {
            RRVector elementValues = rVariable.getValues(j);
            RRVector nodeValues(rModel.getNNodes());
            RBVector explicitFlags;
            explicitFlags.resize(rModel.getNElements(),false);
            rModel.convertElementToNodeVector(elementValues,explicitFlags,nodeValues);
            for (uint k=0;k<rModel.getNNodes();k++)
            {
                newVariable.setValue(j,k,nodeValues[k]);
            }
        }
        rVariable = newVariable;
    }

    RLogger::unindent();
    Application::instance()->getSession()->setResultsChanged(modelID);
}

void ModelAction::removeVariable(const ModelActionInput &modelActionInput)
{
    uint modelID = modelActionInput.getModelID();

    Model &rModel = Application::instance()->getSession()->getModel(modelID);
    RVariableType variableType = modelActionInput.getVariableType();

    RLogger::info("Removing variable %s from model %s\n",
                  RVariable::getName(variableType).toUtf8().constData(),
                  rModel.getName().toUtf8().constData());
    RLogger::indent();

    uint variablePosition = RConstants::eod;
    while ((variablePosition = rModel.findVariable(variableType)) != RConstants::eod)
    {
        rModel.removeVariable(variablePosition);
    }

    RLogger::unindent();
    Application::instance()->getSession()->setResultsChanged(modelID);
}
