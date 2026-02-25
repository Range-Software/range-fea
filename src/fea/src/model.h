#ifndef MODEL_H
#define MODEL_H

#include <QColor>
#include <QSet>
#include <QStack>
#include <QMutex>

#include <rml_model.h>
#include <rbl_limit_box.h>

#include "pick_item.h"
#include "pick_value.h"
#include "geometry_transform_input.h"

class GLWidget;

class Model : public RModel
{

    public:

        enum ConsolidateAction
        {
            ConsolidateNone                = 0,
            ConsolidateSurfaceNeighbors    = 1 << 1,
            ConsolidateVolumeNeighbors     = 1 << 2,
            ConsolidateEdgeNodes           = 1 << 3,
            ConsolidateEdgeElements        = 1 << 4,
            ConsolidateHoleElements        = 1 << 5,
            ConsolidateSliverElements      = 1 << 6,
            ConsolidateIntersectedElements = 1 << 7,
            ConsolidateMeshInput           = 1 << 8
        };

        static const int ConsolidateActionAll;

        static const double SliverElementEdgeRatio;

    protected:

        //! Lock enabling model to be drawn.
        QMutex drawLock;

        //! Book of edge nodes.
        QVector<bool> edgeNodes;
        //! Set of edge elements.
        QVector<RElement> edgeElements;
        //! Set of hole elements.
        QVector<RElement> holeElements;
        //! Set of sliver elements.
        QSet<uint> sliverElements;
        //! Set of intersected elements.
        QSet<uint> intersectedElements;
        //! Model filename.
        QString fileName;
        //! Mesh generator input object.
        RMeshInput meshInput;
        //! View-factor matrix.
        RViewFactorMatrix viewFactorMatrix;
        //! Patch colors.
        QList<QColor> patchColors;

        //! Undo stack.
        QStack<Model> undoStack;
        //! Redo stack.
        QStack<Model> redoStack;
        //! Action description.
        QString actionDescription;

    private:

        //! Internal initialization function.
        void _init(const Model *pModel = nullptr);

        //! Return const pointer to element group data.
        const REntityGroupData *getElementGroupData(REntityGroupType elementGroupType,
                                                     uint              position) const;

        //! Return pointer to element group data.
        REntityGroupData *getElementGroupData(REntityGroupType elementGroupType,
                                               uint              position);

    public:

        //! Constructor.
        Model();

        //! Copy constructor.
        Model(const Model &model);

        //! Convert constructor from MSH (old range) model.
        Model (const RModelMsh &modelMsh);

        //! Convert constructor from STL model.
        Model (const RModelStl &modelStl);

        //! Convert constructor from RAW model.
        Model (const RModelRaw &modelRaw,
               const QString &name = QString(),
               const QString &description = QString(),
               bool consolidate = true);

        //! Assignment operator.
        Model & operator = (const Model &model);

        //! Insert model (add entities).
        void insertModel(const Model &model, bool mergeNearNodes, double tolerance = 0.0, bool findNearest = false);

        //! Return model filename.
        const QString & getFileName() const;

        //! Set new model filename.
        void setFileName(const QString &fileName);

        //! Return const reference to mesh generator input.
        const RMeshInput & getMeshInput() const;

        //! Return reference to mesh generator input.
        RMeshInput & getMeshInput();

        //! Set mesh generator input.
        void setMeshInput(const RMeshInput &meshInput);

        //! Update mesh generator inut volume constrain.
        void initializeMeshInput();

        //! Return const reference to view-factor matrix.
        const RViewFactorMatrix &getViewFactorMatrix() const;

        //! Return reference to view-factor matrix.
        RViewFactorMatrix &getViewFactorMatrix();

        //! Return if entity can be colored by patch.
        bool canColorByPatch() const;

        //! Return if entity can be colored by view-factor.
        bool canColorByViewFactor() const;

        //! Return const reference to patch colors.
        const QList<QColor> &getPatchColors() const;

        //! Return reference to patch colors.
        QList<QColor> &getPatchColors();

        //! Generate patch colors.
        void generatePatchColors();

        //! Return last log file name.
        const QString & getLastLogFileName() const;

