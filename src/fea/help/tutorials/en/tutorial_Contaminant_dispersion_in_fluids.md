# Contaminant dispersion in fluids

This tutorial will demonstrate how to set up an advanced multi-physics simulation including a non-linear iterative problem such as **CFD (Computational Fluid Dynamics)**.

To solve **contaminant dispersion in fluid**, the following problem types need to be configured:

1. **Contaminant dispersion** - Calculate distribution of contaminant in the flow field.
2. **Incompressible viscous flow** - Steady-state and transient flow of Newtonian fluids.

Since **CFD** is a nonlinear problem it requires iterative solution. This problem will be solved in two steps:

1. **Steady-state** - First it is necessary to get an "initial" flow field and pressure distribution.
2. **Transient** - In the second step, **time marching** will be used to get a transient solution.

## 1. Load model

Load model **Channel.tmsh**.

## 2. Problem task flow (Step 1)

First a converged initial flow is needed. For this reason steady-state solution of the incompressible viscous flow is needed. In **Problem task flow** dialog select appropriate problem type and set **# of iterations:** to **2000**. To do so double click on the initial value.

![CFD - Problem task flow steady](image-CFD-Problem_task_flow_steady.png)

## 3. Generate 3D mesh

To solve this problem, a volume mesh must be generated.

**Menu:** _Geometry -> Volume -> Generate tetrahedral mesh_

## 4. Assign material

Assign **Water** to all model entities.

## 5. Boundary conditions

Assign following boundary conditions to **surface** entities as described below.

1. **Walls**
    - _Wall_
        - N/A
2. **Inflow**
    - _Volumetric flow rate (inflow)_
        - Volumetric flow rate = 50 `[m^3/sN/A]`
3. **Outflow**
    - _Pressure (implicit)_
        - Pressure = 0 `[Pa]`

## 6. Solve problem

Do the same as in the previous tutorials.

It will take some time until the solver computes all iterations and finds a converged solution. Solver convergence can be checked using the following action:

**Menu:** _Report -> Solver convergence_

## 7. Problem task flow (Step 2)

Once the solver converges, a **transient** problem including **Contaminant dispersion** can be configured.

Since **Incompressible viscous flow** is a nonlinear problem, it will always need some nonlinear iterations to be specified to find a converged solution for each time-step. **Task flow** should look like as shown in the screenshot below.

![CFD - Problem task flow](image-CFD-Problem_task_flow.png)

## 8. Time solver setup

Click on **Problem setup** tab. Enable **Time-solver** and specify values as shown on the screenshot below.

![CFD - Problem time solver](image-CFD-Problem_time_solver.png)

## 9. Boundary conditions

Apply **Particle concentration** boundary condition to **Inflow** model entity.

![CFD - Boundary conditions](image-CFD-Boundary_conditions.png)

Do not specify value but click on **Edit time dependent values** button, to specify time-triggered (time-profile) boundary condition.

In the **Component editor** dialog, time-dependent values can be specified. Values are always valid **from** the specified time.

![CFD - Particle concentration condition](image-CFD-Particle_concentration_condition.png)

## 10. Solve problem (restart)

Once the problem is fully configured, restart the solver. This is done just like when starting a solver, but the **Restart solver / continue** check-box must be selected. This will cause the solver to use already computed results as a starting point and continue in the time-marching simulation.

![CFD - Solver restart](image-CFD-Solver_restart.png)

## 11. Model records (results in time)

As the solver keeps finding solutions for each time-step, model records are being written. Each record contains a solution for a given time-step. A list of these records can be seen in the **Model records** tree. By double-clicking on a record, results for the given time-step will be loaded.

## 12. Record video

To record a video from computed results go to **Model records** tree and click on the **Record** button (Red point at the bottom).

![CFD - Model records](image-CFD-Model_records.png)

After clicking on **Record** button **Video settings** dialog will appear. Click **Ok** to start video recording process.

![CFD - Video settings](image-CFD-Video_settings.png)

Creating a video can take some time since video frames are created from screenshots of the **3D model area** and each model record must be loaded.

All produced screenshots and video itself can be found in **Documents** tree. To view the video double-click on its name **Channel.avi**.

![CFD - Documents](image-CFD-Documents.png)

