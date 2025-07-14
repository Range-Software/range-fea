#ifndef MODEL_ACTION_INPUT_H
#define MODEL_ACTION_INPUT_H

#include <rml_element.h>

#include "geometry_transform_input.h"
#include "pick_list.h"

typedef enum _ModelActionType
{
    MODEL_ACTION_NONE = 0,
    MODEL_ACTION_AUTOMARK_SURFACES,
    MODEL_ACTION_MARK_SURFACES,
    MODEL_ACTION_CLOSE_SURFACE_HOLE,
    MODEL_ACTION_MERGE_NEAR_NODES,
    MODEL_ACTION_MERGE_ENTITIES,
    MODEL_ACTION_REMOVE_ENTITIES,
    MODEL_ACTION_CREATE_ELEMENT,
    MODEL_ACTION_REMOVE_NODES,
    MODEL_ACTION_PURGE_UNUSED_NODES,
    MODEL_ACTION_PURGE_UNUSED_ELEMENTS,
    MODEL_ACTION_REMOVE_DUPLICATE_NODES,
    MODEL_ACTION_REMOVE_DUPLICATE_ELEMENTS,
    MODEL_ACTION_REMOVE_ELEMENTS,
    MODEL_ACTION_GENERATE_LINE_FROM_EDGES,
    MODEL_ACTION_FIND_SLIVER_ELEMENTS,
    MODEL_ACTION_FIX_SLIVER_ELEMENTS,
    MODEL_ACTION_FIND_INTERSECTED_ELEMENTS,
    MODEL_ACTION_BREAK_INTERSECTED_ELEMENTS,
    MODEL_ACTION_EXPORT_SLIVER_ELEMENTS,
    MODEL_ACTION_EXPORT_INTERSECTED_ELEMENTS,
    MODEL_ACTION_BOOL_DIFFERENCE,
    MODEL_ACTION_BOOL_INTERSECTION,
    MODEL_ACTION_BOOL_UNION,
    MODEL_ACTION_CHECK_POINT_INSIDE_SURFACE,
    MODEL_ACTION_GENERATE_PATCHES,
    MODEL_ACTION_CALCULATE_VIEWFACTORS,
    MODEL_ACTION_SWAP_SURFACE_ELEMENT_NORMAL,
    MODEL_ACTION_SWAP_SURFACE_NORMALS,
    MODEL_ACTION_SYNC_SURFACE_NORMALS,
    MODEL_ACTION_COARSEN_SURFACE,
    MODEL_ACTION_TETRAHEDRALIZE_SURFACE,
    MODEL_ACTION_CONSOLIDATE,
    MODEL_ACTION_GEOMETRY_TRANSFORM,
    MODEL_ACTION_APPLY_VARIABLE_ON_NODE,
    MODEL_ACTION_REMOVE_VARIABLE,
    MODEL_ACTION_N_TYPES
} ModelActionType;

class ModelActionInput
{

    protected:

        //! Model ID.
        uint modelID;
        //! Priority.
        uint priority;
        //! Model action type.
        ModelActionType type;
        //! Number of iterations.
        uint nIterations;
        //! Separation angle.
        double separationAngle;
        //! Close hole.
        bool closeHole;
        //! Element type.
        RElementType elementType;
        //! Node IDs;
        QList<uint> nodeIDs;
        //! Element IDs;
        QList<uint> elementIDs;
        //! List of entities.
        QList<SessionEntityID> entityIDs;
        //! Point.
        RR3Vector point;
        //! Edge ration.
        double edgeRatio;
        //! Edge length.
        double edgeLength;
        //! Element area.
        double elementArea;
        //! Tomerance.
        double tolerance;
        //! Geometry transform input.
        GeometryTransformInput geometryTransformInput;
        //! Variable.
        RVariableType variableType;

    private:

        //! Internal initialization function.
        void _init(const ModelActionInput *pModelActionInput = nullptr);

    public:

        //! Constructor.
        explicit ModelActionInput(uint modelID = 0, uint priority = 1000);

        //! Copy constructor.
        ModelActionInput(const ModelActionInput &modelActionInput);

        //! Destructor.
        ~ModelActionInput();

        //! Assignment operator.
        ModelActionInput &operator =(const ModelActionInput &modelActionInput);

        //! Less then operator.
        bool operator <(const ModelActionInput &modelActionInput) const;

        //! Return model ID.
        uint getModelID() const;

