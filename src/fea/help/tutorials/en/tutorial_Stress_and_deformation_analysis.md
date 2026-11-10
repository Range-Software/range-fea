# Stress and deformation analysis

This tutorial will demonstrate how to set up and solve a **Stress analysis** problem. For that reason a simple beam will be supported on both ends with a load in the middle.

## 1. Load model

**Menu:** _File -> Open model_

This will show an **Open model** dialog. Select file **Beam.rbm** and click **Open** to load the model.

Since there is no physical problem assigned to this model, a **Select problem** dialog window will appear soon after loading of the model is complete.

Click the **Stress analysis** tile and click **OK** to accept.

![Select problem - stress analysis](image-Select_problem-Stress.png)

## 2. Generate 3D mesh

To solve this problem, a volume mesh must be generated.

**Menu:** _Geometry -> Volume -> Generate tetrahedral mesh_

This action will show the **Generate 3D mesh** dialog.

![Generate mesh dialog](image-Generate_mesh_dialog.png)

For now, the prefilled values are sufficient. Click **OK** button to accept.

Once the mesh is generated, a new **Volume** entity appears in the **Model tree**. To see the generated mesh, hide the surface entities, show only **Volumes** and check **As wireframe** in **Display properties**.

![Volume mesh](image-Volume_mesh.png)

## 3. Assign material

Material can be assigned only to a selected model entity. Entities can be selected in the **Model tree**. To select multiple entities, hold down the _Ctrl_ key (_⌘ Cmd_ on macOS) while selecting entities, or hold down the _Shift_ key to select a range. Once all entities are selected, the material can be assigned in the **Material** tab on the right side of the main window.

**Model tree** and **material list** can be seen on screenshot below.

![GUI model and material](image-GUI_model_and_material.png)

Material is assigned when check-box in front of the material name is checked. In this tutorial **Iron** is used.

_Note: All model entities must have material assigned._

## 4. Assign boundary conditions

The boundary conditions are the specified values of the field variables (or related variables such as derivatives) on the boundaries of the field. Boundary conditions can be divided into **Explicit** and **Implicit** conditions. **Explicit** conditions are highlighted with **strong** font.

In this tutorial boundary conditions will be applied to **surface** entities as described below:

1. **Load**
    - _Weight_
        - Weight = 1000 `[kg]`
2. **Left** and **Right**
    - _Displacement_
        - Displacement in X direction = 0 `[m]`
        - Displacement in Y direction = 0 `[m]`
        - Displacement in Z direction = 0 `[m]`

To apply **Weight** boundary condition follow these steps:

1. Select **Load** entity in the **Model tree**.
2. Open the **Boundary conditions** tab, then select and check **Weight** boundary condition.
3. Replace **1** with **1000** `[kg]` in value field for **Weight** property.

![Assigned weight bc](image-Assigned_weight_bc.png)

To apply **Displacement** boundary condition follow these steps:

1. Select **Left** and **Right** entities in the **Model tree** (hold down _Ctrl_ / _⌘ Cmd_ to select both).
2. Select and check **Displacement** boundary condition in the **Boundary conditions** tab.
3. Verify that all property values are set to **0** `[m]`.

![Assigned displacement bc](image-Assigned_displacement_bc.png)

## 5. Assign environment conditions

For **Weight** boundary condition to work properly correct **Gravitational acceleration** environment condition must be assigned.

Environment conditions are not checked like boundary conditions. Open the **Environment conditions** tab and select **Gravitational acceleration** to verify its values: 0, 0 and -9.80665 `[m/s^2]` in X, Y and Z direction.

![Assigned gravity ec](image-Assigned_gravity_ec.png)

## 6. Solve problem

Once the problem is correctly configured it can be solved.

**Menu:** _Solution -> Start solver_

The model must be saved before the solver can start. The **Beam** model was opened from a file, so it is saved there automatically; a model that has never been saved shows a **Save model** dialog first (click **Save** to accept, and if a file with the same name already exists, confirm replacing it). Then the **Start solver** dialog will be shown.

![Start solver](image-Start_solver.png)

Click **OK** button to start the solver process.

Solver process is running in the background and all its output is displayed in the **Process output** tab below the 3D area, as it can be seen on the screenshot below. The run is finished when the output ends with **SOLVER - END**.
![Process output](image-Process_output.png)

## 7. Apply computed results

After the solver process is successfully completed computed results should be loaded automatically. To display these results they need to be applied on model entities. This is done in the **Results** tab. Results are applied to the selected model entities. For this purpose return to the **Model** tab below the 3D area, hide all surface entities, show only **Volumes** and select **Volume 1** in the **Model tree**.

![Selected volume entity](image-Selected_volume_entity.png)

### 7.1. Displacement

From drop-down menu select **Displacement** (if not already selected). **Displacement** is a vector variable; it can be shown as a **Color map** (entity will be colored by its magnitude) and/or as a **Deformed shape** (entity will bend). This time select the **Deformed shape** check-box and set **Deformation and arrow scale** to **500** to see the deformation.

![Applied displacement](image-Applied_displacement.png)

### 7.2. Von Mises stress

Similarly as **Displacement** apply **Von Mises stress** on selected **Volume** model entity: choose **Von Mises stress - Scalar** in the drop-down menu and check **Color map**. The deformed shape set up for **Displacement** stays visible. Additionally select **Show legend** check-box to display value range in 3D area.

**Von Mises stress** is computed on elements, so each element is drawn with one color. Check **Smooth display** to average the element values at nodes for a continuous picture; this changes only the display, not the results.

![Applied stress](image-Applied_stress.png)

## 8. Produce report

Sometimes it is necessary to produce a report to publish computed results.

**Menu:** _Report -> Create report_

![Create report](image-Create_report.png)

Software can produce reports in multiple formats (HTML, Markdown, ODF and PDF), all of them are checked by default. This time it is enough to produce only **PDF** document, so uncheck all formats except **Portable Document Format(PDF)** and click **Ok** button.

Once the report is generated, a message **Documents have been generated.** will be shown. Click **OK** to close it.

![Report generated](image-Create_report-done.png)

Generated documents are not displayed automatically. To view them open the **Documents** tab on the left side of the main window, where they are listed under the model name (**Beam**): the report under **Documents** and the image of the 3D area used in the report under **Images**. Double click on the document you would like to view.

![Document tree](image-Document_tree.png)

