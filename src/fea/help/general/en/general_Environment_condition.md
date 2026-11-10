# Environment condition

**Environment condition** is a condition which can be specified at any model entity such as Point, Line, Surface or Volume. **Environment condition** specifies **problem** specific values for given environment (e.g., **Temperature** of the environment or **Gravitational acceleration**).

Available environment conditions depend on the selected **problem types**:

- **Gravitational acceleration** - components in X, Y and Z direction, by default 0, 0 and -9.80665 `[m/s^2]`.
- **Temperature** - by default 293.15 `[K]`.

## Editing an environment condition

Unlike **Boundary conditions**, environment conditions are not checked. They are always present and have default values.

1. Select one or more entities in the **Model tree**.
2. Open the **Environment conditions** tab on the right side of the main window.
3. Select the environment condition and edit its property values in the list below.

![Environment conditions](image-Environment_conditions.png)

If no **Initial condition** for a given model entity is specified then **environment condition** is used.
