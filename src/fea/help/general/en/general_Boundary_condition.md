# Boundary condition

**Boundary condition** is a condition specified at the boundary, where boundary can be any model entity (Point, Line, Surface, Volume).

Boundary conditions are:
- **Problem** specific - Each **Problem type** defines its own set of **boundary conditions**.
- **Entity** specific - Not every boundary condition can be applied to every entity type (e.g., **Pressure** can be applied only to a **Surface**).
- **Explicit** - Exact solution is prescribed at the boundary (e.g., **Temperature** for **Heat transfer**). Bold text in **Boundary conditions** list.
- **Implicit** - Implies a solution at the boundary (e.g., **Heat** for **Heat transfer**).

## Assigning a boundary condition

1. Select one or more entities in the **Model tree**.
2. Open the **Boundary conditions** tab on the right side of the main window. The list contains only boundary conditions which are valid for the selected **problem types** and the selected entities.
3. Check the check-box in front of the boundary condition name to assign it. Uncheck it to remove the boundary condition from the selected entities.
4. Select the boundary condition to edit its property values in the list below.

![Boundary conditions](image-Boundary_conditions.png)

Some properties have a check-box of their own. For example each component of the **Displacement** boundary condition can be unchecked to leave the displacement in that direction unconstrained.

Boundary conditions which act in a direction given by the geometry (**Normal displacement**, **Roller displacement**) derive their local direction from the entity. Check **Use entered local direction** to enter the direction by hand.

To display assigned boundary conditions in the 3D area, check **Boundary conditions** in **Display properties**.

## Time dependent values

Value of each property can be time dependent. Expand the property and click **Edit time dependent values** to enter a table of values for given times. Time dependent values are used when the problem is **transient** (time-dependent), that is, when **Time-solver** is enabled in the **Problem** tab.
