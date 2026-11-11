## Version 1.4.0

### Improvements

#### Problem selection

- New **Select problem** dialog replaces the *Problem(s) task flow* and *Problem type
  selector* dialogs. Problem types are selected directly and the task flow is built
  automatically, in the order the problems need each other's results
- Maximum iterations and convergence are shown only for selections with a flow or
  radiation. Flow starts at 2000 iterations, or 20 per time step, and values left at
  their default follow the time solver
- The task flow tree remains available under **Customize**
- Double-clicking a tile selects it and closes the dialog
- Tiles show the problem types which are required or incompatible, with the reason in
  the tool tip, instead of warning after the click
- The dialog opened for a new model can be closed with **Skip for now**, and the
  **Problem** tab then offers a **Select problem** button
- After selecting a problem the first tab which needs input is shown
- Incompatible problems, missing required problems and a missing problem selection are
  reported before the solver starts

#### Results tab

- Reorganized the **Results** tab into *Show on selected entities*, *Color range* and
  *Deformation and arrow scale*, with **Graph** and **Remove** at the bottom
- *Scalar* and *Displacement* are now called *Color map* and *Deformed shape*, and are
  disabled until entities are selected in the model tree
- Maximum, minimum and scale take one row each and values show their units. The sliders
  and power of ten moved under *Fine adjust*, which is remembered
- The deformation and arrow scale has a logarithmic slider
- Settings which do not apply to a scalar variable are hidden

#### Ribbon layout

- New optional ribbon replaces menus and tool bars when **Ribbon layout** is switched on
  in the **File** menu, the tool bar or the ribbon. The last used layout is remembered
- Commands are ordered by workflow on the *Geometry*, *Mesh*, *Problem*, *Solve* and
  *Results* tabs. Cut, iso, stream line and field tools are found under *Results*
- Buttons on the *Problem* tab open boundary conditions, initial conditions, environment
  conditions and materials in the panel on the right
- The ribbon can be collapsed with its arrow or by double-clicking a tab. The collapsed
  state and the last tab are remembered
- **Compact ribbon** keeps the same tabs and commands in less vertical space, with smaller
  buttons in two rows and no group titles. The choice is remembered

#### Tool bars

- Tool bars of the classic layout are reorganized into a single row of *File and edit*,
  *Geometry*, *Mesh*, *Workflow*, *Results* and application buttons. Their positions are
  reset once after the update
- Related commands share drop-down buttons. *Draw*, *Boolean* and *Normals* remember the
  last used command
- Cut, iso, stream line and field tools are available from the new *Results* tool bar
- **Kill solver** moved under the drop-down of **Stop solver**. **Close model**, **Quit**
  and **About** are available from the menus only

#### Documents tab

- Files of a model are grouped into *Documents*, *Videos* and *Images*
- The **Records** tab is shown only when there are records
- Models in the tree are separated by a line in the color of the model, as in the model
  tree

#### 3D view

- Position, size and order of model windows are restored after restarting the application
- New *Boundary conditions* option in the entity **Display properties**, for
  points, lines, surfaces and volumes. When it is on, symbols of the boundary
  conditions applied to the entity are drawn on it, as long as the entity is
  visible. Only conditions of the current problem type are shown, with values at
  the current time, and the symbols keep their size on screen when zooming:
  - displacement supports as blue cones, one per constrained direction, normal
    displacement with a ground bar and roller displacement with rollers
  - force, pressure and weight as red arrows, fluid inflow and normal velocity as
    orange arrows; every arrow points at the entity. Negative pressure (suction)
    and negative normal velocity reverse the arrow, weight follows the entity's
    gravity setting
  - walls as a green bar with hatching, frictionless walls as a bar with rollers
  - all other conditions as a text tag with the value and units, such as
    `T = 293.1 K`
- The option is saved in the model file
- Symbols of the boundary conditions being drawn are listed with their names in the
  3D view, in a card below the model card or next to it when both do not fit
- New *Highlight selection* option below the 3D view, on by default, turning off the
  highlight of entities selected in the model tree
