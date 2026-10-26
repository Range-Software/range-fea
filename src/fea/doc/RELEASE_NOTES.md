## Version 1.3.1

### Improvements

#### Application

- New **File > Folders** submenu opens the application folders - *Certificates*,
  *Data*, *Documents*, *Logs*, *Materials*, *Sessions* and *Temporary files* - in the
  file browser of the operating system (Finder on macOS, File Explorer on Windows, the
  file manager on Linux)
- Help pages of the new actions in all languages

### Bug fixes

- The Mac App Store version could not write to its folders. They are kept in
  `Documents/Range Software/FEA`, which the App Sandbox does not allow to be written to.
  The sandboxed version now keeps them in `Library/Application Support/Range Software/FEA`
  inside its container (`~/Library/Containers/com.range-software.fea/Data`); other
  versions keep using the Documents folder. Use **File > Folders** to find them
- The question about clearing results of problems removed from the task flow and the
  "+ N *more*" label of hidden entities in the 3D view are translated to all languages

### Submodules

- range-gui-lib @ v1.3.0

---

## Version 1.3.0

### Compatibility

- The model file format is raised to **1.4.0**. Models saved by this version
  cannot be opened by earlier ones; older models open with default values
- **Results of existing models change** for transient stress (now dynamic),
  thermal stress, surface elements with a thickness, electro-statics and
  magneto-statics. See the items below for details

### Improvements

#### Stress analysis

- **Transient stress analysis is now dynamic.** It solves
  `M*u'' + C*u' + K*u = f(t)` with Newmark / Hilber-Hughes-Taylor time
  integration, so inertia, stress waves, vibration and dynamic amplification are
  reproduced. Before, it was a first order march that only relaxed towards the
  static answer. **Transient stress models from earlier versions give different,
  dynamic results**; select *Quasi-static* to solve a static problem at every
  time step instead
- New **Transient stress analysis setup** group, active only while the time
  solver is enabled: analysis type (*Dynamic* or *Quasi-static*), numerical
  damping (HHT alpha) and lumped mass matrix
- New optional *Rayleigh mass damping coefficient* and *Rayleigh stiffness
  damping coefficient* material properties, a *Structural velocity* initial
  condition, and *Structural velocity* and *Structural acceleration* results.
  The *Displacement* initial condition is now applied as well
- Displacement constraints meeting at one node are combined, so *Displacement*
  and *Roller displacement* can act on the same node. Conflicting values stop
  the run with an error naming the node
- Individual *Displacement* components can be switched off with a check box
- The local direction of *Normal displacement* and *Roller displacement* can be
  entered for surfaces and lines with **Use entered local direction**

#### Acoustics

- **Acoustics** is available again, with transient or harmonic analysis set up
  in the new **Acoustic analysis setup** group: frequency sweep and reference
  pressure for the sound pressure level
- New *Acoustic impedance* condition, absorption coefficient of the *Absorbing
  boundary*, *Velocity potential / s* initial condition, *Speed of sound* and
  *Acoustic damping factor* material properties, and sound pressure level,
  intensity and phase results
- A harmonic analysis writes one record per frequency, so the records tree steps
  through the frequency sweep

#### Fluids and contaminant dispersion

- Task groups have a **Convergence** value next to **# of iterations** (default
  `1e-5`). A group stops as soon as all of its tasks have converged, which
  shortens flow runs considerably; `0` runs every iteration as before
- New **Contaminant dispersion setup** in the `Problem` tab: *Diffusion
  coefficient* and *Maximum saturation*. With a saturation set, the source
  weakens as it is approached and *Relative saturation* is computed
- New contaminant boundary conditions *Particle flux (unit area)*, for emission
  or removal through a surface, and *Particle mass transfer*, for deposition or
  exchange with an ambient concentration

#### Flow through porous media

- **Flow through porous media** can be solved. The problem type was listed but
  had no solver behind it; it now computes steady Darcy flow of a fluid through
  a saturated porous body - groundwater, seepage through dams, filters, packed
  beds - with *Pressure* and *Velocity* (Darcy velocity) results
