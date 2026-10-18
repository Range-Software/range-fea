# Multifísica

Este tutorial muestra cómo resolver varios problemas acoplados sobre un mismo modelo. Se reutiliza el modelo **Beam** del tutorial **Análisis de tensiones y deformaciones**. Esta vez, una corriente eléctrica circula por la viga, la corriente calienta la viga (efecto Joule) y el calor hace que la viga se dilate y se deforme.

Se necesitan tres tipos de problema, que se resuelven en este orden:

1. **Electro-statics** (electrostática) – calcula el potencial eléctrico, el campo eléctrico, la densidad de corriente y el calor Joule producido por la corriente.
2. **Heat transfer** (transferencia de calor) – calcula la distribución de temperatura causada por el calor Joule.
3. **Stress analysis** (análisis de tensiones) – calcula la dilatación térmica (desplazamientos) y la tensión resultante.

Cada problema utiliza los resultados del anterior, por lo que el orden de los problemas es importante.

## 1. Cargar el modelo

**Menú:** _Archivo -> Abrir modelo_

En la ventana **Abrir modelo**, seleccione **Beam.tmsh** y haga clic en **Abrir**.

Como el modelo aún no tiene ningún tipo de problema asignado, la ventana **Flujo de tareas del problema** se abre automáticamente cuando termina la carga.

## 2. Flujo de tareas del problema

1. En la ventana **Flujo de tareas del problema**, haga clic en **Añadir tipo de problema**.
2. En el **Selector de tipo de problema**, marque **Electro-statics**, **Heat transfer** y **Stress analysis** y haga clic en **Aceptar**.

![Selector de tipo de problema – multifísica](image-Problem_type_selector-Multi.png)

El **Flujo de tareas del problema** debería mostrar ahora los tres problemas en un único grupo de iteraciones, en el orden indicado a continuación. Si el orden es distinto o hay entradas de más, seleccione una entrada y corríjalo con los botones **Subir**, **Bajar** y **Eliminar**. Haga clic en **Aceptar** para confirmar.

![Flujo de tareas del problema – multifísica](image-Problem_task_flow-Multi.png)

La ventana se puede volver a abrir en cualquier momento con el botón **Flujo de tareas de los problemas** de la barra de herramientas.

## 3. Generar la malla 3D

Los tres problemas se resuelven sobre una malla de volumen.

**Menú:** _Geometría -> Volumen -> Generar malla tetraédrica_

Los valores predeterminados de la ventana **Generar malla 3D** son suficientes. Haga clic en **Aceptar**. Cuando termina el mallado, aparece una nueva entidad **Volume 1** bajo **Volúmenes** en el árbol **Modelo**.

## 4. Asignar el material

Todas las entidades del modelo deben tener un material asignado. El material debe proporcionar las propiedades que necesitan los tres problemas; **Iron** (hierro) las tiene.

1. En el árbol **Modelo**, haga clic en **Surface** y luego haga clic en **Volume 1** con _Mayús_ pulsada para seleccionar todas las entidades.
2. Abra la pestaña **Material** en el lado derecho de la ventana principal.
3. Marque **Iron**.

## 5. Asignar las condiciones de contorno

Como hay varios tipos de problema seleccionados, la pestaña **Condiciones de contorno** ofrece ahora las condiciones de todos ellos, y una misma entidad puede tener asignadas varias condiciones, una por problema. Asigne las siguientes condiciones a las entidades de **superficie**:

1. **Surface** (la superficie exterior de la viga)
    - _Simple convection_ (convección simple)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293.15 `[K]`
2. **Load** (la cara situada en el centro de la viga)
    - _Electric potential_ (potencial eléctrico)
        - Electric potential = 0.4 `[V]`
    - _Simple convection_ (valores como para **Surface**)
3. **Left** y **Right** (las caras de ambos extremos de la viga)
    - _Displacement_ (desplazamiento)
        - Displacement in X, Y and Z direction = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0.4 `[V]`
    - _Simple convection_ (valores como para **Surface**)

Así, la corriente circula desde el centro de la viga hacia ambos extremos, y todas las caras ceden calor al aire circundante por convección.

