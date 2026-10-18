# Flujo de fluidos - Manual de teoría y guía del usuario

Este documento describe el solver de flujo de fluidos de Range FEA -
`RSolverFluid`, que atiende el tipo de problema **Flujo viscoso incompresible**
(*Incompressible viscous flow*): las ecuaciones que resuelve, el significado de
cada dato de entrada que acepta, las partes de la interfaz gráfica que lo
controlan y un tutorial completo.

Los dos tipos de problema que se resuelven sobre el campo de flujo tienen sus
propios manuales: [Transferencia de calor en fluidos](theory_fluid_heat.md)
(`RSolverFluidHeat`) y [Dispersión de contaminantes](theory_fluid_particle.md)
(`RSolverFluidParticle`).

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - flujo estacionario a través de un canal](#3-tutorial---flujo-estacionario-a-través-de-un-canal)
4. [Comprobación de un modelo](#4-comprobación-de-un-modelo)
5. [Limitaciones](#5-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnitas principales - los campos de velocidad y presión

Las incógnitas principales de **Incompressible viscous flow** son el vector
**velocidad** `v` en `m/s` y la **presión** `p` en `Pa`, ambas en cada nodo. Cada
nodo tiene **cuatro** grados de libertad (`vx`, `vy`, `vz` y `p`), por lo que una
malla de `N` nodos da un sistema de `4N` ecuaciones.

Esto hace que un modelo de flujo sea el tipo de física más costoso de Range FEA:
cuatro incógnitas por nodo frente a tres en un modelo estructural y una en un
modelo térmico, una matriz que no es simétrica ni definida positiva, y la
iteración no lineal de la sección 1.6 alrededor de cada resolución lineal.

Los dos tipos de problema asociados añaden un escalar cada uno, resuelto sobre el
campo de flujo que produjo el solver de fluidos:

| Tipo de problema | Solver | Incógnita principal | Unidades | Grados de libertad por nodo |
|---|---|---|---|---|
| Incompressible viscous flow | `RSolverFluid` | velocidad y presión | `m/s`, `Pa` | 4 |
| Heat transfer in fluids | `RSolverFluidHeat` | temperatura | `K` | 1 |
| Contaminant dispersion | `RSolverFluidParticle` | concentración de partículas | `kg/m^3` | 1 |

Ambos tipos asociados **requieren** una tarea *Incompressible viscous flow* en el
mismo problema: leen el campo de velocidad de su resultado y nunca lo calculan
por sí mismos. Cada uno se describe en su propio manual:
[Transferencia de calor en fluidos](theory_fluid_heat.md) y
[Dispersión de contaminantes](theory_fluid_particle.md).

El estado predeterminado de un nodo al que no afecta ninguna condición ni ningún
resultado anterior es velocidad nula y presión nula.

### 1.2 Ecuaciones fundamentales

El solver implementa las **ecuaciones de Navier-Stokes incompresibles** para un
fluido newtoniano de densidad `rho` y viscosidad dinámica `mu`. Ambas son
propiedades del material evaluadas en cada elemento a su temperatura (véase 1.9
para saber de dónde procede esa temperatura y cómo introduce la flotabilidad en
el flujo). Los balances de cantidad de movimiento y de masa son

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

y, con el solver temporal desactivado, el término transitorio desaparece y queda
el balance estacionario

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `v` | velocidad | `m/s` | incógnita resuelta |
| `p` | presión | `Pa` | incógnita resuelta |
| `rho` | densidad | `kg/m^3` | propiedad del material |
| `mu` | viscosidad dinámica | `kg/(m*s)`, es decir, `Pa*s` | propiedad del material |
| `g` | aceleración de la gravedad | `m/s^2` | condición de entorno Gravitational acceleration |

Aquí la presión no es una magnitud termodinámica. La incompresibilidad la deja
como el multiplicador de Lagrange que impone `div(v) = 0`, lo que tiene dos
consecuencias prácticas que conviene asimilar antes de leer ningún resultado:

- Solo tienen sentido las **diferencias de presión**. Un modelo en el que ninguna
  condición fija una presión en ningún lugar determina el campo de presión solo
  salvo una constante aditiva, y la matriz es singular respecto a esa constante
  (véase 1.7).
- La presión responde instantáneamente en todo el dominio. En esta formulación no
  hay ondas acústicas ni velocidad del sonido, por lo que una presión aplicada en
  una salida se nota en la entrada dentro de la misma resolución.

El **número de Reynolds**, construido con una longitud característica `L` y una
velocidad `V`,

```
Re = rho * V * L / mu
```

es el número que decide por sí solo si un modelo de flujo se va a comportar bien.
Por debajo de aproximadamente `Re = 1`, el término convectivo es despreciable y
el problema es casi lineal; hasta unos pocos miles, el flujo es laminar y el
solver está en su terreno; muy por encima, el flujo real es turbulento, y este
solver no modela la turbulencia (véase la sección 5).

### 1.3 Discretización por elementos finitos y estabilización

La velocidad y la presión se interpolan con las **mismas** funciones de forma
lineales en cada elemento: interpolación de igual orden `P1/P1` y `Q1/Q1`. Esa
combinación no cumple por sí sola la condición inf-sup (Babuska-Brezzi) y
produciría un campo de presión en tablero de ajedrez, por lo que la formulación
está **estabilizada**. Se añaden tres términos de estabilización, cada uno con su
propio parámetro a nivel de elemento:

| Término | Finalidad |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | estabiliza el término convectivo, que de otro modo produciría oscilaciones de nodo a nodo en un flujo dominado por la convección |
| **PSPG** - pressure stabilising Petrov-Galerkin | hace admisibles la velocidad y la presión de igual orden, eliminando el modo de presión en tablero de ajedrez |
| **LSIC** - least-squares on the incompressibility constraint | amortigua el error de divergencia, mejorando la conservación de la masa elemento a elemento |

Los parámetros SUPG y PSPG se construyen a partir de un número de Reynolds del
elemento

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`element length along the flow direction` = longitud del elemento en la
dirección del flujo, `element Reynolds number` = número de Reynolds del elemento,
`for` = para), de modo que el descentrado es fuerte donde el elemento está
dominado por la convección y se desvanece donde está dominado por la difusión. El
parámetro LSIC es `|v| * h / 2`. Estos parámetros son propios del solver de
flujo; el solver de dispersión utiliza su propio parámetro SUPG, limitado por el
paso de tiempo (véase el manual
[Dispersión de contaminantes](theory_fluid_particle.md)).

El parámetro SUPG utiliza la velocidad y la longitud **locales** del elemento, por
lo que se adapta elemento a elemento. El parámetro PSPG utiliza en cambio una
**velocidad de corriente global** y una escala de longitud de elemento global, lo
que mantiene uniforme la estabilización de la presión en toda la malla. La
velocidad de corriente se toma de las condiciones de entrada en la primera pasada
de cada resolución (en cada paso de tiempo en un cálculo transitorio), y no del
campo que se está resolviendo,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

(`over inflow surfaces` = sobre las superficies de entrada, `area of inflow
surfaces` = área de las superficies de entrada), es decir, la velocidad media de
entrada ponderada por el área, con `1 m/s` como valor por defecto cuando el modelo
no tiene ninguna condición de entrada. La escala de longitud del elemento se
deduce de su volumen: `cbrt(6*V/pi)` para un tetraedro y `cbrt(V)` para un
hexaedro.

**Elementos admitidos.** Las ecuaciones del flujo se ensamblan **solo en
elementos de volumen**, y solo en **tetraedros lineales** (`TETRA1`) y
**hexaedros lineales** (`HEXA1`). Cualquier otro tipo de elemento de volumen
detiene el cálculo con un error que indica el tipo. Los elementos de superficie
solo intervienen donde llevan una condición *Pressure (implicit)*, en la que
aportan una tracción de presión y nada más. Las entidades de punto y de línea no
aportan nada en absoluto: a diferencia de un modelo térmico, no son conductores
de menor dimensión, simplemente no existen para el flujo.

Esta es la diferencia práctica más marcada respecto a las demás físicas de
Range FEA: un modelo de fluido tiene que ser un **volumen mallado**. Genere la
malla tetraédrica con `Geometría` -> `Volumen` -> `Generar malla tetraédrica`
antes de esperar que una resolución de flujo haga algo.

### 1.4 Condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Wall | explícita | punto, línea, superficie | ninguna |
| Wall (frictionless) | explícita | punto, línea, superficie | ninguna |
| Velocity (inflow) | explícita | superficie | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | explícita | superficie | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | explícita | superficie | Pressure `[Pa]` |
| Pressure (implicit) | natural, tracción | superficie | Pressure `[Pa]` |

**Wall** (pared) es la condición de adherencia. En cada nodo de la entidad, las
tres componentes de la velocidad se eliminan del sistema y se mantienen en cero.
No tiene componentes: no hay nada que introducir, marcarla es toda la
configuración. Es la condición de todo contorno sólido que toca el fluido.

**Wall (frictionless)** (pared sin rozamiento) es la condición de deslizamiento,
o de simetría: el fluido puede moverse a lo largo de la superficie pero no
atravesarla. Se aplica buscando el **eje global dominante** de la normal de cada
elemento de superficie y manteniendo en cero solo esa componente de la velocidad,
dejando libres las otras dos. Fíjese en lo que esto implica: la condición solo es
exacta para una superficie cuya normal esté dirigida según `x`, `y` o `z`. En una
superficie inclinada respecto a los ejes globales restringe la dirección
equivocada, y cuanto más se acerca la normal a 45 grados entre dos ejes, peor es
la aproximación. Utilícela en los planos de simetría planos de un modelo alineado
con los ejes globales, y *Wall* en el resto.

**Velocity (inflow)** (velocidad de entrada) impone una velocidad del módulo dado,
dirigida según la **normal media** de la entidad de superficie y apuntando hacia
el fluido. El signo se toma de la orientación de los elementos de la entidad
respecto al volumen situado detrás de ellos, de modo que un valor positivo
significa flujo que entra en el dominio y un valor negativo flujo que sale de él.
Se introduce una rapidez, no un vector: la dirección procede de la geometría.

**Volumetric flow rate (inflow)** (caudal volumétrico de entrada) impone lo mismo
a partir del caudal total: el solver divide el caudal por el área de la entidad y
aplica la velocidad media resultante según la misma normal media.

```
v = Q / A          A = area of the entity
```

(`area of the entity` = área de la entidad.) Utilícela siempre que la magnitud que
realmente conoce sea un caudal (las características de una bomba, un conducto en
`m^3/s`) y *Velocity (inflow)* cuando conozca la velocidad. Ambas fijan las tres
componentes de la velocidad en cada nodo de la entidad, por lo que ambas convierten
la entrada en un flujo uniforme de tipo pistón; ninguna puede producir un perfil
desarrollado en la cara de entrada. Dé al modelo una longitud de entrada si el
perfil importa ahí.

**Pressure (explicit)** (presión explícita) elimina el grado de libertad de
presión de cada nodo de la entidad y lo mantiene en el valor dado. Es el
equivalente en presión de una velocidad impuesta y una forma de fijar la
constante aditiva de la sección 1.2.

**Pressure (implicit)** (presión implícita) es su contrapartida natural: en lugar
de fijar la presión nodal, aplica una **tracción de presión** sobre los elementos
de superficie y deja libre allí la velocidad, de modo que el fluido decide cuánto
sale y con qué perfil. La tracción incluye una altura hidrostática,

```
t = -( p + rho * g * h ) * n
```

donde `h` es la altura del nodo medida **en la dirección de la gravedad desde el
punto más bajo** del modelo y `n` es la normal a la superficie. La dirección de la
gravedad es la media de las condiciones de entorno *Gravitational acceleration*
asignadas a las superficies que llevan esta condición. Sin gravedad, el término
de altura desaparece y la tracción es solo la presión introducida.

Es la condición de salida que debe utilizarse en casi todos los modelos: deja que
se desarrolle el perfil de salida en lugar de forzarlo plano, e introducir `0`
convierte la salida en una referencia a presión atmosférica. *Pressure
(explicit)* es la opción más rígida y prescriptiva.

Cada componente de todas ellas es una **tabla en función del tiempo**, por lo que
una entrada puede aumentarse progresivamente durante los primeros segundos de un
cálculo transitorio, o a un caudal se le puede dar un ciclo de trabajo.

**Prioridad.** Las condiciones de velocidad y las de presión son independientes:
restringen grados de libertad distintos, por lo que una superficie de entrada
puede llevar una condición de velocidad y una salida una de presión sin
interferir. Entre condiciones del mismo tipo en la misma entidad prevalece la
última leída, y una *Wall* anula la velocidad de sus nodos después de aplicar
todas las demás condiciones de velocidad. Por tanto, una entidad que es a la vez
entrada y pared es una pared.

### 1.5 Condiciones de entorno e iniciales

#### Condiciones de entorno

| Condición de entorno | Componentes | Efecto |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | fuerza volumétrica `rho*g` sobre la entidad y término de altura de *Pressure (implicit)* |
| Temperature | Temperature `[K]` | temperatura de fondo de una tarea *Heat transfer in fluids*; el propio solver de flujo no la lee (véase 1.9) |

**Gravitational acceleration** vale por defecto `(0, 0, -9.80665) m/s^2`, es
decir, gravedad según `Z` negativo. Asígnela a la entidad de volumen para la
fuerza volumétrica y a la superficie de salida para que la altura de *Pressure
(implicit)* se mida en la dirección correcta.

La gravedad entra en la ecuación de cantidad de movimiento como la fuerza
volumétrica `rho*g`. En un dominio cerrado de densidad uniforme, un campo de
gravedad uniforme produce un gradiente de presión hidrostático y ningún
movimiento, que es la respuesta correcta. Donde la densidad varía con la
temperatura, la fuerza volumétrica varía con ella y la diferencia impulsa un
**flujo por flotabilidad**: la convección natural (véase 1.9). El resultado de
presión siempre contiene la parte hidrostática `rho*(g . x)`; no se resta nada.

#### Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Velocity | campo de velocidad inicial, aplicado en el primer cálculo |
| Pressure | campo de presión inicial, aplicado en el primer cálculo |

Los campos iniciales de temperatura y de concentración de los tipos de problema
asociados se describen en sus propios manuales.

Las condiciones iniciales solo se leen **en el primer cálculo**; después, los
campos avanzan a partir del resultado anterior. Esto es lo que hace funcionar el
procedimiento de reinicio de la sección 1.7: un cálculo transitorio iniciado con
**Reiniciar el solver / continuar** retoma el campo estacionario convergido en
lugar de partir de nuevo del reposo.

Una condición de contorno prevalece sobre una condición inicial allí donde
coinciden.

### 1.6 La iteración no lineal

El término convectivo `(v . grad) v` es cuadrático en la incógnita, por lo que las
ecuaciones del flujo son **no lineales** y no pueden resolverse en una sola
pasada. El solver las linealiza e itera: cada pasada ensambla el residuo de los
balances de cantidad de movimiento y de masa en el campo actual, resuelve un
**incremento** y lo suma,

```
J * dx = -R(x)
x <- x + dx
```

donde `x` contiene las velocidades y presiones nodales. El primer miembro es un
jacobiano aproximado en lugar del exacto, lo que convierte el esquema en una
iteración de Newton modificada: converge más despacio que un Newton completo cerca
de la solución, pero la solución a la que converge la fija el **residuo** `R`, que
son las propias ecuaciones de Navier-Stokes discretizadas.

**El paso está amortiguado.** Como la matriz no es la derivada exacta del residuo,
un paso completo `dx` puede pasarse de largo y dejar el campo más lejos de una
solución de lo que estaba, de modo que el residuo de la pasada siguiente resulta
mayor que el anterior. El solver vigila precisamente eso y solo aplica una parte
del paso calculado,

```
x <- x + omega * dx
```

empezando con `omega = 1`. Una pasada que empeoró el residuo lo divide por dos,
hasta un mínimo de `0.1`; una pasada que lo mejoró deja que `omega` vuelva a
crecer un cuarto cada vez, de modo que la retirada es rápida y la recuperación
pausada. El valor vigente se imprime como `Relaxation` en el registro, y se
mantiene en `1` en un cálculo que nunca se pasa de largo: un modelo de flujo que se
comporta bien nunca nota que el mecanismo existe. `omega` se restablece a `1` al
inicio de cada resolución, lo que en un cálculo transitorio significa en cada paso
de tiempo.

De ello se derivan dos consecuencias que determinan todo el procedimiento de un
modelo de flujo:

- **Una pasada nunca basta.** Una única iteración de una tarea de flujo devuelve
  lo que haya dado el primer incremento, que no es la solución de nada. Una tarea
  de flujo debe incluirse en un **grupo de tareas del problema** con un número de
  iteraciones: cientos para un cálculo estacionario y decenas por paso de tiempo
  para uno transitorio.
- **El número es un límite superior, no un objetivo.** El grupo lleva además un
  **valor de convergencia** y termina sus iteraciones en cuanto la tarea de flujo
  indica una convergencia inferior a él. Fije el número lo bastante alto para el
  paso más difícil y deje que el valor de convergencia detenga cada uno cuando se
  haya estabilizado.

Después de cada iteración, el solver escribe cuatro números en el archivo de
convergencia y en el registro:

| Magnitud | Significado |
|---|---|
| `Residual` | norma del vector residuo, la distancia a una solución |
| `Convergence-R` | variación del residuo desde la iteración anterior |
| `Convergence-V` | tamaño relativo del incremento de velocidad, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | tamaño relativo del incremento de presión, `\|\|dp\|\| / \|\|p\|\|` |

`Convergence-V` y `Convergence-P` son las que se comparan con el valor de
convergencia. Ambas son adimensionales y ambas disminuyen a medida que la
iteración se estabiliza: miden el tamaño del paso que todavía da el campo frente
al tamaño del propio campo, de modo que un valor de `1e-5` significa que la
solución se movió una cienmilésima de sí misma en esa pasada.

Sin embargo, un paso pequeño no es por sí solo una solución. Un sistema casi
singular (un modelo sin referencia de presión, o una malla que no puede soportar
el número de Reynolds que se le pide) da pasos diminutos porque **no puede
moverse**, no porque haya llegado, y su residuo se queda donde estaba o sube. Por
eso la prueba tiene dos partes, y el grupo solo se detiene cuando se cumplen
ambas:

| Parte | Condición |
|---|---|
| El campo ha dejado de moverse | `Convergence-V` y `Convergence-P` están ambas por debajo del valor de convergencia |
| El campo ha bajado realmente | el residuo ha caído a la **décima parte** de lo que era en la primera pasada de esta resolución |

La segunda parte es fija y no configurable: es una comprobación de sensatez de la
primera, no un segundo parámetro que ajustar. El registro la imprime como
`Residual ratio` junto a su objetivo, de modo que se ve lo cerca que estuvo de
detenerse un cálculo que no se detuvo antes. En un cálculo transitorio, cada paso
de tiempo es una resolución propia, por lo que la relación se vuelve a medir desde
el inicio de cada paso.

`Informe` -> `Convergencia del solver` traza los cuatro números. Un cálculo
estacionario convergido muestra el residuo cayendo varios órdenes de magnitud y
luego aplanándose, con `Convergence-V` y `Convergence-P` disminuyendo de forma
monótona hacia cero. Un residuo que se aplana en un valor alto, oscila o sube
significa que el cálculo no ha convergido, y en un cálculo así el grupo no se
detendrá antes, hagan lo que hagan los incrementos.

Un valor de convergencia de `0` desactiva la salida anticipada y ejecuta el número
completo, que es como se comportaba el solver antes de que existiera este valor.
Utilícelo cuando quiera una cantidad de trabajo fija por paso,
independientemente de lo que haga el residuo.

### 1.7 Análisis estacionario

Con el solver temporal desactivado, el solver itera los balances estacionarios de
cantidad de movimiento y de masa hasta un punto fijo. El sistema lineal de cada
iteración no es simétrico, por lo que se resuelve con **GMRES** y
precondicionamiento de Jacobi en lugar de con el método del gradiente conjugado
que utilizan las físicas simétricas.

Un modelo de flujo estacionario necesita, como mínimo:

- un **volumen** mallado con densidad y viscosidad dinámica asignadas;
- **Wall** en todos los contornos sólidos;
- una condición de **entrada**, o una diferencia de presión, que impulse el
  flujo;
- una **referencia de presión**: una superficie *Pressure (explicit)* o *Pressure
  (implicit)*. Sin ella, la presión solo está determinada salvo una constante y
  la iteración no se estabilizará.

Un modelo cuyos contornos son todos paredes y velocidades impuestas tiene una
trampa más: el caudal de entrada y el de salida impuestos deben **equilibrarse**,
ya que un fluido incompresible no puede acumularse. Imponer 5 m^3/s de entrada y
4 m^3/s de salida es pedir lo imposible, y la resolución no convergerá. Dejar la
salida como *Pressure (implicit)* evita por completo esta cuestión, que es el
principal motivo para preferirla.

El procedimiento habitual es hacer converger primero un cálculo estacionario y
utilizarlo como campo de partida de un cálculo transitorio, porque un cálculo
transitorio iniciado desde el reposo dedica sus primeros pasos de tiempo a
resolver el transitorio de arranque en lugar de la física de interés.

### 1.8 Análisis transitorio

Con el solver temporal activado, el solver hace avanzar las ecuaciones no
estacionarias con un esquema theta, donde `alpha` es el coeficiente de
aproximación del esquema temporal:

| Aproximación | `alpha` | Esquema resultante | Estabilidad |
|---|---|---|---|
| Backward difference (stable) | `1` | paso totalmente implícito | incondicional |
| Central difference (accurate) | `0.5` | Crank-Nicolson | incondicional, puede oscilar |
| Forward difference (fast) | `0` | paso explícito | condicional |

**Backward difference** es la opción que debe utilizarse en un modelo de flujo. Lo
que cuesta tiempo es la iteración no lineal dentro de cada paso de tiempo, y un
esquema implícito permite elegir el paso de tiempo según la física y no según la
malla.

Incluso con el esquema implícito, el paso de tiempo de un modelo de flujo está
limitado por la precisión, a través del **número de Courant**

```
C = |v| * dt / h
```

donde `h` es el tamaño del elemento en la dirección del flujo. Un número de
Courant cercano a `1` significa que el fluido atraviesa aproximadamente un
elemento por paso, que es el orden adecuado para resolver un transitorio; muy por
encima, el solver difumina el transporte que debería resolver, sea cual sea la
estabilidad del esquema. Elija `dt` a partir de la malla y de la velocidad
esperada:

```
dt ~ h / |v|
```

Cada paso de tiempo ejecuta la iteración no lineal completa de la sección 1.6,
por lo que el coste de un cálculo transitorio es el número de pasos de tiempo por
las iteraciones por paso. Entre diez y cincuenta iteraciones por paso es un rango
típico una vez que el campo de flujo ya ha convergido en un cálculo estacionario.
Un paso cuyo residuo no ha bajado al final de sus iteraciones es un paso cuya
respuesta no ha convergido, y el error se arrastra a todos los pasos siguientes.

### 1.9 Transferencia de calor y dispersión de contaminantes

Sobre el campo de flujo que produce este solver se resuelven dos tipos de
problema, cada uno con su propio manual:

- **Heat transfer in fluids** (`RSolverFluidHeat`) transporta la temperatura con
  el flujo y se acopla al solver térmico de un sólido mallado; véase
  [Transferencia de calor en fluidos](theory_fluid_heat.md);
- **Contaminant dispersion** (`RSolverFluidParticle`) transporta una
  concentración de contaminante con el flujo; véase
  [Dispersión de contaminantes](theory_fluid_particle.md).

Ambos leen la velocidad del resultado del flujo. La dispersión de contaminantes
nunca modifica el flujo. La transferencia de calor en fluidos sí, a través de las
propiedades del material.

#### Propiedades dependientes de la temperatura y flotabilidad

En **cada pasada**, el solver de flujo evalúa la densidad y la viscosidad dinámica
de cada elemento a partir de las tablas del material, a la temperatura del
elemento que dejó la última pasada de transferencia de calor. Sin una tarea de
transferencia de calor en el problema, esa temperatura es la predeterminada,
`293.15 K`, en todas partes. El solver de flujo no lee directamente la condición
de entorno *Temperature*; esta solo llega al flujo a través de una tarea de
transferencia de calor.

La densidad en la fuerza volumétrica `rho(T)*g` es lo que introduce la
flotabilidad en el flujo: el fluido caliente es más ligero que el que lo rodea y
sube, y el frío baja. Por tanto, la convección natural, las plumas térmicas y la
estratificación resultan de un grupo con una tarea *Incompressible viscous flow*
y una *Heat transfer in fluids*, siempre que el material tenga una **tabla de
densidad dependiente de la temperatura**. Los materiales suministrados *Water* y
*Air* la tienen. Un material cuya densidad es un único valor no da flotabilidad
con ninguna diferencia de temperatura.

Cómo intercambian datos las dos tareas:

- **Estacionario.** Ponga ambas tareas en un mismo grupo, primero la de flujo, con
  varias iteraciones y un valor de convergencia. La primera pasada del flujo ve la
  temperatura inicial (la condición inicial *Temperature* o `293.15 K`), por lo que
  todavía no tiene flotabilidad. A partir de la segunda iteración, cada pasada del
  flujo ve la temperatura de la pasada térmica anterior. Un modelo impulsado solo
  por la flotabilidad necesita iteraciones suficientes para que ambos se
  estabilicen juntos; un grupo con una sola iteración nunca los acopla.
- **Transitorio.** Cada paso de tiempo vuelve a ejecutar el grupo, y cada pasada
  del flujo ve la temperatura más reciente. Con una iteración por paso de tiempo,
  el flujo utiliza la temperatura del paso anterior, un retraso de un paso que es
  aceptable mientras el paso de tiempo resuelva el transitorio térmico. Varias
  iteraciones por paso lo eliminan.

El grupo termina cuando converge la tarea de flujo, ya que una tarea *Heat
transfer in fluids* no acoplada se declara convergida en cada pasada. Un cambio de
la temperatura modifica el residuo del flujo a través de `rho(T)` y `mu(T)`, por
lo que un flujo que ha convergido es también uno al que la temperatura ya no
empuja.

La formulación mantiene `rho(T)` en todos los términos del balance de cantidad de
movimiento y `div(v) = 0` para la masa. Para las pequeñas diferencias de densidad
de la convección natural en un líquido, o en un gas a lo largo de unas decenas de
kelvin, esto da la misma respuesta que la aproximación de Boussinesq. No es una
formulación compresible: la dilatación térmica del fluido no entra en el balance
de masa, por lo que un gas calentado cientos de kelvin queda fuera de su rango.

### 1.10 Resultados derivados

| Resultado | Se aplica a | Producido por | Significado |
|---|---|---|---|
| Velocity `[m/s]` | nodo | flujo | el campo de velocidad resuelto, tres componentes |
| Pressure `[Pa]` | nodo | flujo | el campo de presión resuelto |

Los resultados de los tipos de problema asociados se indican en sus propios
manuales.

La velocidad se almacena como **vector** en los nodos, por lo que la pestaña
`Resultados` puede mostrar su módulo como color, sus componentes por separado o
flechas. Conviene conocer dos herramientas de visualización específicas para un
resultado de flujo:

- un **plano de corte** a través del volumen, ya que la estructura interesante de
  un flujo está dentro del dominio y no en su superficie;
- las **líneas de corriente**, creadas con `Geometría` -> `Línea de corriente`,
  que trazan el campo de velocidad y son la forma más rápida de ver una
  recirculación o una zona muerta.

Después de cada registro, el registro del solver muestra las estadísticas de la
velocidad y de la presión, y los valores en los posibles puntos de
monitorización.

### 1.11 ¿Estacionario o transitorio?

| | Estacionario | Transitorio |
|---|---|---|
| Resuelve | el balance estacionario, iterado | las ecuaciones no estacionarias, avanzadas en el tiempo |
| Responde a | «cómo es el flujo desarrollado» | «cómo llega el flujo hasta ahí y qué transporta» |
| Iteraciones | cientos, en un grupo de tareas | decenas por paso de tiempo |
| Registros | uno | uno por paso de tiempo escrito |
| Necesita una referencia de presión | sí | sí |

**Utilice un análisis estacionario para** el campo de flujo desarrollado de un
conducto, un colector o un paso de un intercambiador de calor, la pérdida de
carga a través de un componente y como campo de partida de cualquier cálculo
transitorio. Es por donde empiezan casi todos los modelos de flujo.

**Utilice un análisis transitorio para** el arranque y la parada, una entrada
pulsante o cíclica, el desprendimiento de vórtices y, sobre todo, para la
**dispersión de contaminantes**, que es un problema de transporte en el tiempo sin
una respuesta estacionaria significativa para un suceso de emisión.

El procedimiento en dos etapas del tutorial *Dispersión de contaminantes en
fluidos* es el canónico: hacer converger el flujo en estacionario y reiniciar
después con el solver temporal y la tarea de dispersión activados; véase el
manual [Dispersión de contaminantes](theory_fluid_particle.md).

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Condiciones de entorno`,
`Material`, `Resultados`), el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Añada una tarea y elija **Incompressible viscous
flow**: «Steady-state and transient flow of newtonian fluids» (flujo
estacionario y transitorio de fluidos newtonianos).

Una tarea de flujo debe estar dentro de un **grupo de tareas**, que lleva los dos
valores que controlan la iteración no lineal de 1.6. Haga doble clic en
cualquiera de ellos para editarlo:

| Valor | Significado |
|---|---|
| **N.º de iteraciones** | el número máximo de pasadas que puede dar una resolución (o un paso de tiempo) |
| **Convergencia** | el grupo termina sus pasadas cuando la tarea de flujo se ha estabilizado por debajo de este valor **y** su residuo ha bajado; con `0` siempre se ejecuta el número completo |

El valor de convergencia predeterminado es `1e-5`, un tamaño de paso relativo
seguro para un modelo de flujo. Colocar una tarea de flujo fuera de un grupo,
donde se ejecuta exactamente una vez, no sirve de nada (véase 1.6).

Las combinaciones que conviene conocer:

- *Incompressible viscous flow* solo, en un grupo de varios cientos de
  iteraciones: un campo de flujo estacionario;
- *Incompressible viscous flow* y *Contaminant dispersion* en un mismo grupo: la
  dispersión sobre el flujo, la combinación transitoria clásica; véase
  [Dispersión de contaminantes](theory_fluid_particle.md);
- *Incompressible viscous flow* y *Heat transfer in fluids* en un mismo grupo: un
  flujo calentado o enfriado; véase
  [Transferencia de calor en fluidos](theory_fluid_heat.md).

El flujo excluye *Flow through porous media* y *Modal analysis*; cada uno de los
dos tipos de problema asociados requiere que haya una tarea de flujo.

### 2.2 Pestaña Problema

**Solver temporal**: se muestra para los tres tipos de problema de fluidos, ya
que todos dependen del tiempo. Déjelo desactivado para un cálculo de flujo
estacionario.

| Campo | Significado |
|---|---|
| Activar | activa y desactiva el avance transitorio en el tiempo |
| Aproximación | theta del esquema temporal: regresivo, centrado o progresivo, véase 1.8 |
| Tiempo inicial | primer valor de tiempo en segundos; un reinicio desde un resultado estacionario sitúa ese resultado en el tiempo inicial, y un reinicio de un cálculo transitorio interrumpido continúa desde su último paso |
| Tiempo final | `start time + time-step size * number of time-steps` (tiempo inicial + tamaño del paso de tiempo × número de pasos de tiempo), solo lectura |
| Tamaño del paso de tiempo | `dt` en segundos, véase la regla de Courant en 1.8 |
| Número de pasos de tiempo | cuántos pasos se calculan |
| Frecuencia de salida | escribe un archivo de resultados cada N pasos; `0` solo escribe el último paso |

El tipo de problema de flujo no tiene un grupo de configuración propio; el grupo
*Configuración de la dispersión de contaminantes* se describe en el manual
[Dispersión de contaminantes](theory_fluid_particle.md).
Todo lo demás se asigna por entidad en las pestañas de condiciones, y el número
de iteraciones (que aquí importa más que en ningún otro sitio de Range FEA) está
en la ventana del flujo de tareas del problema y no en esta pestaña.

### 2.3 Pestaña Material

La pestaña `Material` muestra el material asignado a la entidad seleccionada. El
tipo de problema de flujo requiere:

| Propiedad | Unidades | Comentario |
|---|---|---|
| Density | `kg/m^3` | `rho`, unos `1000` para el agua, `1.2` para el aire |
| Dynamic viscosity | `kg/(m*s)` | `mu`, unos `1.0e-3` para el agua, `1.8e-5` para el aire |

Los tipos de problema asociados indican sus propias propiedades en sus manuales.
Una entidad a la que le falte alguna propiedad requerida por un tipo de problema
del flujo de tareas no se resuelve, y la comprobación de la configuración avisa
antes de que empiece el cálculo. La base de datos de materiales suministrada
incluye **Water** (agua) y **Air** (aire), lo que normalmente completa este
paso.

Tenga en cuenta que la viscosidad que se pide es la viscosidad **dinámica** `mu`
en `Pa*s`, no la viscosidad cinemática `nu = mu/rho` en `m^2/s`. Introducir
`1.0e-6` para el agua porque es el número conocido es el error de material más
frecuente en un modelo de flujo, y hace que el fluido sea mil veces menos viscoso
de lo que debería, lo que normalmente se manifiesta en un cálculo que no
converge.

Cada propiedad es una tabla en función de la temperatura, evaluada a la
temperatura del elemento del cálculo anterior.

### 2.4 Pestaña Condiciones de contorno

La pestaña se divide en una lista de gestión de las condiciones de contorno
disponibles, arriba, y un editor de la condición seleccionada, abajo. Seleccione
primero una entidad en el árbol `Modelo`; el gestor ofrece entonces las
condiciones válidas para ese tipo de entidad y para los tipos de problema
seleccionados. Marque una para asignarla y luego edite sus componentes en el
árbol inferior.

Se ofrecen las condiciones de la sección 1.4. Notas sobre su uso:

- **Wall** se pone en todos los contornos sólidos y no lleva ningún valor. Una
  entidad de superficie de un volumen mallado sin ninguna condición *no* es una
  pared: es un contorno sin restricciones, lo que rara vez es lo que se pretende.
  Las paredes son lo primero que hay que comprobar en un modelo que se comporta
  de forma extraña.
- **Wall (frictionless)** es para planos de simetría alineados con los ejes
  globales. En una superficie inclinada restringe la dirección equivocada (véase
  1.4).
- **Velocity (inflow)** y **Volumetric flow rate (inflow)** impulsan ambas el
  flujo hacia dentro a través de una superficie según su normal media. Elija la
  que corresponda al dato que realmente tiene. Un valor negativo impulsa el flujo
  en sentido contrario, que es una forma de definir una salida.
- **Pressure (implicit)** es la condición de salida preferible: deja que se
  desarrolle el perfil de salida e, introducida como `0`, convierte la salida en
  la referencia de presión. **Pressure (explicit)** fija en cambio la presión
  nodal.
- Las condiciones de *Heat transfer in fluids* y de *Contaminant dispersion*
  aparecen en cuanto su tarea está en el flujo de tareas y se describen en sus
  propios manuales.
- Cada componente es una tabla en función del tiempo. Haga clic en **Editar
  valores dependientes del tiempo** para abrir el editor de componentes, donde se
  introducen los valores en función del tiempo; entre dos instantes dados, el
  valor se interpola linealmente, y después del último instante se conserva el
  último valor. Así se modela un suceso de emisión, una entrada con rampa o un
  ciclo de trabajo.

### 2.5 Condiciones iniciales y de entorno

La pestaña `Condiciones iniciales` ofrece *Velocity* y *Pressure* para un problema
de flujo. Definen el campo de partida del primer cálculo y son lo que permite que
un cálculo transitorio parta de algo distinto del reposo.

La pestaña `Condiciones de entorno` ofrece *Gravitational acceleration*, que vale
por defecto `(0, 0, -9.80665) m/s^2`. Asígnela al volumen para la fuerza
volumétrica, y a cualquier superficie que lleve *Pressure (implicit)* para que la
altura hidrostática de esa condición se mida en la dirección correcta.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver de flujo utiliza la entrada **GMRES**, ya que su
matriz no es simétrica.

Distinga las dos iteraciones anidadas antes de cambiar nada aquí: las iteraciones
del **solver matricial** resuelven un sistema linealizado, mientras que las
iteraciones del **grupo de tareas** controlan la no linealidad de la sección 1.6.
Un modelo de flujo que no converge casi siempre necesita más iteraciones del grupo
de tareas, una malla mejor o un conjunto de condiciones de contorno corregido, y
no un número mayor de iteraciones de GMRES.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra: *Velocity* o
*Pressure* para el flujo. En un cálculo transitorio, el historial se muestra con
`Informe` -> `Puntos de monitorización`.

### 2.8 Resultados y registros

La pestaña `Resultados` muestra las variables calculadas y controla la vista 3D.
La velocidad es un vector en los nodos y la presión un escalar en los nodos.

La pestaña `Registros` del panel `Modelo` muestra los registros de resultados:

- **análisis estacionario**: un único registro;
- **análisis transitorio**: un registro por cada paso de tiempo escrito. Al animar
  los registros se reproduce el flujo, y el botón rojo de grabación situado bajo
  el árbol escribe la animación en un archivo de vídeo.

`Informe` -> `Convergencia del solver` traza el historial de convergencia descrito
en 1.6 y es el informe que hay que abrir primero después de cualquier cálculo de
flujo. `Informe` -> `Archivo de registro del solver` muestra la salida completa
del solver, incluidas las estadísticas de velocidad y presión después de cada
registro.

---

## 3. Tutorial - flujo estacionario a través de un canal

**Objetivo.** Calcular el campo de flujo desarrollado y la pérdida de carga de un
fluido impulsado a través de un canal, y confirmar que el resultado ha
convergido.

Este tutorial supone un modelo de volumen mallado; el modelo suministrado
**Channel.tmsh** es exactamente este caso, y el tutorial *Dispersión de
contaminantes en fluidos* también lo utiliza. Si construye el suyo propio, dibuje
una caja, mállela en tetraedros y marque la cara de entrada, la cara de salida y
las caras restantes como tres entidades de superficie independientes para poder
asignarles condiciones.

### Paso 1 - seleccionar el problema

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada un **grupo de tareas** y ajuste su **N.º de iteraciones** a `2000`
   haciendo doble clic en el valor.
3. Deje su **Convergencia** en el valor predeterminado `1e-5`.
4. Añada una tarea **Incompressible viscous flow** dentro del grupo.
5. Confirme con `Aceptar`.

Estos dos valores son el núcleo de este paso. El cálculo da hasta 2000 pasadas y
se detiene en cuanto el campo se estabiliza por debajo de `1e-5` (véase 1.6).
Fije el número con holgura: es un tope, y lo que normalmente termina el cálculo es
el valor de convergencia.

### Paso 2 - generar la malla

`Geometría` -> `Volumen` -> `Generar malla tetraédrica`.

Un modelo de flujo solo se resuelve sobre elementos de volumen. Sin una malla de
volumen, el solver no tiene nada que ensamblar.

### Paso 3 - asignar el material

1. Seleccione la entidad de volumen en el árbol `Modelo`.
2. Abra la pestaña `Material` y asigne **Water** (agua), o introduzca:
   - **Density** (densidad) = `1000` `kg/m^3`
   - **Dynamic viscosity** (viscosidad dinámica) = `1.0e-3` `kg/(m*s)`

Compruebe que la viscosidad es la dinámica en `Pa*s` y no la cinemática (véase
2.3).

### Paso 4 - mantener el análisis estacionario

Abra la pestaña `Problema` y deje **Activar** desmarcado en el grupo *Solver
temporal*.

### Paso 5 - asignar las paredes

1. Seleccione la entidad de superficie que cubre los contornos sólidos del canal.
2. Abra la pestaña `Condiciones de contorno` y marque **Wall**.

No hay ningún valor que introducir. Es la condición de adherencia, y es lo que
hace que el flujo desarrolle un perfil en lugar de deslizarse como un pistón.

### Paso 6 - impulsar el flujo

1. Seleccione la entidad de superficie de entrada.
2. Marque **Volumetric flow rate (inflow)** e introduzca el caudal en `m^3/s`, o
   marque en su lugar **Velocity (inflow)** e introduzca una velocidad en `m/s`.

Antes de continuar, estime el número de Reynolds a partir de la velocidad media y
de la anchura del canal:

```
Re = rho * V * L / mu
```

Por debajo de unos pocos miles, el modelo está en el rango que maneja este
solver. Muy por encima, cuente con que el cálculo no converja, y lea la sección 5
antes de dedicarle tiempo.

### Paso 7 - abrir la salida

1. Seleccione la entidad de superficie de salida.
2. Marque **Pressure (implicit)** y ajuste **Pressure** = `0` `Pa`.

Este es el paso que hace resoluble el modelo. Da al campo de presión su
referencia y deja que se desarrolle el perfil de salida en lugar de forzarlo
plano. Un modelo con una entrada y paredes pero sin ninguna condición de presión
tiene una presión singular y no convergerá.

### Paso 8 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. La comprobación de la configuración indica los materiales o condiciones de
   contorno que faltan antes de que empiece el cálculo.
3. Siga el progreso en `Informe` -> `Archivo de registro del solver`.

Puede tardar un rato: un modelo de flujo de cualquier tamaño supone un trabajo
real, y el registro imprime un bloque por iteración. Busque `Convergence target`
en el registro: la pasada que lo alcanza queda marcada, y el grupo indica
entonces *All sub-tasks have converged* (todas las subtareas han convergido) y se
detiene.

### Paso 9 - comprobar la convergencia

`Informe` -> `Convergencia del solver`.

Hágalo antes de mirar ninguna imagen. El residuo debería caer varios órdenes de
magnitud y luego aplanarse, y `Convergence-V` y `Convergence-P` deberían acercarse
a cero. Si el residuo sigue cayendo con fuerza en la última iteración, el cálculo
fue demasiado corto: aumente el número de iteraciones y reinicie. Si oscila o
sube, la respuesta no es una solución y debe consultar la sección 5.

### Paso 10 - examinar los resultados

- Pestaña `Resultados`: muestre **Velocity**. El perfil a través del canal debería
  ser suave, nulo en las paredes y máximo cerca del centro.
- Corte el volumen con un plano: la estructura interesante de un flujo está dentro
  del dominio, no en su superficie.
- Muestre **Pressure**. Debería disminuir de forma monótona de la entrada a la
  salida, y valer `0` en la salida, donde la fijó.
- Cree **líneas de corriente** con `Geometría` -> `Línea de corriente` para ver de
  un vistazo las recirculaciones o las zonas muertas.
- Compruebe el balance de masa: la velocidad media de salida por el área de salida
  debe ser igual al caudal volumétrico impuesto en la entrada. Es la comprobación
  global más rápida de que el modelo hace lo que usted le pidió (véase la
  sección 4).

### Solución de problemas

| Síntoma | Causa |
|---|---|
| El residuo nunca baja | no hay ninguna condición de presión, por lo que la presión está indeterminada |
| El residuo oscila o sube | el número de Reynolds es demasiado alto para la malla, o la viscosidad se introdujo como cinemática |
| El flujo se desliza sin perfil | no hay condición *Wall* en los contornos sólidos |
| El resultado parece la primera iteración | la tarea no se colocó en un grupo de tareas, por lo que se ejecutó una sola vez |
| El cálculo se detuvo mucho antes del número de iteraciones | ha convergido: tanto los incrementos como el residuo cumplieron la prueba de 1.6 |
| El cálculo nunca se detiene antes, por muy holgado que sea el valor de convergencia | el residuo no está bajando: lea `Residual ratio` en el registro y considere no convergido el campo de flujo |
| *Failed to calculate element scales. Unsupported element type* (no se han podido calcular las escalas de los elementos; tipo de elemento no admitido) | la malla contiene elementos de volumen distintos de tetraedros o hexaedros lineales |
| No se resolvió nada | no se generó ninguna malla de volumen, o al material le falta la densidad o la viscosidad |
| El caudal de entrada y el de salida impuestos no se equilibran | un fluido incompresible no puede acumularse: utilice *Pressure (implicit)* en la salida |

---

## 4. Comprobación de un modelo

El solver no tiene un conjunto de verificación integrado, por lo que merece la
pena validar un modelo nuevo con algo que pueda calcularse a mano antes de
confiar en él. Cuatro comprobaciones baratas, por orden de utilidad:

**Balance de masa.** En cualquier modelo incompresible convergido, lo que entra
tiene que salir:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(`over the inlets` = sobre las entradas, `over the outlets` = sobre las salidas.)
Lea la velocidad media en la salida en las estadísticas del registro del solver,
multiplíquela por el área de salida y compárela con el caudal de entrada impuesto.
Esta única comprobación global detecta un cálculo no convergido, una pared que
falta y una condición de contorno en la entidad equivocada, y no cuesta nada.

**Hidrostática.** Desactive todas las entradas, deje el dominio cerrado con
paredes, asigne una condición de entorno *Gravitational acceleration* y resuelva.
El fluido debe quedar en reposo y la presión debe variar linealmente con la
profundidad:

```
p(h) = p_ref + rho * g * h
```

Cualquier movimiento residual es numérico. Esto valida en un solo cálculo la
fuerza volumétrica de la gravedad, la referencia de presión y las unidades del
material, y es el primer modelo más rápido de construir en una configuración
nueva.

**Flujo de Poiseuille plano.** Para un flujo laminar estacionario entre dos placas
paralelas separadas una distancia `H`, el perfil es parabólico, la velocidad en el
eje es `1.5` veces la media y el gradiente de presión es

```
dp/dx = 12 * mu * V_mean / H^2
```

Modele un canal recto con paredes arriba y abajo, una entrada impuesta y una
salida *Pressure (implicit)*, y compare con estos valores la velocidad calculada en
el eje y la pérdida de carga. Es la comprobación más exigente disponible: valida a
la vez el término viscoso, la pared con adherencia y las unidades de la
viscosidad. Dé al modelo una longitud de entrada suficiente para que el perfil se
desarrolle antes de medir, ya que la condición de entrada impone un perfil
plano.

**Flujo de Hagen-Poiseuille.** La misma comprobación en una tubería circular de
diámetro `D` y longitud `L`, en la que la velocidad en el eje es `2` veces la media
y

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

válida mientras el flujo sea laminar, es decir, por debajo de aproximadamente
`Re = 2300`.

Para cualquiera de ellas, refine la malla al menos una vez y confirme que la
respuesta converge en lugar de derivar, y confirme que el historial de
convergencia se ha aplanado antes de comparar nada. Un modelo de flujo no
convergido fallará todas estas comprobaciones por el mismo motivo.

---

## 5. Limitaciones

### Limitaciones de modelado

- **Sin modelo de turbulencia.** El solver integra las ecuaciones de
  Navier-Stokes laminares. Los términos SUPG, PSPG y LSIC son estabilización
  numérica, no un cierre de turbulencia: no representan la mezcla ni la
  disipación adicional de un flujo turbulento. Por encima de aproximadamente
  `Re = 2300` en una tubería, el flujo real es turbulento y este solver no lo
  modela: el cálculo puede converger igualmente, pero la respuesta describe un
  flujo laminar que no existe. No hay modelo `k`-`e`, ni funciones de pared, ni
  formulación de grandes remolinos.
- **Solo incompresible y newtoniano.** El balance de masa es `div(v) = 0` y la
  tensión viscosa es lineal en la velocidad de deformación. La densidad y la
  viscosidad pueden depender de la temperatura, pero no de la presión. El flujo
  compresible, los efectos del número de Mach, las superficies libres, las
  múltiples fases y el comportamiento no newtoniano quedan fuera de la
  formulación.
- **Flotabilidad sin dilatación del fluido.** Una densidad dependiente de la
  temperatura impulsa la convección natural a través de `rho(T)*g` (véase 1.9),
  pero la dilatación térmica nunca entra en el balance de masa. Esto es preciso
  para las pequeñas diferencias de densidad de la convección natural, pero no
  para un gas calentado cientos de kelvin. El flujo por flotabilidad es laminar
  como cualquier otro flujo aquí: una pluma o una cavidad por encima de un número
  de Rayleigh de aproximadamente `1e8` a `1e9` es turbulenta en la realidad.
- **La temperatura solo llega al flujo desde una tarea de transferencia de
  calor.** Sin ella, las propiedades del material se evalúan a `293.15 K`; la
  condición de entorno *Temperature* por sí sola no las modifica.
- **La dispersión de contaminantes es unidireccional.** El flujo transporta el
  contaminante; la concentración nunca modifica el flujo. Las limitaciones de los
  tipos de problema asociados se indican en sus propios manuales.
- **Solo elementos de volumen, y solo tetraedros y hexaedros lineales.** Un modelo
  de flujo debe ser un volumen mallado con elementos `TETRA1` o `HEXA1`; cualquier
  otro detiene el cálculo. Las superficies solo intervienen para llevar una
  tracción *Pressure (implicit)*, y las entidades de punto y de línea no aportan
  nada.
- **La pared sin rozamiento está alineada con los ejes.** Restringe la única
  componente global dominante de la normal de cada elemento, por lo que solo es
  exacta en una superficie cuya normal esté dirigida según `x`, `y` o `z`.
- **Las entradas son uniformes.** *Velocity (inflow)* y *Volumetric flow rate
  (inflow)* imponen ambas un perfil plano según la normal media de la superficie.
  Un perfil de entrada desarrollado debe obtenerse con una longitud de entrada en
  la malla.

### Limitaciones del solver

- **La iteración es un esquema de Newton modificado**, por lo que la convergencia
  cerca de la solución es lineal y no cuadrática. Cuente con cientos de
  iteraciones para un cálculo estacionario y fije el número de iteraciones del
  grupo de tareas lo bastante alto para que lo que termine el cálculo sea el valor
  de convergencia y no el número. El amortiguamiento de 1.6 mantiene en descenso
  un cálculo así, pero no puede hacerlo cuadrático.
- **El esquema de diferencias centradas del solver de flujo no es
  Crank-Nicolson.** La ponderación theta se aplica a la matriz pero no al residuo,
  por lo que *Central difference (accurate)* resuelve un residuo de Euler
  implícito con una matriz de media rigidez. La respuesta es la de diferencias
  regresivas, alcanzada más despacio. Utilice *Backward difference (stable)* para
  el flujo hasta que esto se corrija. El solver de dispersión de contaminantes no
  se ve afectado: es lineal y su esquema es un auténtico Crank-Nicolson.
- **Los parámetros de estabilización no se derivan.** `Tsupg`, `Tpspg`, `Tlsic` y
  la longitud del elemento dependen todos de la velocidad, y todos entran en la
  matriz como constantes evaluadas en el campo actual. Es la práctica habitual
  en una formulación estabilizada y uno de los motivos por los que el jacobiano no
  es el exacto.
- **La parte de residuo de la prueba de convergencia es fija.** Una resolución
  solo cuenta como convergida cuando su residuo ha caído a la décima parte de lo
  que era al empezar esa resolución, y esa décima parte no se puede cambiar. Es
  deliberadamente conservadora: un modelo que converge lentamente puede no
  cumplirla nunca y ejecutará entonces su número completo de iteraciones. El
  `Residual ratio` del registro indica lo cerca que estuvo un cálculo así.
- **Se requiere una referencia de presión.** Sin una superficie *Pressure
  (explicit)* ni *Pressure (implicit)*, la presión solo está determinada salvo una
  constante y la iteración no se estabilizará.
- **Los caudales impuestos deben equilibrarse.** En un modelo cuyos contornos son
  todos paredes y velocidades impuestas, el caudal de entrada debe ser
  exactamente igual al de salida; un fluido incompresible no puede absorber la
  diferencia.
- **Coste.** Cuatro incógnitas por nodo, una matriz no simétrica y una iteración
  no lineal alrededor de cada resolución lineal hacen que un modelo de flujo sea,
  con diferencia, el tipo de problema más costoso de Range FEA. Dimensione la
  malla en consecuencia y haga converger un campo estacionario antes de iniciar
  cualquier cálculo transitorio.
