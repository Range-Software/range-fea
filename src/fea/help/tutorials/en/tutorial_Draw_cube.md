# Draw cube

This tutorial will show how to draw/generate a simple geometry such as cube.

## 1. Create a new model

**Menu:** _File -> New model_

![New model](image-New_model.png)

Enter **Cube** as the model name (the description is optional) and click **OK**. A new empty model named **Cube** will be created. Since the newly created model has no problem assigned, a **Select problem** dialog will pop up.

### 1.1 Assign problem type

To assign a problem type click the **Heat transfer** tile and then click the **OK** button.

## 2. Draw a cube

**Menu:** _Geometry -> Draw -> Draw hexahedron_

Once the menu item is activated, a **Draw object** panel will be shown on the right side of the main window where the user can change object position, size, and the number of divisions in X, Y and Z direction. The default object is a cube with 1 `[m]` sides placed at the origin.

![Draw hexahedron](image-Draw_hexahedron.png)

Displayed object changes at the same time as values are being changed in the **Draw object** panel. Leave **Merge near nodes** checked and click the **OK** button to add the object to the model. A **Draw objects(s)?** confirmation dialog listing the selected model(s) the object will be added to will pop up.

![Add prepared objects](image-Draw_hexahedron-confirm.png)

Click **Yes** to confirm and the cube will be added to the **Cube** model as one **Surface** entity.

## 3. Save model

To save model just execute following action.

**Menu:** _File -> Save model_

If the model has not been saved before, the program will show a **Save model** dialog offering a file name (**Cube.rbm**) and a folder where to store it. Click **Save** to accept.
