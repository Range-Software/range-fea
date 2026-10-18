# Electrostática - Manual de teoría y guía del usuario

Este documento describe el solver de electrostática (`RSolverElectrostatics`) de
Range FEA, que atiende el tipo de problema **Electrostática**
(*Electro-statics*): las ecuaciones que resuelve, el significado de cada dato de
entrada que acepta, las partes de la interfaz gráfica que lo controlan y dos
tutoriales completos.

**Contenido**

1. [Fundamentos teóricos](#1-fundamentos-teóricos)
2. [Interfaz gráfica](#2-interfaz-gráfica)
3. [Tutorial - potencial y corriente en un conductor](#3-tutorial---potencial-y-corriente-en-un-conductor)
4. [Tutorial - calentamiento resistivo](#4-tutorial---calentamiento-resistivo)
5. [Comprobación de un modelo](#5-comprobación-de-un-modelo)
6. [Limitaciones](#6-limitaciones)

---

## 1. Fundamentos teóricos

### 1.1 Incógnita principal - el campo de potencial eléctrico

La incógnita principal es el **potencial eléctrico** `V` en cada nodo, en
voltios. Cada nodo tiene un único grado de libertad, por lo que una malla de `N`
nodos da un sistema de `N` ecuaciones. El campo eléctrico, la densidad de
corriente y todo lo que se deriva de ellos se obtienen a partir del campo de
potencial tras la resolución.

El potencial es un escalar, lo que hace que un modelo electrostático sea tan
barato como un modelo térmico de la misma malla: una incógnita por nodo en lugar
de tres, y una matriz con una novena parte de los términos de un modelo
estructural.

El potencial predeterminado de un nodo al que no afecta ninguna condición ni
ningún resultado anterior es `0 V`.

Un campo de potencial solo está definido **salvo una constante aditiva**. Nada en
las ecuaciones fija el nivel cero, por lo que todo modelo necesita al menos una
condición de contorno *Electric potential* (véase 1.7). Es el equivalente
electrostático de una estructura sin restricciones o de un modelo térmico sin
temperatura impuesta.

### 1.2 Ecuación fundamental

El solver implementa la **ecuación del campo electrostático**, la ecuación de
Poisson para el potencial de un dieléctrico lineal e isótropo:

```
div( e0 * er * grad(V) ) = -rho
```

y, sin ninguna densidad de carga, se reduce a la ecuación de Laplace

```
div( e0 * er * grad(V) ) = 0
```

El campo eléctrico es el gradiente del potencial cambiado de signo, y la
densidad de corriente se deduce de la ley de Ohm:

```
E = -grad(V)
J = sigma * E
```

| Símbolo | Significado | Unidades | Origen |
|---|---|---|---|
| `V` | potencial eléctrico | `V` | incógnita resuelta |
| `rho` | densidad de carga | `C/m^3`, véase 1.5 | condición Charge density |
| `e0` | permitividad del vacío | `F/m` | integrada, `8.854187817e-12` |
| `er` | permitividad relativa | `-` | propiedad del material |
| `sigma` | conductividad eléctrica | `S/m` | propiedad del material |
| `E` | campo eléctrico | `V/m` | obtenido, véase 1.8 |
| `J` | densidad de corriente | `A/m^2` | obtenida, véase 1.8 |

La permitividad del vacío es una constante del solver, no una propiedad del
material ni un parámetro de configuración: con el material solo se introduce la
permitividad **relativa**.

**El campo depende de la permitividad, no de la conductividad.** Es lo más
importante que hay que saber sobre este solver. La matriz de rigidez se
construye solo a partir de `e0*er`; la conductividad nunca entra en el sistema de
ecuaciones. Se utiliza después, para convertir el campo calculado en densidad de
corriente, resistividad y calor Joule. La consecuencia se detalla en 1.9: en un
cuerpo de un solo material no hay diferencia, pero en un cuerpo de **dos**
materiales el potencial se reparte en proporción a las permitividades y no en
proporción a las resistencias.

**Propiedades de material requeridas.** Un problema electrostático requiere
**tres**:

| Propiedad | Unidades | Se usa para |
|---|---|---|
| Relative permittivity | `-` | la matriz de rigidez |
| Electrical conductivity | `S/m` | densidad de corriente, resistividad, calor Joule |
| Density | `kg/m^3` | nada: requerida, nunca leída |

Una entidad cuyo material carezca de alguna de las tres **no se resuelve**,
aunque este solver nunca lea la densidad. La comprobación de la configuración lo
avisa antes de que empiece el cálculo. La única excepción es una entidad que
lleva una condición de contorno electrostática: esa entidad se incluye en el
sistema diga lo que diga su material, y su permitividad y conductividad son
entonces simplemente cero.

Una permitividad nula no es un valor predeterminado inofensivo: un elemento con
`er = 0` no tiene rigidez, por lo que no aporta nada al sistema y deja sus nodos
flotando. Si una entidad debe resolverse, asígnele una permitividad real: `1`
para el vacío o, con suficiente aproximación, para el aire.

Cada propiedad es una tabla en función de la temperatura, pero una tarea de
electrostática por sí sola no tiene ningún campo de temperatura en el que
evaluarla, por lo que todas las tablas se evalúan a `293.15 K`. Las tablas solo
se tienen en cuenta cuando una tarea *Heat transfer* situada antes en el mismo
flujo de tareas ha calculado temperaturas en los elementos, que se comparten con
las tareas siguientes.

La permitividad relativa es **adimensional** (en la interfaz aparece como `N/A`,
igual que la emisividad y el coeficiente de Poisson) y su valor se utiliza como
simple multiplicador de `e0`. Introduzca `1` para el vacío, `1.00059` para el
aire, `2.1` para el PTFE y unos `4` para el vidrio.

### 1.3 Discretización por elementos finitos

La forma débil de la ecuación del campo da, para cada elemento,

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(`stiffness` = rigidez, `charge source` = fuente de carga), donde `B` se
construye a partir de las derivadas de las funciones de forma y `N` son las
propias funciones de forma. Cada matriz de elemento es `n x n` para un elemento
de `n` nodos: una fila por nodo, no tres. No hay matriz de masa ni de capacidad:
el problema es estático, y 1.10 explica lo que esto supone para el flujo de
tareas.

**Medidas de los elementos.** Como en el resto de Range FEA, a las entidades de
menor dimensión se les asigna una medida geométrica para que puedan representar
cuerpos reales:

| Entidad | Medida | Dónde se define |
|---|---|---|
| Volumen | 1 | - |
| Superficie | espesor de la superficie | propiedades geométricas de la entidad |
| Línea | sección de la línea | propiedades geométricas de la entidad |
| Punto | volumen del punto | propiedades geométricas de la entidad |

Una **superficie** conduce el campo en su propio plano, a través de una sección
igual a su espesor. No hay gradiente de potencial a través del espesor: un
elemento de superficie tiene un solo nodo de espesor, por lo que por
construcción es equipotencial en su espesor. Por tanto, una superficie de
**espesor nulo** no tiene ninguna rigidez y no conecta nada: a diferencia del
solver térmico, donde una superficie de espesor nulo sigue llevando convección,
aquí una superficie de espesor nulo es simplemente inerte. Asigne un espesor a
una entidad de superficie siempre que deba conducir, y recuerde que una cara de
espesor nulo tampoco obtiene **resultados derivados** (1.8).

Una **línea** conduce el campo a lo largo de su propia dirección, a través de su
sección. Un **punto** no tiene ninguna dirección a lo largo de la cual conducirlo,
por lo que **no aporta rigidez**: solo contiene una fuente de carga. Un elemento
de punto cuyo nodo no comparte ningún otro elemento calculable produce por tanto
una fila vacía en la matriz, que el solver de gradiente conjugado no puede
manejar: un punto es un lugar para inyectar carga o imponer un potencial, no un
conductor.

### 1.4 Fuentes y condiciones de contorno

| Condición de contorno | Tipo | Se aplica a | Componentes |
|---|---|---|---|
| Electric potential | explícita (Dirichlet) | punto, línea, superficie, volumen | Electric potential `[V]` |
| Charge density | natural, fuente | punto, línea, superficie, volumen | Charge density, véase 1.5 |

Estas dos condiciones son todo el vocabulario de este tipo de problema. No hay
flujo impuesto, ni carga superficial, ni condición de campo lejano o de contorno
abierto, ni condición de simetría (véase la sección 6).

**Electric potential** (potencial eléctrico) es la única condición explícita del
problema electrostático. Cada nodo de la entidad a la que se asigna se elimina
del sistema de ecuaciones y se mantiene en el valor dado; su contribución pasa
al segundo miembro de las ecuaciones vecinas. Todo modelo necesita al menos una.
Una condición recién asignada empieza en `1 V`.

Donde las entidades se solapan, **prevalece la entidad de menor dimensión**: un
potencial impuesto en una superficie sustituye, en los nodos compartidos, al
impuesto en el volumen situado detrás; una línea prevalece sobre una superficie y
un punto sobre todo lo demás. Los potenciales no se combinan: solo hay un grado
de libertad por nodo que mantener, así que simplemente prevalece la última
condición leída.

**Charge density** (densidad de carga) es el término fuente de la ecuación de
Poisson. A diferencia de la condición *Heat* del solver térmico, su valor **no**
se reparte sobre la medida de la entidad: el número introducido se aplica tal
cual a cada elemento de la entidad, como densidad. Por tanto, la carga total
situada en una entidad de volumen es el valor multiplicado por el volumen de esa
entidad, y aumenta si se agranda la entidad: no se mantiene fija como un valor
*Heat* en vatios.

| Entidad | Qué significa el valor |
|---|---|
| Volumen | carga por unidad de volumen |
| Superficie | carga por unidad de volumen de la sección ponderada por el espesor |
| Línea | carga por unidad de volumen de la sección |
| Punto | una fuente de carga nodal |

Ambas componentes admiten una tabla en función del tiempo. Como el problema en sí
es estático, un historial temporal solo tiene efecto cuando otra tarea,
dependiente del tiempo, comparte el flujo de tareas, y, como explica 1.10, esa
combinación está a su vez restringida.

### 1.5 El valor de la densidad de carga

La componente *Charge density* es una **densidad volumétrica de carga** en
`C/m^3`, como indica su etiqueta en la interfaz. Es una densidad y no un total, lo
que la hace independiente de la malla y, como señala 1.4, lo que la distingue de
la condición *Heat* del solver térmico, que toma un total en vatios y lo reparte
por usted.

El signo sigue el convenio habitual. Al integrar por partes
`div(e0*er*grad(V)) = -rho`, la carga queda en el segundo miembro con signo
positivo,

```
every entity type      f = +INT( N^T * rho )
```

(`every entity type` = cualquier tipo de entidad), de modo que una densidad de
carga **positiva** eleva el potencial a su alrededor y una negativa lo reduce,
igual en entidades de volumen, superficie, línea y punto.

Recuerde que una fuente de carga por sí sola no hace que el modelo sea
resoluble. El potencial sigue definido solo salvo una constante mientras nada lo
fije, por lo que un modelo basado en cargas necesita además al menos una
condición *Electric potential*, en el contorno exterior de la región mallada si
no en otro sitio. Véase 1.7.

### 1.6 Condiciones de entorno, iniciales y de acoplamiento

#### Condiciones de entorno

| Condición de entorno | Componentes | Efecto |
|---|---|---|
| Temperature | Temperature `[K]` | se ofrece, pero no se lee (véase más abajo) |

La condición de entorno *Temperature* se ofrece para un problema electrostático
porque las propiedades de los materiales son tablas en función de la
temperatura. El solver de electrostática **no** la lee: solo toma las
temperaturas de los elementos de los datos compartidos que deja una tarea *Heat
transfer* y, en otro caso, evalúa todas las tablas a `293.15 K`. Asignarla a un
modelo solo electrostático no cambia nada.

#### Condiciones iniciales

| Condición inicial | Efecto |
|---|---|
| Electric potential | campo de potencial inicial, aplicado en el primer cálculo |

El potencial inicial solo se lee **en el primer cálculo**. Como la resolución es
directa y no un avance en el tiempo, no modifica la respuesta convergida: solo
fija el punto de partida de la iteración del gradiente conjugado y el valor de
cualquier nodo que el solver deje fuera del sistema.

Las condiciones de entorno, las condiciones iniciales y las condiciones de
contorno se leen en ese orden y cada una sobrescribe a la anterior, de modo que
**una condición de contorno prevalece sobre una condición inicial, que a su vez
prevalece sobre una condición de entorno**.

#### Acoplamiento con otros solvers

El solver de electrostática produce dos resultados que utilizan otros solvers.

| Sentido | Variable | Efecto |
|---|---|---|
| Electrostática -> térmico | Joule heat | se añade al término fuente de cada elemento |
| Electrostática -> térmico de fluidos | Joule heat | se añade al término fuente de cada elemento |
| Electrostática -> magnetostática | Current density | fuente del campo magnético |
| Térmico -> electrostática | Temperature | selecciona la fila de cada tabla de material dependiente de la temperatura |

El **calor Joule** lo recoge una tarea *Heat transfer* situada **después** de la
tarea de electrostática en el flujo de tareas del problema, donde se añade al
término fuente del elemento exactamente como lo haría una condición *Heat rate
(unit volume)*. Por tanto, el calentamiento resistivo no necesita ninguna
configuración aparte del orden de las dos tareas; el tutorial 4 lo explica paso a
paso.

La **magnetostática** es un tipo de problema independiente que *requiere* una
tarea de electrostática: lee la densidad de corriente en los elementos que
almacena este solver (en entidades de volumen, superficie y línea por igual) y
evalúa el campo magnético de esa corriente con la ley de Biot-Savart. Añada
primero la tarea de electrostática.

Como el solver se declara convergido incondicionalmente, un grupo de tareas que
solo contenga una tarea de electrostática siempre se detiene tras una
iteración. Iterar solo merece la pena cuando algo de lo que depende la resolución
va a su vez con retraso, por ejemplo una permitividad muy dependiente de la
temperatura alimentada por una tarea térmica.

### 1.7 Resolución del sistema

El solver ensambla y resuelve

```
K * V = f
```

`K` es simétrica definida positiva en cuanto se ha impuesto al menos un
potencial, por lo que se resuelve con el método del **gradiente conjugado** y
precondicionamiento de Jacobi. El ensamblaje se paraleliza por elementos con
búferes por hilo que se fusionan una sola vez al final, de modo que el resultado
no depende del número de hilos.

Un modelo **sin condición de contorno Electric potential** es singular: el
potencial solo está definido salvo una constante, la matriz tiene un núcleo no
trivial y la resolución no convergerá. Todo modelo necesita al menos un potencial
impuesto, y un modelo por el que deba circular corriente necesita dos de valores
distintos.

Conviene conocer otras tres formas de llegar a un sistema singular o vacío,
porque las tres son silenciosas:

- una entidad cuyo material carece de una de las tres propiedades requeridas no
  se resuelve, y sus nodos salen del sistema;
- una entidad con `er = 0` no tiene rigidez, por lo que sus nodos permanecen en
  el sistema pero no están conectados a nada;
- una superficie de espesor nulo, o una línea de sección nula, tampoco tiene
  rigidez.

### 1.8 Resultados derivados

Tras la resolución, los resultados en los elementos se obtienen a partir del
campo de potencial nodal. El campo eléctrico es el gradiente del potencial
cambiado de signo, promediado sobre los puntos de integración y girado al sistema
de coordenadas global en los elementos de línea y de superficie:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`electric energy` = energía eléctrica, `electrical resistivity` = resistividad
eléctrica, `Joule heat` = calor Joule.)

| Resultado | Se aplica a | Significado |
|---|---|---|
| Electric potential `[V]` | nodo | el campo de potencial resuelto |
| Electric field `[V/m]` | elemento | vector del campo |
| Current density `[A/m^2]` | elemento | `sigma*E`, nula donde la conductividad es nula |
| Electric energy `[J/m^3]` | elemento | densidad de energía del campo |
| Electrical resistivity `[V*m/A]` | elemento | `1/sigma` donde circula corriente, `0` en el resto |
| Joule heat `[W/m^3]` | elemento | disipación resistiva, que se transmite al solver térmico |

Todos ellos son **densidades**: las derivadas de las funciones de forma se
promedian sobre los puntos de integración del elemento y no se ponderan con su
determinante jacobiano, y el espesor de la superficie y la sección de la línea
permanecen en la rigidez, donde corresponden, en lugar de entrar en el campo
obtenido. Por tanto, el campo, la densidad de corriente y la energía eléctrica
son independientes del tamaño del elemento en el que se calcularon, de modo que
pueden compararse entre elementos y con un cálculo manual, y convergen a medida
que se refina la malla.

La **resistividad eléctrica** es el resultado al que recurrir para comprobar un
modelo, porque los dos factores de los que se construye se anulan:
`|E|/|sigma*E|` es exactamente `1/sigma`, haga lo que haga el campo. Es la
confirmación más rápida de que la conductividad prevista para una entidad llegó
realmente a sus elementos. Se almacena como `0` en cualquier elemento en el que
la densidad de corriente quede por debajo de la tolerancia numérica, lo que
incluye todos los elementos de conductividad nula.

El **calor Joule** es la densidad de disipación `sigma*|E|^2`, en `W/m^3`. Es
exactamente lo que espera una tarea *Heat transfer* cuando añade el valor a su
término fuente y lo integra sobre el elemento, de modo que la potencia que pasa a
un cálculo térmico acoplado es la potencia que el modelo disipa realmente. Como
los demás, es una densidad, por lo que no depende del tamaño del elemento y
converge al refinar.

Otros dos casos en los que los resultados derivados simplemente quedan en cero:
los **elementos de punto** nunca se posprocesan, y una **superficie de espesor
nulo** o una **línea de sección nula** se omiten, por lo que ambas conservan un
campo nulo por grande que sea el gradiente de potencial que las atraviesa.

Una vez escrito el registro, el registro del solver muestra las estadísticas de
las seis variables y los valores en los posibles puntos de monitorización.

### 1.9 Lo que es el solver y lo que no es

El nombre es exacto: es un solver **electrostático**. Calcula el campo de una
distribución de cargas y potenciales en un dieléctrico. La densidad de corriente
que indica es la que da la ley de Ohm para ese campo: una magnitud derivada,
calculada a posteriori, que nunca influye en el propio campo.

Un problema de **corriente continua**, en cambio, se rige por

```
div( sigma * grad(V) ) = 0
```

con la conductividad en el lugar donde este solver pone la permitividad. Ambos
dan el mismo campo de potencial en un cuerpo homogéneo, porque ambos se reducen a
la ecuación de Laplace y la constante se simplifica. Se separan en cuanto el
cuerpo está formado por más de un material:

| | Este solver | Un solver de corriente continua |
|---|---|---|
| El potencial se reparte en proporción a | `1/er` de cada región | `1/sigma` de cada región |
| Adecuado para | un condensador, un dieléctrico, un aislante | una red de resistencias, una barra colectora, un camino conductor |

Así pues, un conductor de un solo material con potenciales impuestos en sus
extremos se modela correctamente: el potencial es lineal y, dividiendo por la
longitud, se obtiene el campo. Dos conductores de distinta conductividad en serie
no: el reparto sigue a las permitividades, lo que difícilmente será lo que se
pretendía. Modele un caso así con un solo material o calcule el reparto a mano.

### 1.10 Análisis estático y solver temporal

La electrostática se declara como tipo de problema **sin** solver temporal. Esto
tiene dos consecuencias visibles.

En un modelo solo electrostático, el grupo *Solver temporal* no aparece en
absoluto en la pestaña `Problema`, el cálculo produce un único registro y no hay
nada que configurar.

Más sorprendentemente, añadir una tarea de electrostática a un flujo de tareas
**desactiva el solver temporal para todo el flujo**. El solver temporal solo se
ofrece cuando *todos* los tipos de problema del flujo lo admiten, de modo que un
análisis térmico transitorio al que se añade una tarea de electrostática se
convierte en un análisis estacionario, sin otro aviso que la desaparición del
grupo. Si se necesita una respuesta térmica transitoria para un calefactor
resistivo, ejecute la tarea de electrostática por separado y lleve su disipación
al modelo térmico transitorio como condición *Heat*, en lugar de poner ambas
tareas en el mismo flujo.

---

## 2. Interfaz gráfica

Todo lo necesario se encuentra en el panel **Solver** (pestañas `Problema`,
`Condiciones de contorno`, `Condiciones iniciales`, `Condiciones de entorno`,
`Material`, `Resultados`), el menú **Problema** y el panel **Modelo**.

### 2.1 Selección del problema

`Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`) abre la ventana del
flujo de tareas del problema. Añada una tarea y elija **Electro-statics**:
«Calculate electric field, current density and generated Joule's heat»
(calcular el campo eléctrico, la densidad de corriente y el calor Joule
generado).

La electrostática se combina libremente con todas las demás físicas excepto
*Modal analysis*. Las cadenas que conviene conocer son:

- *Electro-statics* y luego *Heat transfer*: calentamiento resistivo;
- *Electro-statics* y luego *Magneto-statics*: el campo magnético de la corriente
  calculada, que es la única forma de realizar un análisis magnetostático;
- *Electro-statics*, luego *Heat transfer* y luego *Stress analysis*: tensión
  térmica por calentamiento resistivo.

Recuerde 1.10: cualquier flujo que contenga una tarea de electrostática se
resuelve como estacionario.

### 2.2 Pestaña Problema

Para un modelo solo electrostático, esta pestaña está **vacía**. No hay solver
temporal, porque el tipo de problema no lo admite, y la electrostática no tiene
un grupo de configuración propio. Todo se asigna por entidad en las pestañas de
condiciones.

Si la pestaña muestra un grupo *Solver temporal*, lo ha puesto otra tarea del
flujo y, según 1.10, volverá a desaparecer en cuanto se añada la tarea de
electrostática.

### 2.3 Pestaña Material

La pestaña `Material` muestra el material asignado a la entidad seleccionada. La
electrostática requiere **las tres** propiedades siguientes:

| Propiedad | Unidades | Comentario |
|---|---|---|
| Relative permittivity | `N/A`, adimensional | `er`, `1` para el vacío, `1.00059` para el aire, unos `4` para el vidrio |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` para el cobre, `5e-15` para el aire |
| Density | `kg/m^3` | necesaria para que la entidad se resuelva, nunca se lee |

Una entidad a la que le falte alguna no se resuelve, y la comprobación de la
configuración avisa antes de que empiece el cálculo con *Entity ... has material
assigned which is missing required properties* (la entidad tiene asignado un
material al que le faltan propiedades requeridas). Como este solver nunca utiliza
la densidad, cualquier valor positivo cumple el requisito.

La biblioteca de materiales suministrada ya incluye las tres para los metales,
para el aire y para los vidrios, así que basta con asignar *Copper* o *Air* para
empezar.

Cada propiedad es una tabla en función de la temperatura, pero la dependencia de
la temperatura solo se tiene en cuenta cuando una tarea térmica situada antes en
el flujo ha producido temperaturas en los elementos (véase 1.6).

### 2.4 Pestaña Condiciones de contorno

La pestaña se divide en una lista de gestión de las condiciones de contorno
disponibles, arriba, y un editor de la condición seleccionada, abajo. Seleccione
primero una entidad en el árbol `Modelo`; el gestor ofrece entonces las
condiciones válidas para ese tipo de entidad y para el problema seleccionado.
Marque una para asignarla y luego edite sus componentes en el árbol inferior.

Solo se ofrecen las dos condiciones de la sección 1.4. Notas sobre su uso:

- **Electric potential** es la condición que mantiene un valor en lugar de
  impulsarlo, y se ofrece para todos los tipos de entidad. Asígnela a la cara,
  arista o punto cuyo potencial se conoce. Un modelo sin ninguna de ellas no se
  resolverá.
- **Charge density** es el término fuente, y un valor positivo eleva el potencial
  a su alrededor. Lea 1.5 antes de utilizarla: el valor es una densidad y, a
  diferencia de la condición *Heat* del solver térmico, no se reparte sobre la
  entidad.
- Ambas componentes son tablas en función del tiempo, lo que solo importa en un
  flujo de tareas impulsado por otra tarea dependiente del tiempo.

### 2.5 Condiciones iniciales y de entorno

La pestaña `Condiciones iniciales` ofrece *Electric potential*, que define el
campo de partida de la resolución iterativa. No modifica la respuesta
convergida.

La pestaña `Condiciones de entorno` ofrece *Temperature*, que este solver ignora
(véase 1.6).

### 2.6 Configuración del solver matricial

`Problema` -> `Configurar el solver matricial de los problemas` configura los
solvers iterativos. El solver de electrostática utiliza la entrada **CG**.

Si la resolución no converge, la causa habitual es un modelo sin potencial
impuesto, o una entidad de permitividad nula, más que un parámetro del solver.
Compruebe las condiciones de contorno y el material antes de aumentar el número
de iteraciones.

### 2.7 Puntos de monitorización

`Problema` -> `Definir puntos de monitorización` coloca sondas en las
coordenadas indicadas y selecciona la variable que se registra; para un problema
electrostático, la elección habitual es *Electric potential*, y es la forma más
rápida de leer un valor de un modelo como número en lugar de como color.
`Informe` -> `Puntos de monitorización` muestra los valores.

### 2.8 Resultados y registros

La pestaña `Resultados` muestra las variables calculadas y controla la vista 3D.
El potencial eléctrico es un escalar en los nodos; el campo eléctrico y la
densidad de corriente son vectores en los elementos; la energía eléctrica, la
resistividad eléctrica y el calor Joule son escalares en los elementos.

La pestaña `Registros` del panel `Modelo` muestra los registros de resultados. Un
análisis electrostático es estático, por lo que hay exactamente **uno**.

`Informe` -> `Archivo de registro del solver` muestra la salida completa del
solver, incluidas las estadísticas de las seis variables una vez escrito el
registro.

---

## 3. Tutorial - potencial y corriente en un conductor

**Objetivo.** Calcular la distribución de potencial a lo largo de una barra de
cobre mantenida a dos potenciales distintos y obtener el campo, la corriente y la
resistencia.

Este tutorial supone un modelo de volumen mallado; una caja alargada funciona
bien. Utilice `Archivo` -> `Nuevo modelo`, dibuje una caja con `Geometría` ->
`Crear elemento` o importe una geometría, y mállela en tetraedros; el tutorial
*Dibujar un cubo* trata este tema. Marque las dos caras de los extremos como
entidades de superficie independientes para poder asignarles condiciones.

Tome una barra de `0.1 m` de largo con una sección de `0.01 m` por `0.01 m`, lo
que facilita seguir los cálculos manuales del final.

### Paso 1 - seleccionar el problema

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada una tarea y seleccione **Electro-statics**.
3. Confirme con `Aceptar`.

La pestaña `Problema` permanece vacía: no hay nada que configurar.

### Paso 2 - asignar el material

1. Seleccione la entidad de volumen en el árbol `Modelo`.
2. Abra la pestaña `Material` y asigne **Copper** (cobre), o un material con las
   tres propiedades:
   - **Relative permittivity** (permitividad relativa) = `2.8`
   - **Electrical conductivity** (conductividad eléctrica) = `5.882e7` `S/m`
   - **Density** (densidad) = `8960` `kg/m^3`

Este solver no utiliza la densidad, pero debe estar presente para que la entidad
se resuelva. La permitividad es lo que determina el campo; la conductividad solo
convierte después el campo en corriente.

### Paso 3 - imponer los dos extremos

1. Seleccione la entidad de superficie de un extremo.
2. Abra la pestaña `Condiciones de contorno` y marque **Electric potential**.
3. Ajuste el valor a `0` V: es la masa.
4. Seleccione la entidad de superficie del otro extremo, marque de nuevo
   **Electric potential** y ajústelo a `10` V.

Este es el paso que hace resoluble el modelo. Un único potencial impuesto es el
mínimo; dos de valores distintos son los que crean un campo.

No añada una condición *Charge density*. Un conductor con potenciales impuestos
en los extremos no tiene carga espacial, por lo que el término fuente debe ser
cero y el potencial es una recta entre los dos extremos, lo que hace exacta la
comprobación manual del paso 6.

### Paso 4 - resolver

1. `Solución` -> `Iniciar solver` (`Ctrl+R`).
2. La comprobación de la configuración indica los materiales que faltan antes de
   que empiece el cálculo.
3. Siga el progreso en `Informe` -> `Archivo de registro del solver`.

El cálculo es una única resolución por gradiente conjugado y termina en segundos
con una malla de este tamaño.

### Paso 5 - examinar los resultados

- Pestaña `Resultados`: muestre **Electric potential**. Debería descender
  linealmente de `10` V a `0` V a lo largo de la barra, con planos
  equipotenciales planos perpendiculares a su eje. Cualquier curvatura en una
  barra homogénea significa que la malla no está conectada como se supone.
- **Electrical resistivity** debería indicar `1/5.882e7 = 1.7e-8` `V*m/A` en todos
  los elementos de la barra. Es la mejor comprobación única de que el material
  llegó a los elementos: es exacta, independiente de la malla y errónea en cuanto
  lo sea la conductividad.
- **Electric field** está dirigido a lo largo de la barra y debería indicar
  uniformemente `10/0.1 = 100` `V/m`, y **Current density**
  `sigma*E = 5.88e9` `A/m^2`. Ambas son densidades, por lo que los valores se
  mantienen sea cual sea el tamaño del elemento; el paso 6 los deduce.
- Un punto de monitorización sobre *Electric potential* a mitad de la barra
  debería indicar `5` V.

### Paso 6 - calcular los valores a mano

Las dos primeras líneas siguientes son lo que ya deberían mostrar los resultados
**Electric field** y **Current density**, por lo que son una comprobación directa
de la resolución. Las tres últimas son totales y no densidades, que ningún
resultado en los elementos proporciona, y deben calcularse. Para una barra
homogénea de longitud `L` y sección `A` sometida a una diferencia de potencial
`U`:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

La corriente es enorme porque una barra maciza de cobre de esa sección es muy
buena conductora y `10` V entre sus extremos son un cortocircuito: un recordatorio
útil de que un modelo electrostático calculará sin problema una situación que
vaporizaría la pieza real.

### Paso 7 - opcional, refinar la malla

Refine la malla una vez y vuelva a resolver. Todos los resultados (el
**potencial**, el **campo eléctrico**, la **densidad de corriente**, la **energía
eléctrica**, la **resistividad** y el **calor Joule**) deben conservar su valor.
Todos son densidades, por lo que un valor que deriva al refinar es señal de un
problema del modelo y no de una malla más fina, y merece la pena investigarlo
antes de seguir.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La resolución no converge | no hay ninguna condición *Electric potential*, por lo que el potencial solo está definido salvo una constante |
| *Entity has material assigned which is missing required properties* | falta una de las tres propiedades requeridas, a menudo la densidad, que el solver nunca utiliza |
| Todo el modelo está a un único potencial | solo se impuso un potencial, o el segundo tiene el mismo valor |
| El potencial es nulo en todas partes salvo en las caras impuestas | la entidad tiene `er = 0`, por lo que no tiene rigidez y no conecta nada |
| La resistividad es nula en todas partes | la conductividad eléctrica es nula, por lo que no circula corriente y la resistividad se almacena como cero |
| Una entidad de superficie no muestra ningún campo | su espesor es nulo, por lo que no se ensambla ni se posprocesa |

---

## 4. Tutorial - calentamiento resistivo

**Objetivo.** Tomar el calor Joule calculado a partir del campo eléctrico y
utilizarlo para calentar la barra, encadenando una tarea de electrostática y una
tarea térmica.

### Paso 1 - construir el flujo de tareas

1. `Problema` -> `Flujo de tareas de los problemas` (`Ctrl+P`).
2. Añada una tarea **Electro-statics**.
3. Añada una tarea **Heat transfer** **después** de ella.
4. Confirme con `Aceptar`.

El orden importa: el solver térmico lee el calor Joule que dejó el solver de
electrostática, por lo que la tarea de electrostática debe ejecutarse primero. No
se necesita ninguna condición ni ningún parámetro para conectarlas.

Observe lo que ocurre en la pestaña `Problema`: el grupo *Solver temporal* que
mostraría una tarea térmica sola ha desaparecido, porque la electrostática no
tiene solver temporal y el flujo adopta la configuración más restrictiva (véase
1.10). Esta cadena es solo estacionaria.

### Paso 2 - completar el material

La tarea térmica necesita cuatro propiedades propias además de las tres que
necesita la tarea de electrostática, por lo que el material de la entidad debe
tener todas las siguientes:

| Propiedad | Valor para el cobre | La necesita |
|---|---|---|
| Relative permittivity | `2.8` | electrostática |
| Electrical conductivity | `5.882e7` `S/m` | electrostática |
| Density | `8960` `kg/m^3` | ambas |
| Thermal conductivity | `401` `W/(m*K)` | térmica |
| Heat capacity | `385` `J/(kg*K)` | térmica |
| Emissivity | `0` | térmica |

El material suministrado *Copper* ya las tiene. Una entidad a la que le falte
alguna propiedad requerida por **cualquiera** de las dos tareas es omitida por esa
tarea.

### Paso 3 - conservar las condiciones de contorno eléctricas

Las dos condiciones **Electric potential** del tutorial 3 se mantienen sin
cambios. Reduzca el potencial de excitación a algo menos violento: `0.01` V entre
los extremos de la barra sigue produciendo una corriente grande y mantiene la
fuente de calor en un rango en el que la respuesta térmica es legible.

### Paso 4 - añadir las condiciones de contorno térmicas

La mitad térmica del modelo necesita su propia evacuación, exactamente como si
estuviera sola:

1. Seleccione la entidad de superficie que cubre los laterales de la barra.
2. Abra la pestaña `Condiciones de contorno` y marque **Simple convection**.
3. Ajuste **Convection coefficient** = `10` `W/(m^2*K)` y **Fluid temperature** =
   `293.15` K.

Sin una superficie con convección ni una temperatura impuesta, el modelo térmico
tiene una fuente y ninguna evacuación, y el régimen estacionario no existe. Tenga
en cuenta que una cara utilizada para la convección no necesita espesor para el
solver térmico, pero una cara de espesor nulo tampoco conduce el campo eléctrico:
mantenga las condiciones eléctricas en las caras de los extremos y la convección
en los laterales, y no interferirán.

### Paso 5 - resolver

`Solución` -> `Iniciar solver` (`Ctrl+R`). El registro muestra primero la tarea
de electrostática, con las estadísticas de sus seis variables, y luego la tarea
térmica con las estadísticas de temperatura, flujo de calor y coeficiente de
transferencia de calor.

### Paso 6 - examinar los resultados

- Pestaña `Resultados`: muestre **Joule heat**. Es el puente entre las dos
  físicas: allí donde no es nulo, el solver térmico vio una fuente.
- Muestre **Temperature**. La barra debería estar más caliente donde el calor
  Joule es mayor y descender hacia la temperatura del fluido en las caras con
  convección.
- Muestre **Electric potential** para confirmar que la mitad eléctrica del modelo
  sigue siendo el campo lineal del tutorial 3.

### Paso 7 - comprobar el balance de energía

El calor Joule es una densidad de disipación en `W/m^3`, por lo que la potencia
total que la mitad eléctrica del modelo entrega a la mitad térmica es esa
densidad multiplicada por el volumen sobre el que actúa. Para la barra homogénea
debería resultar el `P = U^2/R` del tutorial 3, con `R = L/(sigma*A)`.

En régimen estacionario, la superficie con convección tiene que evacuar
exactamente esa potencia, lo que da la misma comprobación global que describe el
manual térmico:

```
P = h * A_s * ( Ts_mean - Tf )
```

Lea la temperatura media de la superficie en las estadísticas del registro del
solver y confirme que el balance cuadra. Es la única comprobación que abarca las
dos físicas a la vez: detecta una conductividad errónea, un coeficiente de
convección erróneo y una entidad que quedó sin resolver sin avisar en cualquiera
de las dos mitades.

### Solución de problemas

| Síntoma | Causa |
|---|---|
| La temperatura es uniforme a `293.15` K | el calor Joule es nulo: compruebe la conductividad eléctrica y que la tarea de electrostática se ejecuta primero |
| La resolución térmica no converge | la mitad térmica no tiene temperatura impuesta ni superficie con convección |
| Ha desaparecido el grupo *Solver temporal* | es lo esperado: una tarea de electrostática hace estacionario todo el flujo (véase 1.10) |
| El aumento de temperatura cambia al refinar la malla | no es lo esperado: todos los resultados electrostáticos son densidades, así que compruebe la mitad térmica del modelo y la calidad de la malla |
| Solo aparecen los resultados electrostáticos | a la tarea térmica le falta una de sus cuatro propiedades de material requeridas |

---

## 5. Comprobación de un modelo

El solver no tiene un conjunto de verificación integrado, por lo que merece la
pena validar un modelo nuevo con algo que pueda calcularse a mano antes de
confiar en él. Las comprobaciones siguientes se basan en el **potencial**, que es
a la vez la incógnita principal y la magnitud en la que se expresa toda solución
cerrada; los resultados de campo eléctrico y densidad de corriente deben
coincidir con los gradientes que implican.

**Barra homogénea.** Para un cuerpo de longitud `L` mantenido a `V1` y `V2` en dos
caras opuestas, con un solo material y sin carga, el potencial es lineal a lo
largo de la barra y

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Coloque un punto de monitorización en el plano medio y confirme que indica la
media de los dos valores impuestos, y otro a un cuarto de la longitud para el
valor del cuarto. Esto detecta una malla desconectada, una condición asignada a
la entidad equivocada y una permitividad nula. Refine la malla una vez y confirme
que los valores no cambian.

**Carga espacial uniforme.** La barra anterior solo pone a prueba la rigidez.
Para comprobar también el término fuente, tome el mismo cuerpo, mantenga **ambas**
caras de los extremos a `0` V y asigne al volumen una *Charge density* uniforme
`rho`. El potencial es entonces una parábola, con `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`at the mid-plane` = en el plano medio.) Un punto de monitorización en el plano
medio da directamente `Vmax`. Se comprueban dos cosas a la vez: la magnitud, que
confirma que la permitividad y la densidad de carga se leen como se espera, y el
**signo**: una densidad de carga positiva debe producir una joroba de potencial
positiva entre dos caras a masa. Repita con la carga en una entidad de superficie
o de línea, y en un punto, y confirme que la polaridad no cambia con el tipo de
entidad.

**Resistividad.** Todo elemento por el que circule corriente debe almacenar una
resistividad eléctrica de exactamente `1/sigma`. Léala en las estadísticas del
registro del solver y compárela con la conductividad del material. Es exacta y no
aproximada, independiente de la malla, y la confirmación más rápida de que el
material llegó a los elementos previstos.

**Campo coaxial.** Para una corona entre los radios `a` y `b`, con la superficie
interior a `V1` y la exterior a `V2`, el potencial es logarítmico:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Modele la corona, imponga ambas superficies y compare puntos de monitorización en
unos cuantos radios. Esto valida el solver en un campo no uniforme, cosa que la
barra no puede hacer, y muestra si la malla es suficientemente fina cerca del
conductor interior.

**Dieléctricos en serie.** Para dos capas de espesores `d1`, `d2` y
permitividades relativas `er1`, `er2` apiladas entre dos potenciales impuestos,
el potencial en la interfaz es

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Compárelo con el potencial de interfaz calculado. Esto confirma directamente lo
que describe la sección 1.9: el reparto sigue a las **permitividades**. Ejecutar
el mismo modelo con dos conductividades distintas y la misma permitividad, y ver
que el potencial de interfaz no se mueve, demuestra que se trata de un solver
electrostático y no de uno de corriente continua.

**Resistencia y disipación.** Los resultados en los elementos son densidades, por
lo que un total (una corriente, una resistencia, una potencia) tiene que salir
del potencial. Para la barra homogénea:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Utilice estas fórmulas siempre que un total deba salir del modelo, para
dimensionar un conductor o para contrastar con una medición. `P` es también lo
que debe dar la integral del resultado **Joule heat** sobre el volumen de la
barra, lo que comprueba que las mitades eléctrica y térmica de un cálculo
acoplado coinciden.

Para cualquiera de ellas, refine la malla al menos una vez y confirme que el
potencial converge en lugar de derivar.

---

## 6. Limitaciones

### Limitaciones de modelado

- **Electrostática, no corriente continua.** La rigidez se construye a partir de
  la permitividad; la conductividad nunca entra en el sistema de ecuaciones. Por
  tanto, el potencial en un cuerpo de dos materiales se reparte en proporción a
  las permitividades, no a las resistencias. Un conductor de un solo material se
  modela correctamente; una red de resistencias no (véase 1.9).
- **Solo estático.** No hay solver temporal, ni capacidad, ni inductancia, por lo
  que los transitorios de carga, los campos alternos, la corriente de
  desplazamiento, las corrientes de Foucault y el efecto pelicular quedan fuera
  del modelo. Añadir una tarea de electrostática a un flujo de tareas elimina
  además el solver temporal de todo el flujo, por lo que una tarea térmica
  encadenada tras ella solo puede ser estacionaria (véase 1.10).
- **Solo dieléctricos lineales e isótropos.** La permitividad y la conductividad
  de un elemento son escalares y constantes durante una resolución. No se modelan
  materiales anisótropos, ferroeléctricos ni dependientes del campo, y no hay
  ruptura dieléctrica.
- **Solo dos condiciones de contorno.** Un potencial impuesto y una densidad de
  carga son todo el vocabulario. No hay carga superficial impuesta, ni flujo
  normal impuesto, ni condición de simetría, ni tratamiento de contorno abierto o
  de elementos infinitos: un problema de campo no acotado debe truncarse mallando
  suficiente aire circundante e imponiendo un potencial en su contorno exterior.
- **Sin redistribución de carga libre.** La carga no se desplaza a la superficie
  de un conductor; el solver no tiene ninguna noción de conductor más allá de las
  propiedades de material que se le dan. Modele un conductor imponiendo su
  potencial, no asignándole una conductividad grande.
- **Sin retroalimentación de la corriente.** La densidad de corriente se
  posprocesa a partir del campo, por lo que una conductividad que varíe con el
  campo, o con la temperatura que produce la disipación, no tiene efecto dentro
  de una misma resolución.
- **Las superficies son equipotenciales en su espesor.** Una entidad de
  superficie solo conduce el campo en su propio plano. Una superficie de espesor
  nulo o una línea de sección nula no tiene ninguna rigidez y se omite tanto en el
  ensamblaje como en el posprocesado.
- **Los elementos de punto no conducen.** Un punto lleva una fuente de carga y
  puede mantener un potencial impuesto, nada más, y nunca se posprocesa. Un punto
  cuyo nodo no pertenece a ningún otro elemento calculable deja una fila vacía en
  la matriz.
- **No se lee la dependencia del material respecto a la temperatura.** Las tablas
  se evalúan a `293.15 K` salvo que una tarea térmica situada antes en el flujo
  haya producido temperaturas en los elementos; la condición de entorno
  *Temperature* no tiene efecto en este solver.
- **Un modelo necesita un potencial impuesto.** Sin ninguno, la matriz tiene un
  núcleo no trivial y la resolución no convergerá.