        //! Return last log file name.
        void setLastLogFileName(const QString &lastLogFileName);

        //! Return last convergence file name.
        const QString & getLastConvergenceFileName() const;

        //! Return last convergence file name.
        void setLastConvergenceFileName(const QString &lastConvergenceFileName);

        //! Return last monitoring file name.
        const QString & getLastMonitoringFileName() const;

        //! Return last monitoring file name.
        void setLastMoinitoringFileName(const QString &lastMonitoringFileName);

        //! Generate line(s) from surface edges.
        void generateLineFromSurfaceEdges(double separationAngle);

        //! Automark surfaces.
        void autoMarkSurfaces(double angle);

        //! Automark surfaces.
        void markSurface(double angle, QList<uint> elementIDs);

        //! Close surface hole.
        void closeSurfaceHole(QList<uint> edgeIDs);

        //! Transform model geometry.
        void transformGeometry(const GeometryTransformInput &geometryTransformInput, const QList<SessionEntityID> &entityIDs);

        //! Coarsen surface elements.
        uint coarsenSurfaceElements(const std::vector<uint> surfaceIDs, double edgeLength, double elementArea);

        //! Tetrahedralize surface.
        uint tetrahedralizeSurface(const std::vector<uint> surfaceIDs);

        //! Merge near nodes.
        uint mergeNearNodes(double tolerance = RConstants::eps);

        //! Purge unused nodes.
        uint purgeUnusedNodes();

        //! Purge unused elements.
        uint purgeUnusedElements();

        //! Merge duplicate elements.
        uint removeDuplicateElements();

        //! Fix sliver elements.
        //! Return number of affected elements.
        uint fixSliverElements(double edgeRatio);

        //! Update sliver elements list.
        void updateSliverElements(double edgeRatio);

        //! Update intersected elements list.
        void updateIntersectedElements();

        //! Break intersected elements list.
        uint breakIntersectedElements(uint nIterations);

        //! Export sliver elements list.
        bool exportSliverElements() const;

        //! Export intersected elements list.
        bool exportIntersectedElements() const;

        //! Perform boolean difference operation.
        //! Return true if successful.
        bool boolDifference(uint nIterations, QList<uint> surfaceEntityIDs, uint cuttingSurfaceEntityId);

        //! Perform boolean intersection operation.
        //! entityIDs specifies global entity ID.
        //! Return true if successful.
        bool boolIntersection(uint nIterations, QList<uint> surfaceEntityIDs);

        //! Perform boolean union operation.
        //! Return true if successful.
        bool boolUnion(uint nIterations, QList<uint> surfaceEntityIDs);

        //! Chech whether the element with given id is sliver.
        bool isSliver(uint elementID) const;

        //! Chech whether the element with given id is intersecting with other element.
        bool isIntersected(uint elementID) const;

        //! Check whether the model selection status is same as
        //! the provided one.
        bool isSelected(bool selected = true) const;

        //! Check whether the entity group selection status is same as
        //! the provided one.
        bool isSelected(REntityGroupType elementGroupType,
                        bool              selected = true) const;

        //! Return true if model is selected.
        bool getSelected() const;

        //! Return true if entity is selected.
        bool getSelected(REntityGroupType elementGroupType,
                         uint             position) const;

        //! Set model selection status.
        void setSelected(bool selected);

        //! Set entity selection status.
        void setSelected(REntityGroupType elementGroupType,
                         uint              position,
                         bool              selected);

        //! Return list of all selected entity IDs.
        QList<SessionEntityID> getSelectedEntityIDs(uint modelID) const;

        //! Return list of all picked entity IDs.
        QList<SessionEntityID> getPickedEntityIDs(uint modelID) const;

        //! Return list of all visible entity IDs.
        QList<SessionEntityID> getVisibleEntityIDs(uint modelID) const;

        //! Return list of element IDs for given list of entity IDs.
        QSet<uint> getElementIDs(const QList<SessionEntityID> &entityIDs) const;

        //! Return list of node IDs for given list of element IDs.
        QSet<uint> getNodeIDs(const QSet<uint> &elementIDs) const;

        //! Return number of sliver elements.
        uint getNSlivers() const;

        //! Return number of intersected elements.
        uint getNIntersected() const;

