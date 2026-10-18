# Transferencia de calor en fluidos - Manual de teoría y guía del usuario

Este documento describe el solver de transferencia de calor en fluidos de
Range FEA - `RSolverFluidHeat`, que atiende el tipo de problema
**Transferencia de calor en fluidos** (*Heat transfer in fluids*): la ecuación
que resuelve, el significado de cada dato de entrada que acepta, su acoplamiento
con el solver térmico de un sólido y las partes de la interfaz gráfica que lo
controlan.

La transferencia de calor en fluidos se resuelve sobre el campo de flujo de una
tarea **Flujo viscoso incompresible** (*Incompressible viscous flow*). El solver
de flujo, sus condiciones de contorno, su iteración no lineal y su avance en el
tiempo se describen en el manual [Flujo de fluidos](theory_fluid.md), y el
solver térmico de sólidos en el manual
[Transferencia de calor](theory_heat.md).

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Limitaciones](#3-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnita principal - la temperatura

La incógnita principal es la **temperatura** `T` en `K`, un grado de libertad
por nodo.

| Tipo de problema | Solver | Incógnita principal | Unidades | Grados de libertad por nodo |
|---|---|---|---|---|
| Heat transfer in fluids | `RSolverFluidHeat` | temperatura | `K` | 1 |

Este tipo de problema **requiere** una tarea *Incompressible viscous flow* en el
mismo problema: lee el campo de velocidad de su resultado y nunca lo calcula por
sí mismo.

### 1.2 Ecuación fundamental

**Heat transfer in fluids** resuelve la advección-difusión de la temperatura
sobre el campo de flujo,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

con la misma estabilización SUPG del término convectivo que el solver de flujo y
el mismo esquema temporal theta. La velocidad `v` se **toma del resultado del
flujo** y no se resuelve aquí; por eso este tipo de problema requiere una tarea
*Incompressible viscous flow*.

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `T` | temperatura | `K` | incógnita resuelta |
| `v` | velocidad | `m/s` | resultado del flujo |
| `k` | conductividad térmica | `W/(m*K)` | propiedad del material |
| `c` | capacidad calorífica | `J/(kg*K)` | propiedad del material |
| `rho` | densidad | `kg/m^3` | propiedad del material |
| `q` | densidad de fuente de calor | `W/m^3` | condición Heat, calor Joule, radiación |

La diferencia con el tipo de problema simple **Heat transfer** es precisamente
el término `v . grad(T)`: la energía es transportada por el fluido en movimiento
y no solo conducida. Esto significa también que este solver no tiene
condiciones de contorno de convección: no hay nada que correlacionar, ya que el
calor que pasa de una pared al fluido lo resuelve la malla en lugar de
modelarse.

### 1.3 Condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Temperature | explícita (Dirichlet) | punto, línea, superficie, volumen | Temperature `[K]` |
| Heat | natural, fuente | punto, línea, superficie, volumen | Heat `[W]` |

**Temperature** (temperatura) mantiene los nodos de su entidad en el valor
dado. Asígnela a la superficie de entrada para fijar la temperatura del fluido
que entra en el dominio, y a una pared para modelar un contorno calentado o
enfriado.

**Heat** (calor) es una fuente y toma la **potencia total en `W`** aportada a la
entidad, repartida sobre la medida de la entidad antes del ensamblaje: la misma
convención que en el solver térmico. *Heat rate (unit area)* y *Heat rate (unit
volume)* aparecen en la interfaz para este tipo de problema, pero el solver
térmico de fluidos **no las lee**; utilice aquí *Heat* para una fuente.

Cada componente es una **tabla en función del tiempo**.

Las condiciones del flujo (paredes, entradas y salidas) pertenecen a la tarea de
flujo y se describen en el manual [Flujo de fluidos](theory_fluid.md).

### 1.4 Transferencia de calor conjugada

Una pared entre un fluido mallado y un sólido mallado que lleve la condición
**Forced convection** (convección forzada) del solver simple *Heat transfer*
acopla ambos. Una vez ejecutado el solver térmico, el solver térmico de fluidos
mantiene los nodos de la pared a la temperatura del sólido (hasta entonces la
pared está aislada) y devuelve, para cada elemento de pared, el par `(h, Tf)`
que reproduce el flujo de calor que su solución hace pasar a través de la pared.
El solver térmico aplica ese par como condición de convección. El flujo es el
residuo de las ecuaciones del fluido en los nodos de la pared, coherente con la
discretización, por lo que sigue siendo preciso en una capa límite térmica
delgada. La temperatura de la pared se relaja de una pasada a otra con un factor
de Aitken, lo que estabiliza el intercambio en unas pocas pasadas. El manual
[Transferencia de calor](theory_heat.md) describe el esquema en *Transferencia
de calor conjugada*. El calor radiado y el calor Joule procedentes de otros
solvers se añaden al término fuente igual que en el solver térmico.

### 1.5 Convergencia

Sin acoplamiento, el solver térmico de fluidos se declara convergido
incondicionalmente: es lineal una vez fijado el campo de flujo, de modo que una
sola resolución da la respuesta final. Por tanto, un grupo que contenga tanto la
tarea de flujo como la tarea térmica de fluidos termina cuando converge la tarea
de flujo. Con paredes acopladas, en cambio, indica la variación relativa de su
campo de temperatura, de modo que el grupo itera hasta que la temperatura de la
pared se estabiliza.

### 1.6 Acoplamiento de vuelta al flujo - flotabilidad

La temperatura que produce este solver se devuelve al flujo. En cada pasada, el
solver de flujo evalúa la densidad y la viscosidad dinámica de cada elemento a la
última temperatura, de modo que, con un material cuya densidad dependa de la
temperatura, la fuerza volumétrica `rho(T)*g` impulsa la **convección
natural**: el fluido caliente sube y el frío baja. Los materiales suministrados
*Water* y *Air* tienen densidad dependiente de la temperatura.

En un cálculo estacionario, la primera pasada del flujo solo ve la temperatura
inicial, por lo que la flotabilidad aparece a partir de la segunda iteración del
grupo; dé al grupo varias iteraciones y un valor de convergencia. En un cálculo
transitorio con una iteración por paso de tiempo, el flujo ve la temperatura del
paso anterior. El manual [Flujo de fluidos](theory_fluid.md) describe el esquema
en la sección 1.9.

### 1.7 Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Temperature | campo de temperatura inicial |

Las condiciones iniciales solo se leen **en el primer cálculo**; después, el
campo avanza a partir del resultado anterior. Una condición de contorno
prevalece sobre una condición inicial allí donde coinciden.

### 1.8 Resultados

| Resultado | Se aplica a | Significado |
|---|---|---|
| Temperature `[K]` | nodo | el campo de temperatura resuelto |
| Heat flux `[W/m^2]` | elemento | flujo conductivo `-k*grad(T)` |

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Material`, `Resultados`),
el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Las combinaciones que conviene conocer:

- *Incompressible viscous flow* y *Heat transfer in fluids* en un mismo grupo:
  un flujo calentado o enfriado. Dé al grupo varias iteraciones cuando el flujo
  esté impulsado o claramente modificado por la flotabilidad, para que el flujo
  y la temperatura se estabilicen juntos (véase 1.6);
- *Heat transfer in fluids* y una tarea *Heat transfer* independiente sobre un
  sólido mallado, primero la tarea térmica de fluidos, en un mismo grupo de
  varias iteraciones: transferencia de calor conjugada a través de las paredes
  que llevan *Forced convection*.

Este tipo de problema requiere que haya una tarea de flujo. Los valores del
grupo de tareas, **N.º de iteraciones** y **Convergencia**, se describen en el
manual [Flujo de fluidos](theory_fluid.md).

### 2.2 Pestaña Problema

**Solver temporal**: se muestra porque este tipo de problema depende del tiempo.
Déjelo desactivado para un cálculo estacionario. Sus campos se describen en el
manual [Flujo de fluidos](theory_fluid.md).

Este tipo de problema no tiene un grupo de configuración propio. Todo lo demás
se asigna por entidad en las pestañas de condiciones.

### 2.3 Pestaña Material

| Propiedad | Unidades | Comentario |
|---|---|---|
| Density | `kg/m^3` | `rho`, unos `1000` para el agua, `1.2` para el aire |
| Dynamic viscosity | `kg/(m*s)` | `mu`, unos `1.0e-3` para el agua, `1.8e-5` para el aire |
| Thermal conductivity | `W/(m*K)` | `k`, unos `0.6` para el agua, `0.025` para el aire |
| Heat capacity | `J/(kg*K)` | `c`, unos `4180` para el agua, `1005` para el aire |

Una entidad a la que le falte alguna propiedad requerida no se resuelve, y la
comprobación de la configuración avisa antes de que empiece el cálculo. La base
de datos de materiales suministrada incluye **Water** (agua) y **Air** (aire), lo
que normalmente completa este paso. Cada propiedad es una tabla en función de la
temperatura, evaluada a la temperatura del elemento del cálculo anterior.

### 2.4 Pestaña Condiciones de contorno

**Temperature** y **Heat** aparecen en cuanto hay una tarea *Heat transfer in
fluids* en el flujo de tareas (véase la sección 1.3). Cada componente es una
tabla en función del tiempo; haga clic en **Editar valores dependientes del
tiempo** para introducir valores en función del tiempo.

### 2.5 Condiciones iniciales

La pestaña `Condiciones iniciales` ofrece *Temperature*. Define el campo de
partida del primer cálculo.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver térmico de fluidos utiliza la entrada **GMRES**,
ya que su matriz no es simétrica.

### 2.7 Puntos de monitorización y resultados

`Problema` -> `Definir puntos de monitorización` puede registrar *Temperature*.
La pestaña `Resultados` muestra la temperatura como escalar en los nodos y el
flujo de calor como vector en los elementos.

---

## 3. Limitaciones

- **Flotabilidad sin dilatación del fluido.** La temperatura modifica el flujo a
  través de `rho(T)` y `mu(T)` (véase 1.6), pero la dilatación térmica del fluido
  nunca entra en el balance de masa `div(v) = 0`. Esto es preciso para las
  pequeñas diferencias de densidad de la convección natural, pero no para un gas
  calentado cientos de kelvin. El flujo por flotabilidad es laminar, como todos
  los flujos en Range FEA.
- **El acoplamiento es escalonado, no monolítico.** El flujo y la temperatura se
  resuelven uno tras otro e intercambian datos entre pasadas. Un flujo
  estacionario con flotabilidad necesita un grupo de varias iteraciones; con una
  iteración por paso de tiempo, un flujo transitorio va un paso por detrás de la
  temperatura.
- **Las condiciones `Heat rate` se ignoran.** *Heat rate (unit area)* y *Heat
  rate (unit volume)* aparecen en la interfaz para *Heat transfer in fluids*,
  pero `RSolverFluidHeat` no las lee; solo le llega la condición *Heat*. El tipo
  de problema simple *Heat transfer* sí lee las tres.
- **Sin condiciones de contorno de convección.** El calor que pasa de una pared
  a un fluido mallado se resuelve en lugar de correlacionarse, por lo que las
  condiciones *Simple*, *Forced* y *Natural convection* pertenecen al solver
  térmico simple. La transferencia de calor conjugada se configura como una
  tarea *Heat transfer* sobre el sólido, cuyas paredes *Forced convection* leen
  el estado calculado del fluido.
- **Sin modelo de turbulencia.** El flujo es laminar (véase el manual
  [Flujo de fluidos](theory_fluid.md)), por lo que no se representa la
  transferencia de calor turbulenta.
