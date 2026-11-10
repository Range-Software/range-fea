# Problema

**Problema** es un término general para uno o varios fenómenos físicos aplicados a una geometría. Está definido por el **tipo de problema**, la **configuración específica del problema**, las **condiciones de contorno**, las **condiciones iniciales**, las **condiciones de entorno** y la **configuración del material**.

El problema se define por separado para cada modelo. Sus partes se configuran en las pestañas del lado derecho de la ventana principal: **Problema**, **Condiciones de contorno**, **Condiciones iniciales**, **Condiciones de entorno** y **Material**.

## Selección de un problema

**Menú:** _Problema -> Seleccionar problema_

El diálogo **Seleccionar problema** se utiliza para elegir qué se debe resolver (véase [Tipo de problema](general_Problem_type.md)). También se muestra automáticamente al cargar un modelo sin ningún problema. Mientras no se seleccione un problema, la pestaña **Problema** solo contiene el botón **Seleccionar problema**.

## Configuración específica del problema

Una vez seleccionado un problema, la pestaña **Problema** contiene la configuración específica de los **tipos de problema** seleccionados.

![Pestaña Problema](image-Problem_tab.png)

- **Solver temporal** – márquelo para resolver el problema como **transitorio** (dependiente del tiempo). Al marcarlo se pueden ajustar el método de aproximación, el tiempo inicial, el tamaño del paso de tiempo, el número de pasos de tiempo y la frecuencia de salida; el tiempo final se calcula a partir de ellos (_tiempo final = tiempo inicial + tamaño del paso de tiempo * número de pasos de tiempo_). Si no se marca, el problema se resuelve como **estacionario**. El solver temporal solo está disponible para los tipos de problema que lo admiten.
- Las demás secciones dependen de los **tipos de problema** seleccionados (p. ej., **Configuración del análisis transitorio de tensiones**, configuración de radiación, de acústica o de análisis modal).

Otras acciones relacionadas con el problema se encuentran en el menú **Problema**: _Configurar el solver matricial de los problemas_, _Definir puntos de monitorización_ y _Restablecer configuración_.