        //! Return number of hole elements.
        uint getNHoleElements() const;

        //! Check whether the model visibility is same as
        //! the provided one.
        bool isVisible(bool visible = true) const;

        //! Check whether the entity group visibility is same as
        //! the provided one.
        bool isVisible(REntityGroupType elementGroupType,
                       bool              visible = true) const;

        //! Return true if entity is visible.
        bool getVisible(REntityGroupType elementGroupType,
                        uint              position) const;

        //! Set entity visibility.
        void setVisible(REntityGroupType elementGroupType,
                        uint              position,
                        bool              visible);

        //! Return true if draw wire.
        bool getDrawWire(REntityGroupType elementGroupType,
                         uint              position) const;

        //! Set entity draw wire.
        void setDrawWire(REntityGroupType elementGroupType,
                         uint              position,
                         bool              drawWire);

        //! Return true if draw element edges.
        bool getDrawEdges(REntityGroupType elementGroupType,
                          uint              position) const;

        //! Set entity draw element edges.
        void setDrawEdges(REntityGroupType elementGroupType,
                          uint              position,
                          bool              drawEdges);

        //! Return true if draw element nodes.
        bool getDrawNodes(REntityGroupType elementGroupType,
                          uint              position) const;

        //! Set entity draw element nodes.
        void setDrawNodes(REntityGroupType elementGroupType,
                          uint              position,
                          bool              drawNodes);

        //! Return true if draw element numbers.
        bool getDrawElementNumbers(REntityGroupType elementGroupType,
                                   uint              position) const;

        //! Set entity draw element numbers.
        void setDrawElementNumbers(REntityGroupType elementGroupType,
                                   uint              position,
                                   bool              drawElementNumbers);

        //! Return true if draw node numbers.
        bool getDrawNodeNumbers(REntityGroupType elementGroupType,
                                uint              position) const;

        //! Set entity draw node numbers.
        void setDrawNodeNumbers(REntityGroupType elementGroupType,
                                uint              position,
                                bool              drawNodeNumbers);

        //! Return true if draw arrow heads.
        bool getDrawArrowHeads(REntityGroupType elementGroupType,
                               uint             position) const;

        //! Set entity draw arrow heads.
        void setDrawArrowHeads(REntityGroupType elementGroupType,
                               uint             position,
                               bool             drawArrowHeads);

        //! Return true if draw equal arrow lengths.
        bool getDrawEqualArrowLengths(REntityGroupType entityGroupType,
                                      uint             position) const;

        //! Set entity draw equal arrow lengths.
        void setDrawEqualArrowLengths(REntityGroupType entityGroupType,
                                      uint             position,
                                      bool             drawEqualArrowLengths);

        //! Return true if draw arrow from.
        bool getDrawArrowFrom(REntityGroupType entityGroupType,
                              uint             position) const;

        //! Set entity draw arrow from.
        void setDrawArrowFrom(REntityGroupType entityGroupType,
                              uint             position,
                              bool             drawArrowFrom);

        //! Return true if color by patch.
        bool getColorByPatch(REntityGroupType entityGroupType,
                             uint             position) const;

        //! Set entity color by patch.
        void setColorByPatch(REntityGroupType entityGroupType,
                             uint             position,
                             bool             colorByPatch);

        //! Return true if color by view-factor.
        bool getColorByViewFactor(REntityGroupType entityGroupType,
                                  uint             position) const;

        //! Set entity color by view-factor.
        void setColorByViewFactor(REntityGroupType entityGroupType,
                                  uint             position,
                                  bool             colorByViewFactor);

        //! Return entity color.
        QColor getColor(REntityGroupType elementGroupType,
                        uint              position) const;

        //! Set entity color.
        void setColor(REntityGroupType  elementGroupType,
                      uint               position,
                      const QColor      &color);

        //! Compute bounding box for an entity group.
        //! Returns true if the entity has elements, false if empty.
        bool computeEntityBounds(REntityGroupType entityType, uint entityIndex, RLimitBox &bounds) const;

        //! Lock model for drawing.
        void glDrawLock();

        //! Try-lock model for drawing.
        bool glDrawTrylock();

