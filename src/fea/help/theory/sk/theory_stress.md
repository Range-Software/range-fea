# Stress Analysis - Theory Manual and User Guide

This document describes the structural solver (`RSolverStress`) of Range FEA,
which serves both the **Stress analysis** and the **Modal analysis** problem
types: the equations it solves, the meaning of every input it accepts, the parts
of the graphical user interface that drive it, and two worked tutorials.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - static stress analysis](#3-tutorial---static-stress-analysis)
4. [Tutorial - modal analysis](#4-tutorial---modal-analysis)
5. [Checking a model](#5-checking-a-model)
6. [Limitations](#6-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the displacement field

The primary unknown is the **displacement vector** `u = (ux, uy, uz)` at every
node, in metres. Every node carries three degrees of freedom, so a mesh with
`N` nodes gives a system of `3N` equations. Strains, stresses and reaction
forces are all recovered from the displacement field after the solve.

### 1.2 Governing equations

The solver implements **linear, small-strain, isotropic elasticity**. Three
relations define it.

**Equilibrium** of a body under a volume load `b` and surface traction `t`:

```
div(sigma) + b = 0
```

**Strain-displacement** relation, small strain:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

**Constitutive** relation with a thermal term:

```
sigma = D * ( eps - alpha * dT )
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `u` | displacement | `m` | solved for |
| `sigma` | Cauchy stress | `Pa` | recovered |
| `eps` | engineering strain | `-` | recovered |
| `E` | modulus of elasticity | `Pa` | material property |
| `v` | Poisson ratio | `-` | material property |
| `rho` | density | `kg/m^3` | material property |
| `alpha` | thermal expansion coefficient | `1/K` | material property |
| `dT` | temperature rise above the stress-free state | `K` | see 1.5 |

All four material properties are **required** - an entity that is missing any of
them is not solved.

### 1.3 Elasticity matrix

**Volume elements** use the full three-dimensional isotropic elasticity matrix
in Voigt notation, with the stress vector ordered
`[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

**Surface elements** use the two-dimensional **plane strain** form with the
stress vector `[sigma_xx, sigma_yy, tau_xy]`, evaluated in the local element
plane and rotated back into global coordinates:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

This is plane **strain**, not plane stress. A thin plate loaded in its own plane
will therefore come out stiffer than a plane-stress idealisation would predict.

**Line elements** are treated as **trusses**: only the axial stiffness
`E * A` along the element direction is assembled, where `A` is the line cross
area. There is no bending, shear or torsional stiffness, even for element types
named "beam".

### 1.4 Finite element discretisation

The weak form of equilibrium gives, for each element,

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

where `B` is the strain-displacement matrix built from the shape function
derivatives. Every element matrix is `3n x 3n` for an element with `n` nodes,
with the degrees of freedom interleaved as
`[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Element measures.** As elsewhere in Range FEA, lower-dimensional entities are
given a geometric measure so that they can represent real bodies:

| Entity | Measure | Where it is set |
|---|---|---|
| Volume | 1 | - |
| Surface | surface thickness | entity geometric properties |
| Line | line cross area | entity geometric properties |
| Point | point volume | entity geometric properties |

A surface with zero thickness or a line with zero cross area contributes no
stiffness and no mass - it can still carry loads, which is exactly what is
wanted for a load-carrying face of a meshed solid.

The mass matrix is assembled only when it is needed, that is for a transient
analysis or for a modal analysis.

### 1.5 Loads, boundary and environment conditions

#### Boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Displacement | explicit (Dirichlet) | point, line, surface, volume | Displacement in X, Y, Z `[m]` |
| Normal displacement | explicit, local frame | surface | Displacement `[m]` |
| Roller displacement | explicit, local frame | point, line, surface | Displacement `[m]` |
| Force | natural | point, line, surface | Force in X, Y, Z `[N]` |
| Force (unit area) | natural | surface | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | natural | surface | Pressure `[Pa]` |
| Weight | natural | point, line, surface | Weight `[kg]` |

**Displacement** is an **optional**-component condition: each of its three
components can be switched on or off individually. A component that is switched
on constrains that global direction of every node of the entity to the given
value; a component that is switched off leaves that direction completely free.
Switching off *Displacement in Y* and *Displacement in Z* therefore builds a
support that holds X and lets the node slide in the YZ plane.

Use *Roller displacement* instead when the free directions are not the global
ones but the tangent plane of a curved surface.

**Normal displacement** and **Roller displacement** work in a **local frame**
built for each constrained node. By default the frame follows the geometry:

- on a **surface**, the local X axis is the averaged normal of the adjacent
  surface elements;
- on a **line**, the local frame is built from the element direction;
- on a **point**, there is no geometry to follow, so the direction entered with
  the boundary condition is always used.

Ticking **Use entered local direction** overrides that for a surface or a line
and takes the local X axis from the direction entered below it. Use it when the
direction you want to restrain is not the one the mesh happens to give - a
roller that should slide along a faceted surface along a global axis, for
example.

Constraints from different entities **combine** on a node they share. The
solver collects every displacement constraint acting on a node, each of them a
statement `d . u = v` about one direction, and reduces the collection to at most
three mutually perpendicular held directions by Gram-Schmidt, carrying the
prescribed values through the same operations. Those held directions become the
leading axes of the node frame; whatever is left over completes the frame and
stays free. A node may therefore be held in one direction by a global
*Displacement*, in another by a *Roller displacement* on a tilted face, and stay
free in the third - the two conditions do not overwrite one another and a
globally phrased component is never silently re-read in somebody else's local
frame.

Two entities which prescribe **different values in the same direction** cannot
both be satisfied. The solver detects that while orthogonalising and stops with
an error naming the node, rather than resolving it in favour of whichever
condition happened to be read last.

The element matrices and load vectors of a constrained node are rotated into
that frame, the constraint is applied, and the resulting displacement is rotated
back. In that frame:

- **Normal displacement** locks all three local components, and the prescribed
  scalar is applied along the local normal. The node moves by exactly the given
  amount along the normal and is fixed otherwise.
- **Roller displacement** locks only the local normal component on a surface or
  a point, so the node is free to slide in the tangent plane. On a line entity
  it locks the two components perpendicular to the line instead, so the node is
  free to slide along the line.

**Force** is a total force in newtons spread over the entity it is assigned to -
it is divided by the entity length for a line, by the entity area for a surface
and by the number of points of a point entity. Refining the mesh therefore does
not change the total load.

**Force (unit area)** is a traction in `N/m^2` applied in the given global
direction, integrated over the element area.

**Pressure** acts along the surface **normal**. The solver determines for every
boundary surface whether its normal points into the adjacent computable volume
element and flips the sign accordingly, so that a positive pressure always
pushes on the body. A surface that bounds no volume element is never classified
as inward, so there the load acts along the **negative** of the surface normal -
check the direction of the resulting load in that case.

**Weight** is a mass in kilograms attached to the entity. It is multiplied by
the gravitational acceleration of the environment condition and spread over the
entity in the same way as Force. Use it for non-modelled attached masses.

#### Environment conditions

| Environment condition | Components | Effect |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | self weight and the Weight boundary condition |
| Temperature | Temperature `[K]` | stress-free reference temperature |

**Self weight** is generated automatically for every entity that has both a
density and a geometric measure, as `rho * g` integrated over the element. It
appears as soon as a gravitational acceleration environment condition is
assigned; the default is `-9.80665 m/s^2` in Z.

**Thermal load.** The temperature difference driving the thermal expansion term
is

```
dT = element temperature - environment temperature
```

The **environment temperature** comes from the *Temperature* environment
condition and is the stress-free reference state. The **element temperature**
comes from a heat transfer solver running earlier in the same problem task flow,
handed over through the solver shared data. Running *Heat transfer* and then
*Stress analysis* in one task flow therefore gives a thermal stress analysis
with no further setup. Without a heat solver the element temperature stays at
its default and the thermal term is the difference against the environment
condition alone.

#### Initial conditions

| Initial condition | Effect |
|---|---|
| Displacement | initial displacement field, applied on the first run |
| Pressure | initial surface pressure load |
| Velocity | offered, but not read by the stress solver - see 6 |

#### Coupling with a fluid solver

The solver recovers the **Pressure** result of a previous solve and uses it as a
surface load wherever no explicit pressure boundary condition is assigned. A
fluid task followed by a stress task in the same task flow therefore transfers
the fluid pressure onto the structure automatically.

In the other direction, every non-structural solver moves the mesh by the stored
**Displacement** result before it runs and moves it back afterwards, so a fluid
or thermal solve that follows a stress solve sees the deformed shape.

### 1.6 Static analysis

With the time solver disabled and the problem type set to *Stress analysis*, the
solver assembles and solves

```
K * u = f
```

`K` is symmetric positive definite once the rigid body modes have been removed
by the displacement constraints, so it is solved with the **conjugate gradient**
method with Jacobi preconditioning and a block size of 3.

If the model is under-constrained the matrix is singular and the solve will not
converge. Every static model needs enough constraints to suppress all six rigid
body modes - three translations and three rotations.

### 1.7 Transient analysis

With the time solver enabled, the solver marches

```
M * du/dt + K * u = f
```

with a theta scheme, where `alpha` is the time-march approximation coefficient
(`1` backward, `0.5` central, `0` forward):

```
( M + alpha*dt*K ) * u_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * u_n
```

Note that this is a **first-order** march. There is no second time derivative of
the displacement in the assembled system, so this is a relaxation-type transient
rather than true elastodynamics: it does not reproduce inertia waves, ringing or
resonance. Use it for a load history applied slowly compared with the structural
periods, and use *Modal analysis* to learn where the structural periods are.

### 1.8 Modal analysis

With the problem type set to *Modal analysis*, the solver assembles `K` and `M`
and solves the generalised eigenvalue problem

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

Each eigenpair is a **mode**: `phi` is the mode shape, stored and displayed as a
displacement field, and `lambda` is the eigenvalue from which the natural
frequency follows as

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

The solver performs that conversion itself: the modal setup and the 3D view
report the natural frequency in Hz, while the solver log prints the underlying
eigenvalue alongside it. A mode that the eigenvalue iteration failed to resolve
is reported as `0` Hz with a warning in the log.

Two methods are available, selected by the modal method setting:

| Modal method | Eigenvalue solver | Extracts |
|---|---|---|
| Multiple modes | subspace iteration with a Rayleigh-Ritz projection | the requested number of modes |
| Only most dominant mode | inverse power iteration with a Rayleigh quotient | a single mode |

Both methods iterate on `K^-1 * M`, so they converge towards the **lowest**
frequencies - which are the ones that matter. Each step is a linear solve with
`K`, using the conjugate gradient settings of the matrix solver setup, so the
cost of a modal analysis is roughly the cost of a static solve multiplied by the
number of subspace vectors and iterations.

Subspace iteration carries a block of vectors, projects `K` and `M` onto the
subspace they span and solves the resulting small dense eigenproblem exactly.
A few extra vectors beyond the requested number of modes are carried along,
because they make the wanted modes converge markedly faster. For `p` requested
modes the block holds `min(2p, p+8)` vectors, which is the usual compromise
between the speed of convergence and the cost of an iteration. The eigenvalues
come out as `lambda` directly and are sorted ascending, so **mode 0 is the
fundamental mode**.

**This is where the run time goes.** One subspace iteration performs one full
linear solve per vector of the block, so asking for 10 modes costs 18 solves per
iteration and asking for 100 costs 108. The iteration stops early only when
**every** requested eigenvalue has settled to within the convergence value, and
the highest requested mode is always the last to settle - so a large mode count
both makes each iteration expensive and makes the early stop unlikely. Ask for
the modes you actually intend to look at.

If the requested modes have not settled by the last iteration the solver says so
in the log and reports the change it reached, rather than presenting the
unconverged values without comment. The lowest modes of such a run are usually
still sound; the highest are not.

The mass matrix is always assembled for a modal analysis, so the density must be
present on every entity that takes part.

**Prestressed modal analysis.** Before assembling the matrices, a modal analysis
moves every node by the displacement result of a previous solve, and restores
the mesh afterwards. Running *Stress analysis* and then *Modal analysis* in one
task flow therefore gives the modes of the **loaded, deformed** structure rather
than the undeformed one. The stored mode shape is the eigenvector added on top
of that initial displacement.

Only the geometry is updated - the stiffness matrix is still the linear elastic
one evaluated on the moved mesh. This captures the change of shape, not the
geometric stiffening of a tensioned member.

Modal analysis is **exclusive**: it cannot be combined with any other physics
except a preceding stress analysis.

### 1.9 Derived results

After the solve the element stresses are recovered from the displacement field
as the average over the integration points of

```
sigma = D * B * u  -  D * alpha * dT
```

From the six components the solver forms two invariants and combines them in
quadrature into the von Mises stress:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

which is the classical definition

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

For surface elements the two-dimensional equivalents are used, `QN` from the two
in-plane normal stresses and `QS = sqrt(3 * tau_xy^2)`. For line elements only
the axial value is reported and the shear part is zero.

**Nodal forces** are recovered element by element as the internal elastic force

```
f = K * u
```

and accumulated at the nodes. At a constrained node this is the reaction force;
at a free node it should come out near zero for a converged static solve, which
makes the Force result a useful residual check.

| Result | Apply to | Meaning |
|---|---|---|
| Displacement `[m]` | node | displacement vector, or the mode shape in a modal analysis |
| Von Mises stress `[Pa]` | element | `sqrt(QN^2 + QS^2)` as above |
| Normal stress `[Pa]` | element | `QN` |
| Shear stress `[Pa]` | element | `QS` |
| Normal stress in X, Y, Z `[Pa]` | element | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | element | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | node | recovered nodal force, reaction at constrained nodes |

The six stress components are reported in **global** coordinates for volume
elements. Surface and line elements evaluate their stress in their own local
element frame, so their components are reported in that frame: a surface fills
the in-plane `sigma_xx`, `sigma_yy` and `tau_xy`, and a line fills the axial
`sigma_xx` only.

### 1.10 Static, transient or modal?

| | Static | Transient | Modal |
|---|---|---|---|
| Solves | `K*u = f` | `M*u' + K*u = f` in time | `K*phi = omega^2*M*phi` |
| Answers | "how much does it deflect and how highly is it stressed" | "how does it settle as the load changes" | "at which frequencies does it want to vibrate, and in what shape" |
| Needs mass | no | yes | yes |
| Needs constraints against rigid body motion | yes | yes | yes |
| Records | one | one per time step | one per extracted mode |

**Use a static analysis for** strength and stiffness checks, deflection limits,
reaction forces, self weight, pressure vessels, thermal stress. This is the
default and covers most work.

**Use a transient analysis for** a load that changes over time and where the
structure follows it quasi-statically. Do not expect dynamic amplification.

**Use a modal analysis for** natural frequencies and mode shapes, avoiding
resonance with a known excitation, judging whether a structure is stiff enough
dynamically, and checking for unintended mechanisms - a near-zero eigenvalue
usually means a missing constraint or a disconnected part.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Environment conditions`,
`Material`, `Results` tabs), the **Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick either:

- **Stress analysis** - "Stress and linear elasto-dynamics analysis including
  modal analysis";
- **Modal analysis** - "Stress modal analysis".

Stress analysis combines freely with other physics. Two useful chains are
*Heat transfer* then *Stress analysis* for thermal stress, and
*Incompressible viscous flow* then *Stress analysis* for pressure loading.

Modal analysis excludes every other problem type except a preceding stress
analysis, which turns it into a prestressed modal analysis.

### 2.2 Problem tab

**Time-solver** - shown for stress analysis, since it is a time-dependent
problem type. Leave it disabled for a static analysis.

| Field | Meaning |
|---|---|
| Enable | switches the transient march on and off |
| Approximation | theta of the time march: backward, central or forward |
| Start time | first time value in seconds; ignored on a solver restart |
| End time | `start time + time-step size * number of time-steps`, read-only |
| Time-step size | `dt` in seconds |
| Number of time-steps | how many steps to compute |
| Output frequency | write a result file every N steps; `0` writes only the last step |

**Modal analysis setup** - shown when the problem type is *Modal analysis*.

| Field | Meaning | Default |
|---|---|---|
| Modal method | *Only most dominant mode* or *Multiple modes* | Multiple modes |
| Iterations | iterations of the eigenvalue solver; disabled for the dominant-mode method | 100 |
| Extract modes | how many modes to extract; disabled for the dominant-mode method | 10 |
| Convergence value | eigenvalue solver convergence threshold | 1.0e-9 |

The two counts are independent: the iterations refine the whole set of modes
together, they do not produce them one at a time.

**Extract modes is the setting that decides how long the run takes.** Each mode
adds a linear solve to every iteration, and the run can only stop early once all
of the requested modes have settled, so the cost grows faster than the count.
Ten is a sensible starting point and is the default; a hundred is a different
order of calculation, and on a large mesh it can take hours. Raise it only when
you know you need the higher modes.

If a run does take longer than you expect, the solver log names the subspace
vector it is working on, so the progress within an iteration is visible, and it
prints the convergence rate reached after each iteration next to the value
required.

### 2.3 Material tab

The `Material` tab lists the material assigned to the selected entity. Stress
analysis requires **all four** of these:

| Property | Units | Comment |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, about `2.1e11` for steel |
| Poisson ratio | `-` | `v`, about `0.3` for steel; must stay below `0.5` |
| Density | `kg/m^3` | needed for self weight, transient and modal analysis |
| Thermal expansion coefficient | `1/K` | set to `0` if there is no thermal load |

An entity missing any of them is not solved, and the setup checker warns before
the run starts. Every property is a table against temperature, so a
temperature-dependent modulus is honoured when a heat solver runs first.

### 2.4 Boundary conditions tab

The tab is split into a manager list of available boundary conditions on top and
an editor for the selected one below. Select an entity in the `Model` tree
first; the manager then offers the conditions valid for that entity type and for
the selected problem. Tick one to assign it, then edit its components in the
lower tree.

The conditions of section 1.5 are offered. Notes on using them:

- **Displacement** shows a check box next to each of its three components. Only
  the checked components are constrained, so a face can be held in X and left
  free in Y and Z. The value box of an unchecked component is greyed out.
- **Normal displacement** and **Roller displacement** take their local frame
  from the geometry by default - the averaged element normals of a surface, the
  element direction of a line. On a **point** entity there is no geometry to
  follow, so the *Local direction* editor is always shown and always used.
- On a surface or a line the condition tree shows a **Use entered local
  direction** check box. Tick it to reveal the *Local direction* editor and take
  the frame from the direction you enter instead of from the mesh. The direction
  arrows drawn in the 3D view follow whichever of the two is in effect. If a
  surface constraint behaves oddly with the box unticked, check that the surface
  normals are consistent and synchronise them with the geometry tools.
- Constraints from several entities meeting at one node are combined, so a face
  may be held in a global direction by *Displacement* and rolled on a tilted
  plane at the same time. If two of them ask for different values in the same
  direction the run stops with an error naming the node.
- **Pressure** and **Force (unit area)** apply to surfaces only.
- Every component is a table against time, so a load can be given a time history
  for a transient run.

### 2.5 Initial and environment conditions

The `Environment conditions` tab is where **gravity** is switched on. Assign
*Gravitational acceleration* to the entity that should carry its self weight -
usually the whole model - and set the vector; the default is `-9.80665` in Z.
Assign *Temperature* to set the stress-free reference temperature for a thermal
stress analysis.

The `Initial conditions` tab offers *Displacement* and *Pressure*, applied on
the first run of a transient analysis.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The structural solver uses the **CG** entry, both for the static and transient
solve and for the inner solves of the eigenvalue iteration.

If a static solve does not converge, the usual cause is an under-constrained
model rather than a solver setting. Check the constraints before raising the
iteration count.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record - typically *Displacement* or *Von Mises stress*.
In a transient run the history is shown by `Report` -> `Monitoring points`.

### 2.8 Results and records

The `Results` tab lists the computed variables and controls the 3D view.
Displacement is a node vector; the three stress measures are element scalars.

For a deformed-shape plot, display *Displacement* and enable the deformation
scaling in the view settings - real displacements are usually far too small to
see at true scale.

The `Records` tab of the `Model` dock lists the result records:

- **static analysis** - a single record;
- **transient analysis** - one record per written time step;
- **modal analysis** - one record per extracted mode, numbered from the
  fundamental. Stepping through the records steps through the modes, and
  animating them animates the mode shapes.

For a modal record the 3D view labels the record with the mode number and the
natural frequency of that mode in Hz.

`Report` -> `Solver log file` shows the full solver output, including the
eigenvalue printed for each extracted mode and the statistics of displacement,
von Mises stress and force after each record.

---

## 3. Tutorial - static stress analysis

**Goal.** Compute the deflection and stress of a cantilever loaded by its own
weight and a tip load, and read the reaction at the support.

This tutorial assumes a meshed volume model - a long box works well. Use
`File` -> `New model`, draw a box with `Geometry` -> `Create element` or import
a geometry, and tetrahedralise it; the *Draw cube* tutorial covers this ground.
Mark the fixed end and the loaded end as separate surface entities so that
boundary conditions can be assigned to them.

### Step 1 - select the problem

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add a task and select **Stress analysis**.
3. Confirm with `OK`.

### Step 2 - assign the material

1. Select the volume entity in the `Model` tree.
2. Open the `Material` tab and assign a material with all four properties, for
   structural steel:
   - **Modulus of elasticity** = `2.1e11` `Pa`
   - **Poisson ratio** = `0.3`
   - **Density** = `7850` `kg/m^3`
   - **Thermal expansion coefficient** = `1.2e-5` `1/K`

### Step 3 - keep the analysis static

Open the `Problem` tab and leave **Enable** unticked in the *Time-solver* group
box. The solver then assembles and solves `K*u = f` once.

### Step 4 - constrain the model

1. Select the surface entity at the fixed end.
2. Open the `Boundary conditions` tab and tick **Displacement**.
3. Leave all three components checked and set to `0`.

This fixes the face completely and removes all six rigid body modes. Check that
nothing else is left floating - a disconnected part with no constraint makes the
system singular and the solve will not converge.

For a support that should slide rather than being fully fixed, use **Roller
displacement** on the surface instead. It locks only the surface normal
direction and leaves the node free to move in the tangent plane.

### Step 5 - apply the loads

**Self weight.** Select the volume entity, open the `Environment conditions` tab
and tick **Gravitational acceleration**. Leave the default `-9.80665` in Z.

**Tip load.** Select the surface entity at the free end, open the
`Boundary conditions` tab and tick **Force**. Enter the total force in newtons,
for example `-1000` in Z. The value is the total over the entity, so it does not
change when the mesh is refined.

Alternatives, depending on what the load physically is:

- **Pressure** for a uniform normal pressure on a face, in `Pa`;
- **Force (unit area)** for a directed traction in `N/m^2`;
- **Weight** for an attached mass in `kg` that is not modelled as geometry.

### Step 6 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. The setup checker reports missing materials or missing boundary conditions
   before the run starts.
3. Follow the progress in `Report` -> `Solver log file`.

### Step 7 - inspect the results

- `Results` tab: display **Displacement** to see the deflected shape, and
  **Von Mises stress** for the stress distribution. Enable deformation scaling
  in the view settings to make the deflection visible.
- **Normal stress** and **Shear stress** separate the two contributions that
  make up the reported von Mises value.
- **Force** at the fixed face is the reaction. Its sum should balance the
  applied load plus the self weight - a quick and effective check that the model
  is loaded the way you intended.
- A monitoring point on *Displacement* at the tip gives the deflection as a
  number rather than a colour.

### Step 8 - optional, thermal stress

To add a thermal load:

1. Add a **Heat transfer** task *before* the stress task in the problem task
   flow, and set up its boundary conditions and material properties.
2. Assign a **Temperature** environment condition to the structural entity with
   the stress-free reference temperature, for example `293.15` K.

The stress solver picks up the computed element temperatures from the heat
solver and adds the thermal expansion term automatically.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The solve does not converge | the model is under-constrained; a part is floating or the supports do not remove all six rigid body modes |
| *Entity has material assigned which is missing required properties* | one of the four required properties is absent |
| Displacements are zero everywhere | no load reached the model - check that gravity is assigned, or that the Force components are non-zero |
| Displacements are enormous | the modulus of elasticity is wrong by orders of magnitude, or a surface entity has zero thickness and so no stiffness |
| The load changes when the mesh is refined | a *Force (unit area)* or *Pressure* was intended as a total force, or the reverse |
| Pressure pushes the wrong way | the surface normals are not consistent, or the surface does not bound a volume |

---

## 4. Tutorial - modal analysis

**Goal.** Extract the first few natural modes of the same cantilever.

### Step 1 - select the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`), add a task, select
**Modal analysis**.

Modal analysis cannot be combined with other physics, with the single exception
of a stress analysis placed before it, which makes it a prestressed modal
analysis.

### Step 2 - assign the material

The same four properties as in the static tutorial. The **density** matters more
here than anywhere else: it is the entire mass matrix, and the frequencies scale
as `1/sqrt(rho)`.

### Step 3 - constrain the model

Assign the same **Displacement** boundary condition at the fixed end.

Constraints are what distinguish the modes of a cantilever from those of a free
floating bar. If the model is left unconstrained the first six eigenvalues will
be near zero - the rigid body modes - and the interesting modes will be pushed
further up the list.

### Step 4 - configure the modal setup

Open the `Problem` tab and in *Modal analysis setup* set:

| Field | Value |
|---|---|
| Modal method | `Multiple modes` |
| Iterations | `100` |
| Extract modes | `6` |
| Convergence value | `1.0e-9` |

Six modes is a sensible starting point, and fewer than the default ten. Every
extra mode adds a linear solve to every iteration, so a large mode count on a
large mesh is slow.

Choose `Only most dominant mode` when a single mode is all that is needed; it
uses inverse power iteration on a single vector and is much cheaper.

### Step 5 - no loads are needed

A modal analysis has no load vector - the eigenvalue problem is homogeneous.
Loads assigned to the model are ignored, with one exception: if a stress
analysis ran before, its displacement result deforms the mesh for the modal
assembly.

### Step 6 - solve

`Solution` -> `Start solver` (`Ctrl+R`). The log reports the subspace iterations
and their convergence rate, and then one block per extracted mode:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

Modes are processed from the highest extracted index down to the fundamental, so
the fundamental is the last one computed and is stored as record 1.

### Step 7 - inspect the modes

- `Model` dock, `Records` tab: one record per mode, numbered from the
  fundamental. Step through them to see each mode shape.
- `Results` tab: display **Displacement**. A mode shape is only defined up to a
  scale factor, so the magnitudes are arbitrary - what matters is the shape.
  Enable deformation scaling and animate the record to visualise the mode.
- The eigenvalue of each mode is printed in the solver log and shown as the
  record label in the 3D view.

### Step 8 - read the natural frequencies

The 3D view labels each modal record with its mode number and its natural
frequency in Hz. The solver log lists the same frequency for every extracted
mode together with the underlying eigenvalue:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

A mode the eigenvalue iteration could not resolve is reported as `0` Hz and
carries a warning in the log.

As with any result, it is worth checking the fundamental frequency against a
hand calculation the first time a model is set up - section 5 gives the formula
for a cantilever.

### Step 9 - optional, prestressed modes

1. In the problem task flow, place a **Stress analysis** task *before* the
   **Modal analysis** task.
2. Set up the static loads and constraints as in tutorial 3.

The modal analysis then runs on the deformed mesh. This is the way to see how a
static load changes the shape a structure vibrates in.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The first eigenvalues are near zero | the model is under-constrained, or a part is disconnected - a genuine mechanism |
| The eigenvalue solver does not converge | too few iterations, or an ill-conditioned mass matrix because an entity has no density or no geometric measure |
| Extracting modes is very slow | too many modes requested; each mode costs a full linear solve |
| The run takes hours, or never seems to end | too many modes requested - each one costs a linear solve in every iteration, and the run cannot stop early until all of them have settled. Watch the subspace vector count in the log to see the progress within an iteration |
| A frequency looks absurd | the subspace iteration has not converged - the log says so at the end of an unconverged run; extract fewer modes, raise the iteration count, or check the density and the constraints |
| Mode shapes look identical | closely spaced modes of a symmetric structure, or too few subspace iterations to separate them |
| Fewer records than the modes requested | the subspace lost independent directions; the log says how many modes were extracted and stores only those |

---

## 5. Checking a model

The solver has no built-in verification suite, so it is worth validating a new
model against something you can compute by hand before trusting it. Three cheap
checks, in order of usefulness:

**Reaction balance.** Sum the *Force* result over the constrained nodes. It must
balance the applied loads plus the self weight `rho * V * g`. This catches wrong
load magnitudes, wrong units and wrong pressure directions.

**Cantilever tip deflection.** For a beam of length `L`, second moment of area
`I` and end load `F`, Euler-Bernoulli theory gives

```
delta = F * L^3 / (3 * E * I)
```

Model the beam with volume elements and compare. Expect the finite element
result to be slightly stiffer on a coarse mesh, converging from below as the
mesh is refined. Do not use line elements for this check - they carry no bending
stiffness.

**Cantilever fundamental frequency.** For the same beam with cross section area
`A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Compare against `sqrt(lambda_0) / (2*pi)` from the modal analysis. This
simultaneously validates the mass matrix, the stiffness matrix and your
eigenvalue conversion.

For any of these, refine the mesh at least once and confirm the answer is
converging rather than drifting.

---

## 6. Limitations

### Modelling limitations

- **Linear elasticity only.** Small strains, small displacements, isotropic
  material, no plasticity, no contact, no large deformation and no buckling.
  Results are meaningful only while the structure stays in its linear range.
- **Transient analysis is first order.** The assembled system is
  `M*u' + K*u = f`, not `M*u'' + C*u' + K*u = f`. There is no second time
  derivative and no damping matrix, so inertia waves, dynamic amplification and
  resonant response are not reproduced. Treat it as a relaxation-type march for
  slowly changing loads, and use modal analysis for dynamic properties.
- **Surface elements are plane strain.** A thin plate loaded in its own plane
  will be stiffer than a plane stress idealisation. Model thin structures with
  volume elements when the difference matters.
- **Line elements are trusses.** Only axial stiffness `E*A` is assembled. There
  is no bending, shear or torsion, whatever the element type is called, so a
  frame modelled with line elements will behave as a pin-jointed truss.
- **Prestress is geometric only.** A prestressed modal analysis re-evaluates the
  linear stiffness on the moved mesh. It does not add a geometric stiffness
  term, so the stiffening of a tensioned cable or the softening of a compressed
  strut is not captured.
- **No modal participation factors or effective masses.** The solver reports
  eigenvalues and mode shapes only, so there is no direct indication of which
  modes matter for a given excitation direction.
