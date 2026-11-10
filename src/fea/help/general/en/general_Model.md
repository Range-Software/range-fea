# Model

**Model** is a virtual geometric representation of a three dimensional object. Model consists of geometric **Entities**.

![Model](image-Model.png)

## Entities

**Entity** is a collection of elements (geometric simplexes) of similar type and is divided into these types:

- **Point** - collection of 0D elements (node).
- **Line** - collection of 1D elements (segment).
- **Surface** - collection of 2D elements (triangle, quadrilateral).
- **Volume** - collection of 3D elements (tetrahedron).

Regardless of whether elements are 0D, 1D, 2D or 3D they all are represented in 3D space. For example point (node) has no dimensions. It only has a position, whereas line has one dimension (length).

Apart from these geometric entities, a model can contain entities created from computed **Results**: **Cuts**, **ISOs**, **Stream lines**, **Scalar fields** and **Vector fields**.

## Model tree

All entities of a model are listed in the **Model tree** (the **Model** tab on the left side of the main window), grouped by their type. The **#** column shows the number of entities in a group or the number of elements in an entity.

![Model tree](image-Model_tree.png)

- The check-box in front of a name shows or hides the model, group or entity in the 3D area.
- Clicking a row selects it. To select multiple entities, hold down the _Ctrl_ key (_⌘ Cmd_ on macOS) while selecting, or hold down the _Shift_ key to select a range. Selecting a model or a group selects all entities it contains.
- **Boundary conditions**, **Initial conditions**, **Environment conditions**, **Material** and **Results** are always assigned to the entities which are selected in the **Model tree**.

**Display properties** below the tree control how the selected entities are drawn (e.g., **As wireframe**, **Element edges**, **Element nodes**, **Boundary conditions**) and their color.

## Geometric properties

Each **entity** except **Volume** can have a **geometric** property assigned to complete its 3D representation. It is shown in the **Geometric properties** list below the **Model tree** for the selected entities:

- **Point volume** - volume of a **Point** in cubic meters.
- **Line cross area** - cross-sectional area of a **Line** in square meters.
- **Surface thickness** - thickness of a **Surface** in meters.
