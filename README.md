# Range Software - FEA

Range FEA is an open-source desktop application for computer-aided engineering using the Finite Element Method. It provides an integrated environment for mesh generation, simulation setup, and 3D visualisation of results across a range of physics domains — including structural stress, heat transfer, fluid dynamics, acoustics, electrostatics, and magnetostatics.

The application is built on Qt6 and ships with a standalone CLI solver (`fea-solver`) alongside the main GUI (`fea`). Mesh generation uses the embedded TetGen library for automatic tetrahedral meshing.

![Range Software](http://www.range-software.com/files/common/Range3-CFD.png)

## Help and tutorials

1. [General help](./src/fea/help/general/en/general_index.md)
2. [Tutorials](./src/fea/help/tutorials/en/tutorial_index.md)

## Prepare build environment
Initialize all submodules
```
git submodule update --init --recursive
```
The following command will attempt to download and install all required packages, therefore it must be executed under a privileged (root) user
```
sudo ./src/range-build-tools/prereqs.sh
```
_NOTE: In case your OS does not provide you with Qt version 6.8 or newer download and install it from [https://www.qt.io/download/](https://www.qt.io/download/)._
## Build
```
cmake -S src -B build-Release && \
cmake --build build-Release --parallel
```
## Create installation packages and installers
```
cmake --build build-Release --target package
```
## Installation using generated installers
### Windows

### MacOS
#### Drag & Drop
```
open './build-Release/range-fea-1.2.1-darwin-arm64.dmg'
```
### Linux
#### Self extracting Tar GZip
```
./build-Release/range-fea-1.2.1-linux-x86_64.sh
```
#### Debian DEB package
```
sudo apt install ./build-Release/range-fea-1.2.1-linux-x86_64.deb
```
#### RedHat RPM package
```
sudo dnf install ./build-Release/range-fea-1.2.1-linux-x86_64.rpm
```

## Rendering backends

The GUI can render the 3D view either through the legacy OpenGL pipeline or through
Qt's Rendering Hardware Interface, which runs on Metal, Vulkan, Direct3D or OpenGL.

The backend is configured in _Application settings → General → Rendering backend_,
together with the graphics API used by the `rhi` backend. Both are stored per user
and applied the next time the application starts.

The stored setting can be overridden for a single run from the command line:

```
fea --render-backend=opengl                  # legacy OpenGL pipeline (default)
fea --render-backend=rhi                     # Qt Rendering Hardware Interface
fea --render-backend=rhi --rhi-api=metal     # auto|opengl|vulkan|metal|d3d11|d3d12|null
```

A command line override does not change the stored setting.

Both backends share the whole scene traversal, so they render the same geometry.
Two fixed-function features have no QRhi equivalent and are therefore ignored by the
`rhi` backend: line stipple (dashed grid and axis lines are drawn solid) and line
widths greater than one on APIs without wide-line support, such as Metal.

## Download
To download already built binaries please visit http://range-software.com

## Powered by

* Qt - https://www.qt.io/
* TetGen - http://tetgen.org
