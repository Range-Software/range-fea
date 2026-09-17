## Version 1.3.0

### Improvements

- `--verify-jacobian` compares the matrix the fluid solver assembles against a
  finite difference of the residual it is solved against, once, and reports where
  the two disagree. It is a diagnostic for the formulation rather than something
  to run on a real model: each unknown is perturbed in turn and the whole system
  reassembled either side of it, so it belongs on a mesh of a few elements and
  nowhere else. The run is slow and says so
- The report gives, per block of the system, the ratio of the assembled entry to
  the measured one, grouped so that a block made of several terms can be told
  apart. A block whose ratio is one throughout is assembled correctly; a block
  whose ratio is some other number throughout carries that factor too many.
  `RSolverFluid` documents what the check has found so far

### Bug fixes

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
- The magnetostatic source term uses the vacuum permeability instead of the
  vacuum permittivity and is assembled in its Galerkin form, and the current
  density it reads from the electrostatics result is averaged onto the nodes
  rather than taken from one neighbouring element. Every magnetostatic result
  changes. The problem type is still incomplete - no boundary condition exists
  for it, so the level of the computed field is arbitrary
