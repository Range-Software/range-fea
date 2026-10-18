# Análisis de tensiones - Manual de teoría y guía del usuario

Este documento describe el solver estructural (`RSolverStress`) de Range FEA, que
atiende los tipos de problema **Análisis de tensiones** (*Stress analysis*) y
**Análisis modal** (*Modal analysis*): las ecuaciones que resuelve, el
significado de cada dato de entrada que acepta, las partes de la interfaz gráfica
que lo controlan y dos tutoriales completos.

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - análisis estático de tensiones](#3-tutorial---análisis-estático-de-tensiones)
4. [Tutorial - análisis modal](#4-tutorial---análisis-modal)
5. [Comprobación de un modelo](#5-comprobación-de-un-modelo)
6. [Limitaciones](#6-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnita principal - el campo de desplazamientos

La incógnita principal es el **vector desplazamiento** `u = (ux, uy, uz)` en cada
nodo, en metros. Cada nodo tiene tres grados de libertad, por lo que una malla de
`N` nodos da un sistema de `3N` ecuaciones. Las deformaciones, las tensiones y las
fuerzas de reacción se obtienen todas a partir del campo de desplazamientos tras
la resolución.

### 1.2 Ecuaciones fundamentales

El solver implementa la **elasticidad lineal, isótropa y con pequeñas
deformaciones**. Tres relaciones la definen.

**Equilibrio** de un cuerpo sometido a una carga volumétrica `b` y a una tracción
superficial `t`:

```
div(sigma) + b = 0
```

Relación **deformación-desplazamiento**, en pequeñas deformaciones:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

Relación **constitutiva** con un término térmico:

```
sigma = D * ( eps - alpha * dT )
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `u` | desplazamiento | `m` | incógnita resuelta |
| `sigma` | tensión de Cauchy | `Pa` | obtenida |
| `eps` | deformación ingenieril | `-` | obtenida |
| `E` | módulo de elasticidad | `Pa` | propiedad del material |
| `v` | coeficiente de Poisson | `-` | propiedad del material |
| `rho` | densidad | `kg/m^3` | propiedad del material |
| `alpha` | coeficiente de dilatación térmica | `1/K` | propiedad del material |
| `dT` | aumento de temperatura respecto al estado sin tensiones | `K` | véase 1.5 |

Las cuatro propiedades de material son **obligatorias**: una entidad a la que le
falte alguna no se resuelve. Los dos coeficientes de amortiguamiento de Rayleigh
de un análisis dinámico (véase 1.7) son opcionales.

### 1.3 Matriz de elasticidad

Los **elementos de volumen** utilizan la matriz de elasticidad isótropa
tridimensional completa en notación de Voigt, con el vector de tensiones ordenado
como `[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

Los **elementos de superficie** utilizan la forma bidimensional de **deformación
plana** con el vector de tensiones `[sigma_xx, sigma_yy, tau_xy]`, evaluada en el
plano local del elemento y girada de vuelta al sistema de coordenadas global:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Se trata de **deformación** plana, no de tensión plana. Por tanto, una placa
delgada cargada en su propio plano resultará más rígida de lo que predeciría una
idealización en tensión plana.

Los **elementos de línea** se tratan como **barras articuladas**: solo se
ensambla la rigidez axial `E * A` a lo largo de la dirección del elemento, siendo
`A` la sección de la línea. No hay rigidez a flexión, a cortante ni a torsión,
ni siquiera para los tipos de elemento denominados «beam» (viga).

### 1.4 Discretización por elementos finitos

La forma débil del equilibrio da, para cada elemento,

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(`stiffness matrix` = matriz de rigidez, `consistent mass matrix` = matriz de
masa consistente, `load vector, see 1.5` = vector de cargas, véase 1.5), donde `B`
es la matriz deformación-desplazamiento construida a partir de las derivadas de
las funciones de forma. Cada matriz de elemento es `3n x 3n` para un elemento de
`n` nodos, con los grados de libertad intercalados como
`[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Medidas de los elementos.** Como en el resto de Range FEA, a las entidades de
menor dimensión se les asigna una medida geométrica para que puedan representar
cuerpos reales:

| Entidad | Medida | Dónde se define |
|---|---|---|
| Volumen | 1 | - |
| Superficie | espesor de la superficie | propiedades geométricas de la entidad |
| Línea | sección de la línea | propiedades geométricas de la entidad |
| Punto | volumen del punto | propiedades geométricas de la entidad |

Una superficie de espesor nulo o una línea de sección nula no aporta rigidez ni
masa, aunque puede seguir llevando cargas, que es exactamente lo que se quiere
para una cara cargada de un sólido mallado.

La matriz de masa solo se ensambla cuando es necesaria, es decir, en un análisis
transitorio dinámico o en un análisis modal.

### 1.5 Cargas, condiciones de contorno y de entorno

#### Condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Displacement | explícita (Dirichlet) | punto, línea, superficie, volumen | Displacement in X, Y, Z `[m]` |
| Normal displacement | explícita, sistema local | superficie | Displacement `[m]` |
| Roller displacement | explícita, sistema local | punto, línea, superficie | Displacement `[m]` |
| Force | natural | punto, línea, superficie | Force in X, Y, Z `[N]` |
| Force (unit area) | natural | superficie | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | natural | superficie | Pressure `[Pa]` |
| Weight | natural | punto, línea, superficie | Weight `[kg]` |

**Displacement** (desplazamiento) es una condición con componentes
**opcionales**: cada una de sus tres componentes puede activarse o desactivarse
por separado. Una componente activada restringe esa dirección global de cada nodo
de la entidad al valor dado; una componente desactivada deja esa dirección
totalmente libre. Por tanto, desactivar *Displacement in Y* y *Displacement in Z*
crea un apoyo que fija X y deja que el nodo se deslice en el plano YZ.

Utilice en su lugar *Roller displacement* cuando las direcciones libres no sean
las globales, sino el plano tangente de una superficie curva.

**Normal displacement** (desplazamiento normal) y **Roller displacement**
(desplazamiento de apoyo deslizante) trabajan en un **sistema local** construido
para cada nodo restringido. Por defecto, el sistema sigue la geometría:

- en una **superficie**, el eje X local es la normal promediada de los elementos
  de superficie adyacentes;
- en una **línea**, el sistema local se construye a partir de la dirección del
  elemento;
- en un **punto** no hay geometría que seguir, por lo que siempre se utiliza la
  dirección introducida con la condición de contorno.

Marcar **Usar la dirección local introducida** sustituye este comportamiento en
una superficie o una línea y toma el eje X local de la dirección introducida
debajo. Utilícelo cuando la dirección que desea restringir no sea la que resulta
de la malla, por ejemplo un apoyo deslizante que debe deslizarse sobre una
superficie facetada a lo largo de un eje global.

Las restricciones de distintas entidades se **combinan** en un nodo que
comparten. El solver reúne todas las restricciones de desplazamiento que actúan
sobre un nodo, cada una de ellas una relación `d . u = v` sobre una dirección, y
reduce el conjunto a un máximo de tres direcciones restringidas mutuamente
perpendiculares mediante Gram-Schmidt, aplicando las mismas operaciones a los
valores impuestos. Esas direcciones restringidas pasan a ser los primeros ejes
del sistema del nodo; lo que queda completa el sistema y permanece libre. Así, un
nodo puede estar restringido en una dirección por un *Displacement* global, en
otra por un *Roller displacement* sobre una cara inclinada y quedar libre en la
tercera: las dos condiciones no se sobrescriben entre sí, y una componente
expresada en el sistema global nunca se reinterpreta silenciosamente en el
sistema local de otra condición.

Dos entidades que imponen **valores distintos en la misma dirección** no pueden
satisfacerse a la vez. El solver lo detecta durante la ortogonalización y se
detiene con un error que indica el nodo, en lugar de resolverlo a favor de la
condición que se haya leído en último lugar.

Las matrices de elemento y los vectores de cargas de un nodo restringido se giran
a ese sistema, se aplica la restricción y el desplazamiento resultante se gira de
vuelta. En ese sistema:

- **Normal displacement** bloquea las tres componentes locales, y el escalar
  impuesto se aplica a lo largo de la normal local. El nodo se desplaza
  exactamente la cantidad dada a lo largo de la normal y queda fijo en lo demás.
- **Roller displacement** solo bloquea la componente normal local en una
  superficie o un punto, por lo que el nodo puede deslizarse libremente en el
  plano tangente. En una entidad de línea bloquea en cambio las dos componentes
  perpendiculares a la línea, por lo que el nodo puede deslizarse libremente a lo
  largo de ella.

**Force** (fuerza) es una fuerza total en newtons repartida sobre la entidad a la
que se asigna: se divide por la longitud de la entidad en una línea, por el área
de la entidad en una superficie y por el número de puntos en una entidad de
puntos. Por tanto, refinar la malla no cambia la carga total.

**Force (unit area)** (fuerza por unidad de área) es una tracción en `N/m^2`
aplicada en la dirección global dada, integrada sobre el área del elemento.

**Pressure** (presión) actúa según la **normal** a la superficie. El solver
determina para cada superficie de contorno si su normal apunta hacia el elemento
de volumen calculable adyacente y cambia el signo en consecuencia, de modo que una
presión positiva siempre empuja sobre el cuerpo. Una superficie que no delimita
ningún elemento de volumen nunca se clasifica como orientada hacia dentro, por lo
que en ella la carga actúa según la **opuesta** de la normal a la superficie;
compruebe en ese caso la dirección de la carga resultante.

**Weight** (peso) es una masa en kilogramos unida a la entidad. Se multiplica por
la aceleración de la gravedad de la condición de entorno y se reparte sobre la
entidad del mismo modo que Force. Utilícela para masas unidas no modeladas.

#### Condiciones de entorno

| Condición de entorno | Componentes | Efecto |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | peso propio y condición de contorno Weight |
| Temperature | Temperature `[K]` | temperatura de referencia sin tensiones |

El **peso propio** se genera automáticamente para cada entidad que tenga a la vez
densidad y medida geométrica, como `rho * g` integrado sobre el elemento. Aparece
en cuanto se asigna una condición de entorno de aceleración de la gravedad; el
valor predeterminado es `-9.80665 m/s^2` en Z.

**Carga térmica.** La diferencia de temperatura que determina el término de
dilatación térmica es

```
dT = element temperature - environment temperature
```

(`element temperature` = temperatura del elemento, `environment temperature` =
temperatura del entorno.) La **temperatura del entorno** procede de la condición
de entorno *Temperature* y es el estado de referencia sin tensiones. La
**temperatura del elemento** procede de un solver de transferencia de calor
ejecutado antes en el mismo flujo de tareas del problema, transmitida a través de
los datos compartidos de los solvers. Por tanto, ejecutar *Heat transfer* y luego
*Stress analysis* en un mismo flujo de tareas da un análisis de tensiones térmicas
sin más configuración. Sin solver térmico, la temperatura del elemento se queda en
su valor predeterminado y el término térmico es la diferencia respecto a la sola
condición de entorno.

#### Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Displacement | campo de desplazamientos inicial de un análisis transitorio dinámico |
| Structural velocity | campo de velocidades inicial de un análisis transitorio dinámico |
| Pressure | carga de presión superficial inicial |

Las condiciones iniciales solo se leen en el primer cálculo de un análisis
transitorio. Un análisis dinámico sin ellas parte del reposo. Las direcciones
restringidas no conservan ninguna velocidad inicial propia, sino que siguen el
desplazamiento impuesto.

#### Acoplamiento con un solver de fluidos

El solver recupera el resultado **Pressure** de una resolución anterior y lo
utiliza como carga superficial allí donde no hay asignada ninguna condición de
contorno de presión explícita. Por tanto, una tarea de fluido seguida de una
tarea de tensiones en el mismo flujo de tareas transfiere automáticamente la
presión del fluido a la estructura.

En sentido contrario, todo solver no estructural desplaza la malla según el
resultado **Displacement** almacenado antes de ejecutarse y la devuelve después a
su posición, de modo que una resolución fluida o térmica que sigue a una
resolución de tensiones ve la forma deformada.

### 1.6 Análisis estático

Con el solver temporal desactivado y el tipo de problema ajustado a *Stress
analysis*, el solver ensambla y resuelve

```
K * u = f
```

`K` es simétrica definida positiva una vez que las restricciones de
desplazamiento han eliminado los modos de sólido rígido, por lo que se resuelve
con el método del **gradiente conjugado** con precondicionamiento de Jacobi y un
tamaño de bloque de 3.

Si el modelo está insuficientemente restringido, la matriz es singular y la
resolución no convergerá. Todo modelo estático necesita restricciones suficientes
para suprimir los seis modos de sólido rígido: tres traslaciones y tres
rotaciones.

### 1.7 Análisis transitorio

Con el solver temporal activado, la *Configuración del análisis transitorio de
tensiones* selecciona uno de dos tipos de análisis.

#### Análisis dinámico

Es la opción predeterminada. El solver integra las ecuaciones de la
elastodinámica lineal

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

de modo que se reproducen la inercia, las ondas de tensión, las vibraciones, la
amplificación dinámica y la resonancia.

**Amortiguamiento.** `C` es el amortiguamiento de Rayleigh, construido elemento a
elemento a partir de dos propiedades de material opcionales,

```
C = alpha_M * M + beta_K * K
```

*Rayleigh mass damping coefficient* `alpha_M` `[1/s]` amortigua las frecuencias
bajas y *Rayleigh stiffness damping coefficient* `beta_K` `[s]` las altas. La
razón de amortiguamiento de un modo de frecuencia angular `omega` es

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

de modo que una razón objetivo `zeta` en dos frecuencias `omega_1` y `omega_2`
se obtiene con

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Tome las dos frecuencias de un análisis modal, alrededor de la banda que le
interese. Las razones de amortiguamiento estructural típicas van de 0.01 a 0.05.
Sin las dos propiedades no hay ningún amortiguamiento físico.

**Integración temporal.** El método de Newmark con la modificación de
Hilber-Hughes-Taylor (HHT). Con el desplazamiento `u`, la velocidad `v` y la
aceleración `a` al inicio del paso, cada paso resuelve

```
( a0*M + (1+alpha)*(K + a1*C) ) * u_n+1 =
      (1+alpha)*f_n+1 - alpha*f_n
    + M * ( a0*u_n + a2*v_n + a3*a_n )
    + (1+alpha) * C * ( a1*u_n + a4*v_n + a5*a_n )
    + alpha * ( C*v_n + K*u_n )
```

```
a0 = 1/(beta*dt^2)   a1 = gamma/(beta*dt)   a2 = 1/(beta*dt)
a3 = 1/(2*beta) - 1  a4 = gamma/beta - 1    a5 = dt/2 * (gamma/beta - 2)
```

y luego actualiza

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

La matriz efectiva es simétrica definida positiva, por lo que se resuelve con el
mismo método CG que un análisis estático.

- Con **HHT alpha = 0** (predeterminado), el esquema es un Newmark simple con
  `gamma = 1/2` y `beta = max(theta/2, 1/4)`, donde `theta` es el coeficiente de
  aproximación del esquema temporal (`1` regresivo, `0.5` centrado, `0`
  progresivo). El centrado y el progresivo dan ambos el esquema de aceleración
  media: incondicionalmente estable, de segundo orden de precisión y sin
  amortiguamiento numérico. El regresivo da `beta = 1/2`, que también es
  incondicionalmente estable pero alarga los periodos.
- Con **HHT alpha** entre `-1/3` y `0`, `gamma = 1/2 - alpha` y
  `beta = (1 - alpha)^2 / 4`. El esquema sigue siendo incondicionalmente estable y
  de segundo orden de precisión, y amortigua las frecuencias altas que una malla
  gruesa resuelve mal, dejando casi intactos los modos bajos. Un valor de `-0.05`
  a `-0.1` es una elección habitual cuando la respuesta está contaminada por
  ruido de alta frecuencia.

**Aceleración inicial.** En el primer paso, la aceleración se obtiene del
equilibrio, `M*a = f - K*u - C*v`, de modo que una carga aplicada al inicio
produce la aceleración inicial correcta. Las direcciones sin masa reciben cero.

**Masa concentrada.** La opción **Matriz de masa concentrada** sustituye la masa
consistente por una diagonal de la misma masa total, escalando la diagonal de la
matriz consistente (concentración HRZ, que se mantiene positiva también para
elementos cuadráticos). Reduce ligeramente las frecuencias propias y da una
respuesta más suave detrás de los frentes de onda abruptos.

**Elección del paso de tiempo.** El esquema es estable para cualquier `dt`, pero
la precisión requiere aproximadamente 20 pasos o más por periodo del modo más
alto que se quiera resolver. Para una onda que atraviesa la malla, un paso
próximo al tiempo que necesita la onda para cruzar un elemento (`dt ~ h/c`,
`c = sqrt(E/rho)`) es un buen punto de partida.

**Resultados.** Además del desplazamiento y la tensión, un análisis dinámico
almacena los vectores en los nodos *Structural velocity* y *Structural
acceleration*. También trasladan el estado al siguiente paso de tiempo y a través
de un reinicio del solver.

#### Análisis cuasiestático

Cada paso de tiempo es una solución estática independiente

```
K * u(t) = f(t)
```

despreciando la inercia y el amortiguamiento. Utilícelo para un historial de
cargas aplicado lentamente en comparación con los periodos de la estructura,
cuando solo importa la sucesión de estados de equilibrio. No necesita densidad ni
tiene límite de paso de tiempo.

### 1.8 Análisis modal

Con el tipo de problema ajustado a *Modal analysis*, el solver ensambla `K` y `M`
y resuelve el problema generalizado de valores propios

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(`with` = con.) Cada par propio es un **modo**: `phi` es la forma modal,
almacenada y mostrada como campo de desplazamientos, y `lambda` es el valor
propio del que se deduce la frecuencia propia como

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

El solver realiza esa conversión por sí mismo: la configuración modal y la vista
3D indican la frecuencia propia en Hz, mientras que el registro del solver
muestra junto a ella el valor propio correspondiente. Un modo que la iteración de
valores propios no consiguió resolver se indica como `0` Hz con una advertencia en
el registro.

Hay dos métodos disponibles, que se seleccionan con el parámetro de método modal:

| Método modal | Solver de valores propios | Extrae |
|---|---|---|
| Multiple modes | iteración en subespacio con proyección de Rayleigh-Ritz | el número de modos solicitado |
| Only most dominant mode | iteración de potencia inversa con cociente de Rayleigh | un único modo |

Ambos métodos iteran sobre `K^-1 * M`, por lo que convergen hacia las frecuencias
**más bajas**, que son las que importan. Cada paso es una resolución lineal con
`K`, utilizando la configuración del gradiente conjugado del solver matricial,
por lo que el coste de un análisis modal es aproximadamente el de una resolución
estática multiplicado por el número de vectores del subespacio y de iteraciones.

La iteración en subespacio trabaja con un bloque de vectores, proyecta `K` y `M`
sobre el subespacio que generan y resuelve exactamente el pequeño problema denso
de valores propios resultante. Se arrastran algunos vectores adicionales además
del número de modos solicitado, porque hacen que los modos buscados converjan
notablemente más rápido. Para `p` modos solicitados, el bloque contiene
`min(2p, p+8)` vectores, que es el compromiso habitual entre la velocidad de
convergencia y el coste de una iteración. Los valores propios se obtienen
directamente como `lambda` y se ordenan de forma ascendente, por lo que **el modo
0 es el modo fundamental**.

**Aquí es donde se va el tiempo de cálculo.** Una iteración en subespacio realiza
una resolución lineal completa por vector del bloque, de modo que pedir 10 modos
cuesta 18 resoluciones por iteración y pedir 100 cuesta 108. La iteración solo se
detiene antes cuando **todos** los valores propios solicitados se han
estabilizado dentro del valor de convergencia, y el modo solicitado más alto
siempre es el último en estabilizarse, por lo que un número grande de modos hace
cara cada iteración y a la vez poco probable la parada anticipada. Pida los modos
que realmente piense examinar.

Si los modos solicitados no se han estabilizado en la última iteración, el solver
lo indica en el registro e informa de la variación alcanzada, en lugar de
presentar sin comentarios los valores no convergidos. Los modos más bajos de un
cálculo así suelen ser todavía fiables; los más altos no.

En un análisis modal siempre se ensambla la matriz de masa, por lo que la
densidad debe estar presente en todas las entidades que participan.

**Análisis modal pretensado.** Antes de ensamblar las matrices, un análisis modal
desplaza cada nodo según el resultado de desplazamiento de una resolución
anterior y restablece la malla después. Por tanto, ejecutar *Stress analysis* y
luego *Modal analysis* en un mismo flujo de tareas da los modos de la estructura
**cargada y deformada** en lugar de los de la estructura sin deformar. La forma
modal almacenada es el vector propio sumado a ese desplazamiento inicial.

Solo se actualiza la geometría: la matriz de rigidez sigue siendo la elástica
lineal evaluada sobre la malla desplazada. Esto recoge el cambio de forma, pero no
la rigidización geométrica de un elemento traccionado.

El análisis modal es **exclusivo**: no puede combinarse con ninguna otra física
salvo un análisis de tensiones previo.

### 1.9 Resultados derivados

Tras la resolución, las tensiones en los elementos se obtienen a partir del campo
de desplazamientos como el promedio sobre los puntos de integración de

```
sigma = D * ( B * u  -  eps_th )
```

con la deformación térmica `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]` en los
elementos de volumen. Los elementos de superficie están en deformación plana: la
dilatación fuera de su plano está impedida, lo que eleva la deformación térmica en
el plano a `eps_th = (1+v) * alpha * dT * [1, 1, 0]`. La misma deformación
térmica entra en la carga térmica `f_th = INT( B^T * D * eps_th )`. Los elementos
de línea indican `E * (eps - alpha * dT)`.

A partir de las seis componentes, el solver forma dos invariantes y los combina en
cuadratura en la tensión de von Mises:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(`normal part` = parte normal, `shear part` = parte de cortante), que es la
definición clásica

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

En los elementos de superficie se utilizan los equivalentes bidimensionales: `QN`
a partir de las dos tensiones normales en el plano y `QS = sqrt(3 * tau_xy^2)`. En
los elementos de línea solo se indica el valor axial, y la parte de cortante es
nula.

Las **fuerzas nodales** se obtienen elemento a elemento como la fuerza elástica
interna

```
f = K * u - f_th
```

y se acumulan en los nodos. En un nodo restringido es la fuerza de reacción; en un
nodo libre sin carga debería resultar casi nula en una resolución estática
convergida, lo que convierte el resultado Force en una útil comprobación del
residuo. Por tanto, un cuerpo que puede dilatarse libremente no muestra ni
fuerzas ni tensiones.

| Resultado | Se aplica a | Significado |
|---|---|---|
| Displacement `[m]` | nodo | vector desplazamiento, o la forma modal en un análisis modal |
| Von Mises stress `[Pa]` | elemento | `sqrt(QN^2 + QS^2)` como arriba |
| Normal stress `[Pa]` | elemento | `QN` |
| Shear stress `[Pa]` | elemento | `QS` |
| Normal stress in X, Y, Z `[Pa]` | elemento | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | elemento | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | nodo | fuerza nodal obtenida, reacción en los nodos restringidos |

En los elementos de volumen, las seis componentes de la tensión se indican en el
sistema de coordenadas **global**. Los elementos de superficie y de línea evalúan
su tensión en su propio sistema local de elemento, por lo que sus componentes se
indican en ese sistema: una superficie rellena `sigma_xx`, `sigma_yy` y `tau_xy`
en el plano, y una línea solo el `sigma_xx` axial.

### 1.10 ¿Estático, transitorio o modal?

| | Estático | Transitorio dinámico | Transitorio cuasiestático | Modal |
|---|---|---|---|---|
| Resuelve | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` en el tiempo | `K*u = f(t)` en cada instante | `K*phi = omega^2*M*phi` |
| Responde a | «cuánto se deforma y cuánta tensión soporta» | «cómo responde a una carga que cambia rápidamente» | «cómo sigue una carga que cambia lentamente» | «a qué frecuencias tiende a vibrar y con qué forma» |
| Necesita masa | no | sí | no | sí |
| Necesita restricciones frente al movimiento de sólido rígido | sí | sí | sí | sí |
| Registros | uno | uno por paso de tiempo | uno por paso de tiempo | uno por modo extraído |

**Utilice un análisis estático para** comprobaciones de resistencia y rigidez,
límites de flecha, fuerzas de reacción, peso propio, recipientes a presión y
tensión térmica. Es la opción predeterminada y cubre la mayor parte del trabajo.

**Utilice un análisis transitorio dinámico para** impactos, cargas aplicadas o
retiradas bruscamente, historiales de choques y vibraciones, propagación de
ondas y la respuesta a una excitación armónica cercana a una resonancia: siempre
que importe la inercia.

**Utilice un análisis transitorio cuasiestático para** una carga que cambia con
el tiempo lo bastante despacio como para que la estructura la siga en equilibrio.

**Utilice un análisis modal para** frecuencias propias y formas modales, evitar la
resonancia con una excitación conocida, juzgar si una estructura es
suficientemente rígida desde el punto de vista dinámico y detectar mecanismos no
deseados: un valor propio casi nulo suele indicar una restricción que falta o una
pieza desconectada.

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Condiciones de entorno`,
`Material`, `Resultados`), el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Añada una tarea y elija una de las siguientes:

- **Stress analysis**: «Stress and linear elasto-dynamics analysis including
  modal analysis» (análisis de tensiones y elastodinámica lineal, incluido el
  análisis modal);
- **Modal analysis**: «Stress modal analysis» (análisis modal estructural).

El análisis de tensiones se combina libremente con otras físicas. Dos cadenas
útiles son *Heat transfer* y luego *Stress analysis* para la tensión térmica, e
*Incompressible viscous flow* y luego *Stress analysis* para la carga de
presión.

El análisis modal excluye todos los demás tipos de problema salvo un análisis de
tensiones previo, que lo convierte en un análisis modal pretensado.

### 2.2 Pestaña Problema

**Solver temporal**: se muestra para el análisis de tensiones, ya que es un tipo
de problema dependiente del tiempo. Déjelo desactivado para un análisis estático.

| Campo | Significado |
|---|---|
| Activar | activa y desactiva el avance transitorio en el tiempo |
| Aproximación | fija el beta de Newmark de un análisis dinámico, véase 1.7; la opción recomendada es la centrada |
| Tiempo inicial | primer valor de tiempo en segundos; se ignora al reiniciar el solver |
| Tiempo final | `start time + time-step size * number of time-steps` (tiempo inicial + tamaño del paso de tiempo × número de pasos de tiempo), solo lectura |
| Tamaño del paso de tiempo | `dt` en segundos |
| Número de pasos de tiempo | cuántos pasos se calculan |
| Frecuencia de salida | escribe un archivo de resultados cada N pasos; `0` solo escribe el último paso |

**Configuración del análisis transitorio de tensiones**: se muestra para el
análisis de tensiones y solo está activa mientras el solver temporal está
activado.

| Campo | Significado | Predeterminado |
|---|---|---|
| Tipo de análisis | *Dynamic* (con inercia) o *Quasi-static* (una resolución estática por paso de tiempo) | Dynamic |
| Amortiguamiento numérico (alfa HHT) | de `-1/3` a `0`; los valores negativos amortiguan el ruido de alta frecuencia | 0 |
| Matriz de masa concentrada | usa una matriz de masa diagonal en lugar de la consistente | desactivada |

Las dos últimas solo se aplican al análisis dinámico y aparecen atenuadas en los
demás casos.

**Configuración del análisis modal**: se muestra cuando el tipo de problema es
*Modal analysis*.

| Campo | Significado | Predeterminado |
|---|---|---|
| Método modal | *Only most dominant mode* (solo el modo dominante) o *Multiple modes* (varios modos) | Multiple modes |
| Iteraciones | iteraciones del solver de valores propios; desactivado para el método del modo dominante | 100 |
| Extraer modos | cuántos modos se extraen; desactivado para el método del modo dominante | 10 |
| Valor de convergencia | umbral de convergencia del solver de valores propios | 1.0e-9 |

Los dos números son independientes: las iteraciones refinan juntos todo el
conjunto de modos, no los producen uno a uno.

**Extraer modos es el parámetro que decide cuánto dura el cálculo.** Cada modo
añade una resolución lineal a cada iteración, y el cálculo solo puede detenerse
antes cuando se han estabilizado todos los modos solicitados, por lo que el coste
crece más deprisa que el número de modos. Diez es un punto de partida sensato y
el valor predeterminado; cien es un cálculo de otro orden de magnitud y, en una
malla grande, puede tardar horas. Auméntelo solo cuando sepa que necesita los
modos más altos.

Si un cálculo tarda más de lo esperado, el registro del solver indica el vector
del subespacio en el que está trabajando, de modo que se ve el progreso dentro de
una iteración, y muestra después de cada iteración la tasa de convergencia
alcanzada junto al valor requerido.

### 2.3 Pestaña Material

La pestaña `Material` muestra el material asignado a la entidad seleccionada. El
análisis de tensiones requiere **las cuatro** propiedades siguientes:

| Propiedad | Unidades | Comentario |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, unos `2.1e11` para el acero |
| Poisson ratio | `-` | `v`, unos `0.3` para el acero; debe ser inferior a `0.5` |
| Density | `kg/m^3` | necesaria para el peso propio y para los análisis dinámico y modal |
| Thermal expansion coefficient | `1/K` | póngalo a `0` si no hay carga térmica |

Una entidad a la que le falte alguna no se resuelve, y la comprobación de la
configuración avisa antes de que empiece el cálculo.

Otras dos propiedades son opcionales y solo las lee un análisis transitorio
dinámico:

| Propiedad | Unidades | Comentario |
|---|---|---|
| Rayleigh mass damping coefficient | `1/s` | `alpha_M`, véase 1.7; cero si falta |
| Rayleigh stiffness damping coefficient | `s` | `beta_K`, véase 1.7; cero si falta | Cada propiedad es una tabla en función de la temperatura, por lo que se
tiene en cuenta un módulo dependiente de la temperatura cuando se ejecuta antes un
solver térmico.

### 2.4 Pestaña Condiciones de contorno

La pestaña se divide en una lista de gestión de las condiciones de contorno
disponibles, arriba, y un editor de la condición seleccionada, abajo. Seleccione
primero una entidad en el árbol `Modelo`; el gestor ofrece entonces las
condiciones válidas para ese tipo de entidad y para el problema seleccionado.
Marque una para asignarla y luego edite sus componentes en el árbol inferior.

Se ofrecen las condiciones de la sección 1.5. Notas sobre su uso:

- **Displacement** muestra una casilla junto a cada una de sus tres componentes.
  Solo se restringen las componentes marcadas, de modo que una cara puede fijarse
  en X y dejarse libre en Y y Z. La casilla de valor de una componente desmarcada
  aparece atenuada.
- **Normal displacement** y **Roller displacement** toman por defecto su sistema
  local de la geometría: las normales promediadas de los elementos de una
  superficie o la dirección de los elementos de una línea. En una entidad de
  **punto** no hay geometría que seguir, por lo que el editor *Dirección local*
  siempre se muestra y siempre se utiliza.
- En una superficie o una línea, el árbol de la condición muestra la casilla
  **Usar la dirección local introducida**. Márquela para mostrar el editor
  *Dirección local* y tomar el sistema de la dirección introducida en lugar de la
  malla. Las flechas de dirección dibujadas en la vista 3D siguen la que esté
  activa de las dos. Si una restricción de superficie se comporta de forma extraña
  con la casilla desmarcada, compruebe que las normales de la superficie son
  coherentes y sincronícelas con las herramientas de geometría.
- Las restricciones de varias entidades que coinciden en un nodo se combinan, de
  modo que una cara puede fijarse en una dirección global mediante
  *Displacement* y deslizarse a la vez sobre un plano inclinado. Si dos de ellas
  piden valores distintos en la misma dirección, el cálculo se detiene con un
  error que indica el nodo.
- **Pressure** y **Force (unit area)** solo se aplican a superficies.
- Cada componente es una tabla en función del tiempo, por lo que a una carga se le
  puede dar un historial temporal para un cálculo transitorio.

### 2.5 Condiciones iniciales y de entorno

En la pestaña `Condiciones de entorno` es donde se activa la **gravedad**. Asigne
*Gravitational acceleration* a la entidad que debe soportar su peso propio
(normalmente todo el modelo) y ajuste el vector; el valor predeterminado es
`-9.80665` en Z. Asigne *Temperature* para fijar la temperatura de referencia sin
tensiones de un análisis de tensiones térmicas.

La pestaña `Condiciones iniciales` ofrece *Displacement*, *Structural velocity* y
*Pressure*, que se aplican en el primer cálculo de un análisis transitorio.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver estructural utiliza la entrada **CG**, tanto para
la resolución estática y transitoria como para las resoluciones internas de la
iteración de valores propios.

Si una resolución estática no converge, la causa habitual es un modelo
insuficientemente restringido más que un parámetro del solver. Compruebe las
restricciones antes de aumentar el número de iteraciones.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra, normalmente
*Displacement* o *Von Mises stress*. En un cálculo transitorio, el historial se
muestra con `Informe` -> `Puntos de monitorización`.

### 2.8 Resultados y registros

La pestaña `Resultados` muestra las variables calculadas y controla la vista 3D.
El desplazamiento es un vector en los nodos; las tres medidas de tensión son
escalares en los elementos. Un análisis transitorio dinámico añade los vectores en
los nodos *Structural velocity* y *Structural acceleration*.

Para representar la deformada, muestre *Displacement* y active el escalado de la
deformación en la configuración de la vista: los desplazamientos reales suelen ser
demasiado pequeños para verse a escala real.

La pestaña `Registros` del panel `Modelo` muestra los registros de resultados:

- **análisis estático**: un único registro;
- **análisis transitorio**: un registro por cada paso de tiempo escrito;
- **análisis modal**: un registro por modo extraído, numerados a partir del
  fundamental. Recorrer los registros equivale a recorrer los modos, y animarlos
  anima las formas modales.

En un registro modal, la vista 3D etiqueta el registro con el número de modo y la
frecuencia propia de ese modo en Hz.

`Informe` -> `Archivo de registro del solver` muestra la salida completa del
solver, incluido el valor propio impreso para cada modo extraído y las
estadísticas de desplazamiento, tensión de von Mises y fuerza después de cada
registro.

---

## 3. Tutorial - análisis estático de tensiones

**Objetivo.** Calcular la flecha y la tensión de una ménsula cargada con su propio
peso y una carga en el extremo, y leer la reacción en el apoyo.

Este tutorial supone un modelo de volumen mallado; una caja alargada funciona
bien. Utilice `Archivo` -> `Nuevo modelo`, dibuje una caja con `Geometría` ->
`Crear elemento` o importe una geometría, y mállela en tetraedros; el tutorial
*Dibujar un cubo* trata este tema. Marque el extremo fijo y el extremo cargado
como entidades de superficie independientes para poder asignarles condiciones de
contorno.

### Paso 1 - seleccionar el problema

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada una tarea y seleccione **Stress analysis**.
3. Confirme con `Aceptar`.

### Paso 2 - asignar el material

1. Seleccione la entidad de volumen en el árbol `Modelo`.
2. Abra la pestaña `Material` y asigne un material con las cuatro propiedades;
   para acero estructural:
   - **Modulus of elasticity** (módulo de elasticidad) = `2.1e11` `Pa`
   - **Poisson ratio** (coeficiente de Poisson) = `0.3`
   - **Density** (densidad) = `7850` `kg/m^3`
   - **Thermal expansion coefficient** (coeficiente de dilatación térmica) =
     `1.2e-5` `1/K`

### Paso 3 - mantener el análisis estático

Abra la pestaña `Problema` y deje **Activar** desmarcado en el grupo *Solver
temporal*. El solver ensambla y resuelve entonces `K*u = f` una sola vez.

### Paso 4 - restringir el modelo

1. Seleccione la entidad de superficie del extremo fijo.
2. Abra la pestaña `Condiciones de contorno` y marque **Displacement**.
3. Deje las tres componentes marcadas y con valor `0`.

Esto fija la cara por completo y elimina los seis modos de sólido rígido.
Compruebe que no queda nada más suelto: una pieza desconectada sin restricción
hace singular el sistema y la resolución no convergerá.

Para un apoyo que deba deslizarse en lugar de estar totalmente fijo, utilice en su
lugar **Roller displacement** sobre la superficie. Solo bloquea la dirección
normal a la superficie y deja el nodo libre para moverse en el plano tangente.

### Paso 5 - aplicar las cargas

**Peso propio.** Seleccione la entidad de volumen, abra la pestaña `Condiciones
de entorno` y marque **Gravitational acceleration**. Deje el valor predeterminado
`-9.80665` en Z.

**Carga en el extremo.** Seleccione la entidad de superficie del extremo libre,
abra la pestaña `Condiciones de contorno` y marque **Force**. Introduzca la fuerza
total en newtons, por ejemplo `-1000` en Z. El valor es el total sobre la entidad,
por lo que no cambia al refinar la malla.

Alternativas, según lo que sea físicamente la carga:

- **Pressure** para una presión normal uniforme sobre una cara, en `Pa`;
- **Force (unit area)** para una tracción dirigida en `N/m^2`;
- **Weight** para una masa unida en `kg` que no está modelada como geometría.

### Paso 6 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. La comprobación de la configuración indica los materiales o condiciones de
   contorno que faltan antes de que empiece el cálculo.
3. Siga el progreso en `Informe` -> `Archivo de registro del solver`.

### Paso 7 - examinar los resultados

- Pestaña `Resultados`: muestre **Displacement** para ver la deformada y **Von
  Mises stress** para la distribución de tensiones. Active el escalado de la
  deformación en la configuración de la vista para que la flecha sea visible.
- **Normal stress** y **Shear stress** separan las dos contribuciones que forman
  el valor de von Mises indicado.
- **Force** en la cara fija es la reacción. Su suma debe equilibrar la carga
  aplicada más el peso propio: una comprobación rápida y eficaz de que el modelo
  está cargado como se pretendía.
- Un punto de monitorización sobre *Displacement* en el extremo da la flecha como
  número en lugar de como color.

### Paso 8 - opcional, tensión térmica

Para añadir una carga térmica:

1. Añada una tarea **Heat transfer** *antes* de la tarea de tensiones en el flujo
   de tareas del problema y configure sus condiciones de contorno y propiedades
   de material.
2. Asigne a la entidad estructural una condición de entorno **Temperature** con la
   temperatura de referencia sin tensiones, por ejemplo `293.15` K.

El solver de tensiones recoge las temperaturas de los elementos calculadas por el
solver térmico y añade automáticamente el término de dilatación térmica.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La resolución no converge | el modelo está insuficientemente restringido; hay una pieza suelta o los apoyos no eliminan los seis modos de sólido rígido |
| *Entity has material assigned which is missing required properties* | falta una de las cuatro propiedades requeridas |
| Los desplazamientos son nulos en todas partes | ninguna carga ha llegado al modelo: compruebe que la gravedad está asignada o que las componentes de Force no son nulas |
| Los desplazamientos son enormes | el módulo de elasticidad está mal por varios órdenes de magnitud, o una entidad de superficie tiene espesor nulo y por tanto ninguna rigidez |
| La carga cambia al refinar la malla | se pretendía que una *Force (unit area)* o una *Pressure* fuera una fuerza total, o al revés |
| La presión empuja en sentido contrario | las normales de la superficie no son coherentes, o la superficie no delimita ningún volumen |

---

## 4. Tutorial - análisis modal

**Objetivo.** Extraer los primeros modos propios de la misma ménsula.

### Paso 1 - seleccionar el problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`), añada una tarea y
seleccione **Modal analysis**.

El análisis modal no puede combinarse con otras físicas, con la única excepción de
un análisis de tensiones situado antes, que lo convierte en un análisis modal
pretensado.

### Paso 2 - asignar el material

Las mismas cuatro propiedades que en el tutorial estático. Aquí la **densidad**
importa más que en ningún otro sitio: es toda la matriz de masa, y las
frecuencias varían como `1/sqrt(rho)`.

### Paso 3 - restringir el modelo

Asigne la misma condición de contorno **Displacement** en el extremo fijo.

Las restricciones son lo que distingue los modos de una ménsula de los de una
barra libre. Si el modelo se deja sin restricciones, los seis primeros valores
propios serán casi nulos (los modos de sólido rígido) y los modos interesantes
quedarán desplazados más abajo en la lista.

### Paso 4 - configurar el análisis modal

Abra la pestaña `Problema` y, en *Configuración del análisis modal*, ajuste:

| Campo | Valor |
|---|---|
| Método modal | `Multiple modes` |
| Iteraciones | `100` |
| Extraer modos | `6` |
| Valor de convergencia | `1.0e-9` |

Seis modos es un punto de partida sensato, y menos que los diez predeterminados.
Cada modo adicional añade una resolución lineal a cada iteración, por lo que un
número grande de modos en una malla grande es lento.

Elija `Only most dominant mode` cuando solo se necesite un modo; utiliza la
iteración de potencia inversa sobre un único vector y es mucho más barata.

### Paso 5 - no se necesitan cargas

Un análisis modal no tiene vector de cargas: el problema de valores propios es
homogéneo. Las cargas asignadas al modelo se ignoran, con una excepción: si antes
se ejecutó un análisis de tensiones, su resultado de desplazamiento deforma la
malla para el ensamblaje modal.

### Paso 6 - resolver

`Solución` -> `Iniciar solver` (`Ctrl+R`). El registro indica las iteraciones en
subespacio y su tasa de convergencia y, a continuación, un bloque por modo
extraído:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

(`Extracting mode 6` = extracción del modo 6, `Eigen-value` = valor propio,
`Storing results` = almacenamiento de los resultados.) Los modos se procesan desde
el índice extraído más alto hasta el fundamental, por lo que el fundamental es el
último en calcularse y se almacena como registro 1.

### Paso 7 - examinar los modos

- Panel `Modelo`, pestaña `Registros`: un registro por modo, numerados a partir
  del fundamental. Recórralos para ver cada forma modal.
- Pestaña `Resultados`: muestre **Displacement**. Una forma modal solo está
  definida salvo un factor de escala, por lo que las magnitudes son arbitrarias:
  lo que importa es la forma. Active el escalado de la deformación y anime el
  registro para visualizar el modo.
- El valor propio de cada modo se imprime en el registro del solver y se muestra
  como etiqueta del registro en la vista 3D.

### Paso 8 - leer las frecuencias propias

La vista 3D etiqueta cada registro modal con su número de modo y su frecuencia
propia en Hz. El registro del solver indica la misma frecuencia para cada modo
extraído junto con el valor propio correspondiente:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

(`frequency` = frecuencia.) Un modo que la iteración de valores propios no pudo
resolver se indica como `0` Hz y lleva una advertencia en el registro.

Como con cualquier resultado, merece la pena comprobar la frecuencia fundamental
con un cálculo manual la primera vez que se configura un modelo; la sección 5 da
la fórmula para una ménsula.

### Paso 9 - opcional, modos pretensados

1. En el flujo de tareas del problema, coloque una tarea **Stress analysis**
   *antes* de la tarea **Modal analysis**.
2. Configure las cargas estáticas y las restricciones como en el tutorial 3.

El análisis modal se ejecuta entonces sobre la malla deformada. Así se ve cómo una
carga estática cambia la forma con la que vibra una estructura.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| Los primeros valores propios son casi nulos | el modelo está insuficientemente restringido, o hay una pieza desconectada: un auténtico mecanismo |
| El solver de valores propios no converge | demasiado pocas iteraciones, o una matriz de masa mal condicionada porque una entidad no tiene densidad o medida geométrica |
| La extracción de modos es muy lenta | se han pedido demasiados modos; cada modo cuesta una resolución lineal completa |
| El cálculo tarda horas o parece no terminar nunca | se han pedido demasiados modos: cada uno cuesta una resolución lineal en cada iteración, y el cálculo no puede detenerse antes hasta que se hayan estabilizado todos. Observe en el registro el contador de vectores del subespacio para ver el progreso dentro de una iteración |
| Una frecuencia parece absurda | la iteración en subespacio no ha convergido (el registro lo indica al final de un cálculo no convergido); extraiga menos modos, aumente el número de iteraciones o compruebe la densidad y las restricciones |
| Las formas modales parecen idénticas | modos muy próximos de una estructura simétrica, o demasiado pocas iteraciones en subespacio para separarlos |
| Hay menos registros que modos solicitados | el subespacio perdió direcciones independientes; el registro indica cuántos modos se extrajeron y solo almacena esos |

---

## 5. Comprobación de un modelo

El solver no tiene un conjunto de verificación integrado, por lo que merece la
pena validar un modelo nuevo con algo que pueda calcularse a mano antes de
confiar en él. Tres comprobaciones baratas, por orden de utilidad:

**Equilibrio de reacciones.** Sume el resultado *Force* en los nodos
restringidos. Debe equilibrar las cargas aplicadas más el peso propio
`rho * V * g`. Esto detecta magnitudes de carga erróneas, unidades erróneas y
direcciones de presión erróneas.

**Flecha en el extremo de una ménsula.** Para una viga de longitud `L`, momento de
inercia `I` y carga en el extremo `F`, la teoría de Euler-Bernoulli da

```
delta = F * L^3 / (3 * E * I)
```

Modele la viga con elementos de volumen y compare. Es de esperar que el resultado
de elementos finitos sea algo más rígido en una malla gruesa y converja por
defecto al refinar la malla. No utilice elementos de línea para esta
comprobación: no tienen rigidez a flexión.

**Frecuencia fundamental de una ménsula.** Para la misma viga con sección `A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Compárela con `sqrt(lambda_0) / (2*pi)` del análisis modal. Esto valida a la vez
la matriz de masa, la matriz de rigidez y su conversión de los valores propios.

**Barra cargada bruscamente.** Una barra fija en un extremo y cargada en el otro
con una fuerza axial `F` aplicada en `t = 0` es una comprobación de un análisis
dinámico. La onda de tensión recorre la barra hasta el extremo fijo y vuelve a la
velocidad `c = sqrt(E/rho)`, de modo que el extremo libre se desplaza
linealmente hasta el doble de la flecha estática, `2 * F * L / (E * A)`, en
`t = 2*L/c` y vuelve a cero en `t = 4*L/c`. Sin amortiguamiento, el ciclo se
repite. Con amortiguamiento de Rayleigh se extingue y la barra queda en reposo en
la flecha estática. Utilice un coeficiente de Poisson nulo para que la barra sea
realmente unidimensional.

Para cualquiera de ellas, refine la malla al menos una vez y confirme que la
respuesta converge en lugar de derivar.

---

## 6. Limitaciones

### Limitaciones de modelado

- **Solo elasticidad lineal.** Pequeñas deformaciones, pequeños desplazamientos,
  material isótropo, sin plasticidad, sin contacto, sin grandes deformaciones y
  sin pandeo. Los resultados solo tienen sentido mientras la estructura se
  mantenga en su rango lineal.
- **El amortiguamiento es solo de Rayleigh.** La matriz de amortiguamiento es
  `alpha_M*M + beta_K*K` por material, por lo que la razón de amortiguamiento
  sigue la curva de 1.7 y no puede fijarse modo a modo. No hay amortiguadores
  discretos, ni amortiguamiento independiente de la frecuencia (histerético), ni
  condición de contorno de amortiguamiento.
- **Solo integración temporal implícita.** Cada paso es una resolución lineal. No
  hay esquema explícito de diferencias centradas, por lo que los sucesos muy
  breves, como un impacto balístico, necesitan muchos pasos costosos.
- **Los elementos de superficie están en deformación plana.** Una placa delgada
  cargada en su propio plano será más rígida que una idealización en tensión
  plana. Modele las estructuras delgadas con elementos de volumen cuando la
  diferencia importe.
- **Los elementos de línea son barras articuladas.** Solo se ensambla la rigidez
  axial `E*A`. No hay flexión, cortante ni torsión, se llame como se llame el
  tipo de elemento, por lo que un pórtico modelado con elementos de línea se
  comportará como una celosía articulada.
- **El pretensado es solo geométrico.** Un análisis modal pretensado vuelve a
  evaluar la rigidez lineal sobre la malla desplazada. No añade un término de
  rigidez geométrica, por lo que no se recoge la rigidización de un cable tenso
  ni el ablandamiento de un puntal comprimido.
- **Sin factores de participación modal ni masas efectivas.** El solver solo
  indica valores propios y formas modales, por lo que no hay ninguna indicación
  directa de qué modos importan para una dirección de excitación dada.
