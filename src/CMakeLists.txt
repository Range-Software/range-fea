cmake_minimum_required(VERSION 3.21)
project(RangeFea)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Config.cmake)

set(DEBUG_EXT "")
set(PROJECT_HUMAN_REL_NAME "${PROJECT_HUMAN_NAME}")
set(PROJECT_SHORT_REL_NAME "${PROJECT_SHORT_NAME}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_EXT "_debug")
    add_compile_definitions(DEBUG)
    set(PROJECT_HUMAN_REL_NAME "${PROJECT_HUMAN_REL_NAME} (${CMAKE_BUILD_TYPE})")
    set(PROJECT_SHORT_REL_NAME "${PROJECT_SHORT_REL_NAME} (${CMAKE_BUILD_TYPE})")
endif()

set(BUILD_SHARED_LIBS OFF)

# Open MP configuration -------------------------------------------------------
if(APPLE AND (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    execute_process(COMMAND brew --prefix libomp
                    OUTPUT_VARIABLE OMP_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT OMP_PREFIX)
        message(FATAL_ERROR "brew --prefix libomp failed. Is libomp installed?")
    endif()

    message(STATUS "libomp prefix: ${OMP_PREFIX}")

    # macOS + Clang + possibly Homebrew or MacPorts
    find_path(OpenMP_INCLUDES
        NAMES omp.h
        PATHS "${OMP_PREFIX}/include"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED
        DOC "Path to OpenMP headers"
    )
    find_library(OpenMP_LIBRARY
        NAMES omp
        PATHS "${OMP_PREFIX}/lib"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED
        DOC "Path to OpenMP library"
    )

    if(OpenMP_INCLUDES AND OpenMP_LIBRARY)
        message(STATUS "Found OpenMP headers at: ${OpenMP_INCLUDES}")
        message(STATUS "Found OpenMP library at: ${OpenMP_LIBRARY}")

        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${OpenMP_INCLUDES}")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OpenMP_INCLUDES}")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY ${OpenMP_LIBRARY})
    else()
        message(WARNING "OpenMP not found. Please install libomp (e.g., brew install libomp).")
        if(NOT OpenMP_INCLUDES)
            message(WARNING "OpenMP header not found.")
        endif()
        if(NOT OpenMP_LIBRARY)
            message(WARNING "OpenMP library not found.")
        endif()
    endif()
endif()
# Open MP configuration - End  ------------------------------------------------

find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Concurrent HttpServer Network Widgets OpenGL OpenGLWidgets Multimedia PrintSupport LinguistTools)
qt_standard_project_setup()

add_library(common_defines INTERFACE)
target_compile_definitions(common_defines
    INTERFACE
        VERSION="${PROJECT_VERSION}"
        YEAR=${PROJECT_YEAR}
        PACKAGENAME="${PROJECT_PACKAGE_NAME}"
        NAME="${PROJECT_HUMAN_REL_NAME}"
        SHORTNAME="${PROJECT_SHORT_NAME}"
        FAMILYNAME="${PROJECT_FAMILY_NAME}"
        TITLE="${PROJECT_TITLE}"
        DESCRIPTION="${PROJECT_DESCRIPTION}"
        AUTHOR="${PROJECT_AUTHOR}"
        EMAIL="${PROJECT_EMAIL}"
        WWW="${PROJECT_WWW}"
        WWWDOMAIN="${PROJECT_WWW_DOMAIN}"
)

function(set_debug_suffix_if_needed target_name)
    if(CMAKE_CONFIGURATION_TYPES)  # Multi-config (e.g. Visual Studio, Xcode)
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME_DEBUG "${target_name}${DEBUG_EXT}"
            OUTPUT_NAME_RELEASE "${target_name}"
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")  # Single-config (e.g. Make, Ninja)
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME "${target_name}${DEBUG_EXT}"
        )
    else()
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME "${target_name}"
        )
    endif()
endfunction()

set(all_targets TetGen range-base-lib range-model-lib range-solver-lib range-cloud-lib range-gui-lib fea-solver fea)

foreach(tgt IN LISTS all_targets)
    add_subdirectory(${tgt})
    set_debug_suffix_if_needed(${tgt})
endforeach()

# Deploy Qt -------------------------------------------------------------------

get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

set(_qt_deploy_dir "$<TARGET_FILE_DIR:${PROJECT_TARGET_NAME}>/qtlib")

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    set(DEPLOY_COMMAND
        ${WINDEPLOYQT_EXECUTABLE}
        $<TARGET_FILE:${PROJECT_TARGET_NAME}>
        --dir ${_qt_deploy_dir}
    )
