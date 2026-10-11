# Contaminant Dispersion - Theory Manual and User Guide

This document describes the contaminant dispersion solver of Range FEA -
`RSolverFluidParticle`, which serves the **Contaminant dispersion** problem
type: the equation it solves, the meaning of every input it accepts, the parts
of the graphical user interface that drive it, and a worked tutorial.

Contaminant dispersion is solved on the flow field of an **Incompressible
viscous flow** task. The flow solver, its boundary conditions, its non-linear
iteration and its time march are described in the [Fluid flow](theory_fluid.md)
manual, which this one builds on.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - transient flow with contaminant dispersion](#3-tutorial---transient-flow-with-contaminant-dispersion)
4. [Checking a model](#4-checking-a-model)
5. [Limitations](#5-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the particle concentration

The primary unknown is the **particle concentration** `C` in `kg/m^3`, one
degree of freedom per node.

| Problem type | Solver | Primary unknown | Units | DOF per node |
|---|---|---|---|---|
| Contaminant dispersion | `RSolverFluidParticle` | particle concentration | `kg/m^3` | 1 |

The problem type **requires** an *Incompressible viscous flow* task in the same
problem: it reads the velocity field from its result and never computes it
itself.

### 1.2 Governing equation

**Contaminant dispersion** transports a scalar concentration on the flow field,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

with SUPG stabilisation and the theta time march of the flow solver, reading the
velocity from the flow result.

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `C` | particle concentration | `kg/m^3` | solved for |
| `v` | velocity | `m/s` | the flow result |
| `D` | diffusion coefficient | `m^2/s` | *Contaminant dispersion setup*, `0` by default |
| `C_sat` | maximum saturation | `kg/m^3` | *Contaminant dispersion setup*, `0` (unlimited) by default |
| `s` | particle source rate | `kg/(m^3*s)` | Particle rate condition |

### 1.3 Contaminant dispersion setup

The two parameters of the problem type are set once per model in the
*Contaminant dispersion setup* group box of the `Problem` tab (section 2.2).
Both default to zero, which reproduces pure advection with an unlimited source.

#### Diffusion coefficient

**Diffusion coefficient** `D` spreads the contaminant down its concentration
gradient. With `D = 0` the transport is purely advective: the contaminant is
carried by the flow and spreads only through the numerical diffusion of the
stabilisation, so the solver answers "where does the flow take it, and how long
does it take", not "how wide is the plume". A positive `D` makes the width of
the plume a physical result.

There is no turbulence model (section 5), so `D` is an **effective**
diffusivity: the mixing of a real flow is dominated by turbulence, which can be
estimated as `D ~ nu_t / Sc_t` with the turbulent Schmidt number `Sc_t ~ 0.7`.

| Molecular diffusion (the physical property) | `D [m^2/s]` |
|---|---|
| gases and vapours in air (water vapour `2.5e-5`, CO2 `1.6e-5`, solvent vapours about `1e-5`) | `1e-6` - `1e-4` |
| dissolved species in water (O2 `2e-9`, salts about `1.5e-9`) | `1e-10` - `1e-8` |
| aerosol particles in air, Brownian motion (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

At engineering scales molecular diffusion is almost always negligible next to the
transport by the flow. The value normally entered is an effective one:

| Effective (turbulent) diffusion | `D_eff [m^2/s]` |
|---|---|
| ventilated room, indoor air mixing | `1e-3` - `1e-2` |
| duct or channel flow, about `D ~ 0.01 - 0.05 * U * L` (`U` mean velocity, `L` duct size) | `1e-3` - `1e-1` |
| rivers, transverse mixing | `1e-2` - `1` |
| atmosphere, horizontal mixing | `1` - `100` |

Whether a value matters depends on the mesh. With mean velocity `U`, flow length
`L`, element size `h` and time step `dt`:

- **when `D` has any visible effect** - the contaminant spreads by about
  `sqrt( 2 * D * L / U )` while it crosses the domain. If that is smaller than
  `h`, `D` does nothing, so `D` only matters above roughly `U * h^2 / ( 2 * L )`.
  For a channel with `U ~ 1 m/s`, `L ~ 15 m` and `h ~ 0.1 m` that is about
  `3e-4 m^2/s`;
- **when `D` dominates** - above about `U * L` (about `15 m^2/s` for the same
  channel) diffusion smears out everything and the flow barely matters;
- **time stepping** - if `D * dt / h^2` exceeds about `1`, switch to *Backward
  difference (stable)*; the central difference march does not damp the fastest
  diffusive modes and oscillates.

#### Maximum saturation

**Maximum saturation** `C_sat` is the largest concentration the contaminant can
reach in the fluid - a saturation vapour concentration or a solubility. When it
is set, a positive *Particle rate* is weakened as the concentration approaches
it and stops at saturation, `s * ( 1 - C / C_sat )`; this models evaporation or
dissolution, and `s` is then the rate into a clean fluid, about
`k_m * C_sat * A / V`. Sinks (`s < 0`) are not limited. The limit acts only on
the source: a concentration prescribed by a boundary or initial condition is not
limited, and the solver log warns when it exceeds `C_sat`. Concentration is not
clipped either, so a slight overshoot next to a steep front is still possible.

A saturation limit exists only where there is a thermodynamic one: a vapour
pressure or a solubility. For a vapour in air the saturation concentration
follows from the saturation vapour pressure, `C_sat = p_sat * M / ( R * T )`:

| Vapour in air | Temperature | `C_sat [kg/m^3]` |
|---|---|---|
| water vapour | 0 degC | `0.0048` |
| water vapour | 20 degC | `0.017` |
| water vapour | 40 degC | `0.051` |
| water vapour | 100 degC | `0.60` |
| ethanol or toluene | 20 degC | about `0.11` |
| methanol | 20 degC | about `0.17` |
| benzene | 20 degC | about `0.32` |
| acetone | 20 degC | about `0.59` |

For a substance dissolved in water the limit is its solubility:

| Dissolved in water | `C_sat [kg/m^3]` |
|---|---|
| oxygen in equilibrium with air, 20 degC | `0.009` |
| carbon dioxide at 1 atm | `1.7` |
| gypsum | `2.4` |
| sodium chloride | about `360` |

Particles, dust and smoke have no saturation; leave the value at `0`
(unlimited). For scale only, workplace exposure limits are about
`1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) and dust explosion lower limits about
`0.02` - `0.06 kg/m^3`.
Saturation depends strongly on temperature, while the solver uses one constant
value: for a non-isothermal case take the value at the coldest relevant
temperature, where condensation would start.

### 1.4 Stabilisation

The SUPG parameter of the dispersion solver is limited by the time step and
includes the diffusion,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

with the `dt` term dropped in a steady-state analysis. It reduces to
`h / ( 2 * |v| )` where the flow is fast, never exceeds `dt / 2`, and approaches
`h^2 / ( 12 * D )` - the limit of the nodally exact one-dimensional parameter -
where diffusion dominates. The time-step limit matters in slow and recirculating
regions: there the element Courant number `|v| * dt / h` is tiny and an
unlimited `tau` would turn the stabilisation into noise that creates
concentration ahead of the front. This parameter is the dispersion solver's own;
the flow solver uses the one described in the [Fluid flow](theory_fluid.md)
manual.

The Galerkin advection term is assembled in **advective form with a divergence
sink**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, with the continuous nodal
velocity. A velocity from the flow solver is never exactly divergence free - on
a coarse mesh `div(v)` of tens of `1/s` is common next to walls and corners.
Where the flow expands (`div(v) > 0`) plain advection would create
concentration on its own, a checkerboard pattern that grows from step to step
once the time step is small; the sink cancels it. Where the flow converges no
term is added: a symmetric correction `0.5 * div(v) * C` would act there as a
source and pile the contaminant up next to walls. For an exactly divergence
free velocity the sink is zero.

With a negligible diffusion coefficient the transport is almost pure advection,
which Galerkin with SUPG does not keep monotone: expect over- and undershoots,
largest at no-slip walls where the velocity is zero. An effective diffusion
coefficient (section 1.3) removes most of them.

Unlike the flow solver, the dispersion solver applies the theta weighting
consistently, so *Central difference (accurate)* is a genuine Crank-Nicolson
march here.

### 1.5 Boundary conditions

| Boundary condition | Type | Applies to | Components |
|---|---|---|---|
| Particle concentration | explicit (Dirichlet) | point, line, surface | Particle concentration `[kg/m^3]` |
| Particle rate | natural, source | point, line, surface, volume | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | natural (Neumann) | surface | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | natural (Robin) | surface | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** holds its nodes at the given concentration. Assigned
to an inlet surface with a table against time, it is how a release is modelled:
zero before the event, the release concentration during it, zero afterwards.
This is the single most useful idiom of the problem type, and the one the
tutorial of section 3 builds on.

**Particle rate** is a source and is a **density** in `kg/(m^3*s)` rather than a
total - unlike the *Heat* condition, it is not spread over the entity measure.
It models a source releasing continuously inside the domain.

**Particle flux (unit area)** prescribes the mass flux `q` entering the domain
through a surface, `D * grad(C) . n = q` with `n` the outward normal. Unlike
*Particle rate* it is a flux per unit area, so the total release is `q * A`.
Positive `q` is emission - an evaporating film, an off-gassing wall - and is
limited by the maximum saturation the same way as a positive particle rate,
`q * ( 1 - C / C_sat )`. Negative `q` is a fixed removal rate regardless of
the local concentration and can drive it below zero; deposition proportional
to the concentration is *Particle mass transfer* with `C_inf = 0`. On a wall
the flux enters by diffusion alone; on a part of the boundary where the flow
enters, the inflow is treated as clean fluid carrying the flux, so `q` becomes
the total incoming mass flux and `C ~ q / |v . n|`. Assigned to an internal
surface between two volumes, it releases `q` per unit area as a sheet source.

**Particle mass transfer** exchanges contaminant with an ambient reservoir,
`q = k_m * ( C_inf - C )`, where `k_m` is the *Mass transfer coefficient* and
`C_inf` the *Ambient particle concentration*. It is the contaminant counterpart
of simple convection in heat transfer. With `C_inf = 0` it models deposition on
a surface with deposition velocity `k_m`; with `C_inf > 0` an exchange with
ambient air through an opening, or a dissolving surface held at `C_inf`. Typical
values of `k_m` are `1e-4` - `1e-2 m/s` for particle deposition in rooms and
`1e-3` - `1e-1 m/s` for evaporation into a moving gas. A large `k_m` drives the
surface towards `C_inf` and approaches a *Particle concentration* condition.

Every component is a **table against time**, so a release can start and stop
during a transient run.

**Boundaries without a condition.** Walls and outlets without a *Particle
concentration*, *Particle flux* or *Particle mass transfer* condition keep the
natural condition of **zero diffusive flux**: the contaminant does not diffuse
through a wall, and leaves through an outlet only with the flow.

Where the flow **enters** through such a boundary - typically backflow at part
of a *Pressure (implicit)* outlet - it is treated as clean fluid: the solver
adds the weak inflow term `max(-v . n, 0) * C` on that part of the surface,
spread over the face so that it also damps the edge nodes where the outlet meets
a no-slip wall.
Without it the boundary would feed the solution and the concentration at the
outlet would grow without bound. If the re-entering fluid is not clean, extend
the domain so that the outlet sees only outflow.

The flow conditions - walls, inlets and outlets - belong to the flow task and
are described in the [Fluid flow](theory_fluid.md) manual.

### 1.6 Initial conditions

| Initial condition | Effect |
|---|---|
| Particle concentration | initial concentration field |

Initial conditions are read on the **first run only**; after that the field
marches from the previous result. A boundary condition overrides an initial
condition where the two meet.

### 1.7 Steady-state or transient?

Contaminant dispersion is above all a **transient** problem: a transport problem
in time, with no meaningful steady answer for a release event. A steady-state
dispersion answers a different question - the concentration field a continuous
source settles to - and is a single linear solve on the flow field (see
section 2.6 on its matrix solver).

The two-step workflow of the tutorial in section 3 is the canonical one:
converge the flow steady-state, then restart with the time solver and the
dispersion task enabled.

### 1.8 Results

| Result | Apply to | Meaning |
|---|---|---|
| Particle concentration `[kg/m^3]` | node | the solved concentration field |
| Relative saturation `[-]` | node | `C / C_sat`, only when a maximum saturation is set |

Relative saturation is displayed from `0` to `1` by default and is removed when
the maximum saturation is set back to zero.

Only the material **density** is required for this problem type.

---

## 2. Graphical user interface

Everything relevant lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Material`, `Results` tabs), the
**Problem** menu and the **Model** dock.

### 2.1 Selecting the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Put a **Contaminant dispersion** task in the same task group as an
**Incompressible viscous flow** task - dispersion on the flow, the classic
transient pairing. The problem type requires the flow task to be present.

The task group values - **# of iterations** and **Convergence** - drive the
non-linear iteration of the flow and are described in the
[Fluid flow](theory_fluid.md) manual. In a transient run they are the
iterations *per time step*.

### 2.2 Problem tab

**Time-solver** - leave it disabled for a steady-state dispersion, enable it
for a release event.

| Field | Meaning |
|---|---|
| Enable | switches the transient march on and off |
| Approximation | theta of the time march: backward, central or forward |
| Start time | first time value in seconds |
| End time | `start time + time-step size * number of time-steps`, read-only |
| Time-step size | `dt` in seconds |
| Number of time-steps | how many steps to compute |
| Output frequency | write a result file every N steps; `0` writes only the last step |

**Contaminant dispersion setup** - shown once a *Contaminant dispersion* task is
in the task flow. It holds the two parameters of section 1.3:

| Field | Meaning |
|---|---|
| Maximum saturation `[kg/m^3]` | largest possible concentration; a positive particle rate stops at it and *Relative saturation* is computed. `0` means unlimited |
| Diffusion coefficient `[m^2/s]` | effective diffusivity of the contaminant in the fluid. `0` means pure advection |

Typical values of both are listed in section 1.3.

### 2.3 Material tab

| Property | Units | Comment |
|---|---|---|
| Density | `kg/m^3` | `rho`, about `1000` for water, `1.2` for air |

The flow task in the same problem needs the dynamic viscosity as well. The
shipped material database includes **Water** and **Air**, which is usually the
whole of this step.

### 2.4 Boundary conditions tab

The conditions of section 1.5 are offered once a *Contaminant dispersion* task
is in the task flow. Select an entity in the `Model` tree first; the manager
then offers the conditions valid for that entity type. Tick one to assign it,
then edit its components in the lower tree.

Every component is a table against time. Click **Edit time dependent values**
to open the component editor, where values are entered against time; between
two given times the value is interpolated linearly, and after the last time the
last value is kept. This is how a release event is modelled.

### 2.5 Initial conditions

The `Initial conditions` tab offers *Particle concentration*. It sets the
starting field of the first run.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
The dispersion solver uses the **GMRES** entry, since its matrix is not
symmetric.

A **steady-state contaminant dispersion with diffusion** needs attention here.
It is a single linear solve, so there are no task group iterations to fall back
on, and a diffusion-dominated system converges slowly under the default GMRES
limit of 10 inner times 10 outer iterations. The matrix solver does not warn
when it reaches its limit: read its iteration table in the solver log, and if
it runs to the last outer iteration with the residual still well above the
solver convergence value, raise the number of outer iterations. An unconverged
solve gives a wrong answer without any other sign.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record - *Particle concentration*, *Particle rate* or
*Relative saturation*. In a transient run the history is shown by `Report` ->
`Monitoring points`.

A monitoring point at the location of interest is the answer in the form you
usually want it: the concentration against time at a given place, read off a
graph rather than out of a sequence of pictures.

### 2.8 Results and records

The `Results` tab lists the computed variables and controls the 3D view.
Particle concentration and relative saturation are node scalars.

The `Records` tab of the `Model` dock lists the result records - one per written
time step of a transient run. Animating the records plays the dispersion, and
the **Record** button at the bottom of the tree writes the animation to a video
file.

---

## 3. Tutorial - transient flow with contaminant dispersion

**Goal.** Release a contaminant into a converged flow and follow the plume
through a channel in time.

This is the two-step workflow of section 1.7, and it mirrors the shipped
*Contaminant dispersion in fluids* tutorial.

### Step 1 - start from a converged flow

Run the steady-state channel tutorial of the [Fluid flow](theory_fluid.md)
manual to convergence first. A dispersion run started from rest wastes its first
time steps developing the flow field rather than transporting anything.

### Step 2 - add the dispersion task

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Keep the task group and the **Incompressible viscous flow** task.
3. Add a **Contaminant dispersion** task inside the same group.
4. Reduce the group's **# of iterations** to something like `20` - these are now
   the non-linear iterations *per time step*, not the iterations of a
   steady-state solve. Leave the **Convergence** value alone; most steps will
   settle before the twentieth pass and end early, and the ones that do not are
   the ones worth the full twenty.

*Contaminant dispersion* requires the flow task to be present; it reads the
velocity field and never computes it.

### Step 3 - configure the time solver

Open the `Problem` tab and in the *Time-solver* group box set:

| Field | Value |
|---|---|
| Enable | ticked |
| Approximation | `Backward difference (stable)` |
| Start time | `0` |
| Time-step size | from the Courant guidance below |
| Number of time-steps | enough to carry the plume through the domain |
| Output frequency | `1` for a smooth animation, more to keep the file small |

Pick the time step from the mesh and the flow speed so that the fluid crosses
about one element per step:

```
dt ~ h / |v|
```

Then pick the number of steps from how long the plume needs to traverse the
domain, `L / |v|`, divided by that step.

In the *Contaminant dispersion setup* group box of the same tab, leave both
values at `0` for pure advection, or enter an effective **Diffusion
coefficient** from the table in section 1.3 if the width of the plume matters.
A **Maximum saturation** only makes sense for a vapour or a dissolved substance;
it must not be below the release concentration of Step 4.

### Step 4 - release the contaminant

1. Select the inlet surface entity.
2. Open the `Boundary conditions` tab and tick **Particle concentration**.
3. Rather than entering a single value, click **Edit time dependent values**.
4. In the component editor enter a release profile - for example `0` at time
   `0`, the release concentration at the moment the release starts, and `0`
   again when it ends.

Between two given times the value is interpolated linearly, and after the last
time the last value is kept, so a release that should start abruptly needs two
entries close together. This time-dependent condition is
what turns a steady inlet into a release event, and it is the heart of a
dispersion model.

### Step 5 - keep the flow conditions

The **Wall**, the inflow condition and the **Pressure (implicit)** outlet carry
over unchanged. The flow is still being solved at every time step - the
dispersion rides on it.

### Step 6 - add a monitoring point

`Problem` -> `Define monitoring points`, place a point downstream and set its
variable to **Particle concentration**. The arrival time and the peak
concentration at a given place are far easier to read off a history than out of
a sequence of coloured pictures.

### Step 7 - solve with restart

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. Tick **Restart solver / continue**.

The restart is what makes the converged steady-state field the starting point of
the march. Without it the run starts from rest and the first steps are wasted.

### Step 8 - inspect the results

- `Model` dock, `Records` tab: one record per written step. Step through them,
  or animate them, to watch the plume travel.
- `Results` tab: display **Particle concentration**. Fix the display range
  across the records so the colours mean the same thing in every frame -
  otherwise the animation rescales itself and the plume appears not to decay.
- `Report` -> `Monitoring points` plots the concentration history at the probe.
  Read the arrival time off it and compare with `L / |v|`, the travel time the
  mean flow implies - that is the cheapest validation of the whole model.
- With a zero diffusion coefficient the plume spreads only by numerical
  diffusion, so its **width** is a property of the mesh and the time step, not
  of the physics. With an effective diffusion coefficient the width is a
  result - check it against a finer mesh before trusting it.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The concentration never leaves the inlet | the flow field is zero - the flow task is missing from the group, or the run was not restarted from a converged flow |
| The plume smears out almost immediately | the Courant number is far above 1, or the mesh is too coarse along the path |
| The concentration goes negative or overshoots | the time step is too large for the mesh, so the stabilised advection is ringing. Slight undershoots next to a steep front are normal; the solution is not clipped at zero, because clipping would add contaminant |
| Only one record was written | the output frequency is `0`, which writes the last step only |
| Each step's residual stays high | too few iterations per time step - the flow within each step is not converged |
| The plume never spreads sideways | the diffusion coefficient is zero, so lateral spreading comes only from the flow field - enter an effective diffusion coefficient, see section 1.3 |
| The plume spreads over the whole domain at once | the diffusion coefficient is far too large - compare it with `U * L` |
| The log warns that prescribed concentration exceeds maximum saturation | a boundary or initial condition is above the maximum saturation; the limit acts only on the particle rate, so check both values |
| A steady-state solve with diffusion looks wrong | the matrix solver stopped on its iteration limit - raise the GMRES outer iterations, see section 2.6 |
| The concentration overshoots the inlet value or goes negative next to walls, but does not keep growing | almost pure advection - the diffusion coefficient is close to zero. Enter an effective diffusion coefficient; on a coarse mesh also check that the flow field is converged |
| The concentration exceeds the inlet value and grows into a noisy pattern | the flow is not converged - `Residual ratio` stays near `1` every step. Its velocity is far from divergence free and no transport on it can be trusted; converge the flow first (mesh, Reynolds number, iterations per step) |

---

## 4. Checking a model

**Travel time.** The time a marker takes to travel a distance `L` at mean speed
`V` is `L/V`. Read the arrival time off a monitoring point history and compare.
This validates that the dispersion task is riding on the flow field you think it
is, and it catches a run that was not restarted from a converged flow.

The flow field itself is best checked first, with the mass balance and the
analytical flows listed in the [Fluid flow](theory_fluid.md) manual. An
unconverged flow model fails every check for the same reason, and no transport
on it can be trusted.

---

## 5. Limitations

- **No turbulence model.** The flow is laminar, see the
  [Fluid flow](theory_fluid.md) manual, so the mixing of a turbulent flow has to
  be entered as an effective diffusion coefficient.
- **Constant, isotropic diffusion.** The diffusion coefficient is a single value
  for the whole model; it cannot vary in space or time, follow the direction of
  the flow, or come from a turbulence model. It has to be estimated as an
  effective diffusivity - see section 1.3.
- **Constant maximum saturation.** The saturation does not follow the
  temperature of a *Heat transfer in fluids* result, and it limits only a
  positive particle rate or particle flux. Concentrations prescribed by
  conditions are not limited, and the field is not clipped.
- **The coupling to the flow is one-way.** The flow drives the transport; the
  transport never changes the flow.
- **Not monotone for near-pure advection.** Galerkin with SUPG produces over- and
  undershoots when the diffusion coefficient is close to zero, largest at no-slip
  walls - see section 1.4.
