# Transferencia de calor - Manual de teoría y guía del usuario

Este documento describe el solver térmico (`RSolverHeat`) de Range FEA, que
atiende el tipo de problema **Transferencia de calor** (*Heat transfer*): las
ecuaciones que resuelve, el significado de cada dato de entrada que acepta, las
partes de la interfaz gráfica que lo controlan y dos tutoriales completos.

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - transferencia de calor estacionaria](#3-tutorial---transferencia-de-calor-estacionaria)
4. [Tutorial - transferencia de calor transitoria](#4-tutorial---transferencia-de-calor-transitoria)
5. [Comprobación de un modelo](#5-comprobación-de-un-modelo)
6. [Limitaciones](#6-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnita principal - el campo de temperatura

La incógnita principal es la **temperatura** `T` en cada nodo, en kelvin. Cada
nodo tiene un único grado de libertad, por lo que una malla de `N` nodos da un
sistema de `N` ecuaciones. El flujo de calor se obtiene a partir del campo de
temperatura tras la resolución.

La temperatura es un escalar, lo que hace que un modelo térmico sea mucho más
barato que un modelo estructural de la misma malla: una incógnita por nodo en
lugar de tres, y una matriz con una novena parte de los términos.

La temperatura predeterminada de un nodo al que no afecta ninguna condición ni
ningún resultado anterior es `293.15 K`.

### 1.2 Ecuación fundamental

El solver implementa la **conducción de calor lineal** con contornos convectivos.
El balance de energía de un cuerpo con conductividad `k`, densidad `rho` y
capacidad calorífica específica `c` es

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

y, con el solver temporal desactivado, el término transitorio desaparece y queda
el balance estacionario

```
div( k * grad(T) ) + q = 0
```

El calor sale de una superficie por convección según la ley de enfriamiento de
Newton,

```
qs = h * ( T - Tf )
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `T` | temperatura | `K` | incógnita resuelta |
| `q` | densidad de fuente de calor | `W/m^3` | condición Heat, calor Joule, radiación |
| `k` | conductividad térmica | `W/(m*K)` | propiedad del material |
| `c` | capacidad calorífica | `J/(kg*K)` | propiedad del material |
| `rho` | densidad | `kg/m^3` | propiedad del material |
| `h` | coeficiente de convección | `W/(m^2*K)` | condición de convección, véase 1.5 |
| `Tf` | temperatura del fluido | `K` | condición de convección |
| `Q` | flujo de calor | `W/m^2` | obtenido, véase 1.9 |

No hay término de advección. Un problema de transferencia de calor mueve la
energía por conducción dentro del cuerpo y por convección a través de su
contorno; no transporta energía con un medio en movimiento. Para eso utilice
*Heat transfer in fluids*.

**Propiedades de material requeridas.** Un problema térmico requiere **cuatro**:

| Propiedad | Unidades | Se usa para |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | la matriz de conducción |
| Heat capacity | `J/(kg*K)` | la matriz de capacidad, solo en transitorio |
| Density | `kg/m^3` | la matriz de capacidad, solo en transitorio |
| Emissivity | `-` | solo para la transferencia de calor por radiación |

Una entidad cuyo material carezca de alguna de las cuatro **no se resuelve**,
incluso en un cálculo de conducción puramente estacionario que nunca consulta la
densidad ni la emisividad. La comprobación de la configuración lo avisa antes de
que empiece el cálculo. La única excepción es una entidad que lleva una condición
de contorno térmica: esa entidad se incluye en el sistema diga lo que diga su
material, y su conductividad y su capacidad son entonces simplemente cero.

Cada propiedad es una tabla en función de la temperatura. El valor utilizado para
un elemento se lee de la tabla a la **temperatura del elemento del cálculo
anterior**, por lo que una conductividad dependiente de la temperatura va una
iteración por detrás. La sección 1.6 explica cómo hacerla converger.

### 1.3 Discretización por elementos finitos

La forma débil del balance de energía da, para cada elemento,

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(`conduction + convection` = conducción + convección, `capacity matrix` = matriz
de capacidad, `sources + convection` = fuentes + convección), donde `B` se
construye a partir de las derivadas de las funciones de forma y `N` son las
propias funciones de forma. Cada matriz de elemento es `n x n` para un elemento
de `n` nodos: una fila por nodo, no tres.

Los términos de convección se ensamblan **solo en los elementos de superficie**,
y solo donde hay asignada una condición de contorno de convección. La rigidez de
convección es la forma consistente `INT( N^T * h * N )`, mientras que la carga de
convección se reparte por igual entre los nodos del elemento. Ambas son
suficientemente coherentes en una malla razonable, y la diferencia desaparece al
refinar la malla.

**Medidas de los elementos.** Como en el resto de Range FEA, a las entidades de
menor dimensión se les asigna una medida geométrica para que puedan representar
cuerpos reales:

| Entidad | Medida | Dónde se define |
|---|---|---|
| Volumen | 1 | - |
| Superficie | espesor de la superficie | propiedades geométricas de la entidad |
| Línea | sección de la línea | propiedades geométricas de la entidad |
| Punto | volumen del punto | propiedades geométricas de la entidad |

Una **superficie** conduce el calor en su propio plano, a través de una sección
igual a su espesor, y almacena energía en proporción a ese espesor. No hay
gradiente de temperatura a través del espesor: un elemento de superficie tiene un
solo nodo de espesor, por lo que por construcción es isotermo en su espesor. Por
tanto, una superficie de **espesor nulo** no conduce ni almacena nada, pero
sigue llevando la convección, que es exactamente lo que se quiere para la cara de
contorno de un volumen mallado, donde la cara solo existe para llevar la
condición de convección.

Una **línea** conduce a lo largo de su propia dirección a través de su sección.
Un **punto** no tiene ninguna dirección a lo largo de la cual conducir, por lo
que no aporta ninguna conducción: solo lleva una capacidad, proporcional a su
volumen de punto, y una fuente de calor. Un punto es una masa térmica
concentrada, no un conductor.

La matriz de capacidad solo se ensambla cuando es necesaria, es decir, en un
análisis transitorio.

### 1.4 Fuentes y condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Temperature | explícita (Dirichlet) | punto, línea, superficie, volumen | Temperature `[K]` |
| Heat | natural, fuente | punto, línea, superficie, volumen | Heat `[W]`, véase más abajo |
| Simple convection | natural (Robin) | superficie | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | natural, correlación | superficie | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | natural, correlación | superficie | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | natural, fuente | superficie | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | natural, fuente | volumen | Heat per unit volume `[W/m^3]` |

**Temperature** (temperatura) es la única condición explícita del problema
térmico. Cada nodo de la entidad a la que se asigna se elimina del sistema de
ecuaciones y se mantiene en el valor dado; su contribución pasa al segundo
miembro de las ecuaciones vecinas. Un modelo estacionario necesita al menos una de
ellas, o al menos una superficie de convección (véase 1.7).

Donde las entidades se solapan, **prevalece la entidad de menor dimensión**: una
temperatura impuesta en una superficie sustituye, en los nodos compartidos, a la
impuesta en el volumen situado detrás; una línea prevalece sobre una superficie y
un punto sobre todo lo demás. A diferencia de las restricciones de desplazamiento
del solver estructural, las temperaturas no se combinan: solo hay un grado de
libertad por nodo que mantener, así que simplemente prevalece la última condición
leída.

**Heat** (calor) es una fuente de calor y su valor es la **potencia total en
`W`** aportada a la entidad a la que se asigna, sea cual sea la dimensión de esa
entidad. El solver necesita una densidad de fuente, por lo que el total se
reparte sobre la medida de la entidad antes del ensamblaje:

| Entidad | Densidad que aplica el solver |
|---|---|
| Volumen | valor / volumen de la entidad, `W/m^3` |
| Superficie | valor / área de la entidad, `W/m^2` |
| Línea | valor / longitud de la entidad, `W/m` |
| Punto | valor / número de elementos de punto, `W` cada uno |

Por tanto, la potencia aportada es exactamente el número introducido: no cambia
con el tamaño de la entidad, con el refinamiento de la malla ni con la escala del
modelo. Solo reciben una parte los elementos que el solver puede calcular, de modo
que una entidad sin propiedades de material se omite y su condición se notifica.
Una fuente de calor superficial no se pondera con el espesor de la superficie.

**Heat rate (unit area)** y **Heat rate (unit volume)** imponen el mismo término
fuente como densidad en lugar de como total: `W/m^2` sobre el área de una entidad
de superficie y `W/m^3` sobre el volumen de una entidad de volumen. Utilícelas
cuando la fuente es por naturaleza una densidad (un calefactor especificado por
metro cuadrado) y *Heat* cuando es por naturaleza un total, como un elemento de
potencia conocida. Todas se suman en una entidad que lleve varias, y todas
admiten una tabla en función del tiempo, de modo que un calefactor puede
encenderse y apagarse durante un cálculo transitorio.

Las condiciones de **convección** se aplican a superficies y añaden tanto un
término de rigidez como una carga, por lo que actúan como condición de contorno de
Robin y no como flujo fijo: al subir la temperatura de la pared aumenta el calor
que pierde. Las tres variantes terminan en el mismo par `(h, Tf)`; solo difieren
en la procedencia de `h`:

- **Simple convection** (convección simple) toma `h` y `Tf` directamente de los
  dos valores introducidos. Utilícela siempre que el coeficiente de convección se
  conozca por un manual, una medición o un cálculo anterior. Es la única de las
  tres que está totalmente bajo su control.
- **Forced convection** (convección forzada) tiene dos modos. En una pared entre
  un sólido mallado y un fluido mallado, una vez que una tarea *Heat transfer in
  fluids* ha producido un resultado, acopla directamente ambas resoluciones; no
  interviene ninguna correlación, ya que el flujo resuelto ya lleva la
  convección. El solver térmico de fluidos entrega, para cada elemento de la
  pared, el par `(h, Tf)` que reproduce el flujo de calor que su propia solución
  hace pasar a través de la pared; véase *Transferencia de calor conjugada* en
  1.6. En cualquier otro caso (una superficie que no limita con ningún fluido
  mallado, o la primera pasada de un cálculo acoplado antes de que se haya
  ejecutado la tarea térmica de fluidos), `h` se calcula con una correlación para
  flujo externo a lo largo de una placa plana, a partir de las propiedades del
  fluido, el diámetro hidráulico, la *Fluid temperature* y la *Velocity*
  introducidas. El registro indica cuál de los dos modos se utiliza.
- **Natural convection** (convección natural) calcula `h` con una correlación
  para placas horizontales, a partir de las propiedades del fluido, el diámetro
  hidráulico y la diferencia de temperatura entre la pared y el fluido.

Si se asigna más de una condición de convección a la misma superficie, no se
suman: primero se lee la simple, la forzada la sobrescribe y la natural
sobrescribe a ambas.

### 1.5 Las correlaciones de convección

Las dos formas calculadas construyen los números adimensionales habituales a
partir de las propiedades del fluido introducidas con la condición de contorno,
con el diámetro hidráulico `d` como longitud característica:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

y convierten el número de Nusselt en coeficiente de convección mediante

```
h = Nu * k / d
```

| Forma | Correlación | Número de Nusselt |
|---|---|---|
| Forzada | flujo externo, placa plana | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Natural | placa horizontal, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Natural | placa horizontal, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

Los `k`, `c`, `rho`, `mu` y `beta` de estas expresiones son las propiedades del
**fluido** introducidas con la condición de contorno, no las propiedades del
material del sólido.

Ambas condiciones se crean con las propiedades del **aire seco a 20 grados
Celsius y 101.325 kPa**, de modo que el fluido ya está descrito y solo queda por
introducir el flujo y la geometría:

| Componente | Valor predeterminado |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, el valor de gas ideal `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Sustitúyalas para cualquier otro fluido: son las propiedades del fluido, no el
material de la pared. Así pues, *Natural convection* está lista para resolverse
tal como se asigna; *Forced convection* sigue necesitando una **velocidad**, que
ningún valor predeterminado puede adivinar, salvo que la superficie limite con un
fluido mallado, en cuyo caso no se lee ninguno de estos valores.

Todos esos números dividen por `mu`, `k` o `d`, y `Re` y `Pr` caen a cero si lo
hacen `rho`, `c` o `v`, lo que dejaría `h = 0` y una superficie que nunca se
enfría. Por ello, un valor cero se trata como un error en lugar de disimularse:
el solver se detiene e indica la componente y la entidad, por ejemplo

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

(«El valor de 'Velocity' configurado en la condición de contorno 'Forced
convection' de la entidad 'Surface' es cero; no se puede evaluar la correlación
de convección».)

La velocidad que calcula el solver de flujo nunca se introduce en la
correlación. Una pared con adherencia mantiene el fluido en reposo, por lo que la
velocidad junto a ella es cero, y la correlación necesitaría la velocidad de la
corriente libre y la temperatura media, que un flujo tridimensional general no
define. En su lugar, una pared que limita con un fluido mallado se acopla a la
resolución del fluido.

Conviene tener en cuenta tres consecuencias:

- La correlación es **fija**. El solver no tiene en cuenta la orientación de la
  superficie, si el flujo es interno o externo, ni si la capa límite es laminar o
  turbulenta. Todo entra a través del diámetro hidráulico, que es la longitud
  característica de la correlación: es su único control sobre la geometría.
- Un diámetro hidráulico nulo daría `h = 0`, es decir, una pared aislada, por lo
  que el solver se detiene en lugar de aceptarlo.
- La convección natural depende de la **temperatura de la pared**, que se toma de
  la temperatura del elemento de la resolución **anterior**. En el primer cálculo
  de un análisis estacionario es la temperatura inicial, por lo que el coeficiente
  se evalúa con una diferencia de temperatura incorrecta. Incluya la tarea en un
  grupo de tareas del problema con varias iteraciones para hacerla converger, o
  utilice *Simple convection* con un coeficiente calculado por usted. Una
  diferencia de temperatura nula da `Nu = 1`.

### 1.6 Condiciones de entorno, iniciales y de acoplamiento

#### Condiciones de entorno

| Condición de entorno | Componentes | Efecto |
|---|---|---|
| Temperature | Temperature `[K]` | temperatura de fondo de la entidad, solo en el primer cálculo |

#### Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Temperature | campo de temperatura inicial, aplicado en el primer cálculo |

Las condiciones de entorno, las condiciones iniciales y las condiciones de
contorno se leen en ese orden y cada una sobrescribe a la anterior, de modo que
**una condición de contorno prevalece sobre una condición inicial, que a su vez
prevalece sobre una condición de entorno**. Los valores de entorno e iniciales
solo se leen **en el primer cálculo**; después, el campo avanza a partir del
resultado anterior.

#### Acoplamiento con otros solvers

El solver térmico utiliza y produce resultados compartidos con el resto del flujo
de tareas.

| Sentido | Variable | Efecto |
|---|---|---|
| Radiación -> térmico | Heat - radiation part | se añade al término fuente de los elementos de **superficie** |
| Electrostática -> térmico | Joule heat | se añade al término fuente de cada elemento |
| Tensiones -> térmico | Displacement | la malla se deforma para la resolución térmica y después se restablece |
| Térmico -> tensiones | Temperature | determina el término de dilatación térmica del solver estructural |
| Térmico -> cualquier solver | Temperature | selecciona la fila de cada tabla de material dependiente de la temperatura |
| Térmico de fluidos -> térmico | `(h, Tf)` de la pared | determina las paredes *Forced convection* que limitan con un fluido mallado |
| Térmico -> térmico de fluidos | temperatura del sólido | mantiene esas mismas paredes a la temperatura del sólido en la resolución del fluido |

La **transferencia de calor por radiación** es un tipo de problema independiente
con su propio solver. Requiere una tarea *Heat transfer*, resuelve un sistema de
radiosidad de recinto sobre parches de superficie mediante factores de visión y
la emisividad del material, y devuelve el resultado como fuente de calor
superficial. Añada ambas tareas a un mismo grupo de tareas del problema y deje
que el grupo itere: el solver de radiación indica la convergencia y termina el
grupo cuando el calor de sus parches deja de cambiar.

El **calor Joule** llega de la misma forma desde una tarea *Electro-statics*
situada antes de la tarea térmica, por lo que el calentamiento resistivo no
necesita más configuración.

Salvo que esté acoplado a un fluido, el solver térmico se declara convergido
incondicionalmente, por lo que un grupo de tareas que solo contenga una tarea
térmica siempre se detiene tras una iteración. Iterar merece la pena cuando algo
de lo que depende la resolución térmica va a su vez con retraso: calor radiado,
un coeficiente de convección natural o una conductividad muy dependiente de la
temperatura.

#### Transferencia de calor conjugada

El solver térmico solo resuelve **sólidos**. Un volumen cuyo material sea un
fluido (un gas o un líquido, o un material de estado no especificado que tenga
viscosidad dinámica, lo que incluye todos los fluidos de la base de datos de
materiales) nunca forma parte de la resolución térmica, tenga las propiedades o
condiciones que tenga. Lo mismo ocurre con una entidad de punto, línea o
superficie situada dentro del fluido, como una entrada; una condición sobre ella
que solo lea el solver térmico se notifica en el registro. El dominio fluido
corresponde al tipo de problema *Heat transfer in fluids*, que transporta el
calor con el flujo.

Ambos se encuentran en las paredes que llevan la condición **Forced convection**.
Con las dos tareas en un mismo grupo de tareas del problema (primero la tarea
térmica de fluidos), cada pasada del grupo hace lo siguiente:

1. El solver térmico de fluidos mantiene los nodos de la pared a la temperatura
   que calculó por última vez el solver térmico en el sólido (en la primerísima
   pasada, cuando aún no existe esa temperatura, la pared está aislada), resuelve
   el fluido y toma el flujo de calor que entra en el fluido en cada nodo de la
   pared. Ese flujo es el residuo de las ecuaciones del fluido en el nodo, el
   mismo que vería una única resolución de ambos dominios, por lo que una capa
   límite térmica delgada no cuesta precisión.
2. Para cada elemento de pared entrega el par `(h, Tf)`. `h = k * G` es la
   conductancia del primer elemento fluido detrás de la pared (`k` dividida por
   la altura del elemento sobre la pared, en un tetraedro) y `Tf` se fija de modo
   que `q = h * (Tw - Tf)` sea el flujo del paso 1.
3. El solver térmico los aplica a la pared como lo haría con una condición
   *Simple convection*, resuelve el sólido y devuelve la temperatura de su pared.

El par `(h, Tf)` es un recurso numérico que hace que ambas resoluciones
coincidan: `h` no es el coeficiente de transferencia de calor de un manual y
cambia con la malla. Lo que converge es la temperatura y el flujo a través de la
pared, que son los del problema acoplado. No se necesita nada sobre la velocidad
del fluido junto a la pared, por lo que una pared con adherencia (donde el flujo
está en reposo) transfiere el calor exactamente como debe.

La temperatura de la pared que se pasa al fluido se relaja con un **factor de
Aitken** calculado a partir de las dos últimas pasadas. Sin él, un sólido poco
conductor junto a un fluido bien resuelto puede necesitar cientos de pasadas; con
él, el grupo suele estabilizarse en unas pocas. Mientras están acoplados, ambos
solvers indican como convergencia la variación relativa de su campo de
temperatura, así que defina el valor de convergencia del grupo de tareas (`1e-6`
es un buen punto de partida) y dele suficientes iteraciones. Con un valor de
convergencia cero se ejecutan todas.

### 1.7 Análisis estacionario

Con el solver temporal desactivado, el solver ensambla y resuelve

```
K * T = f
```

`K` es simétrica definida positiva en cuanto se ha impuesto al menos una
temperatura, por lo que se resuelve con el método del **gradiente conjugado** y
precondicionamiento de Jacobi.

Un modelo estacionario **sin condición de contorno Temperature ni superficie de
convección** es el equivalente térmico de una estructura sin restricciones: se
puede introducir energía pero no tiene por dónde salir, la matriz es singular y
la resolución no convergerá. Todo modelo estacionario necesita al menos un camino
hacia una temperatura conocida.

### 1.8 Análisis transitorio

Con el solver temporal activado, el solver avanza en el tiempo

```
M * dT/dt + K * T = f
```

con un esquema theta, donde `alpha` es el coeficiente de aproximación del
esquema temporal:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

El parámetro *Aproximación* del solver temporal elige `alpha`:

| Aproximación | `alpha` | Esquema resultante | Estabilidad |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, un paso implícito | incondicional |
| Central difference (accurate) | `0.5` | Crank-Nicolson | incondicional, puede oscilar |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, un paso explícito | condicional |

**Backward difference** (diferencias regresivas) es la opción predeterminada
segura para un modelo térmico. Cuesta una resolución matricial por paso, pero el
paso está limitado por la precisión deseada y no por el tamaño del elemento más
pequeño. **Central difference** (diferencias centradas) tiene precisión de
segundo orden y es la mejor opción cuando el paso de tiempo es lo bastante
pequeño para resolver el transitorio, aunque puede oscilar ante un encendido
brusco. **Forward difference** (diferencias progresivas) pone toda la matriz de
conducción en el paso de tiempo anterior, lo que hace barato cada paso y frágil
el cálculo.

La variante progresiva solo es estable mientras el paso de tiempo se mantenga por
debajo de aproximadamente

```
dt < rho * c * h^2 / ( 2 * k )
```

donde `h` es el tamaño del elemento más pequeño. Como la difusividad térmica
`k/(rho*c)` de un metal es del orden de `1e-5 m^2/s`, un elemento de un milímetro
ya obliga a un paso de tiempo muy inferior a una décima de segundo. Salvo para
una resolución temporal muy fina, utilice el esquema implícito.

Un cálculo transitorio inestable es fácil de reconocer: el campo de temperatura
oscila de un paso a otro con amplitud creciente, a menudo entre valores
físicamente imposibles, en lugar de estabilizarse. La solución es un paso de
tiempo menor o el esquema implícito.

### 1.9 Resultados derivados

Tras la resolución, el flujo de calor en los elementos se obtiene a partir del
campo de temperatura como el promedio sobre los puntos de integración de

```
Q = -k * grad(T)
```

evaluado en el elemento y girado al sistema de coordenadas global en los
elementos de línea y de superficie. En una superficie con convección, el flujo
convectivo

```
Qh = h * ( Tf - T )
```

se añade según la normal a la superficie, de modo que el vector de flujo de una
cara con convección apunta en la dirección en que el calor sale realmente.

El flujo es una **densidad** en `W/m^2`, independiente del tamaño del elemento en
el que se calculó, por lo que puede compararse entre elementos y con un cálculo
manual, y converge a medida que se refina la malla.

También se almacena el **coeficiente de transferencia de calor**. Es el
coeficiente que el solver utilizó realmente en cada elemento de superficie, la
única forma de ver lo que produjeron las correlaciones *Forced convection* y
*Natural convection*; en una superficie con convección natural varía de un
elemento a otro, ya que depende de la temperatura local de la pared. En una pared
acoplada a un fluido es la conductancia del primer elemento fluido, que depende
de la malla (véase *Transferencia de calor conjugada* en 1.6). Es nulo en todos
los elementos que no llevan condición de convección.

| Resultado | Se aplica a | Significado |
|---|---|---|
| Temperature `[K]` | nodo | el campo de temperatura resuelto |
| Heat flux `[W/m^2]` | elemento | vector de flujo conductivo, más la parte convectiva en las superficies con convección |
| Heat transfer coefficient `[W/(m^2*K)]` | elemento | el coeficiente de convección utilizado, nulo donde no hay convección |

Después de cada registro, el registro del solver muestra las estadísticas de los
tres y los valores en los posibles puntos de monitorización.

### 1.10 ¿Estacionario o transitorio?

| | Estacionario | Transitorio |
|---|---|---|
| Resuelve | `K*T = f` | `M*T' + K*T = f` en el tiempo |
| Responde a | «cuánto se calienta al final» | «cuánto tarda en llegar ahí» |
| Usa la densidad y la capacidad calorífica | no, pero deben estar presentes | sí |
| Necesita una temperatura impuesta o una superficie con convección | sí | no, basta con una capacidad |
| Registros | uno | uno por paso de tiempo escrito |

**Utilice un análisis estacionario para** la distribución final de temperatura de
una pieza en funcionamiento continuo, el dimensionado de un disipador, un puente
térmico en un muro o el campo de temperatura que alimenta un análisis de
tensiones térmicas. Es la opción predeterminada y cubre la mayor parte del
trabajo.

**Utilice un análisis transitorio para** el calentamiento y el enfriamiento, un
ciclo de trabajo, un choque térmico, un temple o cualquier caso en el que lo que
se busca sea la constante de tiempo de la pieza.

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Condiciones de entorno`,
`Material`, `Resultados`), el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Añada una tarea y elija **Heat transfer**:
«Steady-state and transient Heat-transfer» (transferencia de calor estacionaria
y transitoria).

La transferencia de calor se combina libremente con todas las demás físicas
excepto *Modal analysis*. Las cadenas que conviene conocer son:

- *Heat transfer* y luego *Stress analysis*: tensión térmica;
- *Heat transfer* y *Radiative heat transfer* en un mismo grupo de tareas que
  itera: conducción más radiación de recinto;
- *Electro-statics* y luego *Heat transfer*: calentamiento resistivo.

Para iterar un par acoplado, añada un grupo de tareas y fije su número de
iteraciones; el grupo se detiene antes cuando todas sus tareas indican
convergencia.

### 2.2 Pestaña Problema

**Solver temporal**: se muestra para la transferencia de calor, ya que es un tipo
de problema dependiente del tiempo. Déjelo desactivado para un análisis
estacionario.

| Campo | Significado |
|---|---|
| Activar | activa y desactiva el avance transitorio en el tiempo |
| Aproximación | theta del esquema temporal: regresivo, centrado o progresivo, véase 1.8 |
| Tiempo inicial | primer valor de tiempo en segundos; se ignora al reiniciar el solver |
| Tiempo final | `start time + time-step size * number of time-steps` (tiempo inicial + tamaño del paso de tiempo × número de pasos de tiempo), solo lectura |
| Tamaño del paso de tiempo | `dt` en segundos |
| Número de pasos de tiempo | cuántos pasos se calculan |
| Frecuencia de salida | escribe un archivo de resultados cada N pasos; `0` solo escribe el último paso |

Un problema térmico no tiene un grupo de configuración propio. Todo lo demás se
asigna por entidad en las pestañas de condiciones.

### 2.3 Pestaña Material

La pestaña `Material` muestra el material asignado a la entidad seleccionada. La
transferencia de calor requiere **las cuatro** propiedades siguientes:

| Propiedad | Unidades | Comentario |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, unos `50` para el acero estructural, `15` para el inoxidable, `200` para el aluminio |
| Heat capacity | `J/(kg*K)` | `c`, unos `460` para el acero |
| Density | `kg/m^3` | `rho`, unos `7850` para el acero |
| Emissivity | `-` | solo la utiliza la transferencia de calor por radiación, pero es igualmente obligatoria |

Una entidad a la que le falte alguna no se resuelve, y la comprobación de la
configuración avisa antes de que empiece el cálculo. Si no realiza transferencia
de calor por radiación, una emisividad de `0` basta para cumplir el requisito.

El material también tiene que ser un **sólido**. Un volumen cuyo material sea un
gas o un líquido (o no tenga estado definido y tenga viscosidad dinámica) se deja
al tipo de problema *Heat transfer in fluids*, aunque tenga las cuatro
propiedades.

Cada propiedad es una tabla en función de la temperatura, por lo que se tiene en
cuenta una conductividad dependiente de la temperatura, evaluada a la
temperatura del elemento del cálculo anterior, lo que hace que merezca la pena
iterar la tarea cuando la dependencia es fuerte.

### 2.4 Pestaña Condiciones de contorno

La pestaña se divide en una lista de gestión de las condiciones de contorno
disponibles, arriba, y un editor de la condición seleccionada, abajo. Seleccione
primero una entidad en el árbol `Modelo`; el gestor ofrece entonces las
condiciones válidas para ese tipo de entidad y para el problema seleccionado.
Marque una para asignarla y luego edite sus componentes en el árbol inferior.

Se ofrecen las condiciones de la sección 1.4. Notas sobre su uso:

- **Temperature** es la única condición que mantiene un valor en lugar de
  impulsarlo. Asígnela a la cara, arista o punto cuya temperatura conoce.
- **Heat** se ofrece para todos los tipos de entidad y toma la potencia total en
  `W`; el solver la reparte por usted sobre la entidad. Es la condición que debe
  utilizar cuando conoce la potencia de lo que genera el calor.
- **Heat rate (unit volume)** en un volumen y **Heat rate (unit area)** en una
  superficie imponen la misma fuente como densidad, para una fuente especificada
  por naturaleza por unidad de volumen o de área.
- Las tres condiciones de **convección** solo se aplican a superficies. Asigne
  *Simple convection* salvo que tenga un motivo concreto para que el solver
  correlacione el coeficiente por usted; es la más predecible. *Forced
  convection* en una pared que limita con un fluido mallado acopla el sólido a la
  resolución térmica del fluido y no lee ninguna de sus componentes; en otros
  casos, correlaciona el coeficiente a partir de ellas.
- Cada componente es una tabla en función del tiempo, por lo que a una
  temperatura impuesta o a una fuente de calor se le puede dar un historial
  temporal para un cálculo transitorio. Así se modela un ciclo de trabajo o un
  calefactor con rampa.

### 2.5 Condiciones iniciales y de entorno

La pestaña `Condiciones iniciales` ofrece *Temperature*, que define el campo de
partida de un cálculo transitorio. Asígnela a todo el modelo con la temperatura
a la que empieza la pieza.

La pestaña `Condiciones de entorno` también ofrece *Temperature*, como valor de
fondo para una entidad. Ambas solo se leen en el primer cálculo, y una condición
de contorno prevalece sobre las dos.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver térmico utiliza la entrada **CG**.

Si una resolución estacionaria no converge, la causa habitual es un modelo sin
temperatura impuesta ni superficie con convección, más que un parámetro del
solver. Compruebe las condiciones de contorno antes de aumentar el número de
iteraciones.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra; para un problema
térmico, *Temperature*. En un cálculo transitorio, el historial se muestra con
`Informe` -> `Puntos de monitorización`, que es la forma más rápida de leer en un
modelo una curva de calentamiento o una constante de tiempo.

### 2.8 Resultados y registros

La pestaña `Resultados` muestra las variables calculadas y controla la vista 3D.
La temperatura es un escalar en los nodos, el flujo de calor un vector en los
elementos y el coeficiente de transferencia de calor un escalar en los elementos
que es nulo fuera de las superficies con convección.

La pestaña `Registros` del panel `Modelo` muestra los registros de resultados:

- **análisis estacionario**: un único registro;
- **análisis transitorio**: un registro por cada paso de tiempo escrito. Al
  animar los registros se reproduce el calentamiento.

`Informe` -> `Archivo de registro del solver` muestra la salida completa del
solver, incluidas las estadísticas de temperatura, flujo de calor y coeficiente
de transferencia de calor después de cada registro.

---

## 3. Tutorial - transferencia de calor estacionaria

**Objetivo.** Calcular la distribución de temperatura de una barra mantenida
caliente en un extremo y enfriada por el aire circundante, y leer la temperatura
en el extremo frío.

Este tutorial supone un modelo de volumen mallado; una caja alargada funciona
bien. Utilice `Archivo` -> `Nuevo modelo`, dibuje una caja con `Geometría` ->
`Crear elemento` o importe una geometría, y mállela en tetraedros; el tutorial
*Dibujar un cubo* trata este tema. Marque la cara del extremo caliente y las
caras restantes como entidades de superficie independientes para poder
asignarles condiciones.

### Paso 1 - seleccionar el problema

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada una tarea y seleccione **Heat transfer**.
3. Confirme con `Aceptar`.

### Paso 2 - asignar el material

1. Seleccione la entidad de volumen en el árbol `Modelo`.
2. Abra la pestaña `Material` y asigne un material con las cuatro propiedades;
   para acero estructural:
   - **Thermal conductivity** (conductividad térmica) = `50` `W/(m*K)`
   - **Heat capacity** (capacidad calorífica) = `460` `J/(kg*K)`
   - **Density** (densidad) = `7850` `kg/m^3`
   - **Emissivity** (emisividad) = `0`

Una resolución estacionaria no utiliza la capacidad calorífica ni la densidad,
pero deben estar presentes para que la entidad se resuelva.

### Paso 3 - mantener el análisis estacionario

Abra la pestaña `Problema` y deje **Activar** desmarcado en el grupo *Solver
temporal*. El solver ensambla y resuelve entonces `K*T = f` una sola vez.

### Paso 4 - imponer el extremo caliente

1. Seleccione la entidad de superficie del extremo caliente.
2. Abra la pestaña `Condiciones de contorno` y marque **Temperature**.
3. Ajuste el valor a `353.15` K, es decir, 80 grados Celsius.

Recuerde que en Range FEA todas las temperaturas son absolutas, en kelvin.

### Paso 5 - enfriar el resto

1. Seleccione la entidad de superficie que cubre las caras restantes.
2. Abra la pestaña `Condiciones de contorno` y marque **Simple convection**.
3. Ajuste **Convection coefficient** = `10` `W/(m^2*K)` y **Fluid temperature** =
   `293.15` K.

Un coeficiente de unos `10` es representativo de una superficie en aire en
calma; el aire forzado está más cerca de `50`, y el agua está en los cientos o
miles.

Este es el paso que hace resoluble el modelo. Sin él, la barra tiene un aporte de
calor y ninguna evacuación, y no existe un régimen estacionario.

### Paso 6 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. La comprobación de la configuración indica los materiales o condiciones de
   contorno que faltan antes de que empiece el cálculo.
3. Siga el progreso en `Informe` -> `Archivo de registro del solver`.

### Paso 7 - examinar los resultados

- Pestaña `Resultados`: muestre **Temperature**. El campo debería descender
  suavemente desde el extremo caliente impuesto hacia la temperatura del fluido.
- El punto más frío debería mantenerse por encima de la temperatura del fluido.
  Una temperatura inferior a `Tf` en cualquier lugar, o superior a los
  `353.15` K impuestos, significa que algo aporta energía en sentido contrario:
  compruebe el signo de cualquier fuente de calor.
- **Heat flux** muestra por dónde circula la energía, en `W/m^2`. Multiplique el
  valor en las caras con convección por su área y compárelo con la potencia que
  entra en la barra.
- **Heat transfer coefficient** debería indicar `10` en las caras con convección
  y `0` en el resto. Es la forma más rápida de confirmar que una condición de
  convección llegó a los elementos previstos, y la única de ver lo que calculó una
  condición correlacionada.
- Un punto de monitorización sobre *Temperature* en el extremo frío da la
  respuesta como número en lugar de como color.

### Paso 8 - opcional, añadir una fuente de calor

Para modelar un calefactor integrado en la barra, seleccione la entidad de
volumen, abra la pestaña `Condiciones de contorno` y marque **Heat**. Para aportar
a la barra un total de `P` vatios, introduzca `P`; el solver lo reparte sobre el
volumen de la entidad.

Utilice en su lugar **Heat rate (unit volume)** cuando la fuente esté
especificada por unidad de volumen; introduzca `P / V` `W/m^3` para aportar el
mismo total (véase 1.4).

La temperatura estacionaria tiene que subir entonces hasta que la superficie con
convección pueda evacuar exactamente esa potencia, que es la comprobación del
balance de energía de la sección 5.

### Paso 9 - opcional, tensión térmica

Para llevar el resultado a un análisis estructural:

1. Añada una tarea **Stress analysis** *después* de la tarea térmica en el flujo
   de tareas del problema y asigne a la entidad las propiedades estructurales que
   necesita.
2. Asigne a la entidad estructural una condición de entorno **Temperature** con la
   temperatura de referencia sin tensiones, por ejemplo `293.15` K.

El solver de tensiones recoge las temperaturas calculadas en los elementos y
añade automáticamente el término de dilatación térmica.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La resolución no converge | no hay condición Temperature ni superficie con convección, por lo que el régimen estacionario no está definido |
| *Entity has material assigned which is missing required properties* | falta una de las cuatro propiedades requeridas, a menudo la emisividad |
| Todo el modelo está a una única temperatura | no entra calor, o nada genera un gradiente respecto al valor impuesto |
| Las temperaturas son absurdamente altas | se introdujo una densidad en `W/m^3` en una condición *Heat*, que toma el total en vatios, o el coeficiente de convección es demasiado pequeño |
| Una pared entre sólido y fluido no se enfría | la tarea térmica de fluidos se ejecuta después de la tarea térmica, o el grupo ejecuta una sola iteración: ponga primero la tarea térmica de fluidos y deje que el grupo itere |
| La resolución térmica ignora el fluido | es lo previsto: un material fluido solo lo resuelve *Heat transfer in fluids* |
| Las temperaturas salen cercanas a `293.15` K en todas partes | la entidad no se resolvió: busque en el registro una propiedad de material que falte |

---

## 4. Tutorial - transferencia de calor transitoria

**Objetivo.** Partir de la misma barra a temperatura ambiente, encender el
extremo caliente y averiguar cuánto tarda en calentarse.

### Paso 1 - seleccionar el problema

La misma tarea **Heat transfer** que en el tutorial 3. Si el modelo sigue
configurado, solo hay que añadir la configuración del solver temporal y la
condición inicial.

### Paso 2 - asignar el material

Las mismas cuatro propiedades. Aquí importan la **capacidad calorífica** y la
**densidad**: junto con la conductividad determinan la difusividad térmica
`k/(rho*c)`, que es lo que rige la rapidez con que avanza el frente.

### Paso 3 - definir la temperatura inicial

1. Seleccione la entidad de volumen.
2. Abra la pestaña `Condiciones iniciales` y marque **Temperature**.
3. Ajústela a `293.15` K.

Sin ella, el modelo parte igualmente de los `293.15` K predeterminados, pero
indicarla explícitamente hace que el modelo se documente a sí mismo y permite
partir de cualquier otro estado.

### Paso 4 - configurar el solver temporal

Abra la pestaña `Problema` y, en el grupo *Solver temporal*, configure:

| Campo | Valor |
|---|---|
| Activar | marcado |
| Aproximación | `Backward difference (stable)` |
| Tiempo inicial | `0` |
| Tamaño del paso de tiempo | `1` |
| Número de pasos de tiempo | `300` |
| Frecuencia de salida | `10` |

*Backward difference* es el esquema implícito (véase la tabla de 1.8) y es el que
debe usarse aquí. Se mantiene estable con un paso de tiempo elegido según la
física y no según la malla.

Elija el paso de tiempo a partir de la constante de tiempo esperada, con el
objetivo de tener unos cincuenta pasos en la parte interesante de la curva. La
sección 5 da una fórmula para una primera estimación. Escribir uno de cada diez
pasos mantiene pequeño el archivo de resultados sin perder la forma de la curva.

### Paso 5 - conservar las condiciones de contorno

La **Temperature** impuesta en el extremo caliente y la **Simple convection** en
las caras restantes se mantienen sin cambios. Cada componente es una tabla en
función del tiempo, por lo que al extremo caliente se le puede dar en su lugar
una rampa: abra el editor de componentes y añada instantes para que la
temperatura suba durante los primeros segundos en lugar de cambiar de golpe.

### Paso 6 - añadir un punto de monitorización

`Problema` -> `Definir puntos de monitorización`, coloque un punto en el extremo
frío y ajuste su variable a **Temperature**. La curva de calentamiento se lee
mucho más fácilmente como historial que como secuencia de imágenes coloreadas.

### Paso 7 - resolver

`Solución` -> `Iniciar solver` (`Ctrl+R`). El registro imprime un bloque por paso
de tiempo:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

(`time step: 1 of 300` = paso de tiempo 1 de 300), seguido de las iteraciones del
solver matricial y de las estadísticas de temperatura de ese paso.

### Paso 8 - examinar los resultados

- Panel `Modelo`, pestaña `Registros`: un registro por paso escrito. Recórralos o
  anímelos para ver cómo penetra en la barra el frente de calor.
- Pestaña `Resultados`: muestre **Temperature**. Fije el rango de visualización
  para todos los registros, de modo que los colores signifiquen lo mismo en cada
  fotograma; de lo contrario, la animación se reescala sola y el avance no se ve.
- `Informe` -> `Puntos de monitorización` traza la curva de calentamiento en el
  extremo frío. Lea en ella la constante de tiempo: el tiempo necesario para
  cubrir aproximadamente el 63 por ciento del aumento total.
- El último registro debería aproximarse al resultado estacionario del
  tutorial 3. Si no es así, el cálculo fue demasiado corto.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La temperatura oscila y crece de un paso a otro | esquema explícito con un paso de tiempo demasiado grande: cambie la aproximación a *Backward difference (stable)* o reduzca `dt` |
| El resultado apenas cambia en todo el cálculo | el paso de tiempo es muy inferior a la constante de tiempo, o el cálculo es demasiado corto |
| El primer paso salta a la respuesta final | el paso de tiempo es muy superior a la constante de tiempo, por lo que el avance es en la práctica estacionario |
| Solo se escribió un registro | la frecuencia de salida es `0`, que solo escribe el último paso |
| El transitorio nunca alcanza el régimen estacionario | a menudo es correcto: compárelo con el cálculo estacionario antes de sospechar del solver |

---

## 5. Comprobación de un modelo

El solver no tiene un conjunto de verificación integrado, por lo que merece la
pena validar un modelo nuevo con algo que pueda calcularse a mano antes de
confiar en él. Cuatro comprobaciones baratas, por orden de utilidad:

**Pared plana.** Para una placa de espesor `L` y conductividad `k` mantenida a
`T1` y `T2` en sus dos caras, la temperatura es lineal a través de la placa y el
flujo es

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Modele la placa con elementos de volumen, imponga ambas caras y confirme que el
plano medio queda exactamente a mitad de camino y que el **Heat flux** calculado
coincide con `q`. Esto detecta unidades erróneas, una conductividad errónea y una
malla que no está conectada como usted cree. Refine la malla una vez y confirme
que ambos valores no cambian.

**Pared con convección.** Para la misma placa mantenida a `T1` en una cara y
enfriada en la otra por un fluido a `Tf` con un coeficiente `h`, la superficie
enfriada se estabiliza en

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Compárelo con la temperatura de superficie calculada. Esto valida la condición
de convección, que es la parte de un modelo térmico en la que es más fácil
equivocarse.

**Constante de tiempo concentrada.** Para una comprobación transitoria, tome un
cuerpo de volumen `V` y área superficial `A` sin temperatura impuesta, que parte
de `T0` y se enfría solo por convección. Mientras el número de Biot `h*V/(A*k)`
se mantenga por debajo de aproximadamente `0.1`, el cuerpo es casi isotermo y

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Lea `tau` en el historial del punto de monitorización y compárelo. Esto valida a
la vez la matriz de capacidad, la condición de convección y su paso de tiempo, y
es la forma más rápida de elegir un paso de tiempo para un modelo real, ya que
`tau/50` es una buena primera estimación.

**Balance de energía.** Para un modelo estacionario con una potencia de entrada
conocida `P`, la superficie con convección debe evacuar exactamente `P` a la
temperatura final:

```
P = h * A * ( Ts_mean - Tf )
```

Lea la temperatura media de la superficie en las estadísticas del registro del
solver y compruebe el balance. Es la única comprobación global que detecta un
valor Heat introducido como total en lugar de como densidad.

Para cualquiera de ellas, refine la malla al menos una vez y confirme que la
respuesta converge en lugar de derivar.

---

## 6. Limitaciones

### Limitaciones de modelado

- **Solo conducción lineal.** La conductividad, la capacidad y la densidad de un
  elemento son constantes durante una resolución, leídas de la tabla del material
  a la temperatura del cálculo anterior. Una propiedad muy dependiente de la
  temperatura necesita un grupo de tareas que itere para converger.
- **Sin cambio de fase.** No hay calor latente ni formulación en entalpía, por lo
  que no se modelan la fusión, la solidificación, la ebullición ni la
  condensación. Una tabla de material no puede representar la meseta latente.
- **Sin advección.** La energía se mueve por conducción dentro del cuerpo y por
  convección a través de su contorno. Un medio en movimiento no transporta
  energía consigo; para eso utilice *Heat transfer in fluids*.
- **La convección es un modelo de contorno.** Cada una de las formas
  correlacionadas utiliza una única correlación fija (placa plana externa para el
  flujo forzado, placas horizontales para la convección natural) con el diámetro
  hidráulico como longitud característica. No se detectan la orientación de la
  superficie, el régimen del flujo ni si el flujo es interno o externo. Cuando el
  coeficiente importa, calcúlelo usted mismo e introdúzcalo con *Simple
  convection*.
- **El coeficiente de convección natural va con retraso.** Se evalúa a la
  temperatura de pared de la resolución anterior, por lo que un cálculo
  estacionario de una sola pasada utiliza en la práctica la diferencia de
  temperatura inicial.
- **Radiación solo mediante el tipo de problema independiente.** El propio solver
  térmico no tiene término de emisividad superficial; la radiación de recinto
  requiere una tarea *Radiative heat transfer*, su matriz de factores de visión y
  una iteración entre ambas.
- **Las superficies son isotermas en su espesor.** Una entidad de superficie solo
  conduce en su propio plano. Modele con elementos de volumen una pared cuyo
  gradiente a través del espesor importe.
- **Los elementos de punto no conducen.** Un punto es una capacidad concentrada y
  una fuente de calor, nada más.
- **Un modelo estacionario necesita una evacuación.** Sin temperatura impuesta ni
  superficie con convección, la matriz es singular y la resolución no
  convergerá.
