## Version 1.3.0

### Improvements

#### Command line

- `--verify-jacobian` compares the matrix the fluid solver assembles against a
  finite difference of the residual it is solved against, once, and reports where
  the two disagree. It is a diagnostic for the formulation rather than something
  to run on a real model: each unknown is perturbed in turn and the whole system
  reassembled either side of it, so it belongs on a mesh of a few elements and
  nowhere else. The run is slow and says so
- The report gives, per block of the system, the ratio of the assembled entry to
  the measured one, grouped so that a block made of several terms can be told
  apart. A block whose ratio is one throughout is assembled correctly; a block
  whose ratio is some other number throughout carries that factor too many. The
  range-solver-lib release notes record what the check has found so far

#### Solver run

- A task group ends its iterations as soon as every task in it is below the
  group's **convergence value** instead of always running the full count. Models
  written before version 1.3.0 take the default `1e-5`, so they may now finish
  in fewer iterations; a value of `0` restores the former behaviour. The model
  file format is raised to 1.3.1 to carry the value
- The fluid solver reports convergence (it never did, so a group with a flow task
  always ran every iteration) and damps its Newton step when a pass raises the
  residual. The relaxation is logged as `Relaxation`
- *Forced convection* on a wall between a meshed solid and a meshed fluid couples
  the heat and fluid heat solvers directly instead of using a flat plate
  correlation. The heat solver solves solids only and leaves fluid volumes to the
  fluid heat solver
- **Acoustics** runs again, including a new harmonic (frequency domain) analysis
  that writes one result record per swept frequency
- Displacement constraints from several boundary conditions on one node combine
  instead of the last one overriding the others, and conflicting prescribed
  values are reported. Modal analysis uses subspace iteration and inverse power
  iteration, which converge to the lowest modes

### Bug fixes

#### Electro-statics and magneto-statics

- The electrostatic element results are densities. The recovered potential
  gradient was weighted by the Jacobian determinant of the element, and on a
  surface by its thickness as well, so the electric field, current density,
  electric energy and Joule heat all scaled with the size of the element they
  were computed in and drifted instead of converging as the mesh was refined.
  The electric potential and the electrical resistivity were never affected
- The **Charge density** boundary condition enters the system with the sign the
  Poisson equation calls for, on every entity type. It was assembled negatively
  on line, surface and volume elements and positively on point elements, so a
  positive space charge lowered the potential around it
- Joule heat is the dissipation density `sigma*|E|^2` in `W/m^3`, the source
  density a heat task integrates over the element, rather than that density
  multiplied by a characteristic element size
- These corrections change the results of every model using the
  *Electro-statics* problem type, and the statistics printed for them. The units
  in the solver log follow: relative permittivity is `N/A` rather than `C^2`,
  charge density `C/m^3` rather than `C`, electric energy `J/m^3` and Joule heat
  `W/m^3`
- The magnetic field is computed from the electrostatic current with the
  Biot-Savart law. It used to come from a field equation with no boundary
  condition, so every model was singular and the level of the field was
  arbitrary. Line and surface conductors now carry current as well as volumes,
  and every node receives a value. Every magnetostatic result changes. The field
  of supply leads outside the model is not included

#### Heat transfer

- The *Heat* boundary condition delivers its total in `W`, spread over the
  entity. It acted as a density on volumes, surfaces and lines, so the power
  delivered depended on the size of the model
- A model using *Forced convection* no longer stops with an error about a
  missing fluid temperature
- The fluid heat solver assembled conduction with the wrong sign relative to
  advection, so heat was carried upstream and a heat source cooled the fluid

#### Stress and modal analysis

- Von Mises stress was too high by up to about 41 %, modal frequencies are
  written in Hz instead of as raw eigenvalues, and the eigenvalue solver no longer
  returns wrong or random modes
- Truss (line element) stiffness, axial stress and thermal load, rotated loads
  on rollers whose direction is not along a global axis, and *Displacement*
  holding all three directions whatever components were switched on are fixed.
  The range-solver-lib release notes list the full set of corrections

#### Acoustics

- The transient acoustic solver was unsolvable (negative mass matrix), reset its
  time integration state every step, and computed the pressure from the
  velocity potential rather than from its time derivative. These and the other
  defects listed in the range-solver-lib release notes are fixed

### Submodules

- range-base-lib @ v1.1.0
- range-model-lib @ v1.2.0
- range-solver-lib @ v1.2.0
