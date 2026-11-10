# Material

**Material** is a collection of material properties relevant for given **Problem type**. Values of each material property can be temperature dependent.

## Assigning a material

1. Select one or more entities in the **Model tree**.
2. Open the **Material** tab on the right side of the main window.
3. Check the check-box in front of the material name to assign it to the selected entities.

![Material](image-Material.png)

Materials which contain all properties required by the selected **problem types** are listed in **bold** text, all other materials in _italic_ text.

_Note: All model entities must have material assigned._

## Material properties

Properties of the selected material are listed in the lower part of the **Material** tab. A value can be edited directly in the **Value** column. Use the **Add** and **Remove** buttons to add or remove a property.

To make a property temperature dependent, expand it and click **Edit temperature dependent values** to enter a table of values for given temperatures. A property with temperature dependent values is grayed out in the **Value** column and can be changed only in that table.

## Material database

The list of materials is a database which is shared by all models. Each material is stored in a file of its own, shown in the **File** column. Use the **New** button to create a new material and the **Delete** button to delete the selected one. Changes of material properties are stored in the database.
