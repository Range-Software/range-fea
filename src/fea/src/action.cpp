#include <QString>
#include <QFileDialog>
#include <QMessageBox>

#include <rbl_error.h>
#include <rbl_logger.h>
#include <rbl_utils.h>

#include <rml_file_manager.h>

#include <rgl_application_settings_dialog.h>
#include <rgl_cloud_file_manager_dialog.h>
#include <rgl_cloud_session_dialog.h>
#include <rgl_log_browser_dialog.h>
#include <rgl_message_box.h>
#include <rgl_software_manager_dialog.h>
#include <rgl_text_browser_dialog.h>

#include "action.h"

#include "application.h"
#include "application_settings_dialog.h"
#include "bool_difference_dialog.h"
#include "bool_intersection_dialog.h"
#include "bool_union_dialog.h"
#include "break_intersected_elements_dialog.h"
#include "convergence_graph_dialog.h"
#include "drop_results_dialog.h"
#include "new_model_dialog.h"
#include "coarsen_surface_dialog.h"
#include "cut_dialog.h"
#include "iso_dialog.h"
#include "find_sliver_elements_dialog.h"
#include "fix_sliver_elements_dialog.h"
#include "help_dialog.h"
#include "mark_entity_dialog.h"
#include "merge_entity_dialog.h"
#include "merge_near_nodes_dialog.h"
#include "remove_entity_dialog.h"
#include "main_window.h"
#include "matrix_solver_config_dialog.h"
#include "mesh_generator_dialog.h"
#include "model_action.h"
#include "model_io.h"
#include "model_statistics_dialog.h"
#include "monitoring_points_dialog.h"
#include "monitoring_point_graph_dialog.h"
#include "move_node_dialog.h"
#include "point_inside_surface_dialog.h"
#include "problem_task_dialog.h"
#include "rename_model_dialog.h"
#include "report_dialog.h"
#include "session_entity_id.h"
#include "stream_line_dialog.h"
#include "solver_start_dialog.h"
#include "solver_manager.h"
#include "scalar_field_dialog.h"
#include "vector_field_dialog.h"

Action::Action(RAction::Definition definition, QObject *parent)
    : RAction{definition,parent}
{

    if (this->name == Action::getName(ACTION_SOLVER_START))
    {
        QObject::connect(&SolverManager::getInstance(), &SolverManager::jobStarted, this, &Action::disable);
        QObject::connect(&SolverManager::getInstance(), &SolverManager::jobFinished, this, &Action::enable);
    }
    if (this->name == Action::getName(ACTION_SOLVER_STOP) || this->name == Action::getName(ACTION_SOLVER_KILL))
    {
        QObject::connect(&SolverManager::getInstance(), &SolverManager::jobStarted, this, &Action::enable);
        QObject::connect(&SolverManager::getInstance(), &SolverManager::jobFinished, this, &Action::disable);
    }
}

QString Action::getGroupName(GroupType groupType)
{
    switch (groupType)
    {
        case ACTION_GROUP_SESSION:
            return tr("Session");
        case ACTION_GROUP_MODEL:
            return tr("Model");
        case ACTION_GROUP_MATERIAL:
            return tr("Material");
        case ACTION_GROUP_GEOMETRY:
            return tr("Geometry");
        case ACTION_GROUP_PROBLEM:
            return tr("Problem");
        case ACTION_GROUP_SOLVER:
            return tr("Solver");
        case ACTION_GROUP_REPORT:
            return tr("Report");
        case ACTION_GROUP_APPLICATION:
            return tr("Application");
        default:
            return QString();
    }
}

QString Action::getName(Type type)
{
    switch (type)
    {
        case ACTION_SESSION_NEW:                              return "session-new";
        case ACTION_SESSION_OPEN:                             return "session-open";
        case ACTION_SESSION_SAVE:                             return "session-save";
        case ACTION_SESSION_SAVE_AS:                          return "session-save_as";
        case ACTION_SESSION_CLOSE:                            return "session-close";
        case ACTION_MODEL_NEW:                                return "model-new";
        case ACTION_MODEL_OPEN:                               return "model-open";
        case ACTION_MODEL_SAVE:                               return "model-save";
        case ACTION_MODEL_SAVE_AS:                            return "model-save_as";
        case ACTION_MODEL_EXPORT_MSH:                         return "model-export_msh";
        case ACTION_MODEL_EXPORT_RAW:                         return "model-export_raw";
        case ACTION_MODEL_EXPORT_STL_ASCII:                   return "model-export_stl_ascii";
        case ACTION_MODEL_EXPORT_STL_BINARY:                  return "model-export_stl_binary";
        case ACTION_MODEL_CLOSE:                              return "model-close";
        case ACTION_MODEL_RELOAD_RESULTS:                     return "model-reload_results";
        case ACTION_MODEL_DROP_RESULTS:                       return "model-drop_results";
        case ACTION_MODEL_RENAME:                             return "model-rename";
        case ACTION_GEOMETRY_UNDO:                            return "geometry-undo";
        case ACTION_GEOMETRY_REDO:                            return "geometry-redo";
        case ACTION_GEOMETRY_DRAW_POINT:                      return "geometry-draw_point";
        case ACTION_GEOMETRY_DRAW_LINE:                       return "geometry-draw_line";
        case ACTION_GEOMETRY_DRAW_TRIANGLE:                   return "geometry-draw_triangle";
        case ACTION_GEOMETRY_DRAW_QUADRILATERAL:              return "geometry-draw_quadrilateral";
        case ACTION_GEOMETRY_DRAW_CIRCLE:                     return "geometry-draw_circle";
        case ACTION_GEOMETRY_DRAW_ELLIPSE:                    return "geometry-draw_ellipse";
        case ACTION_GEOMETRY_DRAW_TETRAHEDRON:                return "geometry-draw_tetrahedron";
        case ACTION_GEOMETRY_DRAW_HEXAHEDRON:                 return "geometry-draw_hexahedron";
        case ACTION_GEOMETRY_DRAW_CYLINDER:                   return "geometry-draw_cylinder";
        case ACTION_GEOMETRY_DRAW_SPHERE:                     return "geometry-draw_sphere";
        case ACTION_GEOMETRY_DRAW_RAW:                        return "geometry-draw_raw";
        case ACTION_GEOMETRY_CREATE_ELEMENT:                  return "geometry-create_element";
        case ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS:            return "geometry-find_sliver_elements";
        case ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS:             return "geometry-fix_sliver_elements";
        case ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS:       return "geometry-find_intersected_elements";
        case ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS:      return "geometry-break_intersected_elements";
        case ACTION_GEOMETRY_BOOL_UNION:                      return "geometry-bool_union";
        case ACTION_GEOMETRY_BOOL_DIFFERENCE:                 return "geometry-bool_difference";
        case ACTION_GEOMETRY_BOOL_INTERSECTION:               return "geometry-bool_intersection";
        case ACTION_GEOMETRY_MERGE_NEAR_NODES:                return "geometry-merge_near_nodes";
        case ACTION_GEOMETRY_MOVE_NODE:                       return "geometry-move_node";
        case ACTION_GEOMETRY_REMOVE_NODE:                     return "geometry-remove_node";
        case ACTION_GEOMETRY_REMOVE_NODE_AND_CLOSE_HOLE:      return "geometry-remove_node_and_close_hole";
        case ACTION_GEOMETRY_REMOVE_ELEMENT:                  return "geometry-remove_element";
        case ACTION_GEOMETRY_LINE_GENERATE_FROM_EDGES:        return "geometry-line_generate_from_edges";
        case ACTION_GEOMETRY_SURFACE_MARK:                    return "geometry-surface_mark";
        case ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL:     return "geometry-surface_swap_element_normal";
        case ACTION_GEOMETRY_SURFACE_SWAP_NORMALS:            return "geometry-surface_swap_normals";
        case ACTION_GEOMETRY_SURFACE_SYNC_NORMALS:            return "geometry-surface_sync_normals";
        case ACTION_GEOMETRY_SURFACE_CLOSE_HOLE:              return "geometry-surface_close_hole";
        case ACTION_GEOMETRY_SURFACE_COARSEN:                 return "geometry-surface_coarsen";
        case ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA:      return "geometry-volume_generate_tetrahedra";
        case ACTION_GEOMETRY_VECTOR_FIELD_CREATE:             return "geometry-vector_field_create";
        case ACTION_GEOMETRY_VECTOR_FIELD_EDIT:               return "geometry-vector_field_edit";
        case ACTION_GEOMETRY_SCALAR_FIELD_CREATE:             return "geometry-scalar_field_create";
        case ACTION_GEOMETRY_SCALAR_FIELD_EDIT:               return "geometry-scalar_field_edit";
        case ACTION_GEOMETRY_STREAM_LINE_CREATE:              return "geometry-stream_line_create";
        case ACTION_GEOMETRY_STREAM_LINE_EDIT:                return "geometry-stream_line_edit";
        case ACTION_GEOMETRY_CUT_CREATE:                      return "geometry-cut_create";
        case ACTION_GEOMETRY_CUT_EDIT:                        return "geometry-cut_edit";
        case ACTION_GEOMETRY_ISO_CREATE:                      return "geometry-iso_create";
        case ACTION_GEOMETRY_ISO_EDIT:                        return "geometry-iso_edit";
        case ACTION_GEOMETRY_ENTITY_MERGE:                    return "geometry-entity_merge";
        case ACTION_GEOMETRY_ENTITY_REMOVE:                   return "geometry-entity_remove";
        case ACTION_GEOMETRY_TRANSFORM:                       return "geometry-transform";
        case ACTION_GEOMETRY_DEV_EXPORT_SLIVER_ELEMENTS:      return "geometry-dev_export_sliver_elements";
        case ACTION_GEOMETRY_DEV_EXPORT_INTERSECTED_ELEMENTS: return "geometry-dev_export_intersected_elements";
        case ACTION_GEOMETRY_DEV_PURGE_UNUSED_NODES:          return "geometry-dev_purge_unused_nodes";
        case ACTION_GEOMETRY_DEV_PURGE_UNUSED_ELEMENTS:       return "geometry-dev_purge_unused_elements";
        case ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_NODES:      return "geometry-dev_remove_duplicate_nodes";
        case ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_ELEMENTS:   return "geometry-dev_remove_duplicate_elements";
        case ACTION_GEOMETRY_DEV_POINT_INSIDE_SURFACE:        return "geometry-dev_point_inside_surface";
        case ACTION_GEOMETRY_DEV_TETRAHEDRALIZE_SURFACE:      return "geometry-dev_tetrahedralize_surface";
        case ACTION_GEOMETRY_DEV_CONSOLIDATE:                 return "geometry-dev_consolidate";
        case ACTION_PROBLEM_TASK_FLOW:                        return "problem-task_flow";
        case ACTION_PROBLEM_SOLVER_SETUP:                     return "problem-solver_setup";
        case ACTION_PROBLEM_DEFINE_MONITORING_POINTS:         return "problem-define_monitoring_points";
        case ACTION_PROBLEM_RESET:                            return "problem-reset";
        case ACTION_SOLVER_START:                             return "solver-start";
        case ACTION_SOLVER_STOP:                              return "solver-stop";
        case ACTION_SOLVER_KILL:                              return "solver-kill";
        case ACTION_REPORT_MODEL_STATISTICS:                  return "report-model_statistics";
        case ACTION_REPORT_SOLVER_LOG:                        return "report-solver_log";
        case ACTION_REPORT_CONVERGENCE_GRAPH:                 return "report-convergence_graph";
        case ACTION_REPORT_MONITORING_POINT_GRAPH:            return "report-monitoring_point_graph";
        case ACTION_REPORT_CREATE_REPORT:                     return "report-create_report";
        case ACTION_APPLICATION_SETTINGS:                     return "application-settings";
        case ACTION_APPLICATION_UPDATE:                       return "application-update";
        case ACTION_APPLICATION_ABOUT:                        return "application-about";
        case ACTION_APPLICATION_ABOUT_QT:                     return "application-about_qt";
        case ACTION_APPLICATION_LICENSE:                      return "application-license";
        case ACTION_APPLICATION_RELEASE_NOTES:                return "application-release_notes";
        case ACTION_APPLICATION_HELP:                         return "application-help";
        case ACTION_APPLICATION_QUIT:                         return "application-quit";
        case ACTION_CLOUD_SESSION_MANAGER:                    return "cloud-sessiion_manager";
        case ACTION_CLOUD_FILE_MANAGER:                       return "cloud-file_manager";
        default: return QString();
    }
}

