# Condición inicial

Una **condición inicial** es una condición que se puede especificar en cualquier entidad del modelo (Punto, Línea, Superficie, Volumen) y puede verse como un punto de partida para el solver. En los **problemas estacionarios**, una **condición inicial** bien elegida mejora la convergencia del solver. En los problemas **transitorios** (dependientes del tiempo) define el estado en el tiempo inicial.

Las condiciones iniciales son específicas del **problema**: cada **tipo de problema** define su propio conjunto de **condiciones iniciales** (p. ej., **Temperature** para **Heat transfer**, **Velocity** y **Pressure** para **Incompressible viscous flow**, o **Displacement** para **Stress analysis**).

## Asignación de una condición inicial

1. Seleccione una o varias entidades en el **árbol del modelo**.
2. Abra la pestaña **Condiciones iniciales** en el lado derecho de la ventana principal.
3. Marque la casilla situada delante del nombre de la condición inicial para asignarla. Desmárquela para quitar la condición inicial de las entidades seleccionadas.
4. Seleccione la condición inicial para editar los valores de sus propiedades en la lista inferior.

![Condiciones iniciales](image-Initial_conditions.png)

Las condiciones iniciales son opcionales. Si no se especifica ninguna **condición inicial** para una entidad del modelo, se utiliza la **condición de entorno**. En las entidades con una **condición de contorno explícita** se utiliza el valor prescrito por la condición de contorno.