Para cada entidad: selecciónela en el árbol **Modelo** (mantenga pulsada _Ctrl_ / _⌘ Cmd_ para seleccionar a la vez **Left** y **Right**), marque la condición en la pestaña **Condiciones de contorno**, seleccione la condición para mostrar sus propiedades bajo la lista y, después, haga doble clic en el valor, escriba el nuevo y pulse _Intro_. Los valores predeterminados de **Simple convection** (100 `[W/(m^2*K)]` y 293.15 `[K]`) y de **Displacement** (0 `[m]`) ya son los correctos, así que solo hay que cambiar **Electric potential**.

## 6. Comprobar las condiciones de entorno

Las condiciones de entorno se aplican a todo el modelo y no se marcan como las condiciones de contorno. Abra la pestaña **Condiciones de entorno** y seleccione cada condición para comprobar sus valores:

- _Gravitational acceleration_ (aceleración de la gravedad)
    - 0, 0 y -9.80665 `[m/s^2]` en las direcciones X, Y y Z
- _Temperature_ (temperatura)
    - Temperature = 293.15 `[K]`

## 7. Resolver el problema

**Menú:** _Solución -> Iniciar solver_

El modelo debe guardarse antes de que pueda iniciarse el solver. Si aún no se ha guardado, primero se muestra la ventana **Guardar modelo**; haga clic en **Guardar** para confirmar (si ya existe un archivo con el mismo nombre, confirme que desea reemplazarlo). A continuación se muestra la ventana **Iniciar el solver para el modelo**.

La ventana muestra algunas advertencias, como **Heat transfer: No explicit boundary condition assigned.** Son esperadas en esta configuración y pueden ignorarse. Haga clic en **Aceptar** para iniciar el solver.

El solver se ejecuta en segundo plano y su salida se muestra en la pestaña **Salida del proceso**. Los problemas se resuelven uno tras otro en el orden del flujo de tareas; la ejecución ha terminado cuando la salida acaba con **SOLVER - END**.

## 8. Crear un campo vectorial

Un **campo vectorial** es una entidad de visualización que dibuja una variable vectorial como flechas. Aquí se utiliza para mostrar el campo eléctrico dentro de la viga.

**Menú:** _Geometría -> Campo vectorial -> Crear campo vectorial_

En la ventana **Editor de campos vectoriales**, seleccione **Volume 1** en la lista **Modelo** y **Electric field** (campo eléctrico) en la lista **Variables**, y deje marcado **Campo vectorial 3D**.

![Crear campo vectorial](image-Create_vector_field.png)

Haga clic en **Aceptar** para crear el campo vectorial. Se añade al árbol **Modelo** bajo **Campos vectoriales**, y se le pueden aplicar los resultados calculados como a cualquier otra entidad del modelo.

## 9. Aplicar los resultados

Los resultados se aplican en la pestaña **Resultados** a la entidad seleccionada en el árbol **Modelo**, del mismo modo que en el tutorial **Análisis de tensiones y deformaciones**.

### 9.1. Temperatura y deformación del volumen

1. En el árbol **Modelo**, marque **Volúmenes** (dejando desmarcadas las superficies) y seleccione **Volume 1**.
2. En la pestaña **Resultados**, elija **Temperature - Scalar** en la lista desplegable y marque **Scalar** y **Mostrar el rango de valores**. Si el rango mostrado no corresponde a los resultados, haga clic en **Establecer valores mín./máx. predeterminados**.
3. Elija **Displacement - Vector [3]** en la lista desplegable, marque **Displacement** y ajuste la **Escala** a **20** para que la deformación sea visible (los desplazamientos calculados son de solo unos 1.3 mm).

La temperatura varía entre unos 505 K y 620 K; la viga está más caliente en el centro de sus dos tramos exteriores y se curva hacia arriba porque los extremos fijos no le permiten dilatarse libremente.

![Temperatura aplicada en el volumen](image-Applied_temperature_on_volume.png)

### 9.2. Temperatura en el campo vectorial

1. En el árbol **Modelo**, desmarque **Volúmenes**, marque **Campos vectoriales** y seleccione **Electric field**.
2. En la pestaña **Resultados**, elija **Temperature - Scalar** y marque **Scalar** y **Mostrar el rango de valores**. Las flechas se colorean ahora según la temperatura en su posición.
3. En las **Propiedades de visualización**, bajo el árbol, marque **Dibujar puntas de flecha** para mostrar la dirección del campo eléctrico.

![Temperatura aplicada en el campo vectorial](image-Applied_temperature_on_vector_field.png)
