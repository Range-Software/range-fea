## Version 1.3.0

### Improvements

- Materials are now stored in JSON. On each start every material of the material
  database which is not stored in the default format is converted and the former
  file is removed
- **Acoustics** is offered again in the problem selector. The acoustic solver was
  previously marked as not working and was hidden from the application
- The problem tree shows an **Acoustic analysis setup** group box for acoustic
  problems, selecting between a transient and a harmonic analysis and holding
  the frequency sweep (first frequency, frequency step, number of frequencies)
  and the reference pressure used for the sound pressure level. The frequency
  fields are disabled for a transient analysis, and a read-only label shows the
  resulting frequency range
- Acoustic boundary conditions, initial conditions, material properties and
  result variables are offered by the respective trees. Notable additions are
  the *Acoustic impedance* boundary condition, the absorption coefficient of the
  *Absorbing boundary*, the *Velocity potential / s* initial condition, the
  *Speed of sound* and *Acoustic damping factor* material properties, and the
  sound pressure level, acoustic intensity and acoustic phase results
- A harmonic acoustic analysis produces one model record per solved frequency,
  so the records tree steps through the frequency sweep the same way it steps
  through time steps
- Added theory manuals under `help/theory/en` - `theory_acoustic.md` and
  `theory_stress.md` - covering the formulation, the parts of the user interface
  which belong to each problem type, and worked tutorials
- Displacement constraints meeting at one node are now combined rather than
  overwriting one another, so a face can be held in a global direction by
  *Displacement* and rolled on a tilted plane by *Roller displacement* at the
  same time. Two entities asking for different values in the same direction stop
  the run with an error naming the node

### Bug fixes

- Menus, tool bars and the model tree context menu show all of their separators.
  They were all built from one shared separator action, which a widget can only
  hold once, so every menu ended up with a single separator at its bottom.
  Menus and tool bars now use `QMenu::addSeparator()` and
  `QToolBar::addSeparator()`, and the model tree a separator of its own
- Converting a material file to the default format now writes the new file
  before removing the former one, so the material is not lost when writing fails
- Editing or renaming a material which has just been converted no longer writes
  it back to the former file path, recreating a file in the former format
- The local direction of a *Normal displacement* or a *Roller displacement* can
  now be entered for a surface or a line, not only for a point. The condition
  tree shows a **Use entered local direction** check box; ticking it reveals the
  direction editor and makes the solver take the local frame from the entered
  direction instead of from the element normals or the element direction. The
  direction arrows drawn in the 3D view follow whichever is in effect
- Components of an optional boundary condition can be switched on and off. The
  boundary condition tree shows a check box next to each component of the
  *Displacement* condition, so a support can hold one global direction and leave
  the others free. The value box of an unchecked component is greyed out
- The solver setup checker no longer reports material properties as missing when
  they are optional for the selected problem, or when they are one of several
  accepted alternatives. An acoustic entity carrying a density and a speed of
  sound is no longer flagged for missing a modulus of elasticity

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
