qt_add_executable(fea
    src/action.cpp
    src/action_list.cpp
    src/application.cpp
    src/application_settings.cpp
    src/application_settings_dialog.cpp
    src/application_settings_widget.cpp
    src/bc_manager_tree.cpp
    src/bc_tree.cpp
    src/bool_difference_dialog.cpp
    src/bool_intersection_dialog.cpp
    src/bool_union_dialog.cpp
    src/break_intersected_elements_dialog.cpp
    src/central_tab_widget.cpp
    src/clipping_plane_widget.cpp
    src/coarsen_surface_dialog.cpp
    src/color.cpp
    src/color_combo_box.cpp
    src/color_scale.cpp
    src/color_scale_combo_box.cpp
    src/component_edit_dialog.cpp
    src/component_edit_table.cpp
    src/convergence_graph_dialog.cpp
    src/convergence_graph_object.cpp
    src/convergence_graph_widget.cpp
    src/cut_dialog.cpp
    src/direction_widget.cpp
    src/document_generator.cpp
    src/document_tree.cpp
    src/draw_engine.cpp
    src/draw_engine_circle.cpp
    src/draw_engine_cylinder.cpp
    src/draw_engine_ellipse.cpp
    src/draw_engine_hexahedron.cpp
    src/draw_engine_input.cpp
    src/draw_engine_line.cpp
    src/draw_engine_object.cpp
    src/draw_engine_point.cpp
    src/draw_engine_quadrilateral.cpp
    src/draw_engine_raw.cpp
    src/draw_engine_sphere.cpp
    src/draw_engine_tetrahedron.cpp
    src/draw_engine_triangle.cpp
    src/draw_input_tree.cpp
    src/draw_input_widget.cpp
    src/drop_results_dialog.cpp
    src/ec_manager_tree.cpp
    src/ec_tree.cpp
    src/find_sliver_elements_dialog.cpp
    src/fix_sliver_elements_dialog.cpp
    src/geometry_rotate_widget.cpp
    src/geometry_scale_widget.cpp
    src/geometry_transform_input.cpp
    src/geometry_transform_widget.cpp
    src/geometry_translate_widget.cpp
    src/gl_acion_event.cpp
    src/gl_arrow.cpp
    src/gl_axis.cpp
    src/gl_cut_plane.cpp
    src/gl_dimension.cpp
    src/gl_display_properties.cpp
    src/gl_display_properties_dialog.cpp
    src/gl_element.cpp
    src/gl_element_base.cpp
    src/gl_element_group.cpp
    src/gl_entity.cpp
    src/gl_entity_list.cpp
    src/gl_functions.cpp
    src/gl_grid.cpp
    src/gl_interpolated_element.cpp
    src/gl_interpolated_entity.cpp
    src/gl_line.cpp
    src/gl_list.cpp
    src/gl_model_list.cpp
    src/gl_node.cpp
    src/gl_object.cpp
    src/gl_point.cpp
    src/gl_rotation_sphere.cpp
    src/gl_scalar_field.cpp
    src/gl_simplex.cpp
    src/gl_simplex_point.cpp
    src/gl_simplex_polygon.cpp
    src/gl_simplex_segment.cpp
    src/gl_simplex_tetrahedra.cpp
    src/gl_text_renderer.cpp
    src/gl_text_renderer_item.cpp
    src/gl_texture.cpp
    src/gl_vector_field.cpp
    src/gl_widget.cpp
    src/graph_data.cpp
    src/graph_dialog.cpp
    src/graph_object.cpp
    src/graph_widget.cpp
    src/help_dialog.cpp
    src/help_gl_action_event_widget.cpp
    src/ic_manager_tree.cpp
    src/ic_tree.cpp
    src/iso_dialog.cpp
    src/light_properties_widget.cpp
    src/local_direction_widget.cpp
    src/main.cpp
    src/main_window.cpp
    src/mark_entity_dialog.cpp
    src/material_manager_tree.cpp
    src/material_property_add_dialog.cpp
    src/material_property_edit_dialog.cpp
    src/material_property_edit_table.cpp
    src/material_property_line_edit.cpp
    src/material_tree.cpp
    src/matrix_solver_config_dialog.cpp
    src/merge_entity_dialog.cpp
    src/merge_near_nodes_dialog.cpp
    src/mesh_generator.cpp
    src/mesh_generator_dialog.cpp
    src/mesh_setup_widget.cpp
    src/modal_setup_widget.cpp
    src/model.cpp
    src/model_action.cpp
    src/model_action_input.cpp
    src/model_entity_display_properties_tree.cpp
    src/model_entity_geometric_properties_tree.cpp
    src/model_io.cpp
    src/model_records_selector.cpp
    src/model_records_tree.cpp
    src/model_statistics_dialog.cpp
    src/model_tree.cpp
    src/model_tree_simple.cpp
    src/monitoring_point_graph_dialog.cpp
    src/monitoring_points_dialog.cpp
    src/move_node_dialog.cpp
    src/new_model_dialog.cpp
    src/pick_details_tree.cpp
    src/pick_item.cpp
    src/pick_list.cpp
    src/pick_value.cpp
    src/point_inside_surface_dialog.cpp
    src/position_widget.cpp
    src/problem_selector_dialog.cpp
    src/problem_selector_tree.cpp
    src/problem_task_dialog.cpp
    src/problem_task_tree.cpp
    src/problem_tree.cpp
    src/push_button.cpp
    src/radiation_setup_widget.cpp
    src/remove_entity_dialog.cpp
    src/rename_model_dialog.cpp
    src/report_dialog.cpp
    src/report_generator.cpp
    src/results_variable_selector.cpp
    src/results_variable_tree.cpp
    src/scalar_field_dialog.cpp
    src/session.cpp
    src/session_entity_id.cpp
    src/session_node_id.cpp
    src/solver_manager.cpp
    src/solver_setup_checker.cpp
    src/solver_start_dialog.cpp
    src/solver_task.cpp
    src/solver_task_id.cpp
    src/stream_line_dialog.cpp
    src/sub_window.cpp
    src/sub_window_manager.cpp
    src/text_edit_widget.cpp
    src/time_solver_setup_widget.cpp
    src/usage_info.cpp
    src/value_line_edit.cpp
    src/value_scale.cpp
    src/value_set_generator_dialog.cpp
    src/value_table.cpp
    src/variable_selector.cpp
    src/variable_value_edit.cpp
    src/vector_field_dialog.cpp
    src/video_encoder.cpp
    src/video_settings.cpp
    src/video_settings_dialog.cpp

    src/action.h
    src/action_list.h
    src/application.h
    src/application_settings.h
    src/application_settings_dialog.h
    src/application_settings_widget.h
    src/bc_manager_tree.h
    src/bc_tree.h
    src/bool_difference_dialog.h
    src/bool_intersection_dialog.h
    src/bool_union_dialog.h
    src/break_intersected_elements_dialog.h
    src/central_tab_widget.h
    src/clipping_plane_widget.h
    src/coarsen_surface_dialog.h
    src/color.h
    src/color_combo_box.h
    src/color_scale.h
    src/color_scale_combo_box.h
    src/component_edit_dialog.h
    src/component_edit_table.h
    src/convergence_graph_dialog.h
    src/convergence_graph_object.h
    src/convergence_graph_widget.h
    src/cut_dialog.h
    src/direction_widget.h
    src/document_generator.h
    src/document_tree.h
    src/draw_engine.h
    src/draw_engine_circle.h
    src/draw_engine_cylinder.h
    src/draw_engine_ellipse.h
    src/draw_engine_hexahedron.h
    src/draw_engine_input.h
    src/draw_engine_line.h
    src/draw_engine_object.h
    src/draw_engine_point.h
    src/draw_engine_quadrilateral.h
    src/draw_engine_raw.h
    src/draw_engine_sphere.h
    src/draw_engine_tetrahedron.h
    src/draw_engine_triangle.h
    src/draw_input_tree.h
    src/draw_input_widget.h
    src/drop_results_dialog.h
    src/ec_manager_tree.h
    src/ec_tree.h
    src/find_sliver_elements_dialog.h
    src/fix_sliver_elements_dialog.h
    src/geometry_rotate_widget.h
    src/geometry_scale_widget.h
    src/geometry_transform_input.h
    src/geometry_transform_widget.h
    src/geometry_translate_widget.h
    src/gl_acion_event.h
    src/gl_arrow.h
    src/gl_axis.h
    src/gl_cut_plane.h
    src/gl_dimension.h
    src/gl_display_properties.h
    src/gl_display_properties_dialog.h
    src/gl_element.h
    src/gl_element_base.h
    src/gl_element_group.h
    src/gl_entity.h
    src/gl_entity_list.h
    src/gl_functions.h
    src/gl_grid.h
    src/gl_interpolated_element.h
    src/gl_interpolated_entity.h
    src/gl_line.h
    src/gl_list.h
    src/gl_model_list.h
    src/gl_node.h
    src/gl_object.h
    src/gl_point.h
    src/gl_rotation_sphere.h
    src/gl_scalar_field.h
    src/gl_simplex.h
    src/gl_simplex_point.h
    src/gl_simplex_polygon.h
    src/gl_simplex_segment.h
    src/gl_simplex_tetrahedra.h
    src/gl_text_renderer.h
    src/gl_text_renderer_item.h
    src/gl_texture.h
    src/gl_vector_field.h
    src/gl_widget.h
    src/graph_data.h
    src/graph_dialog.h
    src/graph_object.h
    src/graph_widget.h
    src/help_dialog.h
    src/help_gl_action_event_widget.h
    src/ic_manager_tree.h
    src/ic_tree.h
    src/iso_dialog.h
    src/light_properties_widget.h
    src/local_direction_widget.h
    src/main_window.h
    src/mark_entity_dialog.h
    src/material_manager_tree.h
    src/material_property_add_dialog.h
    src/material_property_edit_dialog.h
    src/material_property_edit_table.h
    src/material_property_line_edit.h
    src/material_tree.h
    src/matrix_solver_config_dialog.h
    src/merge_entity_dialog.h
    src/merge_near_nodes_dialog.h
    src/mesh_generator.h
    src/mesh_generator_dialog.h
    src/mesh_setup_widget.h
    src/modal_setup_widget.h
    src/model.h
    src/model_action.h
    src/model_action_input.h
    src/model_entity_display_properties_tree.h
    src/model_entity_geometric_properties_tree.h
    src/model_io.h
    src/model_records_selector.h
    src/model_records_tree.h
    src/model_statistics_dialog.h
    src/model_tree.h
    src/model_tree_simple.h
    src/monitoring_point_graph_dialog.h
    src/monitoring_points_dialog.h
    src/move_node_dialog.h
    src/new_model_dialog.h
    src/pick_details_tree.h
    src/pick_item.h
    src/pick_list.h
    src/pick_value.h
    src/point_inside_surface_dialog.h
    src/position_widget.h
    src/problem_selector_dialog.h
    src/problem_selector_tree.h
    src/problem_task_dialog.h
    src/problem_task_tree.h
    src/problem_tree.h
    src/push_button.h
    src/radiation_setup_widget.h
    src/remove_entity_dialog.h
    src/rename_model_dialog.h
    src/report_dialog.h
    src/report_generator.h
    src/results_variable_selector.h
    src/results_variable_tree.h
    src/scalar_field_dialog.h
    src/session.h
    src/session_entity_id.h
    src/session_node_id.h
    src/solver_manager.h
    src/solver_setup_checker.h
    src/solver_start_dialog.h
    src/solver_task.h
    src/solver_task_id.h
    src/stream_line_dialog.h
    src/sub_window.h
    src/sub_window_manager.h
    src/text_edit_widget.h
    src/time_solver_setup_widget.h
    src/usage_info.h
    src/value_line_edit.h
    src/value_scale.h
    src/value_set_generator_dialog.h
    src/value_table.h
    src/variable_data.h
    src/variable_selector.h
    src/variable_value_edit.h
    src/vector_field_dialog.h
    src/video_encoder.h
    src/video_settings.h
    src/video_settings_dialog.h
)

