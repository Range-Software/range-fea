#ifndef ACTION_H
#define ACTION_H

#include <QAction>

#include <rcl_cloud_session_manager.h>

#include <rgl_action.h>

#define ACTION_GROUP_ALL (             \
    Action::ACTION_GROUP_SESSION     | \
    Action::ACTION_GROUP_MODEL       | \
    Action::ACTION_GROUP_MATERIAL    | \
    Action::ACTION_GROUP_GEOMETRY    | \
    Action::ACTION_GROUP_PROBLEM     | \
    Action::ACTION_GROUP_SOLVER      | \
    Action::ACTION_GROUP_REPORT      | \
    Action::ACTION_GROUP_APPLICATION | \
    Action::ACTION_GROUP_CLOUD         \
    )

class Action : public RAction
{

    Q_OBJECT

    public:

        typedef int GroupTypeMask;

        enum GroupType
        {
            ACTION_GROUP_NONE        = 0,
            ACTION_GROUP_SESSION     = 1 << 0,
            ACTION_GROUP_MODEL       = 1 << 1,
            ACTION_GROUP_MATERIAL    = 1 << 2,
            ACTION_GROUP_GEOMETRY    = 1 << 3,
            ACTION_GROUP_PROBLEM     = 1 << 4,
            ACTION_GROUP_SOLVER      = 1 << 5,
            ACTION_GROUP_REPORT      = 1 << 6,
            ACTION_GROUP_APPLICATION = 1 << 7,
            ACTION_GROUP_CLOUD       = 1 << 8
        };

        enum Type
        {
            ACTION_SEPARATOR = 0,
            ACTION_SESSION_NEW,
            ACTION_SESSION_OPEN,
            ACTION_SESSION_SAVE,
            ACTION_SESSION_SAVE_AS,
            ACTION_SESSION_CLOSE,
            ACTION_MODEL_NEW,
            ACTION_MODEL_OPEN,
            ACTION_MODEL_SAVE,
            ACTION_MODEL_SAVE_AS,
            ACTION_MODEL_EXPORT_MSH,
            ACTION_MODEL_EXPORT_RAW,
            ACTION_MODEL_EXPORT_STL_ASCII,
            ACTION_MODEL_EXPORT_STL_BINARY,
            ACTION_MODEL_CLOSE,
            ACTION_MODEL_RELOAD_RESULTS,
            ACTION_MODEL_DROP_RESULTS,
            ACTION_MODEL_RENAME,
            ACTION_GEOMETRY_UNDO,
            ACTION_GEOMETRY_REDO,
            ACTION_GEOMETRY_DRAW_POINT,
            ACTION_GEOMETRY_DRAW_LINE,
            ACTION_GEOMETRY_DRAW_TRIANGLE,
            ACTION_GEOMETRY_DRAW_QUADRILATERAL,
            ACTION_GEOMETRY_DRAW_CIRCLE,
            ACTION_GEOMETRY_DRAW_ELLIPSE,
            ACTION_GEOMETRY_DRAW_TETRAHEDRON,
            ACTION_GEOMETRY_DRAW_HEXAHEDRON,
            ACTION_GEOMETRY_DRAW_CYLINDER,
            ACTION_GEOMETRY_DRAW_SPHERE,
            ACTION_GEOMETRY_DRAW_RAW,
            ACTION_GEOMETRY_CREATE_ELEMENT,
            ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS,
            ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS,
            ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS,
            ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS,
            ACTION_GEOMETRY_BOOL_UNION,
            ACTION_GEOMETRY_BOOL_DIFFERENCE,
            ACTION_GEOMETRY_BOOL_INTERSECTION,
            ACTION_GEOMETRY_MERGE_NEAR_NODES,
            ACTION_GEOMETRY_MOVE_NODE,
            ACTION_GEOMETRY_REMOVE_NODE,
            ACTION_GEOMETRY_REMOVE_NODE_AND_CLOSE_HOLE,
            ACTION_GEOMETRY_REMOVE_ELEMENT,
            ACTION_GEOMETRY_LINE_GENERATE_FROM_EDGES,
            ACTION_GEOMETRY_SURFACE_MARK,
            ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL,
            ACTION_GEOMETRY_SURFACE_SWAP_NORMALS,
            ACTION_GEOMETRY_SURFACE_SYNC_NORMALS,
            ACTION_GEOMETRY_SURFACE_CLOSE_HOLE,
            ACTION_GEOMETRY_SURFACE_COARSEN,
            ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA,
            ACTION_GEOMETRY_VECTOR_FIELD_CREATE,
            ACTION_GEOMETRY_VECTOR_FIELD_EDIT,
            ACTION_GEOMETRY_SCALAR_FIELD_CREATE,
            ACTION_GEOMETRY_SCALAR_FIELD_EDIT,
            ACTION_GEOMETRY_STREAM_LINE_CREATE,
            ACTION_GEOMETRY_STREAM_LINE_EDIT,
            ACTION_GEOMETRY_CUT_CREATE,
            ACTION_GEOMETRY_CUT_EDIT,
            ACTION_GEOMETRY_ISO_CREATE,
            ACTION_GEOMETRY_ISO_EDIT,
            ACTION_GEOMETRY_ENTITY_MERGE,
            ACTION_GEOMETRY_ENTITY_REMOVE,
            ACTION_GEOMETRY_TRANSFORM,
            ACTION_GEOMETRY_DEV_EXPORT_SLIVER_ELEMENTS,
            ACTION_GEOMETRY_DEV_EXPORT_INTERSECTED_ELEMENTS,
            ACTION_GEOMETRY_DEV_PURGE_UNUSED_NODES,
            ACTION_GEOMETRY_DEV_PURGE_UNUSED_ELEMENTS,
            ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_NODES,
            ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_ELEMENTS,
            ACTION_GEOMETRY_DEV_POINT_INSIDE_SURFACE,
            ACTION_GEOMETRY_DEV_TETRAHEDRALIZE_SURFACE,
            ACTION_GEOMETRY_DEV_CONSOLIDATE,
            ACTION_PROBLEM_TASK_FLOW,
            ACTION_PROBLEM_SOLVER_SETUP,
            ACTION_PROBLEM_DEFINE_MONITORING_POINTS,
            ACTION_PROBLEM_RESET,
            ACTION_SOLVER_START,
            ACTION_SOLVER_STOP,
            ACTION_SOLVER_KILL,
            ACTION_REPORT_MODEL_STATISTICS,
            ACTION_REPORT_SOLVER_LOG,
            ACTION_REPORT_CONVERGENCE_GRAPH,
            ACTION_REPORT_MONITORING_POINT_GRAPH,
            ACTION_REPORT_CREATE_REPORT,
            ACTION_APPLICATION_SETTINGS,
            ACTION_APPLICATION_UPDATE,
            ACTION_APPLICATION_ABOUT,
            ACTION_APPLICATION_ABOUT_QT,
            ACTION_APPLICATION_LICENSE,
            ACTION_APPLICATION_RELEASE_NOTES,
            ACTION_APPLICATION_HELP,
            ACTION_APPLICATION_QUIT,
            ACTION_CLOUD_SESSION_MANAGER,
            ACTION_CLOUD_FILE_MANAGER,
            ACTION_N_TYPES
        };