QList<Action::GroupType> Action::getGroupTypes(GroupTypeMask groupTypeMask)
{
    QList<Action::GroupType> groupTypes;

    if (groupTypeMask & ACTION_GROUP_APPLICATION) groupTypes.append(ACTION_GROUP_APPLICATION);
    if (groupTypeMask & ACTION_GROUP_SESSION) groupTypes.append(ACTION_GROUP_SESSION);
    if (groupTypeMask & ACTION_GROUP_MODEL) groupTypes.append(ACTION_GROUP_MODEL);
    if (groupTypeMask & ACTION_GROUP_MATERIAL) groupTypes.append(ACTION_GROUP_MATERIAL);
    if (groupTypeMask & ACTION_GROUP_GEOMETRY) groupTypes.append(ACTION_GROUP_GEOMETRY);
    if (groupTypeMask & ACTION_GROUP_PROBLEM) groupTypes.append(ACTION_GROUP_PROBLEM);
    if (groupTypeMask & ACTION_GROUP_SOLVER) groupTypes.append(ACTION_GROUP_SOLVER);
    if (groupTypeMask & ACTION_GROUP_REPORT) groupTypes.append(ACTION_GROUP_REPORT);

    return groupTypes;
}

QList<RAction::Definition> Action::generateActionDefinitionList()
{
    QList<RAction::Definition> actionDef;

    actionDef.resize(ACTION_N_TYPES);

    actionDef[ACTION_SEPARATOR] = RAction::definition();
    Action::regDef(actionDef, ACTION_GROUP_SESSION, ACTION_SESSION_NEW, tr("New session"), tr("Create a new session."), "Ctrl+Alt+N", ":/icons/file/pixmaps/range-session_new.svg",static_cast<PointerToMemberTrigger>(&Action::onSessionNew));
    Action::regDef(actionDef, ACTION_GROUP_SESSION, ACTION_SESSION_OPEN, tr("Open session"), tr("Open previously saved session."), "Ctrl+Alt+O", ":/icons/file/pixmaps/range-session_open.svg",static_cast<PointerToMemberTrigger>(&Action::onSessionOpen));
    Action::regDef(actionDef, ACTION_GROUP_SESSION, ACTION_SESSION_SAVE, tr("Save session"), tr("Save current session."), "Ctrl+Alt+S", ":/icons/file/pixmaps/range-session_save.svg",static_cast<PointerToMemberTrigger>(&Action::onSessionSave));
    Action::regDef(actionDef, ACTION_GROUP_SESSION, ACTION_SESSION_SAVE_AS, tr("Save session as"), tr("Save current session under different filename."), "Ctrl+Alt+Shift+S", ":/icons/file/pixmaps/range-session_save_as.svg",static_cast<PointerToMemberTrigger>(&Action::onSessionSaveAs));
    Action::regDef(actionDef, ACTION_GROUP_SESSION, ACTION_SESSION_CLOSE, tr("Close Session"), tr("Close current session."), "Ctrl+Alt+W", ":/icons/file/pixmaps/range-session_close.svg",static_cast<PointerToMemberTrigger>(&Action::onSessionClose));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_NEW, tr("New model"), tr("Create a new empty model."), "Ctrl+N", ":/icons/file/pixmaps/range-model_new.svg",static_cast<PointerToMemberTrigger>(&Action::onModelNew));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_OPEN, tr("Open model"), tr("Open previously saved model."), "Ctrl+O", ":/icons/file/pixmaps/range-model_open.svg",static_cast<PointerToMemberTrigger>(&Action::onModelOpen));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_SAVE, tr("Save model"), tr("Save selected model."), "Ctrl+S", ":/icons/file/pixmaps/range-model_save.svg",static_cast<PointerToMemberTrigger>(&Action::onModelSave));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_SAVE_AS, tr("Save model as"), tr("Save selected model under a different filename."), "Ctrl+Shift+S", ":/icons/file/pixmaps/range-model_save_as.svg",static_cast<PointerToMemberTrigger>(&Action::onModelSaveAs));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_EXPORT_MSH, tr("Export MSH model"), "", "", ":/icons/file/pixmaps/range-model_export_msh.svg",static_cast<PointerToMemberTrigger>(&Action::onModelExportMsh));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_EXPORT_RAW, tr("Export RAW model"), "", "", ":/icons/file/pixmaps/range-model_export_raw.svg",static_cast<PointerToMemberTrigger>(&Action::onModelExportRaw));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_EXPORT_STL_ASCII, tr("Export STL model (ascii)"), "", "", ":/icons/file/pixmaps/range-model_export_stl.svg",static_cast<PointerToMemberTrigger>(&Action::onModelExportStlAscii));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_EXPORT_STL_BINARY, tr("Export STL model (binary)"), "", "", ":/icons/file/pixmaps/range-model_export_stl.svg",static_cast<PointerToMemberTrigger>(&Action::onModelExportStlBinary));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_CLOSE, tr("Close model"), "", "Ctrl+W", ":/icons/file/pixmaps/range-model_close.svg",static_cast<PointerToMemberTrigger>(&Action::onModelClose));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_RELOAD_RESULTS, tr("Reload results"), "", "", ":/icons/file/pixmaps/range-refresh.svg",static_cast<PointerToMemberTrigger>(&Action::onModelReloadResults));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_DROP_RESULTS, tr("Drop results"), "", "", ":/icons/file/pixmaps/range-model_drop_results.svg",static_cast<PointerToMemberTrigger>(&Action::onModelDropResults));
    Action::regDef(actionDef, ACTION_GROUP_MODEL, ACTION_MODEL_RENAME, tr("Rename model"), "", "", ":/icons/file/pixmaps/range-model_rename.svg",static_cast<PointerToMemberTrigger>(&Action::onModelRename));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_UNDO, tr("Undo"), "", "Ctrl+Z", ":/icons/geometry/pixmaps/range-undo.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryUndo));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_REDO, tr("Redo"), "", "Ctrl+Shift+Z", ":/icons/geometry/pixmaps/range-redo.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryRedo));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_POINT, tr("Draw point"), "", "", ":/icons/geometry/pixmaps/range-draw_point.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawPoint));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_LINE, tr("Draw line"), "", "", ":/icons/geometry/pixmaps/range-draw_line.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawLine));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_TRIANGLE, tr("Draw triangle"), "", "", ":/icons/geometry/pixmaps/range-draw_triangle.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawTriangle));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_QUADRILATERAL, tr("Draw quadrilateral"), "", "", ":/icons/geometry/pixmaps/range-draw_quadrilateral.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawQuadrilateral));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_CIRCLE, tr("Draw circle"), "", "", ":/icons/geometry/pixmaps/range-draw_circle.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawCircle));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_ELLIPSE, tr("Draw ellipse"), "", "", ":/icons/geometry/pixmaps/range-draw_ellipse.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawEllipse));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_TETRAHEDRON, tr("Draw tetrahedron"), "", "", ":/icons/geometry/pixmaps/range-draw_tetrahedron.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawTetrahedron));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_HEXAHEDRON, tr("Draw hexahedron"), "", "", ":/icons/geometry/pixmaps/range-draw_hexahedron.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawHexahedron));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_CYLINDER, tr("Draw cylinder"), "", "", ":/icons/geometry/pixmaps/range-draw_cylinder.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawCylinder));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_SPHERE, tr("Draw sphere"), "", "", ":/icons/geometry/pixmaps/range-draw_sphere.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawSphere));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DRAW_RAW, tr("Draw RAW object"), "", "", ":/icons/geometry/pixmaps/range-draw_raw.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryDrawRaw));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_CREATE_ELEMENT, tr("Create element"), "", "Ctrl+Alt+E", ":/icons/geometry/pixmaps/range-element_create.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryCreateElement));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_FIND_SLIVER_ELEMENTS, tr("Find sliver elements"), "", "", ":/icons/geometry/pixmaps/range-sliver_find.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryFindSliverElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_FIX_SLIVER_ELEMENTS, tr("Fix sliver elements"), "", "", ":/icons/geometry/pixmaps/range-sliver_fix.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryFixSliverElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_FIND_INTERSECTED_ELEMENTS, tr("Find intersected elements"), "", "Ctrl+I", ":/icons/geometry/pixmaps/range-entity_surface_find_intersected.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryFindIntersectedElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_BREAK_INTERSECTED_ELEMENTS, tr("Break intersected elements"), "", "Ctrl+J", ":/icons/geometry/pixmaps/range-entity_surface_break_intersected.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryBreakIntersectedElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_BOOL_UNION, tr("Union"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_bool_union.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryBoolUnion));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_BOOL_DIFFERENCE, tr("Difference"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_bool_difference.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryBoolDifference));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_BOOL_INTERSECTION, tr("Intersection"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_bool_intersection.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryBoolIntersection));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_MERGE_NEAR_NODES, tr("Merge near nodes"), "", "", ":/icons/geometry/pixmaps/range-merge_near_nodes.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryMergeNearNodes));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_MOVE_NODE, tr("Move node"), "", "", ":/icons/geometry/pixmaps/range-move_node.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryMoveNode));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_REMOVE_NODE, tr("Remove node"), "", "Ctrl+Shift+X", ":/icons/geometry/pixmaps/range-node_remove.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryRemoveNode));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_REMOVE_NODE_AND_CLOSE_HOLE, tr("Remove node and close hole"), "", "", ":/icons/geometry/pixmaps/range-node_remove_and_close.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryRemoveNodeAndCloseHole));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_REMOVE_ELEMENT, tr("Remove element"), "", "Ctrl+Alt+X", ":/icons/geometry/pixmaps/range-element_remove.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryRemoveElement));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_LINE_GENERATE_FROM_EDGES, tr("Generate line(s) from surface edges"), "", "", ":/icons/geometry/pixmaps/range-entity_line_from_surface.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryLineGenerateFromEdges));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SURFACE_MARK, tr("Mark surface"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_mark.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometrySurfaceMark));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SURFACE_SWAP_ELEMENT_NORMAL, tr("Swap element normal"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_swap_element_normal.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometrySurfaceSwapElementNormal));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SURFACE_SWAP_NORMALS, tr("Swap normals"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_swap_normals.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometrySurfaceSwapNormals));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SURFACE_SYNC_NORMALS, tr("Synchronize normals"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_sync_normals.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometrySurfaceSyncNormals));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SURFACE_CLOSE_HOLE, tr("Close hole"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_close_hole.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometrySurfaceCloseHole));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SURFACE_COARSEN, tr("Coarsen surface"), "", "", ":/icons/geometry/pixmaps/range-entity_surface_coarsen.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometrySurfaceCoarsen));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_VOLUME_GENERATE_TETRAHEDRA, tr("Generate tetrahedral mesh"), "", "Ctrl+G", ":/icons/geometry/pixmaps/range-generate_volume_mesh.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryGenerateTetrahedra));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_VECTOR_FIELD_CREATE, tr("Create vector field"), "", "", ":/icons/geometry/pixmaps/range-entity_vector_field_create.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryVectorFieldCreate));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_VECTOR_FIELD_EDIT, tr("Edit vector field"), "", "", ":/icons/geometry/pixmaps/range-entity_vector_field_edit.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryVectorFieldEdit));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SCALAR_FIELD_CREATE, tr("Create scalar field"), "", "", ":/icons/geometry/pixmaps/range-entity_scalar_field_create.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryScalarFieldCreate));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_SCALAR_FIELD_EDIT, tr("Edit scalar field"), "", "", ":/icons/geometry/pixmaps/range-entity_scalar_field_edit.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryScalarFieldEdit));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_STREAM_LINE_CREATE, tr("Create stream line"), "", "", ":/icons/geometry/pixmaps/range-entity_stream_line_create.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryStreamLineCreate));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_STREAM_LINE_EDIT, tr("Edit stream line"), "", "", ":/icons/geometry/pixmaps/range-entity_stream_line_edit.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryStreamLineEdit));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_CUT_CREATE, tr("Create cut"), "", "", ":/icons/geometry/pixmaps/range-entity_cut_create.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryCutCreate));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_CUT_EDIT, tr("Edit cut"), "", "", ":/icons/geometry/pixmaps/range-entity_cut_edit.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryCutEdit));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_ISO_CREATE, tr("Create iso"), "", "", ":/icons/geometry/pixmaps/range-entity_iso_create.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryIsoCreate));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_ISO_EDIT, tr("Edit iso"), "", "", ":/icons/geometry/pixmaps/range-entity_iso_edit.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryIsoEdit));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_ENTITY_MERGE, tr("Merge selected entities"), "", "Ctrl+M", ":/icons/geometry/pixmaps/range-entity_merge.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryEntityMerge));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_ENTITY_REMOVE, tr("Remove selected entities"), "", "Ctrl+X", ":/icons/geometry/pixmaps/range-entity_remove.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryEntityRemove));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_TRANSFORM, tr("Scale, translate, rotate"), "", "Ctrl+T", ":/icons/geometry/pixmaps/range-geometry_transform.svg",static_cast<PointerToMemberTrigger>(&Action::onGeometryTransform));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_EXPORT_SLIVER_ELEMENTS, tr("Export sliver elements"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevExportSliverElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_EXPORT_INTERSECTED_ELEMENTS, tr("Export intersected elements"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevExportIntersectedElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_PURGE_UNUSED_NODES, tr("Purge unused nodes"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevPurgeUnusedNodes));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_PURGE_UNUSED_ELEMENTS, tr("Purge unused elements"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevPurgeUnusedElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_NODES, tr("Remove duplicate nodes"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevRemoveDuplicateNodes));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_REMOVE_DUPLICATE_ELEMENTS, tr("Remove duplicate elements"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevRemoveDuplicateElements));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_POINT_INSIDE_SURFACE, tr("Check if point is inside surface"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevPointInsideSurface));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_TETRAHEDRALIZE_SURFACE, tr("Tetrahedralize surface"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevTetrahedralizeeSurface));
    Action::regDef(actionDef, ACTION_GROUP_GEOMETRY, ACTION_GEOMETRY_DEV_CONSOLIDATE, tr("Consolidate geometry"), "", "", "",static_cast<PointerToMemberTrigger>(&Action::onGeometryDevConsolidate));
    Action::regDef(actionDef, ACTION_GROUP_PROBLEM, ACTION_PROBLEM_TASK_FLOW, tr("Problem(s) task flow"), "", "Ctrl+P", ":/icons/problem/pixmaps/range-problem_task_flow.svg",static_cast<PointerToMemberTrigger>(&Action::onProblemTaskFlow));
    Action::regDef(actionDef, ACTION_GROUP_PROBLEM, ACTION_PROBLEM_SOLVER_SETUP, tr("Setup Problem(s) matrix solver"), "", "", ":/icons/problem/pixmaps/range-problem_matrix_solver.svg",static_cast<PointerToMemberTrigger>(&Action::onProblemSolverSetup));
    Action::regDef(actionDef, ACTION_GROUP_PROBLEM, ACTION_PROBLEM_DEFINE_MONITORING_POINTS, tr("Define monitoring points"), "", "", ":/icons/problem/pixmaps/range-problem_monitoring_points.svg",static_cast<PointerToMemberTrigger>(&Action::onProblemDefineMonitoringPoints));
    Action::regDef(actionDef, ACTION_GROUP_PROBLEM, ACTION_PROBLEM_RESET, tr("Reset setup"), "", "", ":/icons/problem/pixmaps/range-problem_reset.svg",static_cast<PointerToMemberTrigger>(&Action::onProblemReset));
    Action::regDef(actionDef, ACTION_GROUP_SOLVER, ACTION_SOLVER_START, tr("Start solver"), "", "Ctrl+R", ":/icons/solver/pixmaps/range-solver-start.svg",static_cast<PointerToMemberTrigger>(&Action::onSolverStart));
    Action::regDef(actionDef, ACTION_GROUP_SOLVER, ACTION_SOLVER_STOP, tr("Stop solver"), "", "Ctrl+E", ":/icons/solver/pixmaps/range-solver-stop.svg",static_cast<PointerToMemberTrigger>(&Action::onSolverStop));
    Action::regDef(actionDef, ACTION_GROUP_SOLVER, ACTION_SOLVER_KILL, tr("Kill solver"), "", "Ctrl+K", ":/icons/solver/pixmaps/range-solver-kill.svg",static_cast<PointerToMemberTrigger>(&Action::onSolverKill));
    Action::regDef(actionDef, ACTION_GROUP_REPORT, ACTION_REPORT_MODEL_STATISTICS, tr("Model statistics"), "", "", ":/icons/report/pixmaps/range-report_model_statistics.svg",static_cast<PointerToMemberTrigger>(&Action::onReportModelStatistics));
    Action::regDef(actionDef, ACTION_GROUP_REPORT, ACTION_REPORT_SOLVER_LOG, tr("Solver log file"), "", "", ":/icons/report/pixmaps/range-report_solver_log.svg",static_cast<PointerToMemberTrigger>(&Action::onReportSolverLog));
    Action::regDef(actionDef, ACTION_GROUP_REPORT, ACTION_REPORT_CONVERGENCE_GRAPH, tr("Solver convergence"), "", "Ctrl+Alt+C", ":/icons/report/pixmaps/range-report-convergence.svg",static_cast<PointerToMemberTrigger>(&Action::onReportConvergenceGraph));
    Action::regDef(actionDef, ACTION_GROUP_REPORT, ACTION_REPORT_MONITORING_POINT_GRAPH, tr("Monitoring points"), "", "", ":/icons/report/pixmaps/range-report_monitoring_point.svg",static_cast<PointerToMemberTrigger>(&Action::onReportMonitoringPointGraph));
    Action::regDef(actionDef, ACTION_GROUP_REPORT, ACTION_REPORT_CREATE_REPORT, tr("Create report"), "", "Ctrl+Shift+R", ":/icons/report/pixmaps/range-report.svg",static_cast<PointerToMemberTrigger>(&Action::onReportCreateReport));
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_SETTINGS, tr("Application settings"), "", "Ctrl+A", ":/icons/application/pixmaps/range-application_settings.svg",static_cast<PointerToMemberTrigger>(&Action::onApplicationSettings), QAction::PreferencesRole);
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_UPDATE, tr("Check for update"), "", "", ":/icons/file/pixmaps/range-download.svg",static_cast<PointerToMemberTrigger>(&Action::onUpdate));
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_ABOUT, tr("About"), "", "F1", ":/icons/logos/pixmaps/range-fea.svg",static_cast<PointerToMemberTrigger>(&Action::onAbout), QAction::AboutRole);
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_ABOUT_QT, tr("About Qt"), "", "", ":/icons/file/pixmaps/range-qt.svg",static_cast<PointerToMemberTrigger>(&Action::onAboutQt), QAction::AboutQtRole);
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_LICENSE, tr("License"), "", "", ":/icons/logos/pixmaps/range-fea.svg",static_cast<PointerToMemberTrigger>(&Action::onLicense));
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_RELEASE_NOTES, tr("Release notes"), "", "", ":/icons/logos/pixmaps/range-fea.svg",static_cast<PointerToMemberTrigger>(&Action::onReleaseNotes));
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_HELP, tr("Help"), "", "Ctrl+H", ":/icons/file/pixmaps/range-help.svg",static_cast<PointerToMemberTrigger>(&Action::onHelp));
    Action::regDef(actionDef, ACTION_GROUP_APPLICATION, ACTION_APPLICATION_QUIT, tr("Quit"), "", "Q", ":/icons/application/pixmaps/range-quit.svg",static_cast<PointerToMemberTrigger>(&Action::onQuit), QAction::QuitRole);
    Action::regDef(actionDef, ACTION_GROUP_CLOUD, ACTION_CLOUD_SESSION_MANAGER, tr("Cloud session manager"), "", "", ":/icons/cloud/pixmaps/range-session_manager.svg", static_cast<PointerToMemberTrigger>(&Action::onCloudSessionManager));
    Action::regDef(actionDef, ACTION_GROUP_CLOUD, ACTION_CLOUD_FILE_MANAGER, tr("Cloud file manager"), "", "", ":/icons/cloud/pixmaps/range-file_manager.svg", static_cast<PointerToMemberTrigger>(&Action::onCloudFileManager));

    return actionDef;
}

