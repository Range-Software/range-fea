# Heat Transfer - Theory Manual and User Guide

This document describes the heat solver (`RSolverHeat`) of Range FEA, which
serves the **Heat transfer** problem type: the equations it solves, the meaning
of every input it accepts, the parts of the graphical user interface that drive
it, and two worked tutorials.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - steady-state heat transfer](#3-tutorial---steady-state-heat-transfer)
4. [Tutorial - transient heat transfer](#4-tutorial---transient-heat-transfer)
5. [Checking a model](#5-checking-a-model)
6. [Limitations](#6-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the temperature field

The primary unknown is the **temperature** `T` at every node, in kelvin. Every
node carries a single degree of freedom, so a mesh with `N` nodes gives a system
of `N` equations. Heat flux is recovered from the temperature field after the
solve.

Temperature is a scalar, which makes a heat model markedly cheaper than a
structural model of the same mesh: one unknown per node instead of three, and a
matrix with a ninth of the entries.

The default temperature of a node that no condition and no previous result
touches is `293.15 K`.

### 1.2 Governing equation

The solver implements **linear heat conduction** with convective boundaries.
The energy balance of a body with conductivity `k`, density `rho` and specific
heat capacity `c` is

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

and with the time solver disabled the transient term drops out, leaving the
steady-state balance

```
div( k * grad(T) ) + q = 0
```

Heat leaves a surface by convection according to Newton's law of cooling,

```
qs = h * ( T - Tf )
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `T` | temperature | `K` | solved for |
| `q` | heat source density | `W/m^3` | Heat condition, Joule heat, radiation |
| `k` | thermal conductivity | `W/(m*K)` | material property |
| `c` | heat capacity | `J/(kg*K)` | material property |
| `rho` | density | `kg/m^3` | material property |
| `h` | convection coefficient | `W/(m^2*K)` | convection condition, see 1.5 |
| `Tf` | fluid temperature | `K` | convection condition |
| `Q` | heat flux | `W/m^2` | recovered, see 1.9 |

There is no advection term. A heat transfer problem moves energy by conduction
inside the body and by convection across its boundary; it does not move energy
with a flowing medium. Use *Heat transfer in fluids* for that.

**Required material properties.** A heat problem requires **four** of them:

| Property | Units | Used for |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | the conduction matrix |
| Heat capacity | `J/(kg*K)` | the capacity matrix, transient only |
| Density | `kg/m^3` | the capacity matrix, transient only |
| Emissivity | `-` | radiative heat transfer only |

An entity whose material is missing any of the four is **not solved**, even for
a pure steady-state conduction run that never looks at the density or the
emissivity. The setup checker warns about it before the run starts. The one
exception is an entity that carries a heat boundary condition: that entity is
included in the system whatever its material says, and its conductivity and
capacity are then simply zero.

Every property is a table against temperature. The value used for an element is
read from the table at the **element temperature of the previous run**, so a
temperature-dependent conductivity lags by one iteration. Section 1.6 explains
how to converge it.

### 1.3 Finite element discretisation

The weak form of the energy balance gives, for each element,

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

where `B` is built from the shape function derivatives and `N` are the shape
functions themselves. Every element matrix is `n x n` for an element with `n`
nodes - one row per node, not three.

The convection terms are assembled on **surface elements only**, and only where
a convection boundary condition is assigned. The convection stiffness is the
consistent form `INT( N^T * h * N )`, while the convection load is lumped
equally onto the nodes of the element. The two are consistent enough on a
reasonable mesh, and the difference disappears as the mesh is refined.

**Element measures.** As elsewhere in Range FEA, lower-dimensional entities are
given a geometric measure so that they can represent real bodies:

| Entity | Measure | Where it is set |
|---|---|---|
| Volume | 1 | - |
| Surface | surface thickness | entity geometric properties |
| Line | line cross area | entity geometric properties |
| Point | point volume | entity geometric properties |

A **surface** conducts heat in its own plane, through a cross section equal to
its thickness, and stores energy in proportion to that thickness. There is no
through-thickness temperature gradient: a surface element is one node thick, so
it is isothermal across its thickness by construction. A surface of **zero
thickness** therefore conducts nothing and stores nothing, but still carries
convection - which is exactly what is wanted for the boundary face of a meshed
volume, where the face exists only to hold the convection condition.

A **line** conducts along its own direction through its cross area. A **point**
has no direction to conduct along, so it contributes no conduction at all: it
carries only a capacity, in proportion to its point volume, and a heat source.
A point is a lumped thermal mass, not a conductor.

The capacity matrix is assembled only when it is needed, that is for a transient
analysis.

### 1.4 Sources and boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Temperature | explicit (Dirichlet) | point, line, surface, volume | Temperature `[K]` |
| Heat | natural, source | point, line, surface, volume | Heat `[W]`, see below |
| Simple convection | natural (Robin) | surface | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | natural, correlation | surface | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | natural, correlation | surface | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | natural, source | surface | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | natural, source | volume | Heat per unit volume `[W/m^3]` |

**Temperature** is the only explicit condition of the heat problem. Every node
of the entity it is assigned to is removed from the system of equations and held
at the given value; its contribution is carried to the right-hand side of the
neighbouring equations. A steady-state model needs at least one of these, or at
least one convection surface - see 1.7.

Where entities overlap, the **lower-dimensional entity wins**: a temperature
prescribed on a surface overrides one prescribed on the volume behind it at the
nodes they share, a line overrides a surface, and a point overrides everything.
Unlike the displacement constraints of the structural solver, temperatures are
not combined - there is only one degree of freedom per node to hold, so the last
condition read simply prevails.

**Heat** is a heat source and its value is the **total power in `W`** delivered
into the entity it is assigned to, whatever the dimension of that entity. The
solver needs a source density, so the total is spread over the measure of the
entity before assembly:

| Entity | Density the solver applies |
|---|---|
| Volume | value / volume of the entity, `W/m^3` |
| Surface | value / area of the entity, `W/m^2` |
| Line | value / length of the entity, `W/m` |
| Point | value / number of point elements, `W` each |

The power delivered is therefore exactly the number entered - it does not change
with the size of the entity, with mesh refinement, or with the scale of the
model. Only elements the solver can compute carry a share, so an entity without
material properties is skipped and its condition reported instead. A surface
heat source is not scaled by the surface thickness.

**Heat rate (unit area)** and **Heat rate (unit volume)** prescribe the same
source term as a density instead of a total: `W/m^2` over the area of a surface
entity and `W/m^3` over the volume of a volume entity. Reach for them when the
source is naturally a density - a heater rated per square metre - and for *Heat*
when it is naturally a total, such as an element of known wattage. All of them
add up on an entity that carries more than one, and all of them accept a table against time, so a heater
can be switched on and off during a transient run.

**Convection** conditions apply to surfaces and add both a stiffness term and a
load, so they act as a Robin boundary condition rather than as a fixed flux:
raising the wall temperature increases the heat the wall loses. All three
flavours end up as the same pair `(h, Tf)`; they differ only in where `h` comes
from:

- **Simple convection** takes `h` and `Tf` straight from the two values you
  enter. Use it whenever the convection coefficient is known from a handbook, a
  measurement or a previous calculation. It is the only one of the three that is
  fully under your control.
- **Forced convection** computes `h` from a correlation for external flow along
  a flat plate, using the fluid properties and the hydraulic diameter you enter.
  Two of its inputs are normally not the ones you enter: where the surface
  borders a meshed fluid and a *Fluid heat transfer* task has produced a result,
  `Tf` is the bulk temperature and `v` the mean speed that solver holds for the
  element behind the wall, so the wall follows the fluid as it heats up and
  speeds up. The *Fluid temperature* and *Velocity* components are the
  fall-backs, used on any surface no such result covers - a model with no fluid
  domain, or the first pass of a coupled run. A fluid at rest counts as no
  result, so a velocity field of zeros falls back rather than collapsing the
  Reynolds number. The log says which of the two sources is in use.
- **Natural convection** computes `h` from a correlation for horizontal plates,
  using the fluid properties, the hydraulic diameter and the temperature
  difference between the wall and the fluid.

If more than one convection condition is assigned to the same surface they do
not add up: simple is read first, forced overwrites it, and natural overwrites
both.

### 1.5 The convection correlations

Both computed forms build the usual dimensionless groups from the fluid
properties entered with the boundary condition, with the hydraulic diameter `d`
as the characteristic length:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

and convert the Nusselt number into a convection coefficient as

```
h = Nu * k / d
```

| Form | Correlation | Nusselt number |
|---|---|---|
| Forced | external flow, flat plate | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Natural | horizontal plate, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Natural | horizontal plate, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

The `k`, `c`, `rho`, `mu` and `beta` in these expressions are the **fluid**
properties entered with the boundary condition, not the material properties of
the solid.

Both conditions are created holding the properties of **dry air at 20 degrees
Celsius and 101.325 kPa**, so the fluid is described for you and only the flow
and the geometry are left to enter:

| Component | Default |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, the ideal gas value `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Replace them for any other fluid - they are the fluid's properties, not the
material of the wall. *Natural convection* is therefore ready to solve as
assigned; *Forced convection* still needs a **velocity**, which no default can
guess.

Every one of those groups divides by `mu`, `k` or `d`, and `Re` and `Pr` fall to
zero if `rho`, `c` or `v` do, which would leave `h = 0` and a surface that never
cools. A value of zero is therefore treated as a mistake rather than papered
over: the solver stops and names the component and the entity, for example

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

A velocity of zero that the *fluid heat* solver computed is a different matter -
it is a result, not a mistake. The wall is then left unconvected for that pass
and the log says so, rather than the solve being stopped or the configured
velocity being used against what the flow says.

Three consequences are worth keeping in mind:

- The correlation is **fixed**. The solver does not look at the orientation of
  the surface, whether the flow is internal or external, or whether the boundary
  layer is laminar or turbulent. Everything enters through the hydraulic
  diameter, which is the characteristic length of the correlation - it is your
  only handle on the geometry.
- A hydraulic diameter of zero would give `h = 0`, that is an insulated wall,
  so the solver stops rather than accept it.
- Natural convection depends on the **wall temperature**, which is taken from
  the element temperature of the **previous** solve. On the first run of a
  steady-state analysis that is the initial temperature, so the coefficient is
  evaluated at the wrong temperature difference. Wrap the task in a problem task
  group with several iterations to converge it, or use *Simple convection* with
  a coefficient you have computed yourself. A vanishing temperature difference
  gives `Nu = 1`.

### 1.6 Environment, initial and coupling conditions

#### Environment conditions

| Environment condition | Components | Effect |
|---|---|---|
| Temperature | Temperature `[K]` | background temperature of the entity, first run only |

#### Initial conditions

| Initial condition | Effect |
|---|---|
| Temperature | initial temperature field, applied on the first run |

Environment conditions, initial conditions and boundary conditions are read in
that order and each overwrites the previous one, so a **boundary condition wins
over an initial condition, which wins over an environment condition**. The
environment and initial values are read on the **first run only**; after that
the field marches from the previous result.

#### Coupling with other solvers

The heat solver both consumes and produces results shared with the rest of the
task flow.

| Direction | Variable | Effect |
|---|---|---|
| Radiative heat transfer -> heat | Heat - radiation part | added to the source term of **surface** elements |
| Electro-statics -> heat | Joule heat | added to the source term of every element |
| Stress -> heat | Displacement | the mesh is deformed for the heat solve and restored afterwards |
| Heat -> stress | Temperature | drives the thermal expansion term of the structural solver |
| Heat -> any solver | Temperature | selects the row of every temperature-dependent material table |

**Radiative heat transfer** is a separate problem type with its own solver. It
requires a *Heat transfer* task, solves an enclosure radiosity system over
surface patches using view factors and the emissivity of the material, and hands
the result back as a surface heat source. Add both tasks to one problem task
group and let the group iterate: the radiative solver reports convergence and
ends the group when its patch heat stops changing.

**Joule heat** arrives the same way from an *Electro-statics* task placed before
the heat task, so resistive heating needs no further setup.

Because the heat solver reports itself as converged unconditionally, a task
group that contains only a heat task always stops after one iteration. Iterating
is worthwhile when something the heat solve depends on is itself lagged -
radiative heat, a natural convection coefficient, or a strongly
temperature-dependent conductivity.

### 1.7 Steady-state analysis

With the time solver disabled the solver assembles and solves

```
K * T = f
```

`K` is symmetric positive definite once at least one temperature has been
prescribed, so it is solved with the **conjugate gradient** method with Jacobi
preconditioning.

A steady-state model with **no Temperature boundary condition and no convection
surface** is the thermal equivalent of an unconstrained structure: energy can be
poured in but has nowhere to leave, the matrix is singular and the solve will
not converge. Every steady-state model needs at least one path to a known
temperature.

### 1.8 Transient analysis

With the time solver enabled, the solver marches

```
M * dT/dt + K * T = f
```

with a theta scheme, where `alpha` is the time-march approximation coefficient:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

The *Approximation* setting of the time solver picks `alpha`:

| Approximation | `alpha` | Resulting march | Stability |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, an implicit step | unconditional |
| Central difference (accurate) | `0.5` | Crank-Nicolson | unconditional, can oscillate |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, an explicit step | conditional |

**Backward difference** is the safe default for a heat model. It costs a matrix
solve per step, but the step is limited by the accuracy you want rather than by
the size of the smallest element. **Central difference** is second-order
accurate and is the better choice once the time step is small enough to resolve
the transient, though it can ring at a sharp switch-on. **Forward difference**
puts the whole conduction matrix on the old time step, which makes each step
cheap and the run fragile.

The forward branch is stable only while the time step stays below roughly

```
dt < rho * c * h^2 / ( 2 * k )
```

where `h` is the smallest element size. Since the thermal diffusivity
`k/(rho*c)` of a metal is of the order of `1e-5 m^2/s`, a millimetre-sized
element already forces a time step of well under a tenth of a second. For
anything but a very fine time resolution, use the implicit march.

An unstable transient run is easy to recognise: the temperature field oscillates
from step to step with a growing amplitude, often between physically impossible
values, rather than settling. The cure is a smaller time step or the implicit
march.

### 1.9 Derived results

After the solve the element heat flux is recovered from the temperature field as
the average over the integration points of

```
Q = -k * grad(T)
```

evaluated in the element and rotated into global coordinates for line and
surface elements. On a surface that carries convection, the convective flux

```
Qh = h * ( Tf - T )
```

is added along the surface normal, so the flux vector of a convecting face
points the way the heat is actually leaving.

The flux is a **density** in `W/m^2`, independent of the size of the element it
was computed in, so it can be compared between elements and against a hand
calculation, and it converges as the mesh is refined.

The **heat transfer coefficient** is stored as well. It is the coefficient the
solver actually used on each surface element, which is the only way to see what
the *Forced convection* and *Natural convection* correlations produced - and on
a natural convection surface it varies from element to element, since it depends
on the local wall temperature. It is zero on every element that carries no
convection condition.

| Result | Apply to | Meaning |
|---|---|---|
| Temperature `[K]` | node | the solved temperature field |
| Heat flux `[W/m^2]` | element | conductive flux vector, plus the convective part on convecting surfaces |
| Heat transfer coefficient `[W/(m^2*K)]` | element | the convection coefficient used, zero where there is no convection |

After each record the solver log prints the statistics of all three, and the
values at any monitoring points.

### 1.10 Steady-state or transient?

| | Steady-state | Transient |
|---|---|---|
| Solves | `K*T = f` | `M*T' + K*T = f` in time |
| Answers | "how hot does it get in the end" | "how long does it take to get there" |
| Uses density and heat capacity | no, but they must still be present | yes |
| Needs a prescribed temperature or a convecting surface | yes | no, a capacity is enough |
| Records | one | one per written time step |

**Use a steady-state analysis for** the final temperature distribution of a
continuously operating part, sizing a heat sink, a thermal bridge in a wall, or
the temperature field that feeds a thermal stress analysis. This is the default
and covers most work.

**Use a transient analysis for** warm-up and cool-down, a duty cycle, a thermal
shock, quenching, or anything where the time constant of the part is what you
are after.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Environment conditions`,
`Material`, `Results` tabs), the **Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick **Heat transfer** - "Steady-state and transient
Heat-transfer".

Heat transfer combines freely with every other physics except *Modal analysis*.
The chains worth knowing are:

- *Heat transfer* then *Stress analysis* - thermal stress;
- *Heat transfer* and *Radiative heat transfer* in one iterating task group -
  conduction plus enclosure radiation;
- *Electro-statics* then *Heat transfer* - resistive heating.

To iterate a coupled pair, add a task group and set its number of iterations;
the group stops early when every task in it reports convergence.

### 2.2 Problem tab

**Time-solver** - shown for heat transfer, since it is a time-dependent problem
type. Leave it disabled for a steady-state analysis.

| Field | Meaning |
|---|---|
| Enable | switches the transient march on and off |
| Approximation | theta of the time march: backward, central or forward, see 1.8 |
| Start time | first time value in seconds; ignored on a solver restart |
| End time | `start time + time-step size * number of time-steps`, read-only |
| Time-step size | `dt` in seconds |
| Number of time-steps | how many steps to compute |
| Output frequency | write a result file every N steps; `0` writes only the last step |

A heat problem has no setup group box of its own. Everything else is assigned
per entity in the condition tabs.

### 2.3 Material tab

The `Material` tab lists the material assigned to the selected entity. Heat
transfer requires **all four** of these:

| Property | Units | Comment |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, about `50` for structural steel, `15` for stainless, `200` for aluminium |
| Heat capacity | `J/(kg*K)` | `c`, about `460` for steel |
| Density | `kg/m^3` | `rho`, about `7850` for steel |
| Emissivity | `-` | only used by radiative heat transfer, but required all the same |

An entity missing any of them is not solved, and the setup checker warns before
the run starts. If you are not doing radiative heat transfer, an emissivity of
`0` is enough to satisfy the requirement.

Every property is a table against temperature, so a temperature-dependent
conductivity is honoured - evaluated at the element temperature of the previous
run, which makes it worth iterating the task when the dependence is strong.

### 2.4 Boundary conditions tab

The tab is split into a manager list of available boundary conditions on top and
an editor for the selected one below. Select an entity in the `Model` tree
first; the manager then offers the conditions valid for that entity type and for
the selected problem. Tick one to assign it, then edit its components in the
lower tree.

The conditions of section 1.4 are offered. Notes on using them:

- **Temperature** is the one condition that holds a value rather than driving
  one. Assign it to the face, edge or point whose temperature you know.
- **Heat** is offered on every entity type and takes the total power in `W`; the
  solver spreads it over the entity for you. It is the condition to reach for
  when you know the wattage of whatever generates the heat.
- **Heat rate (unit volume)** on a volume and **Heat rate (unit area)** on a
  surface prescribe the same source as a density instead, for a source that is
  naturally rated per unit volume or area.
- The three **convection** conditions apply to surfaces only. Assign *Simple
  convection* unless you have a specific reason to let the solver correlate the
  coefficient for you; it is the predictable one. *Forced convection* prefers
  the fluid temperature and velocity computed by the fluid heat solver on the
  other side of the wall, and falls back to its own *Fluid temperature* and
  *Velocity* components where there is no such result.
- Every component is a table against time, so a prescribed temperature or a heat
  source can be given a time history for a transient run. This is how a duty
  cycle or a ramped heater is modelled.

### 2.5 Initial and environment conditions

The `Initial conditions` tab offers *Temperature*, which sets the starting field
of a transient run. Assign it to the whole model and give it the temperature the
part starts at.

The `Environment conditions` tab also offers *Temperature*, as a background
value for an entity. Both are read on the first run only, and a boundary
condition overrides both.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The heat solver uses the **CG** entry.

If a steady-state solve does not converge, the usual cause is a model with no
prescribed temperature and no convecting surface rather than a solver setting.
Check the boundary conditions before raising the iteration count.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record - for a heat problem, *Temperature*. In a
transient run the history is shown by `Report` -> `Monitoring points`, which is
the quickest way to read a warm-up curve or a time constant off a model.

### 2.8 Results and records

The `Results` tab lists the computed variables and controls the 3D view.
Temperature is a node scalar, heat flux an element vector, and the heat transfer
coefficient an element scalar that is zero away from the convecting surfaces.

The `Records` tab of the `Model` dock lists the result records:

- **steady-state analysis** - a single record;
- **transient analysis** - one record per written time step. Animating the
  records plays the warm-up.

`Report` -> `Solver log file` shows the full solver output, including the
temperature, heat flux and heat transfer coefficient statistics after each
record.

---

## 3. Tutorial - steady-state heat transfer

**Goal.** Compute the temperature distribution of a bar held hot at one end and
cooled by the surrounding air, and read the temperature at the cold end.

This tutorial assumes a meshed volume model - a long box works well. Use
`File` -> `New model`, draw a box with `Geometry` -> `Create element` or import
a geometry, and tetrahedralise it; the *Draw cube* tutorial covers this ground.
Mark the hot end face and the remaining faces as separate surface entities so
that conditions can be assigned to them.

### Step 1 - select the problem

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add a task and select **Heat transfer**.
3. Confirm with `OK`.

### Step 2 - assign the material

1. Select the volume entity in the `Model` tree.
2. Open the `Material` tab and assign a material with all four properties, for
   structural steel:
   - **Thermal conductivity** = `50` `W/(m*K)`
   - **Heat capacity** = `460` `J/(kg*K)`
   - **Density** = `7850` `kg/m^3`
   - **Emissivity** = `0`

The heat capacity and the density are not used by a steady-state solve, but they
must be present for the entity to be solved at all.

### Step 3 - keep the analysis steady-state

Open the `Problem` tab and leave **Enable** unticked in the *Time-solver* group
box. The solver then assembles and solves `K*T = f` once.

### Step 4 - prescribe the hot end

1. Select the surface entity at the hot end.
2. Open the `Boundary conditions` tab and tick **Temperature**.
3. Set the value to `353.15` K, that is 80 degrees Celsius.

Remember that all temperatures in Range FEA are absolute, in kelvin.

### Step 5 - cool the rest

1. Select the surface entity covering the remaining faces.
2. Open the `Boundary conditions` tab and tick **Simple convection**.
3. Set **Convection coefficient** = `10` `W/(m^2*K)` and **Fluid temperature** =
   `293.15` K.

A coefficient of about `10` is representative of a surface in still air; forced
air is nearer `50`, and water is in the hundreds or thousands.

This is the step that makes the model solvable. Without it the bar has a heat
input and no outlet, and a steady state does not exist.

### Step 6 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. The setup checker reports missing materials or missing boundary conditions
   before the run starts.
3. Follow the progress in `Report` -> `Solver log file`.

### Step 7 - inspect the results

- `Results` tab: display **Temperature**. The field should fall smoothly from
  the prescribed hot end towards the fluid temperature.
- The coldest point should stay above the fluid temperature. A temperature below
  `Tf` anywhere, or above the prescribed `353.15` K, means something is feeding
  energy the wrong way - check the sign of any heat source.
- **Heat flux** shows where the energy travels, in `W/m^2`. Multiply the value
  on the convecting faces by their area and check it against the power entering
  the bar.
- **Heat transfer coefficient** should read `10` on the convecting faces and `0`
  everywhere else. It is the quickest way to confirm that a convection condition
  reached the elements you meant, and the only way to see what a correlated
  condition computed.
- A monitoring point on *Temperature* at the cold end gives the answer as a
  number rather than a colour.

### Step 8 - optional, add a heat source

To model a heater embedded in the bar, select the volume entity, open the
`Boundary conditions` tab and tick **Heat**. To deliver a total of `P` watts
into the bar, enter `P` - the solver spreads it over the volume of the entity.

Use **Heat rate (unit volume)** instead when the source is rated per unit
volume; enter `P / V` `W/m^3` to deliver the same total - see 1.4.

The steady-state temperature must then rise until the convecting surface can
shed exactly that power - which is the energy balance check of section 5.

### Step 9 - optional, thermal stress

To carry the result into a structural analysis:

1. Add a **Stress analysis** task *after* the heat task in the problem task
   flow, and give the entity the structural properties it needs.
2. Assign a **Temperature** environment condition to the structural entity with
   the stress-free reference temperature, for example `293.15` K.

The stress solver picks up the computed element temperatures and adds the
thermal expansion term automatically.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The solve does not converge | no Temperature condition and no convecting surface, so the steady state is undefined |
| *Entity has material assigned which is missing required properties* | one of the four required properties is absent - often the emissivity |
| The whole model sits at one temperature | no heat is entering, or nothing drives a gradient away from the prescribed value |
| Temperatures are absurdly high | a density in `W/m^3` was entered into a *Heat* condition, which takes the total in watts, or the convection coefficient is far too small |
| Nothing is cooled | a *Forced convection* wall was disabled because the fluid heat result has the fluid at rest - the log names the entity |
| Temperatures come out near `293.15` K everywhere | the entity was not solved - check the log for a missing material property |

---

## 4. Tutorial - transient heat transfer

**Goal.** Start the same bar at room temperature, switch the hot end on, and
find out how long it takes to warm up.

### Step 1 - select the problem

The same **Heat transfer** task as in tutorial 3. If the model is still set up,
only the time solver settings and the initial condition need to be added.

### Step 2 - assign the material

The same four properties. The **heat capacity** and the **density** matter here:
together with the conductivity they set the thermal diffusivity `k/(rho*c)`,
which is what governs how fast the front travels.

### Step 3 - set the initial temperature

1. Select the volume entity.
2. Open the `Initial conditions` tab and tick **Temperature**.
3. Set it to `293.15` K.

Without it the model still starts from the default `293.15` K, but stating it
explicitly makes the model self-documenting and lets you start from any other
state.

### Step 4 - configure the time solver

Open the `Problem` tab and in the *Time-solver* group box set:

| Field | Value |
|---|---|
| Enable | ticked |
| Approximation | `Backward difference (stable)` |
| Start time | `0` |
| Time-step size | `1` |
| Number of time-steps | `300` |
| Output frequency | `10` |

*Backward difference* is the implicit march - see the table in 1.8 - and it is
the one to use here. It stays stable at a time step chosen for the physics
rather than for the mesh.

Pick the time step from the time constant you expect, aiming for something like
fifty steps over the interesting part of the curve. Section 5 gives a formula
for a first estimate. Writing every tenth step keeps the result file small
without losing the shape of the curve.

### Step 5 - keep the boundary conditions

The prescribed **Temperature** at the hot end and the **Simple convection** on
the remaining faces carry over unchanged. Each component is a table against
time, so the hot end can instead be given a ramp: open the component editor and
add time points to make the temperature rise over the first few seconds rather
than stepping instantly.

### Step 6 - add a monitoring point

`Problem` -> `Define monitoring points`, place a point at the cold end and set
its variable to **Temperature**. The warm-up curve is far easier to read as a
history than as a sequence of coloured pictures.

### Step 7 - solve

`Solution` -> `Start solver` (`Ctrl+R`). The log prints one block per time step:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

followed by the matrix solver iterations and the temperature statistics of that
step.

### Step 8 - inspect the results

- `Model` dock, `Records` tab: one record per written step. Step through them,
  or animate them, to watch the heat front move into the bar.
- `Results` tab: display **Temperature**. Fix the display range across the
  records so that the colours mean the same thing in every frame, otherwise the
  animation rescales itself and the progress is invisible.
- `Report` -> `Monitoring points` plots the warm-up curve at the cold end. Read
  the time constant off it: the time to cover about 63 per cent of the total
  rise.
- The last record should approach the steady-state result of tutorial 3. If it
  does not, the run was too short.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The temperature oscillates and grows from step to step | the explicit march with too large a time step - switch the approximation to *Backward difference (stable)* or reduce `dt` |
| The result barely changes over the whole run | the time step is far below the time constant, or the run is too short |
| The first step jumps to the final answer | the time step is far above the time constant, so the march is effectively steady-state |
| Only one record was written | the output frequency is `0`, which writes the last step only |
| The transient never reaches the steady state | that is often correct - check against the steady-state run before suspecting the solver |

---

## 5. Checking a model

The solver has no built-in verification suite, so it is worth validating a new
model against something you can compute by hand before trusting it. Four cheap
checks, in order of usefulness:

**Plane wall.** For a slab of thickness `L` and conductivity `k` held at `T1`
and `T2` on its two faces, the temperature is linear across the slab and the
flux is

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Model the slab with volume elements, prescribe both faces, confirm that the
mid-plane sits exactly half way and that the computed **Heat flux** matches `q`.
This catches wrong units, a wrong conductivity and a mesh that is not connected
the way you think it is. Refine the mesh once and confirm that both numbers stay
put.

**Wall with convection.** For the same slab held at `T1` on one face and cooled
by a fluid at `Tf` with a coefficient `h` on the other, the cooled surface
settles at

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Compare against the computed surface temperature. This validates the convection
condition, which is the part of a heat model that is easiest to get wrong.

**Lumped time constant.** For a transient check, take a body of volume `V` and
surface area `A` with no prescribed temperature, starting at `T0` and cooled by
convection alone. While the Biot number `h*V/(A*k)` stays below about `0.1` the
body is nearly isothermal and

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Read `tau` off the monitoring point history and compare. This simultaneously
validates the capacity matrix, the convection condition and your time step - and
it is the quickest way to choose a time step for a real model, since `tau/50` is
a good first guess.

**Energy balance.** For a steady-state model with a known input power `P`, the
convecting surface must shed exactly `P` at the final temperature:

```
P = h * A * ( Ts_mean - Tf )
```

Read the mean surface temperature from the statistics in the solver log and
check the balance. This is the one global check that catches a Heat value
entered as a total instead of a density.

For any of these, refine the mesh at least once and confirm the answer is
converging rather than drifting.

---

## 6. Limitations

### Modelling limitations

- **Linear conduction only.** The conductivity, capacity and density of an
  element are constants during a solve, read from the material table at the
  temperature of the previous run. A strongly temperature-dependent property
  needs an iterating task group to converge.
- **No phase change.** There is no latent heat and no enthalpy formulation, so
  melting, solidification, boiling and condensation are not modelled. A material
  table cannot represent the latent plateau.
- **No advection.** Energy moves by conduction inside the body and by convection
  across its boundary. A moving medium carries no energy with it; use *Heat
  transfer in fluids* for that.
- **Convection is a boundary model.** The correlated forms use one fixed
  correlation each - external flat plate for forced flow, horizontal plates for
  natural convection - with the hydraulic diameter as the characteristic length.
  The orientation of the surface, the flow regime and whether the flow is
  internal or external are not detected. When the coefficient matters, compute
  it yourself and enter it with *Simple convection*.
- **The natural convection coefficient is lagged.** It is evaluated at the wall
  temperature of the previous solve, so a single-pass steady-state run
  effectively uses the initial temperature difference.
- **Radiation only through the separate problem type.** There is no surface
  emissivity term in the heat solver itself; enclosure radiation requires a
  *Radiative heat transfer* task, its view factor matrix, and an iteration
  between the two.
- **Surfaces are isothermal through their thickness.** A surface entity conducts
  in its own plane only. Model a wall whose through-thickness gradient matters
  with volume elements.
- **Point elements do not conduct.** A point is a lumped capacity and a heat
  source, nothing more.
- **A steady-state model needs an outlet.** With neither a prescribed
  temperature nor a convecting surface the matrix is singular and the solve will
  not converge.
