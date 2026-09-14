# Acoustic Analysis - Theory Manual and User Guide

This document describes the acoustic solver (`RSolverAcoustic`) of Range FEA:
the equations it solves, the meaning of every input it accepts, the parts of the
graphical user interface that drive it, and two worked tutorials.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - transient acoustic analysis](#3-tutorial---transient-acoustic-analysis)
4. [Tutorial - harmonic acoustic analysis](#4-tutorial---harmonic-acoustic-analysis)
5. [Verification](#5-verification)
6. [Limitations](#6-limitations)

---

## 1. Theoretical background

### 1.1 Primary unknown - the velocity potential

The acoustic solver does not solve for pressure directly. Its primary unknown is
the **velocity potential** `phi`, a scalar field with units of `m^2/s`. Both
physical quantities of interest are recovered from it:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

Using a potential rather than the pressure has two practical advantages. The
problem stays scalar even though the particle velocity is a vector, and a
prescribed wall velocity - the way most acoustic sources are described - enters
as a natural boundary condition rather than as a constraint.

The sign convention above (`u = -grad(phi)`, `p = +rho * dphi/dt`) is used
consistently throughout the solver and the stored results.

### 1.2 Governing equation

Small-amplitude sound in a quiescent, homogeneous fluid obeys the damped wave
equation for the velocity potential:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `phi` | velocity potential | `m^2/s` | solved for |
| `c` | speed of sound | `m/s` | material property |
| `rho` | density of the fluid | `kg/m^3` | material property |
| `beta` | bulk (volumetric) damping factor | `1/s` | material property, default `0` |

The equation is linear. Consequently a single solve per time step, or per
frequency, is the final answer - no outer non-linear iteration is performed and
`hasConverged()` always reports success once the linear system has been solved.

**Speed of sound.** `c` may be given directly through the *Speed of sound*
material property. If it is absent or zero, it is derived from the bulk stiffness
and the density:

```
c = sqrt(E / rho)
```

where `E` is the *Modulus of elasticity* material property. For a gas, `E` is the
adiabatic bulk modulus (`gamma * p0`, about `1.42e5 Pa` for air at atmospheric
pressure), which together with `rho = 1.2 kg/m^3` gives the familiar
`c = 344 m/s`. Giving `c` directly is usually less error-prone.

Elements that carry no material of their own - a boundary surface bounding a
meshed volume, for example - inherit the average `c` and `rho` of the computable
domain, so that impedance and radiation terms evaluate correctly on them.

### 1.3 Finite element discretisation

Multiplying by a weight function `w`, integrating over the domain, applying the
divergence theorem and multiplying through by `c^2` gives the weak form

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

which after substituting the element shape functions `N` becomes the second-order
system

```
M * phi'' + C * phi' + K * phi = f
```

with

| Matrix | Definition | Comment |
|---|---|---|
| `M` | `INT( N_m * N_n )` | mass matrix |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | stiffness matrix |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | bulk damping plus boundary impedance |
| `f` | `c^2 * SURF( v_n * N_m )` | prescribed normal velocity source |

`M` and `K` are positive semi-definite; `C` is positive semi-definite as long as
the impedance ratio `zeta` is positive.

**Element measures.** Domain terms (`M`, `K`, the bulk part of `C`) are scaled by
the geometric measure of the entity they belong to:

| Entity | Measure | Where it is set |
|---|---|---|
| Volume | 1 | - |
| Surface | surface thickness | entity geometric properties |
| Line | line cross area | entity geometric properties |
| Point | point volume | entity geometric properties |

Boundary terms (the impedance part of `C`, and the source `f`) act over the plain
element measure - the surface area of a surface element, the length of a line
element, unit area for a point element. A surface that bounds a meshed volume
therefore contributes only its boundary terms, because its thickness is zero by
default.

Point elements carry no integration points. Their contributions are lumped
directly onto their single node, which is what makes point boundary conditions
work at the ends of one-dimensional ducts.

### 1.4 Boundary conditions

| Boundary condition | Type | Applies to | Component |
|---|---|---|---|
| Velocity potential | explicit (Dirichlet) | point, line, surface | Potential `[m^2/s]` |
| Velocity | natural (source) | point, line, surface | Velocity `[m/s]` |
| Absorbing boundary | natural (damping) | point, line, surface | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | natural (damping) | point, line, surface | Acoustic impedance `[Pa*s/m]` |

**Velocity.** The classic acoustic source: a surface that vibrates with a
prescribed normal velocity, such as a loudspeaker cone or a piston. A positive
value drives the fluid, i.e. it points *into* the acoustic domain. It enters the
right-hand side as `f = c^2 * SURF(v_n * N)`.

A node with no boundary condition at all is an ideal **rigid wall** - the natural
condition of the weak form is zero normal velocity. Nothing has to be assigned to
obtain a hard-walled cavity.

**Acoustic impedance.** Prescribes the ratio of pressure to outward normal
particle velocity on the boundary, `p = Z * u_n`. Internally it is expressed
through the dimensionless **specific impedance ratio**

```
zeta = Z / (rho * c)
```

and contributes the boundary damping matrix `(c/zeta) * SURF(N_m * N_n)`.
`Z = rho*c` (`zeta = 1`) is the characteristic impedance of the fluid and gives a
perfectly non-reflecting termination. For air at room conditions `rho*c` is close
to `415 Pa*s/m`, which is the value the boundary condition offers by default.

**Absorbing boundary.** The same term, but parameterised by the normal-incidence
**absorption coefficient** `alpha` in `[0, 1]`, which is what material data
sheets quote. It is converted to an impedance ratio through the plane-wave
reflection coefficient `R = (zeta - 1) / (zeta + 1)` and `alpha = 1 - |R|^2`:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Behaviour |
|---|---|---|
| `1.0` | `1` | fully absorbing, anechoic termination, no reflection |
| `0.5` | `5.83` | partially absorbing |
| `0.1` | `38.0` | weakly absorbing |
| `0.0` | infinite | rigid wall, no damping at all |

Use `alpha = 1` on any artificial cut through an unbounded domain to emulate free
radiation, and a measured `alpha` on a lined wall. If both an *Acoustic
impedance* and an *Absorbing boundary* condition are assigned to the same entity,
the impedance wins.

### 1.5 Initial conditions

| Initial condition | Sets | Comment |
|---|---|---|
| Velocity potential | `phi` at `t = 0` | rarely non-zero on its own |
| Velocity potential / s | `dphi/dt` at `t = 0` | this is what sets the initial **pressure**, `p = rho * dphi/dt` |

Both are applied on the first time step only. To start a transient run from a
pressurised region, assign *Velocity potential / s* `= p0 / rho` to that entity.
Initial conditions have no effect on a harmonic analysis.

### 1.6 Transient analysis

The second-order system is integrated with the **Newmark** scheme using
`gamma = 1/2` and

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

The clamp guarantees `beta_N >= 1/4`, i.e. at least the unconditionally stable
average-acceleration variant, and prevents a division by zero for the forward
approximation whose coefficient is `0`.

Each step solves

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

with the usual coefficients

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

followed by the state update

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

The system matrix is symmetric positive definite, so it is solved with the
**conjugate gradient** method and Jacobi preconditioning.

`phi'` and `phi''` are written to the result file as *Potential / s* and
*Potential / s^2*. They are the state of the time integrator, so they must be
present for a restart or for the next time step to continue correctly.

**A transient analysis requires an enabled time solver.** Without it the mass and
damping terms vanish and the problem degenerates into a meaningless Laplace
problem; the solver reports this as an error rather than producing nonsense.

**Choosing the time step.** The scheme is unconditionally stable, so `dt` is
governed by accuracy, not stability. A useful rule is to resolve the highest
frequency of interest `f_max` with at least 20 steps per period:

```
dt <= 1 / (20 * f_max)
```

**Choosing the mesh.** Resolve the shortest wavelength `lambda_min = c / f_max`
with at least 10 linear elements:

```
h <= c / (10 * f_max)
```

### 1.7 Harmonic analysis

For a steady-state response at a single angular frequency
`omega = 2*pi*f`, write `phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Then
`dphi/dt -> i*omega*PHI` and `d2phi/dt2 -> -omega^2*PHI`, and the system becomes
the complex **Helmholtz** problem

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

The matrix solvers of Range FEA are real, so the complex system is solved as an
equivalent real block system of twice the size. With `S = K - omega^2*M` and
`D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

This block matrix is non-symmetric and, above the static limit, indefinite, so it
is solved with **GMRES** and Jacobi preconditioning. Restarted GMRES stagnates on
indefinite systems whenever the Krylov space is much smaller than the system, so
the solver automatically widens the GMRES restart to the system size, bounded by
a fixed 256 MB budget for the two Krylov bases. The configured restart is only
ever increased, never reduced, and the change is reported in the solver log:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

A harmonic analysis sweeps a list of frequencies

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

Each frequency is an independent solve and produces its own result record, in the
same way that a transient analysis produces one record per time step. The time
solver plays no role; if it happens to be enabled it is skipped, with a warning,
and its settings are left untouched so that switching back to a transient
analysis does not lose them.

### 1.8 Derived results

| Result | Apply to | Transient | Harmonic |
|---|---|---|---|
| Potential `[m^2/s]` | node | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | node | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | node | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | node | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | node | `rho * dphi/dt`, instantaneous | `omega * rho * abs(PHI)`, amplitude |
| Acoustic phase `[Deg]` | node | - | `arg(P)` |
| Sound pressure level `[dB]` | node | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | element | `-grad(phi)` | amplitude of `-grad(PHI)`, per component |
| Acoustic intensity `[W/m^2]` | element | `p * u`, instantaneous | `0.5 * Re{P * conj(U)}`, time averaged |

The **sound pressure level** is computed against the *Reference pressure* of the
acoustic setup, `2e-5 Pa` by default - the threshold of hearing in air. In a
harmonic analysis the RMS value of the harmonic signal is used, which is the
amplitude divided by `sqrt(2)`; in a transient analysis the instantaneous
magnitude is used, so the reported level oscillates within a period.

The **acoustic intensity** in a harmonic analysis is the time-averaged active
intensity, i.e. the net acoustic power flux. It is identically zero in a pure
standing wave and equals `0.5 * |p| * |u|` in a plane travelling wave.

### 1.9 Transient or harmonic?

| | Transient | Harmonic |
|---|---|---|
| Solves | `M*phi'' + C*phi' + K*phi = f` in time | `(K - w^2*M + i*w*C)*PHI = F` per frequency |
| Answers | "what happens when ..." | "what is the steady response at ..." |
| Excitation | any time history, including a single pulse | one sinusoid at a time |
| Cost | one SPD solve per time step | one indefinite solve of double size per frequency |
| Records | one per time step | one per frequency |
| Phase information | implicit in the time history | explicit, as a result field |

**Use a transient analysis for**

- impulse response, echo and reverberation studies;
- transient events - a bang, a valve closure, a door slam;
- broadband excitation, where a single run followed by a Fourier transform of a
  monitoring point covers the whole spectrum at once;
- any case where the time history itself is the answer.

**Use a harmonic analysis for**

- frequency response functions and transmission loss curves;
- cavity and duct resonances, and the pressure distribution at a resonance;
- muffler, silencer and absorber performance over a frequency band;
- sound pressure level maps at a single tone;
- any case where only the steady state matters. A harmonic run is usually far
  cheaper than marching a transient long enough for the transients to die out.

A practical compromise is to locate resonances with a coarse harmonic sweep and
then refine only the interesting band.

---

## 2. Graphical user interface

Everything acoustic-related lives in the **Solver** dock (`Problem`,
`Boundary conditions`, `Initial conditions`, `Material`, `Results` tabs), the
**Problem** menu and the **Model** dock.

### 2.1 Selecting the acoustics problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`) opens the problem task flow
dialog. Add a task and pick **Acoustics** - "Customized wave equation for solving
acoustic problems".

Acoustics is mutually exclusive with **Wave - general** and with **Modal
analysis**. It can be combined with other physics, in which case the task flow
determines the execution order.

### 2.2 Problem tab

Once acoustics is selected, the `Problem` tab of the Solver dock shows two
group boxes.

**Time-solver** - shown because acoustics is a time-dependent problem type.

| Field | Meaning |
|---|---|
| Enable | switches transient time marching on and off |
| Approximation | time-march approximation; maps to the Newmark `beta_N` as described in 1.6 |
| Start time | first time value in seconds; ignored on a solver restart |
| End time | `start time + time-step size * number of time-steps`, read-only |
| Time-step size | `dt` in seconds |
| Number of time-steps | how many steps to compute |
| Output frequency | write a result file every N steps; `0` writes only the last step |

**Acoustic analysis setup** - the group box added for acoustics.

| Field | Meaning |
|---|---|
| Analysis type | *Transient* or *Harmonic* |
| First frequency `[Hz]` | first frequency of the harmonic sweep |
| Frequency step `[Hz]` | increment between two consecutive frequencies |
| Number of frequencies | how many frequencies to solve; each produces its own record |
| Sweep | read-only label showing the resulting frequency range |
| Reference pressure `[Pa]` | reference used for the sound pressure level, `2e-5 Pa` by default |

The three frequency fields are greyed out while *Transient* is selected. The
reference pressure applies to both analysis types.

### 2.3 Material tab

The `Material` tab lists the material assigned to the selected entity and the
material properties it holds. For acoustics the relevant properties are:

| Property | Units | Required | Comment |
|---|---|---|---|
| Density | `kg/m^3` | yes | `rho` |
| Speed of sound | `m/s` | one of the two | `c`, used directly when non-zero |
| Modulus of elasticity | `Pa` | one of the two | fallback, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | no | `beta`, bulk attenuation, `0` by default |

An entity is solved by the acoustic solver when it has a density **and** either a
speed of sound or a modulus of elasticity. Because the two are alternatives,
neither is reported as missing by the setup checker; a model that has neither is
rejected by the solver with an explicit message.

Every property is a table against temperature, so a temperature-dependent speed
of sound can be entered and will be interpolated if a heat transfer problem runs
alongside.

### 2.4 Boundary conditions tab

The tab is split into a manager list of available boundary conditions on top and
an editor for the selected one below. Select an entity in the `Model` tree first;
the manager then offers the boundary conditions valid for that entity type and
for the selected problem. Tick one to assign it, then edit its components in the
lower tree.

For acoustics the manager offers:

- **Velocity potential** - explicit, points, lines and surfaces. Component:
  *Potential* `[m^2/s]`.
- **Velocity** - points, lines and surfaces. Component: *Velocity* `[m/s]`.
- **Absorbing boundary** - points, lines and surfaces. Component: *Acoustic
  absorption coefficient*, dimensionless, default `1.0`.
- **Acoustic impedance** - points, lines and surfaces. Component: *Acoustic
  impedance* `[Pa*s/m]`, default `415`.

Every component is a table against time, so a source can be given a time history:
add rows to ramp a velocity up, or to fire a short pulse. In a harmonic analysis
the value at the current solver time is used, which for a single-valued table is
simply that value.

Leave a wall without any boundary condition to make it acoustically rigid.

### 2.5 Initial conditions tab

Same layout as the boundary conditions tab. For acoustics it offers **Velocity
potential** and **Velocity potential / s**. The latter is the one that sets the
initial pressure field.

### 2.6 Matrix solver setup

`Problem` -> `Setup Problem(s) matrix solver` configures the iterative solvers.
A transient acoustic analysis uses the **CG** entry, a harmonic analysis uses the
**GMRES** entry.

The default GMRES restart of `10` is far too small for the frequency-domain block
system. The solver raises it automatically, but the number of *outer* iterations
and the convergence value are taken as configured, so raise them here if a
harmonic run does not converge.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record. A monitoring point on *Acoustic pressure* is the
natural microphone: in a transient run it produces the pressure time history at
that location, which can be inspected through `Report` -> `Monitoring points`.

### 2.8 Results

After the run, the `Results` tab lists the computed variables and controls what
the 3D view displays. The acoustic variables are those of section 1.8.

Node variables (pressure, sound pressure level, phase, potential) are drawn as
smooth fields; element variables (particle velocity, intensity) are vectors and
are best shown through a vector field or a cut.

The `Records` tab of the `Model` dock lists the individual result records - one
per time step for a transient run, one per frequency for a harmonic sweep - and
lets you step through them or animate them.

`Report` -> `Solver log file` shows the full solver output, including the
statistics block printed for every acoustic variable after each record.

---

## 3. Tutorial - transient acoustic analysis

**Goal.** Compute the impulse response of a closed air cavity and read the
pressure history at a microphone position.

This tutorial assumes a meshed volume model. Use `File` -> `New model`, draw a
box with `Geometry` -> `Create element` or import a geometry, and tetrahedralise
it - the *Draw cube* tutorial covers this ground.

### Step 1 - select the problem

1. `Problem` -> `Problem(s) task flow` (`Ctrl+P`).
2. Add a task and select **Acoustics**.
3. Confirm with `OK`.

The `Problem` tab of the Solver dock now shows the *Time-solver* and *Acoustic
analysis setup* group boxes.

### Step 2 - assign the material

1. Select the volume entity in the `Model` tree.
2. Open the `Material` tab.
3. Assign a material, or create one, and give it:
   - **Density** = `1.2` `kg/m^3`
   - **Speed of sound** = `344` `m/s`
4. Leave **Acoustic damping factor** at `0` for an undamped cavity, or set it to
   a small value such as `1.0` `1/s` to make the response decay.

### Step 3 - choose the analysis type and the time step

1. Open the `Problem` tab.
2. In *Acoustic analysis setup* set **Analysis type** = `Transient`.
3. In *Time-solver*:
   - tick **Enable**;
   - **Start time** = `0`;
   - **Time-step size** = `2e-5` s. Following `dt <= 1/(20*f_max)` this resolves
     frequencies up to about `2.5 kHz`;
   - **Number of time-steps** = `500`, giving `10 ms` of simulated time;
   - **Output frequency** = `5`, writing 100 result records.

Check the mesh against `h <= c/(10*f_max)`: for `f_max = 2.5 kHz` and
`c = 344 m/s` the element size should not exceed about `14 mm`.

### Step 4 - excite the cavity

Two options, pick one.

**A - a moving wall.** Select the surface that represents the source, open the
`Boundary conditions` tab and tick **Velocity**. Enter a time history in the
component table, for example a short pulse:

| Time `[s]` | Velocity `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - an initially pressurised region.** Select the entity that should start
pressurised, open the `Initial conditions` tab and tick **Velocity potential /
s**. Enter `p0 / rho`; for `p0 = 1 Pa` and `rho = 1.2 kg/m^3` that is `0.833`.

Leave every other surface without a boundary condition so that the cavity is
hard-walled, or tick **Absorbing boundary** with an absorption coefficient
matching the wall lining to let energy leave.

### Step 5 - add a microphone

1. `Problem` -> `Define monitoring points`.
2. Add a point at the location of interest.
3. Select **Acoustic pressure** as its variable.

### Step 6 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. The setup checker reports missing materials or boundary conditions before the
   run starts.
3. Follow the progress in `Report` -> `Solver log file`. Each time step prints
   the CG convergence and the statistics of every acoustic variable.

### Step 7 - inspect the results

- `Model` dock, `Records` tab: step through or animate the time records.
- `Results` tab: display **Acoustic pressure** or **Sound pressure level** on the
  model, and **Acoustic particle velocity** as a vector field.
- `Report` -> `Monitoring points`: the pressure history at the microphone. Its
  decay rate gives the reverberation time; its Fourier transform gives the
  cavity resonances.

### Troubleshooting

| Symptom | Cause |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* | *Enable* is unticked in *Time-solver*, or the analysis type should be *Harmonic* |
| *"Acoustic solver requires a positive time step size"* | time-step size is zero |
| *"No element with a valid speed of sound was found"* | no entity has a density together with a speed of sound or a modulus of elasticity |
| Response never decays | no damping anywhere - expected for a perfectly rigid, lossless cavity; add an absorbing boundary or an acoustic damping factor |
| Wiggles ahead of the wave front | the mesh or the time step is too coarse for the frequency content; refine both |

---

## 4. Tutorial - harmonic acoustic analysis

**Goal.** Compute the frequency response of a duct terminated anechoically, and a
sound pressure level map at each frequency.

### Step 1 - select the problem

`Problem` -> `Problem(s) task flow` (`Ctrl+P`), add a task, select **Acoustics**.

### Step 2 - assign the material

As in the transient tutorial: **Density** `1.2 kg/m^3` and **Speed of sound**
`344 m/s` on the fluid entity.

Size the mesh for the **highest** frequency of the sweep:
`h <= c / (10 * f_max)`.

### Step 3 - configure the sweep

Open the `Problem` tab and in *Acoustic analysis setup* set:

| Field | Value |
|---|---|
| Analysis type | `Harmonic` |
| First frequency | `50` Hz |
| Frequency step | `50` Hz |
| Number of frequencies | `20` |
| Reference pressure | `2e-5` Pa |

The *Sweep* label confirms `50 - 1000 [Hz]`, i.e. 20 solves and 20 result
records.

The *Time-solver* group box may stay enabled - a harmonic analysis ignores it and
writes a warning to the log. Its settings are preserved.

### Step 4 - drive the duct

Select the inlet surface, open the `Boundary conditions` tab and tick
**Velocity**. Enter a single row with the piston velocity, for example
`0.01` `m/s`. The amplitude is interpreted as the amplitude of the harmonic
excitation at every frequency of the sweep.

### Step 5 - terminate the duct

Select the outlet surface and tick either:

- **Absorbing boundary** with **Acoustic absorption coefficient** = `1.0` for a
  perfectly non-reflecting termination; or
- **Acoustic impedance** with **Acoustic impedance** = `rho * c` = `413`
  `Pa*s/m`, which is the same thing expressed differently.

Use a smaller absorption coefficient to model a partially reflecting termination,
and leave the outlet without any condition to model a closed, rigid end.

The duct walls need no boundary condition - they are rigid by default.

### Step 6 - check the matrix solver

`Problem` -> `Setup Problem(s) matrix solver`, **GMRES** entry. The restart is
raised automatically, but if the log shows a stagnating residual, increase the
number of outer iterations and loosen or tighten the convergence value as needed.

### Step 7 - solve

`Solution` -> `Start solver` (`Ctrl+R`). The log shows one block per frequency:

```
Frequency:         1 of 20        | f =  5.000000e+01 [Hz]
{
    Increasing GMRES restart from 10 to ... for the frequency domain system.
    ...
    Acoustic pressure [Pa]
    Sound pressure level [dB]
    ...
}
```

### Step 8 - inspect the results

- `Model` dock, `Records` tab: one record per frequency. Stepping through them is
  the frequency sweep.
- `Results` tab: display **Sound pressure level** for a decibel map, **Acoustic
  pressure** for the amplitude, and **Acoustic phase** to see the wave fronts.
- **Acoustic intensity** shows where acoustic power flows. In a well-terminated
  duct it is uniform and directed towards the outlet; in a resonant cavity it is
  near zero.
- A monitoring point on **Acoustic pressure** records the amplitude at each
  frequency, i.e. the frequency response function at that location.

### Interpreting the result

For a straight duct driven at `v` and terminated anechoically, the analytic plane
wave solution is

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

For `rho = 1.2`, `c = 344`, `v = 0.01` that is `|p| = 4.13 Pa`, an SPL of
`103.3 dB` and an intensity of `0.0206 W/m^2`. Deviations from a flat `|p|` along
the duct mean the termination is reflecting - either intentionally, or because
the absorption coefficient is not `1`.

With a rigid termination the same duct becomes resonant. The pressure amplitude
peaks at the quarter-wave resonances `f = (2n+1) * c / (4L)` and the phase is
constant at `+/-90` degrees, with zero net intensity.

### Troubleshooting

| Symptom | Cause |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* | first frequency is zero and the frequency step is zero |
| Residual stagnates, results look noisy | GMRES has not converged; raise the outer iterations in the matrix solver setup, or coarsen the mesh |
| Amplitude blows up at one frequency | the sweep has hit an undamped resonance; add an absorbing boundary or an acoustic damping factor |
| Pressure varies strongly along a supposedly anechoic duct | the termination reflects; check the absorption coefficient or the impedance value |

---

## 5. Verification

The unit test `tst_solver_acoustic` (in `range-solver-lib/tests`, built with
`-DRANGE_BUILD_TESTS=ON`) checks the solver against closed-form solutions on a
one-dimensional duct of `1 m` discretised with 200 truss elements, using the
stock matrix solver settings.

**Harmonic, anechoic termination** - `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz`:

| Quantity | Analytic | Computed |
|---|---|---|
| Pressure amplitude | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` at every node |
| Potential amplitude | `v/k = 1.0823e-3 m^2/s` | within 1 % |
| Particle velocity | `0.01 m/s` | `0.009999 m/s` |
| Sound pressure level | `103.18 dB` | within `0.1 dB` |
| Intensity | `0.0204 W/m^2` | within 2 % |
| Phase gradient | `k * dx` | within 5 degrees |

**Harmonic, rigid termination** - the standing wave gives a phase of exactly
`+/-90` degrees and zero net intensity, and the peak amplitude exceeds the
travelling wave amplitude.

**Transient** - a driven, anechoically terminated duct stays bounded, the Newmark
state is published so that the integration continues correctly across steps, and
the excited field is non-zero and finite everywhere.

---

## 6. Limitations

- **No structural-acoustic coupling.** Acoustics cannot be combined with modal
  analysis, and there is no fluid-structure interface with `RSolverStress`. A
  vibrating structure has to be represented by a prescribed *Velocity* boundary
  condition.
- **Linear acoustics only.** Small-amplitude, quiescent medium. No mean flow, no
  convective effects, no non-linear propagation.
- **Real impedance only.** The absorbing and impedance boundary conditions take a
  real value. A complex, frequency-dependent surface impedance is not supported.
- **Frequency-independent material data.** The speed of sound and the damping
  factor are tabulated against temperature, not against frequency, so they are
  constant across a sweep.
- **Boundary terms use the plain element measure.** For a one- or
  two-dimensional model whose cross area or thickness is not `1`, the source and
  impedance terms are not scaled by it, while the domain terms are. Use a unit
  cross area or a full three-dimensional model to avoid the mismatch.
- **Harmonic excitation is real.** A prescribed velocity enters as the real part
  of the complex load; sources with a relative phase offset cannot be defined.