elseif(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
    set(DEPLOY_COMMAND
        ${MACDEPLOYQT_EXECUTABLE}
        $<TARGET_BUNDLE_DIR:${PROJECT_TARGET_NAME}>
    )
    if(DEFINED APPLE_ID)
        set(DEPLOY_COMMAND ${DEPLOY_COMMAND} -codesign=${APPLE_ID})
    endif()
else()
    get_filename_component(_qt_dir "${_qt_bin_dir}" DIRECTORY)
    set(DEPLOY_COMMAND
        ${CMAKE_SOURCE_DIR}/range-build-tools/linux_deploy_qt.sh
        --executable=$<TARGET_FILE:${PROJECT_TARGET_NAME}>
        --install-to=${_qt_deploy_dir}
        --only-qt
        --qt-path=${_qt_dir}
    )
endif()

add_custom_target(deploy-qt ALL
    COMMAND ${DEPLOY_COMMAND}
    DEPENDS fea
    COMMENT "Deploying QT libraries"
    VERBATIM
)

# Put fea-solver into FEA.app/Contents/MacOS on macOS -------------------------
if(APPLE)
    # This copies the already-built fea-solver next to the main executable inside the bundle
    add_custom_target(embed-fea-solver ALL
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:fea-solver>
                $<TARGET_FILE_DIR:fea>
        DEPENDS fea fea-solver
        COMMENT "Embedding fea-solver into FEA.app/Contents/MacOS"
        VERBATIM
    )
 
    # Make sure we copy the solver BEFORE running macdeployqt/codesign
    add_dependencies(deploy-qt embed-fea-solver)
endif()

# Create custom .DS_Store on MacOS --------------------------------------------
if(APPLE)
    set(APP_DIR_OLD "$<TARGET_BUNDLE_DIR:${PROJECT_TARGET_NAME}>")
    set(APP_DIR_TMP "${CMAKE_BINARY_DIR}/${PROJECT_TARGET_NAME}/${PROJECT_SHORT_REL_NAME}_tmp.app")
    set(APP_DIR_NEW "${CMAKE_BINARY_DIR}/${PROJECT_TARGET_NAME}/${PROJECT_SHORT_REL_NAME}.app")
    set(DSSTORE_FILE "${CMAKE_BINARY_DIR}/${PROJECT_TARGET_NAME}.dsstore")

    add_custom_target(app-rename ALL
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${APP_DIR_TMP}
        COMMAND ${CMAKE_COMMAND} -E rename ${APP_DIR_OLD} ${APP_DIR_TMP}
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${APP_DIR_NEW}
        COMMAND ${CMAKE_COMMAND} -E rename ${APP_DIR_TMP} ${APP_DIR_NEW}
        DEPENDS deploy-qt
        COMMENT "Renaming ${APP_DIR_OLD} bundle to ${APP_DIR_NEW}"
        VERBATIM
    )

    add_custom_target(deploy-data ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/help ${APP_DIR_NEW}/Contents/Resources/help
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/data ${APP_DIR_NEW}/Contents/Resources/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/doc ${APP_DIR_NEW}/Contents/Resources/doc
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/materials ${APP_DIR_NEW}/Contents/Resources/materials
        COMMAND ${CMAKE_COMMAND} -E copy           ${CMAKE_BINARY_DIR}/LICENSE.txt ${APP_DIR_NEW}/Contents/Resources/doc/LICENSE.txt
        DEPENDS app-rename
        COMMENT "Deploying application data"
        VERBATIM
    )

    add_custom_target(gen-dsstore ALL
        COMMAND "${CMAKE_SOURCE_DIR}/range-build-tools/create_dsstore.sh"
                --app-bundle=${APP_DIR_NEW}
                --background=${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/pixmaps/range-dmg_background.png
                --icon=${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/pixmaps/${PROJECT_PACKAGE_NAME}.icns
                --ds-store=${DSSTORE_FILE}
        DEPENDS app-rename
        COMMENT "Generating custom .DS_Store for Drag-and-Drop DMG"
        VERBATIM
    )
endif()

# Install ---------------------------------------------------------------------

if(WIN32)
    install(TARGETS fea DESTINATION bin COMPONENT Application)
    install(TARGETS fea-solver DESTINATION bin COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/help/ DESTINATION help COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/data/ DESTINATION data COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/doc/ DESTINATION doc COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/materials/ DESTINATION materials COMPONENT Application)
    install(DIRECTORY ${_qt_deploy_dir}/ DESTINATION bin COMPONENT Application)
    install(FILES ${CMAKE_BINARY_DIR}/LICENSE.txt DESTINATION doc COMPONENT Application)
elseif(APPLE)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_TARGET_NAME}/${PROJECT_SHORT_REL_NAME}.app DESTINATION . USE_SOURCE_PERMISSIONS COMPONENT Application)
    install(TARGETS fea-solver BUNDLE DESTINATION bin COMPONENT Application)
else()
    set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_FAMILY_NAME}/${PROJECT_SHORT_REL_NAME}" CACHE PATH "Install path prefix" FORCE)

    install(TARGETS fea DESTINATION bin COMPONENT Application)
    install(TARGETS fea-solver DESTINATION bin COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/help/ DESTINATION help COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/data/ DESTINATION data COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/doc/ DESTINATION doc COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/materials/ DESTINATION materials COMPONENT Application)
    install(DIRECTORY ${_qt_deploy_dir}/ DESTINATION . COMPONENT Application)
    install(FILES ${CMAKE_BINARY_DIR}/LICENSE.txt DESTINATION doc COMPONENT Application)
