# Condición de contorno

Una **condición de contorno** es una condición especificada en el contorno, donde el contorno puede ser cualquier entidad del modelo (Punto, Línea, Superficie, Volumen).

Las condiciones de contorno son:
- **Específicas del problema** – Cada **tipo de problema** define su propio conjunto de **condiciones de contorno**.
- **Específicas de la entidad** – No todas las condiciones de contorno se pueden aplicar a todos los tipos de entidad (p. ej., **Pressure** solo se puede aplicar a una **Superficie**).
- **Explícitas** – Se prescribe la solución exacta en el contorno (p. ej., **Temperature** para **Heat transfer**). Se muestran en negrita en la lista **Condiciones de contorno**.
- **Implícitas** – Implican una solución en el contorno (p. ej., **Heat** para **Heat transfer**).

## Asignación de una condición de contorno

1. Seleccione una o varias entidades en el **árbol del modelo**.
2. Abra la pestaña **Condiciones de contorno** en el lado derecho de la ventana principal. La lista contiene solo las condiciones de contorno válidas para los **tipos de problema** seleccionados y para las entidades seleccionadas.
3. Marque la casilla situada delante del nombre de la condición de contorno para asignarla. Desmárquela para quitar la condición de contorno de las entidades seleccionadas.
4. Seleccione la condición de contorno para editar los valores de sus propiedades en la lista inferior.

![Condiciones de contorno](image-Boundary_conditions.png)

Algunas propiedades tienen su propia casilla. Por ejemplo, cada componente de la condición de contorno **Displacement** se puede desmarcar para dejar libre el desplazamiento en esa dirección.

Las condiciones de contorno que actúan en una dirección dada por la geometría (**Normal displacement**, **Roller displacement**) obtienen su dirección local de la entidad. Marque **Usar la dirección local introducida** para introducir la dirección a mano.

Para mostrar las condiciones de contorno asignadas en el área 3D, marque **Condiciones de contorno** en las **Propiedades de visualización**.

## Valores dependientes del tiempo

El valor de cada propiedad puede depender del tiempo. Expanda la propiedad y haga clic en **Editar valores dependientes del tiempo** para introducir una tabla de valores para determinados instantes. Los valores dependientes del tiempo se utilizan cuando el problema es **transitorio** (dependiente del tiempo), es decir, cuando el **Solver temporal** está activado en la pestaña **Problema**.
