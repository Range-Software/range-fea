# Multi physics

This tutorial shows how to solve several coupled problems on one model. The **Beam** model from the **Stress and deformation analysis** tutorial is used again. This time an electric current flows through the beam, the current heats the beam up (Joule heating) and the heat makes the beam expand and deform.

Three problem types are needed, and they are solved in this order:

1. **Electro-statics** - computes the electric potential, the electric field, the current density and the Joule heat produced by the current.
2. **Heat transfer** - computes the temperature distribution caused by the Joule heat.
3. **Stress analysis** - computes the thermal expansion (displacements) and the resulting stress.

Each problem uses the results of the previous one, so the order of the problems matters.

## 1. Load model

**Menu:** _File -> Open model_

In the **Open model** dialog select **Beam.tmsh** and click **Open**.

Since no problem type is assigned to the model yet, the **Problem task flow** dialog opens automatically once loading is complete.

## 2. Problem task flow

1. In the **Problem task flow** dialog click **Add problem type**.
2. In the **Problem type selector** check **Electro-statics**, **Heat transfer** and **Stress analysis** and click **OK**.

![Problem type selector - Multi](image-Problem_type_selector-Multi.png)

The **Problem task flow** should now list the three problems in one iteration group, in the order shown below. If the order is different, or there are extra entries, select an entry and use the **Up**, **Down** and **Remove** buttons to fix it. Click **OK** to accept.

![Problem task flow - Multi](image-Problem_task_flow-Multi.png)

The dialog can be opened again at any time with the **Problem(s) task flow** button in the toolbar.

## 3. Generate 3D mesh

All three problems are solved on a volume mesh.

**Menu:** _Geometry -> Volume -> Generate tetrahedral mesh_

The prefilled values in the **Generate 3D mesh** dialog are sufficient. Click **OK**. When meshing is finished, a new entity **Volume 1** appears under **Volumes** in the **Model** tree.

## 4. Assign material

All model entities must have a material assigned. The material must provide the properties needed by all three problems; **Iron** does.

1. In the **Model** tree click **Surface** and then _Shift_-click **Volume 1** to select all entities.
2. Open the **Material** tab on the right side of the main window.
3. Check **Iron**.

## 5. Assign boundary conditions

Because several problem types are selected, the **Boundary conditions** tab now offers the conditions of all of them, and one entity can have several conditions assigned - one for each problem. Assign the following conditions to the **surface** entities:

1. **Surface** (the outer surface of the beam)
    - _Simple convection_
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293.15 `[K]`
2. **Load** (the face in the middle of the beam)
    - _Electric potential_
        - Electric potential = 0.4 `[V]`
    - _Simple convection_ (values as for **Surface**)
3. **Left** and **Right** (the faces at both ends of the beam)
    - _Displacement_
        - Displacement in X, Y and Z direction = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0.4 `[V]`
    - _Simple convection_ (values as for **Surface**)

The current therefore flows from the middle of the beam to both ends, and all faces lose heat to the surrounding air by convection.

For each entity: select it in the **Model** tree (hold down _Ctrl_ / _⌘ Cmd_ to select both **Left** and **Right**), check the condition in the **Boundary conditions** tab, select the condition to show its properties below the list, then double-click the value, type the new one and press _Enter_. The default values of **Simple convection** (100 `[W/(m^2*K)]` and 293.15 `[K]`) and of **Displacement** (0 `[m]`) already match, so only **Electric potential** has to be changed.

## 6. Check environment conditions

Environment conditions apply to the whole model and are not checked like boundary conditions. Open the **Environment conditions** tab and select each condition to verify its values:

- _Gravitational acceleration_
    - 0, 0 and -9.80665 `[m/s^2]` in X, Y and Z direction
- _Temperature_
    - Temperature = 293.15 `[K]`

## 7. Solve problem

**Menu:** _Solution -> Start solver_

The model must be saved before the solver can start. If it has not been saved yet, a **Save model** dialog is shown first; click **Save** to accept (if a file with the same name already exists, confirm replacing it). Then the **Start solver** dialog is shown.

The dialog lists a few warnings, such as **Heat transfer: No explicit boundary condition assigned.** They are expected for this setup and can be ignored. Click **OK** to start the solver.

The solver runs in the background and its output is shown in the **Process output** tab. The problems are solved one after another in the order of the task flow; the run is finished when the output ends with **SOLVER - END**.

## 8. Create a vector field

A **vector field** is a display entity that draws a vector variable as arrows. Here it is used to show the electric field inside the beam.

**Menu:** _Geometry -> Vector field -> Create vector field_

In the **Vector field editor** dialog select **Volume 1** in the **Model** list and **Electric field** in the **Variables** list, and leave **3D vector field** checked.

![Create vector field](image-Create_vector_field.png)

Click **OK** to create the vector field. It is added to the **Model** tree under **Vector fields**, and computed results can be applied to it like to any other model entity.

## 9. Apply results

Results are applied in the **Results** tab to the entity selected in the **Model** tree, in the same way as in the **Stress and deformation analysis** tutorial.

### 9.1. Temperature and deformation of the volume

1. In the **Model** tree check **Volumes** (and leave the surfaces unchecked), then select **Volume 1**.
2. In the **Results** tab choose **Temperature - Scalar** in the drop-down list, check **Scalar** and **Display value range**. If the shown range does not match the results, click **Set default min/max values**.
3. Choose **Displacement - Vector [3]** in the drop-down list, check **Displacement** and set **Scale** to **20** so that the deformation is visible (the computed displacements are only about 1.3 mm).

The temperature ranges from about 505 K to 620 K; the beam is hottest in the middle of its two outer sections, and it bends upwards because the fixed ends do not let it expand freely.

![Applied temperature on volume](image-Applied_temperature_on_volume.png)

### 9.2. Temperature on the vector field

1. In the **Model** tree uncheck **Volumes**, check **Vector fields** and select **Electric field**.
2. In the **Results** tab choose **Temperature - Scalar**, check **Scalar** and **Display value range**. The arrows are now colored by the temperature at their position.
3. In **Display properties** below the tree check **Draw arrow heads** to show the direction of the electric field.

![Applied temperature on vector field](image-Applied_temperature_on_vector_field.png)