    private:

        //! Constructor.
        explicit Action(Definition definition, QObject *parent = nullptr);

    public:

        //! Return action group name.
        static QString getGroupName(GroupType groupType);

        //! Return action name.
        static QString getName(Type type);

        //! Return list of group types.
        static QList<GroupType> getGroupTypes(GroupTypeMask groupTypeMask = ACTION_GROUP_ALL);

        //! Generate list action descriptions.
        static QList<RAction::Definition> generateActionDefinitionList();

    private:

        static void regDef(QList<RAction::Definition> &actionDefinition,
                           GroupType groupType,
                           Type type,
                           const QString &text,
                           const QString &description,
                           const QString &shortCut,
                           const QString &icon,
                           PointerToMemberTrigger triggerSlot,
                           QAction::MenuRole menuRole = QAction::NoRole);

        static void regDef(QList<RAction::Definition> &actionDefinition,
                           GroupType groupType,
                           Type type,
                           const QString &text,
                           const QString &description,
                           const QString &shortCut,
                           const QString &icon,
                           PointerToMemberToggle toggleSlot,
                           QAction::MenuRole menuRole = QAction::NoRole);

    public slots:

        //! New session.
        void onSessionNew();

        //! Open session.
        void onSessionOpen();

        //! Save session.
        void onSessionSave();

        //! Save session as.
        void onSessionSaveAs();

        //! Close session.
        void onSessionClose();

        //! New model.
        void onModelNew();

        //! Open model.
        void onModelOpen();

        //! Save model.
        void onModelSave();

        //! Save model as.
        void onModelSaveAs();

        //! Export MSH model.
        void onModelExportMsh();

        //! Export RAW model.
        void onModelExportRaw();

        //! Export STL model (ascii).
        void onModelExportStlAscii();

        //! Export STL model (binary).
        void onModelExportStlBinary();

        //! Close model.
        void onModelClose();

        //! Reload results.
        void onModelReloadResults();

        //! Rename model.
        void onModelRename();

        //! Drop results.
        void onModelDropResults();

        //! Geometry undo.
        void onGeometryUndo();

        //! Geometry redo.
        void onGeometryRedo();

        //! Drav point.
        void onGeometryDrawPoint();

        //! Drav line.
        void onGeometryDrawLine();

        //! Drav triangle.
        void onGeometryDrawTriangle();

        //! Drav quadrilateral.
        void onGeometryDrawQuadrilateral();

