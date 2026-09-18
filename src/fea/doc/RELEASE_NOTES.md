## Version 1.3.0

### Improvements

#### Acoustics

- **Acoustics** is offered again in the problem selector. The solver was
  previously marked as not working and hidden from the application
- The problem tree carries an **Acoustic analysis setup** group: transient or
  harmonic analysis, the frequency sweep (first frequency, frequency step,
  number of frequencies) and the reference pressure used for the sound pressure
  level. The frequency fields are disabled for a transient analysis, and a
  read-only label shows the resulting frequency range
- Acoustic boundary conditions, initial conditions, material properties and
  result variables are offered by the respective trees. Notable additions are
  the *Acoustic impedance* boundary condition, the absorption coefficient of the
  *Absorbing boundary*, the *Velocity potential / s* initial condition, the
  *Speed of sound* and *Acoustic damping factor* material properties, and the
  sound pressure level, acoustic intensity and acoustic phase results
- A harmonic analysis writes one model record per solved frequency, so the
  records tree steps through the frequency sweep the way it steps through time
  steps

#### Solver and task flow

- The problem task flow dialog offers a **Convergence** value next to
  **# of iterations** on every task group. The group ends its iterations as soon
  as every task in it is below that value instead of always running the full
  count, which on a flow model is most of the run time. It defaults to `1e-5`;
  `0` restores the former behaviour of running every iteration
- **The model file format is raised to 1.3.1** to carry that value. Models
  written by this version cannot be read by earlier ones; models written by
  earlier ones load unchanged and take the default

#### Boundary conditions

- Displacement constraints meeting at one node are combined rather than
  overwriting one another, so a face can be held in a global direction by
  *Displacement* and roll on a tilted plane by *Roller displacement* at the same
  time. Two entities asking for different values in the same direction stop the
  run with an error naming the node
- Individual components of the *Displacement* condition can be switched off with
  a check box, so a support can hold one global direction and leave the others
  free. The value box of an unchecked component is greyed out
- The local direction of *Normal displacement* and *Roller displacement* can be
  entered for a surface or a line, not only for a point. Ticking **Use entered
  local direction** reveals the direction editor and makes the solver take the
  local frame from the entered direction instead of from the element normals or
  the element direction. The direction arrows in the 3D view follow whichever is
  in effect

#### Geometry

- The **Geometry** menu offers a **Merge nodes** action next to *Merge near
  nodes*. It merges the picked nodes - at least two, per model - into a single
  node placed in their center and rebuilds every element which contained them,
  downgrading an element which degenerates in the process. Unlike *Merge near
  nodes* it takes no tolerance and merges exactly what is picked, however far
  apart the nodes are

#### 3D view

- The view can be rendered through **QRhi**, the Qt rendering hardware
  interface, next to the legacy **OpenGL** pipeline. Both backends share the
  whole scene traversal and draw the same geometry; `GLWidget` keeps the view,
  the picking and the display properties and delegates the drawing to a render
  surface which is a `QOpenGLWidget` or a `QRhiWidget`. Value ranges, message
  and info boxes and the projected text labels moved to a transparent overlay
  widget, because a `QRhiWidget` cannot be painted into with `QPainter`
- The backend and the graphics API used by QRhi - automatic, Metal, Vulkan,
  Direct3D 11, Direct3D 12, OpenGL or null, offering only what the platform has
  - are chosen in the **General** tab of the application settings. Both are
  stored per user and take effect on the next application start, which is
  reported next to the option. The `--render-backend` and `--rhi-api` command
  line options override them for a single run without changing what is stored
- Two fixed-function features have no QRhi counterpart and are ignored by that
  backend: line stipple, so dashed grid and axis lines are drawn solid, and line
  widths greater than one on graphics APIs without wide line support, Metal
  among them
- Holding **Shift** while scrolling zooms ten times faster, for crossing a wide
  range of scales without scrolling repeatedly
- Applying a result variable as **Scalar** or as **Displacement** clears that
  role from every other variable of the entity. Only one variable at a time can
  be displayed, and a variable left over from a previously solved problem type
  used to win the lookup and leave the entity in its plain colour

#### Materials

- Materials are stored in JSON. On each start every material of the database
  which is not in the default format is converted and the former file removed

#### Application settings

- `--reset-settings` resets all application settings to their defaults before
  the main window is built, which is the way out of a configuration keeping the
  application from starting. The stored session is part of the settings, so the
  application starts with an empty document. The reset re-applies the number of
  solver threads, which the application reads once while starting and would
  otherwise keep from the settings being discarded
- On macOS the **Appearance** tab offers a *do not use native menubar* option,
  which draws the menubar inside the main window instead of the system menubar
  at the top of the screen. It takes effect on the next application start

#### Icons