        //! Unlock model for drawing.
        void glDrawUnlock();

        //! Draw model to OpenGL context.
        void glDraw(GLWidget *glWidget) const;

        //! Draw picked items to OpenGL context.
        void glDraw(GLWidget *glWidget, const QVector<PickItem> &pickedItems) const;

        //! Return results values for picked item.
        QMultiMap<RVariableType, PickValue> getPickedResultsValues(const PickItem &rPickItem) const;

        //! Return true if node is on edge.
        bool nodeIsOnEdge(uint nodeID) const;

        //! Return true if element is on edge.
        bool elementIsOnEdge(uint elementID) const;

        //! Find intersected element by pick ray.
        bool findPickedElement(const RR3Vector &position, const RR3Vector &direction, double tolerance, PickItem &pickItem);

        //! Find intersected node by pick ray.
        bool findPickedNode(const RR3Vector &position, const RR3Vector &direction, double tolerance, PickItem &pickItem);

        //! Find intersected edge by pick ray.
        bool findPickedHoleElement(const RR3Vector &position, const RR3Vector &direction, double tolerance, PickItem &pickItem);

        //! Update model.
        void update(const RModel &rModel);

        //! Read model from the file.
        void read (const QString &fileName);

        //! Write model to the file.
        void write (const QString &fileName, bool writeLinkFile = true);

        //! Load view-factor matrix.
        void loadViewFactorMatrix();

        //! Unload view-factor matrix.
        void unloadViewFactorMatrix();

        //! Update (recompute) internal structures.
        void consolidate(int consolidateActionMask);

        //! Build file name from data directory.
        QString buildDataFileName(const QString &format, bool withTimeStep = false) const;

        //! Build file name from doc directory.
        QString buildDocFileName(const QString &format, bool withTimeStep = false) const;

        //! Build file name from tmp directory.
        QString buildTmpFileName(const QString &format, bool withTimeStep = false) const;

        //! Build file name from tmp directory.
        QString buildTmpFileName(const QString &format, const QString &idString = QString()) const;

        //! Build screen-shot file name.
        QString buildScreenShotFileName(const QString &format = QString("png")) const;

        //! Build animation file name.
        QString buildAnimationFileName(const QString &format = QString("mp4")) const;

        //! Return list of existing results record files.
        QList<QString> getRecordFiles(bool onlyExistingFiles = false) const;

        //! Return list of existing documents.
        QList<QString> getDocumentFiles() const;

        //! Return size of undo stack.
        uint getUndoStackSize() const;

        //! Perform undo.
        void undo(uint revision);

        //! Return undo action message.
        QString getUndoActionMessage() const;

        //! Return size of redo stack.
        uint getRedoStackSize() const;

        //! Perform redo.
        void redo(uint revision);

        //! Return redo action message.
        QString getRedoActionMessage() const;

        //! Store current version.
        void storeCurentVersion(uint maxDepth, const QString &actionDescription);

        //! Update histor stackm size.
        void updateHistoryStackSize(uint maxDepth);

        //! Clear edge nodes.
        void clearEdgeNodes();

    protected:

        //! Return list of edge node IDs for given list of entity IDs.
        QSet<uint> findEdgeNodeIDs(const QList<SessionEntityID> &entityIDs) const;

        //! Split given nodes and return map of original nodes pointing to new nodes IDs.
        //! New nodes will be assigned to elements except those provided if applicable.
        QMap<uint,uint> splitNodes(const QSet<uint> &nodeIDs, const QSet<uint> &elementIDs);

        //! Generate edge elements from given set of edge nodes.
        QList<RElement> generateEdgeElements(const QSet<uint> &edgeNodeIDs, const QSet<uint> &elementIDs) const;

        //! Create new elements between splitted nodes which formed edge elements.
        void createSweepEdgeElements(const QList<RElement> &edgeElements, const QMap<uint,uint> &edgeNodeMap, bool useLastGroupID, bool selectNewEntities, bool showNewEntities);

        //! Find edge elements.
        QVector<RElement> findEdgeElements(double separationAngle) const;

        //! Find hole elements.
        QVector<RElement> findHoleElements() const;

        friend class ModelAction;

};

#endif /* MODEL_H */
