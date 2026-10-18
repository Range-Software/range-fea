# Dispersión de contaminantes en fluidos

Este tutorial muestra cómo configurar y resolver una simulación multifísica que combina un problema no lineal e iterativo – **CFD (dinámica de fluidos computacional)** – con el transporte de un contaminante arrastrado por el flujo.

Se necesitan dos tipos de problema:

1. **Incompressible viscous flow** (flujo viscoso incompresible) – calcula la velocidad y la presión del fluido (flujo estacionario o transitorio de fluidos newtonianos).
2. **Contaminant dispersion** (dispersión de contaminantes) – calcula cómo se dispersa un contaminante en ese campo de flujo.

Como el problema de flujo es no lineal, se resuelve de forma iterativa. Por ello, la simulación se realiza en dos etapas:

1. **Flujo estacionario** – primero se calculan, sin contaminante, un campo de flujo «inicial» convergido y la distribución de presión.
2. **Simulación transitoria** – después, partiendo de ese campo de flujo, se utiliza la **integración en el tiempo** para seguir cómo se transporta a lo largo del canal un pulso de contaminante que entra en él.

## 1. Cargar el modelo

**Menú:** _Archivo -> Abrir modelo_

En la ventana **Abrir modelo**, seleccione **Channel.tmsh** y haga clic en **Abrir**.

El modelo consta de tres entidades de superficie – **Walls**, **Inflow** y **Outflow**. Como el modelo aún no tiene ningún tipo de problema asignado, la ventana **Flujo de tareas del problema** se abre automáticamente cuando termina la carga.

## 2. Flujo de tareas del problema (etapa 1 – flujo estacionario)

En la primera etapa solo se resuelve el flujo, con muchas iteraciones no lineales para que pueda converger.

1. En la ventana **Flujo de tareas del problema**, haga clic en **Añadir tipo de problema**.
2. En el **Selector de tipo de problema**, marque **Incompressible viscous flow** y haga clic en **Aceptar**.
3. Haga doble clic en el valor **1** junto a **N.º de iteraciones:**, escriba **2000** y pulse _Intro_.
4. Haga clic en **Aceptar** para cerrar la ventana.

La ventana se puede volver a abrir en cualquier momento con el botón **Flujo de tareas de los problemas** de la barra de herramientas. Ahora debería tener este aspecto:

![CFD – Flujo de tareas del problema, estacionario](image-CFD-Problem_task_flow_steady.png)

## 3. Generar la malla de volumen

El modelo solo está definido por sus superficies. Para resolver un flujo en su interior, primero hay que generar una malla de volumen (tetraédrica).

**Menú:** _Geometría -> Volumen -> Generar malla tetraédrica_

Los valores predeterminados de la ventana **Generar malla 3D** son suficientes. Haga clic en **Aceptar**. Cuando termina el mallado, aparece una nueva entidad **Volume 1** bajo **Volúmenes** en el árbol **Modelo**.

## 4. Asignar el material

Todas las entidades del modelo deben tener un material asignado.

1. En el árbol **Modelo**, haga clic en **Walls** y luego haga clic en **Volume 1** con _Mayús_ pulsada para seleccionar todas las entidades.
2. Abra la pestaña **Material** en el lado derecho de la ventana principal.
3. Marque **Water** (agua).

## 5. Condiciones de contorno (etapa 1)

Las condiciones de contorno se asignan en la pestaña **Condiciones de contorno** a la entidad seleccionada en el árbol **Modelo**. Asigne las siguientes condiciones a las entidades de **superficie**:

1. **Walls**
    - _Wall_ (pared; no requiere ningún valor)
2. **Inflow**
    - _Volumetric flow rate (inflow)_ (caudal volumétrico de entrada)
        - Volumetric flow rate = 50 `[m^3/s]`
3. **Outflow**
    - _Pressure (implicit)_ (presión implícita)
        - Pressure = 0 `[Pa]` (el valor predeterminado es 1000 `[Pa]`)

Para cada entidad: selecciónela en el árbol **Modelo**, marque la condición en la pestaña **Condiciones de contorno**, seleccione la condición para mostrar sus propiedades bajo la lista y, después, haga doble clic en el valor, escriba el nuevo y pulse _Intro_.

## 6. Resolver el flujo estacionario

