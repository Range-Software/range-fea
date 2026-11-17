# Range Mesh Generator - Theory Manual and User Guide

This document describes the tetrahedral mesh generator of Range FEA,
`range-element-lib`: the geometry it accepts, the algorithms used to fill that
geometry with elements, the meaning of its quality controls, the graphical user
interface, and a worked meshing example.

The **Range** generator is the default choice in **Generate 3D mesh**. It is
separate from TetGen. Both produce tetrahedral volume meshes, but their recovery,
refinement and optimization algorithms differ, so the same settings do not imply
the same nodes, elements or running time.

The generator prepares the discretization on which a physics problem is solved.
It does not solve a field equation, assign materials or establish that a solution
is independent of its mesh. The corresponding problem manuals, such as
[Fluid flow](theory_fluid.md) and [Heat transfer](theory_heat.md), explain those
parts of an analysis.

**Contents**

1. [Theoretical background](#1-theoretical-background)
2. [Graphical user interface](#2-graphical-user-interface)
3. [Tutorial - mesh a unit cube](#3-tutorial---mesh-a-unit-cube)
4. [Checking a mesh](#4-checking-a-mesh)
5. [Limitations and troubleshooting](#5-limitations-and-troubleshooting)
6. [Library interface](#6-library-interface)

---

## 1. Theoretical background

### 1.1 Domain, input geometry and constraints

Let `Omega` be the three-dimensional region to be meshed. Its boundary is given
as flat triangles. Additional triangles can separate materials or subdomains
inside `Omega`, and prescribed edges can represent lines or geometric features
which must survive meshing.

This description is a **piecewise linear complex**: the geometry is made of
points, straight segments and planar pieces. A curved CAD surface is represented
by its surface triangles; the generator has no underlying CAD surface to which
it can project a new node.

| Input | Meaning |
|---|---|
| Nodes | Coordinates, an optional point marker and an optional positive mesh-size value |
| Triangles | The outside boundary and internal interfaces, with surface markers |
| Edges | Prescribed segments, with line markers |
| Existing tetrahedra | Optional source volumes and interfaces for reconstruction |

Markers are integer identifiers. They let the application connect generated
entities back to point, line, surface and volume groups. The library itself does
not interpret a marker as a material, a boundary condition or a physical unit.

A surface must be **closed and conforming**. Triangles may meet at a common
vertex or along a complete common edge. A crossing, a gap, or a vertex lying in
the interior of another triangle's edge without a matching split is not a
conforming boundary. Explicit line constraints on a surface also need compatible
surface connectivity. Consistent surface orientation is recommended.

There is a distinction between the library's input contract and the application
workflow. **Check surface integrity** in the application can attempt to split
intersecting elements before calling the generator. That preprocessing belongs
to `range-model-lib`; it is not an operation performed by `RTetraMesher` itself.

### 1.2 Tetrahedra and geometric measures

A linear tetrahedron has four corner positions `a`, `b`, `c` and `d`. Define

```
u = b - a
v = c - a
w = d - a

D = det[ u v w ]
V = D / 6
```

The generator orders output corners so that `D > 0`. A zero determinant means a
flat tetrahedron; a negative determinant means the corner ordering is inverted.
Numerical diagnostics also reject non-finite volumes.

The six edge lengths are `l_ij = |x_j - x_i|`. Two shape measures used by the
generator must not be confused:

| Measure | Definition | Regular tetrahedron |
|---|---|---|
| Radius/edge ratio `q_R` | Circumsphere radius `R` divided by shortest edge `l_min` | `sqrt(6)/4`, approximately `0.6123724` |
| Aspect ratio `q_A` | Longest edge `l_max` divided by smallest altitude `h_min` | `sqrt(3/2)`, approximately `1.224745` |
| Minimum dihedral angle | Smallest internal angle between two tetrahedron faces | approximately `70.52878 degrees` |

The **radius/edge ratio** is a refinement control. The **aspect ratio** is a
reported statistic. A radius/edge limit of `1.6` therefore does not mean an
aspect-ratio limit of `1.6`.

If `A_i` is the area of the face opposite corner `i`,

```
h_i   = 3 * V / A_i
h_min = min( h_i )
q_A   = l_max / h_min
```

The circumcenter `o = a + r` is found from

```
2 * [ u^T ] * r = [ u . u ]
    [ v^T ]       [ v . v ]
    [ w^T ]       [ w . w ]

R = |r|
q_R = R / l_min
```

For consistently oriented face normals `n_i` and `n_j`, the internal dihedral
angle can be evaluated as

```
theta_ij = atan2( |n_i x n_j|, -n_i . n_j )
```

with conversion to degrees. This form retains small angles more reliably than
subtracting nearly equal values in an inverse-cosine calculation.

A **sliver** can have edges of comparable lengths and still have almost zero
volume. Edge lengths or the radius/edge ratio alone are therefore insufficient
quality checks: inspect dihedral angles and the nearly-flat count as well.

### 1.3 Initial Delaunay tetrahedralization

`RDelaunayTetrahedralizer` starts from the input points. It selects a
non-degenerate initial tetrahedron and inserts the remaining points with an
incremental **Bowyer-Watson** construction:

1. Locate the new point in the current tetrahedralization.
2. Find the connected cavity of tetrahedra whose circumspheres contain it.
3. Remove those tetrahedra.
4. Join the new point to the triangular boundary of the cavity.
5. Update neighbor and vertex-location information.

A Delaunay tetrahedralization satisfies the empty-sphere condition: the open
circumsphere of a tetrahedron contains no other input point. This is a useful
starting arrangement, but it neither reproduces every prescribed surface
triangle nor guarantees a good minimum dihedral angle.

Points are ordered along a space-filling curve before insertion so that
successive locations tend to be close. Duplicate coordinates detected during
insertion are mapped to an existing active vertex. This is not a general
near-node welding or surface-repair operation. The direct output data can retain
unused duplicate input entries because input-node positions in the node list are
preserved.

The outside of the convex hull is represented by **ghost tetrahedra** with a
vertex at infinity. They provide an explicit exterior for point location and
later region classification; they are never exported as finite elements.

### 1.4 Robust predicates and geometric tolerance

Two predicates drive the topological decisions:

| Predicate | Decision |
|---|---|
| `orient3d(a,b,c,d)` | Which side of the plane through three points contains the fourth point |
| `inSphere(a,b,c,d,p)` | Whether `p` is inside, outside or on the circumsphere of a positively oriented tetrahedron |

`RGeometricPredicates` first evaluates a floating-point expression with an error
bound. An uncertain sign is evaluated again with floating-point expansions.
Exactly cospherical configurations are resolved by a deterministic symbolic
perturbation based on point ordering, rather than by physically moving the nodes.

These predicates protect sign decisions. Coordinates, circumcenters, smoothing
positions and most quality values are still floating-point quantities. Exact
predicates do not make all constructed geometry exact or make every constrained
input recoverable.

The dimensionless **coplanarity tolerance** has a separate role. Neighboring
surface triangles with the same marker can belong to one facet when their normals
satisfy the angular test

```
|n_1 x n_2| <= tolerance * |n_1| * |n_2|
```

and the triangles lie on opposite sides of their common edge. The mesh also uses
scaled orientation tolerances to avoid creating numerically flat cells near a
plane. The option is not an absolute distance for merging arbitrary nearby
surfaces or closing holes.

### 1.5 Boundary recovery and planar facets

The initial Delaunay mesh may omit prescribed edges or triangles. Boundary
recovery makes them part of the tetrahedral mesh.

The working boundary contains **subsegments** and **subfaces**. A subsegment is
an edge which must not be flipped away: an explicit line, a surface-group border,
a sharp feature, or another edge separating facets. A subface is a triangle of
the current boundary triangulation.

Neighboring coplanar triangles with the same marker, not separated by a
subsegment, form a **facet**. The facet's outline is prescribed; its internal
diagonals are free to change. Consequently, a planar surface can have a different
triangle arrangement after generation even though its shape and surface group
are preserved.

Missing subsegments are handled before missing subfaces. Recovery can:

- adopt a compatible diagonal or a validated planar-patch triangulation already
  present in the volume mesh;
- split a constrained segment;
- insert a node on a facet and replace its local surface triangulation;
- retry an unsuccessful circumcenter placement at a triangle centroid.

A planar-patch replacement is checked before it is committed, including its
boundary, constrained edges, vertex set, area, orientation and surface marker.
This avoids forcing one choice of planar diagonals through repeated subdivision.

An inserted point replaces a volume cavity whose boundary is visible from that
point. Recovered boundary subfaces block cavities unless those subfaces are being
explicitly replaced. This is how refinement avoids crossing an already recovered
wall or material interface.

Recovery has length and work safeguards. The current minimum splitting scale is
based on the model bounding-box diagonal `L_model`:

```
l_split = 1e-8 * L_model
```

A constrained feature can reach this limit through repeated recovery splits even
when the original surface has much larger triangles. A failure at that scale is
a recovery failure, not proof that the input triangles intersect.

### 1.6 Exterior removal and region classification

After recovery, triangles act as barriers in the tetrahedron adjacency graph.
Classification first floods from the ghost tetrahedra without crossing those
barriers. All finite cells reached by this flood are outside the domain and are
excluded from the exported mesh.

Each remaining connected set of tetrahedra becomes a **region**. This permits
separate solids and conforming internal material interfaces.

An enclosed inner shell is not automatically an empty cavity. It separates
another bounded region, which is also meshed. A through-hole connected to the
exterior is different: its empty space is reachable by the exterior flood.
`RTetraMeshData` has no explicit hole-seed input for removing an arbitrary
sealed region. Check the retained regions when modelling nested shells.

### 1.7 Refinement: size, shape and encroachment

With **Quality mesh** enabled, `RTetraMeshRefiner` places unacceptable cells in
a priority queue. Size violations are handled before shape violations.

A tetrahedron can be marked for refinement when any active criterion fails:

```
V > V_max                          maximum-volume criterion
R / l_min > q_max                  radius/edge criterion
theta_min,free < theta_requested    dihedral-angle criterion
R > 0.75 * h                       mesh-size-function criterion
```

`h` is the mean of the positive mesh-size values at the cell's corner nodes.
Sizes for new nodes are interpolated from existing mesh information. When both
maximum volume and a size function are active, both can mark a cell as too large.

The size function is a desired local scale, not a promise that every edge lies
between the GUI's minimum and maximum edge-length entries. Its actual cell test
uses the circumradius. For reference, a regular tetrahedron of edge length `a`
has

```
V = a^3 / (6 * sqrt(2))
R = sqrt(6) * a / 4

a = (6 * sqrt(2) * V)^(1/3)
```

Refinement proposes the circumcenter. If reaching or inserting it would encroach
on a constrained edge or face, the boundary feature is considered for splitting
instead. Insertions are also subject to minimum-spacing and feature-protection
checks. Local repair follows accepted insertions, and newly created bad cells
are put back on the queue.

A pair of prescribed faces fixes the dihedral angle between them. The generator
separates such **geometry-constrained** angles from free angles when checking the
requested minimum dihedral angle. Subdivision cannot remove an angle imposed by
the input surface without changing that surface.

Refinement is bounded by passes, retry limits, minimum lengths, spacing safeguards
and node budgets. It can return a valid mesh which still misses quality or size
targets. The warning and the final counts, rather than successful completion
alone, tell whether those targets were reached.

### 1.8 Bounded local optimization

`RTetraMeshOptimizer` performs an initial repair after classification. With
quality meshing enabled, it also performs finishing optimization after
refinement. Repairs are local operations, including face and edge flips,
relocation of eligible added interior vertices, and bounded patch or point-placement
trials for difficult cells.

The optimizer checks proposed replacements for positive volume, connectivity,
region consistency, prescribed faces and edges, and quality acceptance. Boundary
vertices are not smoothed off their surface. Original input vertices are not
eligible for the added-interior-vertex relocation operations.

The quality evaluation includes the mean ratio

```
q_mean = 12 * (3 * V)^(2/3) / sum( l_ij^2 over the six edges )
```

which is `1` for a regular tetrahedron and tends to `0` as the cell becomes flat.
It is considered together with radius/edge and dihedral measures, with
stage-specific acceptance rules.

There is no global optimum or termination-at-perfect-quality claim. Ordered
sweeps, patch visits and placement attempts have finite budgets. A message about
a reached queue budget describes a work limit; it is not the same as an invalid
element or a failed boundary recovery.

### 1.9 Linear and quadratic output

The geometric mesh is generated with four-node tetrahedra. For **Quadratic
(second order)** output, one node is then added at the midpoint of every unique
edge, shared by the elements incident to that edge:

| Entity | Linear | Quadratic |
|---|---|---|
| Edge | 2 corner nodes | 2 corners and 1 mid-edge node |
| Triangle | 3 corner nodes | 3 corners and 3 mid-edge nodes |
| Tetrahedron | 4 corner nodes | 4 corners and 6 mid-edge nodes |

For tetrahedron barycentric coordinates `L_0` to `L_3`, the standard quadratic
shape functions associated with those nodes are

```
N_i  = L_i * (2 * L_i - 1)     corner i
N_ij = 4 * L_i * L_j           edge i-j
```

Mid-edge positions remain on straight edges. Choosing second order does not
project a faceted boundary onto a curved CAD surface, improve the underlying
linear cell shapes, or perform another refinement pass. Quality statistics use
the corner-node geometry; the added mid-edge nodes are counted separately.

Quadratic output also does not establish that every physics solver supports
those element types. Check the relevant problem manual before choosing the
order for an analysis.

### 1.10 Reconstruction and transfer of results

**Reconstruct mesh** supplies existing tetrahedra as additional information about
regions and their interfaces. Faces separating different source-volume markers
are recovered even if the model has no explicit surface triangles there.

Reconstruction still starts with a new tetrahedralization of the input corner
nodes. It does not retain every old element, every old face, or the old element
numbers. Output regions are associated with source-volume markers by sampling
source tetrahedra. Inspect the region and material assignments after a complex
remesh.

At application level, second-order input is reduced to corner-node geometry
before generation. New midpoint nodes are created if second-order output is
requested. Direct library callers must manage the corresponding input conversion
themselves.

**Keep computed results** is handled by `RMeshResultsInterpolator` in
`range-model-lib`. Nodal variables are transferred using element interpolation;
element variables take values from located source elements of the appropriate
kind. Closest-element information can be used where the new position lies outside
the old mesh.

This transfer is not a new solution and is not a conservative projection of mass,
energy or forces. Check balances and solve again before accepting remeshed results.

---

## 2. Graphical user interface

Open `Geometry` -> `Volume` -> `Generate tetrahedral mesh` to display **Generate
3D mesh**. The shorter action reference is
[Generate tetrahedral mesh](../../actions/en/action_geometry-volume_generate_tetrahedra.md).

A material or problem type is not needed to generate the geometry. They are
needed later for the analysis which uses it.

### 2.1 Generator, order and geometry checks

| Control | Meaning for the Range generator |
|---|---|
| Mesh generator | Select **Range**; TetGen is an independent alternative |
| Element order | Linear or quadratic output; an existing volume mesh's order is preselected |
| Check surface integrity | Attempt intersection splitting before generation; enabled by default for a model without volumes |
| Reconstruct mesh (reuse current nodes) | Use existing tetrahedral regions and interfaces; available when volumes already exist |
| Keep computed results | Interpolate existing variables to the new mesh; available when variables exist |

The application performs meshing on a working copy. The generated model is put
back into the session after successful generation and consolidation. The
pre-generation model version is stored by the application's mesh job.

Surface-integrity processing does not replace inspection of the geometry. An
intersection-free surface can still have a gap, a hanging node, incompatible
interfaces, or features which recovery cannot resolve.

### 2.2 Quality controls

| Control | Meaning |
|---|---|
| Quality mesh | Enable size/quality refinement and final optimization; boundary recovery and initial repair still occur when it is off |
| Maximum element volume | A positive requested upper volume; `0` disables that individual criterion |
| Generate mesh size function | Build a spatially varying desired scale from an existing result variable |
| Source variable | The result whose spatial gradient drives the size field, including direction changes of vector results |
| Minimum edge length | Lower desired size-field value, assigned to high-variation locations |
| Maximum edge length | Upper desired size-field value, assigned to low-variation locations |

The size-field controls require an existing variable and are used only while
**Quality mesh** and **Generate mesh size function** are both enabled. The size
function is not generated simply by reopening the dialog: it is an explicit
option. Choose positive lengths with the minimum no greater than the maximum.
The size field follows **change per unit distance**, rather than the value or
magnitude of the selected result. For each element, the indicator is the largest
pairwise directional change

```
g_element = max( |u_j - u_i| / |x_j - x_i| )
```

where the numerator is the Euclidean norm of component differences for vector
results. A velocity field changing direction can therefore request refinement
even at constant speed. Element results are first interpolated to nodes.
Each node receives the largest indicator of its incident elements.

Zero gradients and gradients no greater than `0.001` times the strongest gradient
use **Maximum edge length**. Differences at floating-point roundoff scale are
ignored. Stronger gradients decrease the requested size toward **Minimum edge
length**; the dialog saturates at half the strongest gradient. The comparison is
relative within the selected field, not an absolute threshold in temperature,
pressure or another variable's units. A uniformly nonzero gradient requests a
uniform size. A constant field requests the largest size everywhere.

The size field does not remove existing nodes or disable other refinement
criteria. To avoid a small global volume limit forcing refinement in flat regions,
set **Maximum element volume** to `0`, or choose a suitably coarse limit, and set
**Maximum edge length** to the desired coarse scale. Element-quality constraints
and boundary recovery can still require local refinement.

The current application defaults include a radius/edge limit of `1.6`, a
maximum volume of `0.1 m^3`, linear elements, and quality meshing enabled. The
application passes a minimum dihedral-angle target of **10 degrees** to the Range
generator. Radius/edge ratio, dihedral target, tolerance and the library node
budget do not have separate controls in this dialog. Section 6 distinguishes
the direct library defaults from application settings.

**TetGen parameters** are disabled for Range. Switches such as `q`, `a` and `o2`
are TetGen's interface; they do not override the Range generator's options.

### 2.3 Units and model scale

The application labels volume in `m^3` and size-field lengths in `m`. Ensure the
imported coordinates use the intended scale before setting either value. A model
imported a thousand times too large has volumes a billion times too large.

The bare library assigns no unit system. If its coordinates use a length unit
`U`, mesh sizes use `U` and volume limits use `U^3`. Angles are in degrees and
shape ratios are dimensionless.

For approximately uniform three-dimensional refinement, halving the length scale
can require roughly eight times as many cells. Boundary detail, quality repairs
and constraints can change that estimate substantially.

---

## 3. Tutorial - mesh a unit cube

This example separates meshing checks from any physics solution. Use a
surface-only cube extending from `(0,0,0)` to `(1,1,1)` in metres, with six closed
faces. A triangulated version has eight corners and twelve triangles. Section
6.2 provides a complete library example for exactly that geometry.

The analytic checks are

```
Domain volume = 1 m^3
Exterior area = 6 m^2
```

### Step 1 - inspect the surface

Create or import the closed cube, confirm its dimensions, and check for open
edges, duplicate faces and inconsistent connectivity. A surface which only looks
closed in the viewer can still contain distinct nodes at the same apparent
position.

Save the model before changing its mesh. Keep surface groups if separate faces
will later receive different boundary conditions.

### Step 2 - choose the generator and a moderate size

Open **Generate 3D mesh** and set:

| Setting | Value |
|---|---|
| Mesh generator | Range |
| Element order | Linear (first order) |
| Check surface integrity | On |
| Reconstruct mesh | Off for this surface-only model |
| Keep computed results | Off; the example has no solution variables |
| Quality mesh | On |
| Maximum element volume | `0.01 m^3` |
| Generate mesh size function | Off |

Select **OK**. The generator recovers the surface, classifies the enclosed region,
refines the cells and optimizes the mesh.

### Step 3 - read the result before solving

Confirm that one volume was created and that no invalid elements were reported.
The sum of tetrahedron volumes should be `1 m^3`, and the exterior area should
remain `6 m^2`, to numerical precision.

If every element meets `V <= 0.01 m^3`, at least 100 tetrahedra are needed to
fill this cube. This is a lower bound, not a prediction of the exact cell count:
shape criteria can require more cells, and a warning can indicate an unmet size
target.

Inspect the quality ranges, histograms and warning counters. Do not substitute
"generation finished successfully" for checking those values.

### Step 4 - refine and compare

Repeat with `Maximum element volume = 0.00125 m^3`, preserving a copy of the first
mesh. This divides the volume target by eight and approximately halves the
regular-tetrahedron length scale.

Both meshes should represent the same cube and retain its surface groups. Their
node counts, element counts and internal diagonals need not have a fixed ratio.
When a physics problem is assigned, compare an appropriate result on the two
meshes as well; geometric validity alone does not establish solution convergence.

### Step 5 - compare quadratic output

Generate the first mesh again with **Quadratic (second order)** selected. With
otherwise identical generation settings, the final conversion adds shared
mid-edge nodes to the linear topology. It does not improve the corner-node
quality measures. Inspect the separate mid-edge count and check that the chosen
physics solver accepts quadratic elements before running it.

---

## 4. Checking a mesh

### 4.1 Geometry, topology and groups

Check the following before relying on a generated mesh:

- The expected regions exist; no enclosed region has been mistaken for a void.
- Internal interfaces separate the intended volume groups.
- Exterior surface area and total enclosed volume agree with the intended model.
- Surface, line and point markers still correspond to the intended application
  groups and conditions.
- The invalid-element count is zero and every exported tetrahedron has positive
  signed volume.

For a closed mesh, an exterior triangular face belongs to one tetrahedron. An
ordinary internal face belongs to two tetrahedra. With `T` tetrahedra and `F_ext`
exterior faces, the unique triangular-face count satisfies

```
F = (4 * T + F_ext) / 2
```

for a conforming manifold tetrahedral mesh. Internal material-interface triangles
are also shared by two tetrahedra; they are not exterior faces.

### 4.2 Statistics and histogram populations

Verbose generation prints Range-style aligned tables with the input and output
counts, quality minima/maxima, and histograms.

| Table or field | Interpretation |
|---|---|
| Input nodes and surface triangles | The data supplied to the generator, before added nodes |
| Input constrained edges | Explicit and geometry-derived segments after boundary initialization; not just the line elements originally drawn |
| Hole seeds | Zero because the input type has no explicit hole-seed list |
| Generated nodes | Corner nodes and, for quadratic output, the additional mid-edge nodes |
| Exterior boundary faces | Faces incident to only one exported tetrahedron |
| Faces on input facets | Recovered boundary and interface triangles |
| Added nodes on facets/segments/interior | Where Steiner nodes were inserted |
| Mid-edge nodes | Nodes added by the final quadratic conversion, separate from refinement |
| Minimum input angles | Angles in the original input triangles and between incident input facets, rather than output-cell angles |

The aspect-ratio histogram has **one sample per valid tetrahedron**. Face-angle
histograms have **twelve samples per valid tetrahedron**, three for each of its
four faces. Dihedral-angle histograms have **six samples per valid tetrahedron**.
An internal face contributes through both incident cells; the face-angle
histogram is not a histogram of unique boundary triangles.

The bins are compatible with TetGen's report:

- Aspect ratio: thresholds `1.5, 2, 2.5, 3, 4, 6, 10, 15, 25, 50, 100`, then the
  unbounded upper bin. An exact threshold is included in the lower bin.
- Face angles: ten-degree bins from `0` through `180 degrees`.
- Dihedral angles: `0-5`, `5-10`, ten-degree bins through `70-80`, `80-110`,
  ten-degree bins through `160-170`, then `170-175` and `175-180 degrees`.

Invalid cells are counted separately and excluded from the range/histogram
calculation. A low histogram count must not hide a nonzero invalid-element count.

### 4.3 Quality warnings

The final constraint summary is more specific than a single minimum angle:

| Counter | Meaning |
|---|---|
| Radius/edge | Cells exceeding the active radius/edge target |
| Dihedral | Cells with a below-target angle that is not between two prescribed faces |
| Volume/size | Cells failing the active volume or circumradius/size-function criterion |
| Nearly flat | Cells with `6*V/l_max^3 < 1e-12` |
| Invalid | Cells with invalid connectivity, non-positive volume or non-finite volume |
| Geometry-constrained angles | Cells with a below-target angle between prescribed faces |

The radius/edge `p95` is the empirical 95th percentile of cell radius/edge values.
It is not an aspect-ratio percentile. Inspect both the distribution and its
worst cells; a good percentile can coexist with a few very poor elements.

The sub-degree report distinguishes internal/free face pairs and prescribed face
pairs. A cell can appear in both categories, so those counts must not simply be
added as disjoint sets.

The application's **sliver-element warning** is another, separate check based on
node-distance ratios. It can identify surface as well as volume elements. It is
not identical to the generator's minimum-dihedral or nearly-flat test.

### 4.4 Progress, profiling and diagnostic export

Progress labels distinguish preparation, boundary recovery, refinement, repair,
optimization, quadratic conversion and diagnostics. Percentages describe the
current measured work list, not a percentage of total wall-clock time or a
remaining-time estimate. Adaptive work can grow when a split creates more cells.

The log also reports phase timings and work counters such as quality evaluations,
cache hits, vertex-star searches, patch visits and reached budgets. These help
identify whether a long run is spending time recovering geometry, adding nodes
or trying local quality repairs.

Direct library callers can set `qualityReportPath` to request a JSON diagnostic
report. It includes the quality summary and up to 50 worst cells, their neighbors,
constraints and repair-attempt information. This option has no separate control
in the **Generate 3D mesh** dialog.

### 4.5 Mesh convergence of a physics result

A geometrically valid mesh is necessary, but it does not guarantee an accurate
finite-element solution. Repeat an analysis on a finer mesh and compare the
quantity which matters: a displacement, heat flux, pressure drop, reaction force
or another relevant observable.

A result-driven size function uses differences in an existing variable to request
more detail. It is not a certified error estimator. Reinterpolating old results
onto a finer mesh without solving again does not perform a convergence study.

---

## 5. Limitations and troubleshooting

### 5.1 Geometry and numerical limitations

- **Piecewise planar geometry only.** Surface refinement follows the supplied
  triangulation. CAD curvature is not recovered from an STL file or from midpoint
  insertion.
- **Conforming input is required.** The library does not perform arbitrary CAD
  healing, Boolean unions, gap closure or intersection repair. Application-level
  preprocessing is a separate, bounded attempt to improve the input.
- **No explicit hole seeds.** Every bounded region left by exterior classification
  is meshed. Nested shells do not by themselves request an empty sealed cavity.
- **No universal quality guarantee.** Sharp angles, narrow features and protected
  constraints can prevent the requested radius/edge, angle or size targets from
  being attained.
- **Bounded work.** Node, pass, spacing and patch limits prevent indefinite repair;
  they can leave quality violations or terminate boundary recovery with an error.
- **Tetrahedral volume meshes.** The generator does not create hexahedral grids,
  prism boundary layers, weighted Delaunay meshes or a directional anisotropic
  size-field mesh.
- **Reconstruction changes connectivity.** It retains input corner-node geometry
  and uses old regions, but it is not a promise to preserve old element numbers or
  every old diagonal.
- **Result transfer requires verification.** Interpolation is not a conservative
  remapping or an equilibrium solve.

### 5.2 Interpreting common outcomes

| Outcome | Check or next action |
|---|---|
| Triangles do not enclose any volume | Inspect gaps, disconnected seams and missing faces; a visually closed shell can still be topologically open |
| A boundary edge or triangle cannot be split | Read the stated rejection reason, node identifiers, precise coordinates and minimum split scale; inspect the local feature before changing tolerances |
| No triangle intersections were found, but recovery fails | Check conformity, feature scale and numerical recovery limits; the intersection check is not a complete meshability test |
| Refinement safeguards rejected insertions | Inspect narrow features and constrained angles; forcing more insertions is not automatically a valid remedy |
| Queue or node budget reached | Inspect which targets remain unmet and whether more work or a geometry change is justified |
| Unexpected extra region inside a shell | Check the distinction between an enclosed material region and a desired void |
| More nodes after changing only element order | Check the mid-edge-node count; quadratic conversion adds interpolation nodes without another linear refinement pass |
| Remeshed results change after solving again | Compare interpolation error, discretization error and convergence of the physics solve separately |

Do not increase the coplanarity tolerance blindly to make a failing case pass.
It changes geometric classifications and can erase distinctions that the model
needs. Likewise, lowering the minimum splitting scale alone can prolong repeated
subdivision without recovering the missing constraint.

---

## 6. Library interface

### 6.1 Classes, defaults and option scope

`range-element-lib` depends on `range-base-lib` and Qt, and operates on plain mesh
data. It does not depend on `RModel` or the application's session. Conversion to
and from an application model belongs to `RModelTetraMesh` in `range-model-lib`.

| Class | Responsibility |
|---|---|
| `RTetraMesher` | Complete generation pipeline |
| `RTetraMeshData` | Nodes, edges, triangles, tetrahedra, markers and neighbors |
| `RTetraMeshOptions` | Generation, refinement and reporting controls |
| `RGeometricPredicates` | Robust orientation and in-sphere signs |
| `RDelaunayTetrahedralizer` | Initial point tetrahedralization |
| `RTetraMesh` | Point location, cavities, connectivity and local replacements |
| `RBoundaryRecovery` | Prescribed segments/facets and region classification |
| `RTetraMeshRefiner` | Size/shape refinement and insertion safeguards |
| `RTetraMeshOptimizer` | Bounded repair and quality improvement |
| `RTetraMeshQuality` | Quality evaluation and constraint diagnostics |
| `RTetraMeshStatistics` | Counts, ranges and histograms for the verbose report |

These are **direct library defaults**, not a copy of every application setting:

| `RTetraMeshOptions` setter | Default | Meaning |
|---|---|---|
| `setVerbose(bool)` | `false` | Detailed statistics and profiling output |
| `setReconstruct(bool)` | `false` | Use source tetrahedra for regions/interfaces |
| `setQualityMesh(bool)` | `true` | Refinement and final optimization |
| `setRadiusEdgeRatio(double)` | `2.0` | Maximum requested `R/l_min`; non-positive disables this refinement test |
| `setMinDihedralAngle(double)` | `0 degrees` | Explicit minimum-angle refinement target; positive values enable it |
| `setVolumeConstraint(double)` | `0` | Maximum-volume target; positive values enable it |
| `setUseSizeFunction(bool)` | `false` | Use positive sizes supplied at input nodes |
| `setTolerance(double)` | `1e-8` | Relative coplanarity tolerance |
| `setMaxSteinerNodes(unsigned int)` | `0` | Use the internal default budget, currently 5,000,000 added nodes |
| `setElementOrder(int)` | `1` | Linear output; `2` requests mid-edge conversion |
| `setQualityReportPath(string)` | empty | Optional JSON diagnostic file |

Node limits control work, not the exact final node count. A boundary split can
require more than one insertion before a refinement limit is checked again;
recovery and optimization also have their own bounded work policies. Quadratic
mid-edge nodes are added afterward and are not the Steiner refinement budget.
Initial boundary recovery has its own guard, currently the smaller of the node
budget and `100*N_input + 100000` added nodes.

### 6.2 Example - generate and check the unit cube

The following C++ example uses the same settings as the first tutorial mesh.
Headers and link dependencies are supplied by the `range-element-lib` CMake target.
A larger application should also handle `RError` when generation fails.

```cpp
#include <cmath>
#include <iostream>
#include <rel_tetra_mesher.h>

int main()
{
    RTetraMeshData input, output;
    const double nodes[8][3] = {
        {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0},
        {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}
    };
    const int triangles[12][3] = {
        {0,2,1}, {0,3,2}, {4,5,6}, {4,6,7},
        {0,1,5}, {0,5,4}, {1,2,6}, {1,6,5},
        {2,3,7}, {2,7,6}, {3,0,4}, {3,4,7}
    };
    for (const auto &node : nodes)
    {
        input.addNode(node[0], node[1], node[2]);
    }
    for (const auto &triangle : triangles)
    {
        input.addTriangle(triangle[0], triangle[1], triangle[2], 0);
    }

    RTetraMeshOptions options;
    options.setVerbose(true);
    options.setQualityMesh(true);
    options.setRadiusEdgeRatio(1.6);
    options.setMinDihedralAngle(10.0);
    options.setVolumeConstraint(0.01);
    options.setTolerance(1e-10);
    options.setElementOrder(1);
    RTetraMesher::generate(input, options, output);

    double totalVolume = 0.0;
    for (size_t i = 0; i < output.getTetrahedra().size(); i++)
    {
        const double volume = output.findTetrahedronVolume(int(i));
        if (!(volume > 0.0) || !std::isfinite(volume))
        {
            return 1;
        }
        totalVolume += volume;
    }
    std::cout << "Tetrahedra: " << output.getTetrahedra().size()
              << ", total volume: " << totalVolume << '\n';
    return std::fabs(totalVolume - 1.0) < 1e-10 ? 0 : 1;
}
```

The exported tetrahedra carry region markers and neighbor indices. A missing
neighbor is `RTetraMeshData::noTetrahedron`. Inspect the generated region markers
rather than relying on a particular cell order or an exact tetrahedron count.

For quadratic data, corner connectivity remains in `nodeIDs`; the additional
indices are in `midNodeIDs`, or `midNodeID` for an edge. Tetrahedron edge order is
`0-1, 1-2, 2-0, 0-3, 1-3, 2-3`; triangle edge order is `0-1, 1-2, 2-0`.
Mid-edge nodes follow the input and Steiner nodes in the output node list.

A direct caller reconstructing quadratic data must supply an appropriate linear
corner-node description first. The application wrapper performs this conversion;
passing every midpoint as a new independent corner is a different meshing input.
