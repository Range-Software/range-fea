# Reconstruct surface

This action retriangulates the selected surfaces. It merges coplanar elements, removes nodes which do not define the shape (nodes inside planar regions and on straight edges) and removes tiny and sliver elements.

Existing nodes are never moved, new nodes are created only by the quality mesh option. Nodes and edges shared with other entities (points, lines, surfaces which are not selected) are preserved, closed surfaces stay closed and no new self-intersections are created. Only linear triangles are processed and the volume mesh has to be removed first.

* **Angle tolerance** - maximum angle between normals of coplanar elements and between directions of collinear edges.
* **Distance tolerance** - maximum distance of a node from a plane (line) to be considered coplanar (collinear).
* **Minimum edge length** - edges shorter than this value are collapsed. Features smaller than this value may be removed.
* **Sliver angle** - elements with the smallest angle below this value are considered slivers.
* **Delaunay triangulation** - finish with Delaunay triangulation of the surface: edges whose opposite angles sum to more than 180 degrees are flipped, provided the shape is preserved within the distance tolerance.
* **Quality mesh (insert nodes)** - insert new nodes (Delaunay refinement) so that elements have the smallest angle of at least the **Quality angle** and edges are not longer than the **Maximum edge length** (zero means no limit). New nodes are placed on the existing surface and on straight feature edges, so the shape is preserved. Edges shared with unselected entities are not split. Curved surfaces made of individually tilted facets can be improved only partially.

Default tolerances are relative to the size of the selected surfaces. Larger tolerances result in more simplified surfaces.