- The window of a model selected in the model tree, or with a part of it selected, is
  highlighted with an outline in the color of the model
- Arrows and boundary condition symbols always have the same color on both sides,
  their back faces are no longer drawn silver
- Model information and boundary conditions in the 3D view are shown in cards with
  the same look as the value range. Text which does not fit in a card is shortened
- Zoom and model draw time are shown as one line of text beside the orientation axis,
  with the current action above it, in white or black to contrast with the background
- The 3D view is redrawn when the problem setup changes, so edited boundary
  conditions are shown straight away
- Redesigned value range in the 3D view, now a slim legend over the full height of
  the view which follows the color scheme and covers less of the model:
  - values use readable units, such as `328.5 MPa` instead of `3.28452e+08 Pa`. Simple
    units such as Pa, N, m or s take an SI prefix, other units a common multiplier
    such as `×10⁶ W/m³`. Temperature in K keeps its unit
  - round tick values between the minimum and the maximum
  - a smooth color bar, which is no longer loaded from its file on every redraw
  - one line with the steady state, time step and time, or mode and frequency
  - value ranges of several variables share the height of the view
  - a picked value is shown next to the picked point, with a line to a marker on the
    color bar
- Stepped color scales look better in the 3D view and the value range:
  - bands have equal width and smooth edges
  - lines between bands keep a constant thin width at any zoom and are a darker shade
    of the neighbouring colors instead of black. They fade out where bands get too
    narrow to show them
  - ticks of the value range are at band edges, showing the value where the color
    changes
  - values at the ends of a color scale no longer wrap around to the other end
- New *Smooth display* option on the **Results** tab for stresses, replacing the
  *Display on nodes* button for them. It averages element results at nodes for display
  only, so it can be turned off again at any time:
  - von Mises, normal and shear stress of volumes are computed from averaged stress
    components rather than being averaged themselves
  - elements are averaged only with elements of the same material, thickness or cross
    area, so real jumps of stress between them are preserved
  - averaging lowers peak values, so the value range also shows the range of the
    original element values
  - the setting is not saved in the model file
- Redesigned orientation axis in the lower left corner of the 3D view, with thin
  labelled arrows in the colors of the global axis. Arrows pointing away from the
  viewer are drawn fainter
- Redesigned model windows to match the application: a flat title bar with thin
  window buttons, a thin outline marking the active window, and an area behind the
  windows which follows the color scheme
- A new model window which is not maximized opens at 60% of the area, which is also
  the size it returns to when first restored from maximized
- Each model has its own color, shown as a line at the top of its row in the model
  tree and of its window title bar. Models in the model tree are separated by a small
  gap
- New *Tile windows* button next to the tabs below the 3D view, shown when several
  model windows are open. Clicking it again moves the windows one position clockwise
- Windows are tiled automatically when a model is opened while another window is maximized
  or the windows are tiled
- New *Windows* menu next to *Tile windows* lists the open model windows, and clicking one
  maximizes it. The maximized window is marked with a dot

#### Model files

- Opening a model whose file format is newer than this version supports asks
  whether to load it anyway, as it may not load correctly. This applies to
  *Open model*, models given on the command line or restored with the session,
  *Reload results* and selecting a result record
- Support reactions are saved with the results, at the end of the model file,
  whose format stays at 1.5.0; models saved before read without them

#### Support reactions

- After a stress solution the log lists the total reaction force of every support,
  that is every entity with a *Displacement*, *Normal displacement* or *Roller*
  boundary condition, followed by the total reaction and the total applied load so
  that equilibrium can be checked. Reactions are correct on nodes which are both
  loaded and held, and a node held by several supports shares each held direction
  among the supports which hold it. Inertia and damping forces of a dynamic
  analysis are not included
- The *Results* chapter of generated reports shows the same statistics as the
  **Model statistics** dialog, grouped by problem type: located minimum and
  maximum, weighted averages and the support reactions with the imbalance. Both
  use one implementation, so they always agree

#### Model statistics