        //! Drav circle.
        void onGeometryDrawCircle();

        //! Drav ellipse.
        void onGeometryDrawEllipse();

        //! Drav tetrahedron.
        void onGeometryDrawTetrahedron();

        //! Drav hexahedron.
        void onGeometryDrawHexahedron();

        //! Drav cylinder.
        void onGeometryDrawCylinder();

        //! Drav sphere.
        void onGeometryDrawSphere();

        //! Drav RAW object.
        void onGeometryDrawRaw();

        //! Create element.
        void onGeometryCreateElement();

        //! Find sliver elements.
        void onGeometryFindSliverElements();

        //! Fix sliver elements.
        void onGeometryFixSliverElements();

        //! Find intersected elements.
        void onGeometryFindIntersectedElements();

        //! Break intersected elements.
        void onGeometryBreakIntersectedElements();

        //! Create union.
        void onGeometryBoolUnion();

        //! Create difference.
        void onGeometryBoolDifference();

        //! Create intersection.
        void onGeometryBoolIntersection();

        //! Merge near nodes.
        void onGeometryMergeNearNodes();

        //! Move node.
        void onGeometryMoveNode();

        //! Remove node.
        void onGeometryRemoveNode();

        //! Remove node and close hole.
        void onGeometryRemoveNodeAndCloseHole();

        //! Remove element.
        void onGeometryRemoveElement();

        //! Generate line from edges.
        void onGeometryLineGenerateFromEdges();

        //! Mark surfaces.
        void onGeometrySurfaceMark();

        //! Swap surface element normal.
        void onGeometrySurfaceSwapElementNormal();

        //! Swap surface normals.
        void onGeometrySurfaceSwapNormals();

        //! Synchronize surface normals.
        void onGeometrySurfaceSyncNormals();

        //! Close hole in surface.
        void onGeometrySurfaceCloseHole();

        //! Coarsen surface elements.
        void onGeometrySurfaceCoarsen();

        //! Create vector field.
        void onGeometryVectorFieldCreate();

        //! Edit vector field.
        void onGeometryVectorFieldEdit();

        //! Create scalar field.
        void onGeometryScalarFieldCreate();

        //! Edit scalar field.
        void onGeometryScalarFieldEdit();

        //! Create stream line.
        void onGeometryStreamLineCreate();

        //! Edit stream line.
        void onGeometryStreamLineEdit();

        //! Create cut.
        void onGeometryCutCreate();

        //! Edit cut.
        void onGeometryCutEdit();

        //! Create iso.
        void onGeometryIsoCreate();

        //! Edit iso.
        void onGeometryIsoEdit();

        //! Merge entities.
        void onGeometryEntityMerge();

        //! Remove entity.
        void onGeometryEntityRemove();

        //! Generate mesh.
        void onGeometryGenerateTetrahedra ();

        //! Transform geometry.
        void onGeometryTransform();

        //! Export sliver elements.
        void onGeometryDevExportSliverElements();

        //! Export intersected elements.
        void onGeometryDevExportIntersectedElements();

        //! Purge unused nodes.
        void onGeometryDevPurgeUnusedNodes();

        //! Purge unused elements.
        void onGeometryDevPurgeUnusedElements();

        //! Merge duplicate nodes.
        void onGeometryDevRemoveDuplicateNodes();

        //! Merge duplicate elements.
        void onGeometryDevRemoveDuplicateElements();

        //! Check if point is inside/behind surface.
        void onGeometryDevPointInsideSurface();

        //! Tetrahedralize surface.
        void onGeometryDevTetrahedralizeeSurface();

        //! Consolidate.
        void onGeometryDevConsolidate();

        //! Problem(s) task flow.
        void onProblemTaskFlow();

        //! Setup problem matrix solver solver.
        void onProblemSolverSetup();

        //! Define monitoring points.
        void onProblemDefineMonitoringPoints();

        //! Reset problem.
        void onProblemReset();

        //! Start solver.
        void onSolverStart();

        //! Stop solver.
        void onSolverStop();

        //! Kill solver.
        void onSolverKill();

        //! Produce model statistics.
        void onReportModelStatistics();

        //! Show solver log file.
        void onReportSolverLog();

        //! Convergence graph.
        void onReportConvergenceGraph();

        //! Monitoring point graph.
        void onReportMonitoringPointGraph();

        //! Create report.
        void onReportCreateReport();

        //! Help center.
        void onHelp();

        //! Quit program.
        void onQuit();

        //! Application settings.
        void onApplicationSettings();

        //! Check for update.
        void onUpdate();

        //! About.
        void onAbout();

        //! About Qt.
        void onAboutQt();

        //! License.
        void onLicense();

        //! Release notes.
        void onReleaseNotes();

        //! Open cloud session manager.
        void onCloudSessionManager();

        //! Cloud file manager.
        void onCloudFileManager();

};

#endif // ACTION_H