        //! Return model action type.
        ModelActionType getType() const;

        //! Return number of iterations.
        uint getNIterations() const;

        //! Return separation angle.
        double getSeparationAngle() const;

        //! Return close hole.
        bool getCloseHole() const;

        //! Return element type.
        RElementType getElementType() const;

        //! Return vector of node IDs.
        const QList<uint> &getNodeIDs() const;

        //! Return vector of element IDs.
        const QList<uint> &getElementIDs() const;

        //! Return vector of entity IDs.
        const QList<SessionEntityID> &getEntityIDs() const;

        //! Return point.
        const RR3Vector &getPoint() const;

        //! Return edge ration.
        double getEdgeRatio() const;

        //! Return edge length.
        double getEdgeLength() const;

        //! Return element area.
        double getElementArea() const;

        //! Return tolerance.
        double getTolerance() const;

        //! Return geometry transform input.
        const GeometryTransformInput &getGeometryTransformInput() const;

        //! Return variable type.
        RVariableType getVariableType() const;

        //! Set auto-mark surfaces and its input.
        void setAutoMarkSurfaces(double separationAngle);

        //! Set mark surfaces and its input.
        void setMarkSurfaces(double separationAngle, const QList<uint> &elementIDs);

        //! Set remove nodes.
        void setCloseSurfaceHole(const QList<uint> &edgeIDs);

        //! Set merge near nodes and its input.
        void setMergeNearNodes(double tolerance);

        //! Set merge entities and its input.
        void setMergeEntities(const QList<SessionEntityID> &entityIDs);

        //! Set remove entities and its input.
        void setRemoveEntities(const QList<SessionEntityID> &entityIDs);

        //! Set create element.
        void setCreateElement(const QList<uint> &nodeIDs, RElementType elementType);

        //! Set remove nodes.
        void setRemoveNodes(const QList<uint> &nodeIDs, bool closeHole);

        //! Set purge unused nodes.
        void setPurgeUnusedNodes();

        //! Set purge unused elements.
        void setPurgeUnusedElements();

        //! Set merge duplicate nodes.
        void setRemoveDuplicateNodes();

        //! Set merge duplicate elements.
        void setRemoveDuplicateElements();

        //! Set remove elements.
        void setRemoveElements(const QList<uint> &elementIDs, bool closeHole);

        //! Set generate lines from surface edges.
        void setGenerateLinesFromEdges(double separationAngle);

        //! Set find sliver elements.
        void setFindSliverElements(double edgeRatio);

        //! Set fix sliver elements.
        void setFixSliverElements(double edgeRatio);

        //! Set find intersected surface elements.
        void setFindIntersectedElements();

        //! Set break intersected surface elements.
        void setBreakIntersectedElements(uint nIterations);

        //! Set export sliver surface elements.
        void setExportSliverElements();

        //! Set export intersected surface elements.
        void setExportIntersectedElements();

        //! Set bool difference.
        void setBoolDifference(const QList<SessionEntityID> &entityIDs);

        //! Set bool intersection.
        void setBoolIntersection(const QList<SessionEntityID> &entityIDs);

        //! Set bool union.
        void setBoolUnion(const QList<SessionEntityID> &entityIDs);

        //! Set to check if point is inside surface.
        void setCheckPointInsideSurface(const RR3Vector &point, const QList<SessionEntityID> &entityIDs);

        //! Set generate patches.
        void setGeneratePatches();

        //! Set calculate view-factors.
        void setCalculateViewFactors();

        //! Set swap surface element normal.
        void setSwapSurfaceElementNormal(const QList<uint> &elementIDs);

        //! Set swap surface normals.
        void setSwapSurfaceNormals(const QList<SessionEntityID> &entityIDs);

        //! Set synchronize surface normals.
        void setSyncSurfaceNormals();

        //! Set coarsen surface.
        void setCoarsenSurface(const QList<SessionEntityID> &entityIDs, double edgeLength, double elementArea);

        //! Set tetrahedralize surface.
        void setTetrahedralizeSurface(const QList<SessionEntityID> &entityIDs);

        //! Set consolidate.
        void setConsolidate();

        //! Set geometry transform.
        void setGeometryTransform(GeometryTransformInput geometryTransformInput);

        //! Set apply variable on node.
        void setApplyVariableOnNode(RVariableType variableType);

        //! Set remove variable.
        void setRemoveVariable(RVariableType variableType);

};

#endif // MODEL_ACTION_INPUT_H