- The **Model statistics** dialog has a new *Results* chapter. It shows the time
  step and time, or the mode and frequency, of the results, and for each solved
  problem type the minimum, maximum and average of every result it computes, with
  the node or element and entity where the minimum and maximum occur. Averages are
  weighted by element volume, area or length, with a separate average for each
  kind of element present, so they do not depend on how fine the mesh is. Results
  on nodes are integrated over the elements, which gives their exact average over
  the body for the linear elements in use. A result without elements to weight it
  by shows the plain *average of node values* or *of element values*.
  Vectors are described by their magnitude. Element results of the body, such as
  stress or heat flux, leave out the elements the solver computes no values on:
  point elements, entities whose material lacks the properties the problem type
  needs, and surfaces without a thickness or lines without a cross area, such as
  the boundary surface of a solid. Boundary results, such as heat radiation, keep
  the boundary surfaces. A result which is the same everywhere is shown as one
  value. Stress analysis
  adds the support reactions, the total applied load and the imbalance between
  them, with a note on what the imbalance means and how large it is relative to
  the applied load. All real values in the dialog are shown in scientific format,
  such as `4,000000e-02` Results which belong to no solved problem type are listed under *Other
  results*

#### Cloud

- New *Verification* setting of the cloud host in the **Cloud session manager**.
  *Full verification* (default) checks that the host certificate is issued by a
  trusted certificate authority and matches the host name. *Trusted certificate only
  (skip host name)* is meant for a server reached by its IP address: set its
  certificate authority as the session's host *Certificate* and the host name is not
  checked. *No verification* accepts any certificate and does not guarantee the
  identity of the host
- Cloud requests are faster. A request completes as soon as the reply arrives, where
  it used to wait up to an extra second, and connections to the cloud are kept open
  and reused between requests instead of being opened for each one
- On macOS the private key and the certificate signing request for the cloud are
  generated with the Apple Security framework instead of the `openssl` tool, and so
  is the text shown by the private key view. The `openssl` tool is no longer bundled
  with the application, only the OpenSSL libraries used for TLS connections. The paths
  of the OpenSSL tool and its configuration are hidden in the application settings.
  Configure with `-DRANGE_NATIVE_KEY_TOOL=OFF` to keep using the `openssl` tool

#### Problem types

- Problem types are now selected from tiles with an icon and a name, with the
  description shown as a tooltip. Buttons in the problem type selector switch between
  the tiles and the original list, and the last used view is remembered

### Bug fixes

- Changing the style or the color scheme while the application runs shrank the
  tool bar icons to the default size of the style. Tool bars now keep the icon
  size set in the application settings
- After the color scheme changed, the model window kept drawing its check boxes
  and sliders for the previous scheme, the labels of the central tabs kept the
  previous text color, so a dark scheme left them white on a light background,
  and text already printed to the application and process output kept the
  previous colors. The model window also used the desktop style instead of the
  style chosen in the application settings
- PDF reports came out wrong when the display scaling was not 100 %: pages were
  drawn too large, so their right part was cut off, the bottom of each page with
  the page footer was lost and content was missing between pages. The page size
  was taken at the printer resolution while the text was laid out at the screen
  resolution; both now use the resolution of the PDF page
- Reports exported to OpenDocument had no title page, header, footer or page
  setup, every table had a grid and stretched or squeezed columns, text used a
  different font, and the screenshot ran over the page edge. They now look like
  the PDF: A4 page with its margins, title page, header with the logo, footer with
  page numbers, tables sized and placed as printed, and the same font
- The report format is now called *OpenDocument Text (ODT)*, and the file gets
  the extension `.odt`. It was offered as ODF with the extension `.odf`, which is
  for OpenDocument formulas and may open in a formula editor
- Report files always get the extension of their format: a name without it gets
  it appended, and `.odf` is replaced by `.odt`
- The **Documents** tab listed `.odf` files and missed the `.odt` reports
- The help of *Create report* lists Markdown among the supported formats
- HTML and Markdown reports referred to a screenshot file which did not exist.
  The screenshot is now saved next to the report under the name it refers to
