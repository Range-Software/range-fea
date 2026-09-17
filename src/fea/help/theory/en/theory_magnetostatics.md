# Magneto-statics - Theory Manual and User Guide

This document describes the magnetostatics solver (`RSolverMagnetostatics`) of
Range FEA, which serves the **Magneto-statics** problem type: the equation it
sets out to solve, the inputs it accepts, the parts of the graphical user
interface that drive it, a worked tutorial and what its result is worth.

> **Status.** The magnetostatics solver is **incomplete**. Its element
> formulation is now right - the weak form, the Galerkin source term and the
> vacuum permeability are all in place - but **no boundary condition of any kind
> exists for the problem type**, so nothing can pin the field and every model is
> singular. The solve completes and the shape of the field is meaningful; its
> level is arbitrary. Section 5 lists what remains. This manual documents the
> solver as it stands rather than as it is meant to be, so that what the numbers
> are worth is not left to be guessed.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - the field of a current-carrying conductor](#3-tutorial---the-field-of-a-current-carrying-conductor)
4. [Checking a model](#4-checking-a-model)
5. [Limitations](#5-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the magnetic field

The primary unknown is the **magnetic flux density** `B` at every node, in tesla.
It is solved for directly as a vector: every node carries **three** degrees of
freedom, one per component, so a mesh with `N` nodes gives a system of `3N`
equations and a matrix of the same size as a structural model of the same mesh.

Most magnetostatic codes solve for a scalar or vector **potential** and
differentiate it to get the field, which enforces `div(B) = 0` by construction.
This solver does not: it treats the three components of `B` as independent
unknowns, coupled only through the source term. Section 1.9 explains what that
costs.

The default field of a node that no result touches is zero in all three
components.

### 1.2 Governing equation

In a current-carrying region, Ampere's law and the absence of magnetic monopoles
give

```
curl(B) = u0 * J
div(B)  = 0
```

and taking the curl of the first, with the second, leads to the vector Poisson
equation the solver sets out to solve:

```
laplace(B) = -u0 * curl(J)
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `B` | magnetic flux density | `T` | solved for |
| `J` | current density | `A/m^2` | from the electro-statics task, see 1.4 |
| `u0` | vacuum permeability | `H/m` | built in, `1.25663706212e-6` |

The permeability of the material does not enter anywhere. There is no relative
permeability material property, no magnetisation, and no distinction between `B`
and `H` - the solver works in free space whatever the model is made of.

**No material property is required.** The magnetostatics problem type declares
none at all, so unlike every other solver in Range FEA it does not skip an entity
whose material is incomplete, and the setup checker has nothing to check on its
behalf. Every element of the model is taken as computable. The electro-statics
task the problem type depends on has its own three requirements - see the
electro-statics manual.

### 1.3 Finite element discretisation

The weak form of the vector Poisson equation, integrating the curl of the source
by parts so that only first derivatives of `J` are needed, is

```
INT( grad(w) . grad(B) ) = -u0 * INT( grad(w) x J )
```

which gives, for each element and each of the three components,

```
K = -INT( B^T * B )                     stiffness, one block per component
f = +INT( grad(N) x J ) * u0            source
```

where `B` here is built from the shape function derivatives - not to be confused
with the magnetic field of the same letter - and `N` are the shape functions.
Each element matrix is `3n x 3n` for an element with `n` nodes, and the three
components are **decoupled** in the stiffness: the `x` equation contains only
`x` unknowns, and the components meet only through the cross product in the
source. The current density in the cross product is **interpolated at the
integration point** from the nodal values, as a Galerkin source term requires.

Both sides are assembled negated with respect to the expression above, which is
self-consistent and leaves the solution unchanged, but it does mean the matrix
handed to the conjugate gradient solver is negative definite. With Jacobi
preconditioning both the matrix and its diagonal carry that sign, so the
iteration behaves; see 1.7 for what does not.

The constant `u0` is the vacuum permeability `1.25663706212e-6 H/m`, held by the
solver alongside the vacuum permittivity the electrostatics solver uses. Neither
is a material property or a setting.

**Both boundary terms are dropped.** Integrating the Laplacian by parts leaves a
flux term over the boundary, and integrating the curl of the source by parts
leaves another; neither is assembled. That is a homogeneous natural condition on
every outer face, and it is the reason the system has no constraint - see 1.7.

**Volume elements only.** The assembly loops over volume entities and nothing
else. Surface, line and point entities contribute no stiffness and no source
whatever they carry, but their nodes are still counted among the unknowns, which
matters in 1.7.

### 1.4 The current density source

The magnetic field is driven by one input and one only: the **current density**
left behind by an *Electro-statics* task earlier in the problem task flow. There
is nothing to enter and nothing to assign.

The electrostatics solver stores a current density per element. The magnetostatic
solver reads those three component vectors and converts them to nodal values
before assembly, as an **inverse distance weighted average** of the elements
meeting at each node, so a node shared by several elements takes a mean of them
weighted towards the nearest element centres. Nodes no computable element reaches
are left at zero.

From there the assembly interpolates the nodal values back to the integration
points of each element, which is what the Galerkin source term of 1.3 calls for.

Because the source is a **curl** of the current density, a uniform current
density produces no field at all inside the conductor: only where the current
density varies from node to node does the source term become non-zero. The field
of a straight conductor therefore arises entirely from the variation of `J`
across the mesh, which puts a great deal of weight on how well the mesh resolves
the conductor and its boundary.

If no electrostatics result is present the recovered current density is zero
everywhere, the source term vanishes, and the solve returns a zero field.

### 1.5 Boundary, initial and environment conditions

There are **none**. No boundary condition, no initial condition and no
environment condition is declared for the magnetostatics problem type anywhere in
the model library.

| Condition kind | Offered for magneto-statics |
|---|---|
| Boundary conditions | none |
| Initial conditions | none |
| Environment conditions | none |
| Material properties | none |

The practical consequences run through the rest of this manual:

- Nothing can prescribe the field on a face, an edge or a point, so the system
  has no Dirichlet constraint and is singular - see 1.7.
- Nothing can represent a far field, a symmetry plane, a permanent magnet or an
  externally imposed background field.
- A coil or a winding cannot be described as such. The only current the solver
  knows about is the one the electrostatics solve produced in a conducting body.

When an electrostatics task and a magnetostatics task share a task flow - which
they always do, see 1.6 - the condition tabs still offer the **electro-static**
conditions, because the tabs are filled from the problem types of the whole flow.
*Electric potential* and *Charge density* belong to the electrostatics task; none
of them reaches the magnetostatic assembly.

### 1.6 Coupling with other solvers

| Direction | Variable | Effect |
|---|---|---|
| Electro-statics -> magneto-statics | Current density | the only source of the magnetic field |

**Magneto-statics requires Electro-statics.** The dependency is declared in the
problem type, and the user interface enforces it: ticking *Magneto-statics* in
the problem selector ticks *Electro-statics* automatically, and unticking
*Electro-statics* unticks *Magneto-statics* with it. The default task flow places
the electrostatics task first, which is the order the coupling needs.

Nothing consumes the magnetic field. It is the end of its chain - no solver reads
it, and there is no Lorentz force, no induced current and no magnetic
contribution to any other physics.

Because the solver reports itself as converged unconditionally, a task group
containing a magnetostatics task always stops after one iteration.

### 1.7 Solving the system

The solver assembles and solves

```
K * B = f
```

with the **conjugate gradient** method and Jacobi preconditioning, the same
`CG` entry of the matrix solver setup that the electrostatics solver uses.

**The system is singular.** A Laplace problem needs its solution pinned
somewhere, and with no boundary condition available (1.5) nothing pins it: adding
the same constant to every node of a component leaves all three equations
satisfied, so the matrix has a three-dimensional null space of uniform fields.
This is not a property of a particular model - it is true of every magnetostatic
model the application can build.

**Nodes outside the volume mesh produce empty rows.** Because no material is
required, every element of the model is marked computable and every node enters
the system; because only volume elements are assembled, a node belonging solely
to a surface, line or point entity receives no matrix entry at all. Its diagonal
is zero. The Jacobi preconditioner stores a zero for a zero diagonal rather than
dividing by it, so the run does not fail - the row is simply carried along
unsolved. In a tetrahedralised solid whose surfaces wrap the volume this affects
nothing, since those nodes belong to volume elements too; a free-standing surface
or a stray point entity is where it shows.

Taken together, the solve completes and reports iterations, and the field it
returns is not the solution of a well-posed problem.

### 1.8 Results

| Result | Apply to | Meaning |
|---|---|---|
| Magnetic field `[T]` | node | the solved field vector |

That is the whole output. `process()` does nothing, so there is no derived
quantity - no field magnitude stored separately, no flux, no energy density, no
recomputed current density. The 3D view can show the magnitude and the components
of the vector variable in the usual way, and `Magnetic field in X direction` and
the `Y` and `Z` equivalents are available for component display.

The magnetic field is stored as a **node** variable, unlike the electrostatic
field results, which are element variables. It is directly comparable to the
electric potential in that respect, and monitoring points read it the same way.

After the record is written the solver log prints the statistics of the magnetic
field and the values at any monitoring points.

### 1.9 What the solver is, and what it is not

Two design choices set this solver apart from the magnetostatic solvers most
people will have met, and both are worth understanding before reading anything
into a result.

**It solves for the field, not for a potential.** A vector potential formulation,
`B = curl(A)`, satisfies `div(B) = 0` identically, whatever the mesh and whatever
the solution error. Solving for the three components of `B` directly does not:
nothing in the assembled system requires the divergence of the computed field to
vanish, and in general it will not. A field with a non-zero divergence is not a
magnetic field, so the closer that divergence is to zero the more the result is
worth - and there is no result stored that lets it be inspected.

**It is driven by the curl of a conduction current.** The source exists only
where the current density varies. There is no way to state a current directly, no
coil, no winding, no current sheet and no permanent magnet; the current has to be
produced by an electrostatics solve on a conducting body, and its own accuracy
sets a ceiling on the magnetic result. The electro-statics manual describes what
that current density is and is not.

Taken with what section 5 still lists, the honest summary is that this problem
type is **not finished**: the plumbing between the electrostatic and
magnetostatic tasks works and the element formulation is right, but until a
boundary condition exists to constrain the field the system it assembles is
singular and the level of its solution is arbitrary.

### 1.10 Static analysis and the time solver

Magneto-statics is declared as a problem type **without** a time solver, exactly
as electro-statics is. The *Time-solver* group box does not appear in the
`Problem` tab, the run produces a single record, and there is nothing to
configure.

Since a magnetostatics flow always contains an electrostatics task as well, and
neither supports a time solver, adding either to a flow removes the time solver
from the flow as a whole. A heat task chained behind them can only be
steady-state. The electro-statics manual covers this in its own section 1.10.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`, `Material`,
`Results` tabs), the **Problem** menu and the **Model** dock. The condition tabs
hold nothing for this problem type.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick **Magneto-statics** - "Calculate magnetic field".

**Electro-statics is ticked for you.** The selector enforces the dependency of
1.6 in both directions: ticking *Magneto-statics* ticks *Electro-statics*, and
unticking *Electro-statics* unticks *Magneto-statics*. The default task flow puts
the electrostatics task first.

Magneto-statics combines with every other physics except *Modal analysis*. In
practice the only chain that means anything is the one it is built on:

- *Electro-statics* then *Magneto-statics* - the magnetic field of a computed
  conduction current.

### 2.2 Problem tab

This tab is **empty** for an electro-statics plus magneto-statics flow. Neither
problem type has a time solver and neither has a setup group box of its own.

### 2.3 Material tab

Magneto-statics requires **nothing** from the material. The electro-statics task
it depends on requires three properties - relative permittivity, electrical
conductivity and density - and an entity whose material is missing any of them is
skipped by *that* task, which leaves it with no current density and therefore no
contribution to the magnetic source.

So the material still matters, entirely through the electrostatic half of the
chain. Assign a material with all three, such as the bundled *Copper*, and see
the electro-statics manual for what each is used for.

There is no relative permeability property to assign. The solver works in free
space.

### 2.4 Boundary conditions tab

Nothing in this tab belongs to the magnetostatic task. The conditions it offers -
*Electric potential* and *Charge density* - are the electro-static ones, and they
drive the current density which in turn drives the magnetic field. Assign them as
the electro-statics manual describes.

There is no magnetostatic boundary condition to assign, so there is no way to
prescribe the field, impose a symmetry plane, or close the model at a far
boundary. See 1.5.

### 2.5 Initial and environment conditions

Neither tab offers anything for magneto-statics. The *Electric potential* initial
condition and the *Temperature* environment condition shown belong to the
electro-statics task.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The magnetostatics solver uses the **CG** entry, which it shares with the
electrostatics solver.

Raising the iteration count will not rescue a magnetostatic solve. The system has
no constraint to converge towards - see 1.7 - and that is a property of the
problem type rather than of the setup.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record. *Magnetic field* is available in the list, along
with the electro-static variables, and reads the nodal field at the probe.

### 2.8 Results and records

The `Results` tab lists **Magnetic field** as a node vector, alongside the six
electro-static results the first task produced. The `Records` tab of the `Model`
dock holds exactly one record: both problem types are static.

`Report` -> `Solver log file` shows the full output, with the electrostatics
statistics first and the magnetic field statistics after them.

---

## 3. Tutorial - the field of a current-carrying conductor

**Goal.** Run the electro-statics and magneto-statics chain end to end on a
copper bar carrying a current, see where the magnetic field appears, and
understand what the numbers are and are not.

This tutorial assumes the meshed bar of the electro-statics manual - a box
`0.1 m` long with a `0.01 m` square section, tetrahedralised, with the two end
faces marked as separate surface entities. If that model is still set up, only
the task flow changes.

### Step 1 - select the problems

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Tick **Magneto-statics**. Watch **Electro-statics** tick itself.
3. Confirm with `OK`.

The task flow now holds two tasks, electro-statics first. The `Problem` tab stays
empty.

### Step 2 - assign the material

Select the volume entity and assign **Copper** in the `Material` tab, or a
material carrying the three properties the electro-statics task needs:

- **Relative permittivity** = `2.8`
- **Electrical conductivity** = `5.882e7` `S/m`
- **Density** = `8960` `kg/m^3`

Nothing here is read by the magnetostatic task. It is the electrostatic task
which needs them, and without a current density there is no magnetic source.

### Step 3 - drive a current

1. Select the surface entity at one end, open the `Boundary conditions` tab and
   tick **Electric potential**. Set it to `0` V.
2. Select the surface entity at the other end, tick **Electric potential** and
   set it to `0.01` V.

The current density that results is the whole input to the magnetostatic solve.
A bar with no potential difference carries no current, produces no source term,
and gives a magnetic field of exactly zero - which is the first thing to check if
the result comes out empty.

### Step 4 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. Follow the progress in `Report` -> `Solver log file`.

The log shows the electrostatics task with its six variables, then the
magnetostatics task with the magnetic field statistics. Both run once.

### Step 5 - inspect the result

- `Results` tab: display **Magnetic field**. It is a node vector, so the 3D view
  offers its magnitude and its three components.
- Expect the field to be **concentrated where the current density changes** -
  near the ends of the bar, at the corners, and wherever the mesh makes the
  computed current density jump from one element to the next. A uniform current
  produces no source at all, so the middle of a well-resolved bar is where the
  field is smallest, not largest. Section 1.4 explains why.
- Compare the pattern against the **Current density** result of the first task.
  The magnetic source follows the variation of that field, so the two should be
  read together.

### Step 6 - read the magnitude critically

The field is in tesla and the source term now carries the right constant, so the
numbers are of a sensible scale. What they are not is pinned: with no boundary
condition available, the level of the field is arbitrary and a uniform field may
be added to all three components without the solver noticing - see 1.7.

Read **differences** rather than values, then. The way the magnitude falls off
away from the conductor is meaningful; the value at any one point is not, and
neither is the field far from the current, where a well-posed model would return
something approaching zero. Section 4 gives the closed-form results to compare
the shape against.

What the run also demonstrates is the **chain**: that the electrostatics task
produced a current density, that the magnetostatics task picked it up, and that
the coupled flow completes.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The magnetic field is zero everywhere | no current density - check that the electro-statics task ran first, that the potentials differ, and that the material carries an electrical conductivity |
| **Magneto-statics** cannot be unticked on its own | expected - it requires *Electro-statics*, and unticking that unticks both |
| The *Time-solver* group box is missing | expected - neither problem type has a time solver, see 1.10 |
| The field is largest at the ends and corners | expected - the source is the curl of the current density, which is largest where that density varies |
| The field magnitude is not what a hand calculation gives | expected - the level of the field is unconstrained, see 1.7 and section 4 |
| Nothing appears in the `Results` tab for the magnetic field | the model has no volume entity; only volume elements are assembled |

---

## 4. Checking a model

The checks below are the ones that would validate a working magnetostatic solver.
They are given both because they are what to use once the defects of section 5
are fixed, and because running them now is the clearest way to see the size of
the gap.

**Straight conductor.** For a long conductor of radius `R` carrying a total
current `I` uniformly, the field is circumferential and

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

with `u0 = 1.2566e-6`. The field is largest at the surface of the conductor,
`B = u0*I/(2*pi*R)`, and falls off as `1/r` beyond it. For a `1 A` current in a
`5 mm` radius conductor that peak is `4e-5 T`, about the size of the earth's
field.

**Solenoid.** For a long solenoid of `n` turns per metre carrying `I`, the field
inside is uniform and along the axis,

```
B = u0 * n * I
```

and close to zero outside. This is the check that would exercise a coil, and it
is not one this solver can be asked to perform - there is no way to describe a
winding, see 1.5.

**Divergence.** For any magnetic field, `div(B) = 0` everywhere. Because this
solver works with the field components directly rather than with a potential,
nothing enforces it and nothing reports it - see 1.9. If the divergence could be
computed from the stored nodal field it would be the single most informative
check available, since it tests the formulation rather than any one model.

**What to expect today.** The source term now carries the right constant and the
right form, so the scale of the computed field is no longer wrong by
construction. What remains is the missing constraint of 1.7: with no boundary
condition to pin it, the level of the field is arbitrary and the solution is
determined only up to a uniform field added to all three components.

The practical consequence for these checks is that a **difference** between two
points may be meaningful where the value at either one is not. Comparing the fall
of the field between two radii against the `1/r` of the straight conductor
formula is the most that can be asked of the solver as it stands; comparing an
absolute magnitude against `u0*I/(2*pi*r)` is not, and agreement there would be
coincidence rather than validation.

---

## 5. Limitations

### Known defects

- **No boundary condition exists, so every model is singular.** Nothing can
  prescribe the field anywhere, which leaves the Laplace system with a
  three-dimensional null space of uniform fields. The solve runs and reports
  iterations; the level of the field it returns is arbitrary. This affects every
  magnetostatic model, not a particular one.
- **Nodes outside the volume mesh are left in the system unsolved.** No material
  property is required for the problem type, so every element is taken as
  computable and every node is given unknowns, while only volume elements are
  assembled. A node belonging solely to a surface, line or point entity gets a
  zero diagonal; the Jacobi preconditioner stores zero rather than dividing by
  it, so the run completes with those rows carried along untouched.

### Modelling limitations

- **Free space only.** No permeability enters the formulation. There is no
  relative permeability material property, no distinction between `B` and `H`,
  no magnetisation and no saturation, so iron, ferrites and shielding cannot be
  represented at all.
- **No permanent magnets and no imposed field.** The only source is the curl of a
  conduction current computed by an electro-statics task. A magnet, a background
  field and an external excitation have no representation.
- **No coils or windings.** A current can only be produced by an electrostatic
  solve on a conducting body. There is no current sheet, no filament, no turn
  count and no way to state a current directly.
- **A uniform current produces no field.** The source is a curl, so it is
  non-zero only where the current density varies from node to node. The result
  is correspondingly sensitive to the mesh and to the quality of the electrostatic
  current density it is built on.
- **The divergence of the field is not enforced.** The three components are
  solved as independent unknowns rather than derived from a potential, so
  `div(B) = 0` holds only to the extent the solution happens to satisfy it, and
  nothing measures how far off it is - see 1.9.
- **Volume elements only.** Surface, line and point entities contribute neither
  stiffness nor source, whatever geometric properties they carry.
- **No derived results.** `process()` computes nothing, so the magnetic field
  vector is the entire output. There is no flux, no energy, no force and no
  field magnitude stored as a variable of its own.
- **Nothing consumes the result.** No other solver reads the magnetic field.
  There is no Lorentz force on the conductor, no induced current and no coupling
  back into the electro-static, thermal or structural physics.
- **Static only.** There is no time solver, and adding a magnetostatics task -
  which brings an electrostatics task with it - removes the time solver from the
  whole task flow, so any other physics chained behind it is steady-state as
  well.