endif()

# CPack -----------------------------------------------------------------------

set(CPACK_COMPONENTS_ALL
    Application
)

set(CPACK_PACKAGE_NAME "${PROJECT_PACKAGE_NAME}${DEBUG_EXT}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_RELEASE "1")
set(CPACK_PACKAGE_CONTACT "${PROJECT_EMAIL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_WWW}")
set(CPACK_SYSTEM_NAME "${CMAKE_HOST_SYSTEM_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if(APPLE)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.rtf")
else()
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
endif()
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/../README.md")

if(WIN32)
    set(CPACK_GENERATOR "IFW;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "IFW;ZIP;TGZ;STGZ;DEB;RPM")
endif()

# DEB
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27)")

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
#set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.27")

# DMG
if(APPLE)
    set(DMG_ICON "${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/pixmaps/${PROJECT_PACKAGE_NAME}.icns")

    set(CPACK_BUNDLE_ICON "${DMG_ICON}")
    set(CPACK_PACKAGE_ICON "${DMG_ICON}")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_HUMAN_REL_NAME} Installer")
    set(CPACK_DMG_VOLUME_ICON "${DMG_ICON}")
    set(CPACK_DMG_FORMAT "UDZO")
    set(CPACK_DMG_CREATE_APPLICATIONS_LINK "ON")
    set(CPACK_DMG_DS_STORE "${DSSTORE_FILE}")
endif()

# IFW
set(CPACK_IFW_PACKAGE_TITLE "${PROJECT_TITLE}")
set(CPACK_IFW_PACKAGE_NAME "${PROJECT_PACKAGE_NAME}")
set(CPACK_IFW_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_IFW_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
#set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME "MaintenanceTool")
set(CPACK_IFW_PACKAGE_PUBLISHER "${PROJECT_AUTHOR}")
set(CPACK_IFW_PACKAGE_URL "${PROJECT_WWW}")
set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY "${PROJECT_FAMILY_NAME}/${PROJECT_SHORT_REL_NAME}")
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/pixmaps/${PROJECT_PACKAGE_NAME}.png")
set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS "ON")
set(CPACK_IFW_TARGET_DIRECTORY "@HomeDir@/${PROJECT_FAMILY_NAME}/${PROJECT_SHORT_REL_NAME}/${PROJECT_VERSION}")
#set(CPACK_IFW_PACKAGE_ADMIN_TARGET_DIRECTORY "ON")

if(WIN32)
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/pixmaps/${PROJECT_PACKAGE_NAME}.ico")
elseif(APPLE)
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/${PROJECT_TARGET_NAME}/pixmaps/${PROJECT_PACKAGE_NAME}.icns")
endif()

set(IFW_QS_TARGET_FILE_BASE "${PROJECT_TARGET_NAME}${DEBUG_EXT}")
set(IFW_QS_TARGET_NAME_BASE "${PROJECT_SHORT_REL_NAME}")

# Convert files ---------------------------------------------------------------

file(READ "${CMAKE_SOURCE_DIR}/../LICENSE" LICENSE_TEXT)

configure_file(
    ${CMAKE_SOURCE_DIR}/range-build-tools/LICENSE.txt.in
    ${CMAKE_BINARY_DIR}/LICENSE.txt
    @ONLY
)

configure_file(
    ${CMAKE_SOURCE_DIR}/range-build-tools/installscript.qs.in
    ${CMAKE_BINARY_DIR}/installscript.qs
    @ONLY
)

if(APPLE)
    execute_process(
        COMMAND textutil -convert rtf -encoding UTF-8 ${CMAKE_BINARY_DIR}/LICENSE.txt -output ${CMAKE_BINARY_DIR}/LICENSE.rtf
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to generate LICENSE.rtf")
    endif()
endif()

# -----------------------------------------------------------------------------

include(CPackIFW)

cpack_ifw_configure_component(Application
    DISPLAY_NAME "${PROJECT_SHORT_REL_NAME}"
    LICENSES "${PROJECT_FAMILY_NAME} ${PROJECT_SHORT_REL_NAME} License" "${CPACK_RESOURCE_FILE_LICENSE}"
    SCRIPT "${CMAKE_BINARY_DIR}/installscript.qs"
    DEFAULT "TRUE"
)

include(InstallRequiredSystemLibraries)
include(CPack)