**Menú:** _Solución -> Iniciar solver_

Si el modelo aún no se ha guardado, primero se muestra la ventana **Guardar modelo**. Haga clic en **Guardar** para aceptar el nombre de archivo propuesto (**Channel.rbm**) y, a continuación, haga clic en **Aceptar** en la ventana **Iniciar el solver para el modelo**.

El solver se ejecuta en segundo plano y su salida se muestra en la pestaña **Salida del proceso**. El cálculo de las 2000 iteraciones tarda unos minutos; la ejecución ha terminado cuando la salida acaba con **SOLVER - END**. La convergencia de la solución se puede comprobar en cualquier momento con:

**Menú:** _Informe -> Convergencia del solver_

## 7. Crear un corte

Una buena forma de examinar un flujo es mostrar la velocidad como un **campo vectorial**, en el que cada flecha indica la dirección y la magnitud del flujo en un punto. Sin embargo, un campo vectorial dibujado en todo el volumen se convierte rápidamente en una nube densa de flechas superpuestas difícil de leer. Suele ser más claro cortar primero el modelo con un **corte** – un plano que atraviesa el volumen – y mostrar después el campo vectorial solo en ese plano.

_Consejo: marque **Volúmenes** en el árbol **Modelo** antes de empezar, para que el volumen sea visible mientras se coloca el corte._

**Menú:** _Geometría -> Corte -> Crear corte_

En la ventana **Editor de cortes**:

1. Seleccione **Volume 1** en la lista **Modelo**. El corte se creará a través de este volumen.
2. **Posición del plano** define un punto por el que pasa el plano de corte. Los valores predeterminados lo sitúan en el centro del modelo (**X** = 7.5, **Y** = 2.5, **Z** = 1.5 `[m]`); consérvelos.
3. **Normal del plano** define la orientación del plano. La normal predeterminada (**X** = 1) da un plano vertical; cámbiela a **X** = 0, **Y** = 0, **Z** = 1 para obtener un plano horizontal que corte el canal a media altura.

El plano de corte se previsualiza en el **área 3D del modelo** mientras se modifican los valores. Haga clic en **Aceptar** para crear el corte. Aparece una nueva entidad **Cut 1** en el árbol **Modelo** bajo **Cortes**.

![CFD – Crear una entidad de corte](image-CFD-Create_cut.png)

## 8. Crear un campo vectorial

Ahora muestre la velocidad calculada como flechas sobre el plano de corte.

**Menú:** _Geometría -> Campo vectorial -> Crear campo vectorial_

En la ventana **Editor de campos vectoriales**:

1. Seleccione **Cut 1** en la lista **Modelo**. Las flechas se dibujarán sobre el plano de corte creado en el paso anterior.
2. Seleccione **Velocity** (velocidad) en la lista **Variables**.
3. Deje marcado **Campo vectorial 3D**. Así las flechas muestran el vector velocidad completo. Si se desmarca, cada flecha se proyecta sobre el plano de corte y solo se muestra la parte del flujo contenida en el plano.

Haga clic en **Aceptar** para crear el campo vectorial. Aparece en el árbol **Modelo** bajo **Campos vectoriales**. Desmarque **Volúmenes** para que el volumen sólido no oculte las flechas.

![CFD – Crear una entidad de campo vectorial](image-CFD-Create_vector.png)

Puede que las flechas sean demasiado cortas para verse con claridad, porque su longitud es proporcional a la velocidad del flujo. Para alargarlas, seleccione el campo vectorial en el árbol **Modelo**, abra la pestaña **Resultados** y aumente el valor en la sección **Escala**. La escala es el producto del valor de **Escala** y de la potencia de diez definida en **Factor**; en el ejemplo siguiente, **Escala** 1 y **Factor** 2 dan una escala de `1 × 10^2 = 100`.

![CFD – Escala de la entidad de campo vectorial](image-CFD-Scale_vector.png)

## 9. Flujo de tareas del problema (etapa 2 – transitoria)

Con un campo de flujo convergido disponible, se puede configurar la simulación **transitoria** que incluye **Contaminant dispersion**.