- All application icons are redrawn in one clean, light style: a 24 x 24 grid,
  1.5 px rounded strokes, pale tinted fills and no gradients or black outlines.
  Colours carry one meaning each - slate for neutral objects, blue for geometry
  and navigation, teal for results, violet for problem setup, green for create
  and confirm, red for remove and errors, amber for edit and warnings
- Variants of an icon share one set of corner badges, so *new*, *open*, *save*,
  *save as*, *close*, *create* and *edit* look the same on sessions, models,
  materials and result entities
- A few icons show a different picture than before: *Access rights* is a lock,
  the generic help action a lightning bolt, *Drop results* the results symbol
  with a remove badge, and the solver start, stop and kill actions a play
  button, a stop button and a stop sign instead of calculators. The Range and Qt
  logos are unchanged
- Every icon has a variant for the dark colour scheme, drawn with brighter
  strokes and deep tinted fills. The variant in use follows the active palette
  and switches immediately when the colour scheme is changed, without a restart
- Both sets are generated by `pixmaps/generate_icons.js` from the same
  drawings, so the light and the dark icons cannot drift apart. The icon
  resources, about 590 kB before, are well under 100 kB per set

#### Documentation

- Theory manuals under `help/theory/en`, each covering the formulation, the
  boundary conditions and their units, the parts of the user interface which
  belong to the problem type and worked tutorials: `theory_heat.md`,
  `theory_stress.md`, `theory_acoustic.md`, `theory_fluid.md` (all three fluid
  problem types - *Incompressible viscous flow*, *Heat transfer in fluids* and
  *Contaminant dispersion* - the stabilised Navier-Stokes formulation and the
  non-linear task group iteration the flow solver depends on),
  `theory_electrostatics.md` (including what separates an electro-static
  analysis from a current-flow one and the coupling to heat transfer and
  magneto-statics) and `theory_magnetostatics.md`, which states at the top that
  the problem type is incomplete and lists what is wrong rather than leaving the
  value of the result to be guessed

#### Packaging

- The build can produce packages for the **Apple App Store** and the **Microsoft
  Store**, enabled with `-DRANGE_STORE_BUILD=ON`. An ordinary release build is
  unaffected: the store artifacts are built by their own `mac-app-store` and
  `msix` targets, next to the DMG, the Inno Setup installer and the Linux
  packages
- On macOS the bundle is sandboxed and signed with the two *3rd Party Mac
  Developer* certificates, carries the Mac App Store provisioning profile and is
  wrapped into an installer package. `Info.plist` is generated from a template
  rather than patched after the build, so the bundle identifier, the application
  category, the copyright and the encryption declaration the store requires are
  all present. The bundled `fea-solver` and `openssl` executables are signed to
  inherit the sandbox of the application
- On Windows the installation tree is packed into an MSIX package with
  `makeappx.exe`, from a generated manifest and with the tile and store logos
  taken from `pixmaps/msix`
- A store build defines `STORE_BUILD`, which removes the built-in software
  update checker. Both stores forbid an application from downloading and
  installing its own updates
- A `Store packages` workflow builds both packages on demand and uploads the
  macOS package to App Store Connect

### Bug fixes

#### Electro-statics and magneto-statics

- **These corrections change the results of every model using the
  *Electro-statics* problem type.** A model driven only by prescribed potentials
  keeps its potential field and gains corrected element results; a model using
  *Charge density* on a line, surface or volume also changes the sign of its
  potential
- **Electric field**, **Current density**, **Electric energy** and **Joule
  heat** are densities again. The recovered potential gradient was weighted by
  the Jacobian determinant of the element, and on a surface by its thickness as
  well, so each of them scaled with the size of the element it was computed in
  and drifted instead of converging as the mesh was refined. The electric
  potential and the electrical resistivity, whose two factors cancel, were never
  affected
- The **Charge density** boundary condition enters the system with the sign the
  Poisson equation calls for. It was assembled negatively on line, surface and
  volume elements and positively on point elements, so a positive space charge
  lowered the potential around it while a point charge did the opposite
- **Joule heat** is the dissipation density `sigma*|E|^2` in `W/m^3`, which is
  what a *Heat transfer* task integrates over the element when it picks the
  value up as a source. It carried a characteristic element size on top of that,
  so a resistive heating chain delivered a power which depended on the mesh
- Units corrected to what the solver reads and writes: **Relative permittivity**
  `N/A` instead of `C^2`, **Charge density** `C/m^3` instead of `C`, **Electric
  energy** `J/m^3` and **Joule heat** `W/m^3`
- **Magneto-statics** computes a different field. Its source term used the
  vacuum permittivity where the vacuum permeability belongs and carried a shape
  function factor the weak form does not, and the current density it takes from
  the electro-statics task is now averaged onto the nodes rather than assigned
  from one neighbouring element. **Magnetic field** is offered by the
  magneto-statics problem type rather than by electro-statics, which never
  computes it
- Magneto-statics remains incomplete. No boundary condition exists for it, so
  nothing constrains the field and the level of the computed result is
  arbitrary. The theory manual says so at the top and lists what is left

