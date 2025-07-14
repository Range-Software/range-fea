qt_add_executable(fea-solver
    src/application.cpp
    src/command_processor.cpp
    src/main.cpp
    src/main_task.cpp
    src/solver_input.cpp
    src/solver_task.cpp

    src/application.h
    src/command_processor.h
    src/main_task.h
    src/solver_input.h
    src/solver_task.h
)

add_dependencies(fea-solver range-base-lib range-model-lib range-solver-lib)

target_link_libraries(fea-solver
    PRIVATE
        OpenMP::OpenMP_CXX
        Qt6::Core
        Qt6::Network
        common_defines
        range-base-lib
        range-model-lib
        range-solver-lib
)
