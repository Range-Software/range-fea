#include "model_action_input.h"

void ModelActionInput::_init(const ModelActionInput *pModelActionInput)
{
    if (pModelActionInput)
    {
        this->type = pModelActionInput->type;
        this->modelID = pModelActionInput->modelID;
        this->priority = pModelActionInput->priority;
        this->nIterations = pModelActionInput->nIterations;
        this->separationAngle = pModelActionInput->separationAngle;
        this->closeHole = pModelActionInput->closeHole;
        this->elementType = pModelActionInput->elementType;
        this->nodeIDs = pModelActionInput->nodeIDs;
        this->elementIDs = pModelActionInput->elementIDs;
        this->entityIDs = pModelActionInput->entityIDs;
        this->point = pModelActionInput->point;
        this->edgeRatio = pModelActionInput->edgeRatio;
        this->edgeLength = pModelActionInput->edgeLength;
        this->elementArea = pModelActionInput->elementArea;
        this->tolerance = pModelActionInput->tolerance;
        this->geometryTransformInput = pModelActionInput->geometryTransformInput;
        this->variableType = pModelActionInput->variableType;
    }
}

ModelActionInput::ModelActionInput(uint modelID, uint priority)
    : modelID(modelID)
    , priority(priority)
    , type(MODEL_ACTION_NONE)
    , nIterations(10)
    , separationAngle(0.0)
    , closeHole(false)
    , elementType(R_ELEMENT_NONE)
    , nodeIDs(QList<uint>())
    , elementIDs(QList<uint>())
    , entityIDs(QList<SessionEntityID>())
    , point(RR3Vector(0.0,0.0,0.0))
    , edgeRatio(6.0)
    , edgeLength(RConstants::eps)
    , elementArea(RConstants::eps)
    , tolerance(RConstants::findMachineDoubleEpsilon())
    , variableType(R_VARIABLE_NONE)
{
    this->_init();
}

ModelActionInput::ModelActionInput(const ModelActionInput &modelActionInput)
{
    this->_init(&modelActionInput);
}

ModelActionInput::~ModelActionInput()
{
}

ModelActionInput &ModelActionInput::operator =(const ModelActionInput &modelActionInput)
{
    this->_init(&modelActionInput);
    return (*this);
}

bool ModelActionInput::operator <(const ModelActionInput &modelActionInput) const
{
    return (this->priority < modelActionInput.priority);
}

uint ModelActionInput::getModelID() const
{
    return this->modelID;
}

ModelActionType ModelActionInput::getType() const
{
    return this->type;
}

uint ModelActionInput::getNIterations() const
{
    return this->nIterations;
}

double ModelActionInput::getSeparationAngle() const
{
    return this->separationAngle;
}

bool ModelActionInput::getCloseHole() const
{
    return this->closeHole;
}

RElementType ModelActionInput::getElementType() const
{
    return this->elementType;
}

const QList<uint> &ModelActionInput::getNodeIDs() const
{
    return this->nodeIDs;
}

const QList<uint> &ModelActionInput::getElementIDs() const
{
    return this->elementIDs;
}

const QList<SessionEntityID> &ModelActionInput::getEntityIDs() const
{
    return this->entityIDs;
}

const RR3Vector &ModelActionInput::getPoint() const
{
    return this->point;
}

double ModelActionInput::getEdgeRatio() const
{
    return this->edgeRatio;
}

double ModelActionInput::getEdgeLength() const
{
    return this->edgeLength;
}

double ModelActionInput::getElementArea() const
{
    return this->elementArea;
}

double ModelActionInput::getTolerance() const
{
    return this->tolerance;
}

const GeometryTransformInput &ModelActionInput::getGeometryTransformInput() const
{
    return this->geometryTransformInput;
}

RVariableType ModelActionInput::getVariableType() const
{
    return this->variableType;
}

void ModelActionInput::setAutoMarkSurfaces(double separationAngle)
{
    this->type = MODEL_ACTION_AUTOMARK_SURFACES;
    this->separationAngle = separationAngle;
}

void ModelActionInput::setMarkSurfaces(double separationAngle, const QList<uint> &elementIDs)
{
    this->type = MODEL_ACTION_MARK_SURFACES;
    this->separationAngle = separationAngle;
    this->elementIDs = elementIDs;
}

void ModelActionInput::setCloseSurfaceHole(const QList<uint> &edgeIDs)
{
    this->type = MODEL_ACTION_CLOSE_SURFACE_HOLE;
    this->elementIDs = edgeIDs;
}

void ModelActionInput::setMergeNearNodes(double tolerance)
{
    this->type = MODEL_ACTION_MERGE_NEAR_NODES;
    this->tolerance = tolerance;
}

