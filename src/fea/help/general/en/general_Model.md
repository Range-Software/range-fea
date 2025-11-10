# Model

**Model** is a virtual geometric representation of three dimensional object. Model consist of geometric **Entities**.


**Entity** is a collection of elements (geometric simplexes) of similar type and are divided into these types:

- **Point** - collection of 0D elements (node).
- **Line** - collection of 1D elements (segment).
- **Surface** - collection of 2D elements (triangle, quadrilateral).
- **Volume** - collection of 3D elements (tetrahedron, hexahedron).

Regardless of whether elements are 0D, 1D, 2D or 3D they all are represented in 3D space. For example point (node) has no dimensions. It only has a position, whereas line has one dimension (length).


Each **entity** except **Volume** can have **geometric** properties assigned to complete 3D representation. These are:

- **Point** - volume in cubic meters.
- **Line** - cross-sectional area in meters square.
- **Surface** - thickness in meters.
