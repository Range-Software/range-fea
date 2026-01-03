# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Range FEA is a Finite Element Analysis (FEA) desktop application for computer-aided engineering. It provides a Qt6-based GUI for mesh generation, simulation setup, and visualization of engineering analysis results.

## Build Commands

```bash
# Initialize submodules (first time only)
git submodule init && git submodule update --remote

# Install prerequisites (Linux/macOS, requires sudo)
sudo ./src/range-build-tools/prereqs.sh

# Configure and build (Release)
cmake -S src -B build-Release && cmake --build build-Release --parallel

# Build Debug
cmake -S src -B build-Debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build-Debug --parallel

# Create installation packages
cmake --build build-Release --target package
```

## Architecture

### Library Dependency Chain

```
TetGen (mesh generation)
    ↓
range-base-lib (rbl_*) - Core utilities: vectors, matrices, logging, job management
    ↓
range-model-lib (rml_*) - FEA model: elements, nodes, materials, boundary conditions
    ↓
range-solver-lib (rsolver*) - Numerical solvers: heat, fluid, stress, acoustics, etc.
    ↓
range-cloud-lib - Cloud integration services
    ↓
range-gui-lib - Qt widgets and UI components
    ↓
├── fea (Qt6 GUI application)
└── fea-solver (CLI solver executable)
```

### Source Organization

- `src/range-base-lib/` - Base library with math primitives (RBL prefix)
- `src/range-model-lib/` - Model definitions and file I/O (RML prefix)
- `src/range-solver-lib/` - FEA solvers for various physics (RSolver prefix)
- `src/range-cloud-lib/` - Cloud service integration
- `src/range-gui-lib/` - Shared Qt GUI components
- `src/fea/` - Main GUI application
- `src/fea-solver/` - Standalone CLI solver
- `src/TetGen/` - Tetrahedral mesh generator (third-party)

### Key Patterns

- Libraries use Qt6 (requires 6.10+)
- OpenMP for parallelization
- OpenGL for 3D visualization
- Static library linking (BUILD_SHARED_LIBS=OFF)
- C++17 standard

### Solver Types

Located in `range-solver-lib`:
- `rsolverheat.cpp` - Heat transfer
- `rsolverfluid.cpp` - Fluid dynamics
- `rsolverstress.cpp` - Structural stress
- `rsolveracoustic.cpp` - Acoustics
- `rsolverelectrostatics.cpp` - Electrostatics
- `rsolvermagnetostatics.cpp` - Magnetostatics
- `rsolverradiativeheat.cpp` - Radiative heat transfer

### GUI Components

The `fea` application uses:
- `gl_widget.cpp` - OpenGL rendering widget
- `model.cpp` / `session.cpp` - Model and session management
- `draw_engine_*.cpp` - Geometry drawing tools
- `solver_manager.cpp` - Solver process management