- Boundary conditions *Pressure (explicit)* for outlets, *Velocity (inflow)*
  and *Volumetric flow rate (inflow)* for a known inflow; faces without a
  condition are impermeable. Gravity follows the *Gravitational acceleration*
  environment condition, so a fluid at rest carries the hydrostatic pressure
- Materials need *Permeability to fluids* together with the *Density* and
  *Dynamic viscosity* of the pore fluid. New water saturated materials *Gravel*,
  *Coarse Sand*, *Fine Sand*, *Sandstone*, *Silt* and *Clay* are ready to use
- A model without a *Pressure (explicit)* condition stops with an error naming
  the missing condition

#### STEP import and export

- **STEP (ISO 10303-21) files** can be opened (`*.step`, `*.stp`) and models
  exported to STEP (**File > Export > Export STEP model**), next to STL
- Import triangulates exact CAD geometry - planes, cylinders, cones, spheres,
  tori, B-spline, extruded, revolved and offset surfaces - as well as faceted
  and tessellated (AP242) geometry. Edges shared by two faces are divided once,
  so solids come in closed and ready for meshing
- Every solid or shell becomes its own surface, named after the solid or the
  product. Assemblies are placed as in the CAD system, repeated parts included,
  and lengths are converted from the file units to meters
- Export writes each surface as a separate body of planar triangles: closed
  surfaces as solids, open surfaces as shells, in meters
- Faces that cannot be triangulated are skipped and listed in the log by their
  STEP entity number

#### Languages and documentation

- **Multiple languages.** The user interface, action help, tutorials and theory
  manuals are available in English, Czech, French, German, Italian, Polish,
  Slovak, Spanish and Ukrainian. Choose the language in the **Appearance** tab
  of the application settings (takes effect after restart)
- Theory manuals with formulation, boundary conditions, user interface and
  tutorials for heat transfer, stress, acoustics, fluid flow, heat transfer in
  fluids, contaminant dispersion, electro-statics, magneto-statics and flow
  through porous media

#### Application

- Packages for the Apple App Store and the Microsoft Store
- The view can be rendered with **QRhi** (Metal, Vulkan, Direct3D or OpenGL) as
  well as with the legacy OpenGL pipeline. Choose it in the **General** tab of
  the application settings or with `--render-backend` and `--rhi-api`. QRhi
  draws dashed lines solid and, on Metal, lines one pixel wide
- Materials are stored in JSON; existing material files are converted on start
- All icons are redrawn in a consistent style, with variants for the dark colour
  scheme that switch immediately
- `--reset-settings` resets all settings to defaults, a way out of a
  configuration that keeps the application from starting
- Adding a problem type to the task flow skips types already there, so adding
  *Contaminant dispersion* no longer adds a second *Incompressible viscous flow*
- The **Process output** tab keeps at most *Max lines* lines (default `10000`),
  set next to the **Clear** button. The oldest lines are removed first
- On macOS the menubar can be drawn inside the main window (**Appearance** tab,
  takes effect after restart)

#### 3D view and geometry

- Entities selected in the model tree are highlighted in the 3D view
- **Merge nodes** merges the picked nodes into one, placed at their center,
  regardless of their distance
- Applying a variable as *Scalar* or *Displacement* clears that role from the
  other variables of the entity
- **Shift** + scroll zooms ten times faster

### Bug fixes

#### Stress analysis

- **Thermal stress was missing from the stress results** of volume and surface
  elements: a body free to expand showed stress and a clamped one showed none.
  The *Force* result now also balances under a thermal load. Surface elements
  get the plane strain thermal strain `(1+v)*alpha*dT`, so their thermal
  deflection changes. **Results of thermal stress models change**
- Surface elements with a thickness were assembled wrongly. Their stiffness and
  mass coupled every pair of element nodes, only the last integration point of
  a quadrilateral counted, and the stiffness ignored the thickness. **Stress
  results of models with thick surfaces change**; volume and line elements are
  not affected

#### Electro-statics and magneto-statics

- **Results of electro-static models change.** *Electric field*, *Current
  density*, *Electric energy* and *Joule heat* no longer depend on the element
  size, and *Charge density* enters with the correct sign, which reverses the
  potential of models using it on lines, surfaces or volumes
