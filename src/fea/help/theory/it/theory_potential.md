# Flusso attraverso mezzi porosi - Manuale teorico e guida utente

Questo documento descrive il solutore di flusso attraverso mezzi porosi
(`RSolverPotential`) di Range FEA, che serve il tipo di problema **Flusso
attraverso mezzi porosi** (*Flow through porous media*): le equazioni che risolve,
il significato di ogni dato che accetta, le parti dell'interfaccia grafica che lo
comandano e due tutorial svolti.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - flusso in pressione attraverso un filtro a sabbia](#3-tutorial---flusso-in-pressione-attraverso-un-filtro-a-sabbia)
4. [Tutorial - filtrazione ascendente in una colonna verticale](#4-tutorial---filtrazione-ascendente-in-una-colonna-verticale)
5. [Verifica di un modello](#5-verifica-di-un-modello)
6. [Limitazioni](#6-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - il campo di pressione

L'incognita principale è la **pressione** `p` del fluido interstiziale in ogni
nodo, in pascal. Ogni nodo porta un solo grado di libertà, quindi una mesh con `N`
nodi dà un sistema di `N` equazioni - economico quanto un modello termico o
elettrostatico della stessa mesh. La velocità del fluido viene ricavata dal campo
di pressione dopo la soluzione.

Un campo di pressione governato da questa equazione è definito solo **a meno di
una costante additiva**: sommare lo stesso valore alla pressione ovunque non cambia
alcuna velocità. Ogni modello ha quindi bisogno di almeno una condizione al
contorno *Pressure (explicit)*, vedi 1.7. Che la pressione sia intesa come assoluta
o relativa è lasciato all'utente - il solutore lavora solo con il suo gradiente.

### 1.2 Equazioni di governo

Il solutore implementa la **legge di Darcy** per il flusso lento di un unico fluido
incomprimibile che riempie i pori di un corpo poroso rigido:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

insieme alla conservazione della massa di un fluido incomprimibile

```
div( v ) = 0
```

che combinate danno un'equazione di potenziale (di tipo Laplace) per la pressione:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `p` | pressione del fluido interstiziale | `Pa` | risolta |
| `v` | velocità di Darcy (velocità di filtrazione) | `m/s` | ricavata, vedi 1.8 |
| `k` | permeabilità intrinseca del corpo poroso | `m^2` | proprietà del materiale *Permeability to fluids* |
| `mu` | viscosità dinamica del fluido interstiziale | `kg/(m*s)` | proprietà del materiale *Dynamic viscosity* |
| `rho` | densità del fluido interstiziale | `kg/m^3` | proprietà del materiale *Density* |
| `g` | accelerazione di gravità | `m/s^2` | condizione ambientale, vedi 1.6 |

Il rapporto `k/mu` svolge il ruolo che la conducibilità svolge in un modello
termico. Senza gravità l'equazione è l'equazione di Laplace per la pressione e il
flusso segue direttamente il gradiente di pressione. Con la gravità, il termine
`rho*g` fa sì che un fluido in quiete sostenga un gradiente di pressione
idrostatico: `grad(p) = rho*g` dà `v = 0`.

**La velocità è la velocità di Darcy.** È la portata volumetrica per unità di
sezione **totale**, solido e pori insieme - non la velocità del fluido all'interno
dei pori. La velocità media nei pori è maggiore dell'inverso della porosità `n`:

```
v_pore = v / n
```

La porosità non entra in un flusso stazionario, quindi il solutore non ne ha
bisogno e non la accetta. Usatela a mano per stimare i tempi di transito.

**Permeabilità e conducibilità idraulica.** La geotecnica e l'idrogeologia
riportano spesso la *conducibilità idraulica* (coefficiente di permeabilità) `K` in
`m/s` - il coefficiente della legge di Darcy scritta per il carico idraulico - al
posto della permeabilità intrinseca. Le due sono legate dalle proprietà del
fluido:

```
k = K * mu / ( rho * |g| )
```

Per l'acqua a 20 °C, `K = 1e-4 m/s` corrisponde a `k = 1.02e-11 m^2`. L'ingegneria
petrolifera riporta la permeabilità in darcy: `1 D = 9.869e-13 m^2`.

**Proprietà del materiale richieste.** Un problema di flusso attraverso mezzi
porosi ne richiede **tre**:

| Proprietà | Unità | Usata per |
|---|---|---|
| Permeability to fluids | `m^2` | la matrice di rigidezza - permeabilità del solido |
| Dynamic viscosity | `kg/(m*s)` | la matrice di rigidezza - viscosità del **fluido** |
| Density | `kg/m^3` | il termine gravitazionale - densità del **fluido** |

**Densità e viscosità sono quelle del fluido interstiziale, non del corpo poroso.**
Un materiale per questo tipo di problema descrive quindi il mezzo *saturo*: la
permeabilità dello scheletro solido e la densità e la viscosità del fluido che lo
riempie. I materiali forniti con nome *... (water saturated)* sono costruiti così
(vedi 2.3). Un materiale che riporta la densità apparente di un terreno secco
falsa la pressione idrostatica nel rapporto delle due densità.

Un'entità il cui materiale manca di una delle tre **non viene risolta**. Il
controllo della configurazione lo segnala prima dell'avvio del calcolo. L'unica
eccezione è un'entità con una condizione al contorno di questo tipo di problema:
viene inclusa nel sistema qualunque sia il suo materiale, e la sua permeabilità è
allora semplicemente nulla. È così che una faccia di contorno senza materiale
riceve una portata in ingresso.

Ogni proprietà è una tabella in funzione della temperatura. Un task di flusso
attraverso mezzi porosi da solo non ha un campo di temperatura, quindi ogni tabella
viene valutata a `293.15 K`. Le tabelle vengono rispettate solo quando un task
*Heat transfer* posto prima nello stesso flusso dei task ha calcolato le
temperature degli elementi - così un fluido più caldo e meno viscoso scorre più
velocemente.

### 1.3 Discretizzazione agli elementi finiti

La forma debole dell'equazione della pressione dà, per ogni elemento,

```
K = INT( B^T * ( k / mu ) * B )                  rigidezza
f = INT( B^T * ( k / mu ) * rho * g )            carico gravitazionale
  + INT_S( N^T * v_in )                          ingresso attraverso il contorno
```

dove `B` è costruita dalle derivate delle funzioni di forma, `N` sono le funzioni
di forma e `v_in` è la velocità di ingresso imposta su una superficie di contorno.
Ogni matrice di elemento è `n x n` per un elemento con `n` nodi. Non c'è matrice di
massa né di immagazzinamento: il flusso è stazionario, vedi 1.10.

**Misure degli elementi.** Come altrove in Range FEA, le entità di dimensione
inferiore ricevono una misura geometrica per poter rappresentare corpi reali:

| Entità | Misura | Dove si imposta |
|---|---|---|
| Volume | 1 | - |
| Superficie | spessore della superficie | proprietà geometriche dell'entità |
| Linea | sezione della linea | proprietà geometriche dell'entità |
| Punto | - | i punti non conducono flusso |

Una **superficie** con spessore conduce il flusso nel proprio piano, ed è così che
si costruisce un modello bidimensionale (una sezione di una diga o di un
acquifero). Il suo carico gravitazionale usa la componente di `g` che giace nel
piano della superficie. Una superficie di **spessore nullo** - la normale faccia di
contorno di un modello volumetrico - non ha rigidezza e riceve soltanto una portata
in ingresso.

Una **linea** con sezione conduce il flusso lungo la propria direzione, per un
modello monodimensionale di una colonna o di un tubo riempito. Un **punto** non
contribuisce in nulla. Un elemento punto il cui nodo non appartiene a nessun altro
elemento calcolabile lascia una riga vuota nella matrice - non assegnate un
materiale poroso alle entità punto.

### 1.4 Condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Pressure (explicit) | esplicita (Dirichlet) | superficie | Pressure `[Pa]` |
| Velocity (inflow) | naturale (flusso imposto) | superficie | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | naturale (flusso imposto) | superficie | Volumetric flow rate `[m^3/s]` |

Una superficie di contorno senza nessuna di esse è **impermeabile**: nessun fluido
la attraversa. È la condizione naturale dell'equazione, quindi pareti, il fondo di
un acquifero e piani di simmetria non richiedono alcuna condizione.

**Pressure (explicit)** è l'unica condizione esplicita. Ogni nodo della superficie
viene rimosso dal sistema di equazioni e mantenuto alla pressione data. È dove il
fluido lascia il modello (un'uscita, un dreno, l'acqua libera di un serbatoio) o vi
entra a pressione nota. Ogni modello ne richiede almeno una. Una condizione appena
assegnata parte da `1000 Pa`.

**Velocity (inflow)** impone la velocità di Darcy normale attraverso la superficie.
Un valore **positivo** è un ingresso nel corpo poroso, un valore **negativo** un'uscita
(prelievo). Il valore è un flusso per unità di area, quindi la portata totale cresce
con l'area della superficie.

**Volumetric flow rate (inflow)** impone la portata **totale** attraverso la
superficie in `m^3/s`. Il solutore la divide per l'area dell'intera entità e applica
il risultato come velocità di ingresso uniforme, così la portata resta invariata
quando la superficie viene rimeshata o ingrandita. Positivo è di nuovo ingresso.

Nessuna delle condizioni di ingresso fissa la pressione: la pressione all'ingresso
fa parte della risposta. Dove una superficie di ingresso e una superficie con
*Pressure (explicit)* condividono nodi, in quei nodi vince la pressione.

Una superficie di ingresso **non richiede materiale né spessore**: la condizione da
sola la inserisce nel sistema. Se una superficie ha un materiale poroso e uno
spessore, conduce inoltre flusso nel proprio piano.

La condizione *Pressure (implicit)* del solutore di flusso incomprimibile non è
offerta per questo tipo di problema.

Tutte le componenti accettano una tabella in funzione del tempo, che ha effetto con
il solutore temporale attivato, vedi 1.10.

### 1.5 Gravità

La gravità entra attraverso la condizione ambientale *Gravitational acceleration*.
**Ogni entità la possiede per impostazione predefinita con
`g = (0, 0, -9.80665) m/s^2`**, quindi la gravità è diretta lungo l'asse Z negativo
se non viene modificata. Due conseguenze vanno conosciute:

- In un modello con l'asse Z rivolto verso l'alto, un fluido in quiete riceve
  automaticamente una pressione idrostatica che cresce di `rho*|g|` per metro di
  profondità. Di solito è ciò che si desidera.
- Un modello costruito con un altro asse verso l'alto deve avere la condizione
  modificata su ogni entità porosa, altrimenti il fluido «cade» di lato. Ponete a
  zero tutte e tre le componenti per disattivare la gravità - per esempio in un
  modello orizzontale in cui interessa solo la differenza di pressione.

La gravità combinata con una superficie *Pressure (explicit)* non orizzontale
richiede attenzione: una pressione uniforme su una faccia verticale **non** è
idrostatica, quindi il fluido circola vicino a quella faccia. Assegnate la pressione
in funzione dell'altezza (dividete la faccia in più entità) oppure disattivate la
gravità quando non fa parte della domanda.

### 1.6 Condizioni ambientali, iniziali e di accoppiamento

#### Condizioni ambientali

| Condizione ambientale | Componenti | Effetto |
|---|---|---|
| Gravitational acceleration | g in X, Y, Z `[m/s^2]` | termine gravitazionale della legge di Darcy, vedi 1.5 |
| Temperature | Temperature `[K]` | offerta, ma non letta - vedi sotto |

La condizione ambientale *Temperature* è offerta perché le proprietà dei materiali
sono tabelle in funzione della temperatura. Il solutore **non** la legge: prende le
temperature degli elementi solo dai dati condivisi lasciati da un task
*Heat transfer*, e altrimenti valuta ogni tabella a `293.15 K`.

#### Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Pressure | offerta, non letta |
| Velocity | offerta, non letta |

Il flusso è stazionario, quindi la risposta non dipende da un campo iniziale e il
solutore non legge nessuna delle due condizioni iniziali. Entrambe sono offerte
perché il tipo di problema le condivide con il problema di flusso incomprimibile.

#### Accoppiamento con altri solutori

| Direzione | Variabile | Effetto |
|---|---|---|
| Termico -> flusso attraverso mezzi porosi | Temperature | seleziona la riga di ogni tabella di materiale dipendente dalla temperatura |

La pressione e la velocità non sono usate da nessun altro solutore. Il flusso
attraverso mezzi porosi si combina con ogni altra fisica tranne *Incompressible
viscous flow* e *Modal analysis*.

Il solutore si dichiara convergente incondizionatamente, quindi un gruppo di task
che contiene solo un task di flusso attraverso mezzi porosi si ferma sempre dopo
un'iterazione.

### 1.7 Soluzione del sistema

Il solutore assembla e risolve

```
K * p = f
```

`K` è simmetrica definita positiva non appena è imposta almeno una pressione, quindi
viene risolta con il metodo del **gradiente coniugato** con precondizionamento di
Jacobi. L'assemblaggio è parallelizzato sugli elementi con buffer per thread uniti
una volta alla fine, così il risultato non dipende dal numero di thread.

Le permeabilità dei materiali reali vanno da circa `1e-9 m^2` (ghiaia) fino a
`1e-20 m^2` (argilla intatta), il che porterebbe i coefficienti della matrice vicino
ai limiti di arrotondamento del solutore iterativo. Prima della soluzione, ogni
equazione viene quindi divisa per il massimo `k/mu` del modello. Dividere tutte le
equazioni per lo stesso numero non cambia la soluzione; mantiene solo l'iterazione
ben condizionata.

Un modello **senza condizione al contorno Pressure (explicit)** non ha soluzione
unica. Il solutore lo rileva e si ferma con l'errore *Flow through porous media
requires at least one 'Pressure (explicit)' boundary condition*.

Altri modi per arrivare a un sistema singolare o vuoto, tutti silenziosi:

- un'entità il cui materiale manca di una delle tre proprietà richieste non viene
  risolta e i suoi nodi escono dal sistema;
- un'entità con permeabilità o viscosità nulla non ha rigidezza, quindi i suoi nodi
  restano nel sistema ma non sono collegati a nulla;
- una superficie di spessore nullo o una linea di sezione nulla non conduce flusso.

### 1.8 Risultati derivati

Dopo la soluzione la velocità di ogni elemento viene ricavata dal campo nodale di
pressione. Il gradiente di pressione viene mediato sui punti di integrazione
dell'elemento e combinato con il termine gravitazionale; per gli elementi linea e
superficie viene ruotato nelle coordinate globali:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Risultato | Applicato a | Significato |
|---|---|---|
| Pressure `[Pa]` | nodo | il campo di pressione interstiziale risolto |
| Velocity `[m/s]` | elemento | vettore velocità di Darcy |

La velocità è una **densità** nello stesso senso dei risultati di campo degli altri
solutori: non dipende dalla dimensione dell'elemento, è costante in un elemento
lineare e converge raffinando la mesh. Le facce di contorno di spessore nullo
mostrano velocità nulla - visualizzate il risultato sul volume.

La portata totale non viene salvata come risultato. Calcolatela a mano dalla
velocità e dall'area di una sezione, oppure dalle condizioni di ingresso, vedi 5.

Dopo la scrittura del record, il log del solutore stampa le statistiche di
entrambe le variabili e i valori nei punti di monitoraggio.

### 1.9 Validità della legge di Darcy

La legge di Darcy vale per un flusso lento e laminare, in cui le forze viscose
dominano sull'inerzia del fluido. Un criterio comune usa il numero di Reynolds
formato con il diametro medio dei grani `d`:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

Il flusso delle acque sotterranee nei terreni, la filtrazione attraverso dighe e
argini, il flusso attraverso filtri, letti impaccati e la maggior parte dei
materiali porosi tecnici lo soddisfano con ampio margine. Il flusso attraverso
ghiaia grossa, scogliere o schiume molto aperte ad alte velocità no; la legge di
Darcy sottostima allora la perdita di carico (regime di Forchheimer) e questo
solutore fornirà una velocità troppo grande.

### 1.10 Flusso stazionario e solutore temporale

Il tipo di problema consente il solutore temporale, ma il flusso stesso è
**stazionario**: il fluido e il corpo poroso sono incomprimibili e non c'è un
termine di immagazzinamento. Con il solutore temporale attivato, ogni passo
temporale risolve uno stato stazionario separato con i valori delle condizioni in
quell'istante. È il modello giusto quando le condizioni al contorno cambiano
lentamente rispetto al tempo necessario alla pressione per assestarsi - il livello
di un serbatoio che varia nell'arco di ore sopra un filtro a sabbia - e permette a
un task di flusso attraverso mezzi porosi di condividere il flusso dei task con un
task termico transitorio.

La consolidazione, il drenaggio di un terreno non saturo, il flusso di gas
comprimibile e ogni altro fenomeno in cui la quantità di fluido immagazzinata nei
pori varia nel tempo sono fuori dal modello, vedi la sezione 6.

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Condizioni ambientali`,
`Materiale`, `Risultati`), nel menu **Problema** e nel pannello **Modello**.

### 2.1 Scelta del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del flusso
dei task. Aggiungete un task e scegliete **Flow through porous media**.

Il tipo di problema non può essere combinato con *Incompressible viscous flow* né
con *Modal analysis*. Un task *Heat transfer* posto prima di esso fa seguire alle
tabelle del materiale la temperatura calcolata.

### 2.2 Scheda Problema

La scheda mostra il gruppo *Solutore temporale*, perché il tipo di problema
supporta il solutore temporale - cosa significhi qui un calcolo transitorio lo
spiega 1.10. Il flusso attraverso mezzi porosi non ha un gruppo di impostazioni
proprio. Tutto si assegna per entità nelle schede delle condizioni.

### 2.3 Scheda Materiale

La scheda `Materiale` mostra il materiale assegnato all'entità selezionata. Il
flusso attraverso mezzi porosi richiede **tutte e tre** le proprietà seguenti:

| Proprietà | Unità | Commento |
|---|---|---|
| Permeability to fluids | `m^2` | permeabilità intrinseca del solido, vedi la tabella sotto |
| Dynamic viscosity | `kg/(m*s)` | del fluido interstiziale, `1.003e-3` per l'acqua a 20 °C |
| Density | `kg/m^3` | del fluido interstiziale, `998.2` per l'acqua a 20 °C |

La libreria fornita contiene materiali porosi saturi d'acqua pronti all'uso.
Ognuno porta la densità e la viscosità dell'acqua dipendenti dalla temperatura e
una permeabilità tipica:

| Materiale | Permeabilità `[m^2]` | Conducibilità idraulica a 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

La permeabilità di un terreno reale può differire da questi valori di un ordine di
grandezza o più - sono punti di partenza, non dati di un sito specifico. Per
modellare un altro fluido, copiatene uno e sostituite densità e viscosità. Il
materiale semplice *Sand* è un materiale granulare secco senza viscosità, quindi è
contrassegnato come non valido per questo tipo di problema.

### 2.4 Scheda Condizioni al contorno

La scheda è divisa in un elenco delle condizioni al contorno disponibili in alto e
un editor della condizione selezionata in basso. Selezionate prima un'entità
superficie nell'albero `Modello`; l'elenco offre allora le condizioni della
sezione 1.4. Spuntatene una per assegnarla, poi modificatene le componenti
nell'albero inferiore.

- **Pressure (explicit)** mantiene la pressione all'uscita. Un modello senza
  nessuna non si risolve.
- **Velocity (inflow)** e **Volumetric flow rate (inflow)** immettono una portata
  nota nel modello. I valori positivi entrano, quelli negativi escono.
- Le facce senza condizione sono impermeabili.

### 2.5 Condizioni iniziali e ambientali

La scheda `Condizioni ambientali` offre *Gravitational acceleration*, che ogni
entità possiede già con `(0, 0, -9.80665) m/s^2` (vedi 1.5), e *Temperature*, che
questo solutore ignora.

La scheda `Condizioni iniziali` offre *Pressure* e *Velocity*, che questo solutore
non legge.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i solutori
iterativi. Il solutore di flusso attraverso mezzi porosi usa la voce **CG**. Se la
soluzione non converge, cercate prima un'entità con permeabilità nulla o un
elemento punto isolato anziché nelle impostazioni del solutore.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` posiziona sonde in coordinate date
e sceglie la variabile da registrare - *Pressure* o *Velocity*. `Report` ->
`Punti di monitoraggio` mostra i valori.

### 2.8 Risultati e record

La scheda `Risultati` elenca le variabili calcolate e controlla la vista 3D. La
pressione è uno scalare nodale, la velocità un vettore di elemento. Visualizzate la
velocità come campo vettoriale sul volume per vedere i percorsi del flusso.

La scheda `Record` del pannello `Modello` elenca i record dei risultati: uno per un
calcolo stazionario, uno per ogni passo temporale di output con il solutore
temporale attivato.

`Report` -> `File di log del solutore` mostra l'output completo del solutore,
incluse le statistiche di entrambe le variabili.

---

## 3. Tutorial - flusso in pressione attraverso un filtro a sabbia

**Obiettivo.** Calcolare il flusso d'acqua attraverso un filtro a sabbia
orizzontale spinto da una differenza di pressione nota e verificare la portata con
la legge di Darcy.

Questo tutorial presuppone un modello volumetrico con mesh di un parallelepipedo -
usate `File` -> `Nuovo modello`, disegnate un parallelepipedo con `Geometria` ->
`Crea elemento` oppure importate una geometria e generate una mesh di tetraedri; il
tutorial *Draw cube* tratta questo argomento. Marcate le due facce di estremità come
entità superficie separate per poter assegnare loro delle condizioni.

Prendete il filtro lungo `1 m` lungo l'asse X con una sezione di `0.1 m` per
`0.1 m`.

### Passo 1 - scegliere il problema

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungete un task e selezionate **Flow through porous media**.
3. Confermate con `OK`.

Lasciate disattivato il *Solutore temporale* - la risposta è un unico stato
stazionario.

### Passo 2 - assegnare il materiale

1. Selezionate l'entità volume nell'albero `Modello`.
2. Aprite la scheda `Materiale` e assegnate **Coarse Sand (water saturated)**.

Il materiale porta la permeabilità `1e-10 m^2` della sabbia e la densità e la
viscosità dell'acqua.

### Passo 3 - disattivare la gravità

Il filtro è orizzontale e interessa solo la differenza di pressione, quindi
rimuovete la gravità predefinita:

1. Con l'entità volume selezionata, aprite la scheda `Condizioni ambientali`.
2. Selezionate **Gravitational acceleration** e ponete tutte e tre le componenti a
   `0`.

Con la gravità attiva, l'acqua sosterrebbe anche una pressione idrostatica sui
`0.1 m` di altezza del filtro, e le pressioni uniformi sulle facce di estremità
verticali la farebbero circolare vicino a esse - vedi 1.5.

### Passo 4 - imporre le due estremità

1. Selezionate l'entità superficie in `x = 0`.
2. Aprite la scheda `Condizioni al contorno`, spuntate **Pressure (explicit)** e
   impostate `10000` Pa - circa un metro di colonna d'acqua.
3. Selezionate l'entità superficie in `x = 1 m`, spuntate **Pressure (explicit)** e
   impostate `0` Pa.

Le quattro facce laterali non ricevono condizioni: sono pareti impermeabili.

### Passo 5 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Seguite l'avanzamento in `Report` -> `File di log del solutore`.

Il calcolo è un'unica soluzione con gradiente coniugato.

### Passo 6 - esaminare i risultati

- Scheda `Risultati`: visualizzate **Pressure**. Scende linearmente da `10000` Pa a
  `0` Pa lungo il filtro, con isobare piane perpendicolari al suo asse.
- Visualizzate **Velocity**. È diretta lungo +X ed è uguale in ogni elemento.
- Un punto di monitoraggio su *Pressure* a metà filtro indica `5000` Pa.

### Passo 7 - rifare i conti a mano

Con `k = 1e-10 m^2` e `mu = 1.003e-3 kg/(m*s)` per l'acqua a 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

Il risultato **Velocity** deve mostrare `9.97e-4 m/s`. Il numero di Reynolds con
grani di `1 mm` è `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, al limite del regime di
Darcy (1.9). Con una porosità di `0.35` l'acqua stessa si muove a
`9.97e-4 / 0.35 = 2.8e-3 m/s` e impiega circa sei minuti ad attraversare il filtro.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | nessuna pressione di uscita è imposta |
| *Entity has material assigned which is missing required properties* | al materiale manca la viscosità - spesso un materiale di terreno secco come *Sand* |
| La pressione varia con l'altezza e la velocità ha una componente Z | la gravità predefinita è ancora attiva, vedi il passo 3 |
| La velocità è nulla ovunque | le due estremità hanno la stessa pressione o la permeabilità è nulla |

---

## 4. Tutorial - filtrazione ascendente in una colonna verticale

**Obiettivo.** Spingere acqua verso l'alto attraverso una colonna di sabbia
verticale con una portata nota e vedere come la pressione di ingresso si divide in
una parte idrostatica e una parte di attrito.

Usate un parallelepipedo di base `0.1 m` per `0.1 m` e alto `1 m` lungo l'asse
**Z**, con la faccia inferiore (`z = 0`) e la faccia superiore (`z = 1 m`) come
entità superficie separate.

### Passo 1 - problema e materiale

1. Selezionate **Flow through porous media** come nel tutorial 3.
2. Assegnate **Coarse Sand (water saturated)** al volume.

### Passo 2 - mantenere la gravità

La colonna è disposta lungo Z, quindi la gravità predefinita
`(0, 0, -9.80665) m/s^2` è orientata correttamente. Lasciate la condizione
*Gravitational acceleration* com'è.

### Passo 3 - condizioni al contorno

1. Selezionate la faccia superiore, spuntate **Pressure (explicit)** e impostate
   `0` Pa: l'acqua libera sopra la colonna.
2. Selezionate la faccia inferiore, spuntate **Volumetric flow rate (inflow)** e
   impostate `1e-6` m^3/s. Sui `0.01 m^2` della faccia corrisponde a una velocità
   di ingresso di `1e-4 m/s`.

La faccia superiore è orizzontale, quindi una pressione uniforme su di essa è
coerente con la gravità.

### Passo 4 - risolvere ed esaminare

`Soluzione` -> `Avvia solutore` (`Ctrl+R`), poi visualizzate **Pressure** e
**Velocity**.

- La velocità è `1e-4 m/s` verso l'alto (+Z) in ogni elemento.
- La pressione cresce linearmente da `0` Pa in cima fino al suo massimo sul fondo.

### Passo 5 - scomporre la pressione di ingresso

La pressione sul fondo deve sollevare la colonna d'acqua e spingerla attraverso la
sabbia:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Un punto di monitoraggio sulla faccia inferiore conferma la somma. Ponete la portata
a zero e risolvete di nuovo: la velocità scompare e la pressione sul fondo scende ai
`9789 Pa` puramente idrostatici - il solutore riproduce esattamente un fluido in
quiete.

Aumentate la portata finché la parte di attrito raggiunge il peso alleggerito della
sabbia, circa `(rho_s - rho) * g * (1 - n) * L`: è l'inizio della fluidificazione
(sabbie mobili), che lo scheletro rigido di questo modello non può rappresentare -
il solutore continua semplicemente a calcolare una pressione più grande.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La pressione sul fondo è solo la parte di attrito | la gravità è disattivata sul volume |
| La pressione sul fondo è solo la parte idrostatica | l'ingresso è sulla faccia sbagliata o il suo valore è nullo |
| La velocità è diretta di lato | la colonna è stata costruita lungo un asse diverso da Z; modificate la condizione di gravità |

---

## 5. Verifica di un modello

**Colonna uniforme.** Per un corpo di lunghezza `L` mantenuto a `p1` e `p2` su due
facce opposte, di un solo materiale e con la gravità disattivata, la pressione è
lineare lungo il corpo e

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

Il risultato di velocità deve coincidere entro la tolleranza del solutore, e la
pressione nel piano medio deve essere la media dei due valori imposti.

**Idrostatica.** Un corpo soggetto a gravità, con una condizione *Pressure
(explicit)* solo sulla faccia superiore orizzontale e pareti impermeabili
altrove, deve mostrare velocità nulla e una pressione che cresce di `rho*|g|` per
metro di profondità. Ciò verifica insieme la densità e la condizione di gravità.

**Bilancio di portata.** In un modello stazionario tutto il fluido che entra deve
uscire. Per un ingresso imposto con *Volumetric flow rate (inflow)*, la velocità in
una sezione moltiplicata per la sua area deve restituire la portata imposta.
Leggete la velocità nella colonna con un punto di monitoraggio.

**Strati in serie.** Per due strati di lunghezza `L1`, `L2` e permeabilità `k1`,
`k2` in serie tra due pressioni imposte, il flusso resiste come due resistenze in
serie:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          pressione all'interfaccia
```

A differenza del solutore elettrostatico, questo ripartisce la pressione secondo le
**permeabilità**, come farebbe una rete di resistenze.

**Flusso radiale verso un pozzo.** Per un acquifero confinato di spessore `H` tra un
pozzo di raggio `rw` mantenuto a `pw` e un raggio esterno `R` mantenuto a `pR`, la
pressione è logaritmica e la portata del pozzo è

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Ciò verifica il solutore su un campo non uniforme e mostra se la mesh è abbastanza
fine vicino al pozzo.

Raffinate la mesh almeno una volta e verificate che la pressione converga invece di
derivare.

---

## 6. Limitazioni

- **Flusso stazionario, incomprimibile e monofase.** Non c'è un termine di
  immagazzinamento, quindi la consolidazione, il flusso di gas comprimibile e la
  risposta transitoria di un acquifero confinato non sono modellati. Un calcolo
  transitorio è una sequenza di stati stazionari, vedi 1.10.
- **Solo mezzi completamente saturi.** Non c'è superficie libera (linea freatica) né
  zona non satura. La filtrazione attraverso un argine la cui falda va determinata
  è fuori dal modello, a meno che la regione satura non venga meshata
  esplicitamente.
- **Corpo poroso rigido.** Lo scheletro non si deforma, quindi non c'è accoppiamento
  con l'analisi delle tensioni, né tensione efficace, né consolidazione, né
  fluidificazione.
- **Permeabilità isotropa.** La permeabilità è uno scalare. I terreni stratificati
  con permeabilità orizzontale e verticale diverse vanno modellati come strati
  separati.
- **Solo regime di Darcy.** Il flusso inerziale (Forchheimer) e turbolento in mezzi
  molto grossolani non è modellato, vedi 1.9.
- **Condizioni al contorno solo su superfici.** Pressioni e ingressi si impongono su
  entità superficie; non esistono superficie di trasudamento, pozzo o sorgente
  lineare, né pressione imposta su una linea o un punto.
- **Nessun risultato di portata totale.** Le portate attraverso una sezione vanno
  calcolate a mano dalla velocità.
- **Le condizioni iniziali non vengono lette** e la condizione ambientale
  *Temperature* non ha effetto; le tabelle del materiale seguono la temperatura solo
  tramite un task termico posto prima nel flusso dei task.
- **Un modello richiede una pressione imposta.** Senza di essa il solutore si ferma
  con un errore.
