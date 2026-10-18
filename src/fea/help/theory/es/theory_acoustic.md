# Análisis acústico - Manual de teoría y guía del usuario

Este documento describe el solver acústico (`RSolverAcoustic`) de Range FEA: las
ecuaciones que resuelve, el significado de cada dato de entrada que acepta, las
partes de la interfaz gráfica que lo controlan y dos tutoriales completos.

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - análisis acústico transitorio](#3-tutorial---análisis-acústico-transitorio)
4. [Tutorial - análisis acústico armónico](#4-tutorial---análisis-acústico-armónico)
5. [Verificación](#5-verificación)
6. [Limitaciones](#6-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnita principal - el potencial de velocidad

El solver acústico no resuelve directamente la presión. Su incógnita principal
es el **potencial de velocidad** `phi`, un campo escalar con unidades de
`m^2/s`. Las dos magnitudes físicas de interés se obtienen a partir de él:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`particle velocity` = velocidad de partícula, `acoustic pressure` = presión
acústica.)

Utilizar un potencial en lugar de la presión tiene dos ventajas prácticas. El
problema sigue siendo escalar aunque la velocidad de partícula sea un vector, y
una velocidad de pared impuesta (la forma en que se describen la mayoría de las
fuentes acústicas) entra como condición de contorno natural y no como
restricción.

El convenio de signos anterior (`u = -grad(phi)`, `p = +rho * dphi/dt`) se
utiliza de forma coherente en todo el solver y en los resultados almacenados.

### 1.2 Ecuación fundamental

El sonido de pequeña amplitud en un fluido homogéneo en reposo obedece la
ecuación de ondas amortiguada para el potencial de velocidad:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `phi` | potencial de velocidad | `m^2/s` | incógnita resuelta |
| `c` | velocidad del sonido | `m/s` | propiedad del material |
| `rho` | densidad del fluido | `kg/m^3` | propiedad del material |
| `beta` | factor de amortiguamiento volumétrico | `1/s` | propiedad del material, `0` por defecto |

La ecuación es lineal. En consecuencia, una única resolución por paso de tiempo,
o por frecuencia, es la respuesta final: no se realiza ninguna iteración no
lineal externa y `hasConverged()` siempre indica éxito una vez resuelto el
sistema lineal.

**Velocidad del sonido.** `c` puede indicarse directamente mediante la propiedad
de material *Speed of sound*. Si falta o es cero, se deduce de la rigidez
volumétrica y de la densidad:

```
c = sqrt(E / rho)
```

donde `E` es la propiedad de material *Modulus of elasticity*. Para un gas, `E`
es el módulo de compresibilidad adiabático (`gamma * p0`, unos `1.42e5 Pa` para
el aire a presión atmosférica), que junto con `rho = 1.2 kg/m^3` da el conocido
`c = 344 m/s`. Indicar `c` directamente suele ser menos propenso a errores.

Los elementos que no tienen material propio (por ejemplo, una superficie de
contorno que delimita un volumen mallado) heredan los valores medios de `c` y
`rho` del dominio calculable, de modo que los términos de impedancia y de
radiación se evalúan correctamente en ellos.

### 1.3 Discretización por elementos finitos

Multiplicando por una función de ponderación `w`, integrando sobre el dominio,
aplicando el teorema de la divergencia y multiplicando todo por `c^2` se obtiene
la forma débil

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

que, tras sustituir las funciones de forma de los elementos `N`, se convierte en
el sistema de segundo orden

```
M * phi'' + C * phi' + K * phi = f
```

con

| Matriz | Definición | Comentario |
|---|---|---|
| `M` | `INT( N_m * N_n )` | matriz de masa |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | matriz de rigidez |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | amortiguamiento volumétrico más impedancia de contorno |
| `f` | `c^2 * SURF( v_n * N_m )` | fuente de velocidad normal impuesta |

`M` y `K` son semidefinidas positivas; `C` es semidefinida positiva siempre que
la relación de impedancia `zeta` sea positiva.

**Medidas de los elementos.** Los términos de dominio (`M`, `K` y la parte
volumétrica de `C`) se ponderan con la medida geométrica de la entidad a la que
pertenecen:

| Entidad | Medida | Dónde se define |
|---|---|---|
| Volumen | 1 | - |
| Superficie | espesor de la superficie | propiedades geométricas de la entidad |
| Línea | sección de la línea | propiedades geométricas de la entidad |
| Punto | volumen del punto | propiedades geométricas de la entidad |

Los términos de contorno (la parte de impedancia de `C` y la fuente `f`) actúan
sobre la medida simple del elemento: el área de un elemento de superficie, la
longitud de un elemento de línea, un área unidad para un elemento de punto. Por
tanto, una superficie que delimita un volumen mallado solo aporta sus términos de
contorno, ya que su espesor es cero por defecto.

Los elementos de punto no tienen puntos de integración. Sus contribuciones se
concentran directamente en su único nodo, lo que permite que las condiciones de
contorno puntuales funcionen en los extremos de los conductos unidimensionales.

### 1.4 Condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componente |
|---|---|---|---|
| Velocity potential | explícita (Dirichlet) | punto, línea, superficie | Potential `[m^2/s]` |
| Velocity | natural (fuente) | punto, línea, superficie | Velocity `[m/s]` |
| Absorbing boundary | natural (amortiguamiento) | punto, línea, superficie | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | natural (amortiguamiento) | punto, línea, superficie | Acoustic impedance `[Pa*s/m]` |

**Velocity** (velocidad). La fuente acústica clásica: una superficie que vibra
con una velocidad normal impuesta, como el cono de un altavoz o un pistón. Un
valor positivo impulsa el fluido, es decir, apunta *hacia el interior* del
dominio acústico. Entra en el segundo miembro como `f = c^2 * SURF(v_n * N)`.

Un nodo sin ninguna condición de contorno es una **pared rígida** ideal: la
condición natural de la forma débil es una velocidad normal nula. No hay que
asignar nada para obtener una cavidad de paredes rígidas.

**Acoustic impedance** (impedancia acústica). Impone la relación entre la presión
y la velocidad de partícula normal exterior en el contorno, `p = Z * u_n`.
Internamente se expresa mediante la **relación de impedancia específica**
adimensional

```
zeta = Z / (rho * c)
```

y aporta la matriz de amortiguamiento de contorno `(c/zeta) * SURF(N_m * N_n)`.
`Z = rho*c` (`zeta = 1`) es la impedancia característica del fluido y da una
terminación perfectamente no reflectante. Para el aire en condiciones
ambientales, `rho*c` es cercano a `415 Pa*s/m`, que es el valor que ofrece por
defecto la condición de contorno.

**Absorbing boundary** (contorno absorbente). El mismo término, pero
parametrizado por el **coeficiente de absorción** en incidencia normal `alpha`
en `[0, 1]`, que es el que indican las fichas técnicas de los materiales. Se
convierte en relación de impedancia mediante el coeficiente de reflexión de onda
plana `R = (zeta - 1) / (zeta + 1)` y `alpha = 1 - |R|^2`:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Comportamiento |
|---|---|---|
| `1.0` | `1` | totalmente absorbente, terminación anecoica, sin reflexión |
| `0.5` | `5.83` | parcialmente absorbente |
| `0.1` | `38.0` | débilmente absorbente |
| `0.0` | infinito | pared rígida, sin ningún amortiguamiento |

Utilice `alpha = 1` en cualquier corte artificial de un dominio no acotado para
emular la radiación libre, y un `alpha` medido en una pared revestida. Si se
asignan a la misma entidad una condición *Acoustic impedance* y una *Absorbing
boundary*, prevalece la impedancia.

### 1.5 Condiciones iniciales

| Condición inicial | Define | Comentario |
|---|---|---|
| Velocity potential | `phi` en `t = 0` | rara vez es distinto de cero por sí solo |
| Velocity potential / s | `dphi/dt` en `t = 0` | es lo que define la **presión** inicial, `p = rho * dphi/dt` |

Ambas se aplican solo en el primer paso de tiempo. Para iniciar un cálculo
transitorio desde una región presurizada, asigne a esa entidad *Velocity
potential / s* `= p0 / rho`. Las condiciones iniciales no tienen efecto en un
análisis armónico.

### 1.6 Análisis transitorio

El sistema de segundo orden se integra con el esquema de **Newmark** con
`gamma = 1/2` y

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

(`time-march approximation coefficient` = coeficiente de aproximación del
esquema temporal.) El límite garantiza `beta_N >= 1/4`, es decir, como mínimo la
variante de aceleración media, incondicionalmente estable, y evita una división
por cero para la aproximación progresiva, cuyo coeficiente es `0`.

Cada paso resuelve

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

con los coeficientes habituales

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

seguidos de la actualización del estado

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

La matriz del sistema es simétrica definida positiva, por lo que se resuelve con
el método del **gradiente conjugado** y precondicionamiento de Jacobi.

`phi'` y `phi''` se escriben en el archivo de resultados como *Potential / s* y
*Potential / s^2*. Constituyen el estado del integrador temporal, por lo que
deben estar presentes para que un reinicio o el siguiente paso de tiempo
continúen correctamente.

**Un análisis transitorio requiere un solver temporal activado.** Sin él, los
términos de masa y de amortiguamiento desaparecen y el problema degenera en un
problema de Laplace sin sentido; el solver lo notifica como error en lugar de
producir un resultado absurdo.

**Elección del paso de tiempo.** El esquema es incondicionalmente estable, por lo
que `dt` está determinado por la precisión y no por la estabilidad. Una regla
útil es resolver la frecuencia de interés más alta `f_max` con al menos 20 pasos
por periodo:

```
dt <= 1 / (20 * f_max)
```

**Elección de la malla.** Resuelva la longitud de onda más corta
`lambda_min = c / f_max` con al menos 10 elementos lineales:

```
h <= c / (10 * f_max)
```

### 1.7 Análisis armónico

Para una respuesta estacionaria a una única frecuencia angular
`omega = 2*pi*f`, se escribe `phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`.
Entonces `dphi/dt -> i*omega*PHI` y `d2phi/dt2 -> -omega^2*PHI`, y el sistema se
convierte en el problema complejo de **Helmholtz**

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

Los solvers matriciales de Range FEA son reales, por lo que el sistema complejo
se resuelve como un sistema real por bloques equivalente de tamaño doble. Con
`S = K - omega^2*M` y `D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Esta matriz por bloques es no simétrica y, por encima del límite estático,
indefinida, por lo que se resuelve con **GMRES** y precondicionamiento de Jacobi.
GMRES con reinicio se estanca en los sistemas indefinidos siempre que el espacio
de Krylov es mucho menor que el sistema, por lo que el solver amplía
automáticamente el reinicio de GMRES hasta el tamaño del sistema, con un límite
fijo de 256 MB para las dos bases de Krylov. El reinicio configurado solo se
aumenta, nunca se reduce, y el cambio se notifica en el registro del solver:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

(«Aumento del reinicio de GMRES de 10 a 403 para el sistema en el dominio de la
frecuencia».) Un análisis armónico recorre una lista de frecuencias

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(`first frequency` = primera frecuencia, `frequency step` = paso de frecuencia,
`number of frequencies` = número de frecuencias.) Cada frecuencia es una
resolución independiente y produce su propio registro de resultados, del mismo
modo que un análisis transitorio produce un registro por paso de tiempo. El
solver temporal no interviene; si está activado, se omite con una advertencia, y
su configuración se mantiene intacta para que al volver a un análisis
transitorio no se pierda.

### 1.8 Resultados derivados

| Resultado | Se aplica a | Transitorio | Armónico |
|---|---|---|---|
| Potential `[m^2/s]` | nodo | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | nodo | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | nodo | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | nodo | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | nodo | `rho * dphi/dt`, instantánea | `omega * rho * abs(PHI)`, amplitud |
| Acoustic phase `[Deg]` | nodo | - | `arg(P)` |
| Sound pressure level `[dB]` | nodo | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | elemento | `-grad(phi)` | amplitud de `-grad(PHI)`, por componente |
| Acoustic intensity `[W/m^2]` | elemento | `p * u`, instantánea | `0.5 * Re{P * conj(U)}`, promediada en el tiempo |

El **nivel de presión sonora** (*Sound pressure level*) se calcula respecto a la
**Presión de referencia** de la configuración acústica, `2e-5 Pa` por defecto, el
umbral de audición en el aire. En un análisis armónico se utiliza el valor eficaz
de la señal armónica, es decir, la amplitud dividida por `sqrt(2)`; en un
análisis transitorio se utiliza el valor instantáneo, por lo que el nivel
indicado oscila dentro de un periodo.

La **intensidad acústica** en un análisis armónico es la intensidad activa
promediada en el tiempo, es decir, el flujo neto de potencia acústica. Es
idénticamente nula en una onda estacionaria pura y vale `0.5 * |p| * |u|` en una
onda plana progresiva.

### 1.9 ¿Transitorio o armónico?

| | Transitorio | Armónico |
|---|---|---|
| Resuelve | `M*phi'' + C*phi' + K*phi = f` en el tiempo | `(K - w^2*M + i*w*C)*PHI = F` por frecuencia |
| Responde a | «qué ocurre cuando ...» | «cuál es la respuesta estacionaria a ...» |
| Excitación | cualquier historial temporal, incluido un único pulso | una sinusoide cada vez |
| Coste | una resolución SPD por paso de tiempo | una resolución indefinida de tamaño doble por frecuencia |
| Registros | uno por paso de tiempo | uno por frecuencia |
| Información de fase | implícita en el historial temporal | explícita, como campo de resultado |

**Utilice un análisis transitorio para**

- estudios de respuesta al impulso, eco y reverberación;
- sucesos transitorios: una explosión, el cierre de una válvula, un portazo;
- excitación de banda ancha, en la que un único cálculo seguido de una
  transformada de Fourier de un punto de monitorización cubre todo el espectro a
  la vez;
- cualquier caso en el que el propio historial temporal sea la respuesta.

**Utilice un análisis armónico para**

- funciones de respuesta en frecuencia y curvas de pérdida por transmisión;
- resonancias de cavidades y conductos, y la distribución de presión en una
  resonancia;
- el rendimiento de silenciadores y absorbentes en una banda de frecuencias;
- mapas de nivel de presión sonora para un tono único;
- cualquier caso en el que solo importe el régimen estacionario. Un cálculo
  armónico suele ser mucho más barato que avanzar un transitorio el tiempo
  suficiente para que se extingan los transitorios.

Un compromiso práctico es localizar las resonancias con un barrido armónico
grueso y refinar después solo la banda de interés.

---

## 2. Interfaz gráfica

Todo lo relacionado con la acústica se encuentra en el panel **Solver**
(pestañas `Problema`, `Condiciones de contorno`, `Condiciones iniciales`,
`Material`, `Resultados`), el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema acústico

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Añada una tarea y elija **Acoustics**: «Customized
wave equation for solving acoustic problems» (ecuación de ondas adaptada para
resolver problemas acústicos).

Acoustics es mutuamente excluyente con **Wave - general** y con **Modal
analysis**. Puede combinarse con otras físicas, en cuyo caso el flujo de tareas
determina el orden de ejecución.

### 2.2 Pestaña Problema

Una vez seleccionada la acústica, la pestaña `Problema` del panel Solver muestra
dos grupos.

**Solver temporal**: se muestra porque la acústica es un tipo de problema
dependiente del tiempo.

| Campo | Significado |
|---|---|
| Activar | activa y desactiva el avance transitorio en el tiempo |
| Aproximación | aproximación del esquema temporal; corresponde al `beta_N` de Newmark como se describe en 1.6 |
| Tiempo inicial | primer valor de tiempo en segundos; se ignora al reiniciar el solver |
| Tiempo final | `start time + time-step size * number of time-steps` (tiempo inicial + tamaño del paso de tiempo × número de pasos de tiempo), solo lectura |
| Tamaño del paso de tiempo | `dt` en segundos |
| Número de pasos de tiempo | cuántos pasos se calculan |
| Frecuencia de salida | escribe un archivo de resultados cada N pasos; `0` solo escribe el último paso |

**Configuración del análisis acústico**: el grupo añadido para la acústica.

| Campo | Significado |
|---|---|
| Tipo de análisis | *Transient* (transitorio) o *Harmonic* (armónico) |
| Primera frecuencia `[Hz]` | primera frecuencia del barrido armónico |
| Paso de frecuencia `[Hz]` | incremento entre dos frecuencias consecutivas |
| Número de frecuencias | cuántas frecuencias se resuelven; cada una produce su propio registro |
| Barrido | etiqueta de solo lectura que muestra el rango de frecuencias resultante |
| Presión de referencia `[Pa]` | referencia utilizada para el nivel de presión sonora, `2e-5 Pa` por defecto |

Los tres campos de frecuencia aparecen atenuados mientras está seleccionado
*Transient*. La presión de referencia se aplica a ambos tipos de análisis.

### 2.3 Pestaña Material

La pestaña `Material` muestra el material asignado a la entidad seleccionada y las
propiedades que contiene. Para la acústica, las propiedades relevantes son:

| Propiedad | Unidades | Requerida | Comentario |
|---|---|---|---|
| Density | `kg/m^3` | sí | `rho` |
| Speed of sound | `m/s` | una de las dos | `c`, se usa directamente si no es cero |
| Modulus of elasticity | `Pa` | una de las dos | alternativa, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | no | `beta`, atenuación volumétrica, `0` por defecto |

El solver acústico resuelve una entidad cuando tiene densidad **y** una velocidad
del sonido o un módulo de elasticidad. Como ambas son alternativas, la
comprobación de la configuración no indica la falta de ninguna; un modelo que no
tenga ninguna de las dos es rechazado por el solver con un mensaje explícito.

Cada propiedad es una tabla en función de la temperatura, por lo que puede
introducirse una velocidad del sonido dependiente de la temperatura, que se
interpolará si se resuelve a la vez un problema de transferencia de calor.

### 2.4 Pestaña Condiciones de contorno

La pestaña se divide en una lista de gestión de las condiciones de contorno
disponibles, arriba, y un editor de la condición seleccionada, abajo. Seleccione
primero una entidad en el árbol `Modelo`; el gestor ofrece entonces las
condiciones de contorno válidas para ese tipo de entidad y para el problema
seleccionado. Marque una para asignarla y luego edite sus componentes en el árbol
inferior.

Para la acústica, el gestor ofrece:

- **Velocity potential**: explícita, puntos, líneas y superficies. Componente:
  *Potential* `[m^2/s]`.
- **Velocity**: puntos, líneas y superficies. Componente: *Velocity* `[m/s]`.
- **Absorbing boundary**: puntos, líneas y superficies. Componente: *Acoustic
  absorption coefficient*, adimensional, `1.0` por defecto.
- **Acoustic impedance**: puntos, líneas y superficies. Componente: *Acoustic
  impedance* `[Pa*s/m]`, `415` por defecto.

Cada componente es una tabla en función del tiempo, por lo que se puede dar un
historial temporal a una fuente: añada filas para aumentar progresivamente una
velocidad o para emitir un pulso corto. En un análisis armónico se utiliza el
valor en el tiempo actual del solver, que para una tabla de un solo valor es
simplemente ese valor.

Deje una pared sin ninguna condición de contorno para que sea acústicamente
rígida.

### 2.5 Pestaña Condiciones iniciales

Misma disposición que la pestaña de condiciones de contorno. Para la acústica
ofrece **Velocity potential** y **Velocity potential / s**. La segunda es la que
define el campo de presión inicial.

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. Un análisis acústico transitorio utiliza la entrada **CG** y
un análisis armónico la entrada **GMRES**.

El reinicio predeterminado de GMRES, `10`, es demasiado pequeño para el sistema
por bloques en el dominio de la frecuencia. El solver lo aumenta
automáticamente, pero el número de iteraciones *externas* y el valor de
convergencia se toman tal como están configurados, así que auméntelos aquí si un
cálculo armónico no converge.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra. Un punto de
monitorización sobre *Acoustic pressure* es el micrófono natural: en un cálculo
transitorio produce el historial de presión en ese lugar, que puede consultarse
mediante `Informe` -> `Puntos de monitorización`.

### 2.8 Resultados

Tras el cálculo, la pestaña `Resultados` muestra las variables calculadas y
controla lo que muestra la vista 3D. Las variables acústicas son las de la
sección 1.8.

Las variables en los nodos (presión, nivel de presión sonora, fase, potencial) se
dibujan como campos continuos; las variables en los elementos (velocidad de
partícula, intensidad) son vectores y se muestran mejor mediante un campo
vectorial o un corte.

La pestaña `Registros` del panel `Modelo` muestra los distintos registros de
resultados (uno por paso de tiempo en un cálculo transitorio, uno por frecuencia
en un barrido armónico) y permite recorrerlos o animarlos.

`Informe` -> `Archivo de registro del solver` muestra la salida completa del
solver, incluido el bloque de estadísticas que se imprime para cada variable
acústica después de cada registro.

---

## 3. Tutorial - análisis acústico transitorio

**Objetivo.** Calcular la respuesta al impulso de una cavidad de aire cerrada y
leer el historial de presión en la posición de un micrófono.

Este tutorial supone un modelo de volumen mallado. Utilice `Archivo` ->
`Nuevo modelo`, dibuje una caja con `Geometría` -> `Crear elemento` o importe una
geometría, y mállela en tetraedros; el tutorial *Dibujar un cubo* trata este
tema.

### Paso 1 - seleccionar el problema

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada una tarea y seleccione **Acoustics**.
3. Confirme con `Aceptar`.

La pestaña `Problema` del panel Solver muestra ahora los grupos *Solver
temporal* y *Configuración del análisis acústico*.

### Paso 2 - asignar el material

1. Seleccione la entidad de volumen en el árbol `Modelo`.
2. Abra la pestaña `Material`.
3. Asigne un material, o cree uno, con:
   - **Density** (densidad) = `1.2` `kg/m^3`
   - **Speed of sound** (velocidad del sonido) = `344` `m/s`
4. Deje **Acoustic damping factor** en `0` para una cavidad sin amortiguamiento,
   o ajústelo a un valor pequeño como `1.0` `1/s` para que la respuesta decaiga.

### Paso 3 - elegir el tipo de análisis y el paso de tiempo

1. Abra la pestaña `Problema`.
2. En *Configuración del análisis acústico*, ajuste **Tipo de análisis** =
   `Transient`.
3. En *Solver temporal*:
   - marque **Activar**;
   - **Tiempo inicial** = `0`;
   - **Tamaño del paso de tiempo** = `2e-5` s. Según `dt <= 1/(20*f_max)`, esto
     resuelve frecuencias de hasta unos `2.5 kHz`;
   - **Número de pasos de tiempo** = `500`, lo que da `10 ms` de tiempo simulado;
   - **Frecuencia de salida** = `5`, que escribe 100 registros de resultados.

Compruebe la malla con `h <= c/(10*f_max)`: para `f_max = 2.5 kHz` y
`c = 344 m/s`, el tamaño de los elementos no debe superar unos `14 mm`.

### Paso 4 - excitar la cavidad

Dos opciones; elija una.

**A - una pared móvil.** Seleccione la superficie que representa la fuente, abra
la pestaña `Condiciones de contorno` y marque **Velocity**. Introduzca un
historial temporal en la tabla de la componente, por ejemplo un pulso corto:

| Tiempo `[s]` | Velocidad `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - una región inicialmente presurizada.** Seleccione la entidad que debe
empezar presurizada, abra la pestaña `Condiciones iniciales` y marque **Velocity
potential / s**. Introduzca `p0 / rho`; para `p0 = 1 Pa` y `rho = 1.2 kg/m^3`
eso da `0.833`.

Deje todas las demás superficies sin condición de contorno para que la cavidad
tenga paredes rígidas, o marque **Absorbing boundary** con un coeficiente de
absorción acorde con el revestimiento de la pared para dejar escapar la energía.

### Paso 5 - añadir un micrófono

1. `Problema` -> `Definir puntos de monitorización`.
2. Añada un punto en el lugar de interés.
3. Seleccione **Acoustic pressure** como su variable.

### Paso 6 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. La comprobación de la configuración indica los materiales o condiciones de
   contorno que faltan antes de que empiece el cálculo.
3. Siga el progreso en `Informe` -> `Archivo de registro del solver`. Cada paso
   de tiempo imprime la convergencia de CG y las estadísticas de cada variable
   acústica.

### Paso 7 - examinar los resultados

- Panel `Modelo`, pestaña `Registros`: recorra o anime los registros temporales.
- Pestaña `Resultados`: muestre **Acoustic pressure** o **Sound pressure level**
  sobre el modelo, y **Acoustic particle velocity** como campo vectorial.
- `Informe` -> `Puntos de monitorización`: el historial de presión en el
  micrófono. Su velocidad de decaimiento da el tiempo de reverberación; su
  transformada de Fourier, las resonancias de la cavidad.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* (el análisis acústico transitorio requiere un solver temporal activado) | *Activar* está desmarcado en *Solver temporal*, o el tipo de análisis debería ser *Harmonic* |
| *"Acoustic solver requires a positive time step size"* (el solver acústico requiere un tamaño de paso de tiempo positivo) | el tamaño del paso de tiempo es cero |
| *"No element with a valid speed of sound was found"* (no se ha encontrado ningún elemento con una velocidad del sonido válida) | ninguna entidad tiene densidad junto con velocidad del sonido o módulo de elasticidad |
| La respuesta nunca decae | no hay amortiguamiento en ningún sitio: es lo esperado en una cavidad perfectamente rígida y sin pérdidas; añada un contorno absorbente o un factor de amortiguamiento acústico |
| Oscilaciones por delante del frente de onda | la malla o el paso de tiempo son demasiado gruesos para el contenido en frecuencia; refine ambos |

---

## 4. Tutorial - análisis acústico armónico

**Objetivo.** Calcular la respuesta en frecuencia de un conducto con terminación
anecoica y un mapa del nivel de presión sonora en cada frecuencia.

### Paso 1 - seleccionar el problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`), añada una tarea y
seleccione **Acoustics**.

### Paso 2 - asignar el material

Como en el tutorial transitorio: **Density** `1.2 kg/m^3` y **Speed of sound**
`344 m/s` en la entidad del fluido.

Dimensione la malla para la frecuencia **más alta** del barrido:
`h <= c / (10 * f_max)`.

### Paso 3 - configurar el barrido

Abra la pestaña `Problema` y, en *Configuración del análisis acústico*, ajuste:

| Campo | Valor |
|---|---|
| Tipo de análisis | `Harmonic` |
| Primera frecuencia | `50` Hz |
| Paso de frecuencia | `50` Hz |
| Número de frecuencias | `20` |
| Presión de referencia | `2e-5` Pa |

La etiqueta *Barrido* confirma `50 - 1000 [Hz]`, es decir, 20 resoluciones y 20
registros de resultados.

El grupo *Solver temporal* puede seguir activado: un análisis armónico lo ignora
y escribe una advertencia en el registro. Su configuración se conserva.

### Paso 4 - excitar el conducto

Seleccione la superficie de entrada, abra la pestaña `Condiciones de contorno` y
marque **Velocity**. Introduzca una única fila con la velocidad del pistón, por
ejemplo `0.01` `m/s`. La amplitud se interpreta como la amplitud de la excitación
armónica en cada frecuencia del barrido.

### Paso 5 - terminar el conducto

Seleccione la superficie de salida y marque una de las siguientes:

- **Absorbing boundary** con **Acoustic absorption coefficient** = `1.0` para una
  terminación perfectamente no reflectante; o
- **Acoustic impedance** con **Acoustic impedance** = `rho * c` = `413`
  `Pa*s/m`, que es lo mismo expresado de otra forma.

Utilice un coeficiente de absorción menor para modelar una terminación
parcialmente reflectante, y deje la salida sin ninguna condición para modelar un
extremo cerrado y rígido.

Las paredes del conducto no necesitan ninguna condición de contorno: son rígidas
por defecto.

### Paso 6 - comprobar el solver matricial

`Problema` -> `Configurar el solver matricial de los problemas`, entrada
**GMRES**. El reinicio se aumenta automáticamente, pero si el registro muestra un
residuo estancado, aumente el número de iteraciones externas y relaje o endurezca
el valor de convergencia según sea necesario.

### Paso 7 - resolver

`Solución` -> `Iniciar solver` (`Ctrl+R`). El registro muestra un bloque por
frecuencia:

```
Frequency:         1 of 20        | f =  5.000000e+01 [Hz]
{
    Increasing GMRES restart from 10 to ... for the frequency domain system.
    ...
    Acoustic pressure [Pa]
    Sound pressure level [dB]
    ...
}
```

(`Frequency: 1 of 20` = frecuencia 1 de 20.)

### Paso 8 - examinar los resultados

- Panel `Modelo`, pestaña `Registros`: un registro por frecuencia. Recorrerlos
  equivale a recorrer el barrido en frecuencia.
- Pestaña `Resultados`: muestre **Sound pressure level** para un mapa en
  decibelios, **Acoustic pressure** para la amplitud y **Acoustic phase** para ver
  los frentes de onda.
- **Acoustic intensity** muestra hacia dónde fluye la potencia acústica. En un
  conducto bien terminado es uniforme y está dirigida hacia la salida; en una
  cavidad resonante es casi nula.
- Un punto de monitorización sobre **Acoustic pressure** registra la amplitud en
  cada frecuencia, es decir, la función de respuesta en frecuencia en ese lugar.

### Interpretación del resultado

Para un conducto recto excitado con `v` y con terminación anecoica, la solución
analítica de onda plana es

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`constant along the duct` = constante a lo largo del conducto, `with` = con.)
Para `rho = 1.2`, `c = 344`, `v = 0.01` eso da `|p| = 4.13 Pa`, un nivel de
presión sonora de `103.3 dB` y una intensidad de `0.0206 W/m^2`. Las desviaciones
respecto a un `|p|` constante a lo largo del conducto indican que la terminación
refleja, ya sea intencionadamente o porque el coeficiente de absorción no es `1`.

Con una terminación rígida, el mismo conducto se vuelve resonante. La amplitud de
la presión presenta picos en las resonancias de cuarto de onda
`f = (2n+1) * c / (4L)` y la fase es constante en `+/-90` grados, con intensidad
neta nula.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* (el análisis acústico armónico requiere una frecuencia positiva) | la primera frecuencia es cero y el paso de frecuencia es cero |
| El residuo se estanca y los resultados parecen ruidosos | GMRES no ha convergido; aumente las iteraciones externas en la configuración del solver matricial o use una malla más gruesa |
| La amplitud se dispara en una frecuencia | el barrido ha dado con una resonancia sin amortiguar; añada un contorno absorbente o un factor de amortiguamiento acústico |
| La presión varía mucho a lo largo de un conducto supuestamente anecoico | la terminación refleja; compruebe el coeficiente de absorción o el valor de la impedancia |

---

## 5. Verificación

La prueba unitaria `tst_solver_acoustic` (en `range-solver-lib/tests`, compilada
con `-DRANGE_BUILD_TESTS=ON`) compara el solver con soluciones analíticas en un
conducto unidimensional de `1 m` discretizado con 200 elementos de barra,
utilizando la configuración estándar del solver matricial.

**Armónico, terminación anecoica**: `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz`:

| Magnitud | Analítica | Calculada |
|---|---|---|
| Amplitud de la presión | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` en cada nodo |
| Amplitud del potencial | `v/k = 1.0823e-3 m^2/s` | con un margen del 1 % |
| Velocidad de partícula | `0.01 m/s` | `0.009999 m/s` |
| Nivel de presión sonora | `103.18 dB` | con un margen de `0.1 dB` |
| Intensidad | `0.0204 W/m^2` | con un margen del 2 % |
| Gradiente de fase | `k * dx` | con un margen de 5 grados |

**Armónico, terminación rígida**: la onda estacionaria da una fase de exactamente
`+/-90` grados y una intensidad neta nula, y la amplitud máxima supera la
amplitud de la onda progresiva.

**Transitorio**: un conducto excitado con terminación anecoica permanece acotado,
el estado de Newmark se publica para que la integración continúe correctamente de
un paso a otro, y el campo excitado es distinto de cero y finito en todas partes.

---

## 6. Limitaciones

- **Sin acoplamiento estructural-acústico.** La acústica no se puede combinar con
  el análisis modal, y no hay interfaz fluido-estructura con `RSolverStress`. Una
  estructura vibrante debe representarse mediante una condición de contorno
  *Velocity* impuesta.
- **Solo acústica lineal.** Pequeña amplitud y medio en reposo. Sin flujo medio,
  sin efectos convectivos y sin propagación no lineal.
- **Solo impedancia real.** Las condiciones de contorno absorbente y de
  impedancia toman un valor real. No se admite una impedancia superficial
  compleja dependiente de la frecuencia.
- **Datos de material independientes de la frecuencia.** La velocidad del sonido
  y el factor de amortiguamiento están tabulados en función de la temperatura, no
  de la frecuencia, por lo que son constantes en todo un barrido.
- **Los términos de contorno usan la medida simple del elemento.** En un modelo
  de una o dos dimensiones cuya sección o espesor no sea `1`, los términos de
  fuente y de impedancia no se ponderan con ellos, mientras que los términos de
  dominio sí. Utilice una sección unidad o un modelo tridimensional completo para
  evitar esta discrepancia.
- **La excitación armónica es real.** Una velocidad impuesta entra como parte
  real de la carga compleja; no se pueden definir fuentes con un desfase
  relativo.