- **Magneto-statics** computes the magnetic field with the Biot-Savart law. The
  previous formulation had no boundary condition, so its field level was
  arbitrary. Lines with a cross area and surfaces with a thickness now carry
  current too, and every node receives a value
- Units corrected: *Relative permittivity* `N/A`, *Charge density* `C/m^3`,
  *Electric energy* `J/m^3`, *Joule heat* `W/m^3`

#### Fluids and contaminant dispersion

- Transient contaminant dispersion runs no longer blow up. Concentration used to
  grow far above the inflow value where the flow recirculates, and later next to
  an outlet meeting a wall. Negative values are no longer clipped, so small
  undershoots are now visible
- Density and viscosity of a flow follow the temperature of a *Heat transfer in
  fluids* task on every iteration of a task group. A temperature-dependent
  density now drives natural convection in steady-state runs too; the fluid
  flow manuals no longer claim the solver has no buoyancy
- Restarting a transient run from a steady-state result starts at the *Start
  time*. All times, including those of time-dependent conditions, used to be
  shifted by `1.001 s`

#### Solver and materials

- A material missing an optional or alternative property (Rayleigh damping,
  acoustic damping factor, speed of sound or modulus of elasticity) is no longer
  marked invalid or reported as missing by the setup checker, and its entities
  are solved
- A material is not lost when converting its file fails, and editing a converted
  material no longer recreates the old file
- **Start solver**, **Stop solver** and **Kill solver** are enabled only when
  they apply, so a solver can no longer be started twice for the same model
- On Windows the default **Solver executable** includes the `.exe` extension,
  so the application settings no longer report an existing solver as
  `(does not exist)`. A path stored by an older version triggers the
  *Unusual configuration* question once; answer *Yes* to switch to the default
- Changing the selection or a condition no longer re-reads the whole material
  database, which made the user interface slow

#### User interface and 3D view

- Checking or unchecking a *Displacement* component no longer crashes the
  application
- Results could not be displayed on a volume left over from a previously solved
  problem type
- Changing display properties sometimes took effect only after another change
- Picked nodes are visible
- **Pick details** lists every picked item, not only the first
- Model edges are updated after swapping or synchronizing normals
- The *Force* result got the colour range of the displacement on first display
- Local direction arrows (e.g. roller displacement on a surface) are sized
  relative to the model and zoom instead of extending far beyond small models
- Transparent volumes no longer show inner triangles along edges and corners
- Menus, tool bars and the model tree context menu show all their separators
- Missing icons of several buttons and of the **Session** menu are shown

### Submodules

- range-ai-lib @ v1.1.0
- range-base-lib @ v1.2.0
- range-build-tools @ v1.1.0
- range-cloud-lib @ v1.2.0
- range-gui-lib @ v1.2.0
- range-model-lib @ v1.2.0
- range-solver-lib @ v1.2.0

---

## Version 1.2.2

### Improvements

- Model records playback/recording now honors a configurable record range:
  "From"/"To" spin boxes (on a second toolbar row) let you pick the start and
  end record instead of always running from the first to the last record
- Records inside the selected From-To range are highlighted in the records tree
  (slightly darker on light themes, slightly lighter on dark themes)
- Added a context menu to the records tree: load record, remove record(s), and
  set the selected record(s) as the range start, range end, or full range
- The video settings dialog now shows the estimated video length, computed from
  the number of records in the range and the frames-per-record / frames-per-second
  settings, updating live as those values change
- Added Slovak translations for the new records UI

### Bug fixes

- Video animation now encodes only the records within the selected range
  instead of every screenshot present on disk
- Playback/recording no longer overshoots the selected end record by one record
- Fixed model id truncation in the animation export (passed as `bool` instead
  of `uint`)
- Encoded video length now matches the expected length: frames are given explicit
  per-frame timestamps instead of relying on encoder/muxer defaults (which could
  halve the duration), and the frame-rate hint is no longer skipped on the second
  and subsequent recordings (a stale `static` counter)
