# Create a static library
qt_add_library(TetGen
    STATIC
        predicates.cxx
        tetgen.cxx

        tetgen.h
)

# Compiler definitions
target_compile_definitions(TetGen
    PRIVATE
        TETLIBRARY
)

# Include directories
target_include_directories(TetGen
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TetGen
    PRIVATE
        common_defines
)
