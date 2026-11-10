# Initial condition

**Initial condition** is a condition which can be specified at any model entity (Point, Line, Surface, Volume) and can be seen as a starting point for the solver. For **Steady-state problems**, a well chosen **Initial condition** improves solver convergence. For **transient** (time-dependent) problems it defines the state at the start time.

Initial conditions are **Problem** specific where each **Problem type** defines its own set of **initial conditions** (e.g., **Temperature** for **Heat transfer**, **Velocity** and **Pressure** for **Incompressible viscous flow** or **Displacement** for **Stress analysis**).

## Assigning an initial condition

1. Select one or more entities in the **Model tree**.
2. Open the **Initial conditions** tab on the right side of the main window.
3. Check the check-box in front of the initial condition name to assign it. Uncheck it to remove the initial condition from the selected entities.
4. Select the initial condition to edit its property values in the list below.

![Initial conditions](image-Initial_conditions.png)

Initial conditions are optional. If no **Initial condition** for a given model entity is specified then the **Environment condition** is used. On entities with an **explicit Boundary condition** the value prescribed by the boundary condition is used.
