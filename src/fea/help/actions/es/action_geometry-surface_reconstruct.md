# Reconstruir superficie

Esta acción vuelve a triangular las superficies seleccionadas. Une los elementos coplanares, elimina los nodos que no definen la forma (nodos dentro de regiones planas y sobre aristas rectas) y elimina los elementos diminutos y alargados.

Los nodos existentes nunca se mueven, solo la opción de malla de calidad crea nodos nuevos. Los nodos y aristas compartidos con otras entidades (puntos, líneas, superficies no seleccionadas) se conservan, las superficies cerradas siguen cerradas y no se crean nuevas autointersecciones. Solo se procesan triángulos lineales y primero hay que eliminar la malla de volumen.

* **Tolerancia angular** - ángulo máximo entre las normales de elementos coplanares y entre las direcciones de aristas colineales.
* **Tolerancia de distancia** - distancia máxima de un nodo a un plano (línea) para considerarlo coplanar (colineal).
* **Longitud mínima de arista** - las aristas más cortas se colapsan. Los detalles menores que este valor pueden desaparecer.
* **Ángulo de astilla** - los elementos cuyo ángulo mínimo es inferior a este valor se consideran astillas.
* **Triangulación de Delaunay** - al final se realiza la triangulación de Delaunay de la superficie: se voltean las aristas cuyos ángulos opuestos suman más de 180 grados, siempre que la forma se conserve dentro de la tolerancia de distancia.
* **Malla de calidad (insertar nodos)** - inserta nuevos nodos (refinamiento de Delaunay) para que el ángulo mínimo de los elementos sea al menos el **Ángulo de calidad** y las aristas no superen la **Longitud máxima de arista** (cero significa sin límite). Los nuevos nodos se colocan sobre la superficie existente y sobre aristas rectas, por lo que la forma se conserva. Las aristas compartidas con entidades no seleccionadas no se dividen. Las superficies curvas formadas por facetas inclinadas individualmente solo se pueden mejorar parcialmente.

Las tolerancias por defecto son relativas al tamaño de las superficies seleccionadas. Tolerancias mayores producen superficies más simplificadas.
