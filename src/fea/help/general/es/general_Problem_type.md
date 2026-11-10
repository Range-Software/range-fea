# Tipo de problema

El **tipo de problema** es un identificador único del fenómeno físico que se va a resolver.

Los tipos de problema se seleccionan en el diálogo **Seleccionar problema** (**Menú:** _Problema -> Seleccionar problema_). Haga clic en un mosaico para seleccionar o deseleccionar un tipo de problema y haga clic en **Aceptar** para confirmar. Los botones de la esquina superior derecha alternan entre la **Vista de mosaicos** y la **Vista de lista**, que muestra además una breve descripción de cada tipo de problema.

![Seleccionar problema](image-Select_problem.png)

Lista de tipos de problema admitidos:

- **Acoustics** (acústica) – presión acústica, velocidad de partícula, intensidad y nivel de presión sonora, en régimen transitorio y armónico (dominio de la frecuencia).
- **Electro-statics** (electrostática) – campo eléctrico, densidad de corriente y calor de Joule generado.
- **Magneto-statics** (magnetostática) – campo magnético.
- **Incompressible viscous flow** (flujo viscoso incompresible) – flujo estacionario y transitorio de fluidos newtonianos.
- **Contaminant dispersion** (dispersión de contaminantes) – distribución de un «contaminante» en el campo de flujo.
- **Heat transfer in fluids** (transferencia de calor en fluidos) – distribución de temperatura en un fluido en movimiento.
- **Heat transfer** (transferencia de calor) – distribución de temperatura y flujo de calor estacionarios y transitorios, con convección simple, forzada y natural.
- **Radiative heat transfer** (transferencia de calor por radiación) – transferencia de calor entre varias superficies capaces de emitir y/o recibir calor.
- **Stress analysis** (análisis de tensiones) – análisis estático de tensiones y elastodinámica lineal, incluida la dilatación térmica y las cargas debidas a la presión de un fluido.
- **Modal analysis** (análisis modal) – frecuencias naturales de la estructura y sus modos propios.
- **Flow through porous media** (flujo a través de medios porosos) – presión y velocidad de un fluido que atraviesa un material permeable.
- **Mesh reconstruction** (reconstrucción de malla) – reconstrucción de la malla según los criterios indicados.

## Combinación de tipos de problema

Es posible combinar varios **tipos de problema** distintos (p. ej., **Heat transfer** y **Stress analysis**, o **Incompressible viscous flow** y **Contaminant dispersion**).

- Algunos tipos de problema requieren otro. Al seleccionar uno de ellos se selecciona también el requerido: **Magneto-statics** requiere **Electro-statics**, **Contaminant dispersion** y **Heat transfer in fluids** requieren **Incompressible viscous flow**, y **Radiative heat transfer** requiere **Heat transfer**.
- Los tipos de problema que no se pueden combinar con la selección actual aparecen atenuados y marcados como **Incompatible** (p. ej., **Modal analysis** no se puede combinar con ningún otro tipo de problema).

## Flujo de tareas

Los tipos de problema seleccionados se resuelven uno tras otro como tareas. Su orden se establece automáticamente, de modo que los resultados de una tarea puedan ser utilizados por la siguiente (p. ej., **Heat transfer** se resuelve antes que **Stress analysis** para tener en cuenta la dilatación térmica).

Haga clic en **Personalizar…** para ver o cambiar el flujo de tareas. Las tareas se ejecutan de arriba abajo. Use los botones **Subir** y **Bajar** para cambiar su orden, **>** para colocar una tarea en un grupo que se repite (con su propio número de iteraciones y valor de convergencia) y **<** para sacarla del grupo. Haga doble clic en un valor para editarlo. **Usar orden automático** descarta el flujo de tareas personalizado.

![Seleccionar problema – flujo de tareas personalizado](image-Select_problem-customize.png)

Si se quita un tipo de problema de un modelo que ya contiene sus resultados, la aplicación ofrece eliminar esos resultados del modelo.