En una simulación transitoria los problemas se resuelven una vez por paso de tiempo. **Incompressible viscous flow** sigue siendo no lineal, por lo que necesita sus propias iteraciones no lineales dentro de cada paso de tiempo, mientras que **Contaminant dispersion** se resuelve una vez por paso de tiempo, después del flujo. Vuelva a abrir la ventana **Flujo de tareas del problema** (botón **Flujo de tareas de los problemas** de la barra de herramientas) y configúrela como sigue:

1. Seleccione **Incompressible viscous flow** y haga clic en el botón **>**. Así se mueve a un nuevo grupo de iteraciones anidado.
2. Haga doble clic en el valor del **N.º de iteraciones:** exterior y cámbielo de **2000** a **1**. Después cambie el **N.º de iteraciones:** interior (anidado) de **1** a **10**.
3. Haga clic en **Añadir tipo de problema**, marque **Contaminant dispersion** y haga clic en **Aceptar**. **Incompressible viscous flow** también se marca automáticamente, porque la dispersión de contaminantes necesita un campo de flujo. Si se añade una segunda entrada **Incompressible viscous flow** en el nivel exterior, selecciónela y haga clic en **Eliminar**.

El flujo de tareas debería tener el aspecto de la captura de pantalla siguiente. Haga clic en **Aceptar** para confirmar.

![CFD – Flujo de tareas del problema](image-CFD-Problem_task_flow.png)

## 10. Configuración del solver temporal

Abra la pestaña **Problema** en el lado derecho de la ventana principal y configure:

1. Marque **Solver temporal**.
2. **Aproximación**: **Backward difference (stable)** (diferencias regresivas, estable).
3. **Tamaño del paso de tiempo**: **0.01** `[s]`.
4. **Número de pasos de tiempo**: **2000**.
5. **Frecuencia de salida**: **10** – se escribe un registro de resultados cada 10 pasos de tiempo.

El **Tiempo final** se calcula automáticamente (2000 × 0.01 s = 20 s).

En la sección **Configuración de la dispersión de contaminantes**, ajuste la **Saturación máxima** a **2.4** `[kg/m^3]` y el **Coeficiente de difusión** a **0.01** `[m^2/s]`. Para más información, consulte el manual de teoría.

![CFD – Solver temporal del problema](image-CFD-Problem_time_solver.png)

## 11. Condiciones de contorno (etapa 2)

El contaminante entra en el canal por la entrada. Seleccione **Inflow** en el árbol **Modelo** y, en la pestaña **Condiciones de contorno**, marque **Particle concentration** (concentración de partículas); mantenga marcado **Volumetric flow rate (inflow)**.

En lugar de un valor constante, se utilizará un valor dependiente del tiempo. Despliegue la propiedad **Particle concentration** (flecha delante de su nombre) y haga clic en **Editar valores dependientes del tiempo**.

![CFD – Condiciones de contorno](image-CFD-Boundary_conditions.png)

En la ventana **Editor de componentes**, ajuste **Número de valores** a **3** e introduzca los valores siguientes (haga doble clic en una celda para editarla); después haga clic en **Aceptar**.

| Tiempo `[s]` | Particle concentration `[kg/m^3]` |
| ------------ | --------------------------------- |
| 0            | 0                                 |
| 5            | 1                                 |
| 10           | 0                                 |

Entre dos instantes indicados, el valor se interpola linealmente, y después del último instante indicado se conserva el último valor. Con estos valores, la concentración de entrada sube de 0 a 1 `[kg/m^3]` durante los 5 primeros segundos y vuelve a 0 entre los 5 y los 10 segundos.

![CFD – Condición Particle concentration](image-CFD-Particle_concentration_condition.png)

## 12. Resolver el problema transitorio (reinicio)

**Menú:** _Solución -> Iniciar solver_

En la ventana **Iniciar el solver para el modelo**, marque **Reiniciar el solver / continuar** y haga clic en **Aceptar**. El solver utiliza entonces como punto de partida el flujo estacionario ya calculado y continúa con la simulación de integración en el tiempo.

La advertencia **Contaminant dispersion: No implicit boundary condition assigned.** es esperada y puede ignorarse.

![CFD – Reinicio del solver](image-CFD-Solver_restart.png)

## 13. Mostrar los resultados mientras se ejecuta el solver