void Action::regDef(QList<Definition> &actionDefinition, GroupType groupType, Type type, const QString &text, const QString &description, const QString &shortCut, const QString &icon, PointerToMemberTrigger triggerSlot, MenuRole menuRole)
{
    actionDefinition[type] = RAction::definition(Action::getGroupName(groupType),
                                                 Action::getName(type),
                                                 text,
                                                 description,
                                                 shortCut,
                                                 icon,
                                                 triggerSlot,
                                                 menuRole);
}

void Action::regDef(QList<Definition> &actionDefinition, GroupType groupType, Type type, const QString &text, const QString &description, const QString &shortCut, const QString &icon, PointerToMemberToggle toggleSlot, MenuRole menuRole)
{
    actionDefinition[type] = RAction::definition(Action::getGroupName(groupType),
                                                 Action::getName(type),
                                                 text,
                                                 description,
                                                 shortCut,
                                                 icon,
                                                 toggleSlot,
                                                 menuRole);
}

void Action::onSessionNew()
{
    R_LOG_TRACE_IN;
    int response = QMessageBox::question(Application::instance()->getMainWindow(),
                                         tr("Close session"),
                                         tr("Are you sure you want to close current session?"),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
    if (response == QMessageBox::Yes)
    {
        Application::instance()->getSession()->clear();
        this->onSessionSaveAs();
    }
    R_LOG_TRACE_OUT;
}

void Action::onSessionOpen()
{
    R_LOG_TRACE_IN;
    QString fileExtension = Session::getDefaultFileExtension();
    QString dialogDesc = "Range model files (*." + fileExtension + ");;Any files (*)";
    QString fileName = QFileDialog::getOpenFileName(Application::instance()->getMainWindow(),
                                                    tr("Open session"),
                                                    Application::instance()->getApplicationSettings()->getDataDir(),
                                                    dialogDesc);
    if (!fileName.isEmpty())
    {
        try
        {
            Application::instance()->getSession()->read(fileName);
        }
        catch (const RError &error)
        {
            RLogger::error("Failed to read the session file \'%s\'. ERROR: %s\n",fileName.toUtf8().constData(),error.getMessage().toUtf8().constData());
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onSessionSave()
{
    R_LOG_TRACE_IN;
    QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());
    QString dialogDesc = "Range session files (*.ras);;Any files (*)";

    QString fileName = Application::instance()->getSession()->getFileName();
    if (fileName.isEmpty())
    {
        fileName = dataDir.filePath("default.ras");
        fileName = QFileDialog::getSaveFileName(Application::instance()->getMainWindow(),
                                                tr("Save session"),
                                                fileName,
                                                dialogDesc);
    }
    if (!fileName.isEmpty())
    {
        Application::instance()->getSession()->write(fileName,true);
    }
    R_LOG_TRACE_OUT;
}

void Action::onSessionSaveAs()
{
    R_LOG_TRACE_IN;
    QDir sessionDir(Application::instance()->getApplicationSettings()->getSessionDir());
    QString dialogDesc = "Range session files (*.ras);;Any files (*)";

    QString fileName;
    if (fileName.isEmpty())
    {
        fileName = sessionDir.filePath("default.ras");
    }
    fileName = QFileDialog::getSaveFileName(Application::instance()->getMainWindow(),
                                            tr("Save session"),
                                            fileName,
                                            dialogDesc);
    if (!fileName.isEmpty())
    {
        Application::instance()->getSession()->write(fileName,true);
    }
    R_LOG_TRACE_OUT;
}

void Action::onSessionClose()
{
    R_LOG_TRACE_IN;
    int response = QMessageBox::question(Application::instance()->getMainWindow(),
                                         tr("Close session"),
                                         tr("Are you sure you want to close current session?"),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
    if (response == QMessageBox::Yes)
    {
        Application::instance()->getSession()->clear();
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelNew()
{
    R_LOG_TRACE_IN;
    NewModelDialog(Application::instance()->getMainWindow()).exec();
    R_LOG_TRACE_OUT;
}

void Action::onModelOpen()
{
    R_LOG_TRACE_IN;
    QString binaryExtension = RModel::getDefaultFileExtension(true);
    QString asciiExtension = RModel::getDefaultFileExtension(false);
    QString dialogDesc = "All supported files (*." + binaryExtension + " *." + asciiExtension + " *.tmsh *.bmsh *.raw *.stl);;"
                       + "Range model files (*." + binaryExtension + " *." + asciiExtension + ");;" +
                       + "Old Range model mesh files (*.tmsh *.bmsh);;"
                       + "RAW triangle files (*.raw);;"
                       + "STL triangle files (*.stl);;"
                       + "Any files (*)";
    QString fileName = QFileDialog::getOpenFileName(Application::instance()->getMainWindow(),
                                                    tr("Open model"),
                                                    Application::instance()->getApplicationSettings()->getDataDir(),
                                                    dialogDesc);
    if (!fileName.isEmpty())
    {
        ModelIOType modelIOType;

        QString extension = RFileManager::getExtension(fileName).toLower();
        if (extension == "tmsh" || extension == "bmsh")
        {
            modelIOType = MODEL_IO_MSH_IMPORT;
        }
        else if (extension == "raw")
        {
            modelIOType = MODEL_IO_RAW_IMPORT;
        }
        else if (extension == "stl")
        {
            modelIOType = MODEL_IO_STL_IMPORT;
        }
        else
        {
            modelIOType = MODEL_IO_OPEN;
        }

        RJobManager::getInstance().submit(new ModelIO(modelIOType, fileName));
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelSave()
{
    R_LOG_TRACE_IN;
    foreach (uint selectedModelID,Application::instance()->getSession()->getSelectedModelIDs())
    {
        Model &rModel = Application::instance()->getSession()->getModel(selectedModelID);

        QString fileName = ModelIO::getModelSaveName(Application::instance()->getApplicationSettings(),Application::instance()->getMainWindow(),rModel);

        if (!fileName.isEmpty())
        {
            RJobManager::getInstance().submit(new ModelIO(MODEL_IO_SAVE, fileName, &rModel));
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelSaveAs()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);

        QString fileName = ModelIO::getModelSaveName(Application::instance()->getApplicationSettings(),Application::instance()->getMainWindow(),model,true);

        if (!fileName.isEmpty())
        {
            RJobManager::getInstance().submit(new ModelIO(MODEL_IO_SAVE, fileName, &model));
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelExportMsh()
{
    R_LOG_TRACE_IN;
    QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(selectedModelIDs[i]);
        QString fileName = dataDir.filePath(pModel->getName() + ".tmsh");
        fileName = QFileDialog::getSaveFileName(Application::instance()->getMainWindow(),
                                                tr("Export model to MSH file"),
                                                fileName,
                                                "Range mesh files (*.tmsh *.bmsh);;Any files (*)");
        if (!fileName.isEmpty())
        {
            RJobManager::getInstance().submit(new ModelIO(MODEL_IO_MSH_EXPORT, fileName, pModel));
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelExportRaw()
{
    R_LOG_TRACE_IN;
    QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(selectedModelIDs[i]);
        QString fileName = dataDir.filePath(pModel->getName() + ".raw");
        fileName = QFileDialog::getSaveFileName(Application::instance()->getMainWindow(),
                                                tr("Export model to RAW file"),
                                                fileName,
                                                "RAW triangle files (*.raw);;Any files (*)");
        if (!fileName.isEmpty())
        {
            RJobManager::getInstance().submit(new ModelIO(MODEL_IO_RAW_EXPORT, fileName, pModel));
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelExportStlAscii()
{
    R_LOG_TRACE_IN;
    QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(selectedModelIDs[i]);
        QString fileName = dataDir.filePath(pModel->getName() + ".stl");
        fileName = QFileDialog::getSaveFileName(Application::instance()->getMainWindow(),
                                                tr("Export model to STL (ascii) file"),
                                                fileName,
                                                "STL triangle files (*.stl);;Any files (*)");
        if (!fileName.isEmpty())
        {
            RJobManager::getInstance().submit(new ModelIO(MODEL_IO_STL_ASCII_EXPORT, fileName, pModel));
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelExportStlBinary()
{
    R_LOG_TRACE_IN;
    QDir dataDir(Application::instance()->getApplicationSettings()->getDataDir());
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model *pModel = Application::instance()->getSession()->getModelPtr(selectedModelIDs[i]);
        QString fileName = dataDir.filePath(pModel->getName() + ".stl");
        fileName = QFileDialog::getSaveFileName(Application::instance()->getMainWindow(),
                                                tr("Export model to STL (binary) file"),
                                                fileName,
                                                "STL triangle files (*.stl);;Any files (*)");
        if (!fileName.isEmpty())
        {
            RJobManager::getInstance().submit(new ModelIO(MODEL_IO_STL_BINARY_EXPORT, fileName, pModel));
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelClose()
{
    R_LOG_TRACE_IN;
    int response = QMessageBox::question(Application::instance()->getMainWindow(),
                                         tr("Close model?"),
                                         tr("Are you sure you want to close selected models?"));
    if (response == QMessageBox::Yes)
    {
        QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
        for (int i=0;i<modelIDs.size();i++)
        {
            RLogger::info("Closing model \'%s\'\n",Application::instance()->getSession()->getModel(uint(modelIDs.size()-i-1)).getName().toUtf8().constData());
            Application::instance()->getSession()->removeModel(modelIDs[modelIDs.size()-i-1]);
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelReloadResults()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        RJob *updateJob = new ModelIO(MODEL_IO_UPDATE, Application::instance()->getSession()->getModel(modelIDs[i]).getFileName(), &Application::instance()->getSession()->getModel(modelIDs[i]));
        RJobManager::getInstance().submit(updateJob);
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelRename()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        RenameModelDialog(modelIDs.at(i)).exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onModelDropResults()
{
    R_LOG_TRACE_IN;
    DropResultsDialog dropResultsDialog(Application::instance()->getMainWindow());
    dropResultsDialog.exec();
    R_LOG_TRACE_OUT;
}

void Action::onGeometryUndo()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        if (rModel.getUndoStackSize() > 0)
        {
            Application::instance()->getSession()->getPickList().removeItems(selectedModelIDs[i]);
            rModel.undo(0);
        }
        Application::instance()->getSession()->setModelChanged(selectedModelIDs[i]);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryRedo()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        if (rModel.getRedoStackSize() > 0)
        {
            Application::instance()->getSession()->getPickList().removeItems(selectedModelIDs[i]);
            rModel.redo(0);
        }
        Application::instance()->getSession()->setModelChanged(selectedModelIDs[i]);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawPoint()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEnginePoint);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawLine()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineLine);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawTriangle()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineTriangle);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawQuadrilateral()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineQuadrilateral);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawCircle()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineCircle);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawEllipse()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineEllipse);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawTetrahedron()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineTetrahedron);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawHexahedron()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineHexahedron);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawCylinder()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineCylinder);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawSphere()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineSphere);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDrawRaw()
{
    R_LOG_TRACE_IN;
    Application::instance()->getSession()->getDrawEngine()->addObject(new DrawEngineRaw);
    R_LOG_TRACE_OUT;
}

void Action::onGeometryCreateElement()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_NODE) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Create element"),
                                 tr("No node selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }

    QList<uint> modelIDs = pickList.getModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint nNodes = pickList.getNItems(PICK_ITEM_NODE,modelIDs[i]);

        RElementType elementType = R_ELEMENT_NONE;

        switch (nNodes)
        {
            case 0:
            {
                break;
            }
            case 1:
            {
                // Create point element.
                elementType = R_ELEMENT_POINT;
                break;
            }
            case 2:
            {
                // Create line element.
                elementType = R_ELEMENT_TRUSS1;
                break;
            }
            case 3:
            {
                // Create triangle element.
                elementType = R_ELEMENT_TRI1;
                break;
            }
            case 4:
            {
                // Create rectangle or tetrahedra element.
                QMessageBox msgBox(Application::instance()->getMainWindow());
                QPushButton *quadButton = msgBox.addButton(tr("Quadrilateral"), QMessageBox::YesRole);
                QPushButton *tetrButton = msgBox.addButton(tr("Tetrahedra"), QMessageBox::NoRole);

                msgBox.setWindowTitle(tr("Choose element type"));
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setText(tr("Which element type to create?"));
                msgBox.exec();

                if (msgBox.clickedButton() == quadButton)
                {
                    // Create quadrilateral element.
                    elementType = R_ELEMENT_QUAD1;
                }
                else if (msgBox.clickedButton() == tetrButton)
                {
                    // Create tetrahedra element.
                    elementType = R_ELEMENT_TETRA1;
                }
                break;
            }
            default:
            {
                QMessageBox::information(Application::instance()->getMainWindow(),
                                         tr("Create element"),
                                         tr("Too many nodes selected."),
                                         QMessageBox::Close);
                R_LOG_TRACE_OUT;
                return;
            }
        }

        if (elementType == R_ELEMENT_NONE)
        {
            continue;
        }

        QList<uint> nodeIDs;

        QVector<PickItem> pickItems = pickList.getItems(modelIDs[i]);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_NODE)
            {
                uint elementID = pickItems[j].getElementID();
                uint nodeID = pickItems[j].getNodeID();
                if (elementID != RConstants::eod && nodeID != RConstants::eod)
                {
                    nodeIDs.append(nodeID);
                    pickList.removeItem(pickItems[j]);
                }
            }
        }

        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setCreateElement(nodeIDs,elementType);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryFindSliverElements()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        FindSliverElementsDialog findliverElementsDialog(modelIDs[i],Application::instance()->getMainWindow());
        findliverElementsDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryFixSliverElements()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        FixSliverElementsDialog fixSliverElementsDialog(modelIDs[i],Application::instance()->getMainWindow());
        fixSliverElementsDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryFindIntersectedElements()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setFindIntersectedElements();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryBreakIntersectedElements()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
//        // Questionable whether to remove volume elements
//        // It should not be possible to generate volume mesh if intersected elements are present
//        const Model &rModel = Application::instance()->getSession()->getModel(modelIDs[i]);
//        if (rModel.getNVolumes() > 0)
//        {
//            if (QMessageBox::question(Application::instance()->getMainWindow(),
//                                      tr("Volume elements found"),
//                                      tr("To break intersected elements all volume elements must be removed.<br/>Are you sure you want to continue and remove all volume elements?"),
//                                      QMessageBox::No,
//                                      QMessageBox::Yes) == QMessageBox::No)
//            {
//                R_LOG_TRACE_OUT;
//                return;
//            }
//        }

        BreakIntersectedElementsDialog bieDialog(modelIDs[i],Application::instance()->getMainWindow());
        bieDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryBoolUnion()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i) && selectedEntityIDs.at(j).getType() == R_ENTITY_GROUP_SURFACE)
            {
                entityIDs.push_back(selectedEntityIDs.at(j));
            }
        }

        if (entityIDs.size() < 2)
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Too few surfaces selected"),
                              tr("To perform union boolean operation at least two surfaces must be selected."));
            R_LOG_TRACE_OUT;
            return;
        }

        for (int j=0;j<entityIDs.size();j++)
        {
            const Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
            if (!rModel.checkIfSurfaceIsClosed(entityIDs[j].getEid()))
            {
                QMessageBox::warning(Application::instance()->getMainWindow(),
                                     tr("Surface is not closed"),
                                     tr("Surface") + " <b>" + rModel.getSurface(entityIDs[j].getEid()).getName() + "</b> " + tr("is not closed.") + "<br/>"
                                     + tr("To perform union boolean operation all selected surfaces must be closed."));
                R_LOG_TRACE_OUT;
                return;
            }
        }

        BoolUnionDialog boolUnionDialog(selectedModelIDs[i],entityIDs,Application::instance()->getMainWindow());
        boolUnionDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryBoolDifference()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i) && selectedEntityIDs.at(j).getType() == R_ENTITY_GROUP_SURFACE)
            {
                entityIDs.push_back(selectedEntityIDs.at(j));
            }
        }

        if (entityIDs.size() < 2)
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Too few surfaces selected"),
                              tr("To perform difference boolean operation at least two surfaces must be selected."));
            R_LOG_TRACE_OUT;
            return;
        }

        for (int j=0;j<entityIDs.size();j++)
        {
            const Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
            if (!rModel.checkIfSurfaceIsClosed(entityIDs[j].getEid()))
            {
                QMessageBox::warning(Application::instance()->getMainWindow(),
                                     tr("Surface is not closed"),
                                     tr("Surface") + " <b>" + rModel.getSurface(entityIDs[j].getEid()).getName()  + "</b> " + tr("is not closed.") + "<br/>"
                                     + tr("To perform difference boolean operation all selected surfaces must be closed."));
                R_LOG_TRACE_OUT;
                return;
            }
        }

        BoolDifferenceDialog boolDifferenceDialog(selectedModelIDs[i],entityIDs,Application::instance()->getMainWindow());
        boolDifferenceDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryBoolIntersection()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i) && selectedEntityIDs.at(j).getType() == R_ENTITY_GROUP_SURFACE)
            {
                entityIDs.push_back(selectedEntityIDs.at(j));
            }
        }

        if (entityIDs.size() < 2)
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Too few surfaces selected"),
                              tr("To perform intersection boolean operation at least two surfaces must be selected."));
            R_LOG_TRACE_OUT;
            return;
        }

        for (int j=0;j<entityIDs.size();j++)
        {
            const Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
            if (!rModel.checkIfSurfaceIsClosed(entityIDs[j].getEid()))
            {
                QMessageBox::warning(Application::instance()->getMainWindow(),
                                     tr("Surface is not closed"),
                                     tr("Surface") + " <b>" + rModel.getSurface(entityIDs[j].getEid()).getName()  + "</b> " + tr("is not closed.") + "<br/>"
                                     + tr("To perform intersection boolean operation all selected surfaces must be closed."));
                R_LOG_TRACE_OUT;
                return;
            }
        }

        BoolIntersectionDialog boolIntersectionDialog(selectedModelIDs[i],entityIDs,Application::instance()->getMainWindow());
        boolIntersectionDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryMergeNearNodes()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        MergeNearNodesDialog mergeNearNodesDialog(selectedModelIDs[i],Application::instance()->getMainWindow());
        mergeNearNodesDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryMoveNode()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_NODE) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Move node"),
                                 tr("No node selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }

    QList<uint> modelIDs = pickList.getModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        QList<uint> nodeIDs;

        QVector<PickItem> pickItems = pickList.getItems(modelIDs[i]);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_NODE)
            {
                uint elementID = pickItems[j].getElementID();
                uint lNodeID = pickItems[j].getNodeID();
                if (elementID != RConstants::eod && lNodeID != RConstants::eod)
                {
                    nodeIDs.append(lNodeID);
                }
            }
        }

        if (nodeIDs.size() > 0)
        {
            std::sort(nodeIDs.begin(),nodeIDs.end());

            MoveNodeDialog moveNodeDialog(modelIDs[i],nodeIDs,Application::instance()->getMainWindow());
            moveNodeDialog.exec();
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryRemoveNode()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_NODE) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Remove nodes"),
                                 tr("No node selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }
    if (QMessageBox::question(Application::instance()->getMainWindow(),
                               tr("Remove nodes"),
                               tr("Are you sure you want to remove picked nodes?"),
                               QMessageBox::No,
                               QMessageBox::Yes) == QMessageBox::No)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    QList<uint> modelIDs = pickList.getModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QList<uint> nodeIDs;

        QVector<PickItem> pickItems = pickList.getItems(modelIDs[i]);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_NODE)
            {
                uint elementID = pickItems[j].getElementID();
                uint nodeID = pickItems[j].getNodeID();
                if (elementID != RConstants::eod && nodeID != RConstants::eod)
                {
                    nodeIDs.append(nodeID);
                    pickList.removeItem(pickItems[j]);
                }
            }
        }

        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setRemoveNodes(nodeIDs,false);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryRemoveNodeAndCloseHole()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_NODE) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Remove nodes"),
                                 tr("No node selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }
    if (QMessageBox::question(Application::instance()->getMainWindow(),
                               tr("Remove nodes"),
                               tr("Are you sure you want to remove picked nodes?"),
                               QMessageBox::No,
                               QMessageBox::Yes) == QMessageBox::No)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    QList<uint> modelIDs = pickList.getModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QList<uint> nodeIDs;

        QVector<PickItem> pickItems = pickList.getItems(modelIDs[i]);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_NODE)
            {
                uint elementID = pickItems[j].getElementID();
                uint nodeID = pickItems[j].getNodeID();
                if (elementID != RConstants::eod && nodeID != RConstants::eod)
                {
                    nodeIDs.append(nodeID);
                    pickList.removeItem(pickItems[j]);
                }
            }
        }

        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setRemoveNodes(nodeIDs,true);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryRemoveElement()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_ELEMENT) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Remove elements"),
                                 tr("No element selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }
    if (QMessageBox::question(Application::instance()->getMainWindow(),
                               tr("Remove elements"),
                               tr("Are you sure you want to remove picked elements?"),
                               QMessageBox::No,
                               QMessageBox::Yes) == QMessageBox::No)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    QList<uint> modelIDs = pickList.getModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QList<uint> elementIDs;

        QVector<PickItem> pickItems = pickList.getItems(modelIDs[i]);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_ELEMENT)
            {
                uint elementID = pickItems[j].getElementID();
                if (elementID != RConstants::eod)
                {
                    elementIDs.append(elementID);
                    pickList.removeItem(pickItems[j]);
                }
            }
        }

        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setRemoveElements(elementIDs,false);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryLineGenerateFromEdges()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setGenerateLinesFromEdges(40.0);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometrySurfaceMark()
{
    R_LOG_TRACE_IN;
    MarkEntityDialog markEntityDialog(R_ENTITY_GROUP_SURFACE,Application::instance()->getMainWindow());
    markEntityDialog.exec();
    R_LOG_TRACE_OUT;
}