#### Solver setup

- The setup checker no longer reports material properties as missing when they
  are optional for the selected problem, or when they are one of several
  accepted alternatives. An acoustic entity carrying a density and a speed of
  sound is no longer flagged for missing a modulus of elasticity

#### 3D view and picking

- A picked node is visible. It was marked with a single point whose size the
  OpenGL shaders never write, so nothing was drawn to show for the pick, which
  had always worked. A picked node now carries the mark an unpicked node carries
  - the same shape, ten percent larger - drawn in white
- Element nodes were drawn a single pixel wide under the QRhi backend with
  Direct3D selected, the default on Windows, leaving them all but invisible. A
  point size greater than one pixel is an OpenGL and Vulkan only feature;
  Direct3D and Metal rasterise every point one pixel wide whatever size is asked
  for. Points are now expanded into two triangles apiece and sized in the vertex
  shader, so a node is the same size on every graphics API
- Enabling **Draw element nodes** made every surface disappear under the QRhi
  backend. The vertex buffer of an entity is grown when the geometry no longer
  fits it, and growing it destroyed the buffer together with the list of
  primitive batches just recorded for it, leaving the geometry uploaded with
  nothing left to draw it with. Switching the property off again fitted the
  geometry back into the buffer already allocated, which is why the surfaces
  returned. The batch list is now handed to the buffer only once it is ready
- An entity is drawn again when its display properties change. Its vertex buffer
  was kept until something invalidated it explicitly, so a property whose
  invalidation was missed left the entity drawn the way it was when first
  displayed - *Draw element nodes* taking effect only after another property was
  switched on and off. The properties a buffer was recorded with are now stored
  with it and compared before it is reused, so a missed invalidation corrects
  itself on the next frame
- An entity keeps one vertex buffer instead of three. The two reserved for
  picked elements and picked nodes were never recorded into or drawn - picked
  items are drawn as an overlay straight from the pick list - and only ever
  invalidated
- Results could not be displayed on a volume. An entity keeps its results
  display data for variables which are no longer in the model, and the lookup
  returned the first of them rather than one the model holds, so a volume left
  over from a stress analysis kept selecting *Von Mises stress* and rendered in
  its plain colour while the temperature was shown correctly on surfaces and
  cuts. Every scalar and displacement lookup of the 3D view now resolves to a
  variable the model actually holds
- Swapping element or surface normals left the model edges as they were. Model
  edges are found from the angle between the normals of neighbouring elements
  and are cached, and none of *Swap element normal*, *Swap normals* and
  *Synchronize normals* marked that cache as out of date, so the edges only
  caught up after some later operation happened to rebuild them
- The **Pick details** tab lists every picked item. It built the first one and
  left the loop, so a pick of six nodes showed a single entry - and that entry
  was left without its heading and its computed results, because the same early
  exit skipped the code which fills those in

#### Materials

- The material database is no longer read from disk every time the model
  selection or the problem changes. A row of the material manager carries only
  the name and the file path of its material, so matching the database against
  the problem type read every file of it again - on every entity selection, and
  on every boundary, initial or environment condition change. Materials are
  cached as they are read, and the cache is rebuilt by the directory watcher
- Reading a material is not free of side effects - a file which is not in the
  default format is converted and the former one removed - so leaving that to
  the directory watcher also stops an ordinary selection change from writing to
  the materials directory
- Converting a material file writes the new file before removing the former one,
  so the material is not lost when writing fails
- Editing or renaming a material which has just been converted no longer writes
  it back to the former file path, recreating a file in the former format
- A row removed from the material manager is deleted rather than only taken out
  of the tree, which leaked it

#### User interface

- Selecting or unselecting a property of the **Displacement** boundary condition
  crashed the application. The tree rebuilt itself from its own `itemChanged`
  signal, destroying the item Qt was still working with; the rebuild is now
  postponed until the current event has been processed. *Displacement* is the
  only boundary condition whose components carry a check box, which is why it
  was the only one affected
- Menus, tool bars and the model tree context menu show all of their separators.
  They were built from one shared separator action, which a widget can only hold
  once, so every menu ended up with a single separator at its bottom. Menus and
  tool bars now use `QMenu::addSeparator()` and `QToolBar::addSeparator()`, and
  the model tree a separator of its own
- Buttons asking for the `range-add.svg` icon - adding an AI agent, adding a
  file tag and the direct and e-mail certificate requests - were shown without
  an icon, because no such icon existed. It is added to the action icons
- The remove button of the AI agent settings list was shown without an icon.
  It asks for `range-remove.svg` among the action icons, where it was not
  registered; it is now
- The **Session** menu was shown without an icon because of a mistyped icon
  file name

### Submodules

- range-ai-lib @ v1.0.0
- range-base-lib @ v1.0.2
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.0.3
- range-gui-lib @ v1.0.0
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