void ModelActionInput::setMergeEntities(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_MERGE_ENTITIES;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setRemoveEntities(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_REMOVE_ENTITIES;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setCreateElement(const QList<uint> &nodeIDs, RElementType elementType)
{
    this->type = MODEL_ACTION_CREATE_ELEMENT;
    this->nodeIDs = nodeIDs;
    this->elementType = elementType;
}

void ModelActionInput::setRemoveNodes(const QList<uint> &nodeIDs, bool closeHole)
{
    this->type = MODEL_ACTION_REMOVE_NODES;
    this->nodeIDs = nodeIDs;
    this->closeHole = closeHole;
}

void ModelActionInput::setPurgeUnusedNodes()
{
    this->type = MODEL_ACTION_PURGE_UNUSED_NODES;
}

void ModelActionInput::setPurgeUnusedElements()
{
    this->type = MODEL_ACTION_PURGE_UNUSED_ELEMENTS;
}

void ModelActionInput::setRemoveDuplicateNodes()
{
    this->type = MODEL_ACTION_REMOVE_DUPLICATE_NODES;
}

void ModelActionInput::setRemoveDuplicateElements()
{
    this->type = MODEL_ACTION_REMOVE_DUPLICATE_ELEMENTS;
}

void ModelActionInput::setRemoveElements(const QList<uint> &elementIDs, bool closeHole)
{
    this->type = MODEL_ACTION_REMOVE_ELEMENTS;
    this->elementIDs = elementIDs;
    this->closeHole = closeHole;
}

void ModelActionInput::setGenerateLinesFromEdges(double separationAngle)
{
    this->type = MODEL_ACTION_GENERATE_LINE_FROM_EDGES;
    this->separationAngle = separationAngle;
}

void ModelActionInput::setFindSliverElements(double edgeRatio)
{
    this->type = MODEL_ACTION_FIND_SLIVER_ELEMENTS;
    this->edgeRatio = edgeRatio;
}

void ModelActionInput::setFixSliverElements(double edgeRatio)
{
    this->type = MODEL_ACTION_FIX_SLIVER_ELEMENTS;
    this->edgeRatio = edgeRatio;
}

void ModelActionInput::setFindIntersectedElements()
{
    this->type = MODEL_ACTION_FIND_INTERSECTED_ELEMENTS;
}

void ModelActionInput::setBreakIntersectedElements(uint nIterations)
{
    this->type = MODEL_ACTION_BREAK_INTERSECTED_ELEMENTS;
    this->nIterations = nIterations;
}

void ModelActionInput::setExportSliverElements()
{
    this->type = MODEL_ACTION_EXPORT_SLIVER_ELEMENTS;
}

void ModelActionInput::setExportIntersectedElements()
{
    this->type = MODEL_ACTION_EXPORT_INTERSECTED_ELEMENTS;
}

void ModelActionInput::setBoolDifference(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_BOOL_DIFFERENCE;
    this->entityIDs.clear();
    this->entityIDs = entityIDs;
}

void ModelActionInput::setBoolIntersection(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_BOOL_INTERSECTION;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setBoolUnion(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_BOOL_UNION;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setCheckPointInsideSurface(const RR3Vector &point, const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_CHECK_POINT_INSIDE_SURFACE;
    this->point = point;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setGeneratePatches()
{
    this->type = MODEL_ACTION_GENERATE_PATCHES;
}

void ModelActionInput::setCalculateViewFactors()
{
    this->type = MODEL_ACTION_CALCULATE_VIEWFACTORS;
}

void ModelActionInput::setSwapSurfaceElementNormal(const QList<uint> &elementIDs)
{
    this->type = MODEL_ACTION_SWAP_SURFACE_ELEMENT_NORMAL;
    this->elementIDs = elementIDs;
}

void ModelActionInput::setSwapSurfaceNormals(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_SWAP_SURFACE_NORMALS;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setSyncSurfaceNormals()
{
    this->type = MODEL_ACTION_SYNC_SURFACE_NORMALS;
}

void ModelActionInput::setCoarsenSurface(const QList<SessionEntityID> &entityIDs, double edgeLength, double elementArea)
{
    this->type = MODEL_ACTION_COARSEN_SURFACE;
    this->entityIDs = entityIDs;
    this->edgeLength = edgeLength;
    this->elementArea = elementArea;
}

void ModelActionInput::setTetrahedralizeSurface(const QList<SessionEntityID> &entityIDs)
{
    this->type = MODEL_ACTION_TETRAHEDRALIZE_SURFACE;
    this->entityIDs = entityIDs;
}

void ModelActionInput::setConsolidate()
{
    this->type = MODEL_ACTION_CONSOLIDATE;
}

void ModelActionInput::setGeometryTransform(GeometryTransformInput geometryTransformInput)
{
    this->type = MODEL_ACTION_GEOMETRY_TRANSFORM;
    this->geometryTransformInput = geometryTransformInput;
}

void ModelActionInput::setApplyVariableOnNode(RVariableType variableType)
{
    this->type = MODEL_ACTION_APPLY_VARIABLE_ON_NODE;
    this->variableType = variableType;
}

void ModelActionInput::setRemoveVariable(RVariableType variableType)
{
    this->type = MODEL_ACTION_REMOVE_VARIABLE;
    this->variableType = variableType;
}
