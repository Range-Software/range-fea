# Flow through porous media - Theory Manual and User Guide

This document describes the porous media flow solver (`RSolverPotential`) of
Range FEA, which serves the **Flow through porous media** problem type: the
equations it solves, the meaning of every input it accepts, the parts of the
graphical user interface that drive it, and two worked tutorials.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - pressure driven flow through a sand filter](#3-tutorial---pressure-driven-flow-through-a-sand-filter)
4. [Tutorial - upward seepage through a vertical column](#4-tutorial---upward-seepage-through-a-vertical-column)
5. [Checking a model](#5-checking-a-model)
6. [Limitations](#6-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the pressure field

The primary unknown is the **pressure** `p` of the pore fluid at every node, in
pascals. Every node carries a single degree of freedom, so a mesh with `N` nodes
gives a system of `N` equations - as cheap as a heat or an electrostatic model of
the same mesh. The velocity of the fluid is recovered from the pressure field
after the solve.

A pressure field governed by this equation is only defined **up to an additive
constant**: adding the same value to the pressure everywhere changes no velocity.
Every model therefore needs at least one *Pressure (explicit)* boundary
condition, see 1.7. Whether the pressure is read as absolute or as gauge pressure
is up to the user - the solver only ever works with its gradient.

### 1.2 Governing equations

The solver implements **Darcy's law** for the slow flow of a single
incompressible fluid that fills the pores of a rigid porous body:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

together with conservation of mass of an incompressible fluid,

```
div( v ) = 0
```

which combine into a potential (Laplace type) equation for the pressure:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `p` | pore fluid pressure | `Pa` | solved for |
| `v` | Darcy (superficial) velocity | `m/s` | recovered, see 1.8 |
| `k` | intrinsic permeability of the porous body | `m^2` | material property *Permeability to fluids* |
| `mu` | dynamic viscosity of the pore fluid | `kg/(m*s)` | material property *Dynamic viscosity* |
| `rho` | density of the pore fluid | `kg/m^3` | material property *Density* |
| `g` | gravitational acceleration | `m/s^2` | environment condition, see 1.6 |

The ratio `k/mu` plays the role the conductivity plays in a heat model. With no
gravity the equation is Laplace's equation for the pressure, and the flow runs
straight down the pressure gradient. With gravity, the term `rho*g` makes a fluid
at rest carry a hydrostatic pressure gradient: `grad(p) = rho*g` gives `v = 0`.

**The velocity is the Darcy velocity.** It is the volumetric flow rate per unit
of **total** cross section, solid and pores together - not the speed of the fluid
inside the pores. The mean speed in the pores is larger by the inverse of the
porosity `n`:

```
v_pore = v / n
```

The porosity does not enter a steady flow, so the solver neither needs nor
accepts it. Use it by hand to estimate travel times.

**Permeability and hydraulic conductivity.** Soil mechanics and hydrogeology
often quote the *hydraulic conductivity* `K` in `m/s` (the coefficient of
Darcy's law written for the hydraulic head) instead of the intrinsic
permeability. The two are related through the properties of the fluid:

```
k = K * mu / ( rho * |g| )
```

For water at 20 °C, `K = 1e-4 m/s` corresponds to `k = 1.02e-11 m^2`. Petroleum
engineering quotes the permeability in darcy: `1 D = 9.869e-13 m^2`.

**Required material properties.** A porous media flow problem requires **three**
of them:

| Property | Units | Used for |
|---|---|---|
| Permeability to fluids | `m^2` | the stiffness matrix - permeability of the solid |
| Dynamic viscosity | `kg/(m*s)` | the stiffness matrix - viscosity of the **fluid** |
| Density | `kg/m^3` | the gravity term - density of the **fluid** |

**Density and viscosity are those of the pore fluid, not of the porous body.**
A material for this problem type therefore describes the *saturated* medium: the
permeability of the solid skeleton and the density and viscosity of the fluid
that fills it. The bundled materials named *... (water saturated)* are built that
way (see 2.3). A material that gives the bulk density of a dry soil makes the
hydrostatic pressure wrong by the ratio of the two densities.

An entity whose material is missing any of the three is **not solved**. The setup
checker warns about it before the run starts. The one exception is an entity that
carries a boundary condition of this problem type: that entity is included in
the system whatever its material says, and its permeability is then simply zero.
That is how a boundary face without a material receives an inflow.

Every property is a table against temperature. A porous media flow task on its
own has no temperature field to read it at, so every table is evaluated at
`293.15 K`. The tables are honoured only when a *Heat transfer* task earlier in
the same task flow has computed element temperatures - which is how a warm fluid
of lower viscosity flows faster.

### 1.3 Finite element discretisation

The weak form of the pressure equation gives, for each element,

```
K = INT( B^T * ( k / mu ) * B )                  stiffness
f = INT( B^T * ( k / mu ) * rho * g )            gravity load
  + INT_S( N^T * v_in )                          inflow through the boundary
```

where `B` is built from the shape function derivatives, `N` are the shape
functions and `v_in` is the inflow velocity prescribed on a boundary surface.
Every element matrix is `n x n` for an element with `n` nodes. There is no mass
or storage matrix: the flow is steady, see 1.10.

**Element measures.** As elsewhere in Range FEA, lower-dimensional entities are
given a geometric measure so that they can represent real bodies:

| Entity | Measure | Where it is set |
|---|---|---|
| Volume | 1 | - |
| Surface | surface thickness | entity geometric properties |
| Line | line cross area | entity geometric properties |
| Point | - | points carry no flow |

A **surface** with a thickness carries flow in its own plane, which is how a
two-dimensional model (a cross section of a dam or of an aquifer) is built. Its
gravity load uses the component of `g` that lies in the plane of the surface. A
surface of **zero thickness** - the usual boundary face of a volume model - has
no stiffness and only receives an inflow.

A **line** with a cross area carries flow along its own direction, for a
one-dimensional model of a column or a pipe filled with packing. A **point**
contributes nothing. A point element whose node belongs to no other computable
element leaves an empty row in the matrix - do not give point entities a porous
material.

### 1.4 Boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Pressure (explicit) | explicit (Dirichlet) | surface | Pressure `[Pa]` |
| Velocity (inflow) | natural (prescribed flux) | surface | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | natural (prescribed flux) | surface | Volumetric flow rate `[m^3/s]` |

A boundary surface that carries none of them is **impermeable**: no fluid
crosses it. That is the natural condition of the equation, so walls, the bottom
of an aquifer and planes of symmetry need no condition at all.

**Pressure (explicit)** is the only explicit condition. Every node of the
surface is removed from the system of equations and held at the given pressure.
It is where the fluid leaves the model (an outlet, a drain, the free water of a
reservoir) or enters it under a known pressure. Every model needs at least one.
A newly assigned condition starts at `1000 Pa`.

**Velocity (inflow)** prescribes the normal Darcy velocity through the surface.
A **positive** value is an inflow into the porous body, a **negative** value an
outflow (extraction). The value is a flux per unit area, so the total flow rate
grows with the area of the surface.

**Volumetric flow rate (inflow)** prescribes the **total** flow rate through the
surface in `m^3/s`. The solver divides it by the area of the whole surface entity
and applies the result as a uniform inflow velocity, so the flow rate stays put
when the surface is remeshed or enlarged. Positive is again an inflow.

Neither inflow condition fixes the pressure: the pressure at the inlet is part of
the answer. Where an inflow surface and a *Pressure (explicit)* surface share
nodes, the pressure wins at those nodes.

An inflow surface needs **no material and no thickness**: the condition alone
makes it part of the system. If a surface does carry a porous material and a
thickness, it also conducts flow in its own plane.

The *Pressure (implicit)* condition of the incompressible flow solver is not
offered for this problem type.

All components accept a table against time, which takes effect when the time
solver is enabled, see 1.10.

### 1.5 Gravity

Gravity enters through the *Gravitational acceleration* environment condition.
**Every entity carries one by default, with `g = (0, 0, -9.80665) m/s^2`**, so
gravity points along the negative Z axis unless it is changed. Two consequences
are worth knowing:

- In a model whose Z axis points up, a fluid at rest automatically gets a
  hydrostatic pressure that grows by `rho*|g|` per metre of depth. That is
  usually what is wanted.
- A model built with another axis pointing up must have the condition edited on
  every porous entity, otherwise the fluid "falls" sideways. Set all three
  components to zero to switch gravity off - for instance for a horizontal model
  in which only the pressure difference is of interest.

Gravity combined with a *Pressure (explicit)* surface that is not horizontal
needs care: a uniform pressure on a vertical face is **not** hydrostatic, so
fluid circulates near that face. Either give the pressure as a function of the
height (split the face into several entities), or switch gravity off when it is
not part of the question.

### 1.6 Environment, initial and coupling conditions

#### Environment conditions

| Environment condition | Components | Effect |
|---|---|---|
| Gravitational acceleration | g in X, Y, Z `[m/s^2]` | gravity term of Darcy's law, see 1.5 |
| Temperature | Temperature `[K]` | offered, but not read - see below |

The *Temperature* environment condition is offered because material properties
are tables against temperature. The solver does **not** read it: it takes element
temperatures only from the shared data left behind by a *Heat transfer* task, and
otherwise evaluates every table at `293.15 K`.

#### Initial conditions

| Initial condition | Effect |
|---|---|
| Pressure | offered, not read |
| Velocity | offered, not read |

The flow is steady, so the answer does not depend on a starting field and the
solver reads neither initial condition. Both are offered because the problem type
shares them with the incompressible flow problem.

#### Coupling with other solvers

| Direction | Variable | Effect |
|---|---|---|
| Heat -> porous media flow | Temperature | selects the row of every temperature-dependent material table |

The pressure and the velocity are not consumed by any other solver. Porous media
flow combines with every other physics except *Incompressible viscous flow* and
*Modal analysis*.

The solver reports itself as converged unconditionally, so a task group that
contains only a porous media flow task always stops after one iteration.

### 1.7 Solving the system

The solver assembles and solves

```
K * p = f
```

`K` is symmetric positive definite once at least one pressure has been
prescribed, so it is solved with the **conjugate gradient** method with Jacobi
preconditioning. The assembly is parallelised over elements with per-thread
buffers that are merged once at the end, so the result does not depend on the
number of threads.

Permeabilities of real materials span from about `1e-9 m^2` (gravel) down to
`1e-20 m^2` (intact clay), which would put the matrix entries close to the
round-off limits of the iterative solver. Before solving, every equation is
therefore divided by the largest `k/mu` of the model. Dividing all equations by
the same number does not change the solution; it only keeps the iteration well
conditioned.

A model with **no Pressure (explicit) boundary condition** has no unique
solution. The solver detects it and stops with the error *Flow through porous
media requires at least one 'Pressure (explicit)' boundary condition*.

Further ways to arrive at a singular or empty system, all of them silent:

- an entity whose material is missing one of the three required properties is not
  solved, and its nodes drop out of the system;
- an entity with zero permeability or zero viscosity has no stiffness, so its
  nodes stay in the system but are connected to nothing;
- a surface with zero thickness, or a line with zero cross area, carries no flow.

### 1.8 Derived results

After the solve the element velocity is recovered from the nodal pressure field.
The pressure gradient is averaged over the integration points of the element and
combined with the gravity term; for line and surface elements it is rotated into
global coordinates:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Result | Apply to | Meaning |
|---|---|---|
| Pressure `[Pa]` | node | the solved pore pressure field |
| Velocity `[m/s]` | element | Darcy velocity vector |

The velocity is a **density** in the same sense as the field results of the
other solvers: it is independent of the element size, constant within a linear
element and converges as the mesh is refined. Boundary faces of zero thickness
show a zero velocity - display the result on the volume.

A total flow rate is not stored as a result. Compute it by hand from the
velocity and the area of a cross section, or from the inflow conditions, see 5.

After the record is written the solver log prints the statistics of both
variables, and the values at any monitoring points.

### 1.9 Validity of Darcy's law

Darcy's law holds for slow, laminar flow, in which viscous forces dominate the
inertia of the fluid. A common criterion uses the Reynolds number formed with the
mean grain diameter `d`:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

Groundwater flow in soils, seepage through dams and embankments, flow through
filters, packed beds and most porous engineering materials satisfy it with a wide
margin. Flow through coarse gravel, rock fill or very open foams at high speeds
does not; Darcy's law then underestimates the pressure drop (the Forchheimer
regime), and this solver will report too large a velocity.

### 1.10 Steady flow and the time solver

The problem type allows the time solver, but the flow itself is **steady**: the
fluid and the porous body are incompressible and there is no storage term. With
the time solver enabled, each time step solves a separate steady state using the
condition values at that time. This is the right model when the boundary
conditions change slowly compared with the time the pressure needs to settle -
a reservoir level that changes over hours above a sand filter - and it lets a
porous media flow task share a task flow with a transient heat task.

Consolidation, drainage of an unsaturated soil, compressible gas flow and any
other phenomenon in which the amount of fluid stored in the pores changes with
time are outside the model, see section 6.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Environment conditions`,
`Material`, `Results` tabs), the **Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick **Flow through porous media**.

The problem type cannot be combined with *Incompressible viscous flow* or with
*Modal analysis*. Placing a *Heat transfer* task before it makes the material
tables follow the computed temperature.

### 2.2 Problem tab

The tab shows the *Time-solver* group box, because the problem type supports
the time solver - see 1.10 for what a transient run means here. Porous media
flow has no setup group box of its own. Everything is assigned per entity in the
condition tabs.

### 2.3 Material tab

The `Material` tab lists the material assigned to the selected entity. Porous
media flow requires **all three** of these:

| Property | Units | Comment |
|---|---|---|
| Permeability to fluids | `m^2` | intrinsic permeability of the solid, see the table below |
| Dynamic viscosity | `kg/(m*s)` | of the pore fluid, `1.003e-3` for water at 20 °C |
| Density | `kg/m^3` | of the pore fluid, `998.2` for water at 20 °C |

The bundled library contains water-saturated porous materials ready for use.
Each one carries the temperature-dependent density and viscosity of water and a
typical permeability:

| Material | Permeability `[m^2]` | Hydraulic conductivity at 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

The permeability of a real soil can differ from these values by an order of
magnitude or more - they are starting points, not data for a particular site.
To model another fluid, copy one of them and replace the density and the
viscosity. The plain *Sand* material is a dry bulk material without a viscosity,
so it is marked invalid for this problem type.

### 2.4 Boundary conditions tab

The tab is split into a manager list of available boundary conditions on top and
an editor for the selected one below. Select a surface entity in the `Model`
tree first; the manager then offers the conditions of section 1.4. Tick one to
assign it, then edit its components in the lower tree.

- **Pressure (explicit)** holds the pressure on the outlet. A model with none of
  these will not solve.
- **Velocity (inflow)** and **Volumetric flow rate (inflow)** drive a known flow
  into the model. Positive values flow in, negative values flow out.
- Faces without a condition are impermeable.

### 2.5 Initial and environment conditions

The `Environment conditions` tab offers *Gravitational acceleration*, which every
entity already carries with `(0, 0, -9.80665) m/s^2` (see 1.5), and
*Temperature*, which this solver ignores.

The `Initial conditions` tab offers *Pressure* and *Velocity*, which this solver
does not read.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The porous media flow solver uses the **CG** entry. If the solve does not
converge, look first for an entity with zero permeability or an isolated point
element rather than at the solver settings.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record - *Pressure* or *Velocity*. `Report` ->
`Monitoring points` shows the values.

### 2.8 Results and records

The `Results` tab lists the computed variables and controls the 3D view. The
pressure is a node scalar, the velocity an element vector. Display the velocity
as a vector field on the volume to see the flow paths.

The `Records` tab of the `Model` dock lists the result records: one for a steady
run, one per output time step with the time solver enabled.

`Report` -> `Solver log file` shows the full solver output, including the
statistics of both variables.

---

## 3. Tutorial - pressure driven flow through a sand filter

**Goal.** Compute the flow of water through a horizontal sand filter driven by a
known pressure difference, and check the flow rate against Darcy's law.

This tutorial assumes a meshed volume model of a box - use `File` ->
`New model`, draw a box with `Geometry` -> `Create element` or import a
geometry, and tetrahedralise it; the *Draw cube* tutorial covers this ground.
Mark the two end faces as separate surface entities so that conditions can be
assigned to them.

Take the filter to be `1 m` long along the X axis with a `0.1 m` by `0.1 m`
cross section.

### Step 1 - select the problem

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add a task and select **Flow through porous media**.
3. Confirm with `OK`.

Leave the *Time-solver* unchecked - the answer is a single steady state.

### Step 2 - assign the material

1. Select the volume entity in the `Model` tree.
2. Open the `Material` tab and assign **Coarse Sand (water saturated)**.

The material carries the permeability `1e-10 m^2` of the sand and the density
and viscosity of water.

### Step 3 - switch gravity off

The filter is horizontal and only the pressure difference is of interest, so
remove the default gravity:

1. With the volume entity selected, open the `Environment conditions` tab.
2. Select **Gravitational acceleration** and set all three components to `0`.

With gravity left on, the water would also carry a hydrostatic pressure across
the `0.1 m` height of the filter, and the uniform pressures on the vertical end
faces would make it circulate near them - see 1.5.

### Step 4 - prescribe the two ends

1. Select the surface entity at `x = 0`.
2. Open the `Boundary conditions` tab, tick **Pressure (explicit)** and set
   `10000` Pa - about one metre of water.
3. Select the surface entity at `x = 1 m`, tick **Pressure (explicit)** and set
   `0` Pa.

The four side faces get no condition: they are impermeable walls.

### Step 5 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. Follow the progress in `Report` -> `Solver log file`.

The run is a single conjugate gradient solve.

### Step 6 - inspect the results

- `Results` tab: display **Pressure**. It falls linearly from `10000` Pa to
  `0` Pa along the filter, with flat isobars perpendicular to its axis.
- Display **Velocity**. It points along +X and is the same in every element.
- A monitoring point on *Pressure* half way along the filter reads `5000` Pa.

### Step 7 - work out the numbers by hand

With `k = 1e-10 m^2` and `mu = 1.003e-3 kg/(m*s)` for water at 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

The **Velocity** result must show `9.97e-4 m/s`. The Reynolds number with a
grain size of `1 mm` is `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, at the edge of
the Darcy regime (1.9). With a porosity of `0.35` the water itself moves at
`9.97e-4 / 0.35 = 2.8e-3 m/s` and needs about six minutes to cross the filter.

### Troubleshooting

| Symptom | Cause |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | no outlet pressure is prescribed anywhere |
| *Entity has material assigned which is missing required properties* | the material lacks a viscosity - often a dry soil material such as *Sand* |
| The pressure varies over the height and the velocity has a Z component | the default gravity is still on, see step 3 |
| The velocity is zero everywhere | both ends carry the same pressure, or the permeability is zero |

---

## 4. Tutorial - upward seepage through a vertical column

**Goal.** Push water upwards through a vertical sand column with a known flow
rate, and see how the inlet pressure splits into a hydrostatic part and a
friction part.

Use a box `0.1 m` by `0.1 m` wide and `1 m` tall along the **Z** axis, with the
bottom face (`z = 0`) and the top face (`z = 1 m`) as separate surface entities.

### Step 1 - problem and material

1. Select **Flow through porous media** as in tutorial 3.
2. Assign **Coarse Sand (water saturated)** to the volume.

### Step 2 - keep the gravity

The column stands along Z, so the default gravity `(0, 0, -9.80665) m/s^2`
points the right way. Leave the *Gravitational acceleration* condition as it is.

### Step 3 - boundary conditions

1. Select the top face, tick **Pressure (explicit)** and set `0` Pa: the free
   water above the column.
2. Select the bottom face, tick **Volumetric flow rate (inflow)** and set
   `1e-6` m^3/s. Over the `0.01 m^2` of the face that is an inflow velocity of
   `1e-4 m/s`.

The top face is horizontal, so a uniform pressure on it is consistent with
gravity.

### Step 4 - solve and inspect

`Solution` -> `Start solver` (`Ctrl+R`), then display **Pressure** and
**Velocity**.

- The velocity is `1e-4 m/s` pointing up (+Z) in every element.
- The pressure grows linearly from `0` Pa at the top to its maximum at the
  bottom.

### Step 5 - split the inlet pressure

The pressure at the bottom has to lift the water column and push it through the
sand:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

A monitoring point at the bottom face confirms the sum. Set the flow rate to
zero and solve again: the velocity vanishes and the bottom pressure falls to the
pure hydrostatic `9789 Pa` - the solver reproduces a fluid at rest exactly.

Increase the flow rate until the friction part reaches the submerged weight of
the sand, about `(rho_s - rho) * g * (1 - n) * L`: that is the onset of
fluidisation (quicksand), which the rigid skeleton of this model cannot
represent - the solver simply keeps computing a larger pressure.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The bottom pressure is only the friction part | gravity is switched off on the volume |
| The bottom pressure is only the hydrostatic part | the inflow is on the wrong face, or its value is zero |
| The velocity points sideways | the column was built along another axis than Z; edit the gravity condition |

---

## 5. Checking a model

**Uniform column.** For a body of length `L` held at `p1` and `p2` on two
opposite faces, with one material and gravity switched off, the pressure is
linear along the body and

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

The velocity result must match to the solver tolerance, and the pressure at the
mid-plane must be the mean of the two prescribed values.

**Hydrostatics.** A body with gravity, a *Pressure (explicit)* condition only on
its horizontal top face and impermeable walls elsewhere must show zero velocity
and a pressure growing by `rho*|g|` per metre of depth. This checks the density
and the gravity condition at once.

**Flow balance.** In a steady model all fluid that enters must leave. For an
inflow prescribed by *Volumetric flow rate (inflow)*, the velocity in a cross
section times its area must give the prescribed rate back. Read the velocity in
the column with a monitoring point.

**Series layers.** For two layers of length `L1`, `L2` and permeability `k1`,
`k2` in series between two prescribed pressures, the flow resists like two
resistors in series:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          interface pressure
```

Unlike the electrostatic solver, this one divides the pressure by the
**permeabilities**, as a resistor network would.

**Radial flow to a well.** For a confined aquifer of thickness `H` between a well
of radius `rw` held at `pw` and an outer radius `R` held at `pR`, the pressure is
logarithmic and the flow rate of the well is

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

This checks the solver on a non-uniform field and shows whether the mesh is fine
enough near the well.

Refine the mesh at least once and confirm the pressure is converging rather than
drifting.

---

## 6. Limitations

- **Steady, incompressible, single-phase flow.** There is no storage term, so
  consolidation, compressible gas flow and the transient response of a confined
  aquifer are not modelled. A transient run is a sequence of steady states, see
  1.10.
- **Fully saturated media only.** There is no free surface (phreatic line) and no
  unsaturated zone. Seepage through an embankment whose water table has to be
  found is outside the model, unless the saturated region is meshed explicitly.
- **Rigid porous body.** The skeleton does not deform, so there is no coupling to
  stress analysis, no effective stress, no consolidation and no fluidisation.
- **Isotropic permeability.** The permeability is a scalar. Layered soils with a
  different horizontal and vertical permeability have to be modelled as separate
  layers.
- **Darcy regime only.** Inertial (Forchheimer) and turbulent flow in very coarse
  media is not modelled, see 1.9.
- **Boundary conditions on surfaces only.** Pressures and inflows are prescribed
  on surface entities; there is no seepage face, no well or line source and no
  prescribed pressure on a line or a point.
- **No total flow rate result.** Flow rates through a cross section have to be
  computed from the velocity by hand.
- **Initial conditions are not read** and the *Temperature* environment condition
  has no effect; material tables follow the temperature only through a heat task
  earlier in the task flow.
- **A model needs a prescribed pressure.** With none the solver stops with an
  error.
