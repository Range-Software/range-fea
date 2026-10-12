# Fluid Flow - Theory Manual and User Guide

This document describes the fluid flow solver of Range FEA - `RSolverFluid`,
which serves the **Incompressible viscous flow** problem type: the equations it
solves, the meaning of every input it accepts, the parts of the graphical user
interface that drive it, and a worked tutorial.

The two problem types solved on the flow field have their own manuals:
[Heat transfer in fluids](theory_fluid_heat.md) (`RSolverFluidHeat`) and
[Contaminant dispersion](theory_fluid_particle.md) (`RSolverFluidParticle`).

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - steady-state flow through a channel](#3-tutorial---steady-state-flow-through-a-channel)
4. [Checking a model](#4-checking-a-model)
5. [Limitations](#5-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknowns - the velocity and pressure fields

The primary unknowns of **Incompressible viscous flow** are the **velocity**
vector `v` in `m/s` and the **pressure** `p` in `Pa`, both at every node. Every
node carries **four** degrees of freedom - `vx`, `vy`, `vz` and `p` - so a mesh
with `N` nodes gives a system of `4N` equations.

That makes a flow model the most expensive physics type in Range FEA: four
unknowns per node against three for a structural model and one for a heat model,
a matrix that is neither symmetric nor positive definite, and the non-linear
iteration of section 1.6 wrapped around every linear solve.

The two companion problem types add one scalar each, solved on the flow field
the fluid solver produced:

| Problem type | Solver | Primary unknown | Units | DOF per node |
|---|---|---|---|---|
| Incompressible viscous flow | `RSolverFluid` | velocity and pressure | `m/s`, `Pa` | 4 |
| Heat transfer in fluids | `RSolverFluidHeat` | temperature | `K` | 1 |
| Contaminant dispersion | `RSolverFluidParticle` | particle concentration | `kg/m^3` | 1 |

Both companions **require** an *Incompressible viscous flow* task in the same
problem: they read the velocity field from its result and never compute it
themselves. Each is described in its own manual -
[Heat transfer in fluids](theory_fluid_heat.md) and
[Contaminant dispersion](theory_fluid_particle.md).

The default state of a node that no condition and no previous result touches is
zero velocity and zero pressure.

### 1.2 Governing equations

The solver implements the **incompressible Navier-Stokes equations** for a
Newtonian fluid with density `rho` and dynamic viscosity `mu`. Both are
material properties evaluated per element at its temperature - see 1.9 for
where that temperature comes from and how it makes the flow buoyant.
Momentum and mass balance read

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

and with the time solver disabled the transient term drops out, leaving the
steady-state balance

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `v` | velocity | `m/s` | solved for |
| `p` | pressure | `Pa` | solved for |
| `rho` | density | `kg/m^3` | material property |
| `mu` | dynamic viscosity | `kg/(m*s)`, that is `Pa*s` | material property |
| `g` | gravitational acceleration | `m/s^2` | Gravitational acceleration environment condition |

The pressure here is not a thermodynamic quantity. Incompressibility leaves it
as the Lagrange multiplier that enforces `div(v) = 0`, which has two practical
consequences worth internalising before reading any result:

- Only **pressure differences** are meaningful. A model in which no condition
  fixes a pressure anywhere determines the pressure field only up to an additive
  constant, and the matrix is singular in that constant - see 1.7.
- Pressure responds instantly across the whole domain. There is no acoustic wave
  and no speed of sound in this formulation, so a pressure applied at an outlet
  is felt at the inlet within the same solve.

The **Reynolds number** built from a characteristic length `L` and speed `V`,

```
Re = rho * V * L / mu
```

is the one number that decides whether a flow model is going to behave. Below
roughly `Re = 1` the convective term is negligible and the problem is nearly
linear; up to a few thousand the flow is laminar and the solver is in its
element; well beyond that the real flow is turbulent, and this solver does not
model turbulence - see section 5.

### 1.3 Finite element discretisation and stabilisation

Velocity and pressure are interpolated with the **same** linear shape functions
on each element - equal-order `P1/P1` and `Q1/Q1` interpolation. That
combination does not satisfy the inf-sup (Babuska-Brezzi) condition on its own
and would produce a checkerboard pressure field, so the formulation is
**stabilised**. Three stabilisation terms are added, each with its own
element-level parameter:

| Term | Purpose |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | stabilises the convective term, which would otherwise produce node-to-node oscillations in a convection-dominated flow |
| **PSPG** - pressure stabilising Petrov-Galerkin | makes equal-order velocity and pressure admissible, removing the checkerboard pressure mode |
| **LSIC** - least-squares on the incompressibility constraint | damps the divergence error, improving mass conservation element by element |

The SUPG and PSPG parameters are built from an element Reynolds number

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

so the upwinding is strong where the element is convection-dominated and fades
out where it is diffusion-dominated. The LSIC parameter is `|v| * h / 2`. These
parameters belong to the flow solver; the dispersion solver uses its own,
time-step limited SUPG parameter, see the
[Contaminant dispersion](theory_fluid_particle.md) manual.

The SUPG parameter uses the **local** element velocity and element length, so it
adapts element by element. The PSPG parameter uses a **global stream velocity**
and a global element length scale instead, which keeps the pressure
stabilisation uniform across the mesh. The stream velocity is taken from the
inflow conditions at the first pass of each solve - each time step in a
transient run - rather than from the field being solved,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

that is the area-weighted mean inflow speed, falling back to `1 m/s` when the
model has no inflow condition at all. The element length scale is derived from
the element volume: `cbrt(6*V/pi)` for a tetrahedron, `cbrt(V)` for a hexahedron.

**Element support.** The flow equations are assembled on **volume elements
only**, and only on **linear tetrahedra** (`TETRA1`) and **linear hexahedra**
(`HEXA1`). Any other volume element type stops the run with an error naming the
type. Surface elements take part only where they carry a *Pressure (implicit)*
condition, where they contribute a pressure traction and nothing else. Point and
line entities contribute nothing at all - unlike a heat model, they are not
lower-dimensional conductors, they are simply absent.

This is the sharpest practical difference from the other physics in Range FEA:
a fluid model must be a **meshed volume**. Generate the tetrahedral mesh with
`Geometry` -> `Volume` -> `Generate tetrahedral mesh` before expecting a flow
solve to do anything.

### 1.4 Boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Wall | explicit | point, line, surface | none |
| Wall (frictionless) | explicit | point, line, surface | none |
| Velocity (inflow) | explicit | surface | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | explicit | surface | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | explicit | surface | Pressure `[Pa]` |
| Pressure (implicit) | natural, traction | surface | Pressure `[Pa]` |

**Wall** is the no-slip condition. Every node of the entity has all three
velocity components removed from the system and held at zero. It takes no
components - there is nothing to enter, ticking it is the whole of the setup.
This is the condition for every solid boundary the fluid touches.

**Wall (frictionless)** is the slip, or symmetry, condition: the fluid may
travel along the surface but not through it. It is applied by finding the
**dominant global axis** of each surface element normal and holding only that
one velocity component at zero, leaving the other two free. Note what that
means: the condition is exact only for a surface whose normal lies along `x`,
`y` or `z`. On a surface tilted away from the global axes it constrains the
wrong direction, and the closer the normal comes to 45 degrees between two axes
the worse the approximation. Use it for the flat symmetry planes of a model
aligned with the global axes, and use *Wall* elsewhere.

**Velocity (inflow)** prescribes a velocity of the given magnitude directed
along the **average normal** of the surface entity, pointing into the fluid. The
sign is taken from the orientation of the entity's elements with respect to the
volume behind them, so a positive value means flow entering the domain and a
negative value means flow leaving it. You enter a speed, not a vector - the
direction comes from the geometry.

**Volumetric flow rate (inflow)** prescribes the same thing from the total flow
rate instead: the solver divides the rate by the area of the entity and applies
the resulting mean speed along the same average normal.

```
v = Q / A          A = area of the entity
```

Reach for it whenever the quantity you actually know is a throughput - a pump
rating, a duct in `m^3/s` - and for *Velocity (inflow)* when you know the speed.
Both hold all three velocity components at every node of the entity, so both
make the inlet a uniform plug flow; neither can produce a developed profile at
the inlet face. Give the model an entry length if the profile there matters.

**Pressure (explicit)** removes the pressure degree of freedom from every node
of the entity and holds it at the given value. This is the pressure equivalent
of a prescribed velocity, and one way to pin down the additive constant of
section 1.2.

**Pressure (implicit)** is the natural counterpart: rather than fixing the nodal
pressure it applies a **pressure traction** on the surface elements and leaves
the velocity there free, so the fluid decides how much flows out and at what
profile. The traction includes a hydrostatic head,

```
t = -( p + rho * g * h ) * n
```

where `h` is the height of the node measured **along the gravity direction from
the lowest point** of the model and `n` is the surface normal. The gravity
direction is the average of the *Gravitational acceleration* environment
conditions assigned to the surfaces carrying this condition. With no gravity the
head term vanishes and the traction is the entered pressure alone.

This is the outlet condition to reach for in nearly every model: it lets the
outflow profile develop instead of forcing it flat, and entering `0` makes the
outlet a reference at atmospheric pressure. *Pressure (explicit)* is the
stiffer, more prescriptive choice.

Every component of every one of these is a **table against time**, so an inlet
can be ramped up over the first seconds of a transient run, or a flow rate given
a duty cycle.

**Precedence.** Velocity conditions and pressure conditions are independent -
they constrain different degrees of freedom, so an inlet surface can carry a
velocity condition and an outlet a pressure condition without interfering. Among
conditions of the same kind on the same entity the last one read prevails, and a
*Wall* zeroes the velocity of its nodes after every other velocity condition has
been applied. An entity that is both an inlet and a wall is therefore a wall.

### 1.5 Environment and initial conditions

#### Environment conditions

| Environment condition | Components | Effect |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | body force `rho*g` on the entity, and the head term of *Pressure (implicit)* |
| Temperature | Temperature `[K]` | background temperature of a *Heat transfer in fluids* task; not read by the flow solver itself - see 1.9 |

**Gravitational acceleration** defaults to `(0, 0, -9.80665) m/s^2`, that is
gravity along negative `Z`. Assign it to the volume entity for the body force
and to the outlet surface so that the *Pressure (implicit)* head is measured
along the right direction.

Gravity enters the momentum equation as the body force `rho*g`. In a closed
domain of uniform density, a uniform gravity field produces a hydrostatic
pressure gradient and no motion - which is the correct answer. Where the density
varies with temperature, the body force varies with it and the difference
drives a **buoyant flow** - natural convection, see 1.9. The pressure result
always contains the hydrostatic part `rho*(g . x)`; nothing is subtracted.

#### Initial conditions

| Initial condition | Effect |
|---|---|
| Velocity | initial velocity field, applied on the first run |
| Pressure | initial pressure field, applied on the first run |

The initial temperature and concentration fields of the companion problem types
are described in their own manuals.

Initial conditions are read on the **first run only**; after that the fields
march from the previous result. That is what makes the restart workflow of
section 1.7 work: a transient run started with *Restart solver / continue* picks
up the converged steady-state field rather than starting from rest again.

A boundary condition overrides an initial condition where the two meet.

### 1.6 The non-linear iteration

The convective term `(v . grad) v` is quadratic in the unknown, so the flow
equations are **non-linear** and cannot be solved in one pass. The solver
linearises them and iterates: each pass assembles the residual of the momentum
and mass balance at the current field, solves for an **increment**, and adds it,

```
J * dx = -R(x)
x <- x + dx
```

where `x` holds the nodal velocities and pressures. The left-hand side is an
approximate Jacobian rather than the exact one, which makes the scheme a
modified Newton iteration: it converges more slowly than full Newton near the
answer, but the answer it converges to is set by the **residual** `R`, which is
the discretised Navier-Stokes equations themselves.

**The step is damped.** Because the matrix is not the exact derivative of the
residual, a full step `dx` can overshoot and leave the field further from a
solution than it started - the residual of the next pass then comes out higher
than the one before it. The solver watches for exactly that and applies only a
share of the computed step,

```
x <- x + omega * dx
```

starting at `omega = 1`. A pass which made the residual worse halves it, down to
a floor of `0.1`; a pass which improved it lets `omega` grow back by a quarter at
a time, so the retreat is quick and the recovery deliberate. The value in force
is printed as `Relaxation` in the log, and it stays at `1` on a run which never
overshoots - a well behaved flow model never notices the mechanism is there.
`omega` is reset to `1` at the start of each solve, which in a transient run
means at each time step.

Two consequences follow, and they drive the whole workflow of a flow model:

- **One pass is never enough.** A single iteration of a flow task returns
  whatever the first increment happened to give, which is not a solution of
  anything. A flow task must be wrapped in a **problem task group** with a
  number of iterations - hundreds for a steady-state run, tens per time step for
  a transient one.
- **The count is an upper bound, not a target.** The group also carries a
  **convergence value**, and it ends its iterations as soon as the flow task
  reports a convergence below it. Set the count high enough for the hardest
  step and let the convergence value stop each one when it has settled.

After every iteration the solver writes four numbers to the convergence file and
to the log:

| Quantity | Meaning |
|---|---|
| `Residual` | norm of the residual vector, the distance from a solution |
| `Convergence-R` | change in the residual since the previous iteration |
| `Convergence-V` | relative size of the velocity increment, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | relative size of the pressure increment, `\|\|dp\|\| / \|\|p\|\|` |

`Convergence-V` and `Convergence-P` are the ones the convergence value is
compared against. Both are dimensionless and both fall as the iteration settles:
they measure how large a step the field still takes against the size of the
field itself, so a value of `1e-5` means the solution moved by a hundred
thousandth of itself in that pass.

A small step is not on its own a solution, though. A nearly singular system - a
model with no pressure reference, or a mesh which cannot carry the Reynolds
number asked of it - takes tiny steps because it **cannot move**, not because it
has arrived, and its residual sits where it was or climbs. The test therefore
has two parts, and the group stops only when both hold:

| Part | Condition |
|---|---|
| The field has stopped moving | `Convergence-V` and `Convergence-P` are both below the convergence value |
| The field has actually come down | the residual has fallen to a **tenth** of what it was at the first pass of this solve |

The second part is fixed and not configurable - it is a sanity check on the
first, not a second knob to tune. The log prints it as `Residual ratio` against
its target, so it is visible how close a run that did not stop early came to
stopping. In a transient run each time step is its own solve, so the ratio is
measured again from the start of each step.

`Report` -> `Solver convergence` plots all four numbers. A converged steady-state
run shows the residual falling by several orders of magnitude and then
flattening, with `Convergence-V` and `Convergence-P` falling monotonically
towards zero. A residual that flattens high, oscillates, or climbs means the run
has not converged - and the group will not stop early on such a run, whatever the
increments do.

A convergence value of `0` disables the early exit and runs the full count, which
is how the solver behaved before the value existed. Reach for it when you want a
fixed amount of work per step regardless of what the residual does.

### 1.7 Steady-state analysis

With the time solver disabled the solver iterates the steady momentum and mass
balance to a fixed point. The linear system of each iteration is not symmetric,
so it is solved with **GMRES** with Jacobi preconditioning rather than with the
conjugate gradient method the symmetric physics use.

A steady-state flow model needs, at minimum:

- a meshed **volume** with density and dynamic viscosity assigned;
- **Wall** on every solid boundary;
- an **inflow** condition, or a pressure difference, to drive the flow;
- a **pressure reference** - a *Pressure (explicit)* or *Pressure (implicit)*
  surface. Without one the pressure is determined only up to a constant and the
  iteration will not settle.

A model whose boundaries are entirely walls and prescribed velocities has one
more trap: the prescribed inflow and outflow must **balance**, since an
incompressible fluid cannot accumulate. Prescribing 5 m^3/s in and 4 m^3/s out
asks for the impossible and the solve will not converge. Leaving the outlet as
*Pressure (implicit)* avoids the question entirely, which is the main reason to
prefer it.

The usual workflow is to converge a steady-state run first and use it as the
starting field of a transient run, because a transient run started from rest
spends its first time steps resolving the start-up transient rather than the
physics of interest.

### 1.8 Transient analysis

With the time solver enabled the solver marches the unsteady equations with a
theta scheme, where `alpha` is the time-march approximation coefficient:

| Approximation | `alpha` | Resulting march | Stability |
|---|---|---|---|
| Backward difference (stable) | `1` | fully implicit step | unconditional |
| Central difference (accurate) | `0.5` | Crank-Nicolson | unconditional, can oscillate |
| Forward difference (fast) | `0` | explicit step | conditional |

**Backward difference** is the setting to use for a flow model. The non-linear
iteration within each time step is what costs the time, and an implicit march
lets the time step be chosen from the physics rather than from the mesh.

Even with the implicit march the time step of a flow model is bounded by
accuracy, through the **Courant number**

```
C = |v| * dt / h
```

where `h` is the element size along the flow. A Courant number near `1` means
the fluid crosses about one element per step, which is the right order for
resolving a transient; much more than that and the solver is smearing the
transport it is supposed to be resolving, whatever the stability of the march.
Pick `dt` from the mesh and the expected speed:

```
dt ~ h / |v|
```

Each time step runs the full non-linear iteration of section 1.6, so the cost of
a transient run is the number of time steps times the iterations per step. Ten
to fifty iterations per step is a typical range once the flow field is already
converged from a steady-state run. A step whose residual has not fallen by the
end of its iterations is a step whose answer is not converged, and the error
carries into every step that follows.

### 1.9 Heat transfer and contaminant dispersion

Two problem types are solved on the flow field this solver produces, each with
its own manual:

- **Heat transfer in fluids** (`RSolverFluidHeat`) carries temperature with the
  flow and couples to the heat solver of a meshed solid - see
  [Heat transfer in fluids](theory_fluid_heat.md);
- **Contaminant dispersion** (`RSolverFluidParticle`) carries a contaminant
  concentration with the flow - see
  [Contaminant dispersion](theory_fluid_particle.md).

Both read the velocity from the flow result. Contaminant dispersion never
changes the flow. Heat transfer in fluids does, through the material
properties.

#### Temperature-dependent properties and buoyancy

On **every pass** the flow solver evaluates density and dynamic viscosity of
each element from the material tables at the element temperature left by the
latest heat transfer pass. Without a heat transfer task in the problem that
temperature is the default `293.15 K` everywhere. The *Temperature* environment
condition is not read by the flow solver directly; it reaches the flow only
through a heat transfer task.

The density in the body force `rho(T)*g` is what makes the flow buoyant: warm
fluid is lighter than the fluid around it and rises, cold fluid sinks. Natural
convection, thermal plumes and stratification therefore come out of a group of
an *Incompressible viscous flow* and a *Heat transfer in fluids* task, provided
the material has a **temperature-dependent density table**. The shipped *Water*
and *Air* do. A material whose density is a single value gives no buoyancy at
any temperature difference.

How the two tasks exchange data:

- **Steady state.** Put both tasks in one group, the flow task first, with
  several iterations and a convergence value. The first flow pass sees the
  initial temperature - the *Temperature* initial condition, or `293.15 K` - so
  it has no buoyancy yet. From the second iteration on, every flow pass sees the
  temperature of the heat pass before it. A model driven by buoyancy alone
  needs enough iterations for the two to settle together; a group with a single
  iteration never couples them.
- **Transient.** Each time step runs the group again, and every flow pass sees
  the latest temperature. With one iteration per time step the flow uses the
  temperature of the previous step - a lag of one step, which is acceptable
  while the time step resolves the thermal transient. Several iterations per
  step remove it.

The group ends when the flow task converges, since an uncoupled *Heat transfer
in fluids* task reports itself converged on every pass. A change of the
temperature moves the flow residual through `rho(T)` and `mu(T)`, so a flow
that has converged is also one the temperature no longer pushes.

The formulation keeps `rho(T)` in every term of the momentum balance and
`div(v) = 0` for mass. For the small density differences of natural convection
in a liquid, or in a gas over a few tens of kelvin, this gives the same answer
as the Boussinesq approximation. It is not a compressible formulation: the
thermal expansion of the fluid does not enter the mass balance, so a gas heated
by hundreds of kelvin is outside its range.

### 1.10 Derived results

| Result | Apply to | Produced by | Meaning |
|---|---|---|---|
| Velocity `[m/s]` | node | flow | the solved velocity field, three components |
| Pressure `[Pa]` | node | flow | the solved pressure field |

The results of the companion problem types are listed in their own manuals.

The velocity is stored as a node **vector**, so the `Results` tab can display
its magnitude as a colour, its components individually, or arrows. Two display
tools are worth knowing for a flow result specifically:

- a **cut plane** through the volume, since the interesting structure of a flow
  is inside the domain rather than on its surface;
- **stream lines**, created from `Geometry` -> `Stream line`, which trace the
  velocity field and are the quickest way to see a recirculation or a dead zone.

After each record the solver log prints the statistics of the velocity and the
pressure, and the values at any monitoring points.

### 1.11 Steady-state or transient?

| | Steady-state | Transient |
|---|---|---|
| Solves | the steady balance, iterated | the unsteady equations, marched in time |
| Answers | "what does the developed flow look like" | "how does the flow get there, and what does it carry" |
| Iterations | hundreds, in one task group | tens per time step |
| Records | one | one per written time step |
| Needs a pressure reference | yes | yes |

**Use a steady-state analysis for** the developed flow field of a duct, a
manifold or a heat exchanger passage, the pressure drop across a component, and
as the starting field for any transient run. This is where nearly every flow
model begins.

**Use a transient analysis for** start-up and shut-down, a pulsating or cyclic
inlet, vortex shedding, and - above all - for **contaminant dispersion**, which
is a transport problem in time and has no meaningful steady answer for a release
event.

The two-step workflow of the *Contaminant dispersion in fluids* tutorial is the
canonical one: converge the flow steady-state, then restart with the time solver
and the dispersion task enabled - see the
[Contaminant dispersion](theory_fluid_particle.md) manual.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Environment conditions`,
`Material`, `Results` tabs), the **Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick **Incompressible viscous flow** - "Steady-state and
transient flow of newtonian fluids".

A flow task belongs inside a **task group**, which carries the two values that
drive the non-linear iteration of 1.6. Double-click either to edit it:

| Value | Meaning |
|---|---|
| **# of iterations** | the most passes a single solve - or a single time step - may take |
| **Convergence** | the group ends its passes once the flow task has settled below this **and** its residual has come down; `0` always runs the full count |

The convergence value defaults to `1e-5`, which is a safe relative step size for
a flow model. Putting a flow task outside a group, where it runs exactly once,
is not useful - see 1.6.

The combinations worth knowing:

- *Incompressible viscous flow* alone, in a group of several hundred iterations
  - a steady-state flow field;
- *Incompressible viscous flow* and *Contaminant dispersion* in one group -
  dispersion on the flow, the classic transient pairing, see
  [Contaminant dispersion](theory_fluid_particle.md);
- *Incompressible viscous flow* and *Heat transfer in fluids* in one group - a
  heated or cooled flow, see [Heat transfer in fluids](theory_fluid_heat.md).

Flow excludes *Flow through porous media* and *Modal analysis*; the two
companion problem types each require a flow task to be present.

### 2.2 Problem tab

**Time-solver** - shown for all three fluid problem types, since all are
time-dependent. Leave it disabled for a steady-state flow run.

| Field | Meaning |
|---|---|
| Enable | switches the transient march on and off |
| Approximation | theta of the time march: backward, central or forward, see 1.8 |
| Start time | first time value in seconds; a restart from a steady-state result places that result at the start time, a restart of an interrupted transient run continues from its last step |
| End time | `start time + time-step size * number of time-steps`, read-only |
| Time-step size | `dt` in seconds, see the Courant guidance in 1.8 |
| Number of time-steps | how many steps to compute |
| Output frequency | write a result file every N steps; `0` writes only the last step |

The flow problem type has no setup group box of its own; the *Contaminant
dispersion setup* group box is described in the
[Contaminant dispersion](theory_fluid_particle.md) manual.
Everything else is assigned per entity in the condition tabs, and the iteration
count - which matters more here than anywhere else in Range FEA - lives in the
problem task flow dialog rather than on this tab.

### 2.3 Material tab

The `Material` tab lists the material assigned to the selected entity. The flow
problem type requires:

| Property | Units | Comment |
|---|---|---|
| Density | `kg/m^3` | `rho`, about `1000` for water, `1.2` for air |
| Dynamic viscosity | `kg/(m*s)` | `mu`, about `1.0e-3` for water, `1.8e-5` for air |

The companion problem types list their own properties in their manuals. An
entity missing any property required by a problem type in the task flow is
not solved, and the setup checker warns before the run starts. The shipped
material database includes **Water** and **Air**, which is usually the whole of
this step.

Note that the viscosity wanted is the **dynamic** viscosity `mu` in `Pa*s`, not
the kinematic viscosity `nu = mu/rho` in `m^2/s`. Entering `1.0e-6` for water
because that is the familiar number is the most common material mistake in a
flow model, and it makes the fluid a thousand times less viscous than it should
be - which usually shows up as a run that will not converge.

Every property is a table against temperature, evaluated at the element
temperature of the previous run.

### 2.4 Boundary conditions tab

The tab is split into a manager list of available boundary conditions on top and
an editor for the selected one below. Select an entity in the `Model` tree
first; the manager then offers the conditions valid for that entity type and for
the selected problem types. Tick one to assign it, then edit its components in
the lower tree.

The conditions of section 1.4 are offered. Notes on using them:

- **Wall** goes on every solid boundary and takes no value at all. A surface
  entity of a meshed volume that carries no condition is *not* a wall - it is an
  unconstrained boundary, which is rarely what is meant. Walls are the first
  thing to check in a model that behaves oddly.
- **Wall (frictionless)** is for symmetry planes aligned with the global axes.
  On a tilted surface it constrains the wrong direction - see 1.4.
- **Velocity (inflow)** and **Volumetric flow rate (inflow)** both drive flow in
  through a surface along its average normal. Pick whichever matches the number
  you actually have. A negative value drives flow the other way, which is one
  way to specify an outlet.
- **Pressure (implicit)** is the outlet condition to prefer: it lets the outflow
  profile develop and, entered as `0`, makes the outlet the pressure reference.
  **Pressure (explicit)** pins the nodal pressure instead.
- The conditions of *Heat transfer in fluids* and *Contaminant dispersion*
  appear once their task is in the task flow, and are described in their own
  manuals.
- Every component is a table against time. Click **Edit time dependent values**
  to open the component editor, where values are entered against time; between
  two given times the value is interpolated linearly, and after the last time
  the last value is kept. This is how a release event, a ramped inlet or a duty
  cycle is modelled.

### 2.5 Initial and environment conditions

The `Initial conditions` tab offers *Velocity* and *Pressure* for a flow
problem. They set the starting field of the first run and are what make a
transient run start from something other than rest.

The `Environment conditions` tab offers *Gravitational acceleration*, which
defaults to `(0, 0, -9.80665) m/s^2`. Assign it to the volume for the body
force, and to any surface carrying *Pressure (implicit)* so that the hydrostatic
head of that condition is measured along the right direction.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The flow solver uses the **GMRES** entry, since its matrix is not symmetric.

Distinguish the two nested iterations before changing anything here: the
**matrix solver** iterations solve one linearised system, while the **task
group** iterations drive the non-linearity of section 1.6. A flow model that
does not converge almost always needs more task group iterations, a better mesh
or a corrected set of boundary conditions - not a higher GMRES iteration count.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record - *Velocity* or *Pressure* for the flow. In a
transient run the history is shown by `Report` -> `Monitoring points`.

### 2.8 Results and records

The `Results` tab lists the computed variables and controls the 3D view.
Velocity is a node vector and pressure a node scalar.

The `Records` tab of the `Model` dock lists the result records:

- **steady-state analysis** - a single record;
- **transient analysis** - one record per written time step. Animating the
  records plays the flow, and the **Record** button at the bottom of the tree
  writes the animation to a video file.

`Report` -> `Solver convergence` plots the convergence history described in 1.6,
and is the report to open first after any flow run. `Report` ->
`Solver log file` shows the full solver output, including the velocity and
pressure statistics after each record.

---

## 3. Tutorial - steady-state flow through a channel

**Goal.** Compute the developed flow field and the pressure drop of a fluid
driven through a channel, and confirm that the result is converged.

This tutorial assumes a meshed volume model - the shipped **Channel.tmsh** model
is exactly this case, and the *Contaminant dispersion in fluids* tutorial uses
it as well. If you build your own, draw a box, tetrahedralise it, and mark the
inlet face, the outlet face and the remaining faces as three separate surface
entities so that conditions can be assigned to them.

### Step 1 - select the problem

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add a **task group** and set its **# of iterations** to `2000` by
   double-clicking the value.
3. Leave its **Convergence** at the default `1e-5`.
4. Add an **Incompressible viscous flow** task inside the group.
5. Confirm with `OK`.

The two values are the whole point of this step. The run takes up to 2000
passes and stops as soon as the field settles below `1e-5` - see 1.6. Set the
count generously: it is a ceiling, and the convergence value is what normally
ends the run.

### Step 2 - generate the mesh

`Geometry` -> `Volume` -> `Generate tetrahedral mesh`.

A flow model is solved on volume elements only. Without a volume mesh there is
nothing for the solver to assemble.

### Step 3 - assign the material

1. Select the volume entity in the `Model` tree.
2. Open the `Material` tab and assign **Water**, or enter:
   - **Density** = `1000` `kg/m^3`
   - **Dynamic viscosity** = `1.0e-3` `kg/(m*s)`

Check that the viscosity is the dynamic one in `Pa*s` and not the kinematic one
- see 2.3.

### Step 4 - keep the analysis steady-state

Open the `Problem` tab and leave **Enable** unticked in the *Time-solver* group
box.

### Step 5 - assign the walls

1. Select the surface entity covering the solid boundaries of the channel.
2. Open the `Boundary conditions` tab and tick **Wall**.

There is no value to enter. This is the no-slip condition, and it is what makes
the flow develop a profile instead of sliding through as a plug.

### Step 6 - drive the flow

1. Select the inlet surface entity.
2. Tick **Volumetric flow rate (inflow)** and enter the throughput in `m^3/s` -
   or tick **Velocity (inflow)** and enter a speed in `m/s` instead.

Before going on, estimate the Reynolds number from the mean speed and the
channel width:

```
Re = rho * V * L / mu
```

Below a few thousand the model is in the range this solver handles. Far above
it, expect the run not to converge, and read section 5 before spending time on
it.

### Step 7 - open the outlet

1. Select the outlet surface entity.
2. Tick **Pressure (implicit)** and set **Pressure** = `0` `Pa`.

This is the step that makes the model solvable. It gives the pressure field its
reference and lets the outflow profile develop rather than forcing it flat. A
model with an inlet and walls but no pressure condition anywhere has a singular
pressure and will not converge.

### Step 8 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. The setup checker reports missing materials or missing boundary conditions
   before the run starts.
3. Follow the progress in `Report` -> `Solver log file`.

This may take a while - a flow model of any size is real work, and the log
prints one block per iteration. Watch for `Convergence target` in the log: the
pass which reaches it is marked, and the group then reports *All sub-tasks have
converged* and stops.

### Step 9 - check the convergence

`Report` -> `Solver convergence`.

Do this before looking at any picture. The residual should fall by several
orders of magnitude and then flatten, and `Convergence-V` and `Convergence-P`
should approach zero. If the residual is still falling steeply at the last
iteration the run was too short - raise the iteration count and restart. If it
oscillates or climbs, the answer is not a solution and section 5 is the place to
look.

### Step 10 - inspect the results

- `Results` tab: display **Velocity**. The profile across the channel should be
  smooth, zero at the walls and fastest near the centre.
- Cut a plane through the volume - the interesting structure of a flow is inside
  the domain, not on its surface.
- Display **Pressure**. It should fall monotonically from inlet to outlet, and
  read `0` at the outlet where you set it.
- Create **stream lines** with `Geometry` -> `Stream line` to see recirculation
  or dead zones at once.
- Check the mass balance: the mean outlet speed times the outlet area should
  equal the volumetric flow rate prescribed at the inlet. This is the quickest
  global check that the model is doing what you asked - see section 4.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The residual never falls | no pressure condition anywhere, so the pressure is undetermined |
| The residual oscillates or climbs | the Reynolds number is too high for the mesh, or the viscosity was entered as kinematic |
| The flow slides through with no profile | no *Wall* condition on the solid boundaries |
| The result looks like the first iteration | the task was not placed in a task group, so it ran once |
| The run stopped far short of the iteration count | it converged - both the increments and the residual met the test of 1.6 |
| The run never stops early, however loose the convergence value | the residual is not coming down - read `Residual ratio` in the log, and treat the flow field as unconverged |
| *Failed to calculate element scales. Unsupported element type* | the mesh contains volume elements other than linear tetrahedra or hexahedra |
| Nothing was solved at all | no volume mesh was generated, or the material is missing density or viscosity |
| The prescribed inflow and outflow do not balance | an incompressible fluid cannot accumulate - use *Pressure (implicit)* at the outlet |

---

## 4. Checking a model

The solver has no built-in verification suite, so it is worth validating a new
model against something you can compute by hand before trusting it. Four cheap
checks, in order of usefulness:

**Mass balance.** For any converged incompressible model, what goes in must come
out:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

Read the mean velocity on the outlet from the statistics in the solver log,
multiply by the outlet area and compare with the prescribed inflow. This one
global check catches an unconverged run, a missing wall and a boundary condition
on the wrong entity, and it costs nothing.

**Hydrostatics.** Switch every inflow off, leave the domain closed with walls,
assign a *Gravitational acceleration* environment condition and solve. The fluid
must come to rest and the pressure must vary linearly with depth:

```
p(h) = p_ref + rho * g * h
```

Any residual motion is numerical. This validates the gravity body force, the
pressure reference and the units of the material in one run, and it is the
quickest first model to build in a new setup.

**Plane Poiseuille flow.** For a steady laminar flow between two parallel plates
a distance `H` apart, the profile is parabolic, the centreline speed is `1.5`
times the mean, and the pressure gradient is

```
dp/dx = 12 * mu * V_mean / H^2
```

Model a straight channel with walls top and bottom, a prescribed inflow and a
*Pressure (implicit)* outlet, and compare the computed centreline speed and
pressure drop with these. This is the sharpest check available: it validates the
viscous term, the no-slip wall and the units of the viscosity at once. Give the
model enough entry length for the profile to develop before measuring, since the
inlet condition imposes a flat profile.

**Hagen-Poiseuille flow.** The same check in a round pipe of diameter `D` and
length `L`, where the centreline speed is `2` times the mean and

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

valid while the flow is laminar, that is below about `Re = 2300`.

For any of these, refine the mesh at least once and confirm the answer is
converging rather than drifting - and confirm the convergence history has
flattened before comparing anything at all. An unconverged flow model will fail
every check here for the same reason.

---

## 5. Limitations

### Modelling limitations

- **No turbulence model.** The solver integrates the laminar Navier-Stokes
  equations. The SUPG, PSPG and LSIC terms are numerical stabilisation, not a
  turbulence closure - they do not represent the mixing or the extra dissipation
  of a turbulent flow. Above roughly `Re = 2300` in a pipe the real flow is
  turbulent and this solver does not model it: the run may still converge, but
  the answer describes a laminar flow that does not exist. There is no `k`-`e`
  model, no wall function and no large-eddy formulation.
- **Incompressible and Newtonian only.** The mass balance is `div(v) = 0` and
  the viscous stress is linear in the strain rate. Density and viscosity may
  depend on temperature, but not on pressure. Compressible flow, Mach number
  effects, free surfaces, multiple phases and non-Newtonian behaviour are
  outside the formulation.
- **Buoyancy without the expansion of the fluid.** A temperature-dependent
  density drives natural convection through `rho(T)*g` (see 1.9), but the
  thermal expansion never enters the mass balance. That is accurate for the
  small density differences of natural convection, not for a gas heated by
  hundreds of kelvin. Buoyant flow is laminar like every other flow here - a
  plume or a cavity above a Rayleigh number of roughly `1e8` to `1e9` is
  turbulent in reality.
- **The temperature reaches the flow only from a heat transfer task.** Without
  one, the material properties are evaluated at `293.15 K`; the *Temperature*
  environment condition alone does not change them.
- **Contaminant dispersion is one-way.** The flow carries the contaminant; the
  concentration never changes the flow. The limitations of the companion
  problem types are listed in their own manuals.
- **Volume elements only, linear tetrahedra and hexahedra only.** A flow model
  must be a meshed volume of `TETRA1` or `HEXA1` elements; anything else stops
  the run. Surfaces take part only to carry a *Pressure (implicit)* traction,
  and point and line entities contribute nothing.
- **The frictionless wall is axis-aligned.** It constrains the single dominant
  global component of each element normal, so it is exact only on a surface
  whose normal lies along `x`, `y` or `z`.
- **Inlets are uniform.** *Velocity (inflow)* and *Volumetric flow rate
  (inflow)* both impose a flat profile along the average surface normal. A
  developed inlet profile has to be produced by an entry length in the mesh.

### Solver limitations

- **The iteration is a modified Newton scheme**, so convergence near the answer
  is linear rather than quadratic. Expect hundreds of iterations for a
  steady-state run, and set the iteration count of the task group high enough
  that the convergence value, rather than the count, is what ends the run. The
  damping of 1.6 keeps such a run descending, but it cannot make it quadratic.
- **The central difference march of the flow solver is not Crank-Nicolson.** The
  theta weighting is applied to the matrix but not to the residual, so *Central
  difference (accurate)* solves a backward Euler residual with a half-stiffness
  matrix. The answer is the backward difference answer, reached more slowly. Use
  *Backward difference (stable)* for the flow until this is put right. The
  contaminant dispersion solver is not affected - it is linear and its march is
  a genuine Crank-Nicolson one.
- **The stabilisation parameters are not differentiated.** `Tsupg`, `Tpspg`,
  `Tlsic` and the element length all depend on the velocity, and all enter the
  matrix as constants evaluated at the current field. This is the usual practice
  for a stabilised formulation and is one of the reasons the Jacobian is not the
  exact one.
- **The residual part of the convergence test is fixed.** A solve counts as
  converged only once its residual has fallen to a tenth of what it was when
  that solve started, and that tenth cannot be changed. It is deliberately
  conservative: a slowly converging model may never satisfy it and will then run
  its full iteration count. The
  `Residual ratio` in the log says how close such a run came.
- **A pressure reference is required.** With no *Pressure (explicit)* and no
  *Pressure (implicit)* surface the pressure is determined only up to a constant
  and the iteration will not settle.
- **Prescribed flows must balance.** A model whose boundaries are all walls and
  prescribed velocities must have its inflow equal its outflow exactly; an
  incompressible fluid cannot absorb the difference.
- **Cost.** Four unknowns per node, a non-symmetric matrix, and a non-linear
  iteration around every linear solve make a flow model the most expensive
  problem type in Range FEA by a wide margin. Size the mesh accordingly, and
  converge a steady-state field before starting any transient run.
