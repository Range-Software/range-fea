# Material

Un **material** es una colección de propiedades de material relevantes para un **tipo de problema** dado. Los valores de cada propiedad pueden depender de la temperatura.

## Asignación de un material

1. Seleccione una o varias entidades en el **árbol del modelo**.
2. Abra la pestaña **Material** en el lado derecho de la ventana principal.
3. Marque la casilla situada delante del nombre del material para asignarlo a las entidades seleccionadas.

![Material](image-Material.png)

Los materiales que contienen todas las propiedades requeridas por los **tipos de problema** seleccionados aparecen en **negrita**; todos los demás, en _cursiva_.

_Nota: todas las entidades del modelo deben tener un material asignado._

## Propiedades del material

Las propiedades del material seleccionado aparecen en la parte inferior de la pestaña **Material**. Un valor se puede editar directamente en la columna **Valor**. Use los botones **Añadir** y **Eliminar** para añadir o eliminar una propiedad.

Para hacer que una propiedad dependa de la temperatura, expándala y haga clic en **Editar valores dependientes de la temperatura** para introducir una tabla de valores para determinadas temperaturas. Una propiedad con valores dependientes de la temperatura aparece atenuada en la columna **Valor** y solo se puede cambiar en esa tabla.

## Base de datos de materiales

La lista de materiales es una base de datos compartida por todos los modelos. Cada material se guarda en un archivo propio, indicado en la columna **Archivo**. Use el botón **Nuevo** para crear un material y el botón **Eliminar** para eliminar el seleccionado. Los cambios en las propiedades de un material se guardan en la base de datos.
