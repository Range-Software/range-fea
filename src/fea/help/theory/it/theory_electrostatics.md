# Elettrostatica - Manuale teorico e guida utente

Questo documento descrive il solutore di elettrostatica
(`RSolverElectrostatics`) di Range FEA, che gestisce il tipo di problema
**Elettrostatica** (*Electro-statics*): le equazioni che risolve, il significato
di ogni input che accetta, le parti dell'interfaccia grafica che lo controllano
e due tutorial svolti.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - potenziale e corrente in un conduttore](#3-tutorial---potenziale-e-corrente-in-un-conduttore)
4. [Tutorial - riscaldamento resistivo](#4-tutorial---riscaldamento-resistivo)
5. [Verifica di un modello](#5-verifica-di-un-modello)
6. [Limitazioni](#6-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - il campo del potenziale elettrico

L'incognita principale è il **potenziale elettrico** `V` in ogni nodo, in volt.
Ogni nodo ha un solo grado di libertà, quindi una mesh con `N` nodi dà un
sistema di `N` equazioni. Il campo elettrico, la densità di corrente e tutto ciò
che ne deriva vengono ricavati dal campo del potenziale dopo la soluzione.

Il potenziale è uno scalare, il che rende un modello elettrostatico economico
quanto un modello termico sulla stessa mesh: un'incognita per nodo anziché tre, e
una matrice con un nono dei termini di un modello strutturale.

Il potenziale predefinito di un nodo non toccato da alcuna condizione né da un
risultato precedente è `0 V`.

Un campo di potenziale è definito solo **a meno di una costante additiva**. Nulla
nelle equazioni fissa il livello zero, quindi ogni modello richiede almeno una
condizione al contorno *Electric potential* - vedi 1.7. È l'equivalente
elettrostatico di una struttura non vincolata, o di un modello termico senza
temperatura prescritta.

### 1.2 Equazione di governo

Il solutore implementa l'**equazione del campo elettrostatico** - l'equazione di
Poisson per il potenziale di un dielettrico lineare e isotropo:

```
div( e0 * er * grad(V) ) = -rho
```

e, in assenza di qualsiasi densità di carica, si riduce all'equazione di Laplace

```
div( e0 * er * grad(V) ) = 0
```

Il campo elettrico è il gradiente negativo del potenziale, e la densità di
corrente segue dalla legge di Ohm:

```
E = -grad(V)
J = sigma * E
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `V` | potenziale elettrico | `V` | incognita |
| `rho` | densità di carica | `C/m^3`, vedi 1.5 | condizione Charge density |
| `e0` | permittività del vuoto | `F/m` | integrata, `8.854187817e-12` |
| `er` | permittività relativa | `-` | proprietà del materiale |
| `sigma` | conducibilità elettrica | `S/m` | proprietà del materiale |
| `E` | campo elettrico | `V/m` | ricavato, vedi 1.8 |
| `J` | densità di corrente | `A/m^2` | ricavata, vedi 1.8 |

La permittività del vuoto è una costante del solutore, non una proprietà del
materiale né un'impostazione - con il materiale si inserisce solo la
permittività **relativa**.

**Il campo è determinato dalla permittività, non dalla conducibilità.** È la cosa
più importante da sapere su questo solutore. La matrice di rigidezza è costruita
solo da `e0*er`; la conducibilità non entra mai nel sistema di equazioni. Viene
usata in seguito, per trasformare il campo calcolato in densità di corrente,
resistività e calore Joule. La conseguenza è spiegata in 1.9: in un corpo di un
solo materiale non fa differenza, ma in un corpo di **due** materiali il
potenziale si ripartisce in proporzione alle permittività e non in proporzione
alle resistenze.

**Proprietà del materiale richieste.** Un problema elettrostatico ne richiede
**tre**:

| Proprietà | Unità | Usata per |
|---|---|---|
| Relative permittivity | `-` | la matrice di rigidezza |
| Electrical conductivity | `S/m` | densità di corrente, resistività, calore Joule |
| Density | `kg/m^3` | nulla - richiesta, mai letta |

Un'entità il cui materiale manca di una delle tre **non viene risolta**, anche se
la densità non viene mai letta da questo solutore. Il controllo della
configurazione lo segnala prima dell'avvio dell'esecuzione. L'unica eccezione è
un'entità che porta una condizione al contorno elettrostatica: quell'entità viene
inclusa nel sistema qualunque sia il suo materiale, e la sua permittività e
conducibilità sono allora semplicemente nulle.

Una permittività nulla non è un valore predefinito innocuo - un elemento con
`er = 0` non ha rigidezza, quindi non contribuisce al sistema e lascia i suoi
nodi liberi. Se un'entità deve essere risolta, assegnale una permittività reale:
`1` per il vuoto o, con buona approssimazione, per l'aria.

Ogni proprietà è una tabella in funzione della temperatura, ma un task di
elettrostatica da solo non ha un campo di temperatura in cui valutarla, quindi
ogni tabella viene valutata a `293.15 K`. Le tabelle vengono rispettate solo
quando un task *Heat transfer* precedente nello stesso flusso di task ha
calcolato le temperature degli elementi, che vengono condivise con i task
successivi.

La permittività relativa è **adimensionale** - nell'interfaccia è mostrata come
`N/A`, come l'emissività e il coefficiente di Poisson - e il valore viene usato
come semplice moltiplicatore di `e0`. Inserisci `1` per il vuoto, `1.00059` per
l'aria, `2.1` per il PTFE, circa `4` per il vetro.

### 1.3 Discretizzazione agli elementi finiti

La forma debole dell'equazione del campo dà, per ogni elemento,

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(`stiffness` = rigidezza, `charge source` = sorgente di carica), dove `B` è
costruita dalle derivate delle funzioni di forma e `N` sono le funzioni di forma
stesse. Ogni matrice di elemento è `n x n` per un elemento con `n` nodi - una
riga per nodo, non tre. Non c'è matrice di massa né di capacità: il problema è
statico, e 1.10 spiega cosa significa per il flusso dei task.

**Misure degli elementi.** Come altrove in Range FEA, alle entità di dimensione
inferiore si assegna una misura geometrica affinché possano rappresentare corpi
reali:

| Entità | Misura | Dove si imposta |
|---|---|---|
| Volume | 1 | - |
| Superficie | spessore della superficie | proprietà geometriche dell'entità |
| Linea | area della sezione della linea | proprietà geometriche dell'entità |
| Punto | volume del punto | proprietà geometriche dell'entità |

Una **superficie** porta il campo nel proprio piano, attraverso una sezione pari
al suo spessore. Non c'è gradiente di potenziale attraverso lo spessore: un
elemento di superficie è spesso un nodo, quindi per costruzione è equipotenziale
attraverso il suo spessore. Una superficie di **spessore nullo** non ha quindi
alcuna rigidezza e non collega nulla - a differenza del solutore termico, in cui
una superficie di spessore nullo porta comunque la convezione, qui una superficie
di spessore nullo è semplicemente inerte. Assegna uno spessore a un'entità
superficie ogni volta che deve condurre, e ricorda che una faccia di spessore
nullo non riceve nemmeno **risultati derivati** (1.8).

Una **linea** porta il campo lungo la propria direzione, attraverso la sua area
della sezione. Un **punto** non ha una direzione lungo cui portarlo, quindi non
contribuisce con **alcuna rigidezza**: contiene solo una sorgente di carica. Un
elemento punto il cui nodo non è condiviso con nessun altro elemento calcolabile
produce quindi una riga vuota nella matrice, che il solutore a gradiente
coniugato non può gestire - un punto è un luogo in cui iniettare carica o
prescrivere un potenziale, non un conduttore.

### 1.4 Sorgenti e condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Electric potential | esplicita (Dirichlet) | punto, linea, superficie, volume | Electric potential `[V]` |
| Charge density | naturale, sorgente | punto, linea, superficie, volume | Charge density, vedi 1.5 |

Queste due costituiscono l'intero vocabolario del tipo di problema. Non esistono
flusso prescritto, carica superficiale, condizione di campo lontano o di contorno
aperto, né condizione di simmetria - vedi la sezione 6.

**Electric potential** è l'unica condizione esplicita del problema
elettrostatico. Ogni nodo dell'entità a cui è assegnata viene rimosso dal sistema
di equazioni e mantenuto al valore dato; il suo contributo viene portato nel
termine noto delle equazioni adiacenti. Ogni modello ne richiede almeno una. Una
condizione appena assegnata parte da `1 V`.

Dove le entità si sovrappongono, **prevale l'entità di dimensione inferiore**: un
potenziale prescritto su una superficie prevale, nei nodi condivisi, su uno
prescritto sul volume retrostante, una linea prevale su una superficie e un punto
prevale su tutto. I potenziali non vengono combinati - c'è un solo grado di
libertà per nodo da mantenere, quindi prevale semplicemente l'ultima condizione
letta.

**Charge density** è il termine sorgente dell'equazione di Poisson. A differenza
della condizione *Heat* del solutore termico, il suo valore **non** viene
distribuito sulla misura dell'entità: il numero inserito viene applicato così
com'è, come densità, a ogni elemento dell'entità. La carica totale posta in
un'entità volume è quindi il valore moltiplicato per il volume di quell'entità, e
cresce se l'entità viene ingrandita - non resta fissa come fa un valore *Heat* in
watt.

| Entità | Significato del valore |
|---|---|
| Volume | carica per unità di volume |
| Superficie | carica per unità di volume della sezione pesata con lo spessore |
| Linea | carica per unità di volume della sezione trasversale |
| Punto | una sorgente di carica nodale |

Entrambe le componenti accettano una tabella in funzione del tempo. Poiché il
problema stesso è statico, un andamento nel tempo ha effetto solo quando un altro
task, dipendente dal tempo, condivide il flusso dei task - e, come spiega 1.10,
quella combinazione è a sua volta limitata.

### 1.5 Il valore della densità di carica

La componente *Charge density* è una **densità di carica volumetrica** in
`C/m^3`, come indica la sua etichetta nell'interfaccia. È una densità e non un
totale, il che la rende indipendente dalla mesh - e, come nota 1.4, la distingue
dalla condizione *Heat* del solutore termico, che accetta un totale in watt e lo
distribuisce per te.

Il segno segue la convenzione consueta. Integrando per parti
`div(e0*er*grad(V)) = -rho` la carica resta nel termine noto con segno positivo,

```
every entity type      f = +INT( N^T * rho )
```

(`every entity type` = ogni tipo di entità), quindi una densità di carica
**positiva** innalza il potenziale attorno a sé e una negativa lo abbassa, allo
stesso modo su entità volume, superficie, linea e punto.

Ricorda che una sorgente di carica da sola non rende risolvibile un modello. Il
potenziale resta definito solo a meno di una costante finché qualcosa non lo
fissa, quindi un modello guidato dalla carica richiede anche almeno una
condizione *Electric potential* - sul contorno esterno della regione meshata, se
non altrove. Vedi 1.7.

### 1.6 Condizioni ambientali, iniziali e di accoppiamento

#### Condizioni ambientali

| Condizione ambientale | Componenti | Effetto |
|---|---|---|
| Temperature | Temperature `[K]` | offerta, ma non letta - vedi sotto |

La condizione ambientale *Temperature* viene offerta per un problema
elettrostatico perché le proprietà del materiale sono tabelle in funzione della
temperatura. Il solutore di elettrostatica **non** la legge: prende le
temperature degli elementi solo dai dati condivisi lasciati da un task *Heat
transfer*, e altrimenti valuta ogni tabella a `293.15 K`. Assegnarla a un modello
di sola elettrostatica non cambia nulla.

#### Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Electric potential | campo di potenziale iniziale, applicato alla prima esecuzione |

Il potenziale iniziale viene letto **solo alla prima esecuzione**. Poiché la
soluzione è diretta e non un avanzamento, non cambia la risposta convergente -
imposta solo il punto di partenza dell'iterazione a gradiente coniugato e il
valore di qualsiasi nodo che il solutore lascia fuori dal sistema.

Condizioni ambientali, condizioni iniziali e condizioni al contorno vengono lette
in quest'ordine e ciascuna sovrascrive la precedente, quindi **una condizione al
contorno prevale su una condizione iniziale, che prevale su una condizione
ambientale**.

#### Accoppiamento con altri solutori

Il solutore di elettrostatica produce due risultati utilizzati da altri solutori.

| Direzione | Variabile | Effetto |
|---|---|---|
| Elettrostatica -> termico | Joule heat | aggiunto al termine sorgente di ogni elemento |
| Elettrostatica -> termico dei fluidi | Joule heat | aggiunto al termine sorgente di ogni elemento |
| Elettrostatica -> magnetostatica | Current density | sorgente del campo magnetico |
| Termico -> elettrostatica | Temperature | seleziona la riga di ogni tabella del materiale dipendente dalla temperatura |

Il **calore Joule** viene raccolto da un task *Heat transfer* posto **dopo** il
task di elettrostatica nel flusso dei task del problema, dove viene aggiunto al
termine sorgente dell'elemento esattamente come farebbe una condizione *Heat rate
(unit volume)*. Il riscaldamento resistivo non richiede quindi altra
configurazione oltre all'ordine dei due task - il tutorial 4 lo illustra.

La **magnetostatica** è un tipo di problema separato che *richiede* un task di
elettrostatica: legge la densità di corrente di elemento memorizzata da questo
solutore - su entità volume, superficie e linea allo stesso modo - e valuta il
campo magnetico di quella corrente con la legge di Biot-Savart. Aggiungi prima il
task di elettrostatica.

Poiché il solutore si dichiara sempre convergente, un gruppo di task che contiene
solo un task di elettrostatica si ferma sempre dopo un'iterazione. Iterare
conviene solo quando qualcosa da cui dipende la soluzione è a sua volta in
ritardo - ad esempio una permittività fortemente dipendente dalla temperatura
alimentata da un task termico.

### 1.7 Risoluzione del sistema

Il solutore assembla e risolve

```
K * V = f
```

`K` è simmetrica definita positiva non appena è stato prescritto almeno un
potenziale, quindi viene risolta con il metodo del **gradiente coniugato** con
precondizionamento di Jacobi. L'assemblaggio è parallelizzato sugli elementi con
buffer per thread che vengono uniti una sola volta alla fine, quindi il risultato
non dipende dal numero di thread.

Un modello **senza condizione al contorno Electric potential** è singolare: il
potenziale è definito solo a meno di una costante, la matrice ha uno spazio
nullo e la soluzione non converge. Ogni modello richiede almeno un potenziale
prescritto, e un modello che deve portare una corrente ne richiede due con valori
diversi.

Vale la pena conoscere altri tre modi per arrivare a un sistema singolare o
vuoto, perché tutti e tre sono silenziosi:

- un'entità il cui materiale manca di una delle tre proprietà richieste non viene
  risolta, e i suoi nodi escono dal sistema;
- un'entità con `er = 0` non ha rigidezza, quindi i suoi nodi restano nel sistema
  ma non sono collegati a nulla;
- anche una superficie con spessore nullo, o una linea con area della sezione
  nulla, non ha rigidezza.

### 1.8 Risultati derivati

Dopo la soluzione i risultati di elemento vengono ricavati dal campo di
potenziale nodale. Il campo elettrico è il gradiente negativo del potenziale
mediato sui punti di integrazione e ruotato in coordinate globali per gli
elementi linea e superficie:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`electric energy` = energia elettrica, `electrical resistivity` = resistività
elettrica, `Joule heat` = calore Joule.)

| Risultato | Si applica a | Significato |
|---|---|---|
| Electric potential `[V]` | nodo | il campo di potenziale risolto |
| Electric field `[V/m]` | elemento | vettore del campo |
| Current density `[A/m^2]` | elemento | `sigma*E`, nulla dove la conducibilità è nulla |
| Electric energy `[J/m^3]` | elemento | densità di energia del campo |
| Electrical resistivity `[V*m/A]` | elemento | `1/sigma` dovunque scorra corrente, `0` altrove |
| Joule heat `[W/m^3]` | elemento | dissipazione resistiva, passata al solutore termico |

Tutti questi sono **densità**: le derivate delle funzioni di forma vengono
mediate sui punti di integrazione dell'elemento e non pesate con il suo
determinante jacobiano, e lo spessore della superficie e l'area della sezione
della linea restano nella rigidezza, dove devono stare, anziché entrare nel campo
ricavato. Campo, densità di corrente ed energia elettrica sono quindi
indipendenti dalla dimensione dell'elemento in cui sono stati calcolati, possono
essere confrontati tra elementi e con un calcolo manuale, e convergono al
raffinarsi della mesh.

La **resistività elettrica** è il risultato a cui ricorrere per verificare un
modello, perché i due fattori da cui è costruita si annullano: `|E|/|sigma*E|` è
esattamente `1/sigma`, qualunque cosa faccia il campo. È la conferma più rapida
che la conducibilità prevista per un'entità ha effettivamente raggiunto i suoi
elementi. Viene memorizzata come `0` su ogni elemento in cui la densità di
corrente scende sotto la tolleranza numerica, il che include ogni elemento con
conducibilità nulla.

**Il calore Joule** è la densità di dissipazione `sigma*|E|^2`, in `W/m^3`. È
esattamente ciò che si aspetta un task *Heat transfer* quando aggiunge il valore
al proprio termine sorgente e lo integra sull'elemento, quindi la potenza che
passa a un'esecuzione termica accoppiata è la potenza che il modello dissipa
effettivamente. Come gli altri è una densità, quindi è indipendente dalla
dimensione dell'elemento e converge con il raffinamento.

Altri due casi in cui i risultati derivati restano semplicemente a zero: gli
**elementi punto** non vengono mai post-elaborati, e una **superficie con
spessore nullo** o una **linea con area della sezione nulla** viene saltata,
quindi entrambe mantengono un campo nullo per quanto grande sia il gradiente di
potenziale attraverso di esse.

Dopo la scrittura del record il log del solutore stampa le statistiche di tutte e
sei le variabili e i valori in tutti i punti di monitoraggio.

### 1.9 Cosa è il solutore e cosa non è

Il nome è esatto: questo è un solutore **elettrostatico**. Calcola il campo di
una distribuzione di carica e di potenziale in un dielettrico. La densità di
corrente che riporta è ciò che la legge di Ohm dà per quel campo - una grandezza
derivata, calcolata a posteriori, che non influenza mai il campo stesso.

Un problema di **conduzione stazionaria di corrente**, invece, è governato da

```
div( sigma * grad(V) ) = 0
```

con la conducibilità al posto in cui questo solutore mette la permittività. I due
danno lo stesso campo di potenziale in un corpo omogeneo, perché entrambi si
riducono all'equazione di Laplace e la costante si semplifica. Divergono non
appena il corpo è composto da più di un materiale:

| | Questo solutore | Un solutore di conduzione |
|---|---|---|
| Il potenziale si ripartisce in proporzione a | `1/er` di ogni regione | `1/sigma` di ogni regione |
| Corretto per | un condensatore, un dielettrico, un isolante | una rete di resistenze, una barra di distribuzione, un percorso conduttivo |

Un conduttore di un solo materiale con potenziali prescritti alle estremità è
quindi modellato correttamente: il potenziale è lineare e dividendo per la
lunghezza si ottiene il campo. Due conduttori di conducibilità diversa in serie
no - la ripartizione segue le permittività, il che difficilmente è ciò che si
intendeva. Modella un caso del genere come un unico materiale, oppure calcola la
ripartizione a mano.

### 1.10 Analisi statica e solutore temporale

L'elettrostatica è dichiarata come tipo di problema **senza** solutore temporale.
Ciò ha due conseguenze visibili.

In un modello di sola elettrostatica il riquadro *Solutore temporale* non compare
affatto nella scheda `Problema`, l'esecuzione produce un unico record e non c'è
nulla da configurare.

Più sorprendentemente, aggiungere un task di elettrostatica a un flusso di task
**disattiva il solutore temporale per l'intero flusso**. Il solutore temporale
viene offerto solo quando *ogni* tipo di problema del flusso ne supporta uno,
quindi un'analisi termica transitoria a cui si aggiunge un task di elettrostatica
diventa un'analisi stazionaria, senza altro avviso che la scomparsa del
riquadro. Se serve una risposta termica transitoria per un riscaldatore
resistivo, esegui separatamente il task di elettrostatica e riporta la sua
dissipazione nel modello termico transitorio come condizione *Heat*, anziché
mettere entrambi i task in un unico flusso.

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Condizioni ambientali`,
`Materiale`, `Risultati`), nel menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Aggiungi un task e scegli **Electro-statics** -
"Calculate electric field, current density and generated Joule's heat" (calcola
il campo elettrico, la densità di corrente e il calore Joule generato).

L'elettrostatica si combina liberamente con ogni altra fisica tranne *Modal
analysis*. Le catene da conoscere sono:

- *Electro-statics* e poi *Heat transfer* - riscaldamento resistivo;
- *Electro-statics* e poi *Magneto-statics* - il campo magnetico della corrente
  calcolata, che è l'unico modo per eseguire un'analisi magnetostatica;
- *Electro-statics*, poi *Heat transfer*, poi *Stress analysis* - tensioni
  termiche dovute al riscaldamento resistivo.

Ricorda 1.10: qualsiasi flusso che contiene un task di elettrostatica viene
risolto come stazionario.

### 2.2 Scheda Problema

Per un modello di sola elettrostatica questa scheda è **vuota**. Non c'è un
solutore temporale, perché il tipo di problema non lo supporta, e
l'elettrostatica non ha un riquadro di impostazioni proprio. Tutto si assegna per
entità nelle schede delle condizioni.

Se la scheda mostra un riquadro *Solutore temporale*, l'ha aggiunto un altro task
del flusso - e, come da 1.10, scomparirà di nuovo non appena verrà aggiunto il
task di elettrostatica.

### 2.3 Scheda Materiale

La scheda `Materiale` elenca il materiale assegnato all'entità selezionata.
L'elettrostatica richiede **tutte e tre** queste proprietà:

| Proprietà | Unità | Commento |
|---|---|---|
| Relative permittivity | `N/A`, adimensionale | `er`, `1` per il vuoto, `1.00059` per l'aria, circa `4` per il vetro |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` per il rame, `5e-15` per l'aria |
| Density | `kg/m^3` | richiesta perché l'entità venga risolta, mai letta |

Un'entità che ne manca una non viene risolta, e il controllo della
configurazione avvisa prima dell'avvio dell'esecuzione con *Entity ... has
material assigned which is missing required properties*. Poiché la densità non
viene mai usata da questo solutore, qualsiasi valore positivo soddisfa il
requisito.

La libreria dei materiali fornita contiene già tutte e tre per i metalli, per
l'aria e per i vetri, quindi assegnare *Copper* o *Air* basta per iniziare.

Ogni proprietà è una tabella in funzione della temperatura, ma la dipendenza
dalla temperatura viene rispettata solo quando un task termico precedente nel
flusso ha prodotto le temperature degli elementi - vedi 1.6.

### 2.4 Scheda Condizioni al contorno

La scheda è suddivisa in un elenco gestore delle condizioni al contorno
disponibili in alto e in un editor per quella selezionata in basso. Seleziona
prima un'entità nell'albero `Modello`; il gestore offre allora le condizioni
valide per quel tipo di entità e per il problema selezionato. Attivane una per
assegnarla, poi modificane le componenti nell'albero inferiore.

Vengono offerte solo le due condizioni della sezione 1.4. Note sul loro uso:

- **Electric potential** è la condizione che mantiene un valore anziché generarlo,
  ed è offerta per ogni tipo di entità. Assegnala alla faccia, al bordo o al punto
  il cui potenziale è noto. Un modello senza nessuna di queste non si risolve.
- **Charge density** è il termine sorgente, e un valore positivo innalza il
  potenziale attorno a sé. Leggi 1.5 prima di usarla: il valore è una densità e,
  a differenza della condizione *Heat* del solutore termico, non viene
  distribuito sull'entità.
- Entrambe le componenti sono tabelle in funzione del tempo, il che conta solo in
  un flusso di task guidato da un altro task dipendente dal tempo.

### 2.5 Condizioni iniziali e ambientali

La scheda `Condizioni iniziali` offre *Electric potential*, che imposta il campo
di partenza della soluzione iterativa. Non cambia la risposta convergente.

La scheda `Condizioni ambientali` offre *Temperature*, che questo solutore ignora -
vedi 1.6.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Il solutore di elettrostatica usa la voce **CG**.

Se la soluzione non converge, la causa abituale è un modello senza potenziale
prescritto, o un'entità con permittività nulla, più che un'impostazione del
solutore. Controlla le condizioni al contorno e il materiale prima di aumentare
il numero di iterazioni.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare - per un problema elettrostatico la scelta
abituale è *Electric potential*, ed è il modo più rapido per leggere un valore da
un modello come numero anziché come colore. `Report` -> `Punti di monitoraggio`
mostra i valori.

### 2.8 Risultati e record

La scheda `Risultati` elenca le variabili calcolate e controlla la vista 3D. Il
potenziale elettrico è uno scalare nodale; il campo elettrico e la densità di
corrente sono vettori di elemento; l'energia elettrica, la resistività elettrica
e il calore Joule sono scalari di elemento.

La scheda `Record` del pannello `Modello` elenca i record dei risultati.
Un'analisi elettrostatica è statica, quindi ce n'è esattamente **uno**.

`Report` -> `File di log del solutore` mostra l'output completo del solutore,
comprese le statistiche di tutte e sei le variabili dopo la scrittura del record.

---

## 3. Tutorial - potenziale e corrente in un conduttore

**Obiettivo.** Calcolare la distribuzione del potenziale lungo una barra di rame
mantenuta a due potenziali diversi, e leggerne il campo, la corrente e la
resistenza.

Questo tutorial presuppone un modello di volume meshato - un parallelepipedo
allungato va bene. Usa `File` -> `Nuovo modello`, disegna un parallelepipedo con
`Geometria` -> `Crea elemento` o importa una geometria, e tetraedrizzala; il
tutorial *Disegnare un cubo* tratta questi passaggi. Marca le due facce
terminali come entità superficie separate, in modo da potervi assegnare le
condizioni.

Considera la barra lunga `0.1 m` con una sezione di `0.01 m` per `0.01 m`, il che
rende facili da seguire i calcoli manuali alla fine.

### Passo 1 - selezionare il problema

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungi un task e seleziona **Electro-statics**.
3. Conferma con `OK`.

La scheda `Problema` resta vuota - non c'è nulla da configurare.

### Passo 2 - assegnare il materiale

1. Seleziona l'entità volume nell'albero `Modello`.
2. Apri la scheda `Materiale` e assegna **Copper**, oppure un materiale con tutte
   e tre le proprietà:
   - **Relative permittivity** = `2.8`
   - **Electrical conductivity** = `5.882e7` `S/m`
   - **Density** = `8960` `kg/m^3`

La densità non viene usata da questo solutore, ma deve essere presente perché
l'entità venga risolta. La permittività determina il campo; la conducibilità
trasforma il campo in corrente solo in seguito.

### Passo 3 - prescrivere le due estremità

1. Seleziona l'entità superficie a un'estremità.
2. Apri la scheda `Condizioni al contorno` e attiva **Electric potential**.
3. Imposta il valore a `0` V - questa è la massa.
4. Seleziona l'entità superficie all'altra estremità, attiva di nuovo **Electric
   potential** e impostalo a `10` V.

Questo è il passo che rende il modello risolvibile. Un solo potenziale prescritto
è il minimo; due con valori diversi sono ciò che crea un campo.

Non aggiungere una condizione *Charge density*. Un conduttore con potenziali
prescritti alle estremità non ha carica spaziale, quindi il termine sorgente deve
essere nullo e il potenziale è una retta tra le due estremità - il che rende
esatta la verifica manuale del passo 6.

### Passo 4 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Il controllo della configurazione segnala i materiali mancanti prima
   dell'avvio dell'esecuzione.
3. Segui l'avanzamento in `Report` -> `File di log del solutore`.

L'esecuzione è una singola soluzione a gradiente coniugato e su una mesh di
queste dimensioni termina in pochi secondi.

### Passo 5 - esaminare i risultati

- Scheda `Risultati`: visualizza **Electric potential**. Dovrebbe diminuire
  linearmente da `10` V a `0` V lungo la barra, con piani equipotenziali piatti
  perpendicolari al suo asse. Qualsiasi curvatura in una barra uniforme indica
  che la mesh non è collegata come si presume.
- **Electrical resistivity** dovrebbe indicare `1/5.882e7 = 1.7e-8` `V*m/A` su
  ogni elemento della barra. È la migliore verifica singola che il materiale
  abbia raggiunto gli elementi: è esatta, indipendente dalla mesh e sbagliata non
  appena lo è la conducibilità.
- **Electric field** è diretto lungo la barra e dovrebbe indicare uniformemente
  `10/0.1 = 100` `V/m`, e **Current density** `sigma*E = 5.88e9` `A/m^2`.
  Entrambi sono densità, quindi i valori valgono qualunque sia la dimensione
  dell'elemento - il passo 6 li ricava.
- Un punto di monitoraggio su *Electric potential* a metà della barra dovrebbe
  indicare `5` V.

### Passo 6 - ricavare i valori a mano

Le prime due righe sotto sono ciò che i risultati **Electric field** e **Current
density** dovrebbero già mostrare, quindi sono una verifica diretta della
soluzione. Le ultime tre sono totali anziché densità, che nessun risultato di
elemento fornisce, e devono essere calcolate. Per una barra uniforme di lunghezza
`L` e sezione `A` mantenuta a una differenza di potenziale `U`:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

La corrente è enorme perché una barra di rame pieno di quella sezione è un ottimo
conduttore e `10` V ai suoi capi sono un cortocircuito - un utile promemoria che
un modello elettrostatico calcolerà volentieri una situazione che vaporizzerebbe
il pezzo reale.

### Passo 7 - facoltativo, raffinare la mesh

Raffina la mesh una volta e risolvi di nuovo. Ogni risultato - il
**potenziale**, il **campo elettrico**, la **densità di corrente**, l'**energia
elettrica**, la **resistività** e il **calore Joule** - deve mantenere il proprio
valore. Sono tutti densità, quindi un valore che varia con il raffinamento è il
segnale di un problema del modello e non di una mesh più fine, e vale la pena
indagarlo prima di proseguire.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La soluzione non converge | nessuna condizione *Electric potential*, quindi il potenziale è definito solo a meno di una costante |
| *Entity has material assigned which is missing required properties* | manca una delle tre proprietà richieste - spesso la densità, che il solutore non usa mai |
| L'intero modello è a un unico potenziale | è stato prescritto un solo potenziale, oppure il secondo ha lo stesso valore |
| Il potenziale è nullo ovunque tranne sulle facce prescritte | l'entità ha `er = 0`, quindi non ha rigidezza e non collega nulla |
| La resistività è nulla ovunque | la conducibilità elettrica è nulla, quindi non scorre corrente e la resistività viene memorizzata come zero |
| Un'entità superficie non mostra alcun campo | il suo spessore è nullo, quindi non viene né assemblata né post-elaborata |

---

## 4. Tutorial - riscaldamento resistivo

**Obiettivo.** Usare il calore Joule calcolato dal campo elettrico per riscaldare
la barra, concatenando un task di elettrostatica e un task termico.

### Passo 1 - costruire il flusso dei task

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungi un task **Electro-statics**.
3. Aggiungi un task **Heat transfer** **dopo** di esso.
4. Conferma con `OK`.

L'ordine è importante: il solutore termico legge il calore Joule lasciato dal
solutore di elettrostatica, quindi il task di elettrostatica deve essere eseguito
per primo. Per collegarli non serve alcuna condizione né impostazione.

Nota cosa succede alla scheda `Problema`: il riquadro *Solutore temporale* che un
task termico da solo mostrerebbe è ora sparito, perché l'elettrostatica non ha un
solutore temporale e il flusso adotta l'impostazione più restrittiva - vedi 1.10.
Questa catena è solo stazionaria.

### Passo 2 - completare il materiale

Il task termico richiede quattro proprietà proprie oltre alle tre del task di
elettrostatica, quindi il materiale dell'entità deve contenerle tutte:

| Proprietà | Valore per il rame | Richiesta da |
|---|---|---|
| Relative permittivity | `2.8` | elettrostatica |
| Electrical conductivity | `5.882e7` `S/m` | elettrostatica |
| Density | `8960` `kg/m^3` | entrambi |
| Thermal conductivity | `401` `W/(m*K)` | termico |
| Heat capacity | `385` `J/(kg*K)` | termico |
| Emissivity | `0` | termico |

Il materiale *Copper* fornito le contiene già. Un'entità a cui manca una
proprietà richiesta da **uno dei due** task viene saltata da quel task.

### Passo 3 - mantenere le condizioni al contorno elettriche

Le due condizioni **Electric potential** del tutorial 3 restano invariate. Abbassa
il potenziale applicato a un valore meno violento - `0.01` V ai capi della barra
producono comunque una corrente elevata e mantengono la sorgente di calore in un
intervallo in cui la risposta termica è leggibile.

### Passo 4 - aggiungere le condizioni al contorno termiche

La metà termica del modello ha bisogno della propria via di uscita, esattamente
come se fosse da sola:

1. Seleziona l'entità superficie che copre i lati della barra.
2. Apri la scheda `Condizioni al contorno` e attiva **Simple convection**.
3. Imposta **Convection coefficient** = `10` `W/(m^2*K)` e **Fluid temperature**
   = `293.15` K.

Senza una superficie convettiva o una temperatura prescritta il modello termico
ha una sorgente e nessuna uscita, e lo stato stazionario non esiste. Nota che una
faccia usata per la convezione non richiede spessore per il solutore termico, ma
una faccia di spessore nullo non porta nemmeno un campo elettrico - mantieni le
condizioni elettriche sulle facce terminali e la convezione sui lati, e le due non
interferiscono.

### Passo 5 - risolvere

`Soluzione` -> `Avvia solutore` (`Ctrl+R`). Il log mostra prima il task di
elettrostatica, con le statistiche delle sue sei variabili, e poi il task termico
con le statistiche di temperatura, flusso termico e coefficiente di scambio
termico.

### Passo 6 - esaminare i risultati

- Scheda `Risultati`: visualizza **Joule heat**. È il ponte tra le due fisiche -
  ovunque sia diverso da zero, il solutore termico ha visto una sorgente.
- Visualizza **Temperature**. La barra dovrebbe essere più calda dove il calore
  Joule è maggiore e scendere verso la temperatura del fluido sulle facce
  convettive.
- Visualizza **Electric potential** per confermare che la metà elettrica del
  modello è ancora il campo lineare del tutorial 3.

### Passo 7 - verificare il bilancio energetico

Il calore Joule è una densità di dissipazione in `W/m^3`, quindi la potenza totale
che la metà elettrica del modello cede a quella termica è tale densità per il
volume su cui agisce. Per la barra uniforme dovrebbe corrispondere al
`P = U^2/R` del tutorial 3, con `R = L/(sigma*A)`.

Allo stato stazionario la superficie convettiva deve smaltire esattamente quella
potenza, il che dà la stessa verifica globale descritta nel manuale termico:

```
P = h * A_s * ( Ts_mean - Tf )
```

Leggi la temperatura superficiale media dalle statistiche nel log del solutore e
conferma che il bilancio torna. È l'unica verifica che abbraccia entrambe le
fisiche contemporaneamente: rileva una conducibilità sbagliata, un coefficiente
di convezione sbagliato e un'entità rimasta silenziosamente non risolta in una
delle due metà.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La temperatura è uniforme a `293.15` K | il calore Joule è nullo - controlla la conducibilità elettrica e che il task di elettrostatica venga eseguito per primo |
| La soluzione termica non converge | la metà termica non ha né una temperatura prescritta né una superficie convettiva |
| Il riquadro *Solutore temporale* è scomparso | previsto - un task di elettrostatica rende stazionario l'intero flusso, vedi 1.10 |
| L'aumento di temperatura cambia raffinando la mesh | non previsto - ogni risultato elettrostatico è una densità, quindi controlla la metà termica del modello e la qualità della mesh |
| Compaiono solo i risultati dell'elettrostatica | al task termico manca una delle sue quattro proprietà del materiale richieste |

---

## 5. Verifica di un modello

Il solutore non ha una suite di verifica integrata, quindi conviene validare un
nuovo modello rispetto a qualcosa che si può calcolare a mano prima di fidarsene.
Le verifiche seguenti sono scritte in termini del **potenziale**, che è sia
l'incognita principale sia la grandezza in cui è espressa ogni soluzione in forma
chiusa; i risultati di campo elettrico e densità di corrente dovrebbero
concordare con i gradienti che ne derivano.

**Barra uniforme.** Per un corpo di lunghezza `L` mantenuto a `V1` e `V2` su due
facce opposte, con un solo materiale e senza carica, il potenziale è lineare lungo
la barra e

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Colloca un punto di monitoraggio sul piano medio e conferma che indica la media
dei due valori prescritti, e un altro a un quarto della lunghezza per il valore a
un quarto. Ciò rileva una mesh scollegata, una condizione assegnata all'entità
sbagliata e una permittività nulla. Raffina la mesh una volta e conferma che i
valori restano invariati.

**Carica spaziale uniforme.** La barra sopra mette alla prova solo la rigidezza.
Per verificare anche il termine sorgente, prendi lo stesso corpo, mantieni
**entrambe** le facce terminali a `0` V e assegna al volume una *Charge density*
uniforme `rho`. Il potenziale è allora una parabola, con `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`at the mid-plane` = sul piano medio.) Un punto di monitoraggio sul piano medio
dà direttamente `Vmax`. Si verificano due cose insieme: il modulo, che conferma
che permittività e densità di carica vengono lette come previsto, e il **segno** -
una densità di carica positiva deve produrre una gobba di potenziale positiva tra
due facce a massa. Ripeti con la carica su un'entità superficie o linea, e su un
punto, e conferma che la polarità non cambia con il tipo di entità.

**Resistività.** Ogni elemento percorso da corrente deve memorizzare una
resistività elettrica esattamente pari a `1/sigma`. Leggila dalle statistiche nel
log del solutore e confrontala con la conducibilità del materiale. È esatta
anziché approssimata, indipendente dalla mesh, ed è la conferma più rapida che il
materiale abbia raggiunto gli elementi previsti.

**Campo coassiale.** Per una corona circolare tra i raggi `a` e `b` con la
superficie interna a `V1` e quella esterna a `V2`, il potenziale è logaritmico:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Modella la corona, prescrivi entrambe le superfici e confronta i punti di
monitoraggio a qualche raggio. Ciò valida il solutore su un campo non uniforme,
cosa che la barra non può fare, e mostra se la mesh è abbastanza fine vicino al
conduttore interno.

**Dielettrici in serie.** Per due strati di spessore `d1`, `d2` e permittività
relativa `er1`, `er2` sovrapposti tra due potenziali prescritti, il potenziale
all'interfaccia è

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Confrontalo con il potenziale all'interfaccia calcolato. Ciò conferma
direttamente quanto descritto nella sezione 1.9: la ripartizione segue le
**permittività**. Eseguire lo stesso modello con due conducibilità diverse e la
stessa permittività, e vedere che il potenziale all'interfaccia non si sposta, è
la dimostrazione che questo è un solutore elettrostatico e non di conduzione.

**Resistenza e dissipazione.** I risultati di elemento sono densità, quindi un
totale - una corrente, una resistenza, una potenza - deve essere ricavato dal
potenziale. Per la barra uniforme:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Usali ogni volta che un totale deve uscire dal modello - per dimensionare un
conduttore, o per un controllo di plausibilità rispetto a una misura. `P` è anche
ciò a cui deve integrarsi il risultato **Joule heat** sul volume della barra, che
è la verifica che le metà elettrica e termica di un'esecuzione accoppiata
concordino.

Per ognuna di queste verifiche, raffina la mesh almeno una volta e conferma che
il potenziale converge anziché variare.

---

## 6. Limitazioni

### Limitazioni di modellazione

- **Elettrostatica, non conduzione di corrente.** La rigidezza è costruita dalla
  permittività; la conducibilità non entra mai nel sistema di equazioni. Il
  potenziale in un corpo di due materiali si ripartisce quindi in proporzione
  alle permittività, non alle resistenze. Un conduttore di un solo materiale è
  modellato correttamente; una rete di resistenze no - vedi 1.9.
- **Solo statica.** Non c'è solutore temporale, né capacità né induttanza, quindi
  transitori di carica, campi alternati, corrente di spostamento, correnti
  parassite ed effetto pelle sono tutti esclusi dal modello. Aggiungere un task di
  elettrostatica a un flusso di task rimuove inoltre il solutore temporale
  dall'intero flusso, quindi un task termico concatenato dopo di esso può essere
  solo stazionario - vedi 1.10.
- **Solo dielettrici lineari e isotropi.** La permittività e la conducibilità di
  un elemento sono scalari e costanti durante una soluzione. Materiali
  anisotropi, ferroelettrici e dipendenti dal campo non sono modellati, e non c'è
  scarica dielettrica.
- **Solo due condizioni al contorno.** Un potenziale prescritto e una densità di
  carica sono l'intero vocabolario. Non esistono carica superficiale prescritta,
  flusso normale prescritto, condizione di simmetria, né trattamento di contorni
  aperti o elementi infiniti - un problema di campo illimitato deve essere
  troncato meshando abbastanza aria circostante e prescrivendo un potenziale sul
  suo contorno esterno.
- **Nessuna ridistribuzione della carica libera.** La carica non si sposta sulla
  superficie di un conduttore; il solutore non ha alcuna nozione di conduttore
  oltre alle proprietà del materiale assegnategli. Modella un conduttore
  prescrivendone il potenziale, non assegnandogli una conducibilità elevata.
- **Nessuna retroazione della corrente.** La densità di corrente viene
  post-elaborata dal campo, quindi una conducibilità che varia con il campo, o con
  la temperatura prodotta dalla dissipazione, non ha effetto all'interno di una
  singola soluzione.
- **Le superfici sono equipotenziali attraverso lo spessore.** Un'entità
  superficie porta il campo solo nel proprio piano. Una superficie di spessore
  nullo o una linea di area della sezione nulla non ha alcuna rigidezza e viene
  saltata sia nell'assemblaggio sia nella post-elaborazione.
- **Gli elementi punto non conducono.** Un punto porta una sorgente di carica e
  può mantenere un potenziale prescritto, niente di più, e non viene mai
  post-elaborato. Un punto il cui nodo non appartiene a nessun altro elemento
  calcolabile lascia una riga vuota nella matrice.
- **La dipendenza del materiale dalla temperatura non viene letta.** Le tabelle
  vengono valutate a `293.15 K` a meno che un task termico precedente nel flusso
  non abbia prodotto le temperature degli elementi; la condizione ambientale
  *Temperature* non ha effetto su questo solutore.
- **Un modello richiede un potenziale prescritto.** Senza di esso la matrice ha
  uno spazio nullo e la soluzione non converge.
