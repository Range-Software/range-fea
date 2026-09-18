# Magneto-statics - Theory Manual and User Guide

This document describes the magnetostatics solver (`RSolverMagnetostatics`) of
Range FEA, which serves the **Magneto-statics** problem type: the law it
evaluates, the inputs it accepts, the parts of the graphical user interface that
drive it, a worked tutorial and what its result is worth.

> **Status.** The solver evaluates the magnetic field of the current computed by
> the electro-statics task directly, with the **Biot-Savart law**. There is no
> system of equations and no boundary condition: the field is determined by the
> current alone, and the condition that it vanishes far from the conductor is
> built into the law. For the current density it is given, the result is exact
> up to a controlled quadrature error well below `1e-4` of the peak field. The
> solver works in **free space** - magnetic materials cannot be represented - and
> it computes the field of the modelled current only, so the current path should
> be closed within the model. Section 5 lists the limitations.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - the field of a current-carrying conductor](#3-tutorial---the-field-of-a-current-carrying-conductor)
4. [Checking a model](#4-checking-a-model)
5. [Limitations](#5-limitations)

---

## 1. Theoretical background

### 1.1 The result - the magnetic field

The result is the **magnetic flux density** `B` at every node of the model, in
tesla, as a vector of three components.

It is **evaluated, not solved for**. Once the current density is known, the
magnetic field in free space follows from it by an integral, so the solver
assembles no matrix and runs no iteration. Every node of the model receives a
value, whether it lies in the conductor, on its surface or anywhere else in the
mesh.

### 1.2 Governing law

In free space the static magnetic field satisfies Ampere's law and has no
sources:

```
curl(B) = u0 * J
div(B)  = 0
```

together with the condition that `B` vanishes far from all currents. The unique
field satisfying all three is given by the **Biot-Savart law**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Symbol | Meaning | Units | Source |
|---|---|---|---|
| `B` | magnetic flux density | `T` | computed |
| `J` | current density | `A/m^2` | from the electro-statics task, see 1.4 |
| `u0` | vacuum permeability | `H/m` | built in, `1.25663706212e-6` |
| `x` | field point - a node of the model | `m` | |
| `y` | source point - inside a conducting element | `m` | |

Three properties follow from the law itself rather than from anything the solver
does:

- **`div(B) = 0` holds exactly**, everywhere, whatever the mesh.
- **The far field is right without a boundary.** `B` falls off with distance as
  the law prescribes; there is no outer surface, no air region to mesh and no
  condition to set there.
- **`curl(B) = u0 * J` holds wherever the current is closed**, that is where
  `div(J) = 0` and the current path forms loops. Section 1.4 explains what that
  means for an electro-static model, whose current enters and leaves the body.

The permeability of the material does not enter anywhere. There is no relative
permeability material property, no magnetisation, and no distinction between `B`
and `H` - the solver works in free space whatever the model is made of. Section
1.9 explains why this cannot be lifted within the same approach.

**No material property is required.** The magnetostatics problem type declares
none at all, and the setup checker has nothing to check on its behalf. The
electro-statics task the problem type depends on has its own three requirements -
see the electro-statics manual.

### 1.3 Discretisation

The electro-statics solver produces a current density that is **constant in each
element**. The integral of 1.2 therefore splits into one integral per element,
each with a constant `J_e` that can be taken outside:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` depends only on the geometry of the element and on the field point. Every
element type the solver accepts is flat-sided, and for those `G_e` has a closed
form, so the solver integrates each element **exactly** rather than
approximately:

- **Tetrahedron.** The integrand is the gradient of `1/|x - y|` with respect to
  `y`, so the volume integral becomes a sum over the four faces,
  `G = SUM_f n_f * INT_f 1/R dA`. The integral of `1/R` over a flat triangle is
  known in closed form: a logarithm along each edge and the solid angle the
  triangle subtends from `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Triangle** of a surface entity with a thickness `t`. The current is a sheet
  current `K = J*t` in `A/m`. The tangential part of `G` is again a sum of edge
  logarithms, `SUM_e m_e * I_e`, and the normal part is the signed solid angle.
- **Quadrilateral.** Split into two triangles.
- **Line segment** of a line entity with a cross area `A`. The current is
  `I = A * (J . t)` along the segment, and the field of a straight finite wire is
  classic:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  with `a` and `b` the vectors from the field point to the two ends.

Here `I_e` is the integral of `1/R` along edge `e`, `P_e` the in-plane distance of
the field point from that edge, `m_e` the outward in-plane normal of the edge,
`d` the height of the field point above the plane of the face and `omega` the
solid angle of the face seen from the field point.

**Distant elements** do not need the closed form. The integrand is smooth there,
and a quadrature rule is as good as exact at a fraction of the cost:

| Distance of the field point from the element centre | Rule |
|---|---|
| below 2 x the longest element edge | closed form |
| 2 to 6 x | degree-two rule - 3 points on a triangle, 4 on a tetrahedron |
| beyond 6 x | midpoint rule - the element as a point current |

Line segments always use the closed form, which is as cheap as a quadrature
point. The combined error of the rules is measured by the unit tests against the
closed form summed over every element of a whole conductor, and stays below
`1e-4` of the peak field at every node - inside the conductor, on its surface and
beyond.

**The field at the conductor itself.**

- For a **volume current** the field is continuous and finite everywhere,
  including at the nodes, edges and faces of the conducting elements. A node on
  the surface of a solid conductor gets the true surface field.
- A **line** or **surface** entity stands for a conductor of zero section, and
  the field of such a conductor is singular: it grows as `1/r` towards a wire and
  logarithmically towards the edge of a sheet, and it jumps by `u0 * K x n` across
  a sheet. At a node on such a conductor the solver returns a finite value: a
  segment contributes nothing to a point on its own line, a sheet contributes the
  mean of the fields on its two sides, and the edge logarithm of a sheet is left
  out at a node lying on that edge. These values are regularised rather than
  physical. Read the field of a thin conductor a few section sizes away from it,
  where the thin model is valid.

### 1.4 The current density source

The magnetic field is driven by one input only: the **current density** left
behind by an *Electro-statics* task earlier in the problem task flow. There is
nothing to enter and nothing to assign.

The solver reads the **element** current density the electro-statics task stores
and uses it as it is - there is no averaging onto nodes. The current carrying
elements are:

| Entity | Condition | Source |
|---|---|---|
| Volume | tetrahedra | `J` |
| Surface | thickness `> 0` | sheet current `J * thickness` |
| Line | cross area `> 0` | current `J * cross area` |

Elements whose current density is zero are skipped, which is every element the
electro-statics task did not compute. So are elements whose current density is
below `1e-10` of the largest in the model: a poorly conducting region - air with
a conductivity of `5e-15 S/m` next to copper - would otherwise add a source per
element and nothing measurable to the field. If no element carries a current - the
electro-statics task has not run, the potentials are equal, or no material has an
electrical conductivity - the log prints a warning and the field is zero
everywhere.

**The current has to close.** The Biot-Savart law gives the field of the current
it is given and nothing else. A physical current flows in closed loops, but an
electro-static model usually holds only part of the loop: a bar between two
electrodes, with the current entering through one end and leaving through the
other. The supply leads and the rest of the circuit are not in the model, and
neither is their field. What the solver computes is then the field of the modelled
conductor as if the current appeared at one end and vanished at the other:

- **near the conductor** this is usually an excellent approximation, since the
  field there is dominated by the nearby current;
- **far from it**, at distances comparable to the conductor itself, the missing
  part of the loop matters as much as the modelled one, and the result is not the
  field of any real circuit.

When the far field matters, include the return path in the electro-static model
so that the current forms a loop. A thin return conductor can be represented by a
line entity with a cross area.

### 1.5 Boundary, initial and environment conditions

There are **none**, and none are needed.

| Condition kind | Offered for magneto-statics |
|---|---|
| Boundary conditions | none |
| Initial conditions | none |
| Environment conditions | none |
| Material properties | none |

The field is fixed entirely by the current and by the requirement that it vanishes
far away, which the Biot-Savart law carries with it. There is no outer boundary
to close and nothing to prescribe on it.

When an electrostatics task and a magnetostatics task share a task flow - which
they always do, see 1.6 - the condition tabs still offer the **electro-static**
conditions, because the tabs are filled from the problem types of the whole flow.
*Electric potential* and *Charge density* belong to the electrostatics task, and
they reach the magnetic field through the current density they produce.

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

### 1.7 Evaluating the field

Every current carrying element contributes to every node, so the work grows as
the **number of nodes times the number of current carrying elements**. The
evaluation runs in parallel over the nodes, which are processed in blocks so
that each element is fetched from memory once per block rather than once per
node.

As an indication, a bar of `120 000` tetrahedra evaluated at its `24 000` nodes
takes about five seconds on 14 threads. Doubling the mesh in every direction
multiplies the number of nodes and of elements by eight each, and the time by
about sixty-four; a model of a few hundred thousand elements takes minutes rather
than seconds.

Only elements carrying current count. A large mesh around a small conductor -
an air region, see 2.8 - adds nodes but no sources.

The matrix solver setup is not used.

### 1.8 Results

| Result | Apply to | Meaning |
|---|---|---|
| Magnetic field `[T]` | node | the field vector |

That is the whole output. `process()` does nothing, so there is no derived
quantity - no field magnitude stored separately, no flux, no energy density. The
3D view can show the magnitude and the components of the vector variable in the
usual way, and `Magnetic field in X direction` and the `Y` and `Z` equivalents are
available for component display.

The magnetic field is stored as a **node** variable, unlike the electrostatic
field results, which are element variables. **Every node** of the model has a
value, including nodes of entities that carry no current, so a mesh around the
conductor shows the field in the space around it.

After the record is written the solver log prints the statistics of the magnetic
field and the values at any monitoring points.

### 1.9 What the solver is, and what it is not

**It is an integral, not a field equation.** Evaluating Biot-Savart needs no
boundary and no mesh outside the conductor, and it is exact for the current it is
given. The price is that it only works in a **uniform** medium. With magnetic
materials the field of a current depends on where the iron is, and no integral of
the current alone can express that; that problem is solved with a potential
formulation - usually a magnetic vector potential on edge elements - which is a
different solver, not an extension of this one.

**It is driven by a computed conduction current.** There is no way to state a
current directly, no permanent magnet and no imposed field; the current has to be
produced by an electrostatics solve on a conducting body, and its accuracy sets
the accuracy of the magnetic result. The electro-statics manual describes what
that current density is and is not. A current density that is constant per
element is a first-order approximation of the real one, and it converges as the
mesh is refined.

**It computes the field of the modelled current.** See 1.4 - the current should
close within the model when the field far from the conductor matters.

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
contribution to the magnetic field.

So the material still matters, entirely through the electrostatic half of the
chain. Assign a material with all three, such as the bundled *Copper*, and see
the electro-statics manual for what each is used for.

There is no relative permeability property to assign. The solver works in free
space.

### 2.4 Boundary conditions tab

Nothing in this tab belongs to the magnetostatic task. The conditions it offers -
*Electric potential* and *Charge density* - are the electro-static ones, and they
drive the current density which in turn produces the magnetic field. Assign them
as the electro-statics manual describes.

No magnetostatic boundary condition exists or is needed - see 1.5.

### 2.5 Initial and environment conditions

Neither tab offers anything for magneto-statics. The *Electric potential* initial
condition and the *Temperature* environment condition shown belong to the
electro-statics task.

### 2.6 Matrix solver setup

The magnetostatics solver solves no system of equations and does not use the
matrix solver setup. The **CG** entry configures the electrostatics solver only.

### 2.7 Monitoring points

`Problem` -> `Define monitoring points` places probes at given coordinates and
selects the variable to record. *Magnetic field* is available in the list, along
with the electro-static variables, and reads the nodal field at the probe.

### 2.8 Results and records

The `Results` tab lists **Magnetic field** as a node vector, alongside the
electro-static results the first task produced. The `Records` tab of the `Model`
dock holds exactly one record: both problem types are static.

`Report` -> `Solver log file` shows the full output: the electrostatics
statistics first, then the number of current carrying elements and nodes the
Biot-Savart integral was evaluated for, then the magnetic field statistics.

**Seeing the field around a conductor.** The field is computed at every node, so
anything meshed around the conductor shows it. Mesh the surrounding space as a
separate volume entity - an air region - and give it a material without an
electrical conductivity: the electro-statics task skips it, so it carries no
current, but its nodes receive the magnetic field. The region costs nodes but no
sources (1.7) and needs no particular size or boundary, since nothing is solved
on it.

The bundled *Air* material does carry an electrical conductivity, a tiny one, so
the electro-statics task would include the region in its own solve. Use a copy of
it with the electrical conductivity property removed. The setup check then warns
that the entity's material is missing required properties; for the air region
that is expected, and the run goes ahead.

---

## 3. Tutorial - the field of a current-carrying conductor

**Goal.** Run the electro-statics and magneto-statics chain end to end on a
copper bar carrying a current, check the magnetic field against a hand
calculation, and see where the approximations of 1.4 show.

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
which needs them, and without a current density there is no magnetic field.

### Step 3 - drive a current

1. Select the surface entity at one end, open the `Boundary conditions` tab and
   tick **Electric potential**. Set it to `0` V.
2. Select the surface entity at the other end, tick **Electric potential** and
   set it to `0.01` V.

The current through the bar is

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

A bar with no potential difference carries no current and gives a magnetic field
of exactly zero - which is the first thing to check if the result comes out
empty.

### Step 4 - solve

1. `Solution` -> `Start solver` (`Ctrl+R`).
2. Follow the progress in `Report` -> `Solver log file`.

The log shows the electrostatics task with its variables, then the
magnetostatics task: the number of current carrying elements, the number of
nodes, and the magnetic field statistics. Both run once.

### Step 5 - inspect the result

- `Results` tab: display **Magnetic field**. It is a node vector, so the 3D view
  offers its magnitude and its three components.
- The field **circulates around the bar**, perpendicular to the current, in the
  sense of the right-hand rule.
- It is **zero on the axis** of the bar and grows towards the surface, where it
  is largest - at the middle of each long face rather than at the corners.
- It is **weaker near the ends** of the bar than in the middle. That is the
  open current path of 1.4: near an end, half of what would be a longer
  conductor is missing.

### Step 6 - check the magnitude

For a round conductor of radius `R` the surface field is `u0*I/(2*pi*R)`. A
square bar of side `a` is not round, but the middle of a face at distance `a/2`
from the axis is close to it: with `a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

The computed field at the middle of a long face, halfway along the bar, is about
`0.020 T`, some 15 % below that - the square section puts much of the current
further from the point than a round one would. At a corner, `a/sqrt(2)` from the
axis, it is lower again, about `0.019 T`.

A sharper check needs a field point away from the bar, where the section no
longer matters. Add an air region as described in 2.8 and read the field at
`r = 0.02 m` from the axis, halfway along. For a straight finite conductor of
length `L`,

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

and the computed value should match to about `0.1 %`, the difference being the
square section of the bar. This value is the field of the bar alone,
as 1.4 explains - a real circuit would add the field of its leads.

### Troubleshooting

| Symptom | Cause |
|---|---|
| The magnetic field is zero everywhere and the log warns that no element carries a current | no current density - check that the electro-statics task ran first, that the potentials differ, and that the material carries an electrical conductivity |
| **Magneto-statics** cannot be unticked on its own | expected - it requires *Electro-statics*, and unticking that unticks both |
| The *Time-solver* group box is missing | expected - neither problem type has a time solver, see 1.10 |
| The field is weaker at the ends of a conductor | expected - the current path is open, see 1.4 |
| The field far from the conductor does not match the circuit | the leads and the rest of the circuit are not in the model, see 1.4 |
| The field at a line or thin surface conductor is erratic | expected - the field of a conductor of zero section is singular there, see 1.3 |
| The run takes long | the cost is nodes times current carrying elements, see 1.7 |

---

## 4. Checking a model

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

**Finite conductor.** For a straight conductor of length `L`, at distance `r`
from its axis and halfway along,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

which tends to the long conductor formula as `L` grows. Away from the conductor
the shape of the section stops mattering, so any section can be checked against
this.

**Wire.** A line entity with a cross area carries its current along the segments,
and the solver uses the closed form of a straight wire for each of them. A
straight line entity therefore reproduces the finite conductor formula exactly,
to rounding, at any distance.

**Solenoid.** For a long solenoid of `n` turns per metre carrying `I`, the field
inside is uniform and along the axis,

```
B = u0 * n * I
```

and close to zero outside. A winding can be modelled as a line entity following
the turns, with the cross area of the wire, and the electro-statics task drives
the current through it from potentials at its two ends.

**Divergence.** For any magnetic field `div(B) = 0` everywhere. The Biot-Savart
law satisfies it exactly, so it does not test this solver; it would test only the
interpolation between the nodes.

**Convergence with the mesh.** The integration itself is exact (1.3); what
depends on the mesh is the current density, which the electro-statics task
computes as constant per element. Refining the mesh in the conductor and
comparing the field at the same point is the check of that part.

---

## 5. Limitations

### Modelling limitations

- **Free space only.** No permeability enters the formulation. There is no
  relative permeability material property, no distinction between `B` and `H`,
  no magnetisation and no saturation, so iron, ferrites and shielding cannot be
  represented at all. See 1.9 for why this needs a different solver.
- **The field of the modelled current only.** The field of the supply leads and
  of the rest of the circuit is not included. When the current path in the model
  is open, the field far from the conductor is not that of a real circuit - see
  1.4.
- **No permanent magnets and no imposed field.** The only source is a conduction
  current computed by an electro-statics task. A magnet, a background field and
  an external excitation have no representation.
- **No current can be stated directly.** A current is produced by an
  electrostatic solve on a conducting body. A coil can be modelled as a line
  entity following the winding, but its current is still set through potentials.
- **Thin conductors are singular.** The field at a node on a line entity or on
  the edge of a surface entity is a regularised value, not a physical one - see
  1.3.
- **Element types.** Tetrahedra, triangles, quadrilaterals and two-node line
  segments carry current; other element types contribute nothing.
- **Cost.** The work grows as the number of nodes times the number of current
  carrying elements - see 1.7.
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
