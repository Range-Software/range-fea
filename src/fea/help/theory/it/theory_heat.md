# Trasmissione del calore - Manuale teorico e guida utente

Questo documento descrive il solutore termico (`RSolverHeat`) di Range FEA, che
gestisce il tipo di problema **Trasmissione del calore** (*Heat transfer*): le
equazioni che risolve, il significato di ogni input che accetta, le parti
dell'interfaccia grafica che lo controllano e due tutorial svolti.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - trasmissione del calore stazionaria](#3-tutorial---trasmissione-del-calore-stazionaria)
4. [Tutorial - trasmissione del calore transitoria](#4-tutorial---trasmissione-del-calore-transitoria)
5. [Verifica di un modello](#5-verifica-di-un-modello)
6. [Limitazioni](#6-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - il campo di temperatura

L'incognita principale è la **temperatura** `T` in ogni nodo, in kelvin. Ogni
nodo ha un solo grado di libertà, quindi una mesh con `N` nodi dà un sistema di
`N` equazioni. Il flusso termico viene ricavato dal campo di temperatura dopo la
soluzione.

La temperatura è uno scalare, il che rende un modello termico decisamente più
economico di un modello strutturale sulla stessa mesh: un'incognita per nodo
anziché tre, e una matrice con un nono dei termini.

La temperatura predefinita di un nodo non toccato da alcuna condizione né da un
risultato precedente è `293.15 K`.

### 1.2 Equazione di governo

Il solutore implementa la **conduzione termica lineare** con contorni
convettivi. Il bilancio energetico di un corpo con conducibilità `k`, densità
`rho` e capacità termica specifica `c` è

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

e con il solutore temporale disattivato il termine transitorio scompare,
lasciando il bilancio stazionario

```
div( k * grad(T) ) + q = 0
```

Il calore lascia una superficie per convezione secondo la legge di
raffreddamento di Newton,

```
qs = h * ( T - Tf )
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `T` | temperatura | `K` | incognita |
| `q` | densità della sorgente di calore | `W/m^3` | condizione Heat, calore Joule, irraggiamento |
| `k` | conducibilità termica | `W/(m*K)` | proprietà del materiale |
| `c` | capacità termica | `J/(kg*K)` | proprietà del materiale |
| `rho` | densità | `kg/m^3` | proprietà del materiale |
| `h` | coefficiente di convezione | `W/(m^2*K)` | condizione di convezione, vedi 1.5 |
| `Tf` | temperatura del fluido | `K` | condizione di convezione |
| `Q` | flusso termico | `W/m^2` | ricavato, vedi 1.9 |

Non c'è termine di avvezione. Un problema di trasmissione del calore trasporta
energia per conduzione all'interno del corpo e per convezione attraverso il suo
contorno; non trasporta energia con un mezzo in movimento. Per questo usa *Heat
transfer in fluids*.

**Proprietà del materiale richieste.** Un problema termico ne richiede
**quattro**:

| Proprietà | Unità | Usata per |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | la matrice di conduzione |
| Heat capacity | `J/(kg*K)` | la matrice di capacità, solo transitorio |
| Density | `kg/m^3` | la matrice di capacità, solo transitorio |
| Emissivity | `-` | solo trasmissione del calore per irraggiamento |

Un'entità il cui materiale manca di una delle quattro **non viene risolta**,
anche per un'esecuzione di pura conduzione stazionaria che non considera mai la
densità né l'emissività. Il controllo della configurazione lo segnala prima
dell'avvio dell'esecuzione. L'unica eccezione è un'entità che porta una
condizione al contorno termica: quell'entità viene inclusa nel sistema qualunque
sia il suo materiale, e la sua conducibilità e capacità sono allora
semplicemente nulle.

Ogni proprietà è una tabella in funzione della temperatura. Il valore usato per
un elemento viene letto dalla tabella alla **temperatura dell'elemento
dell'esecuzione precedente**, quindi una conducibilità dipendente dalla
temperatura è in ritardo di un'iterazione. La sezione 1.6 spiega come farla
convergere.

### 1.3 Discretizzazione agli elementi finiti

La forma debole del bilancio energetico dà, per ogni elemento,

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(`conduction` = conduzione, `convection` = convezione, `capacity matrix` =
matrice di capacità, `sources` = sorgenti), dove `B` è costruita dalle derivate
delle funzioni di forma e `N` sono le funzioni di forma stesse. Ogni matrice di
elemento è `n x n` per un elemento con `n` nodi - una riga per nodo, non tre.

I termini di convezione vengono assemblati **solo sugli elementi di superficie**,
e solo dove è assegnata una condizione al contorno di convezione. La rigidezza di
convezione è la forma consistente `INT( N^T * h * N )`, mentre il carico di
convezione viene concentrato in parti uguali sui nodi dell'elemento. Le due sono
sufficientemente coerenti su una mesh ragionevole, e la differenza scompare al
raffinarsi della mesh.

**Misure degli elementi.** Come altrove in Range FEA, alle entità di dimensione
inferiore si assegna una misura geometrica affinché possano rappresentare corpi
reali:

| Entità | Misura | Dove si imposta |
|---|---|---|
| Volume | 1 | - |
| Superficie | spessore della superficie | proprietà geometriche dell'entità |
| Linea | area della sezione della linea | proprietà geometriche dell'entità |
| Punto | volume del punto | proprietà geometriche dell'entità |

Una **superficie** conduce il calore nel proprio piano, attraverso una sezione
pari al suo spessore, e immagazzina energia in proporzione a tale spessore. Non
c'è gradiente di temperatura attraverso lo spessore: un elemento di superficie è
spesso un nodo, quindi per costruzione è isotermo attraverso il suo spessore.
Una superficie di **spessore nullo** quindi non conduce e non immagazzina nulla,
ma porta comunque la convezione - che è esattamente ciò che serve per la faccia
di contorno di un volume meshato, dove la faccia esiste solo per portare la
condizione di convezione.

Una **linea** conduce lungo la propria direzione attraverso la sua area della
sezione. Un **punto** non ha una direzione lungo cui condurre, quindi non
contribuisce affatto alla conduzione: porta solo una capacità, proporzionale al
suo volume del punto, e una sorgente di calore. Un punto è una massa termica
concentrata, non un conduttore.

La matrice di capacità viene assemblata solo quando serve, cioè per un'analisi
transitoria.

### 1.4 Sorgenti e condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Temperature | esplicita (Dirichlet) | punto, linea, superficie, volume | Temperature `[K]` |
| Heat | naturale, sorgente | punto, linea, superficie, volume | Heat `[W]`, vedi sotto |
| Simple convection | naturale (Robin) | superficie | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | naturale, correlazione | superficie | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | naturale, correlazione | superficie | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | naturale, sorgente | superficie | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | naturale, sorgente | volume | Heat per unit volume `[W/m^3]` |

**Temperature** è l'unica condizione esplicita del problema termico. Ogni nodo
dell'entità a cui è assegnata viene rimosso dal sistema di equazioni e mantenuto
al valore dato; il suo contributo viene portato nel termine noto delle equazioni
adiacenti. Un modello stazionario ne richiede almeno una, oppure almeno una
superficie di convezione - vedi 1.7.

Dove le entità si sovrappongono, **prevale l'entità di dimensione inferiore**:
una temperatura prescritta su una superficie prevale, nei nodi condivisi, su una
prescritta sul volume retrostante, una linea prevale su una superficie e un punto
prevale su tutto. A differenza dei vincoli di spostamento del solutore
strutturale, le temperature non vengono combinate - c'è un solo grado di libertà
per nodo da mantenere, quindi prevale semplicemente l'ultima condizione letta.

**Heat** è una sorgente di calore e il suo valore è la **potenza totale in `W`**
immessa nell'entità a cui è assegnata, qualunque sia la dimensione di tale
entità. Il solutore ha bisogno di una densità di sorgente, quindi il totale viene
distribuito sulla misura dell'entità prima dell'assemblaggio:

| Entità | Densità applicata dal solutore |
|---|---|
| Volume | valore / volume dell'entità, `W/m^3` |
| Superficie | valore / area dell'entità, `W/m^2` |
| Linea | valore / lunghezza dell'entità, `W/m` |
| Punto | valore / numero di elementi punto, `W` ciascuno |

La potenza immessa è quindi esattamente il numero inserito - non cambia con la
dimensione dell'entità, con il raffinamento della mesh o con la scala del
modello. Solo gli elementi che il solutore può calcolare ricevono una quota,
quindi un'entità priva di proprietà del materiale viene saltata e la sua
condizione viene segnalata. Una sorgente di calore superficiale non viene scalata
per lo spessore della superficie.

**Heat rate (unit area)** e **Heat rate (unit volume)** prescrivono lo stesso
termine sorgente come densità anziché come totale: `W/m^2` sull'area di
un'entità superficie e `W/m^3` sul volume di un'entità volume. Usale quando la
sorgente è per sua natura una densità - un riscaldatore con potenza per metro
quadrato - e usa *Heat* quando è per sua natura un totale, come un elemento di
potenza nota. Tutte si sommano su un'entità che ne porta più d'una, e tutte
accettano una tabella in funzione del tempo, quindi un riscaldatore può essere
acceso e spento durante un'esecuzione transitoria.

Le condizioni di **convezione** si applicano alle superfici e aggiungono sia un
termine di rigidezza sia un carico, quindi agiscono come condizione al contorno
di Robin anziché come flusso fisso: aumentando la temperatura della parete
aumenta il calore che la parete cede. Tutte e tre le varianti si riducono alla
stessa coppia `(h, Tf)`; differiscono solo nella provenienza di `h`:

- **Simple convection** prende `h` e `Tf` direttamente dai due valori inseriti.
  Usala ogni volta che il coefficiente di convezione è noto da un manuale, da una
  misura o da un calcolo precedente. È l'unica delle tre interamente sotto il tuo
  controllo.
- **Forced convection** ha due modalità. Su una parete tra un solido meshato e un
  fluido meshato, non appena un task *Heat transfer in fluids* ha prodotto un
  risultato, accoppia direttamente le due soluzioni - non c'è alcuna
  correlazione, poiché il flusso risolto contiene già la convezione. Il solutore
  termico dei fluidi consegna, per ogni elemento della parete, la coppia
  `(h, Tf)` che riproduce il flusso termico che la sua soluzione fa passare
  attraverso la parete; vedi *Scambio termico coniugato* in 1.6. Ovunque altrove -
  una superficie che non confina con alcun fluido meshato, o il primo passaggio
  di un'esecuzione accoppiata prima che il task termico dei fluidi sia stato
  eseguito - `h` viene calcolato da una correlazione per il flusso esterno lungo
  una lastra piana, usando le proprietà del fluido, il diametro idraulico, la
  *Fluid temperature* e la *Velocity* inserite. Il log indica quale delle due
  modalità è in uso.
- **Natural convection** calcola `h` da una correlazione per lastre orizzontali,
  usando le proprietà del fluido, il diametro idraulico e la differenza di
  temperatura tra parete e fluido.

Se alla stessa superficie sono assegnate più condizioni di convezione, non si
sommano: simple viene letta per prima, forced la sovrascrive e natural
sovrascrive entrambe.

### 1.5 Le correlazioni di convezione

Entrambe le forme calcolate costruiscono i consueti gruppi adimensionali dalle
proprietà del fluido inserite con la condizione al contorno, con il diametro
idraulico `d` come lunghezza caratteristica:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

e convertono il numero di Nusselt in un coefficiente di convezione come

```
h = Nu * k / d
```

| Forma | Correlazione | Numero di Nusselt |
|---|---|---|
| Forzata | flusso esterno, lastra piana | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Naturale | lastra orizzontale, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Naturale | lastra orizzontale, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

`k`, `c`, `rho`, `mu` e `beta` in queste espressioni sono le proprietà del
**fluido** inserite con la condizione al contorno, non le proprietà del materiale
del solido.

Entrambe le condizioni vengono create con le proprietà dell'**aria secca a 20
gradi Celsius e 101.325 kPa**, quindi il fluido è già descritto e restano da
inserire solo il flusso e la geometria:

| Componente | Predefinito |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, il valore del gas ideale `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Sostituiscili per qualsiasi altro fluido - sono le proprietà del fluido, non il
materiale della parete. *Natural convection* è quindi pronta per la soluzione così
come assegnata; *Forced convection* richiede ancora una **velocità**, che nessun
valore predefinito può indovinare - a meno che la superficie non confini con un
fluido meshato, nel qual caso nessuno di questi valori viene letto.

Ognuno di questi gruppi divide per `mu`, `k` o `d`, e `Re` e `Pr` si annullano se
lo fanno `rho`, `c` o `v`, il che lascerebbe `h = 0` e una superficie che non si
raffredda mai. Un valore nullo viene quindi trattato come un errore anziché
mascherato: il solutore si arresta e indica la componente e l'entità, ad esempio

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

La velocità calcolata dal solutore del flusso non viene mai usata nella
correlazione. Una parete con aderenza mantiene il fluido fermo, quindi la
velocità accanto a essa è nulla, e la correlazione richiederebbe la velocità
della corrente libera e la temperatura di massa, che un flusso tridimensionale
generico non definisce. Una parete che confina con un fluido meshato viene invece
accoppiata alla soluzione del fluido.

Vale la pena tenere a mente tre conseguenze:

- La correlazione è **fissa**. Il solutore non considera l'orientamento della
  superficie, se il flusso è interno o esterno, né se lo strato limite è laminare
  o turbolento. Tutto entra tramite il diametro idraulico, che è la lunghezza
  caratteristica della correlazione - è l'unico controllo che hai sulla
  geometria.
- Un diametro idraulico nullo darebbe `h = 0`, cioè una parete isolata, quindi il
  solutore si arresta anziché accettarlo.
- La convezione naturale dipende dalla **temperatura della parete**, presa dalla
  temperatura dell'elemento della soluzione **precedente**. Alla prima esecuzione
  di un'analisi stazionaria quella è la temperatura iniziale, quindi il
  coefficiente viene valutato alla differenza di temperatura sbagliata. Inserisci
  il task in un gruppo di task con diverse iterazioni per farlo convergere,
  oppure usa *Simple convection* con un coefficiente calcolato da te. Una
  differenza di temperatura nulla dà `Nu = 1`.

### 1.6 Condizioni ambientali, iniziali e di accoppiamento

#### Condizioni ambientali

| Condizione ambientale | Componenti | Effetto |
|---|---|---|
| Temperature | Temperature `[K]` | temperatura di fondo dell'entità, solo prima esecuzione |

#### Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Temperature | campo di temperatura iniziale, applicato alla prima esecuzione |

Condizioni ambientali, condizioni iniziali e condizioni al contorno vengono lette
in quest'ordine e ciascuna sovrascrive la precedente, quindi **una condizione al
contorno prevale su una condizione iniziale, che prevale su una condizione
ambientale**. I valori ambientali e iniziali vengono letti **solo alla prima
esecuzione**; in seguito il campo avanza a partire dal risultato precedente.

#### Accoppiamento con altri solutori

Il solutore termico utilizza e produce risultati condivisi con il resto del
flusso dei task.

| Direzione | Variabile | Effetto |
|---|---|---|
| Irraggiamento -> termico | Heat - radiation part | aggiunto al termine sorgente degli elementi di **superficie** |
| Elettrostatica -> termico | Joule heat | aggiunto al termine sorgente di ogni elemento |
| Tensioni -> termico | Displacement | la mesh viene deformata per la soluzione termica e ripristinata dopo |
| Termico -> tensioni | Temperature | genera il termine di dilatazione termica del solutore strutturale |
| Termico -> qualsiasi solutore | Temperature | seleziona la riga di ogni tabella del materiale dipendente dalla temperatura |
| Termico dei fluidi -> termico | parete `(h, Tf)` | genera le pareti con *Forced convection* che confinano con un fluido meshato |
| Termico -> termico dei fluidi | temperatura del solido | mantiene le stesse pareti alla temperatura del solido nella soluzione del fluido |

La **trasmissione del calore per irraggiamento** è un tipo di problema separato
con un proprio solutore. Richiede un task *Heat transfer*, risolve un sistema di
radiosità di una cavità su patch di superficie usando i fattori di vista e
l'emissività del materiale, e restituisce il risultato come sorgente di calore
superficiale. Aggiungi entrambi i task a un unico gruppo di task e lascia
iterare il gruppo: il solutore di irraggiamento riporta la convergenza e termina
il gruppo quando il calore delle sue patch smette di variare.

Il **calore Joule** arriva allo stesso modo da un task *Electro-statics* posto
prima del task termico, quindi il riscaldamento resistivo non richiede
ulteriore configurazione.

A meno che non sia accoppiato a un fluido, il solutore termico si dichiara sempre
convergente, quindi un gruppo di task che contiene solo un task termico si ferma
sempre dopo un'iterazione. Iterare conviene quando qualcosa da cui dipende la
soluzione termica è a sua volta in ritardo - calore radiativo, un coefficiente di
convezione naturale o una conducibilità fortemente dipendente dalla temperatura.

#### Scambio termico coniugato

Il solutore termico risolve **solo i solidi**. Un volume il cui materiale è un
fluido - un gas o un liquido, o un materiale di stato non specificato che ha una
viscosità dinamica, il che copre ogni fluido del database dei materiali - non fa
mai parte della soluzione termica, qualunque proprietà o condizione porti.
Altrettanto vale per un'entità punto, linea o superficie che si trova
all'interno del fluido, come un ingresso; una condizione su di essa che legge
solo il solutore termico viene segnalata nel log. Il dominio fluido appartiene al
tipo di problema *Heat transfer in fluids*, che trasporta il calore con il flusso.

I due si incontrano sulle pareti che portano la condizione **Forced convection**.
Con entrambi i task in un unico gruppo di task - prima il task termico dei
fluidi - ogni passaggio del gruppo fa quanto segue:

1. Il solutore termico dei fluidi mantiene i nodi della parete alla temperatura
   che il solutore termico ha calcolato per ultimo nel solido (al primissimo
   passaggio, quando tale temperatura non esiste ancora, la parete è isolata),
   risolve il fluido e ricava il flusso termico che entra nel fluido in ogni nodo
   della parete. Quel flusso è il residuo delle equazioni del fluido nel nodo,
   quello che vedrebbe un'unica soluzione di entrambi i domini, quindi uno strato
   limite termico sottile non costa accuratezza.
2. Per ogni elemento della parete consegna la coppia `(h, Tf)`. `h = k * G` è la
   conduttanza del primo elemento fluido dietro la parete - `k` diviso l'altezza
   dell'elemento sopra la parete, per un tetraedro - e `Tf` viene impostato in
   modo che `q = h * (Tw - Tf)` sia il flusso del passo 1.
3. Il solutore termico li applica alla parete come farebbe con una condizione
   *Simple convection*, risolve il solido e restituisce la temperatura della sua
   parete.

La coppia `(h, Tf)` è un espediente numerico che fa concordare le due soluzioni -
`h` non è il coefficiente di scambio termico di un manuale, e cambia con la mesh.
Ciò che converge sono la temperatura e il flusso attraverso la parete, che sono
quelli del problema accoppiato. Non serve nulla sulla velocità del fluido accanto
alla parete, ed è per questo che una parete con aderenza - dove il flusso è fermo -
trasmette il calore proprio come dovrebbe.

La temperatura della parete passata al fluido viene rilassata con un **fattore di
Aitken** calcolato dagli ultimi due passaggi. Senza di esso, un solido poco
conduttivo accanto a un fluido ben risolto può richiedere centinaia di passaggi;
con esso il gruppo si stabilizza tipicamente in pochi passaggi. Mentre sono
accoppiati, entrambi i solutori riportano come convergenza la variazione relativa
del proprio campo di temperatura, quindi imposta il valore di convergenza del
gruppo di task - `1e-6` è un punto di partenza ragionevole - e assegnagli
iterazioni sufficienti. Un valore di convergenza nullo le esegue tutte.

### 1.7 Analisi stazionaria

Con il solutore temporale disattivato il solutore assembla e risolve

```
K * T = f
```

`K` è simmetrica definita positiva non appena è stata prescritta almeno una
temperatura, quindi viene risolta con il metodo del **gradiente coniugato** con
precondizionamento di Jacobi.

Un modello stazionario **senza condizione al contorno Temperature e senza
superficie di convezione** è l'equivalente termico di una struttura non
vincolata: l'energia può entrare ma non ha dove uscire, la matrice è singolare e
la soluzione non converge. Ogni modello stazionario richiede almeno un percorso
verso una temperatura nota.

### 1.8 Analisi transitoria

Con il solutore temporale attivo, il solutore avanza

```
M * dT/dt + K * T = f
```

con uno schema theta, dove `alpha` è il coefficiente dell'approssimazione
dell'avanzamento nel tempo:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

L'impostazione *Approssimazione* del solutore temporale sceglie `alpha`:

| Approssimazione | `alpha` | Avanzamento risultante | Stabilità |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, un passo implicito | incondizionata |
| Central difference (accurate) | `0.5` | Crank-Nicolson | incondizionata, può oscillare |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, un passo esplicito | condizionata |

**Backward difference** è la scelta predefinita sicura per un modello termico.
Costa una soluzione matriciale per passo, ma il passo è limitato dall'accuratezza
desiderata e non dalla dimensione dell'elemento più piccolo. **Central
difference** è accurato al secondo ordine ed è la scelta migliore quando il passo
temporale è abbastanza piccolo da risolvere il transitorio, anche se può
oscillare a un'accensione brusca. **Forward difference** mette l'intera matrice di
conduzione sul passo temporale precedente, il che rende ogni passo economico e
l'esecuzione fragile.

Il ramo in avanti è stabile solo finché il passo temporale resta al di sotto di
circa

```
dt < rho * c * h^2 / ( 2 * k )
```

dove `h` è la dimensione dell'elemento più piccolo. Poiché la diffusività termica
`k/(rho*c)` di un metallo è dell'ordine di `1e-5 m^2/s`, già un elemento di un
millimetro impone un passo temporale ben inferiore a un decimo di secondo. Per
tutto ciò che non sia una risoluzione temporale molto fine, usa l'avanzamento
implicito.

Un'esecuzione transitoria instabile è facile da riconoscere: il campo di
temperatura oscilla da un passo all'altro con ampiezza crescente, spesso tra
valori fisicamente impossibili, anziché stabilizzarsi. Il rimedio è un passo
temporale più piccolo o l'avanzamento implicito.

### 1.9 Risultati derivati

Dopo la soluzione il flusso termico di elemento viene ricavato dal campo di
temperatura come media sui punti di integrazione di

```
Q = -k * grad(T)
```

valutata nell'elemento e ruotata in coordinate globali per gli elementi linea e
superficie. Su una superficie che porta la convezione, il flusso convettivo

```
Qh = h * ( Tf - T )
```

viene aggiunto lungo la normale della superficie, così il vettore del flusso di
una faccia convettiva punta nella direzione in cui il calore esce effettivamente.

Il flusso è una **densità** in `W/m^2`, indipendente dalla dimensione
dell'elemento in cui è stato calcolato, quindi può essere confrontato tra
elementi e con un calcolo manuale, e converge al raffinarsi della mesh.

Viene memorizzato anche il **coefficiente di scambio termico**. È il coefficiente
effettivamente usato dal solutore su ogni elemento di superficie, l'unico modo
per vedere cosa hanno prodotto le correlazioni *Forced convection* e *Natural
convection* - e su una superficie con convezione naturale varia da elemento a
elemento, poiché dipende dalla temperatura locale della parete. Su una parete
accoppiata a un fluido è la conduttanza del primo elemento fluido, che dipende
dalla mesh - vedi *Scambio termico coniugato* in 1.6. È nullo su ogni elemento
che non porta una condizione di convezione.

| Risultato | Si applica a | Significato |
|---|---|---|
| Temperature `[K]` | nodo | il campo di temperatura risolto |
| Heat flux `[W/m^2]` | elemento | vettore del flusso conduttivo, più la parte convettiva sulle superfici convettive |
| Heat transfer coefficient `[W/(m^2*K)]` | elemento | il coefficiente di convezione usato, nullo dove non c'è convezione |

Dopo ogni record il log del solutore stampa le statistiche di tutti e tre e i
valori in tutti i punti di monitoraggio.

### 1.10 Stazionaria o transitoria?

| | Stazionaria | Transitoria |
|---|---|---|
| Risolve | `K*T = f` | `M*T' + K*T = f` nel tempo |
| Risponde a | «quanto si scalda alla fine» | «quanto tempo impiega per arrivarci» |
| Usa densità e capacità termica | no, ma devono comunque essere presenti | sì |
| Richiede una temperatura prescritta o una superficie convettiva | sì | no, basta una capacità |
| Record | uno | uno per ogni passo temporale scritto |

**Usa un'analisi stazionaria per** la distribuzione finale della temperatura di
un componente in funzionamento continuo, il dimensionamento di un dissipatore, un
ponte termico in una parete o il campo di temperatura che alimenta un'analisi
delle tensioni termiche. È la scelta predefinita e copre la maggior parte dei
casi.

**Usa un'analisi transitoria per** riscaldamento e raffreddamento, un ciclo di
lavoro, uno shock termico, la tempra, o qualsiasi caso in cui ciò che cerchi è la
costante di tempo del componente.

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Condizioni ambientali`,
`Materiale`, `Risultati`), nel menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Aggiungi un task e scegli **Heat transfer** -
"Steady-state and transient Heat-transfer" (trasmissione del calore stazionaria
e transitoria).

La trasmissione del calore si combina liberamente con ogni altra fisica tranne
*Modal analysis*. Le catene da conoscere sono:

- *Heat transfer* e poi *Stress analysis* - tensioni termiche;
- *Heat transfer* e *Radiative heat transfer* in un unico gruppo di task che
  itera - conduzione più irraggiamento in cavità;
- *Electro-statics* e poi *Heat transfer* - riscaldamento resistivo.

Per iterare una coppia accoppiata, aggiungi un gruppo di task e impostane il
numero di iterazioni; il gruppo si ferma in anticipo quando ogni task al suo
interno riporta la convergenza.

### 2.2 Scheda Problema

**Solutore temporale** - mostrato per la trasmissione del calore, poiché è un
tipo di problema dipendente dal tempo. Lascialo disattivato per un'analisi
stazionaria.

| Campo | Significato |
|---|---|
| Abilita | attiva e disattiva l'avanzamento transitorio |
| Approssimazione | theta dell'avanzamento nel tempo: all'indietro, centrale o in avanti, vedi 1.8 |
| Tempo iniziale | primo valore di tempo in secondi; ignorato al riavvio del solutore |
| Tempo finale | `start time + time-step size * number of time-steps`, sola lettura |
| Dimensione del passo temporale | `dt` in secondi |
| Numero di passi temporali | quanti passi calcolare |
| Frequenza di output | scrive un file di risultati ogni N passi; `0` scrive solo l'ultimo passo |

Un problema termico non ha un riquadro di impostazioni proprio. Tutto il resto si
assegna per entità nelle schede delle condizioni.

### 2.3 Scheda Materiale

La scheda `Materiale` elenca il materiale assegnato all'entità selezionata. La
trasmissione del calore richiede **tutte e quattro** queste proprietà:

| Proprietà | Unità | Commento |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, circa `50` per l'acciaio da costruzione, `15` per l'inox, `200` per l'alluminio |
| Heat capacity | `J/(kg*K)` | `c`, circa `460` per l'acciaio |
| Density | `kg/m^3` | `rho`, circa `7850` per l'acciaio |
| Emissivity | `-` | usata solo dalla trasmissione del calore per irraggiamento, ma comunque richiesta |

Un'entità che ne manca una non viene risolta, e il controllo della
configurazione avvisa prima dell'avvio dell'esecuzione. Se non esegui una
trasmissione del calore per irraggiamento, un'emissività di `0` basta a
soddisfare il requisito.

Il materiale deve inoltre essere un **solido**. Un volume il cui materiale è un
gas o un liquido - oppure non ha uno stato impostato e ha una viscosità dinamica -
viene lasciato al tipo di problema *Heat transfer in fluids* anche se ha tutte
e quattro le proprietà.

Ogni proprietà è una tabella in funzione della temperatura, quindi una
conducibilità dipendente dalla temperatura viene rispettata - valutata alla
temperatura dell'elemento dell'esecuzione precedente, il che rende utile iterare
il task quando la dipendenza è forte.

### 2.4 Scheda Condizioni al contorno

La scheda è suddivisa in un elenco gestore delle condizioni al contorno
disponibili in alto e in un editor per quella selezionata in basso. Seleziona
prima un'entità nell'albero `Modello`; il gestore offre allora le condizioni
valide per quel tipo di entità e per il problema selezionato. Attivane una per
assegnarla, poi modificane le componenti nell'albero inferiore.

Vengono offerte le condizioni della sezione 1.4. Note sul loro uso:

- **Temperature** è l'unica condizione che mantiene un valore anziché generarlo.
  Assegnala alla faccia, al bordo o al punto di cui conosci la temperatura.
- **Heat** è offerta per ogni tipo di entità e accetta la potenza totale in `W`;
  il solutore la distribuisce sull'entità per te. È la condizione a cui ricorrere
  quando conosci la potenza di ciò che genera il calore.
- **Heat rate (unit volume)** su un volume e **Heat rate (unit area)** su una
  superficie prescrivono invece la stessa sorgente come densità, per una sorgente
  la cui potenza è per sua natura riferita all'unità di volume o di area.
- Le tre condizioni di **convezione** si applicano solo alle superfici. Assegna
  *Simple convection* a meno che tu non abbia un motivo specifico per lasciare
  che il solutore correli il coefficiente per te; è quella prevedibile. *Forced
  convection* su una parete che confina con un fluido meshato accoppia il solido
  alla soluzione termica del fluido e non legge nessuna delle sue componenti;
  altrove correla il coefficiente a partire da esse.
- Ogni componente è una tabella in funzione del tempo, quindi a una temperatura
  prescritta o a una sorgente di calore si può dare un andamento nel tempo per
  un'esecuzione transitoria. È così che si modella un ciclo di lavoro o un
  riscaldatore con rampa.

### 2.5 Condizioni iniziali e ambientali

La scheda `Condizioni iniziali` offre *Temperature*, che imposta il campo di
partenza di un'esecuzione transitoria. Assegnala all'intero modello con la
temperatura da cui parte il componente.

Anche la scheda `Condizioni ambientali` offre *Temperature*, come valore di fondo
per un'entità. Entrambe vengono lette solo alla prima esecuzione, e una
condizione al contorno prevale su entrambe.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Il solutore termico usa la voce **CG**.

Se una soluzione stazionaria non converge, la causa abituale è un modello senza
temperatura prescritta e senza superficie convettiva, più che un'impostazione del
solutore. Controlla le condizioni al contorno prima di aumentare il numero di
iterazioni.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare - per un problema termico,
*Temperature*. In un'esecuzione transitoria l'andamento è mostrato da `Report` ->
`Punti di monitoraggio`, il modo più rapido per leggere da un modello una curva
di riscaldamento o una costante di tempo.

### 2.8 Risultati e record

La scheda `Risultati` elenca le variabili calcolate e controlla la vista 3D. La
temperatura è uno scalare nodale, il flusso termico un vettore di elemento e il
coefficiente di scambio termico uno scalare di elemento, nullo lontano dalle
superfici convettive.

La scheda `Record` del pannello `Modello` elenca i record dei risultati:

- **analisi stazionaria** - un unico record;
- **analisi transitoria** - un record per ogni passo temporale scritto. L'animazione
  dei record riproduce il riscaldamento.

`Report` -> `File di log del solutore` mostra l'output completo del solutore,
comprese le statistiche di temperatura, flusso termico e coefficiente di scambio
termico dopo ogni record.

---

## 3. Tutorial - trasmissione del calore stazionaria

**Obiettivo.** Calcolare la distribuzione di temperatura di una barra mantenuta
calda a un'estremità e raffreddata dall'aria circostante, e leggere la
temperatura all'estremità fredda.

Questo tutorial presuppone un modello di volume meshato - un parallelepipedo
allungato va bene. Usa `File` -> `Nuovo modello`, disegna un parallelepipedo con
`Geometria` -> `Crea elemento` o importa una geometria, e tetraedrizzala; il
tutorial *Disegnare un cubo* tratta questi passaggi. Marca la faccia terminale
calda e le facce rimanenti come entità superficie separate, in modo da potervi
assegnare le condizioni.

### Passo 1 - selezionare il problema

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungi un task e seleziona **Heat transfer**.
3. Conferma con `OK`.

### Passo 2 - assegnare il materiale

1. Seleziona l'entità volume nell'albero `Modello`.
2. Apri la scheda `Materiale` e assegna un materiale con tutte e quattro le
   proprietà, per l'acciaio da costruzione:
   - **Thermal conductivity** = `50` `W/(m*K)`
   - **Heat capacity** = `460` `J/(kg*K)`
   - **Density** = `7850` `kg/m^3`
   - **Emissivity** = `0`

La capacità termica e la densità non vengono usate da una soluzione stazionaria,
ma devono essere presenti perché l'entità venga risolta.

### Passo 3 - mantenere l'analisi stazionaria

Apri la scheda `Problema` e lascia **Abilita** disattivato nel riquadro
*Solutore temporale*. Il solutore assembla e risolve quindi `K*T = f` una volta.

### Passo 4 - prescrivere l'estremità calda

1. Seleziona l'entità superficie all'estremità calda.
2. Apri la scheda `Condizioni al contorno` e attiva **Temperature**.
3. Imposta il valore a `353.15` K, cioè 80 gradi Celsius.

Ricorda che tutte le temperature in Range FEA sono assolute, in kelvin.

### Passo 5 - raffreddare il resto

1. Seleziona l'entità superficie che copre le facce rimanenti.
2. Apri la scheda `Condizioni al contorno` e attiva **Simple convection**.
3. Imposta **Convection coefficient** = `10` `W/(m^2*K)` e **Fluid temperature**
   = `293.15` K.

Un coefficiente di circa `10` è rappresentativo di una superficie in aria calma;
l'aria forzata è più vicina a `50`, e l'acqua è nell'ordine delle centinaia o
migliaia.

Questo è il passo che rende il modello risolvibile. Senza di esso la barra ha un
apporto di calore e nessuna uscita, e uno stato stazionario non esiste.

### Passo 6 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Il controllo della configurazione segnala materiali o condizioni al contorno
   mancanti prima dell'avvio dell'esecuzione.
3. Segui l'avanzamento in `Report` -> `File di log del solutore`.

### Passo 7 - esaminare i risultati

- Scheda `Risultati`: visualizza **Temperature**. Il campo dovrebbe scendere in
  modo regolare dall'estremità calda prescritta verso la temperatura del fluido.
- Il punto più freddo dovrebbe restare sopra la temperatura del fluido. Una
  temperatura inferiore a `Tf` in qualsiasi punto, o superiore ai `353.15` K
  prescritti, significa che qualcosa sta immettendo energia nel verso sbagliato -
  controlla il segno di qualsiasi sorgente di calore.
- **Heat flux** mostra dove viaggia l'energia, in `W/m^2`. Moltiplica il valore
  sulle facce convettive per la loro area e confrontalo con la potenza che entra
  nella barra.
- **Heat transfer coefficient** dovrebbe indicare `10` sulle facce convettive e
  `0` ovunque altrove. È il modo più rapido per confermare che una condizione di
  convezione abbia raggiunto gli elementi previsti, e l'unico modo per vedere cosa
  ha calcolato una condizione correlata.
- Un punto di monitoraggio su *Temperature* all'estremità fredda dà la risposta
  come numero anziché come colore.

### Passo 8 - facoltativo, aggiungere una sorgente di calore

Per modellare un riscaldatore incorporato nella barra, seleziona l'entità volume,
apri la scheda `Condizioni al contorno` e attiva **Heat**. Per immettere nella
barra un totale di `P` watt, inserisci `P` - il solutore lo distribuisce sul
volume dell'entità.

Usa invece **Heat rate (unit volume)** quando la potenza della sorgente è
riferita all'unità di volume; inserisci `P / V` `W/m^3` per immettere lo stesso
totale - vedi 1.4.

La temperatura stazionaria deve allora salire finché la superficie convettiva non
riesce a smaltire esattamente quella potenza - che è la verifica del bilancio
energetico della sezione 5.

### Passo 9 - facoltativo, tensioni termiche

Per portare il risultato in un'analisi strutturale:

1. Aggiungi un task **Stress analysis** *dopo* il task termico nel flusso dei
   task del problema, e assegna all'entità le proprietà strutturali necessarie.
2. Assegna all'entità strutturale una condizione ambientale **Temperature** con
   la temperatura di riferimento priva di tensioni, ad esempio `293.15` K.

Il solutore delle tensioni prende le temperature degli elementi calcolate e
aggiunge automaticamente il termine di dilatazione termica.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La soluzione non converge | nessuna condizione Temperature e nessuna superficie convettiva, quindi lo stato stazionario non è definito |
| *Entity has material assigned which is missing required properties* | manca una delle quattro proprietà richieste - spesso l'emissività |
| L'intero modello è a un'unica temperatura | non entra calore, oppure nulla genera un gradiente rispetto al valore prescritto |
| Le temperature sono assurdamente alte | in una condizione *Heat*, che accetta il totale in watt, è stata inserita una densità in `W/m^3`, oppure il coefficiente di convezione è decisamente troppo piccolo |
| Una parete tra solido e fluido non si raffredda | il task termico dei fluidi viene eseguito dopo il task termico, oppure il gruppo esegue una sola iterazione - metti per primo il task termico dei fluidi e lascia iterare il gruppo |
| Il fluido viene ignorato dalla soluzione termica | voluto - un materiale fluido viene risolto solo da *Heat transfer in fluids* |
| Le temperature risultano vicine a `293.15` K ovunque | l'entità non è stata risolta - controlla nel log se manca una proprietà del materiale |

---

## 4. Tutorial - trasmissione del calore transitoria

**Obiettivo.** Avviare la stessa barra a temperatura ambiente, accendere
l'estremità calda e scoprire quanto tempo impiega a riscaldarsi.

### Passo 1 - selezionare il problema

Lo stesso task **Heat transfer** del tutorial 3. Se il modello è ancora
configurato, occorre aggiungere solo le impostazioni del solutore temporale e la
condizione iniziale.

### Passo 2 - assegnare il materiale

Le stesse quattro proprietà. Qui contano la **capacità termica** e la
**densità**: insieme alla conducibilità determinano la diffusività termica
`k/(rho*c)`, che governa la velocità con cui avanza il fronte.

### Passo 3 - impostare la temperatura iniziale

1. Seleziona l'entità volume.
2. Apri la scheda `Condizioni iniziali` e attiva **Temperature**.
3. Impostala a `293.15` K.

Senza di essa il modello parte comunque dai `293.15` K predefiniti, ma indicarla
esplicitamente rende il modello autodocumentato e permette di partire da
qualsiasi altro stato.

### Passo 4 - configurare il solutore temporale

Apri la scheda `Problema` e nel riquadro *Solutore temporale* imposta:

| Campo | Valore |
|---|---|
| Abilita | attivato |
| Approssimazione | `Backward difference (stable)` |
| Tempo iniziale | `0` |
| Dimensione del passo temporale | `1` |
| Numero di passi temporali | `300` |
| Frequenza di output | `10` |

*Backward difference* è l'avanzamento implicito - vedi la tabella in 1.8 - ed è
quello da usare qui. Resta stabile con un passo temporale scelto in base alla
fisica anziché alla mesh.

Scegli il passo temporale in base alla costante di tempo prevista, puntando a
circa cinquanta passi sulla parte interessante della curva. La sezione 5 dà una
formula per una prima stima. Scrivere un passo ogni dieci mantiene piccolo il
file dei risultati senza perdere la forma della curva.

### Passo 5 - mantenere le condizioni al contorno

La **Temperature** prescritta all'estremità calda e la **Simple convection** sulle
facce rimanenti restano invariate. Ogni componente è una tabella in funzione del
tempo, quindi all'estremità calda si può invece dare una rampa: apri l'editor dei
componenti e aggiungi punti temporali per far salire la temperatura nei primi
secondi anziché passare istantaneamente al valore finale.

### Passo 6 - aggiungere un punto di monitoraggio

`Problema` -> `Definisci punti di monitoraggio`, colloca un punto all'estremità
fredda e imposta la sua variabile su **Temperature**. La curva di riscaldamento si
legge molto più facilmente come andamento che come sequenza di immagini colorate.

### Passo 7 - risolvere

`Soluzione` -> `Avvia solutore` (`Ctrl+R`). Il log stampa un blocco per passo
temporale:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

seguito dalle iterazioni del solutore matriciale e dalle statistiche di
temperatura di quel passo.

### Passo 8 - esaminare i risultati

- Pannello `Modello`, scheda `Record`: un record per ogni passo scritto.
  Scorrili uno per uno o animali per vedere il fronte di calore avanzare nella
  barra.
- Scheda `Risultati`: visualizza **Temperature**. Fissa l'intervallo di
  visualizzazione su tutti i record in modo che i colori abbiano lo stesso
  significato in ogni fotogramma, altrimenti l'animazione si riscala da sola e
  l'avanzamento è invisibile.
- `Report` -> `Punti di monitoraggio` traccia la curva di riscaldamento
  all'estremità fredda. Leggi da essa la costante di tempo: il tempo necessario a
  coprire circa il 63 per cento dell'aumento totale.
- L'ultimo record dovrebbe avvicinarsi al risultato stazionario del tutorial 3.
  Se non lo fa, l'esecuzione era troppo breve.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La temperatura oscilla e cresce da un passo all'altro | l'avanzamento esplicito con un passo temporale troppo grande - passa all'approssimazione *Backward difference (stable)* o riduci `dt` |
| Il risultato cambia appena durante l'intera esecuzione | il passo temporale è molto inferiore alla costante di tempo, oppure l'esecuzione è troppo breve |
| Il primo passo salta alla risposta finale | il passo temporale è molto superiore alla costante di tempo, quindi l'avanzamento è di fatto stazionario |
| È stato scritto un solo record | la frequenza di output è `0`, che scrive solo l'ultimo passo |
| Il transitorio non raggiunge mai lo stato stazionario | spesso è corretto - confronta con l'esecuzione stazionaria prima di sospettare del solutore |

---

## 5. Verifica di un modello

Il solutore non ha una suite di verifica integrata, quindi conviene validare un
nuovo modello rispetto a qualcosa che puoi calcolare a mano prima di fidartene.
Quattro verifiche economiche, in ordine di utilità:

**Parete piana.** Per una lastra di spessore `L` e conducibilità `k` mantenuta a
`T1` e `T2` sulle sue due facce, la temperatura è lineare attraverso la lastra e
il flusso è

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Modella la lastra con elementi di volume, prescrivi entrambe le facce, conferma
che il piano medio si trovi esattamente a metà e che lo **Heat flux** calcolato
corrisponda a `q`. Ciò rileva unità sbagliate, una conducibilità sbagliata e una
mesh non collegata come pensi. Raffina la mesh una volta e conferma che entrambi i
valori restano invariati.

**Parete con convezione.** Per la stessa lastra mantenuta a `T1` su una faccia e
raffreddata sull'altra da un fluido a `Tf` con coefficiente `h`, la superficie
raffreddata si assesta a

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Confronta con la temperatura superficiale calcolata. Ciò valida la condizione di
convezione, la parte di un modello termico in cui è più facile sbagliare.

**Costante di tempo concentrata.** Per una verifica transitoria, prendi un corpo
di volume `V` e superficie `A` senza temperatura prescritta, che parte da `T0` e
viene raffreddato solo per convezione. Finché il numero di Biot `h*V/(A*k)` resta
al di sotto di circa `0.1`, il corpo è quasi isotermo e

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Leggi `tau` dall'andamento del punto di monitoraggio e confronta. Ciò valida
contemporaneamente la matrice di capacità, la condizione di convezione e il tuo
passo temporale - ed è il modo più rapido per scegliere un passo temporale per un
modello reale, poiché `tau/50` è una buona prima stima.

**Bilancio energetico.** Per un modello stazionario con una potenza in ingresso
nota `P`, la superficie convettiva deve smaltire esattamente `P` alla temperatura
finale:

```
P = h * A * ( Ts_mean - Tf )
```

Leggi la temperatura superficiale media dalle statistiche nel log del solutore e
verifica il bilancio. È l'unica verifica globale che rileva un valore Heat
inserito come totale anziché come densità.

Per ognuna di queste verifiche, raffina la mesh almeno una volta e conferma che la
risposta converge anziché variare.

---

## 6. Limitazioni

### Limitazioni di modellazione

- **Solo conduzione lineare.** Conducibilità, capacità e densità di un elemento
  sono costanti durante una soluzione, lette dalla tabella del materiale alla
  temperatura dell'esecuzione precedente. Una proprietà fortemente dipendente
  dalla temperatura richiede un gruppo di task che itera per convergere.
- **Nessun cambiamento di fase.** Non ci sono calore latente né formulazione in
  entalpia, quindi fusione, solidificazione, ebollizione e condensazione non sono
  modellate. Una tabella del materiale non può rappresentare il plateau latente.
- **Nessuna avvezione.** L'energia si sposta per conduzione all'interno del corpo
  e per convezione attraverso il suo contorno. Un mezzo in movimento non
  trasporta energia con sé; per questo usa *Heat transfer in fluids*.
- **La convezione è un modello di contorno.** Le forme correlate usano ciascuna
  un'unica correlazione fissa - lastra piana esterna per il flusso forzato, lastre
  orizzontali per la convezione naturale - con il diametro idraulico come
  lunghezza caratteristica. L'orientamento della superficie, il regime di flusso
  e se il flusso è interno o esterno non vengono rilevati. Quando il coefficiente
  è importante, calcolalo tu e inseriscilo con *Simple convection*.
- **Il coefficiente di convezione naturale è in ritardo.** Viene valutato alla
  temperatura della parete della soluzione precedente, quindi un'esecuzione
  stazionaria a passaggio singolo usa di fatto la differenza di temperatura
  iniziale.
- **Irraggiamento solo tramite il tipo di problema separato.** Nel solutore
  termico stesso non c'è un termine di emissività superficiale; l'irraggiamento in
  cavità richiede un task *Radiative heat transfer*, la sua matrice dei fattori di
  vista e un'iterazione tra i due.
- **Le superfici sono isoterme attraverso lo spessore.** Un'entità superficie
  conduce solo nel proprio piano. Modella con elementi di volume una parete il cui
  gradiente attraverso lo spessore è importante.
- **Gli elementi punto non conducono.** Un punto è una capacità concentrata e una
  sorgente di calore, niente di più.
- **Un modello stazionario ha bisogno di un'uscita.** Senza una temperatura
  prescritta né una superficie convettiva la matrice è singolare e la soluzione
  non converge.