add_dependencies(fea range-base-lib range-model-lib range-solver-lib range-cloud-lib range-gui-lib)

target_link_libraries(fea
    PRIVATE
        range-gui-lib
        range-solver-lib
        OpenGL::GL
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Multimedia
        Qt6::PrintSupport
        common_defines
)

set_target_properties(fea
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "${PROJECT_PACKAGE_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_SHORT_NAME}"
)

if (WIN32)
    set(WIN32_ICON "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/${PROJECT_PACKAGE_NAME}.ico")
    set(ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/fea.rc")
    file(WRITE "${ICON_RC}"
        "IDI_ICON1 ICON \"${WIN32_ICON}\"\n"
    )
    set_source_files_properties("${ICON_RC}" PROPERTIES LANGUAGE RC)
    target_sources(fea PRIVATE "${ICON_RC}")
elseif (APPLE)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/${PROJECT_PACKAGE_NAME}.icns")
    set_source_files_properties("${APP_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(fea PRIVATE "${APP_ICON}")

    add_custom_command(TARGET fea POST_BUILD
        COMMAND /usr/libexec/PlistBuddy -c "Set :CFBundleName '${PROJECT_SHORT_NAME}'" "$<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Info.plist" || /usr/libexec/PlistBuddy -c "Add :CFBundleName string '${PROJECT_SHORT_NAME}'" "$<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Info.plist"
        COMMAND /usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName '${PROJECT_SHORT_NAME}'" "$<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Info.plist" || /usr/libexec/PlistBuddy -c "Add :CFBundleDisplayName string '${PROJECT_SHORT_NAME}'" "$<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Info.plist"
        COMMENT "Set CFBundleName and CFBundleDisplayName"
    )
endif()

qt_add_resources(fea icons-application
    PREFIX "/icons/application"
    FILES
        pixmaps/range-application_settings.svg
        pixmaps/range-display_preferences.svg
        pixmaps/range-quit.svg
        pixmaps/range-startup.svg
)

qt_add_resources(fea icons-action
    PREFIX "/icons/action"
    FILES
        pixmaps/range-cancel.svg
        pixmaps/range-clear.svg
        pixmaps/range-close.svg
        pixmaps/range-ok.svg
        pixmaps/range-undo.svg
)

qt_add_resources(fea icons-logos
    PREFIX "/icons/logos"
    FILES
        pixmaps/range-app.svg
        pixmaps/range-fea.svg
)

qt_add_resources(fea icons-file
    PREFIX "/icons/file"
    FILES
        pixmaps/range-session.svg
        pixmaps/range-session_new.svg
        pixmaps/range-session_open.svg
        pixmaps/range-session_save.svg
        pixmaps/range-session_save_as.svg
        pixmaps/range-session_close.svg
        pixmaps/range-new.svg
        pixmaps/range-open.svg
        pixmaps/range-material_new.svg
        pixmaps/range-material_import.svg
        pixmaps/range-material_delete.svg
        pixmaps/range-model_new.svg
        pixmaps/range-model_open.svg
        pixmaps/range-model_save.svg
        pixmaps/range-model_save_as.svg
        pixmaps/range-model_close.svg
        pixmaps/range-model_export_msh.svg
        pixmaps/range-model_export_raw.svg
        pixmaps/range-model_export_stl.svg
        pixmaps/range-model_rename.svg
        pixmaps/range-model_drop_results.svg
        pixmaps/range-resetO.svg
        pixmaps/range-resetX.svg
        pixmaps/range-resetY.svg
        pixmaps/range-resetZ.svg
        pixmaps/range-save.svg
        pixmaps/range-screenshot.svg
        pixmaps/range-remove.svg
        pixmaps/range-refresh.svg
        pixmaps/range-download.svg
        pixmaps/range-qt.svg
        pixmaps/range-important.svg
        pixmaps/range-help.svg
        pixmaps/range-fea.ico
        pixmaps/range-information.svg
        pixmaps/range-severe.svg
)

qt_add_resources(fea icons-problem
    PREFIX "/icons/problem"
    FILES
        pixmaps/range-problem_task_flow.svg
        pixmaps/range-problem_matrix_solver.svg
        pixmaps/range-problem_monitoring_points.svg
        pixmaps/range-problem_reset.svg
)

qt_add_resources(fea icons-solver
    PREFIX "/icons/solver"
    FILES
        pixmaps/range-solver-start.svg
        pixmaps/range-solver-stop.svg
        pixmaps/range-solver-kill.svg
)

qt_add_resources(fea icons-colorscale
    PREFIX "/icons/colorscale"
    FILES
        pixmaps/range-scale-01-RGB.png
        pixmaps/range-scale-02-RGB_S.png
        pixmaps/range-scale-03-RGB_SO.png
        pixmaps/range-scale-04-RGB_R.png
        pixmaps/range-scale-05-RGB_RS.png
        pixmaps/range-scale-06-RGB_RSO.png
        pixmaps/range-scale-07-Grayscale.png
        pixmaps/range-scale-08-Grayscale_S.png
        pixmaps/range-scale-09-Grayscale_SO.png
        pixmaps/range-scale-10-Grayscale_R.png
        pixmaps/range-scale-11-Grayscale_RS.png
        pixmaps/range-scale-12-Grayscale_RSO.png
        pixmaps/range-scale-13-RWB.png
        pixmaps/range-scale-14-RWB_R.png
        pixmaps/range-scale-15-Fire.png
        pixmaps/range-scale-16-Fire_R.png
        pixmaps/range-scale-17-WhiteBlue.png
        pixmaps/range-scale-18-WhiteBlue_R.png
        pixmaps/range-scale-19-WhiteGreen.png
        pixmaps/range-scale-20-WhiteGreen_R.png
        pixmaps/range-scale-21-WhiteRed.png
        pixmaps/range-scale-22-WhiteRed_R.png
        pixmaps/range-scale-23-Value_limits.png
        pixmaps/range-scale-24-Value_limits_R.png
)

qt_add_resources(fea icons-media
    PREFIX "/icons/media"
    FILES
        pixmaps/range-play_play.svg
        pixmaps/range-play_pause.svg
        pixmaps/range-play_last.svg
        pixmaps/range-play_forward.svg
        pixmaps/range-play_first.svg
        pixmaps/range-play_backward.svg
        pixmaps/range-play_record.svg
)

qt_add_resources(fea icons-mime
    PREFIX "/icons/mime"
    FILES
        pixmaps/range-model.svg
        pixmaps/range-file-document.svg
)

qt_add_resources(fea icons-geometry
    PREFIX "/icons/geometry"
    FILES
        pixmaps/range-undo.svg
        pixmaps/range-redo.svg
        pixmaps/range-draw_geometry.svg
        pixmaps/range-draw_point.svg
        pixmaps/range-draw_line.svg
        pixmaps/range-draw_triangle.svg
        pixmaps/range-draw_quadrilateral.svg
        pixmaps/range-draw_circle.svg
        pixmaps/range-draw_ellipse.svg
        pixmaps/range-draw_tetrahedron.svg
        pixmaps/range-draw_hexahedron.svg
        pixmaps/range-draw_cylinder.svg
        pixmaps/range-draw_sphere.svg
        pixmaps/range-draw_raw.svg
        pixmaps/range-entity_point.svg
        pixmaps/range-entity_line.svg
        pixmaps/range-entity_line_from_surface.svg
        pixmaps/range-entity_surface.svg
        pixmaps/range-entity_surface_mark.svg
        pixmaps/range-entity_surface_swap_element_normal.svg
        pixmaps/range-entity_surface_swap_normals.svg
        pixmaps/range-entity_surface_sync_normals.svg
        pixmaps/range-entity_surface_close_hole.svg
        pixmaps/range-entity_surface_coarsen.svg
        pixmaps/range-entity_surface_find_intersected.svg
        pixmaps/range-entity_surface_break_intersected.svg
        pixmaps/range-entity_surface_bool_operation.svg
        pixmaps/range-entity_surface_bool_difference.svg
        pixmaps/range-entity_surface_bool_intersection.svg
        pixmaps/range-entity_surface_bool_union.svg
        pixmaps/range-entity_volume.svg
        pixmaps/range-entity_cut.svg
        pixmaps/range-entity_cut_create.svg
        pixmaps/range-entity_cut_edit.svg
        pixmaps/range-entity_iso.svg
        pixmaps/range-entity_iso_create.svg
        pixmaps/range-entity_iso_edit.svg
        pixmaps/range-entity_stream_line.svg
        pixmaps/range-entity_stream_line_create.svg
        pixmaps/range-entity_stream_line_edit.svg
        pixmaps/range-entity_scalar_field.svg
        pixmaps/range-entity_scalar_field_create.svg
        pixmaps/range-entity_scalar_field_edit.svg
        pixmaps/range-entity_vector_field.svg
        pixmaps/range-entity_vector_field_create.svg
        pixmaps/range-entity_vector_field_edit.svg
        pixmaps/range-entity_merge.svg
        pixmaps/range-entity_remove.svg
        pixmaps/range-sliver_find.svg
        pixmaps/range-sliver_fix.svg
        pixmaps/range-generate_volume_mesh.svg
        pixmaps/range-geometry_transform.svg
        pixmaps/range-element_create.svg
        pixmaps/range-element_remove.svg
        pixmaps/range-merge_near_nodes.svg
        pixmaps/range-move_node.svg
        pixmaps/range-node_remove.svg
        pixmaps/range-node_remove_and_close.svg
)

qt_add_resources(fea icons-report
    PREFIX "/icons/report"
    FILES
        pixmaps/range-report.svg
        pixmaps/range-report_model_statistics.svg
        pixmaps/range-report_monitoring_point.svg
        pixmaps/range-report_solver_log.svg
        pixmaps/range-report-convergence.svg
)

qt_add_resources(fea icons-general
    PREFIX "/icons/general"
    FILES
        pixmaps/range-generic_action.svg
)

qt_add_translations(fea
    TS_FILES
        translations/en.ts
        translations/sk.ts
    RESOURCE_PREFIX /i18n/fea
)