El solver transitorio se ejecuta en segundo plano, por lo que los resultados se pueden examinar mientras sigue calculando. En cualquier momento se pueden crear otras entidades de visualización, como cortes, iso, líneas de corriente, campos vectoriales o campos escalares, y mostrar en ellas los resultados calculados. Aquí se reutilizan el corte y el campo vectorial de los pasos 7 y 8.

### 13.1 Concentración del contaminante en el corte

1. En el árbol **Modelo**, despliegue **Cortes**, marque **Cut 1** para que sea visible y selecciónelo.
2. Abra la pestaña **Resultados** y elija **Particle concentration - Scalar** en la lista desplegable superior.
3. En **Aplicar como ...**, marque **Scalar**. El corte se colorea ahora según la concentración del contaminante.
4. En **Rango de valores y escala de colores**, ajuste **Máximo** a **1** y **Mínimo** a **0** (escriba el valor y pulse _Intro_). Esto corresponde al rango de la concentración de entrada, de modo que los mismos colores indican la misma concentración en cada paso de tiempo.
5. Marque **Mostrar el rango de valores** para mostrar la escala de colores en el **área 3D del modelo**.

![Resultados de la concentración de partículas](image-CFD-Particle_concentration_results.png)

### 13.2 Flechas del campo vectorial

Para que la dirección del flujo se lea mejor sobre el corte coloreado, dibuje las flechas con puntas y en blanco:

1. Seleccione el campo vectorial (**Velocity** bajo **Campos vectoriales**) en el árbol **Modelo**.
2. En las **Propiedades de visualización**, bajo el árbol, marque **Dibujar puntas de flecha**.
3. Haga clic en el selector de color situado bajo las casillas y elija el blanco (**#ffffffff**).

Las demás propiedades de visualización determinan si todas las flechas tienen la misma longitud (**Dibujar flechas de igual longitud**) y si las flechas parten del punto donde se evalúa la velocidad (**Dibujar flechas que parten de**).

![CFD – Dibujar puntas de flecha](image-CFD-Draw_arrow_heads.png)

## 14. Registros del modelo (resultados en el tiempo)

Cada 10 pasos de tiempo (la **Frecuencia de salida**), el solver escribe un registro del modelo con la solución para ese instante. Los registros aparecen en la pestaña **Registros**, en el lado izquierdo de la ventana principal; la columna **Registro** indica el número del paso de tiempo (el registro **500** contiene la solución en 500 × 0.01 s = 5 s). Haga doble clic en un registro para cargar los resultados de su paso de tiempo; los resultados aplicados en la pestaña **Resultados** se conservan y se vuelven a dibujar para el registro cargado.

El cálculo de los 2000 pasos de tiempo tarda un rato (unos 20 minutos en un ordenador actual). El solver se puede detener en cualquier momento y los registros escritos hasta entonces se conservan:

**Menú:** _Solución -> Detener solver_ (confirme con **Yes**)

## 15. Grabar un vídeo

El vídeo se crea a partir de lo que se muestra en ese momento en el **área 3D del modelo**, así que primero configure la vista y los resultados mostrados (véase el paso 13). La configuración de visualización no se guarda con el modelo, por lo que hay que volver a definirla tras reabrirlo. Después abra la pestaña **Registros**, use los campos **Desde** y **Hasta** de la parte inferior para elegir el rango de registros y haga clic en el botón rojo de grabación.

![CFD – Registros del modelo](image-CFD-Model_records.png)

Aparece la ventana **Configuración del vídeo**, en la que se pueden ajustar el códec de vídeo, el formato de archivo, los fotogramas por segundo y los fotogramas por registro; debajo se muestra la duración estimada del vídeo. Con 202 registros, 24 fotogramas por segundo y 1 fotograma por registro, el vídeo dura unos 8.4 segundos. Haga clic en **Aceptar** para empezar la grabación.

![CFD – Configuración del vídeo](image-CFD-Video_settings.png)

La grabación puede tardar un tiempo, ya que cada registro del modelo se carga por turno y se toma una captura del **área 3D del modelo** como fotograma del vídeo.

El vídeo y todos los fotogramas (una imagen PNG por registro) se encuentran en la pestaña **Documentos**, bajo **Channel**; el vídeo **Channel.mp4** está al final de la lista. Haga doble clic en él para reproducirlo.

![CFD – Documentos](image-CFD-Documents.png)
