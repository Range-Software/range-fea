#ifndef MODEL_ACTION_H
#define MODEL_ACTION_H

#include <QList>

#include <rbl_job.h>

#include "model_action_input.h"

class ModelAction : public RJob
{

    Q_OBJECT

    protected:

        //! Ordered list of model actions.
        QList<ModelActionInput> actions;

    public:

        //! Constructor.
        explicit ModelAction();

        //! Add new action.
        void addAction(const ModelActionInput &modelActionInput);

    protected:

        //! Run job.
        virtual int perform() override final;

        //! Execute action based on action input.
        void executeAction(const ModelActionInput &modelActionInput);

        //! Auto-mark surfaces.
        void autoMarkSurfaces(const ModelActionInput &modelActionInput);

        //! Mark surfaces.
        void markSurfaces(const ModelActionInput &modelActionInput);

        //! Close surface hole.
        void closeSurfaceHole(const ModelActionInput &modelActionInput);

        //! Merge nearNodes.
        void mergeNearNodes(const ModelActionInput &modelActionInput);

        //! Merge entities.
        void mergeEntities(const ModelActionInput &modelActionInput);

        //! Remove entities.
        void removeEntities(const ModelActionInput &modelActionInput);

        //! Create element.
        void createElement(const ModelActionInput &modelActionInput);

        //! Remove nodes.
        void removeNodes(const ModelActionInput &modelActionInput);

        //! Purge unused nodes.
        void purgeUnusedNodes(const ModelActionInput &modelActionInput);

        //! Purge unused elements.
        void purgeUnusedElements(const ModelActionInput &modelActionInput);

        //! Merge duplicate nodes.
        void removeDuplicateNodes(const ModelActionInput &modelActionInput);

        //! Merge duplicate elements.
        void removeDuplicateElements(const ModelActionInput &modelActionInput);

        //! Remove elements.
        void removeElements(const ModelActionInput &modelActionInput);

        //! Generate line from surface edges.
        void generateLineFromEdges(const ModelActionInput &modelActionInput);

        //! Find sliver elements.
        void findSliverElements(const ModelActionInput &modelActionInput);

        //! Fix sliver elements.
        void fixSliverElements(const ModelActionInput &modelActionInput);

        //! Export sliver elements.
        void exportSliverElements(const ModelActionInput &modelActionInput);

        //! Find intersected elements.
        void findIntersectedElements(const ModelActionInput &modelActionInput);

        //! Break intersected elements.
        void breakIntersectedElements(const ModelActionInput &modelActionInput);

        //! Export intersected elements.
        void exportIntersectedElements(const ModelActionInput &modelActionInput);

        //! Perform boolean difference operation.
        void boolDifference(const ModelActionInput &modelActionInput);

        //! Perform boolean intersection operation.
        void boolIntersection(const ModelActionInput &modelActionInput);

        //! Perform boolean union operation.
        void boolUnion(const ModelActionInput &modelActionInput);

        //! Perform check if point is inside surface.
        void checkPointInsideSurface(const ModelActionInput &modelActionInput);

        //! Generate patches.
        void generatePatches(const ModelActionInput &modelActionInput);

        //! Calculate view-factors.
        void calculateViewFactors(const ModelActionInput &modelActionInput);

        //! Swap surface element normal.
        void swapSurfaceElementNormal(const ModelActionInput &modelActionInput);

        //! Swap surface normals.
        void swapSurfaceNormals(const ModelActionInput &modelActionInput);

        //! Synchronize surface normals.
        void syncSurfaceNormals(const ModelActionInput &modelActionInput);

        //! Coarsen surface.
        void coarsenSurface(const ModelActionInput &modelActionInput);

        //! Tetrahedralize surface.
        void tetrahedralizeSurface(const ModelActionInput &modelActionInput);

        //! Consolidate geometry.
        void consolidate(const ModelActionInput &modelActionInput);

        //! Transform.
        void geometryTransform(const ModelActionInput &modelActionInput);

        //! Apply variable on node.
        void applyVariableOnNode(const ModelActionInput &modelActionInput);

        //! Remove variable.
        void removeVariable(const ModelActionInput &modelActionInput);

};

#endif // MODEL_ACTION_H
