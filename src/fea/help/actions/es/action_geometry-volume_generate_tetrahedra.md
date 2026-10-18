# Generar malla tetraédrica

Genera una malla de volumen (tetraédrica).


La **malla de volumen** es necesaria para la mayoría de los **tipos de problema**.


El generador de mallas 3D solo puede generar la malla si:
1. El modelo contiene una superficie cerrada (estanca).
2. Los elementos de superficie no se intersecan entre sí.


Para generar varias mallas de volumen que representen un modelo de material compuesto, debe incluirse una superficie de separación.


En algunos casos puede ser necesario sustituir la configuración predeterminada e indicar directamente los parámetros de TetGen.

## Interfaz gráfica

![Ventana Generar malla](image-Generate_mesh_dialog.png)

- **Comprobar la integridad de la superficie** - útil al generar una malla 3D a partir de un modelo de superficie. Permite detectar errores y posibles problemas de la malla de superficie.
- **Reconstruir la malla (reutilizar los nodos actuales)** - conserva los nodos actuales y solo añade otros nuevos si es necesario.
- **Conservar los resultados calculados** - todos los resultados calculados se transfieren/interpolan a los nuevos nodos y elementos.
- **Malla de calidad** - genera la malla 3D de forma que cumpla los criterios de calidad.
    - **Volumen máximo del elemento** - volumen máximo de cada elemento generado. Ningún elemento debe tener un volumen mayor que el especificado.
    - **Generar función de tamaño de malla** - debe usarse para refinar la malla en las zonas donde se observan diferencias en los resultados.
- **Parámetros de TetGen** - parámetros que se pasan al generador de mallas TetGen.

La siguiente imagen muestra la diferencia en la resolución de la malla cuando se utiliza el mallado adaptativo.

![Ejemplo de malla adaptativa](image-Adaptive_mesh-example.png)

## Sintaxis de los parámetros

`pYrq_Aa_miO_S_T_XMwcdzfenvgkJBNEFICQVh`


Los guiones bajos indican que ciertos modificadores pueden ir seguidos opcionalmente de números. No deje ningún espacio entre un modificador y su parámetro numérico.


A continuación se ofrece un resumen de todos los modificadores de línea de comandos con una breve descripción.

- **p** - Tetraedriza un complejo lineal por partes (PLC).
- **r** - Reconstruye una malla generada previamente.
- **q** - Refina la malla (para mejorar su calidad).
- **R** - Simplificación de la malla (para reducir el número de elementos).
- **a** - Aplica una restricción de volumen máximo de los tetraedros.
- **O** - Especifica el nivel de optimización de la malla.
- **S** - Especifica el número máximo de puntos añadidos.
- **T** - Establece una tolerancia para la prueba de coplanaridad (por defecto 10−8).
- **X** - Desactiva el uso de aritmética exacta.
- **M** - No fusiona caras coplanares ni vértices muy cercanos.
- **w** - Genera una triangulación de Delaunay ponderada (regular).
- **c** - Conserva la envolvente convexa del PLC.
- **d** - Detecta las autointersecciones de las caras del PLC.
- **n** - Escribe los vecinos de los tetraedros.
- **C** - Comprueba la consistencia de la malla final.
- **Q** - Silencioso: sin salida en el terminal salvo los errores.
- **V** - Detallado: información detallada, más salida en el terminal.
