# Electro-statics - Theory Manual and User Guide

This document describes the electrostatics solver (`RSolverElectrostatics`) of
Range FEA, which serves the **Electro-statics** problem type: the equations it
solves, the meaning of every input it accepts, the parts of the graphical user
interface that drive it, and two worked tutorials.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - potential and current in a conductor](#3-tutorial---potential-and-current-in-a-conductor)
4. [Tutorial - resistive heating](#4-tutorial---resistive-heating)
5. [Checking a model](#5-checking-a-model)
6. [Limitations](#6-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the electric potential field

The primary unknown is the **electric potential** `V` at every node, in volts.
Every node carries a single degree of freedom, so a mesh with `N` nodes gives a
system of `N` equations. The electric field, the current density and everything
derived from them are recovered from the potential field after the solve.

The potential is a scalar, which makes an electrostatic model as cheap as a heat
model of the same mesh: one unknown per node instead of three, and a matrix with
a ninth of the entries of a structural model.

The default potential of a node that no condition and no previous result touches
is `0 V`.

A potential field is only defined **up to an additive constant**. Nothing in the
equations fixes the zero level, so every model needs at least one *Electric
potential* boundary condition - see 1.7. This is the electrostatic equivalent of
an unconstrained structure, or of a heat model with no prescribed temperature.

### 1.2 Governing equation

The solver implements the **electrostatic field equation** - Poisson's equation
for the potential of a linear, isotropic dielectric:

```
div( e0 * er * grad(V) ) = -rho
```

and with no charge density anywhere it reduces to Laplace's equation

```
div( e0 * er * grad(V) ) = 0
```

The electric field is the negative gradient of the potential, and the current
density follows from Ohm's law:

```
E = -grad(V)
J = sigma * E
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `V` | electric potential | `V` | solved for |
| `rho` | charge density | `C/m^3`, see 1.5 | Charge density condition |
| `e0` | vacuum permittivity | `F/m` | built in, `8.854187817e-12` |
| `er` | relative permittivity | `-` | material property |
| `sigma` | electrical conductivity | `S/m` | material property |
| `E` | electric field | `V/m` | recovered, see 1.8 |
| `J` | current density | `A/m^2` | recovered, see 1.8 |

The vacuum permittivity is a constant of the solver, not a material property and
not a setting - only the **relative** permittivity is entered with the material.

**The field is driven by the permittivity, not by the conductivity.** This is the
single most important thing to know about this solver. The stiffness matrix is
built from `e0*er` alone; the conductivity never enters the system of equations.
It is used afterwards, to turn the computed field into a current density, a
resistivity and a Joule heat. The consequence is spelled out in 1.9: in a body
made of one material it makes no difference, but in a body made of **two**, the
potential divides in proportion to the permittivities and not in proportion to
the resistances.

**Required material properties.** An electrostatic problem requires **three** of
them:

| Property | Units | Used for |
|---|---|---|
| Relative permittivity | `-` | the stiffness matrix |
| Electrical conductivity | `S/m` | current density, resistivity, Joule heat |
| Density | `kg/m^3` | nothing - required, never read |

An entity whose material is missing any of the three is **not solved**, even
though the density is never read by this solver at all. The setup checker warns
about it before the run starts. The one exception is an entity that carries an
electrostatic boundary condition: that entity is included in the system whatever
its material says, and its permittivity and conductivity are then simply zero.

A zero permittivity is not a harmless default - an element with `er = 0` has no
stiffness, so it contributes nothing to the system and leaves its nodes floating.
If an entity is meant to be solved, give it a real permittivity: `1` for vacuum
or, near enough, for air.

Every property is a table against temperature, but an electrostatics task on its
own has no temperature field to read it at, so every table is evaluated at
`293.15 K`. The tables are honoured only when a *Heat transfer* task earlier in
the same task flow has computed element temperatures, which are shared with the
tasks that follow it.

The relative permittivity is **dimensionless** - it is shown as `N/A` in the user
interface, like the emissivity and the Poisson ratio - and the value is used as a
plain multiplier of `e0`. Enter `1` for vacuum, `1.00059` for air, `2.1` for
PTFE, about `4` for glass.

### 1.3 Finite element discretisation

The weak form of the field equation gives, for each element,

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

where `B` is built from the shape function derivatives and `N` are the shape
functions themselves. Every element matrix is `n x n` for an element with `n`
nodes - one row per node, not three. There is no mass or capacity matrix: the
problem is static, and 1.10 explains what that means for the task flow.

**Element measures.** As elsewhere in Range FEA, lower-dimensional entities are
given a geometric measure so that they can represent real bodies:

| Entity | Measure | Where it is set |
|---|---|---|
| Volume | 1 | - |
| Surface | surface thickness | entity geometric properties |
| Line | line cross area | entity geometric properties |
| Point | point volume | entity geometric properties |

A **surface** carries the field in its own plane, through a cross section equal
to its thickness. There is no through-thickness potential gradient: a surface
element is one node thick, so it is an equipotential across its thickness by
construction. A surface of **zero thickness** therefore has no stiffness at all
and connects nothing - unlike the heat solver, where a zero-thickness surface
still carries convection, a zero-thickness surface is simply inert here. Give a
surface entity a thickness whenever it is meant to conduct, and remember that a
zero-thickness face also gets **no derived results** (1.8).

A **line** carries the field along its own direction, through its cross area. A
**point** has no direction to carry it along, so it contributes **no stiffness at
all**: it holds only a charge source. A point element whose node is not shared
with any other computable element therefore produces an empty row in the matrix,
which the conjugate gradient solver cannot handle - a point is a place to inject
charge or to prescribe a potential, not a conductor.

### 1.4 Sources and boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Electric potential | explicit (Dirichlet) | point, line, surface, volume | Electric potential `[V]` |
| Charge density | natural, source | point, line, surface, volume | Charge density, see 1.5 |

These two are the whole vocabulary of the problem type. There is no prescribed
flux, no surface charge, no far-field or open-boundary condition and no symmetry
condition - see section 6.

**Electric potential** is the only explicit condition of the electrostatic
problem. Every node of the entity it is assigned to is removed from the system of
equations and held at the given value; its contribution is carried to the
right-hand side of the neighbouring equations. Every model needs at least one of
these. A newly assigned condition starts at `1 V`.

Where entities overlap, the **lower-dimensional entity wins**: a potential
prescribed on a surface overrides one prescribed on the volume behind it at the
nodes they share, a line overrides a surface, and a point overrides everything.
Potentials are not combined - there is only one degree of freedom per node to
hold, so the last condition read simply prevails.

**Charge density** is the source term of the Poisson equation. Unlike the *Heat*
condition of the heat solver, its value is **not** spread over the measure of the
entity: the number entered is applied to every element of the entity as it
stands, as a density. The total charge placed in a volume entity is therefore the
value multiplied by the volume of that entity, and it grows if the entity is
enlarged - it does not stay put the way a *Heat* value in watts does.

| Entity | What the value means |
|---|---|
| Volume | charge per unit volume |
| Surface | charge per unit volume of the thickness-weighted section |
| Line | charge per unit volume of the cross-section |
| Point | a nodal charge source |

Both components accept a table against time. Since the problem itself is static,
a time history only has an effect when another, time-dependent task shares the
task flow - and, as 1.10 explains, that combination is itself restricted.

### 1.5 The charge density value

The *Charge density* component is a **volumetric charge density** in `C/m^3`, as
its label in the user interface says. It is a density rather than a total, which
is what makes it independent of the mesh - and, as 1.4 notes, what distinguishes
it from the *Heat* condition of the heat solver, which takes a total in watts and
spreads it for you.

The sign follows the usual convention. Integrating `div(e0*er*grad(V)) = -rho`
by parts leaves the charge on the right hand side with a positive sign,

```
every entity type      f = +INT( N^T * rho )
```

so a **positive** charge density raises the potential around it and a negative
one lowers it, on volume, surface, line and point entities alike.

Remember that a charge source alone does not make a solvable model. The potential
is still defined only up to a constant until something fixes it, so a
charge-driven model needs at least one *Electric potential* condition as well -
on the outer boundary of the meshed region, if nowhere else. See 1.7.

### 1.6 Environment, initial and coupling conditions

#### Environment conditions

| Environment condition | Components | Effect |
|---|---|---|
| Temperature | Temperature `[K]` | offered, but not read - see below |

The *Temperature* environment condition is offered for an electrostatics problem
because material properties are tables against temperature. The electrostatics
solver does **not** read it: it takes element temperatures only from the shared
data left behind by a *Heat transfer* task, and otherwise evaluates every table at
`293.15 K`. Assigning it to an electrostatics-only model changes nothing.

#### Initial conditions

| Initial condition | Effect |
|---|---|
| Electric potential | initial potential field, applied on the first run |

The initial potential is read on the **first run only**. Since the solve is a
direct one rather than a march, it does not change the converged answer - it only
sets the starting point of the conjugate gradient iteration and the value of any
node the solver leaves out of the system.

Environment conditions, initial conditions and boundary conditions are read in
that order and each overwrites the previous one, so a **boundary condition wins
over an initial condition, which wins over an environment condition**.

#### Coupling with other solvers

The electrostatics solver produces two results that other solvers consume.

| Direction | Variable | Effect |
|---|---|---|
| Electro-statics -> heat | Joule heat | added to the source term of every element |
| Electro-statics -> fluid heat | Joule heat | added to the source term of every element |
| Electro-statics -> magneto-statics | Current density | source of the magnetic field |
| Heat -> electro-statics | Temperature | selects the row of every temperature-dependent material table |

**Joule heat** is picked up by a *Heat transfer* task placed **after** the
electrostatics task in the problem task flow, where it is added to the element
source term exactly as a *Heat rate (unit volume)* condition would be. Resistive
heating therefore needs no setup beyond ordering the two tasks - tutorial 4 walks
through it.

**Magneto-statics** is a separate problem type that *requires* an electrostatics
task: it reads the element current density this solver stores - on volume,
surface and line entities alike - and evaluates the magnetic field of that
current with the Biot-Savart law. Add the electrostatics task first.

Because the solver reports itself as converged unconditionally, a task group that
contains only an electrostatics task always stops after one iteration. Iterating
is worthwhile only when something the solve depends on is itself lagged - a
strongly temperature-dependent permittivity fed by a heat task, for instance.

### 1.7 Solving the system

The solver assembles and solves

```
K * V = f
```

`K` is symmetric positive definite once at least one potential has been
prescribed, so it is solved with the **conjugate gradient** method with Jacobi
preconditioning. The assembly is parallelised over elements with per-thread
buffers that are merged once at the end, so the result does not depend on the
number of threads.

A model with **no Electric potential boundary condition** is singular: the
potential is defined only up to a constant, the matrix has a null space, and the
solve will not converge. Every model needs at least one prescribed potential, and
a model that is meant to carry a current needs two at different values.

Three further ways to arrive at a singular or empty system are worth knowing,
because all three are silent:

- an entity whose material is missing one of the three required properties is not
  solved, and its nodes drop out of the system;
- an entity with `er = 0` has no stiffness, so its nodes stay in the system but
  are connected to nothing;
- a surface with zero thickness, or a line with zero cross area, has no stiffness
  either.

### 1.8 Derived results

After the solve the element results are recovered from the nodal potential field.
The electric field is the negative potential gradient averaged over the
integration points and rotated into global coordinates for line and surface
elements:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

| Result | Apply to | Meaning |
|---|---|---|
| Electric potential `[V]` | node | the solved potential field |
| Electric field `[V/m]` | element | field vector |
| Current density `[A/m^2]` | element | `sigma*E`, zero where the conductivity is zero |
| Electric energy `[J/m^3]` | element | energy density of the field |
| Electrical resistivity `[V*m/A]` | element | `1/sigma` wherever a current flows, `0` elsewhere |
| Joule heat `[W/m^3]` | element | resistive dissipation, handed to the heat solver |

All of these are **densities**: the shape function derivatives are averaged over
the integration points of the element and are not weighted by its Jacobian
determinant, and the surface thickness and the line cross area stay in the
stiffness where they belong rather than entering the recovered field. The field,
the current density and the electric energy are therefore independent of the size
of the element they were computed in, so they can be compared between elements
and against a hand calculation, and they converge as the mesh is refined.

**Electrical resistivity** is the result to reach for when checking a model,
because the two factors it is built from cancel: `|E|/|sigma*E|` is exactly
`1/sigma`, whatever the field does. It is the quickest confirmation that the
conductivity intended for an entity actually reached its elements. It is stored
as `0` on any element where the current density falls below the numerical
tolerance, which includes every element whose conductivity is zero.

**The Joule heat** is the dissipation density `sigma*|E|^2`, in `W/m^3`. That is
exactly what a *Heat transfer* task expects when it adds the value to its source
term and integrates it over the element, so the power crossing into a coupled
thermal run is the power the model actually dissipates. Like the others it is a
density, so it is independent of the element size and converges under refinement.

Two more places where derived results are simply left at zero: **point elements**
are never post-processed at all, and a **surface with zero thickness** or a
**line with zero cross area** is skipped, so both keep a zero field however large
the potential gradient across them.

After the record is written the solver log prints the statistics of all six
variables, and the values at any monitoring points.

### 1.9 What the solver is, and what it is not

The name is exact: this is an **electro-static** solver. It computes the field of
a charge and potential distribution in a dielectric. The current density it
reports is what Ohm's law gives for that field - a derived quantity, computed
after the fact, which never influences the field itself.

A **steady current-flow** problem, by contrast, is governed by

```
div( sigma * grad(V) ) = 0
```

with the conductivity in the place this solver puts the permittivity. The two
give the same potential field in a homogeneous body, because both reduce to
Laplace's equation and the constant cancels. They part company as soon as the
body is made of more than one material:

| | This solver | A current-flow solver |
|---|---|---|
| Potential divides in proportion to | `1/er` of each region | `1/sigma` of each region |
| Right for | a capacitor, a dielectric, an insulator | a resistor network, a busbar, a conductive path |

So a single-material conductor with prescribed potentials at its ends is modelled
correctly: the potential is linear, and dividing by the length gives the field.
Two conductors of different conductivity in series are not - the split follows
the permittivities, which is unlikely to be what was intended. Model such a case
as a single material, or compute the division by hand.

### 1.10 Static analysis and the time solver

Electro-statics is declared as a problem type **without** a time solver. That has
two visible consequences.

In an electrostatics-only model the *Time-solver* group box does not appear in
the `Problem` tab at all, the run produces a single record, and there is nothing
to configure.

More surprisingly, adding an electrostatics task to a task flow **disables the
time solver for the whole flow**. The time solver is offered only when *every*
problem type in the flow supports one, so a transient heat analysis that is given
an electrostatics task becomes a steady-state analysis, with no warning beyond
the group box disappearing. If a transient thermal answer is needed for a
resistive heater, run the electrostatics task separately and carry its
dissipation into the transient heat model as a *Heat* condition, rather than
putting both tasks in one flow.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Environment conditions`,
`Material`, `Results` tabs), the **Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick **Electro-statics** - "Calculate electric field,
current density and generated Joule's heat".

Electro-statics combines freely with every other physics except *Modal analysis*.
The chains worth knowing are:

- *Electro-statics* then *Heat transfer* - resistive heating;
- *Electro-statics* then *Magneto-statics* - the magnetic field of the computed
  current, which is the only way to run a magnetostatic analysis at all;
- *Electro-statics* then *Heat transfer* then *Stress analysis* - thermal stress
  from resistive heating.

Remember 1.10: any flow that contains an electrostatics task is solved as a
steady-state one.

### 2.2 Problem tab

For an electrostatics-only model this tab is **empty**. There is no time solver,
because the problem type does not support one, and electro-statics has no setup
group box of its own. Everything is assigned per entity in the condition tabs.

If the tab shows a *Time-solver* group box, another task in the flow put it
there - and, per 1.10, it will disappear again as soon as the electrostatics task
is added.

### 2.3 Material tab

The `Material` tab lists the material assigned to the selected entity.
Electro-statics requires **all three** of these:

| Property | Units | Comment |
|---|---|---|
| Relative permittivity | `N/A`, dimensionless | `er`, `1` for vacuum, `1.00059` for air, about `4` for glass |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` for copper, `5e-15` for air |
| Density | `kg/m^3` | required for the entity to be solved, never read |

An entity missing any of them is not solved, and the setup checker warns before
the run starts with *Entity ... has material assigned which is missing required
properties*. Since the density is never used by this solver, any positive value
satisfies the requirement.

The bundled material library already carries all three for the metals, for air
and for the glasses, so assigning *Copper* or *Air* is enough to get started.

Every property is a table against temperature, but the temperature dependence is
only honoured when a heat task earlier in the flow has produced element
temperatures - see 1.6.

### 2.4 Boundary conditions tab

The tab is split into a manager list of available boundary conditions on top and
an editor for the selected one below. Select an entity in the `Model` tree first;
the manager then offers the conditions valid for that entity type and for the
selected problem. Tick one to assign it, then edit its components in the lower
tree.

Only the two conditions of section 1.4 are offered. Notes on using them:

- **Electric potential** is the condition that holds a value rather than driving
  one, and it is offered on every entity type. Assign it to the face, edge or
  point whose potential is known. A model with none of these will not solve.
- **Charge density** is the source term, and a positive value raises the
  potential around it. Read 1.5 before using it: the value is a density and,
  unlike the *Heat* condition of the heat solver, it is not spread over the
  entity.
- Both components are tables against time, which only matters in a task flow
  driven by another, time-dependent task.

### 2.5 Initial and environment conditions

The `Initial conditions` tab offers *Electric potential*, which sets the starting
field of the iterative solve. It does not change the converged answer.

The `Environment conditions` tab offers *Temperature*, which this solver ignores -
see 1.6.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The electrostatics solver uses the **CG** entry.

If the solve does not converge, the usual cause is a model with no prescribed
potential, or an entity with zero permittivity, rather than a solver setting.
Check the boundary conditions and the material before raising the iteration
count.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record - for an electrostatic problem, *Electric
potential* is the usual choice, and it is the quickest way to read a value off a
model as a number rather than a colour. `Report` -> `Monitoring points` shows the
values.

### 2.8 Results and records

The `Results` tab lists the computed variables and controls the 3D view. The
electric potential is a node scalar; the electric field and the current density
are element vectors; the electric energy, the electrical resistivity and the
Joule heat are element scalars.

The `Records` tab of the `Model` dock lists the result records. An electrostatic
analysis is static, so there is exactly **one**.

`Report` -> `Solver log file` shows the full solver output, including the
statistics of all six variables after the record is written.

---

## 3. Tutorial - potential and current in a conductor

**Goal.** Compute the potential distribution along a copper bar held at two
different potentials, and read off the field, the current and the resistance.

This tutorial assumes a meshed volume model - a long box works well. Use
`File` -> `New model`, draw a box with `Geometry` -> `Create element` or import a
geometry, and tetrahedralise it; the *Draw cube* tutorial covers this ground.
Mark the two end faces as separate surface entities so that conditions can be
assigned to them.

Take the bar to be `0.1 m` long with a `0.01 m` by `0.01 m` cross section, which
makes the hand calculations at the end easy to follow.

### Step 1 - select the problem

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add a task and select **Electro-statics**.
3. Confirm with `OK`.

The `Problem` tab stays empty - there is nothing to set up.

### Step 2 - assign the material

1. Select the volume entity in the `Model` tree.
2. Open the `Material` tab and assign **Copper**, or a material with all three
   properties:
   - **Relative permittivity** = `2.8`
   - **Electrical conductivity** = `5.882e7` `S/m`
   - **Density** = `8960` `kg/m^3`

The density is not used by this solver but must be present for the entity to be
solved at all. The permittivity is what drives the field; the conductivity only
turns the field into a current afterwards.

### Step 3 - prescribe the two ends

1. Select the surface entity at one end.
2. Open the `Boundary conditions` tab and tick **Electric potential**.
3. Set the value to `0` V - this is the ground.
4. Select the surface entity at the other end, tick **Electric potential** again
   and set it to `10` V.

This is the step that makes the model solvable. A single prescribed potential is
the minimum; two at different values are what makes a field.

Do not add a *Charge density* condition. A conductor with prescribed end
potentials carries no space charge, so the source term belongs at zero and the
potential is a straight line between the two ends - which is what makes the hand
check of step 6 exact.

### Step 4 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. The setup checker reports missing materials before the run starts.
3. Follow the progress in `Report` -> `Solver log file`.

The run is a single conjugate gradient solve and finishes in seconds on a mesh of
this size.

### Step 5 - inspect the results

- `Results` tab: display **Electric potential**. It should fall linearly from
  `10` V to `0` V along the bar, with flat equipotential planes perpendicular to
  its axis. Any curvature in a uniform bar means the mesh is not connected the
  way it is assumed to be.
- **Electrical resistivity** should read `1/5.882e7 = 1.7e-8` `V*m/A` on every
  element of the bar. This is the best single check that the material reached the
  elements: it is exact, mesh independent, and wrong the moment the conductivity
  is.
- **Electric field** points along the bar and should read `10/0.1 = 100` `V/m`
  uniformly, and **Current density** `sigma*E = 5.88e9` `A/m^2`. Both are
  densities, so the numbers hold whatever the element size - step 6 derives them.
- A monitoring point on *Electric potential* half way along the bar should read
  `5` V.

### Step 6 - work out the numbers by hand

The first two lines below are what the **Electric field** and **Current density**
results should already be showing, so they are a direct check on the solve. The
last three are totals rather than densities, which no element result gives, and
have to be computed. For a uniform bar of length `L` and cross section `A` held
at a potential difference `U`:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

The current is enormous because a solid copper bar of that section is a very good
conductor and `10` V across it is a short circuit - a useful reminder that an
electrostatic model will happily compute a situation that would vaporise the real
part.

### Step 7 - optional, refine the mesh

Refine the mesh once and solve again. Every result - the **potential**, the
**electric field**, the **current density**, the **electric energy**, the
**resistivity** and the **Joule heat** - must hold its value. They are all
densities, so a value that drifts under refinement is the sign of a model problem
rather than of a finer mesh, and it is worth chasing before going further.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The solve does not converge | no *Electric potential* condition anywhere, so the potential is defined only up to a constant |
| *Entity has material assigned which is missing required properties* | one of the three required properties is absent - often the density, which the solver never uses |
| The whole model sits at one potential | only one potential was prescribed, or the second one carries the same value |
| The potential is zero everywhere except on the prescribed faces | the entity has `er = 0`, so it has no stiffness and connects nothing |
| Resistivity is zero everywhere | the electrical conductivity is zero, so no current flows and the resistivity is stored as zero |
| A surface entity shows no field at all | its thickness is zero, so it is neither assembled nor post-processed |

---

## 4. Tutorial - resistive heating

**Goal.** Take the Joule heat computed from the electric field and use it to heat
the bar, by chaining an electrostatics task and a heat task.

### Step 1 - build the task flow

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add an **Electro-statics** task.
3. Add a **Heat transfer** task **after** it.
4. Confirm with `OK`.

The order matters: the heat solver reads the Joule heat the electrostatics solver
left behind, so the electrostatics task has to run first. No condition and no
setting is needed to connect them.

Note what happens to the `Problem` tab: the *Time-solver* group box that a heat
task alone would show is now gone, because electro-statics has no time solver and
the flow takes the most restrictive setting - see 1.10. This chain is
steady-state only.

### Step 2 - complete the material

The heat task needs four properties of its own on top of the three the
electrostatics task needs, so the entity's material must carry all of:

| Property | Value for copper | Needed by |
|---|---|---|
| Relative permittivity | `2.8` | electro-statics |
| Electrical conductivity | `5.882e7` `S/m` | electro-statics |
| Density | `8960` `kg/m^3` | both |
| Thermal conductivity | `401` `W/(m*K)` | heat |
| Heat capacity | `385` `J/(kg*K)` | heat |
| Emissivity | `0` | heat |

The bundled *Copper* material already carries them. An entity missing any
property required by **either** task is skipped by that task.

### Step 3 - keep the electrical boundary conditions

The two **Electric potential** conditions from tutorial 3 carry over unchanged.
Lower the driving potential to something less violent - `0.01` V across the bar
still produces a large current, and keeps the heat source in a range where the
thermal answer is readable.

### Step 4 - add the thermal boundary conditions

The heat half of the model needs its own outlet, exactly as it would on its own:

1. Select the surface entity covering the sides of the bar.
2. Open the `Boundary conditions` tab and tick **Simple convection**.
3. Set **Convection coefficient** = `10` `W/(m^2*K)` and **Fluid temperature** =
   `293.15` K.

Without a convecting surface or a prescribed temperature the heat model has a
source and no outlet, and the steady state does not exist. Note that a face used
for convection needs no thickness for the heat solver, but a zero-thickness face
carries no electric field either - keep the electrical conditions on the end
faces and the convection on the sides, and the two do not interfere.

### Step 5 - solve

`Solution` -> `Start solver` (`Ctrl+R`). The log shows the electrostatics task
first, with the statistics of its six variables, and then the heat task with the
temperature, heat flux and heat transfer coefficient statistics.

### Step 6 - inspect the results

- `Results` tab: display **Joule heat**. It is the bridge between the two
  physics - wherever it is non-zero, the heat solver saw a source.
- Display **Temperature**. The bar should be warmest where the Joule heat is
  largest, and fall towards the fluid temperature at the convecting faces.
- Display **Electric potential** to confirm the electrical half of the model is
  still the linear field of tutorial 3.

### Step 7 - check the energy balance

The Joule heat is a dissipation density in `W/m^3`, so the total power the
electrical half of the model delivers to the thermal half is that density times
the volume it acts on. For the uniform bar it should come to the `P = U^2/R` of
tutorial 3, with `R = L/(sigma*A)`.

At the steady state the convecting surface has to shed exactly that power, which
gives the same global check the heat manual describes:

```
P = h * A_s * ( Ts_mean - Tf )
```

Read the mean surface temperature from the statistics in the solver log and
confirm the balance closes. This is the one check that spans both physics at
once: it catches a wrong conductivity, a wrong convection coefficient, and an
entity that quietly went unsolved in either half.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The temperature is uniform at `293.15` K | the Joule heat is zero - check the electrical conductivity, and that the electrostatics task runs first |
| The heat solve does not converge | the thermal half has no prescribed temperature and no convecting surface |
| The *Time-solver* group box vanished | expected - an electrostatics task makes the whole flow steady-state, see 1.10 |
| The temperature rise changes when the mesh is refined | not expected - every electrostatic result is a density, so check the thermal half of the model and the mesh quality |
| Only the electrostatics results appear | the heat task is missing one of its four required material properties |

---

## 5. Checking a model

The solver has no built-in verification suite, so it is worth validating a new
model against something that can be computed by hand before trusting it. The
checks below are written against the **potential**, which is both the primary
unknown and the quantity every closed-form solution is expressed in; the electric
field and current density results should agree with the gradients they imply.

**Uniform bar.** For a body of length `L` held at `V1` and `V2` on two opposite
faces, with one material and no charge, the potential is linear along the bar and

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Place a monitoring point at the mid-plane and confirm it reads the mean of the
two prescribed values, and another at a quarter of the length for the quarter
value. This catches a disconnected mesh, a condition assigned to the wrong
entity, and a zero permittivity. Refine the mesh once and confirm the numbers
stay put.

**Uniform space charge.** The bar above exercises only the stiffness. To check
the source term as well, take the same body, hold **both** end faces at `0` V and
assign a uniform *Charge density* `rho` to the volume. The potential is then a
parabola, with `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

A monitoring point at the mid-plane gives `Vmax` directly. Two things are being
checked at once: the magnitude, which confirms the permittivity and the charge
density are being read as expected, and the **sign** - a positive charge density
must produce a positive potential hump between two grounded faces. Repeat with
the charge on a surface or a line entity, and on a point, and confirm the
polarity does not change with the entity type.

**Resistivity.** Every element that carries a current must store an electrical
resistivity of exactly `1/sigma`. Read it off the statistics in the solver log
and compare against the conductivity in the material. It is exact rather than
approximate, independent of the mesh, and the fastest confirmation that the
material reached the elements it was meant to.

**Coaxial field.** For an annulus between radii `a` and `b` with the inner
surface at `V1` and the outer at `V2`, the potential is logarithmic:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Model the annulus, prescribe both surfaces, and compare monitoring points at a
few radii. This validates the solver on a field that is not uniform, which the
bar cannot do, and it shows whether the mesh is fine enough near the inner
conductor.

**Series dielectrics.** For two layers of thickness `d1`, `d2` and relative
permittivity `er1`, `er2` stacked between two prescribed potentials, the
potential at the interface is

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Compare against the computed interface potential. This confirms directly what
section 1.9 describes: the division follows the **permittivities**. Running the
same model with two different conductivities and the same permittivity, and
watching the interface potential not move, is the demonstration that this is an
electrostatic solver and not a current-flow one.

**Resistance and dissipation.** The element results are densities, so a total -
a current, a resistance, a power - has to come from the potential. For the
uniform bar:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Use these whenever a total has to leave the model - for a conductor sizing, or
for a sanity check against a measurement. `P` is also what the **Joule heat**
result must integrate to over the volume of the bar, which is the check that the
electrical and thermal halves of a coupled run agree.

For any of these, refine the mesh at least once and confirm the potential is
converging rather than drifting.

---

## 6. Limitations

### Modelling limitations

- **Electro-static, not current flow.** The stiffness is built from the
  permittivity; the conductivity never enters the system of equations. The
  potential in a body of two materials therefore divides in proportion to the
  permittivities, not the resistances. A single-material conductor is modelled
  correctly; a resistor network is not - see 1.9.
- **Static only.** There is no time solver, no capacitance and no inductance, so
  charging transients, alternating fields, displacement current, eddy currents
  and skin effect are all outside the model. Adding an electrostatics task to a
  task flow also removes the time solver from the flow as a whole, so a heat task
  chained behind it can only be steady-state - see 1.10.
- **Linear, isotropic dielectrics only.** The permittivity and the conductivity
  of an element are scalars and are constant during a solve. Anisotropic,
  ferroelectric and field-dependent materials are not modelled, and there is no
  dielectric breakdown.
- **Only two boundary conditions.** A prescribed potential and a charge density
  are the whole vocabulary. There is no prescribed surface charge, no prescribed
  normal flux, no symmetry condition, and no open-boundary or infinite-element
  treatment - an unbounded field problem has to be truncated by meshing enough
  surrounding air and prescribing a potential on its outer boundary.
- **No free charge redistribution.** Charge does not move to the surface of a
  conductor; the solver has no notion of a conductor at all beyond the material
  properties given to it. Model a conductor by prescribing its potential, not by
  giving it a large conductivity.
- **No feedback from the current.** The current density is post-processed from
  the field, so a conductivity that varies with the field, or with the
  temperature the dissipation produces, has no effect within a single solve.
- **Surfaces are equipotential through their thickness.** A surface entity
  carries the field in its own plane only. A surface of zero thickness or a line
  of zero cross area has no stiffness at all and is skipped both in the assembly
  and in the post-processing.
- **Point elements do not conduct.** A point carries a charge source and can hold
  a prescribed potential, nothing more, and it is never post-processed. A point
  whose node belongs to no other computable element leaves an empty row in the
  matrix.
- **The temperature dependence of the material is not read.** Tables are
  evaluated at `293.15 K` unless a heat task earlier in the flow has produced
  element temperatures; the *Temperature* environment condition has no effect on
  this solver.
- **A model needs a prescribed potential.** With none the matrix has a null space
  and the solve will not converge.