- Video frames are no longer dropped when the encoder input is momentarily not
  ready; the frame is kept and retried instead
- Renaming a material in the material manager is now persisted to its file
  instead of being silently reverted (the typed name was never written back
  into the material or saved to disk, so the file watcher would restore the
  original name)
- Material list in the material manager no longer rebuilds all of its items on
  every data-directory change (the file-system watcher compared file base names
  instead of full paths)

### Submodules

- range-ai-lib @ v1.0.0
- range-base-lib @ v1.0.1
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.0.1
- range-gui-lib @ v1.0.0
- range-model-lib @ v1.0.1
- range-solver-lib @ v1.0.1

---

## Version 1.2.1

### Improvements

- Added range-ai-lib
- Model consolidation cache: edge nodes, edge elements, surface and volume
  neighbors are now recomputed only when the mesh changes, reducing redundant
  work during repeated model operations
- Improved model insertion and geometry transform performance by preallocating
  model storage, using spatial indexing for merged node insertion, and avoiding
  full node scans when collecting transformed nodes
- Reduced picking overhead by avoiding displaced-node copies when no
  displacement display variable is active and by merging OpenMP pick results once
  per thread
- Optimized surface marking, edge detection, and sweep-node splitting by using
  direct membership scans, cached surface normals, and hash-based node maps
- Vector field arrows are now drawn in two batched GL passes (one `GL_LINES` call
  for all shafts, one `GL_TRIANGLES` call for all cone heads) instead of one
  `GLArrow::paint() `call per arrow, eliminating per-arrow GL state queries and
  redundant `glBegin()`/`glEnd()` pairs
- Auto-expands the model tree item when a new model is added

### Bug fixes

- Axis, grid, lines and element edges were not rendered (GL immediate-mode
  fallback missing from GLFunctions after the VBO pipeline migration)
- Scalar results (e.g. Von Mises stress colormap) were not displayed on volume
  elements or line elements (texture enable/disable bypassed the VBO usesTexture
  flag)
- Draw-engine objects are rendered before releasing the main shader, keeping VBO
  lighting active while fixed-function overlays continue to render correctly
- Record list marker icon remains visible and synchronized for large transient
  result sets, including records beyond 1000

### Submodules

- range-ai-lib @ v1.0.0
- range-base-lib @ v1.0.0
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.0.0
- range-gui-lib @ v1.0.0
- range-model-lib @ v1.0.0
- range-solver-lib @ v1.0.0

---

## Version 1.2.0

- Replace the legacy display-list / immediate-mode rendering pipeline with a
  fully VBO-based backend and GLSL 1.20 shaders
- Enable transparent surfaces
- Improved translations
- Documentation and tutorials translated to Slovak language
- Release notes displayed in a dedicated dialog with markdown rendering

### Bug fixes

- Do not display empty "()" in model info box area when no model comment is set
- Do not allow to accept empty model name when renaming a model or creating a
  new model
- Cannot pick hole edges
- Fix SSL error on macOS by seeding system CA certificates

---

## Version 1.1.3

- Performance improvements in OpenGL model drawing
- Performance improvements in mesh repair functions

### Bug fixes

- Element entities are not painted in 3D area
- Bug in edge element detection

---

## Version 1.1.2

- Automatic client certificate renewal
- Show warning when client certificate is about to expire

---

## Version 1.1.1

- Print time-stamps in output window
- Log Qt debug messages

### Bug fixes

- Automatic file synchronization with Range Cloud

---

## Version 1.1.0

- Support for Range Cloud
- Solver optimizations

### Bug fixes

- Fixed memory leak in draw object widget
- Fixed incorrect deployment on MacOS

---

## Version 1.0.1

- Help and tutorials converted to markdown format
- Default proxy type set to "System"

### Bug fixes

- Fixed not working reset to default number and date formatting.

---

## Version 1.0.0

- User data directory structure moved into "{Path to user documents}/Range
  Software/Range" directory
- Translated to Slovak language
- New software update manager
- Configurable user details
- Report contains author name and email address
- Removed dependency to FFMPEG library
- Extensive code optimization
