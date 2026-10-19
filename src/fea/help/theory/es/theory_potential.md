# Flujo a través de medios porosos - Manual teórico y guía del usuario

Este documento describe el solver de flujo a través de medios porosos
(`RSolverPotential`) de Range FEA, que atiende el tipo de problema **Flujo a través
de medios porosos** (*Flow through porous media*): las ecuaciones que resuelve, el
significado de cada entrada que acepta, las partes de la interfaz gráfica que lo
controlan y dos tutoriales desarrollados.

**Contenido**

1. [Fundamento teórico](#1-fundamento-teórico)
2. [Interfaz gráfica de usuario](#2-interfaz-gráfica-de-usuario)
3. [Tutorial - flujo impulsado por presión a través de un filtro de arena](#3-tutorial---flujo-impulsado-por-presión-a-través-de-un-filtro-de-arena)
4. [Tutorial - filtración ascendente en una columna vertical](#4-tutorial---filtración-ascendente-en-una-columna-vertical)
5. [Comprobación de un modelo](#5-comprobación-de-un-modelo)
6. [Limitaciones](#6-limitaciones)

---

## 1. Fundamento teórico

### 1.1 Incógnita principal - el campo de presión

La incógnita principal es la **presión** `p` del fluido intersticial en cada nodo,
en pascales. Cada nodo tiene un único grado de libertad, de modo que una malla con
`N` nodos da un sistema de `N` ecuaciones - tan económico como un modelo térmico o
electrostático de la misma malla. La velocidad del fluido se recupera del campo de
presión después de la solución.

Un campo de presión regido por esta ecuación solo está definido **salvo una
constante aditiva**: sumar el mismo valor a la presión en todas partes no cambia
ninguna velocidad. Por eso todo modelo necesita al menos una condición de contorno
*Pressure (explicit)*, véase 1.7. Que la presión se interprete como absoluta o
manométrica queda a elección del usuario - el solver solo trabaja con su gradiente.

### 1.2 Ecuaciones de gobierno

El solver implementa la **ley de Darcy** para el flujo lento de un único fluido
incompresible que llena los poros de un cuerpo poroso rígido:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

junto con la conservación de la masa de un fluido incompresible

```
div( v ) = 0
```

que combinadas dan una ecuación de potencial (de tipo Laplace) para la presión:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `p` | presión del fluido intersticial | `Pa` | se resuelve |
| `v` | velocidad de Darcy (superficial) | `m/s` | recuperada, véase 1.8 |
| `k` | permeabilidad intrínseca del cuerpo poroso | `m^2` | propiedad del material *Permeability to fluids* |
| `mu` | viscosidad dinámica del fluido intersticial | `kg/(m*s)` | propiedad del material *Dynamic viscosity* |
| `rho` | densidad del fluido intersticial | `kg/m^3` | propiedad del material *Density* |
| `g` | aceleración de la gravedad | `m/s^2` | condición de entorno, véase 1.6 |

El cociente `k/mu` desempeña el papel que tiene la conductividad en un modelo
térmico. Sin gravedad la ecuación es la ecuación de Laplace para la presión y el
flujo sigue directamente el gradiente de presión. Con gravedad, el término `rho*g`
hace que un fluido en reposo soporte un gradiente de presión hidrostático:
`grad(p) = rho*g` da `v = 0`.

**La velocidad es la velocidad de Darcy.** Es el caudal volumétrico por unidad de
sección **total**, sólido y poros juntos - no la velocidad del fluido dentro de los
poros. La velocidad media en los poros es mayor en la inversa de la porosidad `n`:

```
v_pore = v / n
```

La porosidad no interviene en un flujo estacionario, por lo que el solver ni la
necesita ni la acepta. Úsela a mano para estimar tiempos de tránsito.

**Permeabilidad y conductividad hidráulica.** La mecánica de suelos y la
hidrogeología suelen dar la *conductividad hidráulica* `K` en `m/s` (el
coeficiente de la ley de Darcy escrita para la altura piezométrica) en lugar de la
permeabilidad intrínseca. Ambas se relacionan mediante las propiedades del fluido:

```
k = K * mu / ( rho * |g| )
```

Para agua a 20 °C, `K = 1e-4 m/s` corresponde a `k = 1.02e-11 m^2`. La ingeniería
del petróleo da la permeabilidad en darcy: `1 D = 9.869e-13 m^2`.

**Propiedades de material obligatorias.** Un problema de flujo a través de medios
porosos requiere **tres** de ellas:

| Propiedad | Unidades | Uso |
|---|---|---|
| Permeability to fluids | `m^2` | la matriz de rigidez - permeabilidad del sólido |
| Dynamic viscosity | `kg/(m*s)` | la matriz de rigidez - viscosidad del **fluido** |
| Density | `kg/m^3` | el término gravitatorio - densidad del **fluido** |

**La densidad y la viscosidad son las del fluido intersticial, no las del cuerpo
poroso.** Un material para este tipo de problema describe por tanto el medio
*saturado*: la permeabilidad del esqueleto sólido y la densidad y viscosidad del
fluido que lo llena. Así están construidos los materiales incluidos con el nombre
*... (water saturated)* (véase 2.3). Un material que da la densidad aparente de un
suelo seco falsea la presión hidrostática en la proporción de ambas densidades.

Una entidad cuyo material carece de alguna de las tres **no se resuelve**. El
verificador de configuración lo advierte antes de iniciar el cálculo. La única
excepción es una entidad con una condición de contorno de este tipo de problema:
se incluye en el sistema sea cual sea su material, y su permeabilidad es entonces
simplemente cero. Así recibe un caudal de entrada una cara de contorno sin
material.

Cada propiedad es una tabla en función de la temperatura. Una tarea de flujo a
través de medios porosos por sí sola no tiene campo de temperatura, así que cada
tabla se evalúa a `293.15 K`. Las tablas solo se respetan cuando una tarea
*Heat transfer* anterior en el mismo flujo de tareas ha calculado temperaturas de
elementos - así fluye más rápido un fluido más caliente y menos viscoso.

### 1.3 Discretización por elementos finitos

La forma débil de la ecuación de presión da, para cada elemento,

```
K = INT( B^T * ( k / mu ) * B )                  rigidez
f = INT( B^T * ( k / mu ) * rho * g )            carga gravitatoria
  + INT_S( N^T * v_in )                          entrada a través del contorno
```

donde `B` se construye a partir de las derivadas de las funciones de forma, `N`
son las funciones de forma y `v_in` es la velocidad de entrada prescrita en una
superficie de contorno. Cada matriz de elemento es `n x n` para un elemento con
`n` nodos. No hay matriz de masa ni de almacenamiento: el flujo es estacionario,
véase 1.10.

**Medidas de los elementos.** Como en el resto de Range FEA, las entidades de
menor dimensión reciben una medida geométrica para poder representar cuerpos
reales:

| Entidad | Medida | Dónde se define |
|---|---|---|
| Volumen | 1 | - |
| Superficie | espesor de la superficie | propiedades geométricas de la entidad |
| Línea | sección de la línea | propiedades geométricas de la entidad |
| Punto | - | los puntos no conducen flujo |

Una **superficie** con espesor conduce el flujo en su propio plano, que es como se
construye un modelo bidimensional (una sección de una presa o de un acuífero). Su
carga gravitatoria usa la componente de `g` contenida en el plano de la superficie.
Una superficie de **espesor nulo** - la cara de contorno habitual de un modelo de
volumen - no tiene rigidez y solo recibe un caudal de entrada.

Una **línea** con sección conduce el flujo a lo largo de su propia dirección, para
un modelo unidimensional de una columna o de un tubo relleno. Un **punto** no
aporta nada. Un elemento punto cuyo nodo no pertenece a ningún otro elemento
calculable deja una fila vacía en la matriz - no asigne un material poroso a
entidades punto.

### 1.4 Condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Pressure (explicit) | explícita (Dirichlet) | superficie | Pressure `[Pa]` |
| Velocity (inflow) | natural (flujo prescrito) | superficie | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | natural (flujo prescrito) | superficie | Volumetric flow rate `[m^3/s]` |

Una superficie de contorno sin ninguna de ellas es **impermeable**: ningún fluido
la atraviesa. Es la condición natural de la ecuación, así que paredes, el fondo de
un acuífero y planos de simetría no necesitan ninguna condición.

**Pressure (explicit)** es la única condición explícita. Cada nodo de la superficie
se elimina del sistema de ecuaciones y se mantiene a la presión dada. Es donde el
fluido abandona el modelo (una salida, un dren, el agua libre de un embalse) o
entra en él a una presión conocida. Todo modelo necesita al menos una. Una
condición recién asignada empieza en `1000 Pa`.

**Velocity (inflow)** prescribe la velocidad de Darcy normal a través de la
superficie. Un valor **positivo** es una entrada al cuerpo poroso, uno **negativo**
una salida (extracción). El valor es un flujo por unidad de área, así que el caudal
total crece con el área de la superficie.

**Volumetric flow rate (inflow)** prescribe el caudal **total** a través de la
superficie en `m^3/s`. El solver lo divide por el área de toda la entidad y aplica
el resultado como velocidad de entrada uniforme, de modo que el caudal se mantiene
al volver a mallar o al ampliar la superficie. Positivo vuelve a ser entrada.

Ninguna de las condiciones de entrada fija la presión: la presión en la entrada
forma parte de la respuesta. Donde una superficie de entrada y una superficie con
*Pressure (explicit)* comparten nodos, en esos nodos gana la presión.

Una superficie de entrada **no necesita material ni espesor**: la condición por sí
sola la incorpora al sistema. Si una superficie tiene un material poroso y un
espesor, también conduce flujo en su propio plano.

La condición *Pressure (implicit)* del solver de flujo incompresible no se ofrece
para este tipo de problema.

Todas las componentes aceptan una tabla en función del tiempo, que surte efecto
con el solver temporal activado, véase 1.10.

### 1.5 Gravedad

La gravedad entra mediante la condición de entorno *Gravitational acceleration*.
**Cada entidad la tiene por defecto con `g = (0, 0, -9.80665) m/s^2`**, de modo que
la gravedad apunta según el eje Z negativo salvo que se cambie. Conviene conocer
dos consecuencias:

- En un modelo cuyo eje Z apunta hacia arriba, un fluido en reposo recibe
  automáticamente una presión hidrostática que crece `rho*|g|` por metro de
  profundidad. Normalmente es lo deseado.
- Un modelo construido con otro eje hacia arriba debe tener la condición editada
  en cada entidad porosa; de lo contrario el fluido «cae» de lado. Ponga las tres
  componentes a cero para desactivar la gravedad - por ejemplo en un modelo
  horizontal en el que solo interesa la diferencia de presión.

La gravedad combinada con una superficie *Pressure (explicit)* que no es horizontal
requiere cuidado: una presión uniforme en una cara vertical **no** es
hidrostática, de modo que el fluido circula cerca de esa cara. Dé la presión en
función de la altura (divida la cara en varias entidades) o desactive la gravedad
cuando no forme parte de la pregunta.

### 1.6 Condiciones de entorno, iniciales y de acoplamiento

#### Condiciones de entorno

| Condición de entorno | Componentes | Efecto |
|---|---|---|
| Gravitational acceleration | g en X, Y, Z `[m/s^2]` | término gravitatorio de la ley de Darcy, véase 1.5 |
| Temperature | Temperature `[K]` | se ofrece, pero no se lee - véase abajo |

La condición de entorno *Temperature* se ofrece porque las propiedades del material
son tablas en función de la temperatura. El solver **no** la lee: toma las
temperaturas de los elementos solo de los datos compartidos que deja una tarea
*Heat transfer*, y en otro caso evalúa cada tabla a `293.15 K`.

#### Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Pressure | se ofrece, no se lee |
| Velocity | se ofrece, no se lee |

El flujo es estacionario, por lo que la respuesta no depende de un campo inicial y
el solver no lee ninguna de las dos condiciones iniciales. Ambas se ofrecen porque
el tipo de problema las comparte con el problema de flujo incompresible.

#### Acoplamiento con otros solvers

| Dirección | Variable | Efecto |
|---|---|---|
| Calor -> flujo a través de medios porosos | Temperature | selecciona la fila de cada tabla de material dependiente de la temperatura |

La presión y la velocidad no las consume ningún otro solver. El flujo a través de
medios porosos se combina con cualquier otra física excepto *Incompressible
viscous flow* y *Modal analysis*.

El solver se declara convergido incondicionalmente, así que un grupo de tareas que
solo contiene una tarea de flujo a través de medios porosos siempre se detiene
tras una iteración.

### 1.7 Resolución del sistema

El solver ensambla y resuelve

```
K * p = f
```

`K` es simétrica definida positiva en cuanto se prescribe al menos una presión, por
lo que se resuelve con el método del **gradiente conjugado** con precondicionamiento
de Jacobi. El ensamblaje está paralelizado por elementos con búferes por hilo que
se fusionan una vez al final, de modo que el resultado no depende del número de
hilos.

Las permeabilidades de los materiales reales van desde unos `1e-9 m^2` (grava)
hasta `1e-20 m^2` (arcilla intacta), lo que llevaría los coeficientes de la matriz
cerca de los límites de redondeo del solver iterativo. Por eso, antes de resolver,
cada ecuación se divide por el mayor `k/mu` del modelo. Dividir todas las
ecuaciones por el mismo número no cambia la solución; solo mantiene la iteración
bien condicionada.

Un modelo **sin condición de contorno Pressure (explicit)** no tiene solución
única. El solver lo detecta y se detiene con el error *Flow through porous media
requires at least one 'Pressure (explicit)' boundary condition*.

Otras formas de llegar a un sistema singular o vacío, todas silenciosas:

- una entidad cuyo material carece de una de las tres propiedades obligatorias no
  se resuelve, y sus nodos salen del sistema;
- una entidad con permeabilidad o viscosidad nula no tiene rigidez, así que sus
  nodos permanecen en el sistema pero no están conectados a nada;
- una superficie de espesor nulo o una línea de sección nula no conduce flujo.

### 1.8 Resultados derivados

Tras la solución, la velocidad de cada elemento se recupera del campo nodal de
presión. El gradiente de presión se promedia sobre los puntos de integración del
elemento y se combina con el término gravitatorio; en elementos de línea y de
superficie se gira a coordenadas globales:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Resultado | Se aplica a | Significado |
|---|---|---|
| Pressure `[Pa]` | nodo | el campo de presión intersticial resuelto |
| Velocity `[m/s]` | elemento | vector de velocidad de Darcy |

La velocidad es una **densidad** en el mismo sentido que los resultados de campo de
los demás solvers: no depende del tamaño del elemento, es constante dentro de un
elemento lineal y converge al refinar la malla. Las caras de contorno de espesor
nulo muestran velocidad cero - visualice el resultado sobre el volumen.

El caudal total no se guarda como resultado. Calcúlelo a mano a partir de la
velocidad y el área de una sección, o a partir de las condiciones de entrada,
véase 5.

Tras escribir el registro, el registro del solver imprime las estadísticas de
ambas variables y los valores en los puntos de monitorización.

### 1.9 Validez de la ley de Darcy

La ley de Darcy vale para flujo lento y laminar, en el que las fuerzas viscosas
dominan sobre la inercia del fluido. Un criterio habitual usa el número de Reynolds
formado con el diámetro medio de grano `d`:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

El flujo de agua subterránea en suelos, la filtración a través de presas y
terraplenes, el flujo a través de filtros, lechos empacados y la mayoría de los
materiales porosos técnicos lo cumplen con amplio margen. El flujo a través de
grava gruesa, escollera o espumas muy abiertas a velocidades altas no; la ley de
Darcy subestima entonces la pérdida de carga (régimen de Forchheimer), y este
solver dará una velocidad demasiado grande.

### 1.10 Flujo estacionario y el solver temporal

El tipo de problema admite el solver temporal, pero el flujo en sí es
**estacionario**: el fluido y el cuerpo poroso son incompresibles y no hay término
de almacenamiento. Con el solver temporal activado, cada paso de tiempo resuelve un
estado estacionario independiente con los valores de las condiciones en ese
instante. Es el modelo adecuado cuando las condiciones de contorno cambian despacio
comparado con el tiempo que la presión tarda en asentarse - un nivel de embalse que
varía a lo largo de horas sobre un filtro de arena - y permite que una tarea de
flujo a través de medios porosos comparta flujo de tareas con una tarea térmica
transitoria.

La consolidación, el drenaje de un suelo no saturado, el flujo de gas compresible y
cualquier otro fenómeno en el que la cantidad de fluido almacenada en los poros
cambie con el tiempo quedan fuera del modelo, véase la sección 6.

---

## 2. Interfaz gráfica de usuario

Todo lo relevante está en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Condiciones de entorno`,
`Material`, `Resultados`), en el menú **Problema** y en el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre el diálogo del
flujo de tareas. Añada una tarea y elija **Flow through porous media**.

El tipo de problema no se puede combinar con *Incompressible viscous flow* ni con
*Modal analysis*. Una tarea *Heat transfer* colocada antes hace que las tablas del
material sigan la temperatura calculada.

### 2.2 Pestaña Problema

La pestaña muestra el grupo *Solver temporal*, porque el tipo de problema admite
el solver temporal - qué significa aquí un cálculo transitorio lo explica 1.10. El
flujo a través de medios porosos no tiene grupo de configuración propio. Todo se
asigna por entidad en las pestañas de condiciones.

### 2.3 Pestaña Material

La pestaña `Material` muestra el material asignado a la entidad seleccionada. El
flujo a través de medios porosos requiere **las tres** propiedades siguientes:

| Propiedad | Unidades | Comentario |
|---|---|---|
| Permeability to fluids | `m^2` | permeabilidad intrínseca del sólido, véase la tabla siguiente |
| Dynamic viscosity | `kg/(m*s)` | del fluido intersticial, `1.003e-3` para agua a 20 °C |
| Density | `kg/m^3` | del fluido intersticial, `998.2` para agua a 20 °C |

La biblioteca incluida contiene materiales porosos saturados de agua listos para
usar. Cada uno lleva la densidad y la viscosidad del agua dependientes de la
temperatura y una permeabilidad típica:

| Material | Permeabilidad `[m^2]` | Conductividad hidráulica a 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

La permeabilidad de un suelo real puede diferir de estos valores en un orden de
magnitud o más - son puntos de partida, no datos de un emplazamiento concreto. Para
modelar otro fluido, copie uno de ellos y sustituya la densidad y la viscosidad. El
material simple *Sand* es un material granular seco sin viscosidad, por lo que se
marca como no válido para este tipo de problema.

### 2.4 Pestaña Condiciones de contorno

La pestaña se divide en una lista de condiciones de contorno disponibles arriba y
un editor de la seleccionada abajo. Seleccione primero una entidad de superficie en
el árbol `Modelo`; la lista ofrece entonces las condiciones de la sección 1.4.
Márquela para asignarla y edite sus componentes en el árbol inferior.

- **Pressure (explicit)** mantiene la presión en la salida. Un modelo sin ninguna
  no se resuelve.
- **Velocity (inflow)** y **Volumetric flow rate (inflow)** introducen un caudal
  conocido en el modelo. Los valores positivos entran, los negativos salen.
- Las caras sin condición son impermeables.

### 2.5 Condiciones iniciales y de entorno

La pestaña `Condiciones de entorno` ofrece *Gravitational acceleration*, que cada
entidad ya lleva con `(0, 0, -9.80665) m/s^2` (véase 1.5), y *Temperature*, que
este solver ignora.

La pestaña `Condiciones iniciales` ofrece *Pressure* y *Velocity*, que este solver
no lee.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver de flujo a través de medios porosos usa la entrada
**CG**. Si la solución no converge, busque primero una entidad con permeabilidad
nula o un elemento punto aislado antes que en la configuración del solver.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en coordenadas dadas
y selecciona la variable a registrar - *Pressure* o *Velocity*. `Informe` ->
`Puntos de monitorización` muestra los valores.

### 2.8 Resultados y registros

La pestaña `Resultados` lista las variables calculadas y controla la vista 3D. La
presión es un escalar nodal, la velocidad un vector de elemento. Visualice la
velocidad como campo vectorial sobre el volumen para ver las trayectorias del
flujo.

La pestaña `Registros` del panel `Modelo` lista los registros de resultados: uno
en un cálculo estacionario, uno por paso de tiempo de salida con el solver temporal
activado.

`Informe` -> `Archivo de registro del solver` muestra la salida completa del solver,
incluidas las estadísticas de ambas variables.

---

## 3. Tutorial - flujo impulsado por presión a través de un filtro de arena

**Objetivo.** Calcular el flujo de agua a través de un filtro de arena horizontal
impulsado por una diferencia de presión conocida y comprobar el caudal con la ley
de Darcy.

Este tutorial supone un modelo de volumen mallado de un prisma - use `Archivo` ->
`Nuevo modelo`, dibuje un prisma con `Geometría` -> `Crear elemento` o importe una
geometría y tetraedrícela; el tutorial *Draw cube* cubre este paso. Marque las dos
caras extremas como entidades de superficie separadas para poder asignarles
condiciones.

Tome el filtro de `1 m` de longitud según el eje X con una sección de `0.1 m` por
`0.1 m`.

### Paso 1 - seleccionar el problema

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada una tarea y seleccione **Flow through porous media**.
3. Confirme con `OK`.

Deje el *Solver temporal* desactivado - la respuesta es un único estado
estacionario.

### Paso 2 - asignar el material

1. Seleccione la entidad de volumen en el árbol `Modelo`.
2. Abra la pestaña `Material` y asigne **Coarse Sand (water saturated)**.

El material lleva la permeabilidad `1e-10 m^2` de la arena y la densidad y
viscosidad del agua.

### Paso 3 - desactivar la gravedad

El filtro es horizontal y solo interesa la diferencia de presión, así que elimine
la gravedad por defecto:

1. Con la entidad de volumen seleccionada, abra la pestaña
   `Condiciones de entorno`.
2. Seleccione **Gravitational acceleration** y ponga las tres componentes a `0`.

Con la gravedad activada, el agua soportaría además una presión hidrostática a lo
largo de los `0.1 m` de altura del filtro, y las presiones uniformes en las caras
extremas verticales la harían circular cerca de ellas - véase 1.5.

### Paso 4 - prescribir los dos extremos

1. Seleccione la entidad de superficie en `x = 0`.
2. Abra la pestaña `Condiciones de contorno`, marque **Pressure (explicit)** y
   ponga `10000` Pa - aproximadamente un metro de columna de agua.
3. Seleccione la entidad de superficie en `x = 1 m`, marque **Pressure
   (explicit)** y ponga `0` Pa.

Las cuatro caras laterales no reciben condición: son paredes impermeables.

### Paso 5 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. Siga el progreso en `Informe` -> `Archivo de registro del solver`.

El cálculo es una única solución por gradiente conjugado.

### Paso 6 - examinar los resultados

- Pestaña `Resultados`: visualice **Pressure**. Cae linealmente de `10000` Pa a
  `0` Pa a lo largo del filtro, con isobaras planas perpendiculares a su eje.
- Visualice **Velocity**. Apunta según +X y es igual en todos los elementos.
- Un punto de monitorización en *Pressure* a mitad del filtro marca `5000` Pa.

### Paso 7 - calcular los números a mano

Con `k = 1e-10 m^2` y `mu = 1.003e-3 kg/(m*s)` para agua a 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

El resultado **Velocity** debe mostrar `9.97e-4 m/s`. El número de Reynolds con un
tamaño de grano de `1 mm` es `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, en el límite
del régimen de Darcy (1.9). Con una porosidad de `0.35` el agua misma se mueve a
`9.97e-4 / 0.35 = 2.8e-3 m/s` y tarda unos seis minutos en atravesar el filtro.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | no hay presión de salida prescrita en ningún sitio |
| *Entity has material assigned which is missing required properties* | al material le falta la viscosidad - a menudo un material de suelo seco como *Sand* |
| La presión varía con la altura y la velocidad tiene componente Z | la gravedad por defecto sigue activada, véase el paso 3 |
| La velocidad es nula en todas partes | ambos extremos tienen la misma presión o la permeabilidad es nula |

---

## 4. Tutorial - filtración ascendente en una columna vertical

**Objetivo.** Empujar agua hacia arriba a través de una columna vertical de arena
con un caudal conocido y ver cómo la presión de entrada se reparte en una parte
hidrostática y una parte de fricción.

Use un prisma de `0.1 m` por `0.1 m` de base y `1 m` de altura según el eje **Z**,
con la cara inferior (`z = 0`) y la cara superior (`z = 1 m`) como entidades de
superficie separadas.

### Paso 1 - problema y material

1. Seleccione **Flow through porous media** como en el tutorial 3.
2. Asigne **Coarse Sand (water saturated)** al volumen.

### Paso 2 - mantener la gravedad

La columna está según Z, así que la gravedad por defecto `(0, 0, -9.80665) m/s^2`
apunta en la dirección correcta. Deje la condición *Gravitational acceleration*
como está.

### Paso 3 - condiciones de contorno

1. Seleccione la cara superior, marque **Pressure (explicit)** y ponga `0` Pa: el
   agua libre sobre la columna.
2. Seleccione la cara inferior, marque **Volumetric flow rate (inflow)** y ponga
   `1e-6` m^3/s. Sobre los `0.01 m^2` de la cara, eso es una velocidad de entrada
   de `1e-4 m/s`.

La cara superior es horizontal, así que una presión uniforme en ella es coherente
con la gravedad.

### Paso 4 - resolver y examinar

`Solución` -> `Iniciar solver` (`Ctrl+R`), luego visualice **Pressure** y
**Velocity**.

- La velocidad es `1e-4 m/s` hacia arriba (+Z) en todos los elementos.
- La presión crece linealmente desde `0` Pa arriba hasta su máximo abajo.

### Paso 5 - repartir la presión de entrada

La presión en el fondo tiene que levantar la columna de agua y empujarla a través
de la arena:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Un punto de monitorización en la cara inferior confirma la suma. Ponga el caudal a
cero y resuelva de nuevo: la velocidad desaparece y la presión del fondo baja a los
`9789 Pa` puramente hidrostáticos - el solver reproduce exactamente un fluido en
reposo.

Aumente el caudal hasta que la parte de fricción alcance el peso sumergido de la
arena, aproximadamente `(rho_s - rho) * g * (1 - n) * L`: es el inicio de la
fluidización (arena movediza), que el esqueleto rígido de este modelo no puede
representar - el solver simplemente sigue calculando una presión mayor.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La presión del fondo es solo la parte de fricción | la gravedad está desactivada en el volumen |
| La presión del fondo es solo la parte hidrostática | la entrada está en la cara equivocada o su valor es cero |
| La velocidad apunta de lado | la columna se construyó según otro eje que Z; edite la condición de gravedad |

---

## 5. Comprobación de un modelo

**Columna uniforme.** Para un cuerpo de longitud `L` mantenido a `p1` y `p2` en dos
caras opuestas, de un solo material y con la gravedad desactivada, la presión es
lineal a lo largo del cuerpo y

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

El resultado de velocidad debe coincidir con la tolerancia del solver, y la presión
en el plano medio debe ser la media de los dos valores prescritos.

**Hidrostática.** Un cuerpo con gravedad, con una condición *Pressure (explicit)*
solo en su cara superior horizontal y paredes impermeables en el resto, debe
mostrar velocidad nula y una presión que crece `rho*|g|` por metro de profundidad.
Esto comprueba a la vez la densidad y la condición de gravedad.

**Balance de caudal.** En un modelo estacionario, todo el fluido que entra debe
salir. Para una entrada prescrita con *Volumetric flow rate (inflow)*, la velocidad
en una sección por su área debe devolver el caudal prescrito. Lea la velocidad en
la columna con un punto de monitorización.

**Capas en serie.** Para dos capas de longitud `L1`, `L2` y permeabilidad `k1`,
`k2` en serie entre dos presiones prescritas, el flujo se resiste como dos
resistencias en serie:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          presión en la interfaz
```

A diferencia del solver electrostático, este reparte la presión según las
**permeabilidades**, como lo haría una red de resistencias.

**Flujo radial hacia un pozo.** Para un acuífero confinado de espesor `H` entre un
pozo de radio `rw` mantenido a `pw` y un radio exterior `R` mantenido a `pR`, la
presión es logarítmica y el caudal del pozo es

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Esto comprueba el solver en un campo no uniforme y muestra si la malla es
suficientemente fina cerca del pozo.

Refine la malla al menos una vez y confirme que la presión converge en lugar de
derivar.

---

## 6. Limitaciones

- **Flujo estacionario, incompresible y monofásico.** No hay término de
  almacenamiento, así que la consolidación, el flujo de gas compresible y la
  respuesta transitoria de un acuífero confinado no se modelan. Un cálculo
  transitorio es una secuencia de estados estacionarios, véase 1.10.
- **Solo medios totalmente saturados.** No hay superficie libre (línea freática) ni
  zona no saturada. La filtración a través de un terraplén cuyo nivel freático hay
  que hallar queda fuera del modelo, salvo que la región saturada se malle
  explícitamente.
- **Cuerpo poroso rígido.** El esqueleto no se deforma, así que no hay acoplamiento
  con el análisis de tensiones, ni tensión efectiva, ni consolidación, ni
  fluidización.
- **Permeabilidad isótropa.** La permeabilidad es un escalar. Los suelos
  estratificados con distinta permeabilidad horizontal y vertical deben modelarse
  como capas separadas.
- **Solo régimen de Darcy.** El flujo inercial (Forchheimer) y turbulento en
  medios muy gruesos no se modela, véase 1.9.
- **Condiciones de contorno solo en superficies.** Presiones y entradas se
  prescriben en entidades de superficie; no hay cara de rezume, ni pozo o fuente
  lineal, ni presión prescrita en una línea o un punto.
- **Sin resultado de caudal total.** Los caudales a través de una sección deben
  calcularse a mano a partir de la velocidad.
- **Las condiciones iniciales no se leen** y la condición de entorno *Temperature*
  no tiene efecto; las tablas del material siguen la temperatura solo a través de
  una tarea térmica anterior en el flujo de tareas.
- **Un modelo necesita una presión prescrita.** Sin ella el solver se detiene con
  un error.
