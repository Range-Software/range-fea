# Heat Transfer in Fluids - Theory Manual and User Guide

This document describes the fluid heat solver of Range FEA - `RSolverFluidHeat`,
which serves the **Heat transfer in fluids** problem type: the equation it
solves, the meaning of every input it accepts, its coupling to the heat solver
of a solid, and the parts of the graphical user interface that drive it.

Heat transfer in fluids is solved on the flow field of an **Incompressible
viscous flow** task. The flow solver, its boundary conditions, its non-linear
iteration and its time march are described in the [Fluid flow](theory_fluid.md)
manual, and the heat solver of solids in the [Heat transfer](theory_heat.md)
manual.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Limitations](#3-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the temperature

The primary unknown is the **temperature** `T` in `K`, one degree of freedom per
node.

| Problem type | Solver | Primary unknown | Units | DOF per node |
|---|---|---|---|---|
| Heat transfer in fluids | `RSolverFluidHeat` | temperature | `K` | 1 |

The problem type **requires** an *Incompressible viscous flow* task in the same
problem: it reads the velocity field from its result and never computes it
itself.

### 1.2 Governing equation

**Heat transfer in fluids** solves the advection-diffusion of temperature on the
flow field,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

with the same SUPG stabilisation of the convective term as the flow solver and
the same theta time march. The velocity `v` is **recovered from the flow
result**, not solved here - which is why the problem type requires an
*Incompressible viscous flow* task.

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `T` | temperature | `K` | solved for |
| `v` | velocity | `m/s` | the flow result |
| `k` | thermal conductivity | `W/(m*K)` | material property |
| `c` | heat capacity | `J/(kg*K)` | material property |
| `rho` | density | `kg/m^3` | material property |
| `q` | heat source density | `W/m^3` | Heat condition, Joule heat, radiation |

The difference from the plain **Heat transfer** problem type is exactly the
`v . grad(T)` term: energy is carried along by the moving fluid rather than only
conducted. That also means this solver has no convection boundary conditions -
there is nothing to correlate, since the heat leaving a wall into the fluid is
resolved by the mesh rather than modelled.

### 1.3 Boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Temperature | explicit (Dirichlet) | point, line, surface, volume | Temperature `[K]` |
| Heat | natural, source | point, line, surface, volume | Heat `[W]` |

**Temperature** holds the nodes of its entity at the given value. Assign it to
the inlet surface to set the temperature of the fluid entering the domain, and
to a wall to model a heated or cooled boundary.

**Heat** is a source and takes the **total power in `W`** delivered into the
entity, spread over the measure of the entity before assembly - the same
convention as the heat solver. *Heat rate (unit area)* and *Heat rate (unit
volume)* are offered by the interface for this problem type but are **not read**
by the fluid heat solver; use *Heat* for a source here.

Every component is a **table against time**.

The flow conditions - walls, inlets and outlets - belong to the flow task and
are described in the [Fluid flow](theory_fluid.md) manual.

### 1.4 Conjugate heat transfer

A wall between a meshed fluid and a meshed solid carrying the **Forced
convection** condition of the plain *Heat transfer* solver couples the two. Once
the heat solver has run, the fluid heat solver holds the wall nodes at the
temperature of the solid - until then the wall is insulated - and hands back,
for every wall element, the pair `(h, Tf)` that reproduces the heat flux its
solution takes through the wall. The heat solver applies that pair as a
convection condition. The flux is the residual of the fluid equations at the wall
nodes, consistent with the discretisation, so it stays accurate in a thin thermal
boundary layer. The wall temperature is relaxed with an Aitken factor from pass
to pass, which settles the exchange in a handful of passes. The
[Heat transfer](theory_heat.md) manual describes the scheme under *Conjugate
heat transfer*. Radiative heat and Joule heat arriving from other solvers are
added to the source term the same way as in the heat solver.

### 1.5 Convergence

Uncoupled, the fluid heat solver reports itself as converged unconditionally -
it is linear once the flow field is fixed, so one solve is the final answer. A
group holding both the flow and the fluid heat task therefore ends when the flow
task converges. With coupled walls it reports the relative change of its
temperature field instead, so the group iterates until the wall temperature has
settled.

### 1.6 Coupling back to the flow - buoyancy

The temperature this solver produces is handed back to the flow. On every pass
the flow solver evaluates density and dynamic viscosity of each element at the
latest temperature, so with a material whose density depends on temperature the
body force `rho(T)*g` drives **natural convection** - warm fluid rises, cold
fluid sinks. The shipped *Water* and *Air* have temperature-dependent density.

In a steady-state run the first flow pass sees only the initial temperature,
so buoyancy appears from the second iteration of the group on; give the group
several iterations and a convergence value. In a transient run with one
iteration per time step the flow sees the temperature of the previous step. The
[Fluid flow](theory_fluid.md) manual describes the scheme in section 1.9.

### 1.7 Initial conditions

| Initial condition | Effect |
|---|---|
| Temperature | initial temperature field |

Initial conditions are read on the **first run only**; after that the field
marches from the previous result. A boundary condition overrides an initial
condition where the two meet.

### 1.8 Results

| Result | Apply to | Meaning |
|---|---|---|
| Temperature `[K]` | node | the solved temperature field |
| Heat flux `[W/m^2]` | element | conductive flux `-k*grad(T)` |

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Material`, `Results` tabs), the
**Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. The combinations worth knowing:

- *Incompressible viscous flow* and *Heat transfer in fluids* in one group - a
  heated or cooled flow. Give the group several iterations when the flow is
  driven or noticeably changed by buoyancy, so that flow and temperature settle
  together - see 1.6;
- *Heat transfer in fluids* and a separate *Heat transfer* task on a meshed
  solid, the fluid heat task first, in one group of several iterations -
  conjugate heat transfer through the walls carrying *Forced convection*.

The problem type requires a flow task to be present. The task group values -
**# of iterations** and **Convergence** - are described in the
[Fluid flow](theory_fluid.md) manual.

### 2.2 Problem tab

**Time-solver** - shown since the problem type is time-dependent. Leave it
disabled for a steady-state run. Its fields are described in the
[Fluid flow](theory_fluid.md) manual.

The problem type has no setup group box of its own. Everything else is assigned
per entity in the condition tabs.

### 2.3 Material tab

| Property | Units | Comment |
|---|---|---|
| Density | `kg/m^3` | `rho`, about `1000` for water, `1.2` for air |
| Dynamic viscosity | `kg/(m*s)` | `mu`, about `1.0e-3` for water, `1.8e-5` for air |
| Thermal conductivity | `W/(m*K)` | `k`, about `0.6` for water, `0.025` for air |
| Heat capacity | `J/(kg*K)` | `c`, about `4180` for water, `1005` for air |

An entity missing any required property is not solved, and the setup checker
warns before the run starts. The shipped material database includes **Water**
and **Air**, which is usually the whole of this step. Every property is a table
against temperature, evaluated at the element temperature of the previous run.

### 2.4 Boundary conditions tab

**Temperature** and **Heat** appear once a *Heat transfer in fluids* task is in
the task flow - see section 1.3. Every component is a table against time; click
**Edit time dependent values** to enter values against time.

### 2.5 Initial conditions

The `Initial conditions` tab offers *Temperature*. It sets the starting field of
the first run.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The fluid heat solver uses the **GMRES** entry, since its matrix is not
symmetric.

### 2.7 Monitoring points and results

`Problem` -> `Define monitoring points` can record *Temperature*. The `Results`
tab shows temperature as a node scalar and heat flux as an element vector.

---

## 3. Limitations

- **Buoyancy without the expansion of the fluid.** The temperature changes the
  flow through `rho(T)` and `mu(T)` (see 1.6), but the thermal expansion of the
  fluid never enters the mass balance `div(v) = 0`. That is accurate for the
  small density differences of natural convection, not for a gas heated by
  hundreds of kelvin. The buoyant flow is laminar, like every flow in Range FEA.
- **The coupling is staggered, not monolithic.** Flow and temperature are solved
  one after the other and exchange data between passes. A steady-state
  buoyant flow needs a group of several iterations; with one iteration per time
  step a transient flow lags the temperature by one step.
- **`Heat rate` conditions are ignored.** *Heat rate (unit area)* and *Heat rate
  (unit volume)* are offered by the interface for *Heat transfer in fluids* but
  are not read by `RSolverFluidHeat`; only the *Heat* condition reaches it. The
  plain *Heat transfer* problem type does read all three.
- **No convection boundary conditions.** Heat leaving a wall into a meshed fluid
  is resolved rather than correlated, so the *Simple*, *Forced* and *Natural
  convection* conditions belong to the plain heat solver. Conjugate heat
  transfer is set up as a *Heat transfer* task on the solid whose *Forced
  convection* walls read the computed fluid state.
- **No turbulence model.** The flow is laminar, see the
  [Fluid flow](theory_fluid.md) manual, so turbulent heat transfer is not
  represented.