void Action::onGeometrySurfaceSwapElementNormal()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_ELEMENT) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Swap element normal"),
                                 tr("No element selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }
    QList<uint> modelIDs = pickList.getModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QList<uint> elementIDs;
        QVector<PickItem> pickItems = pickList.getItems(modelIDs[i]);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_ELEMENT)
            {
                uint elementID = pickItems[j].getElementID();
                if (elementID != RConstants::eod)
                {
                    elementIDs.append(elementID);
                    pickList.removeItem(pickItems[j]);
                }
            }
        }

        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setSwapSurfaceElementNormal(elementIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometrySurfaceSwapNormals()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i))
            {
                entityIDs.push_back(selectedEntityIDs.at(j));
            }
        }

        ModelActionInput modelActionInput(selectedModelIDs.at(i));
        modelActionInput.setSwapSurfaceNormals(entityIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometrySurfaceSyncNormals()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        ModelActionInput modelActionInput(selectedModelIDs.at(i));
        modelActionInput.setSyncSurfaceNormals();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometrySurfaceCloseHole()
{
    R_LOG_TRACE_IN;
    PickList &pickList = Application::instance()->getSession()->getPickList();

    if (pickList.getNItems(PICK_ITEM_HOLE_ELEMENT) == 0)
    {
        QMessageBox::information(Application::instance()->getMainWindow(),
                                 tr("Close surface hole"),
                                 tr("No edge selected."),
                                 QMessageBox::Close);
        R_LOG_TRACE_OUT;
        return;
    }

    foreach (uint modelID,pickList.getModelIDs())
    {
        QList<uint> holeIDs;

        QVector<PickItem> pickItems = pickList.getItems(modelID);
        for (int j=0;j<pickItems.size();j++)
        {
            if (pickItems[j].getItemType() == PICK_ITEM_HOLE_ELEMENT)
            {
                uint elementID = pickItems[j].getElementID();
                if (elementID != RConstants::eod)
                {
                    holeIDs.append(elementID);
                    pickList.removeItem(pickItems[j]);
                }
            }
        }

        ModelActionInput modelActionInput(modelID);
        modelActionInput.setCloseSurfaceHole(holeIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometrySurfaceCoarsen()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i))
            {
                entityIDs.push_back(selectedEntityIDs.at(j));
            }
        }

        CoarsenSurfaceDialog coarsenSurfaceDialog(selectedModelIDs[i],entityIDs,Application::instance()->getMainWindow());
        coarsenSurfaceDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryVectorFieldCreate()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        VectorFieldDialog vectorFieldDialog(modelIDs[i],Application::instance()->getMainWindow());
        vectorFieldDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryVectorFieldEdit()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() == R_ENTITY_GROUP_VECTOR_FIELD)
        {
            VectorFieldDialog vectorFieldDialog(entityIDs[i].getMid(),entityIDs[i].getEid(),Application::instance()->getMainWindow());
            vectorFieldDialog.exec();
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryScalarFieldCreate()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ScalarFieldDialog scalarFieldDialog(modelIDs[i],Application::instance()->getMainWindow());
        scalarFieldDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryScalarFieldEdit()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() != R_ENTITY_GROUP_SCALAR_FIELD)
        {
            continue;
        }
        ScalarFieldDialog scalarFieldDialog(entityIDs[i].getMid(),entityIDs[i].getEid(),Application::instance()->getMainWindow());
        scalarFieldDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryStreamLineCreate()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        StreamLineDialog streamLineDialog(modelIDs[i],Application::instance()->getMainWindow());
        streamLineDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryStreamLineEdit()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() != R_ENTITY_GROUP_STREAM_LINE)
        {
            continue;
        }
        StreamLineDialog streamLineDialog(entityIDs[i].getMid(),entityIDs[i].getEid(),Application::instance()->getMainWindow());
        streamLineDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryCutCreate()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        CutDialog *cutDialog = new CutDialog(modelIDs[i],RConstants::eod,Application::instance()->getMainWindow());
        cutDialog->show();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryCutEdit()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() != R_ENTITY_GROUP_CUT)
        {
            continue;
        }
        CutDialog *cutDialog = new CutDialog(entityIDs[i].getMid(),entityIDs[i].getEid(),Application::instance()->getMainWindow());
        cutDialog->show();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryIsoCreate()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        IsoDialog isoDialog(modelIDs[i],Application::instance()->getMainWindow());
        isoDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryIsoEdit()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> entityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

    for (int i=0;i<entityIDs.size();i++)
    {
        if (entityIDs[i].getType() != R_ENTITY_GROUP_ISO)
        {
            continue;
        }
        IsoDialog isoDialog(entityIDs[i].getMid(),entityIDs[i].getEid(),Application::instance()->getMainWindow());
        isoDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryEntityMerge()
{
    R_LOG_TRACE_IN;
    MergeEntityDialog mergeEntityDialog(R_ENTITY_GROUP_ELEMENT,Application::instance()->getMainWindow());
    mergeEntityDialog.exec();
    R_LOG_TRACE_OUT;
}

void Action::onGeometryEntityRemove()
{
    R_LOG_TRACE_IN;
    RemoveEntityDialog removeEntityDialog(R_ENTITY_GROUP_ALL,Application::instance()->getMainWindow());
    removeEntityDialog.exec();
    R_LOG_TRACE_OUT;
}

void Action::onGeometryGenerateTetrahedra()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        MeshGeneratorDialog meshGeneratorDialog(modelIDs[i],Application::instance()->getMainWindow());
        meshGeneratorDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryTransform()
{
    R_LOG_TRACE_IN;
    qobject_cast<MainWindow*>(Application::instance()->getMainWindow())->showTransformGeometryWidget();
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevExportSliverElements()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelActionInput modelActionInput(modelIDs[i]);
        modelActionInput.setExportSliverElements();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevExportIntersectedElements()
{
    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        ModelActionInput modelActionInput(modelID);
        modelActionInput.setExportIntersectedElements();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevPurgeUnusedNodes()
{
    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        ModelActionInput modelActionInput(modelID);
        modelActionInput.setPurgeUnusedNodes();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevPurgeUnusedElements()
{
    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        ModelActionInput modelActionInput(modelID);
        modelActionInput.setPurgeUnusedElements();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevRemoveDuplicateNodes()
{
    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        ModelActionInput modelActionInput(modelID);
        modelActionInput.setRemoveDuplicateNodes();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevRemoveDuplicateElements()
{
    R_LOG_TRACE_IN;
    foreach (uint modelID, Application::instance()->getSession()->getSelectedModelIDs())
    {
        ModelActionInput modelActionInput(modelID);
        modelActionInput.setRemoveDuplicateElements();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevPointInsideSurface()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;
        QList<uint> surfaceIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i) && selectedEntityIDs.at(j).getType() == R_ENTITY_GROUP_SURFACE)
            {
                entityIDs.append(selectedEntityIDs.at(j));
                surfaceIDs.append(selectedEntityIDs.at(j).getEid());
            }
        }

        if (entityIDs.size() == 0)
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Too few surfaces selected"),
                              tr("To perform point inside check at least one surface must be selected."));
            R_LOG_TRACE_OUT;
            return;
        }

        const Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        if (!rModel.checkIfSurfacesAreClosed(surfaceIDs))
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Unclosed surfaces"),
                              tr("Selected surfaces do not form closed surface."));
            R_LOG_TRACE_OUT;
            return;
        }

        PointInsideSurfaceDialog pointInsideSurfaceDialog(selectedModelIDs[i],entityIDs,Application::instance()->getMainWindow());
        pointInsideSurfaceDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevTetrahedralizeeSurface()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        QList<SessionEntityID> entityIDs;
        QList<uint> surfaceIDs;

        for (int j=0;j<selectedEntityIDs.size();j++)
        {
            if (selectedEntityIDs.at(j).getMid() == selectedModelIDs.at(i) && selectedEntityIDs.at(j).getType() == R_ENTITY_GROUP_SURFACE)
            {
                entityIDs.append(selectedEntityIDs.at(j));
                surfaceIDs.append(selectedEntityIDs.at(j).getEid());
            }
        }

        if (entityIDs.size() == 0)
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Too few surfaces selected"),
                              tr("To perform point inside check at least one surface must be selected."));
            R_LOG_TRACE_OUT;
            return;
        }

        const Model &rModel = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        if (!rModel.checkIfSurfacesAreClosed(surfaceIDs))
        {
            QMessageBox::warning(Application::instance()->getMainWindow(),
                              tr("Unclosed surfaces"),
                              tr("Selected surfaces do not form closed surface."));
            R_LOG_TRACE_OUT;
            return;
        }

        ModelActionInput modelActionInput(selectedModelIDs[i]);
        modelActionInput.setTetrahedralizeSurface(entityIDs);

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onGeometryDevConsolidate()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<selectedModelIDs.size();i++)
    {
        ModelActionInput modelActionInput(selectedModelIDs[i]);
        modelActionInput.setConsolidate();

        ModelAction *modelAction = new ModelAction;
        modelAction->setAutoDelete(true);
        modelAction->addAction(modelActionInput);
        RJobManager::getInstance().submit(modelAction);
    }
    R_LOG_TRACE_OUT;
}

