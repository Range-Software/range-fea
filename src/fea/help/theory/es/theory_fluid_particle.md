# Dispersión de contaminantes - Manual de teoría y guía del usuario

Este documento describe el solver de dispersión de contaminantes de Range FEA -
`RSolverFluidParticle`, que atiende el tipo de problema **Dispersión de
contaminantes** (*Contaminant dispersion*): la ecuación que resuelve, el
significado de cada dato de entrada que acepta, las partes de la interfaz
gráfica que lo controlan y un tutorial completo.

La dispersión de contaminantes se resuelve sobre el campo de flujo de una tarea
**Flujo viscoso incompresible** (*Incompressible viscous flow*). El solver de
flujo, sus condiciones de contorno, su iteración no lineal y su avance en el
tiempo se describen en el manual [Flujo de fluidos](theory_fluid.md), en el que
se basa este.

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - flujo transitorio con dispersión de contaminantes](#3-tutorial---flujo-transitorio-con-dispersión-de-contaminantes)
4. [Comprobación de un modelo](#4-comprobación-de-un-modelo)
5. [Limitaciones](#5-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnita principal - la concentración de partículas

La incógnita principal es la **concentración de partículas** `C` en `kg/m^3`,
un grado de libertad por nodo.

| Tipo de problema | Solver | Incógnita principal | Unidades | Grados de libertad por nodo |
|---|---|---|---|---|
| Contaminant dispersion | `RSolverFluidParticle` | concentración de partículas | `kg/m^3` | 1 |

Este tipo de problema **requiere** una tarea *Incompressible viscous flow* en el
mismo problema: lee el campo de velocidad de su resultado y nunca lo calcula por
sí mismo.

### 1.2 Ecuación fundamental

**Contaminant dispersion** transporta una concentración escalar sobre el campo
de flujo,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

(`for s > 0 and a maximum saturation set` = para `s > 0` y una saturación
máxima definida; `otherwise` = en otro caso), con estabilización SUPG y el
esquema temporal theta del solver de flujo, leyendo la velocidad del resultado
del flujo.

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `C` | concentración de partículas | `kg/m^3` | incógnita resuelta |
| `v` | velocidad | `m/s` | resultado del flujo |
| `D` | coeficiente de difusión | `m^2/s` | *Configuración de la dispersión de contaminantes*, `0` por defecto |
| `C_sat` | saturación máxima | `kg/m^3` | *Configuración de la dispersión de contaminantes*, `0` (ilimitada) por defecto |
| `s` | tasa de fuente de partículas | `kg/(m^3*s)` | condición Particle rate |

### 1.3 Configuración de la dispersión de contaminantes

Los dos parámetros del tipo de problema se definen una vez por modelo en el
grupo *Configuración de la dispersión de contaminantes* de la pestaña `Problema`
(sección 2.2). Ambos valen cero por defecto, lo que reproduce una advección pura
con una fuente ilimitada.

#### Coeficiente de difusión

El **Coeficiente de difusión** `D` dispersa el contaminante en el sentido
descendente de su gradiente de concentración. Con `D = 0` el transporte es
puramente advectivo: el contaminante es arrastrado por el flujo y solo se
dispersa por la difusión numérica de la estabilización, de modo que el solver
responde a «adónde lo lleva el flujo y cuánto tarda», no a «qué anchura tiene la
pluma». Un `D` positivo convierte la anchura de la pluma en un resultado físico.

No hay modelo de turbulencia (sección 5), por lo que `D` es una difusividad
**efectiva**: la mezcla de un flujo real está dominada por la turbulencia, que se
puede estimar como `D ~ nu_t / Sc_t` con el número de Schmidt turbulento
`Sc_t ~ 0.7`.

| Difusión molecular (la propiedad física) | `D [m^2/s]` |
|---|---|
| gases y vapores en el aire (vapor de agua `2.5e-5`, CO2 `1.6e-5`, vapores de disolventes unos `1e-5`) | `1e-6` - `1e-4` |
| especies disueltas en agua (O2 `2e-9`, sales unos `1.5e-9`) | `1e-10` - `1e-8` |
| partículas de aerosol en el aire, movimiento browniano (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

A escalas de ingeniería, la difusión molecular es casi siempre despreciable
frente al transporte por el flujo. El valor que normalmente se introduce es un
valor efectivo:

| Difusión efectiva (turbulenta) | `D_eff [m^2/s]` |
|---|---|
| sala ventilada, mezcla del aire interior | `1e-3` - `1e-2` |
| flujo en conducto o canal, aproximadamente `D ~ 0.01 - 0.05 * U * L` (`U` velocidad media, `L` tamaño del conducto) | `1e-3` - `1e-1` |
| ríos, mezcla transversal | `1e-2` - `1` |
| atmósfera, mezcla horizontal | `1` - `100` |

Que un valor importe o no depende de la malla. Con la velocidad media `U`, la
longitud del flujo `L`, el tamaño de elemento `h` y el paso de tiempo `dt`:

- **cuándo `D` tiene algún efecto visible**: el contaminante se ensancha unos
  `sqrt( 2 * D * L / U )` mientras atraviesa el dominio. Si eso es menor que `h`,
  `D` no hace nada, de modo que `D` solo importa por encima de aproximadamente
  `U * h^2 / ( 2 * L )`. Para un canal con `U ~ 1 m/s`, `L ~ 15 m` y
  `h ~ 0.1 m`, eso son unos `3e-4 m^2/s`;
- **cuándo domina `D`**: por encima de unos `U * L` (unos `15 m^2/s` para el
  mismo canal), la difusión lo difumina todo y el flujo apenas importa;
- **paso de tiempo**: si `D * dt / h^2` supera aproximadamente `1`, cambie a
  *Backward difference (stable)*; el esquema de diferencias centradas no
  amortigua los modos difusivos más rápidos y oscila.

#### Saturación máxima

La **Saturación máxima** `C_sat` es la mayor concentración que puede alcanzar el
contaminante en el fluido: una concentración de vapor saturado o una
solubilidad. Cuando está definida, un *Particle rate* positivo se debilita a
medida que la concentración se acerca a ella y se anula en la saturación,
`s * ( 1 - C / C_sat )`; esto modela la evaporación o la disolución, y `s` es
entonces la tasa en un fluido limpio, aproximadamente `k_m * C_sat * A / V`. Los
sumideros (`s < 0`) no se limitan. El límite solo actúa sobre la fuente: una
concentración impuesta por una condición de contorno o inicial no se limita, y el
registro del solver avisa cuando supera `C_sat`. La concentración tampoco se
recorta, por lo que sigue siendo posible un ligero rebasamiento junto a un frente
abrupto.

Solo existe un límite de saturación donde hay uno termodinámico: una presión de
vapor o una solubilidad. Para un vapor en el aire, la concentración de saturación
se deduce de la presión de vapor de saturación, `C_sat = p_sat * M / ( R * T )`:

| Vapor en el aire | Temperatura | `C_sat [kg/m^3]` |
|---|---|---|
| vapor de agua | 0 °C | `0.0048` |
| vapor de agua | 20 °C | `0.017` |
| vapor de agua | 40 °C | `0.051` |
| vapor de agua | 100 °C | `0.60` |
| etanol o tolueno | 20 °C | unos `0.11` |
| metanol | 20 °C | unos `0.17` |
| benceno | 20 °C | unos `0.32` |
| acetona | 20 °C | unos `0.59` |

Para una sustancia disuelta en agua, el límite es su solubilidad:

| Disuelto en agua | `C_sat [kg/m^3]` |
|---|---|
| oxígeno en equilibrio con el aire, 20 °C | `0.009` |
| dióxido de carbono a 1 atm | `1.7` |
| yeso | `2.4` |
| cloruro de sodio | unos `360` |

Las partículas, el polvo y el humo no tienen saturación; deje el valor en `0`
(ilimitada). Solo como orden de magnitud, los límites de exposición laboral son
de unos `1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) y los límites inferiores de
explosividad del polvo de unos `0.02` - `0.06 kg/m^3`.
La saturación depende fuertemente de la temperatura, mientras que el solver
utiliza un único valor constante: para un caso no isotérmico, tome el valor a la
temperatura relevante más fría, donde empezaría la condensación.

### 1.4 Estabilización

El parámetro SUPG del solver de dispersión está limitado por el paso de tiempo e
incluye la difusión,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

suprimiéndose el término en `dt` en un análisis estacionario. Se reduce a
`h / ( 2 * |v| )` donde el flujo es rápido, nunca supera `dt / 2` y tiende a
`h^2 / ( 12 * D )` (el límite del parámetro unidimensional exacto en los nodos)
donde domina la difusión. El límite del paso de tiempo importa en las zonas
lentas y de recirculación: allí el número de Courant del elemento
`|v| * dt / h` es diminuto, y un `tau` sin límite convertiría la estabilización
en un ruido que crea concentración por delante del frente. Este parámetro es
propio del solver de dispersión; el solver de flujo utiliza el descrito en el
manual [Flujo de fluidos](theory_fluid.md).

El término de advección de Galerkin se ensambla en **forma advectiva con un
sumidero de divergencia**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, con la
velocidad nodal continua. Una velocidad procedente del solver de flujo nunca
tiene divergencia exactamente nula: en una malla gruesa es habitual un `div(v)`
de decenas de `1/s` junto a paredes y esquinas. Donde el flujo se expande
(`div(v) > 0`), la advección simple crearía concentración por sí misma, un patrón
de tablero de ajedrez que crece de un paso a otro en cuanto el paso de tiempo es
pequeño; el sumidero lo anula. Donde el flujo converge no se añade ningún
término: una corrección simétrica `0.5 * div(v) * C` actuaría allí como fuente y
acumularía el contaminante junto a las paredes. Para una velocidad con
divergencia exactamente nula, el sumidero es cero.

Con un coeficiente de difusión despreciable, el transporte es casi una advección
pura, que Galerkin con SUPG no mantiene monótona: cabe esperar rebasamientos por
exceso y por defecto, mayores en las paredes con adherencia, donde la velocidad
es nula. Un coeficiente de difusión efectivo (sección 1.3) elimina la mayoría.

A diferencia del solver de flujo, el solver de dispersión aplica la ponderación
theta de forma coherente, por lo que *Central difference (accurate)* es aquí un
auténtico esquema de Crank-Nicolson.

### 1.5 Condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Particle concentration | explícita (Dirichlet) | punto, línea, superficie | Particle concentration `[kg/m^3]` |
| Particle rate | natural, fuente | punto, línea, superficie, volumen | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | natural (Neumann) | superficie | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | natural (Robin) | superficie | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** (concentración de partículas) mantiene sus nodos en
la concentración dada. Asignada a una superficie de entrada con una tabla en
función del tiempo, es la forma de modelar una emisión: cero antes del suceso, la
concentración de emisión durante el mismo y cero después. Es el recurso más útil
de este tipo de problema, y en él se basa el tutorial de la sección 3.

**Particle rate** (tasa de partículas) es una fuente y es una **densidad** en
`kg/(m^3*s)`, no un total: a diferencia de la condición *Heat*, no se reparte
sobre la medida de la entidad. Modela una fuente que emite de forma continua
dentro del dominio.

**Particle flux (unit area)** (flujo de partículas por unidad de área) impone el
flujo másico `q` que entra en el dominio a través de una superficie,
`D * grad(C) . n = q`, siendo `n` la normal exterior. A diferencia de *Particle
rate*, es un flujo por unidad de área, por lo que la emisión total es `q * A`. Un
`q` positivo es una emisión (una película que se evapora, una pared que
desgasifica) y está limitado por la saturación máxima del mismo modo que una
tasa de partículas positiva, `q * ( 1 - C / C_sat )`. Un `q` negativo es una
tasa de extracción fija, independiente de la concentración local, que puede
llevarla por debajo de cero; una deposición proporcional a la concentración
corresponde a *Particle mass transfer* con `C_inf = 0`. En una pared, el flujo
entra solo por difusión; en una parte del contorno por la que entra el flujo, el
fluido entrante se trata como fluido limpio que transporta el flujo, de modo que
`q` pasa a ser el flujo másico entrante total y `C ~ q / |v . n|`. Asignada a una
superficie interna entre dos volúmenes, emite `q` por unidad de área como una
fuente laminar.

**Particle mass transfer** (transferencia de masa de partículas) intercambia
contaminante con un depósito ambiente, `q = k_m * ( C_inf - C )`, donde `k_m` es
el *Mass transfer coefficient* (coeficiente de transferencia de masa) y `C_inf`
la *Ambient particle concentration* (concentración ambiente de partículas). Es el
equivalente, para el contaminante, de la convección simple en la transferencia
de calor. Con `C_inf = 0` modela la deposición sobre una superficie con
velocidad de deposición `k_m`; con `C_inf > 0`, un intercambio con el aire
ambiente a través de una abertura o una superficie que se disuelve mantenida en
`C_inf`. Los valores típicos de `k_m` son `1e-4` - `1e-2 m/s` para la deposición
de partículas en salas y `1e-3` - `1e-1 m/s` para la evaporación en un gas en
movimiento. Un `k_m` grande lleva la superficie hacia `C_inf` y se aproxima a una
condición *Particle concentration*.

Cada componente es una **tabla en función del tiempo**, por lo que una emisión
puede empezar y terminar durante un cálculo transitorio.

**Contornos sin condición.** Las paredes y salidas sin condición *Particle
concentration*, *Particle flux* o *Particle mass transfer* mantienen la
condición natural de **flujo difusivo nulo**: el contaminante no se difunde a
través de una pared y solo sale por una salida con el flujo.

Donde el flujo **entra** por un contorno de este tipo (típicamente un reflujo en
una parte de una salida *Pressure (implicit)*), se trata como fluido limpio: el
solver añade el término débil de entrada `max(-v . n, 0) * C` en esa parte de la
superficie, repartido sobre la cara de modo que amortigüe también los nodos del
borde donde la salida se une a una pared con adherencia.
Sin él, el contorno alimentaría la solución y la concentración en la salida
crecería sin límite. Si el fluido que vuelve a entrar no está limpio, prolongue
el dominio para que la salida solo vea flujo saliente.

Las condiciones del flujo (paredes, entradas y salidas) pertenecen a la tarea de
flujo y se describen en el manual [Flujo de fluidos](theory_fluid.md).

### 1.6 Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Particle concentration | campo de concentración inicial |

Las condiciones iniciales solo se leen **en el primer cálculo**; después, el
campo avanza a partir del resultado anterior. Una condición de contorno
prevalece sobre una condición inicial allí donde coinciden.

### 1.7 ¿Estacionario o transitorio?

La dispersión de contaminantes es ante todo un problema **transitorio**: un
problema de transporte en el tiempo, sin una respuesta estacionaria significativa
para un suceso de emisión. Una dispersión estacionaria responde a otra pregunta
(el campo de concentración al que tiende una fuente continua) y consiste en una
única resolución lineal sobre el campo de flujo (véase la sección 2.6 sobre su
solver matricial).

El procedimiento en dos etapas del tutorial de la sección 3 es el canónico:
converger el flujo en estacionario y reiniciar después con el solver temporal y
la tarea de dispersión activados.

### 1.8 Resultados

| Resultado | Se aplica a | Significado |
|---|---|---|
| Particle concentration `[kg/m^3]` | nodo | el campo de concentración resuelto |
| Relative saturation `[-]` | nodo | `C / C_sat`, solo cuando se define una saturación máxima |

La saturación relativa se muestra por defecto de `0` a `1` y se elimina cuando la
saturación máxima vuelve a ponerse a cero.

Para este tipo de problema solo se requiere la **densidad** del material.

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Material`, `Resultados`),
el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Coloque una tarea **Contaminant dispersion** en el
mismo grupo de tareas que una tarea **Incompressible viscous flow**: la
dispersión sobre el flujo, la combinación transitoria clásica. Este tipo de
problema requiere que haya una tarea de flujo.

Los valores del grupo de tareas, **N.º de iteraciones** y **Convergencia**,
controlan la iteración no lineal del flujo y se describen en el manual
[Flujo de fluidos](theory_fluid.md). En un cálculo transitorio son las
iteraciones *por paso de tiempo*.

### 2.2 Pestaña Problema

**Solver temporal**: déjelo desactivado para una dispersión estacionaria y
actívelo para un suceso de emisión.

| Campo | Significado |
|---|---|
| Activar | activa y desactiva el avance transitorio en el tiempo |
| Aproximación | theta del esquema temporal: regresivo, centrado o progresivo |
| Tiempo inicial | primer valor de tiempo en segundos |
| Tiempo final | `start time + time-step size * number of time-steps` (tiempo inicial + tamaño del paso de tiempo × número de pasos de tiempo), solo lectura |
| Tamaño del paso de tiempo | `dt` en segundos |
| Número de pasos de tiempo | cuántos pasos se calculan |
| Frecuencia de salida | escribe un archivo de resultados cada N pasos; `0` solo escribe el último paso |

**Configuración de la dispersión de contaminantes**: se muestra en cuanto hay una
tarea *Contaminant dispersion* en el flujo de tareas. Contiene los dos
parámetros de la sección 1.3:

| Campo | Significado |
|---|---|
| Saturación máxima `[kg/m^3]` | mayor concentración posible; una tasa de partículas positiva se detiene en ella y se calcula *Relative saturation*. `0` significa ilimitada |
| Coeficiente de difusión `[m^2/s]` | difusividad efectiva del contaminante en el fluido. `0` significa advección pura |

Los valores típicos de ambos se indican en la sección 1.3.

### 2.3 Pestaña Material

| Propiedad | Unidades | Comentario |
|---|---|---|
| Density | `kg/m^3` | `rho`, unos `1000` para el agua, `1.2` para el aire |

La tarea de flujo del mismo problema necesita además la viscosidad dinámica. La
base de datos de materiales suministrada incluye **Water** (agua) y **Air**
(aire), lo que normalmente completa este paso.

### 2.4 Pestaña Condiciones de contorno

Las condiciones de la sección 1.5 aparecen en cuanto hay una tarea *Contaminant
dispersion* en el flujo de tareas. Seleccione primero una entidad en el árbol
`Modelo`; el gestor ofrece entonces las condiciones válidas para ese tipo de
entidad. Marque una para asignarla y luego edite sus componentes en el árbol
inferior.

Cada componente es una tabla en función del tiempo. Haga clic en **Editar
valores dependientes del tiempo** para abrir el editor de componentes, donde se
introducen los valores en función del tiempo; entre dos instantes dados, el valor
se interpola linealmente, y después del último instante se conserva el último
valor. Así se modela un suceso de emisión.

### 2.5 Condiciones iniciales

La pestaña `Condiciones iniciales` ofrece *Particle concentration*. Define el
campo de partida del primer cálculo.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver de dispersión utiliza la entrada **GMRES**, ya que
su matriz no es simétrica.

Una **dispersión de contaminantes estacionaria con difusión** requiere aquí
atención. Es una única resolución lineal, por lo que no hay iteraciones del grupo
de tareas en las que apoyarse, y un sistema dominado por la difusión converge
lentamente con el límite predeterminado de GMRES de 10 iteraciones internas por
10 externas. El solver matricial no avisa cuando alcanza su límite: lea su tabla
de iteraciones en el registro del solver y, si llega a la última iteración
externa con el residuo todavía muy por encima del valor de convergencia del
solver, aumente el número de iteraciones externas. Una resolución no convergida
da una respuesta errónea sin ninguna otra señal.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra: *Particle
concentration*, *Particle rate* o *Relative saturation*. En un cálculo
transitorio, el historial se muestra con `Informe` -> `Puntos de monitorización`.

Un punto de monitorización en el lugar de interés da la respuesta en la forma que
normalmente se desea: la concentración en función del tiempo en un lugar dado,
leída en un gráfico en lugar de en una secuencia de imágenes.

### 2.8 Resultados y registros

La pestaña `Resultados` muestra las variables calculadas y controla la vista 3D.
La concentración de partículas y la saturación relativa son escalares en los
nodos.

La pestaña `Registros` del panel `Modelo` muestra los registros de resultados,
uno por cada paso de tiempo escrito de un cálculo transitorio. Al animar los
registros se reproduce la dispersión, y el botón rojo de grabación situado bajo
el árbol escribe la animación en un archivo de vídeo.

---

## 3. Tutorial - flujo transitorio con dispersión de contaminantes

**Objetivo.** Emitir un contaminante en un flujo convergido y seguir la pluma a
lo largo de un canal en el tiempo.

Es el procedimiento en dos etapas de la sección 1.7 y reproduce el tutorial
suministrado *Dispersión de contaminantes en fluidos*.

### Paso 1 - partir de un flujo convergido

Haga converger primero el tutorial del canal estacionario del manual
[Flujo de fluidos](theory_fluid.md). Un cálculo de dispersión iniciado desde el
reposo malgasta sus primeros pasos de tiempo desarrollando el campo de flujo en
lugar de transportar nada.

### Paso 2 - añadir la tarea de dispersión

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Conserve el grupo de tareas y la tarea **Incompressible viscous flow**.
3. Añada una tarea **Contaminant dispersion** dentro del mismo grupo.
4. Reduzca el **N.º de iteraciones** del grupo a un valor como `20`: ahora son
   las iteraciones no lineales *por paso de tiempo*, no las iteraciones de una
   resolución estacionaria. No modifique el valor de **Convergencia**; la
   mayoría de los pasos se estabilizarán antes de la vigésima pasada y
   terminarán antes, y los que no lo hagan son precisamente los que merecen las
   veinte.

*Contaminant dispersion* requiere que haya una tarea de flujo; lee el campo de
velocidad y nunca lo calcula.

### Paso 3 - configurar el solver temporal

Abra la pestaña `Problema` y, en el grupo *Solver temporal*, configure:

| Campo | Valor |
|---|---|
| Activar | marcado |
| Aproximación | `Backward difference (stable)` |
| Tiempo inicial | `0` |
| Tamaño del paso de tiempo | según la regla de Courant indicada abajo |
| Número de pasos de tiempo | suficientes para que la pluma atraviese el dominio |
| Frecuencia de salida | `1` para una animación fluida, más para mantener pequeño el archivo |

Elija el paso de tiempo a partir de la malla y de la velocidad del flujo, de
modo que el fluido atraviese aproximadamente un elemento por paso:

```
dt ~ h / |v|
```

Elija después el número de pasos a partir del tiempo que necesita la pluma para
recorrer el dominio, `L / |v|`, dividido por ese paso.

En el grupo *Configuración de la dispersión de contaminantes* de la misma
pestaña, deje ambos valores en `0` para una advección pura, o introduzca un
**Coeficiente de difusión** efectivo de la tabla de la sección 1.3 si importa la
anchura de la pluma. Una **Saturación máxima** solo tiene sentido para un vapor o
una sustancia disuelta; no debe ser inferior a la concentración de emisión del
paso 4.

### Paso 4 - emitir el contaminante

1. Seleccione la entidad de superficie de entrada.
2. Abra la pestaña `Condiciones de contorno` y marque **Particle
   concentration**.
3. En lugar de introducir un único valor, haga clic en **Editar valores
   dependientes del tiempo**.
4. En el editor de componentes, introduzca un perfil de emisión: por ejemplo `0`
   en el tiempo `0`, la concentración de emisión en el momento en que empieza la
   emisión y `0` de nuevo cuando termina.

Entre dos instantes dados, el valor se interpola linealmente, y después del
último instante se conserva el último valor, de modo que una emisión que deba
empezar bruscamente necesita dos entradas muy próximas. Esta condición
dependiente del tiempo es lo que convierte una entrada estacionaria en un suceso
de emisión, y es el núcleo de un modelo de dispersión.

### Paso 5 - conservar las condiciones del flujo

La pared **Wall**, la condición de entrada y la salida **Pressure (implicit)** se
mantienen sin cambios. El flujo se sigue resolviendo en cada paso de tiempo: la
dispersión se apoya en él.

### Paso 6 - añadir un punto de monitorización

`Problema` -> `Definir puntos de monitorización`, coloque un punto aguas abajo y
ajuste su variable a **Particle concentration**. El tiempo de llegada y la
concentración máxima en un lugar dado se leen mucho más fácilmente en un
historial que en una secuencia de imágenes coloreadas.

### Paso 7 - resolver con reinicio

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. Marque **Reiniciar el solver / continuar**.

El reinicio es lo que convierte el campo estacionario convergido en el punto de
partida del avance en el tiempo. Sin él, el cálculo parte del reposo y los
primeros pasos se desperdician.

### Paso 8 - examinar los resultados

- Panel `Modelo`, pestaña `Registros`: un registro por paso escrito. Recórralos o
  anímelos para ver cómo avanza la pluma.
- Pestaña `Resultados`: muestre **Particle concentration**. Fije el rango de
  visualización para todos los registros, de modo que los colores signifiquen lo
  mismo en cada fotograma; de lo contrario, la animación se reescala sola y la
  pluma parece no atenuarse.
- `Informe` -> `Puntos de monitorización` traza el historial de la concentración
  en la sonda. Lea en él el tiempo de llegada y compárelo con `L / |v|`, el tiempo
  de recorrido que implica el flujo medio: es la validación más barata de todo el
  modelo.
- Con un coeficiente de difusión nulo, la pluma solo se ensancha por difusión
  numérica, por lo que su **anchura** es una propiedad de la malla y del paso de
  tiempo, no de la física. Con un coeficiente de difusión efectivo, la anchura es
  un resultado; compruébela con una malla más fina antes de fiarse de ella.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La concentración nunca sale de la entrada | el campo de flujo es nulo: falta la tarea de flujo en el grupo, o el cálculo no se reinició desde un flujo convergido |
| La pluma se difumina casi inmediatamente | el número de Courant es muy superior a 1, o la malla es demasiado gruesa a lo largo del recorrido |
| La concentración se vuelve negativa o rebasa | el paso de tiempo es demasiado grande para la malla, y la advección estabilizada oscila. Pequeños rebasamientos por defecto junto a un frente abrupto son normales; la solución no se recorta en cero porque el recorte añadiría contaminante |
| Solo se escribió un registro | la frecuencia de salida es `0`, que solo escribe el último paso |
| El residuo de cada paso sigue alto | demasiado pocas iteraciones por paso de tiempo: el flujo no converge dentro de cada paso |
| La pluma nunca se ensancha lateralmente | el coeficiente de difusión es nulo, por lo que el ensanchamiento lateral solo procede del campo de flujo; introduzca un coeficiente de difusión efectivo (véase la sección 1.3) |
| La pluma se extiende por todo el dominio de golpe | el coeficiente de difusión es demasiado grande: compárelo con `U * L` |
| El registro avisa de que la concentración impuesta supera la saturación máxima | una condición de contorno o inicial está por encima de la saturación máxima; el límite solo actúa sobre la tasa de partículas, así que compruebe ambos valores |
| Una resolución estacionaria con difusión parece errónea | el solver matricial se detuvo en su límite de iteraciones: aumente las iteraciones externas de GMRES (véase la sección 2.6) |
| La concentración rebasa el valor de entrada o se vuelve negativa junto a las paredes, pero no sigue creciendo | advección casi pura: el coeficiente de difusión es casi nulo. Introduzca un coeficiente de difusión efectivo; en una malla gruesa, compruebe también que el campo de flujo ha convergido |
| La concentración supera el valor de entrada y crece formando un patrón ruidoso | el flujo no ha convergido: `Residual ratio` se mantiene cerca de `1` en cada paso. Su velocidad está lejos de tener divergencia nula y no se puede confiar en ningún transporte sobre ella; haga converger primero el flujo (malla, número de Reynolds, iteraciones por paso) |

---

## 4. Comprobación de un modelo

**Tiempo de recorrido.** El tiempo que tarda un trazador en recorrer una
distancia `L` a la velocidad media `V` es `L/V`. Lea el tiempo de llegada en el
historial de un punto de monitorización y compárelo. Así se valida que la tarea
de dispersión se apoya en el campo de flujo que usted cree, y se detecta un
cálculo que no se reinició desde un flujo convergido.

Es mejor comprobar primero el propio campo de flujo, con el balance de masa y
los flujos analíticos indicados en el manual [Flujo de fluidos](theory_fluid.md).
Un modelo de flujo no convergido falla todas las comprobaciones por el mismo
motivo, y no se puede confiar en ningún transporte sobre él.

---

## 5. Limitaciones

- **Sin modelo de turbulencia.** El flujo es laminar (véase el manual
  [Flujo de fluidos](theory_fluid.md)), por lo que la mezcla de un flujo
  turbulento debe introducirse como coeficiente de difusión efectivo.
- **Difusión constante e isótropa.** El coeficiente de difusión es un único valor
  para todo el modelo; no puede variar en el espacio ni en el tiempo, seguir la
  dirección del flujo ni proceder de un modelo de turbulencia. Debe estimarse
  como una difusividad efectiva (véase la sección 1.3).
- **Saturación máxima constante.** La saturación no sigue la temperatura de un
  resultado *Heat transfer in fluids* y solo limita una tasa de partículas o un
  flujo de partículas positivos. Las concentraciones impuestas por condiciones
  no se limitan y el campo no se recorta.
- **El acoplamiento con el flujo es unidireccional.** El flujo impulsa el
  transporte; el transporte nunca modifica el flujo.
- **No monótono para una advección casi pura.** Galerkin con SUPG produce
  rebasamientos por exceso y por defecto cuando el coeficiente de difusión es
  casi nulo, mayores en las paredes con adherencia (véase la sección 1.4).
