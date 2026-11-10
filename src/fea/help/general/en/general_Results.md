# Results

**Results** are produced by the **Solver** and consist of **Result variables**.

## Results variable

**Results variable** is a set of values computed by the **Solver**. **Results variable** is defined by its **type** (e.g., **Temperature** or **Displacement**) and a vector of values. Based on **type**, **Results variable** can contain a vector of scalar values or vector values.

For example, **Temperature** results variable will contain vector of temperatures for each node whereas **Displacement** will contain vector of displacement vectors for each node.

All results variables of the selected model are listed in the drop-down menu at the top of the **Results** tab, together with their kind (**Scalar** or **Vector**).

![Results variables](image-Results_variables.png)

**Results variable** can be computed either on node or on element level. For example, **Displacement** is computed on node level, whereas **Von Mises stress** is computed on element level. Where the selected variable is stored (**On nodes** or **On elements**) is shown below the drop-down menu.

Element level variables are displayed with one value per element. To get a continuous display:

- Stress variables offer a **Smooth display** check-box. It averages element results at nodes for display only, results themselves stay unchanged. Averaging lowers peak values.
- Other element level variables offer a **Display on nodes** button, which changes the variable from elements to nodes.

## Displaying results

Results are displayed on the entities which are selected in the **Model tree**. Results of a problem solved on **Volumes** are best seen when surface entities are hidden.

![Results](image-Results.png)

- **Show on selected entities** - check **Color map** to color the selected entities by the variable value. Vector variables can also be shown as a **Deformed shape**.
- **Color range** - **Max** and **Min** values of the color map. They can be typed in or changed in **Fine adjust**. **Reset to data** sets the range back to the range computed from the results.
- **Colors** - color scheme of the color map.
- **Show legend** - displays the value range in the 3D area.
- **Deformation and arrow scale** - multiplies the deformation of deformed shapes and the length of vector field arrows (vector variables only).
- **Graph...** - shows distribution of values and allows to set the color range on a graph.
- **Remove** - removes the variable from the model.

Results can be also displayed on entities created for that purpose: **Cuts**, **ISOs**, **Stream lines**, **Scalar fields** and **Vector fields**.
