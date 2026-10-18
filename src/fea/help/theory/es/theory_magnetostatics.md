# Magnetostática - Manual de teoría y guía del usuario

Este documento describe el solver de magnetostática (`RSolverMagnetostatics`)
de Range FEA, que atiende el tipo de problema **Magnetostática**
(*Magneto-statics*): la ley que evalúa, los datos de entrada que acepta, las
partes de la interfaz gráfica que lo controlan, un tutorial completo y el valor
de su resultado.

> **Estado.** El solver evalúa directamente, con la **ley de Biot-Savart**, el
> campo magnético de la corriente calculada por la tarea de electrostática. No
> hay sistema de ecuaciones ni condiciones de contorno: el campo queda
> determinado solo por la corriente, y la condición de que se anule lejos del
> conductor está incorporada en la ley. Para la densidad de corriente que recibe,
> el resultado es exacto salvo un error de cuadratura controlado muy inferior a
> `1e-4` del campo máximo. El solver trabaja en **el vacío** (no se pueden
> representar materiales magnéticos) y solo calcula el campo de la corriente
> modelada, por lo que el recorrido de la corriente debe cerrarse dentro del
> modelo. La sección 5 enumera las limitaciones.

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - el campo de un conductor por el que circula corriente](#3-tutorial---el-campo-de-un-conductor-por-el-que-circula-corriente)
4. [Comprobación de un modelo](#4-comprobación-de-un-modelo)
5. [Limitaciones](#5-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 El resultado - el campo magnético

El resultado es la **densidad de flujo magnético** `B` en cada nodo del modelo,
en teslas, como vector de tres componentes.

Se **evalúa, no se resuelve**. Una vez conocida la densidad de corriente, el
campo magnético en el vacío se deduce de ella mediante una integral, por lo que
el solver no ensambla ninguna matriz ni realiza ninguna iteración. Cada nodo del
modelo recibe un valor, tanto si está en el conductor como en su superficie o en
cualquier otra parte de la malla.

### 1.2 Ley fundamental

En el vacío, el campo magnético estático cumple la ley de Ampère y no tiene
fuentes:

```
curl(B) = u0 * J
div(B)  = 0
```

junto con la condición de que `B` se anule lejos de todas las corrientes. El
único campo que cumple las tres condiciones viene dado por la **ley de
Biot-Savart**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `B` | densidad de flujo magnético | `T` | calculada |
| `J` | densidad de corriente | `A/m^2` | de la tarea de electrostática, véase 1.4 |
| `u0` | permeabilidad del vacío | `H/m` | integrada, `1.25663706212e-6` |
| `x` | punto de cálculo: un nodo del modelo | `m` | |
| `y` | punto fuente: dentro de un elemento conductor | `m` | |

Tres propiedades se derivan de la propia ley y no de lo que hace el solver:

- **`div(B) = 0` se cumple exactamente** en todas partes, sea cual sea la malla.
- **El campo lejano es correcto sin contorno.** `B` disminuye con la distancia
  como prescribe la ley; no hay superficie exterior, ni región de aire que
  mallar, ni condición que definir en ella.
- **`curl(B) = u0 * J` se cumple allí donde la corriente está cerrada**, es
  decir, donde `div(J) = 0` y el recorrido de la corriente forma bucles. La
  sección 1.4 explica lo que esto significa para un modelo electrostático, cuya
  corriente entra en el cuerpo y sale de él.

La permeabilidad del material no interviene en ningún sitio. No hay propiedad
de material de permeabilidad relativa, ni magnetización, ni distinción entre `B`
y `H`: el solver trabaja en el vacío, sea cual sea el material del modelo. La
sección 1.9 explica por qué esto no puede superarse con el mismo enfoque.

**No se requiere ninguna propiedad de material.** El tipo de problema de
magnetostática no declara ninguna, y la comprobación de la configuración no
tiene nada que verificar por su parte. La tarea de electrostática de la que
depende este tipo de problema tiene sus propios tres requisitos (véase el manual
de electrostática).

### 1.3 Discretización

El solver de electrostática produce una densidad de corriente **constante en
cada elemento**. Por tanto, la integral de 1.2 se descompone en una integral por
elemento, cada una con un `J_e` constante que puede sacarse fuera:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` solo depende de la geometría del elemento y del punto de cálculo. Todos los
tipos de elemento que acepta el solver tienen caras planas, y para ellos `G_e`
tiene una forma cerrada, por lo que el solver integra cada elemento **de forma
exacta** y no aproximada:

- **Tetraedro.** El integrando es el gradiente de `1/|x - y|` respecto de `y`, de
  modo que la integral de volumen se convierte en una suma sobre las cuatro
  caras, `G = SUM_f n_f * INT_f 1/R dA`. La integral de `1/R` sobre un triángulo
  plano se conoce en forma cerrada: un logaritmo a lo largo de cada arista y el
  ángulo sólido que subtiende el triángulo visto desde `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Triángulo** de una entidad de superficie con espesor `t`. La corriente es una
  corriente laminar `K = J*t` en `A/m`. La parte tangencial de `G` es de nuevo una
  suma de logaritmos de aristas, `SUM_e m_e * I_e`, y la parte normal es el
  ángulo sólido con signo.
- **Cuadrilátero.** Se divide en dos triángulos.
- **Segmento** de una entidad de línea con sección `A`. La corriente es
  `I = A * (J . t)` a lo largo del segmento, y el campo de un hilo recto finito
  es clásico:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  siendo `a` y `b` los vectores que van del punto de cálculo a los dos extremos.

Aquí `I_e` es la integral de `1/R` a lo largo de la arista `e`, `P_e` la
distancia en el plano del punto de cálculo a esa arista, `m_e` la normal exterior
de la arista en el plano, `d` la altura del punto de cálculo sobre el plano de la
cara y `omega` el ángulo sólido de la cara visto desde el punto de cálculo.

Los **elementos lejanos** no necesitan la forma cerrada. Allí el integrando es
suave, y una regla de cuadratura es prácticamente exacta por una fracción del
coste:

| Distancia del punto de cálculo al centro del elemento | Regla |
|---|---|
| menos de 2 x la arista más larga del elemento | forma cerrada |
| de 2 a 6 x | regla de grado dos: 3 puntos en un triángulo, 4 en un tetraedro |
| más de 6 x | regla del punto medio: el elemento como corriente puntual |

Los segmentos siempre usan la forma cerrada, que cuesta lo mismo que un punto de
cuadratura. El error combinado de las reglas se mide en las pruebas unitarias
frente a la forma cerrada sumada sobre todos los elementos de un conductor
completo, y se mantiene por debajo de `1e-4` del campo máximo en cada nodo:
dentro del conductor, en su superficie y más allá.

**El campo en el propio conductor.**

- Para una **corriente volumétrica**, el campo es continuo y finito en todas
  partes, incluidos los nodos, aristas y caras de los elementos conductores. Un
  nodo en la superficie de un conductor macizo recibe el verdadero campo de
  superficie.
- Una entidad de **línea** o de **superficie** representa un conductor de
  sección nula, y el campo de tal conductor es singular: crece como `1/r` hacia
  un hilo y de forma logarítmica hacia el borde de una lámina, y presenta un
  salto de `u0 * K x n` a través de una lámina. En un nodo situado sobre tal
  conductor, el solver devuelve un valor finito: un segmento no aporta nada a un
  punto de su propia recta, una lámina aporta la media de los campos de sus dos
  lados, y el logaritmo de arista de una lámina se omite en un nodo situado sobre
  esa arista. Estos valores están regularizados y no son físicos. Lea el campo de
  un conductor delgado a unas cuantas veces el tamaño de su sección, donde el
  modelo delgado es válido.

### 1.4 La fuente - la densidad de corriente

El campo magnético se basa en un único dato de entrada: la **densidad de
corriente** que deja una tarea *Electro-statics* situada antes en el flujo de
tareas del problema. No hay nada que introducir ni que asignar.

El solver lee la densidad de corriente **en los elementos** que almacena la
tarea de electrostática y la utiliza tal cual, sin promediarla en los nodos. Los
elementos que conducen corriente son:

| Entidad | Condición | Fuente |
|---|---|---|
| Volumen | tetraedros | `J` |
| Superficie | espesor `> 0` | corriente laminar `J * thickness` (`thickness` = espesor) |
| Línea | sección `> 0` | corriente `J * cross area` (`cross area` = sección) |

Los elementos con densidad de corriente nula se omiten, lo que incluye todos los
elementos que no calculó la tarea de electrostática. También se omiten los
elementos cuya densidad de corriente es inferior a `1e-10` de la mayor del
modelo: una región poco conductora (aire con una conductividad de `5e-15 S/m`
junto al cobre) añadiría si no una fuente por elemento y nada medible al campo.
Si ningún elemento conduce corriente (la tarea de electrostática no se ha
ejecutado, los potenciales son iguales o ningún material tiene conductividad
eléctrica), el registro muestra una advertencia y el campo es nulo en todas
partes.

**La corriente tiene que cerrarse.** La ley de Biot-Savart da el campo de la
corriente que recibe y nada más. Una corriente física circula en bucles
cerrados, pero un modelo electrostático suele contener solo una parte del bucle:
una barra entre dos electrodos, con la corriente que entra por un extremo y sale
por el otro. Los cables de alimentación y el resto del circuito no están en el
modelo, ni tampoco su campo. Lo que calcula el solver es entonces el campo del
conductor modelado como si la corriente apareciera en un extremo y desapareciera
en el otro:

- **cerca del conductor** suele ser una aproximación excelente, ya que allí el
  campo está dominado por la corriente cercana;
- **lejos de él**, a distancias comparables al propio conductor, la parte que
  falta del bucle importa tanto como la modelada, y el resultado no es el campo
  de ningún circuito real.

Cuando importa el campo lejano, incluya el camino de retorno en el modelo
electrostático para que la corriente forme un bucle. Un conductor de retorno
delgado puede representarse con una entidad de línea con sección.

### 1.5 Condiciones de contorno, iniciales y de entorno

No hay **ninguna**, y no hace falta ninguna.

| Tipo de condición | Ofrecida para la magnetostática |
|---|---|
| Condiciones de contorno | ninguna |
| Condiciones iniciales | ninguna |
| Condiciones de entorno | ninguna |
| Propiedades del material | ninguna |

El campo queda totalmente determinado por la corriente y por la exigencia de que
se anule lejos, que la ley de Biot-Savart lleva incorporada. No hay contorno
exterior que cerrar ni nada que imponer en él.

Cuando una tarea de electrostática y una de magnetostática comparten un flujo de
tareas (lo que siempre ocurre, véase 1.6), las pestañas de condiciones siguen
ofreciendo las condiciones **electrostáticas**, porque las pestañas se rellenan a
partir de los tipos de problema de todo el flujo. *Electric potential* y *Charge
density* pertenecen a la tarea de electrostática y llegan al campo magnético a
través de la densidad de corriente que producen.

### 1.6 Acoplamiento con otros solvers

| Sentido | Variable | Efecto |
|---|---|---|
| Electrostática -> magnetostática | Current density | la única fuente del campo magnético |

**La magnetostática requiere la electrostática.** La dependencia está declarada
en el tipo de problema y la interfaz la impone: marcar *Magneto-statics* en el
selector de problemas marca automáticamente *Electro-statics*, y desmarcar
*Electro-statics* desmarca también *Magneto-statics*. El flujo de tareas
predeterminado coloca primero la tarea de electrostática, que es el orden que
necesita el acoplamiento.

Nada utiliza el campo magnético. Es el final de su cadena: ningún solver lo lee,
y no hay fuerza de Lorentz, ni corriente inducida, ni contribución magnética a
ninguna otra física.

Como el solver se declara convergido incondicionalmente, un grupo de tareas que
contenga una tarea de magnetostática siempre se detiene tras una iteración.

### 1.7 Evaluación del campo

Cada elemento que conduce corriente contribuye a cada nodo, por lo que el trabajo
crece como el **número de nodos por el número de elementos que conducen
corriente**. La evaluación se ejecuta en paralelo sobre los nodos, que se
procesan en bloques, de modo que cada elemento se lee de la memoria una vez por
bloque y no una vez por nodo.

A modo de referencia, una barra de `120 000` tetraedros evaluada en sus `24 000`
nodos tarda unos cinco segundos con 14 hilos. Duplicar la malla en cada dirección
multiplica por ocho el número de nodos y el de elementos, y el tiempo por unos
sesenta y cuatro; un modelo de unos cientos de miles de elementos tarda minutos
en lugar de segundos.

Solo cuentan los elementos que conducen corriente. Una malla grande alrededor de
un conductor pequeño (una región de aire, véase 2.8) añade nodos pero no
fuentes.

No se utiliza la configuración del solver matricial.

### 1.8 Resultados

| Resultado | Se aplica a | Significado |
|---|---|---|
| Magnetic field `[T]` | nodo | el vector del campo |

Esa es toda la salida. `process()` no hace nada, por lo que no hay magnitudes
derivadas: ni módulo del campo almacenado por separado, ni flujo, ni densidad de
energía. La vista 3D puede mostrar el módulo y las componentes de la variable
vectorial de la forma habitual, y `Magnetic field in X direction` y sus
equivalentes `Y` y `Z` están disponibles para mostrar las componentes.

El campo magnético se almacena como variable **en los nodos**, a diferencia de
los resultados del campo electrostático, que son variables en los elementos.
**Cada nodo** del modelo tiene un valor, incluidos los nodos de entidades que no
conducen corriente, de modo que una malla alrededor del conductor muestra el
campo en el espacio que lo rodea.

Una vez escrito el registro, el registro del solver muestra las estadísticas del
campo magnético y los valores en los posibles puntos de monitorización.

### 1.9 Lo que es el solver y lo que no es

**Es una integral, no una ecuación de campo.** Evaluar Biot-Savart no necesita
contorno ni malla fuera del conductor, y es exacto para la corriente dada. El
precio es que solo funciona en un medio **uniforme**. Con materiales magnéticos,
el campo de una corriente depende de dónde esté el hierro, y ninguna integral de
la corriente por sí sola puede expresarlo; ese problema se resuelve con una
formulación en potencial (normalmente un potencial vector magnético sobre
elementos de arista), que es otro solver y no una extensión de este.

**Se basa en una corriente de conducción calculada.** No hay forma de indicar una
corriente directamente, ni imanes permanentes, ni campos impuestos; la corriente
tiene que producirla una resolución electrostática sobre un cuerpo conductor, y
su precisión determina la del resultado magnético. El manual de electrostática
describe qué es esa densidad de corriente y qué no es. Una densidad de corriente
constante por elemento es una aproximación de primer orden de la real, y converge
a medida que se refina la malla.

**Calcula el campo de la corriente modelada.** Véase 1.4: la corriente debe
cerrarse dentro del modelo cuando importa el campo lejos del conductor.

### 1.10 Análisis estático y solver temporal

La magnetostática se declara como tipo de problema **sin** solver temporal,
exactamente igual que la electrostática. El grupo *Solver temporal* no aparece en
la pestaña `Problema`, el cálculo produce un único registro y no hay nada que
configurar.

Como un flujo de magnetostática siempre contiene también una tarea de
electrostática, y ninguna de las dos admite solver temporal, añadir cualquiera de
ellas a un flujo elimina el solver temporal de todo el flujo. Una tarea térmica
encadenada tras ellas solo puede ser estacionaria. El manual de electrostática lo
trata en su propia sección 1.10.

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Material`, `Resultados`), el menú **Problema** y el panel **Modelo**. Las
pestañas de condiciones no contienen nada para este tipo de problema.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Añada una tarea y elija **Magneto-statics**:
«Calculate magnetic field» (calcular el campo magnético).

**Electro-statics se marca automáticamente.** El selector impone la dependencia
de 1.6 en ambos sentidos: marcar *Magneto-statics* marca *Electro-statics*, y
desmarcar *Electro-statics* desmarca *Magneto-statics*. El flujo de tareas
predeterminado coloca primero la tarea de electrostática.

La magnetostática se combina con todas las demás físicas excepto *Modal
analysis*. En la práctica, la única cadena que tiene sentido es aquella en la que
se basa:

- *Electro-statics* y luego *Magneto-statics*: el campo magnético de una
  corriente de conducción calculada.

### 2.2 Pestaña Problema

Esta pestaña está **vacía** para un flujo de electrostática y magnetostática.
Ninguno de los dos tipos de problema tiene solver temporal ni grupo de
configuración propio.

### 2.3 Pestaña Material

La magnetostática no requiere **nada** del material. La tarea de electrostática
de la que depende requiere tres propiedades (permitividad relativa,
conductividad eléctrica y densidad), y una entidad cuyo material carezca de
alguna de ellas es omitida por *esa* tarea, lo que la deja sin densidad de
corriente y, por tanto, sin contribución al campo magnético.

Así pues, el material sigue importando, pero solo a través de la mitad
electrostática de la cadena. Asigne un material con las tres, como el material
suministrado *Copper* (cobre), y consulte en el manual de electrostática para qué
sirve cada una.

No hay ninguna propiedad de permeabilidad relativa que asignar. El solver trabaja
en el vacío.

### 2.4 Pestaña Condiciones de contorno

Nada de esta pestaña pertenece a la tarea de magnetostática. Las condiciones que
ofrece, *Electric potential* y *Charge density*, son las electrostáticas, y
determinan la densidad de corriente que a su vez produce el campo magnético.
Asígnelas como se describe en el manual de electrostática.

No existe ninguna condición de contorno magnetostática ni hace falta (véase
1.5).

### 2.5 Condiciones iniciales y de entorno

Ninguna de las dos pestañas ofrece nada para la magnetostática. La condición
inicial *Electric potential* y la condición de entorno *Temperature* que se
muestran pertenecen a la tarea de electrostática.

### 2.6 Configuración del solver matricial

El solver de magnetostática no resuelve ningún sistema de ecuaciones y no utiliza
la configuración del solver matricial. La entrada **CG** solo configura el solver
de electrostática.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra. *Magnetic field*
está disponible en la lista, junto con las variables electrostáticas, y lee el
campo nodal en la sonda.

### 2.8 Resultados y registros

La pestaña `Resultados` muestra **Magnetic field** como vector en los nodos, junto
con los resultados electrostáticos que produjo la primera tarea. La pestaña
`Registros` del panel `Modelo` contiene exactamente un registro: ambos tipos de
problema son estáticos.

`Informe` -> `Archivo de registro del solver` muestra la salida completa: primero
las estadísticas de la electrostática, luego el número de elementos que conducen
corriente y de nodos para los que se evaluó la integral de Biot-Savart y, por
último, las estadísticas del campo magnético.

**Ver el campo alrededor de un conductor.** El campo se calcula en cada nodo, por
lo que cualquier cosa mallada alrededor del conductor lo muestra. Malle el
espacio circundante como una entidad de volumen independiente (una región de
aire) y asígnele un material sin conductividad eléctrica: la tarea de
electrostática la omite, de modo que no conduce corriente, pero sus nodos reciben
el campo magnético. La región cuesta nodos pero no fuentes (1.7) y no necesita un
tamaño ni un contorno particulares, ya que en ella no se resuelve nada.

El material suministrado *Air* tiene una conductividad eléctrica, aunque muy
pequeña, por lo que la tarea de electrostática incluiría la región en su propia
resolución. Utilice una copia de él sin la propiedad de conductividad eléctrica.
La comprobación de la configuración avisará entonces de que al material de la
entidad le faltan propiedades requeridas; para la región de aire es lo esperado,
y el cálculo continúa.

---

## 3. Tutorial - el campo de un conductor por el que circula corriente

**Objetivo.** Ejecutar de principio a fin la cadena de electrostática y
magnetostática sobre una barra de cobre por la que circula corriente, comprobar
el campo magnético con un cálculo manual y ver dónde se manifiestan las
aproximaciones de 1.4.

Este tutorial supone la barra mallada del manual de electrostática: una caja de
`0.1 m` de largo con una sección cuadrada de `0.01 m`, mallada en tetraedros, con
las dos caras de los extremos marcadas como entidades de superficie
independientes. Si ese modelo sigue configurado, solo cambia el flujo de tareas.

### Paso 1 - seleccionar los problemas

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Marque **Magneto-statics**. Observe cómo **Electro-statics** se marca solo.
3. Confirme con `Aceptar`.

El flujo de tareas contiene ahora dos tareas, primero la de electrostática. La
pestaña `Problema` sigue vacía.

### Paso 2 - asignar el material

Seleccione la entidad de volumen y asigne **Copper** en la pestaña `Material`, o
un material con las tres propiedades que necesita la tarea de electrostática:

- **Relative permittivity** (permitividad relativa) = `2.8`
- **Electrical conductivity** (conductividad eléctrica) = `5.882e7` `S/m`
- **Density** (densidad) = `8960` `kg/m^3`

La tarea de magnetostática no lee nada de esto. Es la tarea de electrostática la
que lo necesita, y sin densidad de corriente no hay campo magnético.

### Paso 3 - hacer circular una corriente

1. Seleccione la entidad de superficie de un extremo, abra la pestaña
   `Condiciones de contorno` y marque **Electric potential**. Ajústelo a `0` V.
2. Seleccione la entidad de superficie del otro extremo, marque **Electric
   potential** y ajústelo a `0.01` V.

La corriente a través de la barra es

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Una barra sin diferencia de potencial no conduce corriente y da un campo
magnético exactamente nulo: es lo primero que hay que comprobar si el resultado
sale vacío.

### Paso 4 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. Siga el progreso en `Informe` -> `Archivo de registro del solver`.

El registro muestra la tarea de electrostática con sus variables y, a
continuación, la tarea de magnetostática: el número de elementos que conducen
corriente, el número de nodos y las estadísticas del campo magnético. Ambas se
ejecutan una vez.

### Paso 5 - examinar el resultado

- Pestaña `Resultados`: muestre **Magnetic field**. Es un vector en los nodos,
  por lo que la vista 3D ofrece su módulo y sus tres componentes.
- El campo **circula alrededor de la barra**, perpendicular a la corriente, en el
  sentido de la regla de la mano derecha.
- Es **nulo en el eje** de la barra y crece hacia la superficie, donde es máximo:
  en el centro de cada cara larga más que en las esquinas.
- Es **más débil cerca de los extremos** de la barra que en el centro. Es el
  recorrido abierto de la corriente de 1.4: cerca de un extremo falta la mitad de
  lo que sería un conductor más largo.

### Paso 6 - comprobar el orden de magnitud

Para un conductor redondo de radio `R`, el campo en la superficie es
`u0*I/(2*pi*R)`. Una barra cuadrada de lado `a` no es redonda, pero el centro de
una cara, a una distancia `a/2` del eje, se le aproxima: con `a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

El campo calculado en el centro de una cara larga, a mitad de la barra, es de
unos `0.020 T`, alrededor de un 15 % menos: la sección cuadrada sitúa gran parte
de la corriente más lejos del punto que una redonda. En una esquina, a
`a/sqrt(2)` del eje, es aún menor, unos `0.019 T`.

Una comprobación más precisa necesita un punto de cálculo alejado de la barra,
donde la sección ya no importe. Añada una región de aire como se describe en 2.8
y lea el campo a `r = 0.02 m` del eje, a mitad de la barra. Para un conductor
recto finito de longitud `L`,

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

y el valor calculado debería coincidir con un margen de aproximadamente
`0.1 %`, debido a la sección cuadrada de la barra. Este valor es el campo de la
barra sola, como explica 1.4: un circuito real añadiría el campo de sus cables.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| El campo magnético es nulo en todas partes y el registro avisa de que ningún elemento conduce corriente | no hay densidad de corriente: compruebe que la tarea de electrostática se ejecutó primero, que los potenciales son distintos y que el material tiene conductividad eléctrica |
| **Magneto-statics** no se puede desmarcar por sí solo | es lo esperado: requiere *Electro-statics*, y al desmarcar esta se desmarcan ambas |
| Falta el grupo *Solver temporal* | es lo esperado: ninguno de los dos tipos de problema tiene solver temporal (véase 1.10) |
| El campo es más débil en los extremos de un conductor | es lo esperado: el recorrido de la corriente está abierto (véase 1.4) |
| El campo lejos del conductor no corresponde al circuito | los cables y el resto del circuito no están en el modelo (véase 1.4) |
| El campo en un conductor de línea o de superficie delgada es errático | es lo esperado: el campo de un conductor de sección nula es singular allí (véase 1.3) |
| El cálculo tarda mucho | el coste es el número de nodos por el número de elementos que conducen corriente (véase 1.7) |

---

## 4. Comprobación de un modelo

**Conductor recto.** Para un conductor largo de radio `R` por el que circula
uniformemente una corriente total `I`, el campo es circunferencial y

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(`outside` = fuera, `inside` = dentro), con `u0 = 1.2566e-6`. El campo es máximo
en la superficie del conductor, `B = u0*I/(2*pi*R)`, y disminuye como `1/r` más
allá. Para una corriente de `1 A` en un conductor de `5 mm` de radio, ese máximo
es de `4e-5 T`, aproximadamente el orden del campo terrestre.

**Conductor finito.** Para un conductor recto de longitud `L`, a una distancia
`r` de su eje y a mitad de su longitud,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

que tiende a la fórmula del conductor largo a medida que crece `L`. Lejos del
conductor la forma de la sección deja de importar, por lo que cualquier sección
puede comprobarse con esta fórmula.

**Hilo.** Una entidad de línea con sección conduce su corriente a lo largo de los
segmentos, y el solver utiliza para cada uno de ellos la forma cerrada de un hilo
recto. Por tanto, una entidad de línea recta reproduce exactamente, salvo
redondeo, la fórmula del conductor finito a cualquier distancia.

**Solenoide.** Para un solenoide largo de `n` espiras por metro por el que
circula `I`, el campo en su interior es uniforme y dirigido según el eje,

```
B = u0 * n * I
```

y casi nulo en el exterior. Un devanado puede modelarse como una entidad de línea
que sigue las espiras, con la sección del hilo, y la tarea de electrostática hace
circular la corriente por él a partir de los potenciales de sus dos extremos.

**Divergencia.** Para cualquier campo magnético, `div(B) = 0` en todas partes. La
ley de Biot-Savart la cumple exactamente, por lo que no pone a prueba este
solver; solo pondría a prueba la interpolación entre los nodos.

**Convergencia con la malla.** La integración en sí es exacta (1.3); lo que
depende de la malla es la densidad de corriente, que la tarea de electrostática
calcula constante por elemento. Refinar la malla en el conductor y comparar el
campo en el mismo punto es la comprobación de esa parte.

---

## 5. Limitaciones

### Limitaciones de modelado

- **Solo el vacío.** La formulación no incluye ninguna permeabilidad. No hay
  propiedad de material de permeabilidad relativa, ni distinción entre `B` y `H`,
  ni magnetización, ni saturación, por lo que no se pueden representar en absoluto
  el hierro, las ferritas ni los blindajes. Véase 1.9 para saber por qué esto
  requiere otro solver.
- **Solo el campo de la corriente modelada.** No se incluye el campo de los cables
  de alimentación ni del resto del circuito. Cuando el recorrido de la corriente
  en el modelo está abierto, el campo lejos del conductor no es el de un circuito
  real (véase 1.4).
- **Sin imanes permanentes ni campos impuestos.** La única fuente es una
  corriente de conducción calculada por una tarea de electrostática. Un imán, un
  campo de fondo o una excitación externa no tienen representación.
- **No se puede indicar directamente una corriente.** La corriente la produce una
  resolución electrostática sobre un cuerpo conductor. Una bobina puede modelarse
  como una entidad de línea que sigue el devanado, pero su corriente se sigue
  fijando mediante potenciales.
- **Los conductores delgados son singulares.** El campo en un nodo de una entidad
  de línea o en el borde de una entidad de superficie es un valor regularizado, no
  físico (véase 1.3).
- **Tipos de elemento.** Los tetraedros, triángulos, cuadriláteros y segmentos de
  dos nodos conducen corriente; los demás tipos de elemento no aportan nada.
- **Coste.** El trabajo crece como el número de nodos por el número de elementos
  que conducen corriente (véase 1.7).
- **Sin resultados derivados.** `process()` no calcula nada, por lo que el vector
  del campo magnético es toda la salida. No hay flujo, ni energía, ni fuerza, ni
  módulo del campo almacenado como variable propia.
- **Nada utiliza el resultado.** Ningún otro solver lee el campo magnético. No hay
  fuerza de Lorentz sobre el conductor, ni corriente inducida, ni acoplamiento de
  vuelta a la física electrostática, térmica o estructural.
- **Solo estático.** No hay solver temporal, y añadir una tarea de magnetostática
  (que lleva consigo una tarea de electrostática) elimina el solver temporal de
  todo el flujo de tareas, por lo que cualquier otra física encadenada tras ella
  también es estacionaria.
