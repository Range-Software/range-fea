# Trasmissione del calore nei fluidi - Manuale teorico e guida utente

Questo documento descrive il solutore di trasmissione del calore nei fluidi di
Range FEA - `RSolverFluidHeat`, che gestisce il tipo di problema
**Trasmissione del calore nei fluidi** (*Heat transfer in fluids*): l'equazione
che risolve, il significato di ogni input che accetta, il suo accoppiamento con
il solutore termico di un solido e le parti dell'interfaccia grafica che lo
controllano.

La trasmissione del calore nei fluidi viene risolta sul campo di flusso di un
task **Flusso viscoso incomprimibile** (*Incompressible viscous flow*). Il
solutore del flusso, le sue condizioni al contorno, la sua iterazione non
lineare e il suo avanzamento nel tempo sono descritti nel manuale
[Flusso di fluidi](theory_fluid.md), il solutore termico dei solidi nel manuale
[Trasmissione del calore](theory_heat.md).

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Limitazioni](#3-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - la temperatura

L'incognita principale è la **temperatura** `T` in `K`, un grado di libertà per
nodo.

| Tipo di problema | Solutore | Incognita principale | Unità | Gradi di libertà per nodo |
|---|---|---|---|---|
| Heat transfer in fluids | `RSolverFluidHeat` | temperatura | `K` | 1 |

Il tipo di problema **richiede** un task *Incompressible viscous flow* nello
stesso problema: legge il campo di velocità dal suo risultato e non lo calcola
mai da sé.

### 1.2 Equazione di governo

La **trasmissione del calore nei fluidi** risolve l'avvezione-diffusione della
temperatura sul campo di flusso,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

con la stessa stabilizzazione SUPG del termine convettivo del solutore del
flusso e lo stesso avanzamento nel tempo a theta. La velocità `v` è **ricavata
dal risultato del flusso** e non viene risolta qui - per questo il tipo di
problema richiede un task *Incompressible viscous flow*.

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `T` | temperatura | `K` | incognita |
| `v` | velocità | `m/s` | il risultato del flusso |
| `k` | conducibilità termica | `W/(m*K)` | proprietà del materiale |
| `c` | capacità termica | `J/(kg*K)` | proprietà del materiale |
| `rho` | densità | `kg/m^3` | proprietà del materiale |
| `q` | densità della sorgente di calore | `W/m^3` | condizione Heat, calore Joule, irraggiamento |

La differenza rispetto al semplice tipo di problema **Trasmissione del calore**
è esattamente il termine `v . grad(T)`: l'energia viene trasportata dal fluido in
movimento e non solo condotta. Ciò significa anche che questo solutore non ha
condizioni al contorno di convezione - non c'è nulla da correlare, poiché il
calore che passa da una parete al fluido è risolto dalla mesh anziché
modellato.

### 1.3 Condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Temperature | esplicita (Dirichlet) | punto, linea, superficie, volume | Temperature `[K]` |
| Heat | naturale, sorgente | punto, linea, superficie, volume | Heat `[W]` |

**Temperature** mantiene i nodi della sua entità al valore dato. Assegnala alla
superficie di ingresso per impostare la temperatura del fluido che entra nel
dominio, e a una parete per modellare un contorno riscaldato o raffreddato.

**Heat** è una sorgente e accetta la **potenza totale in `W`** immessa
nell'entità, distribuita sulla misura dell'entità prima dell'assemblaggio - la
stessa convenzione del solutore termico. *Heat rate (unit area)* e *Heat rate
(unit volume)* sono offerte dall'interfaccia per questo tipo di problema, ma
**non vengono lette** dal solutore termico dei fluidi; qui usa *Heat* per una
sorgente.

Ogni componente è una **tabella in funzione del tempo**.

Le condizioni del flusso - pareti, ingressi e uscite - appartengono al task del
flusso e sono descritte nel manuale [Flusso di fluidi](theory_fluid.md).

### 1.4 Scambio termico coniugato

Una parete tra un fluido e un solido entrambi meshati, che porta la condizione
**Forced convection** del semplice solutore *Heat transfer*, accoppia i due.
Dopo l'esecuzione del solutore termico, il solutore termico dei fluidi mantiene i
nodi della parete alla temperatura del solido - fino ad allora la parete è
isolata - e restituisce, per ogni elemento della parete, la coppia `(h, Tf)` che
riproduce il flusso termico che la sua soluzione fa passare attraverso la
parete. Il solutore termico applica tale coppia come condizione di convezione.
Il flusso è il residuo delle equazioni del fluido nei nodi della parete,
coerente con la discretizzazione, quindi resta accurato anche in uno strato
limite termico sottile. La temperatura della parete viene rilassata da un
passaggio all'altro con un fattore di Aitken, che stabilizza lo scambio in pochi
passaggi. Il manuale [Trasmissione del calore](theory_heat.md) descrive lo
schema in *Scambio termico coniugato*. Il calore radiativo e il calore Joule
provenienti da altri solutori vengono aggiunti al termine sorgente come nel
solutore termico.

### 1.5 Convergenza

Senza accoppiamento, il solutore termico dei fluidi si dichiara sempre
convergente - è lineare una volta fissato il campo di flusso, quindi una sola
soluzione è già la risposta finale. Un gruppo che contiene sia il task del
flusso sia quello termico dei fluidi termina quindi quando converge il task del
flusso. Con pareti accoppiate riporta invece la variazione relativa del suo
campo di temperatura, così il gruppo itera finché la temperatura della parete
non si è stabilizzata.

### 1.6 Retroazione sul flusso - galleggiamento

La temperatura prodotta da questo solutore viene restituita al flusso. A ogni
passaggio il solutore del flusso valuta densità e viscosità dinamica di ogni
elemento all'ultima temperatura, quindi con un materiale la cui densità dipende
dalla temperatura la forza di volume `rho(T)*g` genera la **convezione
naturale** - il fluido caldo sale, quello freddo scende. I materiali *Water* e
*Air* forniti hanno una densità dipendente dalla temperatura.

In un'esecuzione stazionaria il primo passaggio del flusso vede solo la
temperatura iniziale, quindi il galleggiamento compare dalla seconda iterazione
del gruppo in poi; assegna al gruppo diverse iterazioni e un valore di
convergenza. In un'esecuzione transitoria con un'iterazione per passo temporale
il flusso vede la temperatura del passo precedente. Il manuale
[Flusso di fluidi](theory_fluid.md) descrive lo schema nella sezione 1.9.

### 1.7 Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Temperature | campo di temperatura iniziale |

Le condizioni iniziali vengono lette **solo alla prima esecuzione**; in seguito
il campo avanza a partire dal risultato precedente. Una condizione al contorno
prevale su una condizione iniziale dove le due si incontrano.

### 1.8 Risultati

| Risultato | Si applica a | Significato |
|---|---|---|
| Temperature `[K]` | nodo | il campo di temperatura risolto |
| Heat flux `[W/m^2]` | elemento | flusso conduttivo `-k*grad(T)` |

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Materiale`, `Risultati`), nel
menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Le combinazioni da conoscere:

- *Incompressible viscous flow* e *Heat transfer in fluids* in un unico gruppo -
  un flusso riscaldato o raffreddato. Assegna al gruppo diverse iterazioni quando
  il flusso è generato o modificato in modo sensibile dal galleggiamento, così
  che flusso e temperatura si stabilizzino insieme - vedi 1.6;
- *Heat transfer in fluids* e un task *Heat transfer* separato su un solido
  meshato, prima il task termico dei fluidi, in un unico gruppo con diverse
  iterazioni - scambio termico coniugato attraverso le pareti con *Forced
  convection*.

Il tipo di problema richiede la presenza di un task del flusso. I valori del
gruppo di task - **N. di iterazioni** e **Convergenza** - sono descritti nel
manuale [Flusso di fluidi](theory_fluid.md).

### 2.2 Scheda Problema

**Solutore temporale** - viene mostrato perché il tipo di problema dipende dal
tempo. Lascialo disattivato per un'esecuzione stazionaria. I suoi campi sono
descritti nel manuale [Flusso di fluidi](theory_fluid.md).

Il tipo di problema non ha un riquadro di impostazioni proprio. Tutto il resto si
assegna per entità nelle schede delle condizioni.

### 2.3 Scheda Materiale

| Proprietà | Unità | Commento |
|---|---|---|
| Density | `kg/m^3` | `rho`, circa `1000` per l'acqua, `1.2` per l'aria |
| Dynamic viscosity | `kg/(m*s)` | `mu`, circa `1.0e-3` per l'acqua, `1.8e-5` per l'aria |
| Thermal conductivity | `W/(m*K)` | `k`, circa `0.6` per l'acqua, `0.025` per l'aria |
| Heat capacity | `J/(kg*K)` | `c`, circa `4180` per l'acqua, `1005` per l'aria |

Un'entità a cui manca una proprietà richiesta non viene risolta, e il controllo
della configurazione avvisa prima dell'avvio dell'esecuzione. Il database dei
materiali fornito include **Water** e **Air**, il che di solito esaurisce questo
passaggio. Ogni proprietà è una tabella in funzione della temperatura, valutata
alla temperatura dell'elemento dell'esecuzione precedente.

### 2.4 Scheda Condizioni al contorno

**Temperature** e **Heat** compaiono quando nel flusso dei task è presente un
task *Heat transfer in fluids* - vedi la sezione 1.3. Ogni componente è una
tabella in funzione del tempo; fai clic su **Modifica valori dipendenti dal
tempo** per inserire i valori in funzione del tempo.

### 2.5 Condizioni iniziali

La scheda `Condizioni iniziali` offre *Temperature*. Imposta il campo di
partenza della prima esecuzione.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Il solutore termico dei fluidi usa la voce **GMRES**, poiché
la sua matrice non è simmetrica.

### 2.7 Punti di monitoraggio e risultati

`Problema` -> `Definisci punti di monitoraggio` può registrare *Temperature*. La
scheda `Risultati` mostra la temperatura come scalare nodale e il flusso termico
come vettore di elemento.

---

## 3. Limitazioni

- **Galleggiamento senza dilatazione del fluido.** La temperatura modifica il
  flusso tramite `rho(T)` e `mu(T)` (vedi 1.6), ma la dilatazione termica del
  fluido non entra mai nel bilancio di massa `div(v) = 0`. Ciò è accurato per le
  piccole differenze di densità della convezione naturale, non per un gas
  riscaldato di centinaia di kelvin. Il flusso di galleggiamento è laminare, come
  ogni flusso in Range FEA.
- **L'accoppiamento è sfalsato, non monolitico.** Flusso e temperatura vengono
  risolti uno dopo l'altro e si scambiano dati tra i passaggi. Un flusso di
  galleggiamento stazionario richiede un gruppo con diverse iterazioni; con
  un'iterazione per passo temporale, un flusso transitorio è in ritardo di un
  passo rispetto alla temperatura.
- **Le condizioni `Heat rate` vengono ignorate.** *Heat rate (unit area)* e
  *Heat rate (unit volume)* sono offerte dall'interfaccia per *Heat transfer in
  fluids*, ma non vengono lette da `RSolverFluidHeat`; lo raggiunge solo la
  condizione *Heat*. Il semplice tipo di problema *Heat transfer* le legge tutte
  e tre.
- **Nessuna condizione al contorno di convezione.** Il calore che passa da una
  parete a un fluido meshato viene risolto anziché correlato, quindi le
  condizioni *Simple*, *Forced* e *Natural convection* appartengono al semplice
  solutore termico. Lo scambio termico coniugato si imposta come task *Heat
  transfer* sul solido, le cui pareti con *Forced convection* leggono lo stato
  calcolato del fluido.
- **Nessun modello di turbolenza.** Il flusso è laminare, vedi il manuale
  [Flusso di fluidi](theory_fluid.md), quindi la trasmissione del calore
  turbolenta non è rappresentata.
