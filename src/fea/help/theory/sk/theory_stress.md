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
6. [Limitations and known defects](#6-limitations-and-known-defects)

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

**Displacement** fixes **all three** global components of every node of the
entity, whatever values the three components are given. There is no way to
constrain one global direction and leave the other two free with this condition:
the three component values prescribe how far the node moves, not which degrees
of freedom are constrained. Use *Roller displacement* when a support has to
leave directions free.

**Normal displacement** and **Roller displacement** work in a **local frame**
built for each constrained node:

- on a **surface**, the local X axis is the averaged normal of the adjacent
  surface elements;
- on a **line**, the local frame is built from the element direction;
- on a **point**, the direction stored with the boundary condition is used
  directly.

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
it is divided by the entity length for a line and by the entity area for a
surface, so refining the mesh does not change the total load. On a point it is
applied directly to the node.

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
(`0` backward, `0.5` central, `1` forward):

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

Two methods are available, selected by the modal method setting:

| Modal method | Eigenvalue solver | Extracts |
|---|---|---|
| Multiple modes | Arnoldi iteration followed by a QR/Gram-Schmidt decomposition | the requested number of modes |
| Only most dominant mode | Rayleigh quotient iteration | a single mode |

The Arnoldi iteration works on `K^-1 * M`, so it converges towards the **lowest**
frequencies - which are the ones that matter. Each Arnoldi step is itself a
linear solve with `K`, using the conjugate gradient settings of the matrix
solver setup. The resulting eigenvalues are inverted and sorted in ascending
order, so **mode 0 is the fundamental mode**.

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

From the six components the solver forms two invariant-like measures and reports
their sum as the von Mises stress:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = QN + QS
```

For surface elements the two-dimensional equivalents are used, `QN` from the two
in-plane normal stresses and `QS = sqrt(3) * tau_xy`. For line elements only the
axial value is reported and the shear part is zero.

Be aware that the textbook von Mises stress is `sqrt(QN^2 + QS^2)`, not
`QN + QS`. The reported value is therefore an **upper bound**: it agrees with
the classical definition whenever one of the two parts vanishes - pure tension,
pure shear - and overestimates it by up to about 41 % when the two parts are
equal. See section 6.

**Nodal forces** are recovered element by element as

```
f = M * a + K * u
```

and accumulated at the nodes. At a constrained node this is the reaction force;
at a free node it should come out near zero for a converged static solve, which
makes the Force result a useful residual check. The `M * a` term is an inertia
contribution that is inactive in practice, see section 6.

| Result | Apply to | Meaning |
|---|---|---|
| Displacement `[m]` | node | displacement vector, or the mode shape in a modal analysis |
| Von Mises stress `[Pa]` | element | `QN + QS` as above |
| Normal stress `[Pa]` | element | `QN` |
| Shear stress `[Pa]` | element | `QS` |
| Force `[N]` | node | recovered nodal force, reaction at constrained nodes |

The individual stress components `sigma_xx` ... `tau_xy` are computed internally
but are not stored as results.

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
| Extract modes | how many modes to extract; capped by the iteration count and disabled for the dominant-mode method | 100 |
| Convergence value | eigenvalue solver convergence threshold | 1.0e-9 |

The default of 100 modes is generous. Extracting fewer modes is much faster, and
the low modes are the ones that matter - start with 5 to 10.

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

- **Displacement** constrains all three components of a node at once. Its three
  values say how far the node moves, not which directions are held. A support
  that has to leave a direction free is built with *Roller displacement*, not by
  editing the Displacement components.
- **Normal displacement** and **Roller displacement** show a *Local direction*
  editor in the condition tree. That entered direction is used only for **point**
  entities. On a surface the local frame comes from the averaged element
  normals, and on a line from the element direction, so the entered value is
  ignored there. If a surface constraint behaves oddly, check that the surface
  normals are consistent and synchronise them with the geometry tools.
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

For a modal record the 3D view labels the record with the eigenvalue of that
mode. Read section 6 before interpreting that number as a frequency.

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
3. Leave all three components at `0`.

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

Six modes is a sensible starting point. Extracting the default 100 modes on a
large mesh is slow, because every Arnoldi step is a full linear solve.

Choose `Only most dominant mode` when a single mode is all that is needed; it
uses Rayleigh quotient iteration and is much cheaper, but read the caveat in
section 6 about the eigenvalue it reports.

### Step 5 - no loads are needed

A modal analysis has no load vector - the eigenvalue problem is homogeneous.
Loads assigned to the model are ignored, with one exception: if a stress
analysis ran before, its displacement result deforms the mesh for the modal
assembly.

### Step 6 - solve

`Solution` -> `Start solver` (`Ctrl+R`). The log reports the Arnoldi iterations,
the QR decomposition and then one block per extracted mode:

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

### Step 8 - convert an eigenvalue to a frequency

The solver reports the eigenvalue `lambda = omega^2`. Convert it by hand:

```
f [Hz] = sqrt(lambda) / (2*pi)
```

For example, an eigenvalue of `1.0e6` corresponds to
`sqrt(1.0e6) / 6.2832 = 159 Hz`. The value is labelled *Freq.* with units of Hz
in the viewport but is not converted - see section 6.

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
| A frequency looks absurd | the reported number is the eigenvalue, not the frequency in Hz - convert it as in step 8 |
| Mode shapes look identical | closely spaced modes of a symmetric structure, or too few Arnoldi iterations to separate them |

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

## 6. Limitations and known defects

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

### Known defects

These are defects in the current implementation rather than deliberate
simplifications. They are listed so that results can be interpreted correctly.

- **The reported modal frequency is an eigenvalue, not a frequency.** The value
  stored by the modal setup and shown in the 3D view as *Freq.* with units of Hz
  is `lambda = omega^2` in `rad^2/s^2`. Convert it with
  `f = sqrt(lambda) / (2*pi)`.
- **The eigenvalue normalisation is inconsistent across methods.** The inversion
  and ascending sort that turn the raw iteration values into `omega^2` are
  applied only when more than one eigenvalue was extracted. Requesting exactly
  one mode with the *Multiple modes* method, or using *Only most dominant mode*,
  returns a value on a different scale. Extract at least two modes when the
  numerical value matters.
- **The von Mises stress is overestimated.** The solver reports `QN + QS` where
  the classical definition is `sqrt(QN^2 + QS^2)`. The two agree in pure tension
  and in pure shear, and the reported value is up to about 41 % too high when
  the normal and shear parts are comparable. It is always conservative, never
  unsafe, but it is not the von Mises stress.
- **Line element stress is scaled by the cross area.** For a line element the
  reported *Normal stress* is `E * A * eps`, which is the axial **force** in
  newtons rather than a stress in pascals. It coincides with the stress only for
  a unit cross area. The thermal term of the same expression carries a further
  factor of the cross area and is dimensionally inconsistent.
- **Individual stress components are not stored.** `sigma_xx` through `tau_xy`
  are computed during recovery but only the three derived measures are written
  out, so a directional stress check has to be made from those.
- **The inertia term of the nodal force recovery is always zero.** Nodal forces
  are recovered as `M*a + K*u`, but the nodal acceleration is only ever read
  back from a stored *Acceleration* result which no solver writes, and no
  boundary, initial or environment condition supplies it. The `M*a` term is
  therefore inert and the reported force is `K*u` alone.
- **The Force result is only offered for modal analysis in variable lists.** The
  static and transient solvers store it, but it is registered as a result of the
  modal problem type only, so it may not appear in the monitoring point variable
  selector for a plain stress analysis.
- **The Velocity initial condition has no effect.** It is offered for the stress
  problem type but the solver never reads it.
- **The optional flag of the Displacement boundary condition is inert.** The
  condition is declared as having optional components, but nothing in the
  application or the solver reads that flag, so all three displacement
  components are always constrained together.
- **The local direction of a surface constraint is ignored.** *Normal
  displacement* and *Roller displacement* offer a local direction editor for
  every entity type, but the entered direction is honoured only on point
  entities. Surfaces use their averaged element normals and lines use their
  element direction.