void Action::onProblemTaskFlow()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ProblemTaskDialog problemTaskDialog(modelIDs[i],Application::instance()->getMainWindow());
        problemTaskDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onProblemSolverSetup()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        MatrixSolverConfigDialog matrixSolverConfigDialog(modelIDs[i],Application::instance()->getMainWindow());
        matrixSolverConfigDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onProblemDefineMonitoringPoints()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        MonitoringPointsDialog monitoringPointsDialog(modelIDs[i],Application::instance()->getMainWindow());
        monitoringPointsDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onProblemReset()
{
    R_LOG_TRACE_IN;
    if (QMessageBox::question(Application::instance()->getMainWindow(),
                               tr("Reset problem setup"),
                               tr("Are you sure you want to reset all problem setup including boundary, initial and environment conditions?"),
                               QMessageBox::No,
                               QMessageBox::Yes) == QMessageBox::No)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    for (int i=0;i<modelIDs.size();i++)
    {
        Application::instance()->getSession()->storeCurentModelVersion(modelIDs[i],tr("Problem reset"));
        Application::instance()->getSession()->getModel(modelIDs[i]).RProblem::operator =(RProblem());
        Application::instance()->getSession()->getModel(modelIDs[i]).clearBoundaryConditions();
        Application::instance()->getSession()->getModel(modelIDs[i]).clearInitialConditions();
        Application::instance()->getSession()->getModel(modelIDs[i]).clearEnvironmentConditions();
        Application::instance()->getSession()->getModel(modelIDs[i]).clearEntityVariableData();
        Application::instance()->getSession()->setModelChanged(modelIDs[i]);
    }
    R_LOG_TRACE_OUT;
}

