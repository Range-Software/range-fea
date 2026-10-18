# Flusso di fluidi - Manuale teorico e guida utente

Questo documento descrive il solutore del flusso di Range FEA - `RSolverFluid`,
che gestisce il tipo di problema **Flusso viscoso incomprimibile**
(*Incompressible viscous flow*): le equazioni che risolve, il significato di ogni
input che accetta, le parti dell'interfaccia grafica che lo controllano e un
tutorial svolto.

I due tipi di problema risolti sul campo di flusso hanno manuali propri:
[Trasmissione del calore nei fluidi](theory_fluid_heat.md) (`RSolverFluidHeat`) e
[Dispersione di contaminanti](theory_fluid_particle.md) (`RSolverFluidParticle`).

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - flusso stazionario in un canale](#3-tutorial---flusso-stazionario-in-un-canale)
4. [Verifica di un modello](#4-verifica-di-un-modello)
5. [Limitazioni](#5-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognite principali - i campi di velocità e pressione

Le incognite principali del **flusso viscoso incomprimibile** sono il vettore
**velocità** `v` in `m/s` e la **pressione** `p` in `Pa`, entrambe in ogni nodo.
Ogni nodo ha **quattro** gradi di libertà - `vx`, `vy`, `vz` e `p` - quindi una
mesh con `N` nodi dà un sistema di `4N` equazioni.

Ciò rende un modello di flusso il tipo di fisica più costoso in Range FEA:
quattro incognite per nodo contro tre di un modello strutturale e una di un
modello termico, una matrice né simmetrica né definita positiva, e l'iterazione
non lineare della sezione 1.6 attorno a ogni soluzione lineare.

I due tipi di problema associati aggiungono ciascuno uno scalare, risolto sul
campo di flusso prodotto dal solutore del fluido:

| Tipo di problema | Solutore | Incognita principale | Unità | Gradi di libertà per nodo |
|---|---|---|---|---|
| Incompressible viscous flow | `RSolverFluid` | velocità e pressione | `m/s`, `Pa` | 4 |
| Heat transfer in fluids | `RSolverFluidHeat` | temperatura | `K` | 1 |
| Contaminant dispersion | `RSolverFluidParticle` | concentrazione di particelle | `kg/m^3` | 1 |

Entrambi i tipi associati **richiedono** un task *Incompressible viscous flow*
nello stesso problema: leggono il campo di velocità dal suo risultato e non lo
calcolano mai da sé. Ciascuno è descritto in un manuale proprio -
[Trasmissione del calore nei fluidi](theory_fluid_heat.md) e
[Dispersione di contaminanti](theory_fluid_particle.md).

Lo stato predefinito di un nodo non toccato da alcuna condizione né da un
risultato precedente è velocità nulla e pressione nulla.

### 1.2 Equazioni di governo

Il solutore implementa le **equazioni di Navier-Stokes incomprimibili** per un
fluido newtoniano con densità `rho` e viscosità dinamica `mu`. Entrambe sono
proprietà del materiale valutate per elemento alla sua temperatura - vedi 1.9 per
sapere da dove viene quella temperatura e come rende il flusso soggetto a
galleggiamento. I bilanci di quantità di moto e di massa sono

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

e con il solutore temporale disattivato il termine transitorio scompare,
lasciando il bilancio stazionario

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `v` | velocità | `m/s` | incognita |
| `p` | pressione | `Pa` | incognita |
| `rho` | densità | `kg/m^3` | proprietà del materiale |
| `mu` | viscosità dinamica | `kg/(m*s)`, cioè `Pa*s` | proprietà del materiale |
| `g` | accelerazione di gravità | `m/s^2` | condizione ambientale Gravitational acceleration |

Qui la pressione non è una grandezza termodinamica. L'incomprimibilità la lascia
come moltiplicatore di Lagrange che impone `div(v) = 0`, il che ha due
conseguenze pratiche da assimilare prima di leggere qualsiasi risultato:

- Contano solo le **differenze di pressione**. Un modello in cui nessuna
  condizione fissa una pressione da qualche parte determina il campo di pressione
  solo a meno di una costante additiva, e la matrice è singolare rispetto a
  quella costante - vedi 1.7.
- La pressione risponde istantaneamente in tutto il dominio. In questa
  formulazione non ci sono onde acustiche né velocità del suono, quindi una
  pressione applicata a un'uscita si avverte all'ingresso nella stessa soluzione.

Il **numero di Reynolds** costruito da una lunghezza caratteristica `L` e da una
velocità `V`,

```
Re = rho * V * L / mu
```

è l'unico numero che decide se un modello di flusso si comporterà bene. Al di
sotto di circa `Re = 1` il termine convettivo è trascurabile e il problema è
quasi lineare; fino a qualche migliaio il flusso è laminare e il solutore è nel
suo ambito; ben oltre, il flusso reale è turbolento, e questo solutore non modella
la turbolenza - vedi la sezione 5.

### 1.3 Discretizzazione agli elementi finiti e stabilizzazione

Velocità e pressione vengono interpolate con le **stesse** funzioni di forma
lineari su ogni elemento - interpolazione di pari ordine `P1/P1` e `Q1/Q1`. Questa
combinazione da sola non soddisfa la condizione inf-sup (Babuska-Brezzi) e
produrrebbe un campo di pressione a scacchiera, quindi la formulazione è
**stabilizzata**. Si aggiungono tre termini di stabilizzazione, ciascuno con un
proprio parametro a livello di elemento:

| Termine | Scopo |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | stabilizza il termine convettivo, che altrimenti produrrebbe oscillazioni da nodo a nodo in un flusso dominato dalla convezione |
| **PSPG** - pressure stabilising Petrov-Galerkin | rende ammissibili velocità e pressione di pari ordine, eliminando il modo di pressione a scacchiera |
| **LSIC** - least-squares on the incompressibility constraint | smorza l'errore di divergenza, migliorando la conservazione della massa elemento per elemento |

I parametri SUPG e PSPG sono costruiti da un numero di Reynolds di elemento

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`element length along the flow direction` = lunghezza dell'elemento lungo la
direzione del flusso, `element Reynolds number` = numero di Reynolds di elemento,
`for` = per), quindi l'upwinding è forte dove l'elemento è dominato dalla
convezione e si attenua dove è dominato dalla diffusione. Il parametro LSIC è
`|v| * h / 2`. Questi parametri appartengono al solutore del flusso; il solutore
di dispersione usa un proprio parametro SUPG limitato dal passo temporale, vedi
il manuale [Dispersione di contaminanti](theory_fluid_particle.md).

Il parametro SUPG usa la velocità e la lunghezza **locali** dell'elemento, quindi
si adatta elemento per elemento. Il parametro PSPG usa invece una **velocità di
corrente globale** e una scala di lunghezza globale degli elementi, il che
mantiene uniforme la stabilizzazione della pressione su tutta la mesh. La
velocità di corrente viene presa dalle condizioni di ingresso al primo passaggio
di ogni soluzione - ogni passo temporale in un'esecuzione transitoria - anziché
dal campo che si sta risolvendo,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

(`over inflow surfaces` = sulle superfici di ingresso, `area of inflow surfaces` =
area delle superfici di ingresso), cioè la velocità media di ingresso pesata
sull'area, con `1 m/s` come ripiego quando il modello non ha alcuna condizione di
ingresso. La scala di lunghezza dell'elemento deriva dal volume dell'elemento:
`cbrt(6*V/pi)` per un tetraedro, `cbrt(V)` per un esaedro.

**Elementi supportati.** Le equazioni del flusso vengono assemblate **solo sugli
elementi di volume**, e solo su **tetraedri lineari** (`TETRA1`) ed **esaedri
lineari** (`HEXA1`). Qualsiasi altro tipo di elemento di volume interrompe
l'esecuzione con un errore che ne indica il tipo. Gli elementi di superficie
partecipano solo dove portano una condizione *Pressure (implicit)*, e lì
contribuiscono con una trazione di pressione e nient'altro. Le entità punto e
linea non contribuiscono affatto - a differenza di un modello termico, non sono
conduttori di dimensione inferiore, sono semplicemente assenti.

Questa è la differenza pratica più netta rispetto alle altre fisiche di Range
FEA: un modello di fluido deve essere un **volume meshato**. Genera la mesh
tetraedrica con `Geometria` -> `Volume` -> `Genera mesh tetraedrica` prima di
aspettarti che una soluzione del flusso faccia qualcosa.

### 1.4 Condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Wall | esplicita | punto, linea, superficie | nessuna |
| Wall (frictionless) | esplicita | punto, linea, superficie | nessuna |
| Velocity (inflow) | esplicita | superficie | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | esplicita | superficie | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | esplicita | superficie | Pressure `[Pa]` |
| Pressure (implicit) | naturale, trazione | superficie | Pressure `[Pa]` |

**Wall** è la condizione di aderenza. In ogni nodo dell'entità tutte e tre le
componenti della velocità vengono rimosse dal sistema e mantenute a zero. Non ha
componenti - non c'è nulla da inserire, attivarla è l'intera configurazione. È
la condizione per ogni contorno solido toccato dal fluido.

**Wall (frictionless)** è la condizione di scorrimento, o di simmetria: il fluido
può muoversi lungo la superficie ma non attraversarla. Si applica trovando l'**asse
globale dominante** della normale di ogni elemento di superficie e mantenendo a
zero solo quella componente della velocità, lasciando libere le altre due. Nota
cosa significa: la condizione è esatta solo per una superficie la cui normale
giace lungo `x`, `y` o `z`. Su una superficie inclinata rispetto agli assi globali
vincola la direzione sbagliata, e più la normale si avvicina a 45 gradi tra due
assi, peggiore è l'approssimazione. Usala per i piani di simmetria piatti di un
modello allineato agli assi globali, e usa *Wall* altrove.

**Velocity (inflow)** prescrive una velocità del modulo dato diretta lungo la
**normale media** dell'entità superficie, rivolta verso il fluido. Il segno è
preso dall'orientamento degli elementi dell'entità rispetto al volume
retrostante, quindi un valore positivo indica flusso che entra nel dominio e un
valore negativo flusso che ne esce. Inserisci un modulo, non un vettore - la
direzione deriva dalla geometria.

**Volumetric flow rate (inflow)** prescrive la stessa cosa a partire dalla
portata totale: il solutore divide la portata per l'area dell'entità e applica la
velocità media risultante lungo la stessa normale media.

```
v = Q / A          A = area of the entity
```

(`area of the entity` = area dell'entità.) Usala ogni volta che la grandezza che
conosci davvero è una portata - la prestazione di una pompa, un condotto in
`m^3/s` - e usa *Velocity (inflow)* quando conosci la velocità. Entrambe
mantengono tutte e tre le componenti della velocità in ogni nodo dell'entità,
quindi entrambe rendono l'ingresso un flusso uniforme a pistone; nessuna delle due
può produrre un profilo sviluppato sulla faccia di ingresso. Dai al modello un
tratto di ingresso se il profilo lì è importante.

**Pressure (explicit)** rimuove il grado di libertà della pressione da ogni nodo
dell'entità e lo mantiene al valore dato. È l'equivalente in pressione di una
velocità prescritta, e uno dei modi per fissare la costante additiva della
sezione 1.2.

**Pressure (implicit)** è la controparte naturale: anziché fissare la pressione
nodale applica una **trazione di pressione** sugli elementi di superficie e lascia
libera lì la velocità, così che sia il fluido a decidere quanto esce e con quale
profilo. La trazione include un battente idrostatico,

```
t = -( p + rho * g * h ) * n
```

dove `h` è l'altezza del nodo misurata **lungo la direzione della gravità a
partire dal punto più basso** del modello e `n` è la normale della superficie. La
direzione della gravità è la media delle condizioni ambientali *Gravitational
acceleration* assegnate alle superfici che portano questa condizione. Senza
gravità il termine di battente scompare e la trazione è la sola pressione
inserita.

È la condizione di uscita da usare in quasi ogni modello: lascia sviluppare il
profilo di uscita anziché imporlo piatto, e inserendo `0` l'uscita diventa un
riferimento alla pressione atmosferica. *Pressure (explicit)* è la scelta più
rigida e prescrittiva.

Ogni componente di ognuna di queste condizioni è una **tabella in funzione del
tempo**, quindi un ingresso può essere aumentato gradualmente nei primi secondi di
un'esecuzione transitoria, o a una portata si può dare un ciclo di lavoro.

**Precedenza.** Le condizioni di velocità e di pressione sono indipendenti -
vincolano gradi di libertà diversi, quindi una superficie di ingresso può portare
una condizione di velocità e un'uscita una condizione di pressione senza
interferire. Tra condizioni dello stesso tipo sulla stessa entità prevale l'ultima
letta, e una *Wall* azzera la velocità dei suoi nodi dopo che tutte le altre
condizioni di velocità sono state applicate. Un'entità che è sia ingresso sia
parete è quindi una parete.

### 1.5 Condizioni ambientali e iniziali

#### Condizioni ambientali

| Condizione ambientale | Componenti | Effetto |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | forza di volume `rho*g` sull'entità, e il termine di battente di *Pressure (implicit)* |
| Temperature | Temperature `[K]` | temperatura di fondo di un task *Heat transfer in fluids*; non letta dal solutore del flusso stesso - vedi 1.9 |

**Gravitational acceleration** vale per impostazione predefinita
`(0, 0, -9.80665) m/s^2`, cioè gravità lungo `Z` negativo. Assegnala all'entità
volume per la forza di volume e alla superficie di uscita, in modo che il
battente di *Pressure (implicit)* venga misurato lungo la direzione corretta.

La gravità entra nell'equazione della quantità di moto come forza di volume
`rho*g`. In un dominio chiuso a densità uniforme, un campo di gravità uniforme
produce un gradiente di pressione idrostatico e nessun moto - che è la risposta
corretta. Dove la densità varia con la temperatura, la forza di volume varia con
essa e la differenza genera un **flusso di galleggiamento** - convezione naturale,
vedi 1.9. Il risultato della pressione contiene sempre la parte idrostatica
`rho*(g . x)`; non viene sottratto nulla.

#### Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Velocity | campo di velocità iniziale, applicato alla prima esecuzione |
| Pressure | campo di pressione iniziale, applicato alla prima esecuzione |

I campi iniziali di temperatura e concentrazione dei tipi di problema associati
sono descritti nei rispettivi manuali.

Le condizioni iniziali vengono lette **solo alla prima esecuzione**; in seguito i
campi avanzano a partire dal risultato precedente. È ciò che fa funzionare il
flusso di lavoro con riavvio della sezione 1.7: un'esecuzione transitoria avviata
con *Riavvia solutore / continua* riprende il campo stazionario convergente
anziché ripartire da fermo.

Una condizione al contorno prevale su una condizione iniziale dove le due si
incontrano.

### 1.6 L'iterazione non lineare

Il termine convettivo `(v . grad) v` è quadratico nell'incognita, quindi le
equazioni del flusso sono **non lineari** e non possono essere risolte in un solo
passaggio. Il solutore le linearizza e itera: ogni passaggio assembla il residuo
dei bilanci di quantità di moto e di massa al campo corrente, risolve per un
**incremento** e lo aggiunge,

```
J * dx = -R(x)
x <- x + dx
```

dove `x` contiene le velocità e le pressioni nodali. Il primo membro è uno
jacobiano approssimato anziché quello esatto, il che rende lo schema
un'iterazione di Newton modificata: converge più lentamente del Newton completo
vicino alla soluzione, ma la soluzione verso cui converge è fissata dal
**residuo** `R`, che sono le equazioni di Navier-Stokes discretizzate stesse.

**Il passo è smorzato.** Poiché la matrice non è la derivata esatta del residuo,
un passo completo `dx` può andare oltre e lasciare il campo più lontano da una
soluzione di quanto fosse all'inizio - il residuo del passaggio successivo
risulta allora più alto del precedente. Il solutore controlla esattamente questo
e applica solo una quota del passo calcolato,

```
x <- x + omega * dx
```

partendo da `omega = 1`. Un passaggio che ha peggiorato il residuo lo dimezza,
fino a un minimo di `0.1`; un passaggio che lo ha migliorato lascia crescere
`omega` di un quarto alla volta, così la ritirata è rapida e il recupero
ponderato. Il valore in vigore viene stampato nel log come `Relaxation`, e resta a
`1` in un'esecuzione che non va mai oltre - un modello di flusso ben comportato
non si accorge mai che il meccanismo esiste. `omega` viene riportato a `1` all'inizio
di ogni soluzione, il che in un'esecuzione transitoria significa a ogni passo
temporale.

Ne derivano due conseguenze, che determinano l'intero flusso di lavoro di un
modello di flusso:

- **Un passaggio non basta mai.** Una singola iterazione di un task di flusso
  restituisce ciò che ha dato il primo incremento, che non è la soluzione di
  nulla. Un task di flusso deve essere inserito in un **gruppo di task del
  problema** con un numero di iterazioni - centinaia per un'esecuzione
  stazionaria, decine per passo temporale per una transitoria.
- **Il conteggio è un limite superiore, non un obiettivo.** Il gruppo ha anche un
  **valore di convergenza**, e termina le sue iterazioni non appena il task di
  flusso riporta una convergenza inferiore a esso. Imposta il conteggio
  abbastanza alto per il passo più difficile e lascia che il valore di
  convergenza fermi ciascun passo quando si è stabilizzato.

Dopo ogni iterazione il solutore scrive quattro numeri nel file di convergenza e
nel log:

| Grandezza | Significato |
|---|---|
| `Residual` | norma del vettore residuo, la distanza da una soluzione |
| `Convergence-R` | variazione del residuo rispetto all'iterazione precedente |
| `Convergence-V` | dimensione relativa dell'incremento di velocità, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | dimensione relativa dell'incremento di pressione, `\|\|dp\|\| / \|\|p\|\|` |

`Convergence-V` e `Convergence-P` sono quelli confrontati con il valore di
convergenza. Entrambi sono adimensionali ed entrambi diminuiscono man mano che
l'iterazione si stabilizza: misurano quanto è grande il passo che il campo compie
ancora rispetto alla grandezza del campo stesso, quindi un valore di `1e-5`
significa che la soluzione si è spostata di un centomillesimo di sé stessa in quel
passaggio.

Un passo piccolo, però, non è di per sé una soluzione. Un sistema quasi
singolare - un modello senza riferimento di pressione, o una mesh che non può sostenere il
numero di Reynolds richiesto - compie passi minuscoli perché **non può muoversi**,
non perché è arrivato, e il suo residuo resta dov'era o sale. Il test ha quindi
due parti, e il gruppo si ferma solo quando valgono entrambe:

| Parte | Condizione |
|---|---|
| Il campo ha smesso di muoversi | `Convergence-V` e `Convergence-P` sono entrambi sotto il valore di convergenza |
| Il campo è effettivamente sceso | il residuo è sceso a un **decimo** di quello che era al primo passaggio di questa soluzione |

La seconda parte è fissa e non configurabile - è un controllo di plausibilità
della prima, non una seconda manopola da regolare. Il log la stampa come
`Residual ratio` rispetto al suo obiettivo, così è visibile quanto un'esecuzione
che non si è fermata in anticipo sia stata vicina a farlo. In un'esecuzione
transitoria ogni passo temporale è una soluzione a sé, quindi il rapporto viene
misurato di nuovo dall'inizio di ogni passo.

`Report` -> `Convergenza del solutore` traccia tutti e quattro i numeri.
Un'esecuzione stazionaria convergente mostra il residuo che scende di diversi
ordini di grandezza e poi si appiattisce, con `Convergence-V` e `Convergence-P`
che scendono in modo monotono verso zero. Un residuo che si appiattisce in alto,
oscilla o sale indica che l'esecuzione non è convergente - e il gruppo non si
fermerà in anticipo su un'esecuzione del genere, qualunque cosa facciano gli
incrementi.

Un valore di convergenza di `0` disattiva l'uscita anticipata ed esegue l'intero
conteggio, che è il comportamento del solutore prima che il valore esistesse.
Usalo quando vuoi una quantità fissa di lavoro per passo indipendentemente da
cosa faccia il residuo.

### 1.7 Analisi stazionaria

Con il solutore temporale disattivato il solutore itera i bilanci stazionari di
quantità di moto e di massa fino a un punto fisso. Il sistema lineare di ogni
iterazione non è simmetrico, quindi viene risolto con **GMRES** con
precondizionamento di Jacobi anziché con il metodo del gradiente coniugato usato
dalle fisiche simmetriche.

Un modello di flusso stazionario richiede come minimo:

- un **volume** meshato con densità e viscosità dinamica assegnate;
- **Wall** su ogni contorno solido;
- una condizione di **ingresso**, o una differenza di pressione, per generare il
  flusso;
- un **riferimento di pressione** - una superficie *Pressure (explicit)* o
  *Pressure (implicit)*. Senza di esso la pressione è determinata solo a meno di
  una costante e l'iterazione non si stabilizza.

Un modello i cui contorni sono interamente pareti e velocità prescritte ha
un'ulteriore insidia: le portate prescritte in ingresso e in uscita devono
**bilanciarsi**, poiché un fluido incomprimibile non può accumularsi. Prescrivere
5 m^3/s in ingresso e 4 m^3/s in uscita chiede l'impossibile e la soluzione non
converge. Lasciare l'uscita come *Pressure (implicit)* evita del tutto la
questione, ed è il motivo principale per preferirla.

Il flusso di lavoro abituale consiste nel far convergere prima un'esecuzione
stazionaria e usarla come campo di partenza di un'esecuzione transitoria, perché
un'esecuzione transitoria avviata da fermo spende i primi passi temporali a
risolvere il transitorio di avviamento anziché la fisica di interesse.

### 1.8 Analisi transitoria

Con il solutore temporale attivo il solutore fa avanzare le equazioni non
stazionarie con uno schema theta, dove `alpha` è il coefficiente
dell'approssimazione dell'avanzamento nel tempo:

| Approssimazione | `alpha` | Avanzamento risultante | Stabilità |
|---|---|---|---|
| Backward difference (stable) | `1` | passo completamente implicito | incondizionata |
| Central difference (accurate) | `0.5` | Crank-Nicolson | incondizionata, può oscillare |
| Forward difference (fast) | `0` | passo esplicito | condizionata |

**Backward difference** è l'impostazione da usare per un modello di flusso.
L'iterazione non lineare all'interno di ogni passo temporale è ciò che costa
tempo, e un avanzamento implicito permette di scegliere il passo temporale in base
alla fisica anziché alla mesh.

Anche con l'avanzamento implicito il passo temporale di un modello di flusso è
limitato dall'accuratezza, tramite il **numero di Courant**

```
C = |v| * dt / h
```

dove `h` è la dimensione dell'elemento lungo il flusso. Un numero di Courant
vicino a `1` significa che il fluido attraversa circa un elemento per passo, che è
l'ordine giusto per risolvere un transitorio; molto di più e il solutore sta
appiattendo il trasporto che dovrebbe risolvere, qualunque sia la stabilità
dell'avanzamento. Scegli `dt` in base alla mesh e alla velocità prevista:

```
dt ~ h / |v|
```

Ogni passo temporale esegue l'intera iterazione non lineare della sezione 1.6,
quindi il costo di un'esecuzione transitoria è il numero di passi temporali per
le iterazioni per passo. Da dieci a cinquanta iterazioni per passo è un
intervallo tipico una volta che il campo di flusso è già convergente da
un'esecuzione stazionaria. Un passo il cui residuo non è sceso entro la fine delle
sue iterazioni è un passo la cui risposta non è convergente, e l'errore si
trascina in ogni passo successivo.

### 1.9 Trasmissione del calore e dispersione di contaminanti

Sul campo di flusso prodotto da questo solutore si risolvono due tipi di
problema, ciascuno con un manuale proprio:

- **Trasmissione del calore nei fluidi** (`RSolverFluidHeat`) trasporta la
  temperatura con il flusso e si accoppia al solutore termico di un solido
  meshato - vedi [Trasmissione del calore nei fluidi](theory_fluid_heat.md);
- **Dispersione di contaminanti** (`RSolverFluidParticle`) trasporta una
  concentrazione di contaminante con il flusso - vedi
  [Dispersione di contaminanti](theory_fluid_particle.md).

Entrambi leggono la velocità dal risultato del flusso. La dispersione di
contaminanti non modifica mai il flusso. La trasmissione del calore nei fluidi
sì, tramite le proprietà del materiale.

#### Proprietà dipendenti dalla temperatura e galleggiamento

A **ogni passaggio** il solutore del flusso valuta densità e viscosità dinamica
di ogni elemento dalle tabelle del materiale alla temperatura dell'elemento
lasciata dall'ultimo passaggio di trasmissione del calore. Senza un task di
trasmissione del calore nel problema quella temperatura è ovunque il valore
predefinito `293.15 K`. La condizione ambientale *Temperature* non viene letta
direttamente dal solutore del flusso; raggiunge il flusso solo tramite un task di
trasmissione del calore.

La densità nella forza di volume `rho(T)*g` è ciò che rende il flusso soggetto a
galleggiamento: il fluido caldo è più leggero di quello circostante e sale,
quello freddo scende. Convezione naturale, pennacchi termici e stratificazione
derivano quindi da un gruppo composto da un task *Incompressible viscous flow* e
da un task *Heat transfer in fluids*, purché il materiale abbia una **tabella di
densità dipendente dalla temperatura**. I materiali *Water* e *Air* forniti la
hanno. Un materiale la cui densità è un unico valore non dà galleggiamento per
nessuna differenza di temperatura.

Come i due task si scambiano i dati:

- **Stazionario.** Metti entrambi i task in un unico gruppo, prima il task di
  flusso, con diverse iterazioni e un valore di convergenza. Il primo passaggio
  del flusso vede la temperatura iniziale - la condizione iniziale *Temperature*,
  o `293.15 K` - quindi non ha ancora galleggiamento. Dalla seconda iterazione in
  poi, ogni passaggio del flusso vede la temperatura del passaggio termico
  precedente. Un modello generato dal solo galleggiamento richiede iterazioni
  sufficienti perché i due si stabilizzino insieme; un gruppo con una singola
  iterazione non li accoppia mai.
- **Transitorio.** Ogni passo temporale esegue di nuovo il gruppo, e ogni
  passaggio del flusso vede l'ultima temperatura. Con un'iterazione per passo
  temporale il flusso usa la temperatura del passo precedente - un ritardo di un
  passo, accettabile finché il passo temporale risolve il transitorio termico.
  Più iterazioni per passo lo eliminano.

Il gruppo termina quando converge il task di flusso, poiché un task *Heat transfer
in fluids* non accoppiato si dichiara convergente a ogni passaggio. Una variazione
della temperatura sposta il residuo del flusso tramite `rho(T)` e `mu(T)`, quindi
un flusso convergente è anche un flusso che la temperatura non spinge più.

La formulazione mantiene `rho(T)` in ogni termine del bilancio della quantità di
moto e `div(v) = 0` per la massa. Per le piccole differenze di densità della
convezione naturale in un liquido, o in un gas su qualche decina di kelvin, ciò
dà la stessa risposta dell'approssimazione di Boussinesq. Non è una formulazione
comprimibile: la dilatazione termica del fluido non entra nel bilancio di massa,
quindi un gas riscaldato di centinaia di kelvin è fuori dal suo ambito.

### 1.10 Risultati derivati

| Risultato | Si applica a | Prodotto da | Significato |
|---|---|---|---|
| Velocity `[m/s]` | nodo | flusso | il campo di velocità risolto, tre componenti |
| Pressure `[Pa]` | nodo | flusso | il campo di pressione risolto |

I risultati dei tipi di problema associati sono elencati nei rispettivi manuali.

La velocità è memorizzata come **vettore** nodale, quindi la scheda `Risultati`
può visualizzarne il modulo come colore, le componenti singolarmente o frecce. Due
strumenti di visualizzazione sono particolarmente utili per un risultato di
flusso:

- un **piano di sezione** attraverso il volume, poiché la struttura interessante
  di un flusso è all'interno del dominio e non sulla sua superficie;
- le **linee di flusso**, create da `Geometria` -> `Linea di flusso`, che
  tracciano il campo di velocità e sono il modo più rapido per vedere un ricircolo
  o una zona morta.

Dopo ogni record il log del solutore stampa le statistiche della velocità e della
pressione, e i valori in tutti i punti di monitoraggio.

### 1.11 Stazionario o transitorio?

| | Stazionario | Transitorio |
|---|---|---|
| Risolve | il bilancio stazionario, iterato | le equazioni non stazionarie, fatte avanzare nel tempo |
| Risponde a | «come appare il flusso sviluppato» | «come ci arriva il flusso, e cosa trasporta» |
| Iterazioni | centinaia, in un gruppo di task | decine per passo temporale |
| Record | uno | uno per ogni passo temporale scritto |
| Richiede un riferimento di pressione | sì | sì |

**Usa un'analisi stazionaria per** il campo di flusso sviluppato di un condotto,
di un collettore o del passaggio di uno scambiatore di calore, la perdita di
carico attraverso un componente, e come campo di partenza per qualsiasi
esecuzione transitoria. È da qui che parte quasi ogni modello di flusso.

**Usa un'analisi transitoria per** avviamento e arresto, un ingresso pulsante o
ciclico, il distacco di vortici e - soprattutto - per la **dispersione di
contaminanti**, che è un problema di trasporto nel tempo e non ha una risposta
stazionaria significativa per un evento di rilascio.

Il flusso di lavoro in due fasi del tutorial *Dispersione di contaminanti nei
fluidi* è quello canonico: far convergere il flusso in regime stazionario, poi
riavviare con il solutore temporale e il task di dispersione attivati - vedi il
manuale [Dispersione di contaminanti](theory_fluid_particle.md).

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Condizioni ambientali`,
`Materiale`, `Risultati`), nel menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Aggiungi un task e scegli **Incompressible viscous
flow** - "Steady-state and transient flow of newtonian fluids" (flusso
stazionario e transitorio di fluidi newtoniani).

Un task di flusso va inserito in un **gruppo di task**, che porta i due valori che
guidano l'iterazione non lineare di 1.6. Fai doppio clic su uno dei due per
modificarlo:

| Valore | Significato |
|---|---|
| **N. di iterazioni** | il numero massimo di passaggi che una singola soluzione - o un singolo passo temporale - può richiedere |
| **Convergenza** | il gruppo termina i suoi passaggi quando il task di flusso si è stabilizzato al di sotto di questo valore **e** il suo residuo è sceso; `0` esegue sempre l'intero conteggio |

Il valore di convergenza predefinito è `1e-5`, una dimensione relativa del passo
sicura per un modello di flusso. Mettere un task di flusso fuori da un gruppo,
dove viene eseguito esattamente una volta, non è utile - vedi 1.6.

Le combinazioni da conoscere:

- *Incompressible viscous flow* da solo, in un gruppo di diverse centinaia di
  iterazioni - un campo di flusso stazionario;
- *Incompressible viscous flow* e *Contaminant dispersion* in un unico gruppo -
  dispersione sul flusso, la classica coppia transitoria, vedi
  [Dispersione di contaminanti](theory_fluid_particle.md);
- *Incompressible viscous flow* e *Heat transfer in fluids* in un unico gruppo -
  un flusso riscaldato o raffreddato, vedi
  [Trasmissione del calore nei fluidi](theory_fluid_heat.md).

Il flusso esclude *Flow through porous media* e *Modal analysis*; i due tipi di
problema associati richiedono ciascuno la presenza di un task di flusso.

### 2.2 Scheda Problema

**Solutore temporale** - mostrato per tutti e tre i tipi di problema dei fluidi,
poiché dipendono tutti dal tempo. Lascialo disattivato per un'esecuzione di flusso
stazionaria.

| Campo | Significato |
|---|---|
| Abilita | attiva e disattiva l'avanzamento transitorio |
| Approssimazione | theta dell'avanzamento nel tempo: all'indietro, centrale o in avanti, vedi 1.8 |
| Tempo iniziale | primo valore di tempo in secondi; un riavvio da un risultato stazionario colloca quel risultato al tempo iniziale, un riavvio di un'esecuzione transitoria interrotta prosegue dal suo ultimo passo |
| Tempo finale | `start time + time-step size * number of time-steps`, sola lettura |
| Dimensione del passo temporale | `dt` in secondi, vedi l'indicazione di Courant in 1.8 |
| Numero di passi temporali | quanti passi calcolare |
| Frequenza di output | scrive un file di risultati ogni N passi; `0` scrive solo l'ultimo passo |

Il tipo di problema del flusso non ha un riquadro di impostazioni proprio; il
riquadro *Impostazioni della dispersione di contaminanti* è descritto nel manuale
[Dispersione di contaminanti](theory_fluid_particle.md).
Tutto il resto si assegna per entità nelle schede delle condizioni, e il numero di
iterazioni - che qui conta più che in qualsiasi altro punto di Range FEA - si
trova nella finestra del flusso dei task del problema e non in questa scheda.

### 2.3 Scheda Materiale

La scheda `Materiale` elenca il materiale assegnato all'entità selezionata. Il
tipo di problema del flusso richiede:

| Proprietà | Unità | Commento |
|---|---|---|
| Density | `kg/m^3` | `rho`, circa `1000` per l'acqua, `1.2` per l'aria |
| Dynamic viscosity | `kg/(m*s)` | `mu`, circa `1.0e-3` per l'acqua, `1.8e-5` per l'aria |

I tipi di problema associati elencano le proprie proprietà nei rispettivi
manuali. Un'entità a cui manca una proprietà richiesta da un tipo di problema del
flusso dei task non viene risolta, e il controllo della configurazione avvisa
prima dell'avvio dell'esecuzione. Il database dei materiali fornito include
**Water** e **Air**, il che di solito esaurisce questo passaggio.

Nota che la viscosità richiesta è quella **dinamica** `mu` in `Pa*s`, non la
viscosità cinematica `nu = mu/rho` in `m^2/s`. Inserire `1.0e-6` per l'acqua
perché è il numero familiare è l'errore di materiale più comune in un modello di
flusso, e rende il fluido mille volte meno viscoso di quanto dovrebbe - il che di
solito si manifesta come un'esecuzione che non converge.

Ogni proprietà è una tabella in funzione della temperatura, valutata alla
temperatura dell'elemento dell'esecuzione precedente.

### 2.4 Scheda Condizioni al contorno

La scheda è suddivisa in un elenco gestore delle condizioni al contorno
disponibili in alto e in un editor per quella selezionata in basso. Seleziona
prima un'entità nell'albero `Modello`; il gestore offre allora le condizioni
valide per quel tipo di entità e per i tipi di problema selezionati. Attivane una
per assegnarla, poi modificane le componenti nell'albero inferiore.

Vengono offerte le condizioni della sezione 1.4. Note sul loro uso:

- **Wall** va su ogni contorno solido e non accetta alcun valore. Un'entità
  superficie di un volume meshato che non porta alcuna condizione *non* è una
  parete - è un contorno non vincolato, il che raramente è ciò che si intende. Le
  pareti sono la prima cosa da controllare in un modello che si comporta in modo
  strano.
- **Wall (frictionless)** serve per i piani di simmetria allineati agli assi
  globali. Su una superficie inclinata vincola la direzione sbagliata - vedi 1.4.
- **Velocity (inflow)** e **Volumetric flow rate (inflow)** spingono entrambe il
  flusso all'interno attraverso una superficie lungo la sua normale media.
  Scegli quella che corrisponde al numero che hai davvero. Un valore negativo
  spinge il flusso nell'altro verso, il che è un modo per specificare un'uscita.
- **Pressure (implicit)** è la condizione di uscita da preferire: lascia
  sviluppare il profilo di uscita e, inserita come `0`, rende l'uscita il
  riferimento di pressione. **Pressure (explicit)** fissa invece la pressione
  nodale.
- Le condizioni di *Heat transfer in fluids* e *Contaminant dispersion*
  compaiono quando il loro task è nel flusso dei task, e sono descritte nei
  rispettivi manuali.
- Ogni componente è una tabella in funzione del tempo. Fai clic su **Modifica
  valori dipendenti dal tempo** per aprire l'editor dei componenti, in cui si
  inseriscono i valori in funzione del tempo; tra due istanti dati il valore viene
  interpolato linearmente, e dopo l'ultimo istante si mantiene l'ultimo valore. È
  così che si modella un evento di rilascio, un ingresso con rampa o un ciclo di
  lavoro.

### 2.5 Condizioni iniziali e ambientali

La scheda `Condizioni iniziali` offre *Velocity* e *Pressure* per un problema di
flusso. Impostano il campo di partenza della prima esecuzione e sono ciò che
permette a un'esecuzione transitoria di partire da qualcosa di diverso dalla
quiete.

La scheda `Condizioni ambientali` offre *Gravitational acceleration*, che vale per
impostazione predefinita `(0, 0, -9.80665) m/s^2`. Assegnala al volume per la
forza di volume, e a ogni superficie che porta *Pressure (implicit)*, in modo che
il battente idrostatico di quella condizione venga misurato lungo la direzione
corretta.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Il solutore del flusso usa la voce **GMRES**, poiché la sua
matrice non è simmetrica.

Distingui le due iterazioni annidate prima di modificare qualcosa qui: le
iterazioni del **solutore matriciale** risolvono un sistema linearizzato, mentre
le iterazioni del **gruppo di task** gestiscono la non linearità della sezione
1.6. Un modello di flusso che non converge ha quasi sempre bisogno di più
iterazioni del gruppo di task, di una mesh migliore o di condizioni al contorno
corrette - non di un numero di iterazioni GMRES più alto.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare - *Velocity* o *Pressure* per il flusso. In
un'esecuzione transitoria l'andamento è mostrato da `Report` -> `Punti di
monitoraggio`.

### 2.8 Risultati e record

La scheda `Risultati` elenca le variabili calcolate e controlla la vista 3D. La
velocità è un vettore nodale e la pressione uno scalare nodale.

La scheda `Record` del pannello `Modello` elenca i record dei risultati:

- **analisi stazionaria** - un unico record;
- **analisi transitoria** - un record per ogni passo temporale scritto.
  L'animazione dei record riproduce il flusso, e il pulsante di registrazione in
  fondo all'albero scrive l'animazione in un file video.

`Report` -> `Convergenza del solutore` traccia l'andamento della convergenza
descritto in 1.6, ed è il report da aprire per primo dopo qualsiasi esecuzione di
flusso. `Report` -> `File di log del solutore` mostra l'output completo del
solutore, comprese le statistiche di velocità e pressione dopo ogni record.

---

## 3. Tutorial - flusso stazionario in un canale

**Obiettivo.** Calcolare il campo di flusso sviluppato e la perdita di carico di
un fluido spinto attraverso un canale, e confermare che il risultato è
convergente.

Questo tutorial presuppone un modello di volume meshato - il modello fornito
**Channel.tmsh** è esattamente questo caso, e lo usa anche il tutorial
*Dispersione di contaminanti nei fluidi*. Se ne costruisci uno tuo, disegna un
parallelepipedo, tetraedrizzalo e marca la faccia di ingresso, la faccia di uscita
e le facce rimanenti come tre entità superficie separate, in modo da potervi
assegnare le condizioni.

### Passo 1 - selezionare il problema

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungi un **gruppo di task** e imposta il suo **N. di iterazioni** a `2000`
   con un doppio clic sul valore.
3. Lascia la sua **Convergenza** al valore predefinito `1e-5`.
4. Aggiungi un task **Incompressible viscous flow** all'interno del gruppo.
5. Conferma con `OK`.

I due valori sono il senso di questo passo. L'esecuzione richiede fino a 2000
passaggi e si ferma non appena il campo si stabilizza sotto `1e-5` - vedi 1.6.
Imposta il conteggio con generosità: è un tetto, e normalmente è il valore di
convergenza a terminare l'esecuzione.

### Passo 2 - generare la mesh

`Geometria` -> `Volume` -> `Genera mesh tetraedrica`.

Un modello di flusso si risolve solo su elementi di volume. Senza una mesh di
volume il solutore non ha nulla da assemblare.

### Passo 3 - assegnare il materiale

1. Seleziona l'entità volume nell'albero `Modello`.
2. Apri la scheda `Materiale` e assegna **Water**, oppure inserisci:
   - **Density** = `1000` `kg/m^3`
   - **Dynamic viscosity** = `1.0e-3` `kg/(m*s)`

Verifica che la viscosità sia quella dinamica in `Pa*s` e non quella cinematica -
vedi 2.3.

### Passo 4 - mantenere l'analisi stazionaria

Apri la scheda `Problema` e lascia **Abilita** disattivato nel riquadro
*Solutore temporale*.

### Passo 5 - assegnare le pareti

1. Seleziona l'entità superficie che copre i contorni solidi del canale.
2. Apri la scheda `Condizioni al contorno` e attiva **Wall**.

Non c'è alcun valore da inserire. È la condizione di aderenza, ed è ciò che fa
sviluppare al flusso un profilo invece di scorrere come un pistone.

### Passo 6 - generare il flusso

1. Seleziona l'entità superficie di ingresso.
2. Attiva **Volumetric flow rate (inflow)** e inserisci la portata in `m^3/s` -
   oppure attiva invece **Velocity (inflow)** e inserisci una velocità in `m/s`.

Prima di proseguire, stima il numero di Reynolds dalla velocità media e dalla
larghezza del canale:

```
Re = rho * V * L / mu
```

Sotto qualche migliaio il modello è nell'intervallo gestito da questo solutore.
Molto al di sopra, aspettati che l'esecuzione non converga, e leggi la sezione 5
prima di dedicarvi tempo.

### Passo 7 - aprire l'uscita

1. Seleziona l'entità superficie di uscita.
2. Attiva **Pressure (implicit)** e imposta **Pressure** = `0` `Pa`.

Questo è il passo che rende il modello risolvibile. Dà al campo di pressione il
suo riferimento e lascia sviluppare il profilo di uscita anziché imporlo piatto.
Un modello con un ingresso e pareti ma senza alcuna condizione di pressione ha una
pressione singolare e non converge.

### Passo 8 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Il controllo della configurazione segnala materiali o condizioni al contorno
   mancanti prima dell'avvio dell'esecuzione.
3. Segui l'avanzamento in `Report` -> `File di log del solutore`.

Può richiedere un po' di tempo - un modello di flusso di qualsiasi dimensione è un
lavoro vero, e il log stampa un blocco per iterazione. Cerca `Convergence target`
nel log: il passaggio che lo raggiunge viene contrassegnato, e il gruppo riporta
allora *All sub-tasks have converged* e si ferma.

### Passo 9 - verificare la convergenza

`Report` -> `Convergenza del solutore`.

Fallo prima di guardare qualsiasi immagine. Il residuo dovrebbe scendere di
diversi ordini di grandezza e poi appiattirsi, e `Convergence-V` e
`Convergence-P` dovrebbero avvicinarsi a zero. Se all'ultima iterazione il residuo
sta ancora scendendo ripidamente, l'esecuzione era troppo breve - aumenta il
numero di iterazioni e riavvia. Se oscilla o sale, la risposta non è una soluzione
e la sezione 5 è il posto dove cercare.

### Passo 10 - esaminare i risultati

- Scheda `Risultati`: visualizza **Velocity**. Il profilo attraverso il canale
  dovrebbe essere regolare, nullo alle pareti e più veloce vicino al centro.
- Taglia un piano attraverso il volume - la struttura interessante di un flusso è
  all'interno del dominio, non sulla sua superficie.
- Visualizza **Pressure**. Dovrebbe scendere in modo monotono dall'ingresso
  all'uscita e indicare `0` all'uscita, dove l'hai impostata.
- Crea **linee di flusso** con `Geometria` -> `Linea di flusso` per vedere subito
  ricircoli o zone morte.
- Verifica il bilancio di massa: la velocità media di uscita per l'area di uscita
  dovrebbe essere uguale alla portata volumetrica prescritta all'ingresso. È la
  verifica globale più rapida che il modello stia facendo ciò che hai chiesto -
  vedi la sezione 4.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| Il residuo non scende mai | nessuna condizione di pressione, quindi la pressione è indeterminata |
| Il residuo oscilla o sale | il numero di Reynolds è troppo alto per la mesh, oppure la viscosità è stata inserita come cinematica |
| Il flusso scorre senza profilo | nessuna condizione *Wall* sui contorni solidi |
| Il risultato sembra la prima iterazione | il task non è stato inserito in un gruppo di task, quindi è stato eseguito una volta |
| L'esecuzione si è fermata molto prima del numero di iterazioni | è convergente - sia gli incrementi sia il residuo hanno superato il test di 1.6 |
| L'esecuzione non si ferma mai in anticipo, per quanto largo sia il valore di convergenza | il residuo non sta scendendo - leggi `Residual ratio` nel log e considera il campo di flusso non convergente |
| *Failed to calculate element scales. Unsupported element type* | la mesh contiene elementi di volume diversi da tetraedri o esaedri lineari |
| Non è stato risolto nulla | non è stata generata alcuna mesh di volume, oppure al materiale mancano densità o viscosità |
| Le portate prescritte in ingresso e in uscita non si bilanciano | un fluido incomprimibile non può accumularsi - usa *Pressure (implicit)* all'uscita |

---

## 4. Verifica di un modello

Il solutore non ha una suite di verifica integrata, quindi conviene validare un
nuovo modello rispetto a qualcosa che puoi calcolare a mano prima di fidartene.
Quattro verifiche economiche, in ordine di utilità:

**Bilancio di massa.** Per qualsiasi modello incomprimibile convergente, ciò che
entra deve uscire:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(`over the inlets` = sugli ingressi, `over the outlets` = sulle uscite.) Leggi la
velocità media all'uscita dalle statistiche nel log del solutore, moltiplicala per
l'area di uscita e confrontala con l'ingresso prescritto. Questa sola verifica
globale rileva un'esecuzione non convergente, una parete mancante e una condizione
al contorno sull'entità sbagliata, e non costa nulla.

**Idrostatica.** Disattiva ogni ingresso, lascia il dominio chiuso da pareti,
assegna una condizione ambientale *Gravitational acceleration* e risolvi. Il
fluido deve fermarsi e la pressione deve variare linearmente con la profondità:

```
p(h) = p_ref + rho * g * h
```

Qualsiasi moto residuo è numerico. Ciò valida in una sola esecuzione la forza di
volume della gravità, il riferimento di pressione e le unità del materiale, ed è
il primo modello più rapido da costruire in una nuova configurazione.

**Flusso di Poiseuille piano.** Per un flusso laminare stazionario tra due lastre
parallele a distanza `H`, il profilo è parabolico, la velocità sulla linea
centrale è `1.5` volte quella media e il gradiente di pressione è

```
dp/dx = 12 * mu * V_mean / H^2
```

Modella un canale rettilineo con pareti sopra e sotto, un ingresso prescritto e
un'uscita *Pressure (implicit)*, e confronta la velocità sulla linea centrale e la
perdita di carico calcolate con questi valori. È la verifica più precisa
disponibile: valida contemporaneamente il termine viscoso, la parete con aderenza
e le unità della viscosità. Dai al modello un tratto di ingresso sufficiente
perché il profilo si sviluppi prima di misurare, poiché la condizione di ingresso
impone un profilo piatto.

**Flusso di Hagen-Poiseuille.** La stessa verifica in un tubo circolare di
diametro `D` e lunghezza `L`, dove la velocità sulla linea centrale è `2` volte
quella media e

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

valida finché il flusso è laminare, cioè al di sotto di circa `Re = 2300`.

Per ognuna di queste verifiche, raffina la mesh almeno una volta e conferma che la
risposta converge anziché variare - e conferma che l'andamento della convergenza
si sia appiattito prima di confrontare qualsiasi cosa. Un modello di flusso non
convergente fallirà ogni verifica qui per lo stesso motivo.

---

## 5. Limitazioni

### Limitazioni di modellazione

- **Nessun modello di turbolenza.** Il solutore integra le equazioni di
  Navier-Stokes laminari. I termini SUPG, PSPG e LSIC sono stabilizzazione
  numerica, non una chiusura della turbolenza - non rappresentano il mescolamento
  né la dissipazione aggiuntiva di un flusso turbolento. Oltre circa
  `Re = 2300` in un tubo il flusso reale è turbolento e questo solutore non lo
  modella: l'esecuzione può comunque convergere, ma la risposta descrive un flusso
  laminare che non esiste. Non c'è un modello `k`-`e`, né funzioni di parete, né
  una formulazione large-eddy.
- **Solo incomprimibile e newtoniano.** Il bilancio di massa è `div(v) = 0` e lo
  sforzo viscoso è lineare nella velocità di deformazione. Densità e viscosità
  possono dipendere dalla temperatura, ma non dalla pressione. Flusso
  comprimibile, effetti del numero di Mach, superfici libere, fasi multiple e
  comportamento non newtoniano sono fuori dalla formulazione.
- **Galleggiamento senza dilatazione del fluido.** Una densità dipendente dalla
  temperatura genera la convezione naturale tramite `rho(T)*g` (vedi 1.9), ma la
  dilatazione termica non entra mai nel bilancio di massa. Ciò è accurato per le
  piccole differenze di densità della convezione naturale, non per un gas
  riscaldato di centinaia di kelvin. Il flusso di galleggiamento è laminare come
  ogni altro flusso qui - un pennacchio o una cavità oltre un numero di Rayleigh
  di circa `1e8` - `1e9` è in realtà turbolento.
- **La temperatura raggiunge il flusso solo da un task di trasmissione del
  calore.** Senza di esso, le proprietà del materiale vengono valutate a
  `293.15 K`; la sola condizione ambientale *Temperature* non le modifica.
- **La dispersione di contaminanti è unidirezionale.** Il flusso trasporta il
  contaminante; la concentrazione non modifica mai il flusso. Le limitazioni dei
  tipi di problema associati sono elencate nei rispettivi manuali.
- **Solo elementi di volume, solo tetraedri ed esaedri lineari.** Un modello di
  flusso deve essere un volume meshato di elementi `TETRA1` o `HEXA1`; qualsiasi
  altra cosa interrompe l'esecuzione. Le superfici partecipano solo per portare
  una trazione *Pressure (implicit)*, e le entità punto e linea non contribuiscono.
- **La parete senza attrito è allineata agli assi.** Vincola la sola componente
  globale dominante della normale di ogni elemento, quindi è esatta solo su una
  superficie la cui normale giace lungo `x`, `y` o `z`.
- **Gli ingressi sono uniformi.** *Velocity (inflow)* e *Volumetric flow rate
  (inflow)* impongono entrambe un profilo piatto lungo la normale media della
  superficie. Un profilo di ingresso sviluppato deve essere prodotto da un tratto
  di ingresso nella mesh.

### Limitazioni del solutore

- **L'iterazione è uno schema di Newton modificato**, quindi la convergenza vicino
  alla soluzione è lineare anziché quadratica. Aspettati centinaia di iterazioni
  per un'esecuzione stazionaria, e imposta il numero di iterazioni del gruppo di
  task abbastanza alto da far sì che sia il valore di convergenza, e non il
  conteggio, a terminare l'esecuzione. Lo smorzamento di 1.6 mantiene
  un'esecuzione del genere in discesa, ma non può renderla quadratica.
- **L'avanzamento alle differenze centrali del solutore del flusso non è
  Crank-Nicolson.** La ponderazione theta viene applicata alla matrice ma non al
  residuo, quindi *Central difference (accurate)* risolve un residuo di Eulero
  all'indietro con una matrice a mezza rigidezza. La risposta è quella delle
  differenze all'indietro, raggiunta più lentamente. Usa *Backward difference
  (stable)* per il flusso finché ciò non verrà corretto. Il solutore di
  dispersione di contaminanti non ne è interessato - è lineare e il suo
  avanzamento è un vero Crank-Nicolson.
- **I parametri di stabilizzazione non vengono differenziati.** `Tsupg`, `Tpspg`,
  `Tlsic` e la lunghezza dell'elemento dipendono tutti dalla velocità, e tutti
  entrano nella matrice come costanti valutate al campo corrente. È la pratica
  abituale per una formulazione stabilizzata ed è uno dei motivi per cui lo
  jacobiano non è quello esatto.
- **La parte del test di convergenza relativa al residuo è fissa.** Una soluzione
  conta come convergente solo quando il suo residuo è sceso a un decimo di quello
  che era all'inizio di quella soluzione, e quel decimo non può essere modificato.
  È volutamente prudente: un modello che converge lentamente potrebbe non
  soddisfarlo mai e allora eseguirà l'intero numero di iterazioni. Il
  `Residual ratio` nel log indica quanto vi si è avvicinata un'esecuzione del
  genere.
- **È richiesto un riferimento di pressione.** Senza una superficie *Pressure
  (explicit)* o *Pressure (implicit)* la pressione è determinata solo a meno di
  una costante e l'iterazione non si stabilizza.
- **I flussi prescritti devono bilanciarsi.** Un modello i cui contorni sono tutti
  pareti e velocità prescritte deve avere l'ingresso esattamente uguale
  all'uscita; un fluido incomprimibile non può assorbire la differenza.
- **Costo.** Quattro incognite per nodo, una matrice non simmetrica e
  un'iterazione non lineare attorno a ogni soluzione lineare rendono un modello di
  flusso di gran lunga il tipo di problema più costoso in Range FEA. Dimensiona la
  mesh di conseguenza, e fai convergere un campo stazionario prima di avviare
  qualsiasi esecuzione transitoria.