- PDF reports generated with the dark style active had white text on the white
  page. Reports are now always printed black on white
- Models saved in the ASCII format (`.ram`) could not be opened again: yes/no
  settings were saved as control characters and read back inverted, which made
  the rest of the file fail to load. They are saved as `0` and `1` now, and
  `.ram` files saved by older versions are read with the correct values
- Some yes/no settings of models, environment conditions and initial conditions
  were saved with undefined values. They are now always saved as yes or no
- Fixed results kept when generating a 3D mesh with *Keep computed results*. Some new
  nodes on the model boundary got values of unrelated nodes, which showed as spikes in
  the displaced shape, and surface elements got values of unrelated elements
- Generating a 3D mesh no longer loses points and lines. Lines may be split by the new
  mesh, and a line lying completely outside of the meshed volume is still removed
- The orientation axis did not follow rotations made while it was hidden, so it could
  show a wrong orientation after being shown again
- Arrow heads and cones of boundary condition symbols were drawn black, because their
  surface normals did not reach the shader
- The cloud host certificate was not verified, so a connection to a host presenting a
  certificate from an untrusted authority or for another host name was accepted and
  only reported as SSL errors in the log. Such connections are now refused. A session
  whose host uses a private certificate authority needs that authority set as its host
  *Certificate*, or its *Verification* relaxed
- Certificate signing requests generated on macOS store subject fields with characters
  outside ASCII, such as `Žilina`, correctly. The `openssl` tool used before stored
  them in a mangled form (`Å½ilina`)

### Submodules

- range-cloud-lib @ v1.3.0
- range-gui-lib @ v1.3.1
- range-model-lib @ v1.3.0
- range-solver-lib @ v1.2.1

---

## Version 1.3.1

### Improvements

#### Application

- New **File > Folders** submenu opens the application folders - *Certificates*,
  *Data*, *Documents*, *Logs*, *Materials*, *Sessions* and *Temporary files* - in the
  file browser of the operating system (Finder on macOS, File Explorer on Windows, the
  file manager on Linux)
- Help pages of the new actions in all languages
- On the first start of a new version the main window opens centered at 70 % of the
  screen width and height. Later starts restore the size and position it was closed with
- The **Process output** tab keeps at most `1000` lines by default instead of `10000`.
  A *Max lines* value set before is kept

### Bug fixes

- The Mac App Store version could not write to its folders. They are kept in
  `Documents/Range Software/FEA`, which the App Sandbox does not allow to be written to.
  The sandboxed version now keeps them in `Library/Application Support/Range Software/FEA`
  inside its container (`~/Library/Containers/com.range-software.fea/Data`); other
  versions keep using the Documents folder. Use **File > Folders** to find them
- **Save model as** of a model which was never saved opens in the data folder. Before,
  the dialog opened in the folder the operating system remembered from its last use,
  which in the Mac App Store version was the former data folder in Documents
- In the Mac App Store version **Save model as** of a model opened from outside the
  application folders (e.g. the former data folder in Documents) opens in the data
  folder instead of the folder the model was opened from
- In the Mac App Store version the solver could not connect to the application
  (`Failed to connect to: RANGE_SOLVER_TASK_SERVER_...`), so **Stop solver** did not
  reach it. The path of the local socket used for the connection was too long inside
  the App Sandbox container. The stop command is now sent through the standard input
  of the solver on all platforms
- The question about clearing results of problems removed from the task flow and the
  "+ N *more*" label of hidden entities in the 3D view are translated to all languages
- The Microsoft Store package built with MSVC did not start because the LLVM OpenMP
  runtime (`libomp140.x86_64.dll`) was missing. Visual Studio installs it only in the
  `debug_nonredist` Redist folder, which the build did not search. The build now finds
  it there and stops if it is missing, for the Store package as well as for the
  installer and ZIP package
- The `msix` target was not created when the Windows SDK tools were only in a versioned
  folder such as `Windows Kits/10/bin/10.0.22621.0/x64`. The build now finds
  `makeappx.exe` there, newest SDK version first

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