void Action::onSolverStart()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    // First perform checks whether the setup is correct.
    for (int i=0;i<modelIDs.size();i++)
    {
        Model &rModel = Application::instance()->getSession()->getModel(modelIDs[i]);
        QString modelFileName = ModelIO::getModelSaveName(Application::instance()->getApplicationSettings(),Application::instance()->getMainWindow(),rModel);
        if (!modelFileName.isEmpty())
        {
            rModel.setFileName(modelFileName);
            SolverStartDialog solverStartDialog(modelIDs[i],Application::instance()->getMainWindow());
            solverStartDialog.exec();
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onSolverStop()
{
    R_LOG_TRACE_IN;
    if (QMessageBox::question(Application::instance()->getMainWindow(),
                               tr("Stop solver"),
                               tr("Are you sure you want to stop all running solvers?"),
                               QMessageBox::No,
                               QMessageBox::Yes) == QMessageBox::No)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    try
    {
        SolverManager::getInstance().stopRunningTasks();
    }
    catch (RError &error)
    {
        RLogger::error("Could not stop the solver (%s).\n",error.getMessage().toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void Action::onSolverKill()
{
    R_LOG_TRACE_IN;
    if (QMessageBox::question(Application::instance()->getMainWindow(),
                               tr("Kill solver"),
                               tr("Are you sure you want to kill all running solvers?"),
                               QMessageBox::No,
                               QMessageBox::Yes) == QMessageBox::No)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    try
    {
        SolverManager::getInstance().killRunningTasks();
    }
    catch (RError &error)
    {
        RLogger::error("Could not kill the solver (%s).\n",error.getMessage().toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void Action::onReportModelStatistics()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ModelStatisticsDialog modelStatisticsDialog(modelIDs[i],Application::instance()->getMainWindow());
        modelStatisticsDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onReportSolverLog()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QString fileName(RFileManager::findLastFile(Application::instance()->getSession()->getModel(modelIDs[i]).buildTmpFileName("log",QString("*"))));
        if (fileName.isEmpty())
        {
            QMessageBox::information(Application::instance()->getMainWindow(),tr("No solver log file"),tr("There is no file containing solver log."));
        }
        else
        {
            RLogBrowserDialog logBrowserDialog(fileName,Application::instance()->getMainWindow());
            logBrowserDialog.exec();
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onReportConvergenceGraph()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QStringList fileNames;

        QString filePattern(Application::instance()->getSession()->getModel(modelIDs[i]).buildTmpFileName("cvg",QString("*")));

        std::vector<RProblemType> problemTypes = RProblem::getTypes(R_PROBLEM_ALL);
        for (uint i=0;i<problemTypes.size();i++)
        {
            QString fileName(RFileManager::findLastFile(RFileManager::getFileNameWithSuffix(filePattern,RProblem::getId(problemTypes[i]))));
            if (!fileName.isEmpty())
            {
                fileNames.append(fileName);
            }
        }

        for (RMatrixSolverType type=RMatrixSolverConf::None;type<RMatrixSolverConf::NTypes;type++)
        {
            QString fileName(RFileManager::findLastFile(RFileManager::getFileNameWithSuffix(filePattern,RMatrixSolverConf::getId(type))));
            if (!fileName.isEmpty())
            {
                fileNames.append(fileName);
            }
        }

        if (fileNames.isEmpty())
        {
            QMessageBox::information(Application::instance()->getMainWindow(),tr("No convergence file"),tr("There is no file containing convergence values."));
        }
        else
        {
            ConvergenceGraphDialog *convergenceGraphDialog = new ConvergenceGraphDialog(fileNames,Application::instance()->getMainWindow());
            convergenceGraphDialog->setModal(false);
            convergenceGraphDialog->show();
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onReportMonitoringPointGraph()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        QString fileName(RFileManager::findLastFile(Application::instance()->getSession()->getModel(modelIDs[i]).buildTmpFileName("mon",QString("*"))));
        if (fileName.isEmpty())
        {
            QMessageBox::information(Application::instance()->getMainWindow(),tr("No monitoring file"),tr("There is no file containing monitoring points."));
        }
        else
        {
            MonitoringPointGraphDialog monitoringPointGraphDialog(fileName,Application::instance()->getMainWindow());
            monitoringPointGraphDialog.exec();
        }
    }
    R_LOG_TRACE_OUT;
}

void Action::onReportCreateReport()
{
    R_LOG_TRACE_IN;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        ReportDialog reportDialog(modelIDs[i],Application::instance()->getMainWindow());
        reportDialog.exec();
    }
    R_LOG_TRACE_OUT;
}

void Action::onHelp()
{
    R_LOG_TRACE_IN;

    QMap<QString,QString> topics;

    topics.insert("general",tr("General"));
    topics.insert("tutorials",tr("Tutorials"));
    // topics.insert("tips",tr("Tips"));

    HelpDialog *helpDialog = new HelpDialog(tr("Help"),topics,Application::instance()->getMainWindow());
    helpDialog->setModal(false);
    helpDialog->show();

    R_LOG_TRACE_OUT;
}

void Action::onQuit()
{
    R_LOG_TRACE_IN;
    Application::instance()->getMainWindow()->close();
    R_LOG_TRACE_OUT;
}

void Action::onApplicationSettings()
{
    R_LOG_TRACE_IN;
    ApplicationSettingsDialog applicationSettingsDialog(Application::instance()->getApplicationSettings(),Application::instance()->getMainWindow());
    applicationSettingsDialog.exec();
    R_LOG_TRACE_OUT;
}

void Action::onUpdate()
{
    R_LOG_TRACE_IN;
    RSoftwareManagerDialog softwareManagerDialog(Application::instance()->getApplicationSettings(),Application::instance()->getMainWindow());
    softwareManagerDialog.exec();
    R_LOG_TRACE_OUT;
}

void Action::onAbout()
{
    R_LOG_TRACE_IN;
    QString title = tr("About");
    QString body = QString("<h1>%1</h1>").arg(RVendor::name())
                 + QString("<h2>%1</h2>").arg(RVendor::description())
                 + QString("%1<br/>").arg(RVendor::version().toString())
                 + QString("%1 &copy; %2<br/>").arg(RVendor::author().toHtmlEscaped(),QString::number(RVendor::year()))
                 + QString("<a href=\"mailto:%1\">%1</a><br/>").arg(RVendor::email())
                 + QString("<a href=\"%1\">%1</a>").arg(RVendor::www());

    QMessageBox::about(Application::instance()->getMainWindow(),title,body);
    R_LOG_TRACE_OUT;
}

void Action::onAboutQt()
{
    R_LOG_TRACE_IN;
    QMessageBox::aboutQt(Application::instance()->getMainWindow(),tr("About Qt"));
    R_LOG_TRACE_OUT;
}

void Action::onLicense()
{
    R_LOG_TRACE_IN;
    QString licenseFileName(Application::instance()->getApplicationSettings()->findLicenseFileName());

    try
    {
        QFile file(licenseFileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw RError(RError::Type::OpenFile,R_ERROR_REF,"Failed to open the file \'%s\'.",licenseFileName.toUtf8().constData());
        }
        QString licenseText(file.readAll());
        file.close();

        RTextBrowserDialog textBrowserDialog(tr("License"),
                                            licenseFileName,
                                            licenseText);
        textBrowserDialog.exec();
    }
    catch (const RError &rError)
    {
        RLogger::error("Failed to display license from file \'%s\'. %s\n",licenseFileName.toUtf8().constData(),rError.getMessage().toUtf8().constData());
    }
    catch (...)
    {
        RLogger::error("Failed to display license from file \'%s\'.\n",licenseFileName.toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void Action::onReleaseNotes()
{
    R_LOG_TRACE_IN;
    QString releaseNotesFileName(Application::instance()->getApplicationSettings()->findReleaseNotesFileName());

    try
    {
        QFile file(releaseNotesFileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw RError(RError::Type::OpenFile,R_ERROR_REF,"Failed to open the file \'%s\'.",releaseNotesFileName.toUtf8().constData());
        }
        QString releaseNotesText(file.readAll());
        file.close();

        RTextBrowserDialog textBrowserDialog(tr("Release notes"),
                                            releaseNotesFileName,
                                            releaseNotesText);
        textBrowserDialog.exec();
    }
    catch (const RError &rError)
    {
        RLogger::error("Failed to display release notes from file \'%s\'. %s\n",releaseNotesFileName.toUtf8().constData(),rError.getMessage().toUtf8().constData());
    }
    catch (...)
    {
        RLogger::error("Failed to display release notes from file \'%s\'.\n",releaseNotesFileName.toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void Action::onCloudSessionManager()
{
    R_LOG_TRACE_IN;
    RCloudSessionDialog dialog(Application::instance()->getCloudSessionManager()->findSession(Application::instance()->getCloudSessionManager()->findActiveSessionName()),
                               Application::instance()->getCloudConnectionHandler(),
                               Application::instance()->getApplicationSettings(),
                               Application::instance()->getMainWindow());
    if (dialog.exec() == QDialog::Accepted)
    {
        Application::instance()->getCloudSessionManager()->insertSession(dialog.getSessionInfo());
    }
    R_LOG_TRACE_OUT;
}

void Action::onCloudFileManager()
{
    R_LOG_TRACE_IN;
    RCloudFileManagerDialog cloudFileManagerDialog(Application::instance()->getCloudConnectionHandler(),
                                                   Application::instance()->getApplicationSettings(),
                                                   false,
                                                   Application::instance()->getMainWindow());
    cloudFileManagerDialog.exec();
    R_LOG_TRACE_OUT;
}
