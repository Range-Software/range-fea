# Analisi delle tensioni - Manuale teorico e guida utente

Questo documento descrive il solutore strutturale (`RSolverStress`) di Range
FEA, che gestisce sia il tipo di problema **Analisi delle tensioni** (*Stress
analysis*) sia il tipo di problema **Analisi modale** (*Modal analysis*): le
equazioni che risolve, il significato di ogni input che accetta, le parti
dell'interfaccia grafica che lo controllano e due tutorial svolti.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - analisi statica delle tensioni](#3-tutorial---analisi-statica-delle-tensioni)
4. [Tutorial - analisi modale](#4-tutorial---analisi-modale)
5. [Verifica di un modello](#5-verifica-di-un-modello)
6. [Limitazioni](#6-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - il campo di spostamento

L'incognita principale è il **vettore spostamento** `u = (ux, uy, uz)` in ogni
nodo, in metri. Ogni nodo ha tre gradi di libertà, quindi una mesh con `N` nodi
dà un sistema di `3N` equazioni. Deformazioni, tensioni e forze di reazione
vengono tutte ricavate dal campo di spostamento dopo la soluzione.

### 1.2 Equazioni di governo

Il solutore implementa l'**elasticità lineare, isotropa, a piccole
deformazioni**. Tre relazioni la definiscono.

**Equilibrio** di un corpo soggetto a un carico di volume `b` e a una trazione
superficiale `t`:

```
div(sigma) + b = 0
```

Relazione **deformazione-spostamento**, piccole deformazioni:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

Relazione **costitutiva** con un termine termico:

```
sigma = D * ( eps - alpha * dT )
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `u` | spostamento | `m` | incognita |
| `sigma` | tensione di Cauchy | `Pa` | ricavata |
| `eps` | deformazione ingegneristica | `-` | ricavata |
| `E` | modulo di elasticità | `Pa` | proprietà del materiale |
| `v` | coefficiente di Poisson | `-` | proprietà del materiale |
| `rho` | densità | `kg/m^3` | proprietà del materiale |
| `alpha` | coefficiente di dilatazione termica | `1/K` | proprietà del materiale |
| `dT` | aumento di temperatura rispetto allo stato privo di tensioni | `K` | vedi 1.5 |

Tutte e quattro le proprietà del materiale sono **richieste** - un'entità a cui
ne manca una non viene risolta. I due coefficienti di smorzamento di Rayleigh di
un'analisi dinamica (vedi 1.7) sono facoltativi.

### 1.3 Matrice di elasticità

Gli **elementi di volume** usano la matrice di elasticità isotropa
tridimensionale completa in notazione di Voigt, con il vettore delle tensioni
ordinato come `[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

Gli **elementi di superficie** usano la forma bidimensionale di **deformazione
piana** con il vettore delle tensioni `[sigma_xx, sigma_yy, tau_xy]`, valutata nel
piano locale dell'elemento e ruotata di nuovo in coordinate globali:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Si tratta di **deformazione** piana, non di tensione piana. Una piastra sottile
caricata nel proprio piano risulterà quindi più rigida di quanto prevedrebbe
un'idealizzazione in tensione piana.

Gli **elementi linea** sono trattati come **aste**: viene assemblata solo la
rigidezza assiale `E * A` lungo la direzione dell'elemento, dove `A` è l'area
della sezione della linea. Non c'è rigidezza flessionale, a taglio o torsionale,
nemmeno per i tipi di elemento chiamati "beam" (trave).

### 1.4 Discretizzazione agli elementi finiti

La forma debole dell'equilibrio dà, per ogni elemento,

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(`stiffness matrix` = matrice di rigidezza, `consistent mass matrix` = matrice di
massa consistente, `load vector, see 1.5` = vettore dei carichi, vedi 1.5), dove
`B` è la matrice deformazione-spostamento costruita dalle derivate delle funzioni
di forma. Ogni matrice di elemento è `3n x 3n` per un elemento con `n` nodi, con
i gradi di libertà intercalati come `[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Misure degli elementi.** Come altrove in Range FEA, alle entità di dimensione
inferiore si assegna una misura geometrica affinché possano rappresentare corpi
reali:

| Entità | Misura | Dove si imposta |
|---|---|---|
| Volume | 1 | - |
| Superficie | spessore della superficie | proprietà geometriche dell'entità |
| Linea | area della sezione della linea | proprietà geometriche dell'entità |
| Punto | volume del punto | proprietà geometriche dell'entità |

Una superficie con spessore nullo o una linea con area della sezione nulla non
contribuisce né con rigidezza né con massa - può comunque portare carichi, che è
esattamente ciò che serve per una faccia caricata di un solido meshato.

La matrice di massa viene assemblata solo quando serve, cioè per un'analisi
dinamica transitoria o per un'analisi modale.

### 1.5 Carichi, condizioni al contorno e ambientali

#### Condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Displacement | esplicita (Dirichlet) | punto, linea, superficie, volume | Displacement in X, Y, Z `[m]` |
| Normal displacement | esplicita, sistema locale | superficie | Displacement `[m]` |
| Roller displacement | esplicita, sistema locale | punto, linea, superficie | Displacement `[m]` |
| Force | naturale | punto, linea, superficie | Force in X, Y, Z `[N]` |
| Force (unit area) | naturale | superficie | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | naturale | superficie | Pressure `[Pa]` |
| Weight | naturale | punto, linea, superficie | Weight `[kg]` |

**Displacement** è una condizione a componenti **facoltative**: ciascuna delle
sue tre componenti può essere attivata o disattivata singolarmente. Una
componente attivata vincola quella direzione globale di ogni nodo dell'entità al
valore dato; una componente disattivata lascia quella direzione completamente
libera. Disattivando *Displacement in Y* e *Displacement in Z* si ottiene quindi
un appoggio che blocca X e lascia scorrere il nodo nel piano YZ.

Usa invece *Roller displacement* quando le direzioni libere non sono quelle
globali ma il piano tangente di una superficie curva.

**Normal displacement** e **Roller displacement** lavorano in un **sistema di
riferimento locale** costruito per ogni nodo vincolato. Per impostazione
predefinita il sistema segue la geometria:

- su una **superficie**, l'asse X locale è la normale media degli elementi di
  superficie adiacenti;
- su una **linea**, il sistema locale è costruito dalla direzione dell'elemento;
- su un **punto** non c'è geometria da seguire, quindi viene sempre usata la
  direzione inserita con la condizione al contorno.

Attivando **Usa la direzione locale inserita** si annulla questo comportamento per
una superficie o una linea, e l'asse X locale viene preso dalla direzione
inserita sotto. Usalo quando la direzione che vuoi vincolare non è quella che la
mesh fornisce - ad esempio un carrello che deve scorrere lungo una superficie
sfaccettata secondo un asse globale.

I vincoli di entità diverse si **combinano** in un nodo che condividono. Il
solutore raccoglie ogni vincolo di spostamento che agisce su un nodo, ciascuno
un'affermazione `d . u = v` su una direzione, e riduce l'insieme ad al più tre
direzioni bloccate mutuamente perpendicolari tramite Gram-Schmidt, applicando le
stesse operazioni ai valori prescritti. Queste direzioni bloccate diventano gli
assi principali del sistema del nodo; ciò che resta completa il sistema e rimane
libero. Un nodo può quindi essere bloccato in una direzione da un
*Displacement* globale, in un'altra da un *Roller displacement* su una faccia
inclinata, e restare libero nella terza - le due condizioni non si sovrascrivono a
vicenda e una componente espressa in modo globale non viene mai reinterpretata
silenziosamente nel sistema locale di un'altra.

Due entità che prescrivono **valori diversi nella stessa direzione** non possono
essere soddisfatte entrambe. Il solutore lo rileva durante l'ortogonalizzazione e
si arresta con un errore che indica il nodo, anziché risolvere il conflitto a
favore della condizione letta per ultima.

Le matrici di elemento e i vettori dei carichi di un nodo vincolato vengono
ruotati in quel sistema, il vincolo viene applicato e lo spostamento risultante
viene ruotato indietro. In quel sistema:

- **Normal displacement** blocca tutte e tre le componenti locali, e lo scalare
  prescritto viene applicato lungo la normale locale. Il nodo si sposta
  esattamente della quantità data lungo la normale ed è fisso per il resto.
- **Roller displacement** blocca solo la componente normale locale su una
  superficie o su un punto, quindi il nodo è libero di scorrere nel piano
  tangente. Su un'entità linea blocca invece le due componenti perpendicolari
  alla linea, quindi il nodo è libero di scorrere lungo la linea.

**Force** è una forza totale in newton distribuita sull'entità a cui è assegnata -
viene divisa per la lunghezza dell'entità per una linea, per l'area dell'entità
per una superficie e per il numero di punti di un'entità punto. Raffinare la mesh
non cambia quindi il carico totale.

**Force (unit area)** è una trazione in `N/m^2` applicata nella direzione globale
data, integrata sull'area dell'elemento.

**Pressure** agisce lungo la **normale** della superficie. Il solutore determina
per ogni superficie di contorno se la sua normale punta verso l'elemento di volume
calcolabile adiacente e ne inverte il segno di conseguenza, così che una
pressione positiva spinga sempre sul corpo. Una superficie che non delimita alcun
elemento di volume non viene mai classificata come rivolta verso l'interno,
quindi lì il carico agisce lungo la normale **negativa** della superficie -
verifica in questo caso la direzione del carico risultante.

**Weight** è una massa in chilogrammi associata all'entità. Viene moltiplicata
per l'accelerazione di gravità della condizione ambientale e distribuita
sull'entità come Force. Usala per masse applicate non modellate.

#### Condizioni ambientali

| Condizione ambientale | Componenti | Effetto |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | peso proprio e la condizione al contorno Weight |
| Temperature | Temperature `[K]` | temperatura di riferimento priva di tensioni |

Il **peso proprio** viene generato automaticamente per ogni entità che abbia sia
una densità sia una misura geometrica, come `rho * g` integrato sull'elemento.
Compare non appena viene assegnata una condizione ambientale di accelerazione di
gravità; il valore predefinito è `-9.80665 m/s^2` in Z.

**Carico termico.** La differenza di temperatura che genera il termine di
dilatazione termica è

```
dT = element temperature - environment temperature
```

(`element temperature` = temperatura dell'elemento, `environment temperature` =
temperatura ambiente.) La **temperatura ambiente** deriva dalla condizione
ambientale *Temperature* ed è lo stato di riferimento privo di tensioni. La
**temperatura dell'elemento** deriva da un solutore termico eseguito prima nello
stesso flusso dei task del problema, consegnata tramite i dati condivisi del
solutore. Eseguire *Heat transfer* e poi *Stress analysis* in un unico flusso dei
task dà quindi un'analisi delle tensioni termiche senza ulteriore
configurazione. Senza un solutore termico la temperatura dell'elemento resta al
suo valore predefinito e il termine termico è la sola differenza rispetto alla
condizione ambientale.

#### Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Displacement | campo di spostamento iniziale di un'analisi dinamica transitoria |
| Structural velocity | campo di velocità iniziale di un'analisi dinamica transitoria |
| Pressure | carico di pressione superficiale iniziale |

Le condizioni iniziali vengono lette solo alla prima esecuzione di un'analisi
transitoria. Un'analisi dinamica senza di esse parte da ferma. Le direzioni
vincolate non mantengono una velocità iniziale propria, seguono lo spostamento
prescritto.

#### Accoppiamento con un solutore del flusso

Il solutore recupera il risultato **Pressure** di una soluzione precedente e lo
usa come carico superficiale ovunque non sia assegnata una condizione al contorno
di pressione esplicita. Un task di flusso seguito da un task di tensioni nello
stesso flusso dei task trasferisce quindi automaticamente la pressione del fluido
sulla struttura.

Nell'altra direzione, ogni solutore non strutturale sposta la mesh del risultato
**Displacement** memorizzato prima di essere eseguito e la riporta indietro dopo,
quindi una soluzione del flusso o termica che segue una soluzione delle tensioni
vede la forma deformata.

### 1.6 Analisi statica

Con il solutore temporale disattivato e il tipo di problema impostato su *Stress
analysis*, il solutore assembla e risolve

```
K * u = f
```

`K` è simmetrica definita positiva una volta che i modi rigidi sono stati rimossi
dai vincoli di spostamento, quindi viene risolta con il metodo del **gradiente
coniugato** con precondizionamento di Jacobi e una dimensione di blocco pari a 3.

Se il modello è labile la matrice è singolare e la soluzione non converge. Ogni
modello statico richiede vincoli sufficienti a sopprimere tutti e sei i modi
rigidi - tre traslazioni e tre rotazioni.

### 1.7 Analisi transitoria

Con il solutore temporale attivo, le *Impostazioni dell'analisi transitoria delle
tensioni* selezionano uno di due tipi di analisi.

#### Analisi dinamica

È quella predefinita. Il solutore integra le equazioni dell'elastodinamica
lineare

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

così vengono riprodotti inerzia, onde di tensione, vibrazioni, amplificazione
dinamica e risonanza.

**Smorzamento.** `C` è uno smorzamento di Rayleigh, costruito elemento per
elemento a partire da due proprietà del materiale facoltative,

```
C = alpha_M * M + beta_K * K
```

*Rayleigh mass damping coefficient* `alpha_M` `[1/s]` smorza le basse frequenze,
*Rayleigh stiffness damping coefficient* `beta_K` `[s]` quelle alte. Il rapporto
di smorzamento di un modo con pulsazione `omega` è

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

quindi un rapporto obiettivo `zeta` a due frequenze `omega_1` e `omega_2` si
ottiene con

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Prendi le due frequenze da un'analisi modale, attorno alla banda che ti
interessa. I rapporti di smorzamento strutturale tipici vanno da 0.01 a 0.05.
Senza le due proprietà non c'è alcuno smorzamento fisico.

**Integrazione temporale.** Il metodo di Newmark con la modifica di
Hilber-Hughes-Taylor (HHT). Con lo spostamento `u`, la velocità `v` e
l'accelerazione `a` all'inizio del passo, ogni passo risolve

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

e poi aggiorna

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

La matrice efficace è simmetrica definita positiva, quindi viene risolta con lo
stesso metodo CG di un'analisi statica.

- Con **alfa HHT = 0** (predefinito) lo schema è il Newmark semplice con
  `gamma = 1/2` e `beta = max(theta/2, 1/4)`, dove `theta` è il coefficiente
  dell'approssimazione dell'avanzamento nel tempo (`1` all'indietro, `0.5`
  centrale, `0` in avanti). Centrale e in avanti danno entrambi lo schema ad
  accelerazione media - incondizionatamente stabile, accurato al secondo ordine e
  privo di smorzamento numerico. All'indietro dà `beta = 1/2`, anch'esso
  incondizionatamente stabile ma che allunga i periodi.
- Con **alfa HHT** tra `-1/3` e `0`, `gamma = 1/2 - alpha` e
  `beta = (1 - alpha)^2 / 4`. Lo schema resta incondizionatamente stabile e
  accurato al secondo ordine, e smorza le alte frequenze che una mesh grossolana
  risolve male lasciando quasi intatti i modi bassi. Da `-0.05` a `-0.1` è una
  scelta comune quando la risposta è disturbata da rumore ad alta frequenza.

**Accelerazione iniziale.** Al primo passo l'accelerazione viene risolta
dall'equilibrio, `M*a = f - K*u - C*v`, quindi un carico applicato all'inizio
produce l'accelerazione iniziale corretta. Le direzioni prive di massa ricevono
zero.

**Massa concentrata.** L'opzione *Matrice di massa concentrata* sostituisce la
massa consistente con una diagonale della stessa massa totale, scalando la
diagonale della matrice consistente (concentrazione HRZ, che resta positiva anche
per gli elementi quadratici). Abbassa leggermente le frequenze naturali e dà una
risposta più regolare dietro i fronti d'onda ripidi.

**Scelta del passo temporale.** Lo schema è stabile per qualsiasi `dt`, ma
l'accuratezza richiede circa 20 passi o più per periodo del modo più alto che
vuoi risolvere. Per un'onda che attraversa la mesh, un passo vicino al tempo che
l'onda impiega ad attraversare un elemento (`dt ~ h/c`, `c = sqrt(E/rho)`) è un
buon punto di partenza.

**Risultati.** Oltre a spostamento e tensione, un'analisi dinamica memorizza i
vettori nodali *Structural velocity* e *Structural acceleration*. Portano inoltre
lo stato nel passo temporale successivo e attraverso un riavvio del solutore.

#### Analisi quasi-statica

Ogni passo temporale è una soluzione statica indipendente

```
K * u(t) = f(t)
```

con inerzia e smorzamento trascurati. Usala per una storia di carico applicata
lentamente rispetto ai periodi strutturali, dove conta solo la sequenza di stati
di equilibrio. Non richiede la densità né ha limiti sul passo temporale.

### 1.8 Analisi modale

Con il tipo di problema impostato su *Modal analysis*, il solutore assembla `K` e
`M` e risolve il problema agli autovalori generalizzato

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(`with` = con.) Ogni autocoppia è un **modo**: `phi` è la forma modale,
memorizzata e visualizzata come campo di spostamento, e `lambda` è l'autovalore
da cui si ricava la frequenza naturale come

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

Il solutore esegue da sé questa conversione: le impostazioni modali e la vista 3D
riportano la frequenza naturale in Hz, mentre il log del solutore stampa accanto
l'autovalore sottostante. Un modo che l'iterazione agli autovalori non è riuscita
a risolvere viene riportato come `0` Hz con un avviso nel log.

Sono disponibili due metodi, selezionati dall'impostazione del metodo modale:

| Metodo modale | Solutore agli autovalori | Estrae |
|---|---|---|
| Multiple modes | iterazione nel sottospazio con proiezione di Rayleigh-Ritz | il numero di modi richiesto |
| Only most dominant mode | iterazione inversa delle potenze con quoziente di Rayleigh | un singolo modo |

Entrambi i metodi iterano su `K^-1 * M`, quindi convergono verso le frequenze
**più basse** - che sono quelle che contano. Ogni passo è una soluzione lineare
con `K`, usando le impostazioni del gradiente coniugato della configurazione del
solutore matriciale, quindi il costo di un'analisi modale è all'incirca il costo
di una soluzione statica moltiplicato per il numero di vettori del sottospazio e
di iterazioni.

L'iterazione nel sottospazio porta con sé un blocco di vettori, proietta `K` e `M`
sul sottospazio che essi generano e risolve esattamente il piccolo problema agli
autovalori denso risultante. Vengono portati con sé alcuni vettori in più rispetto
al numero di modi richiesto, perché fanno convergere sensibilmente più in fretta i
modi desiderati. Per `p` modi richiesti il blocco contiene `min(2p, p+8)` vettori,
il consueto compromesso tra velocità di convergenza e costo di un'iterazione. Gli
autovalori risultano direttamente come `lambda` e sono ordinati in modo
crescente, quindi **il modo 0 è il modo fondamentale**.

**È qui che si spende il tempo di esecuzione.** Un'iterazione nel sottospazio
esegue una soluzione lineare completa per ogni vettore del blocco, quindi
chiedere 10 modi costa 18 soluzioni per iterazione e chiederne 100 ne costa 108.
L'iterazione si ferma in anticipo solo quando **ogni** autovalore richiesto si è
stabilizzato entro il valore di convergenza, e il modo richiesto più alto è
sempre l'ultimo a stabilizzarsi - quindi un numero elevato di modi rende costosa
ogni iterazione e improbabile l'arresto anticipato. Chiedi i modi che intendi
effettivamente esaminare.

Se i modi richiesti non si sono stabilizzati entro l'ultima iterazione, il
solutore lo indica nel log e riporta la variazione raggiunta, anziché presentare
senza commenti i valori non convergenti. I modi più bassi di un'esecuzione del
genere sono di solito ancora affidabili; i più alti no.

Per un'analisi modale la matrice di massa viene sempre assemblata, quindi la
densità deve essere presente su ogni entità che vi partecipa.

**Analisi modale precaricata.** Prima di assemblare le matrici, un'analisi modale
sposta ogni nodo del risultato di spostamento di una soluzione precedente, e poi
ripristina la mesh. Eseguire *Stress analysis* e poi *Modal analysis* in un unico
flusso dei task dà quindi i modi della struttura **caricata e deformata** anziché
di quella indeformata. La forma modale memorizzata è l'autovettore sommato a tale
spostamento iniziale.

Viene aggiornata solo la geometria - la matrice di rigidezza è sempre quella
elastica lineare valutata sulla mesh spostata. Ciò coglie il cambiamento di forma,
non l'irrigidimento geometrico di un elemento in trazione.

L'analisi modale è **esclusiva**: non può essere combinata con nessun'altra
fisica tranne un'analisi delle tensioni che la precede.

### 1.9 Risultati derivati

Dopo la soluzione le tensioni di elemento vengono ricavate dal campo di
spostamento come media sui punti di integrazione di

```
sigma = D * ( B * u  -  eps_th )
```

con la deformazione termica `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]` per gli
elementi di volume. Gli elementi di superficie sono in deformazione piana: la
dilatazione fuori dal loro piano è impedita, il che aumenta la deformazione
termica nel piano a `eps_th = (1+v) * alpha * dT * [1, 1, 0]`. La stessa
deformazione termica entra nel carico termico `f_th = INT( B^T * D * eps_th )`.
Gli elementi linea riportano `E * (eps - alpha * dT)`.

Dalle sei componenti il solutore forma due invarianti e li combina in
quadratura nella tensione di von Mises:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(`normal part` = parte normale, `shear part` = parte di taglio), che è la
definizione classica

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

Per gli elementi di superficie si usano gli equivalenti bidimensionali, `QN` dalle
due tensioni normali nel piano e `QS = sqrt(3 * tau_xy^2)`. Per gli elementi linea
viene riportato solo il valore assiale e la parte di taglio è nulla.

Le **forze nodali** vengono ricavate elemento per elemento come forza elastica
interna

```
f = K * u - f_th
```

e accumulate nei nodi. In un nodo vincolato è la forza di reazione; in un nodo
libero non caricato dovrebbe risultare quasi nulla per una soluzione statica
convergente, il che rende il risultato Force un utile controllo del residuo. Un
corpo libero di dilatarsi termicamente non mostra quindi né forze né tensioni.

| Risultato | Si applica a | Significato |
|---|---|---|
| Displacement `[m]` | nodo | vettore spostamento, o la forma modale in un'analisi modale |
| Von Mises stress `[Pa]` | elemento | `sqrt(QN^2 + QS^2)` come sopra |
| Normal stress `[Pa]` | elemento | `QN` |
| Shear stress `[Pa]` | elemento | `QS` |
| Normal stress in X, Y, Z `[Pa]` | elemento | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | elemento | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | nodo | forza nodale ricavata, reazione nei nodi vincolati |

Le sei componenti di tensione sono riportate in coordinate **globali** per gli
elementi di volume. Gli elementi di superficie e linea valutano la propria
tensione nel proprio sistema locale di elemento, quindi le loro componenti sono
riportate in quel sistema: una superficie riempie `sigma_xx`, `sigma_yy` e
`tau_xy` nel piano, e una linea riempie solo `sigma_xx` assiale.

### 1.10 Statica, transitoria o modale?

| | Statica | Dinamica transitoria | Quasi-statica transitoria | Modale |
|---|---|---|---|---|
| Risolve | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` nel tempo | `K*u = f(t)` a ogni istante | `K*phi = omega^2*M*phi` |
| Risponde a | «quanto si deforma e quanto è sollecitata» | «come risponde a un carico che varia rapidamente» | «come segue un carico che varia lentamente» | «a quali frequenze e con quale forma tende a vibrare» |
| Richiede la massa | no | sì | no | sì |
| Richiede vincoli contro il moto rigido | sì | sì | sì | sì |
| Record | uno | uno per passo temporale | uno per passo temporale | uno per modo estratto |

**Usa un'analisi statica per** verifiche di resistenza e rigidezza, limiti di
freccia, forze di reazione, peso proprio, recipienti in pressione, tensioni
termiche. È la scelta predefinita e copre la maggior parte dei casi.

**Usa un'analisi dinamica transitoria per** urti, carichi applicati o rilasciati
improvvisamente, storie di shock e vibrazioni, propagazione di onde e la risposta
a un'eccitazione armonica vicina a una risonanza - ovunque conti l'inerzia.

**Usa un'analisi quasi-statica transitoria per** un carico che varia nel tempo
abbastanza lentamente da permettere alla struttura di seguirlo in equilibrio.

**Usa un'analisi modale per** frequenze naturali e forme modali, evitare la
risonanza con un'eccitazione nota, giudicare se una struttura è abbastanza rigida
dal punto di vista dinamico e cercare meccanismi indesiderati - un autovalore
quasi nullo indica di solito un vincolo mancante o una parte scollegata.

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Condizioni ambientali`,
`Materiale`, `Risultati`), nel menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Aggiungi un task e scegli:

- **Stress analysis** - "Stress and linear elasto-dynamics analysis including
  modal analysis" (analisi delle tensioni ed elastodinamica lineare, compresa
  l'analisi modale);
- **Modal analysis** - "Stress modal analysis" (analisi modale strutturale).

L'analisi delle tensioni si combina liberamente con altre fisiche. Due catene
utili sono *Heat transfer* e poi *Stress analysis* per le tensioni termiche, e
*Incompressible viscous flow* e poi *Stress analysis* per i carichi di pressione.

L'analisi modale esclude ogni altro tipo di problema tranne un'analisi delle
tensioni che la precede, il che la trasforma in un'analisi modale precaricata.

### 2.2 Scheda Problema

**Solutore temporale** - mostrato per l'analisi delle tensioni, poiché è un tipo
di problema dipendente dal tempo. Lascialo disattivato per un'analisi statica.

| Campo | Significato |
|---|---|
| Abilita | attiva e disattiva l'avanzamento transitorio |
| Approssimazione | imposta il beta di Newmark di un'analisi dinamica, vedi 1.7; centrale è la scelta consigliata |
| Tempo iniziale | primo valore di tempo in secondi; ignorato al riavvio del solutore |
| Tempo finale | `start time + time-step size * number of time-steps`, sola lettura |
| Dimensione del passo temporale | `dt` in secondi |
| Numero di passi temporali | quanti passi calcolare |
| Frequenza di output | scrive un file di risultati ogni N passi; `0` scrive solo l'ultimo passo |

**Impostazioni dell'analisi transitoria delle tensioni** - mostrato per l'analisi
delle tensioni e attivo solo mentre il solutore temporale è attivo.

| Campo | Significato | Predefinito |
|---|---|---|
| Tipo di analisi | *Dynamic* (con inerzia) o *Quasi-static* (una soluzione statica per passo temporale) | Dynamic |
| Smorzamento numerico (alfa HHT) | da `-1/3` a `0`; i valori negativi smorzano il rumore ad alta frequenza | 0 |
| Matrice di massa concentrata | usa una matrice di massa diagonale invece di quella consistente | disattivato |

Gli ultimi due si applicano solo all'analisi dinamica e altrimenti sono
disattivati.

**Impostazioni dell'analisi modale** - mostrato quando il tipo di problema è
*Modal analysis*.

| Campo | Significato | Predefinito |
|---|---|---|
| Metodo modale | *Only most dominant mode* o *Multiple modes* | Multiple modes |
| Iterazioni | iterazioni del solutore agli autovalori; disattivato per il metodo del modo dominante | 100 |
| Estrai modi | quanti modi estrarre; disattivato per il metodo del modo dominante | 10 |
| Valore di convergenza | soglia di convergenza del solutore agli autovalori | 1.0e-9 |

I due conteggi sono indipendenti: le iterazioni raffinano insieme l'intero
insieme di modi, non li producono uno alla volta.

**Estrai modi è l'impostazione che decide la durata dell'esecuzione.** Ogni modo
aggiunge una soluzione lineare a ogni iterazione, e l'esecuzione può fermarsi in
anticipo solo quando tutti i modi richiesti si sono stabilizzati, quindi il costo
cresce più rapidamente del conteggio. Dieci è un punto di partenza sensato ed è il
valore predefinito; cento è un calcolo di un altro ordine di grandezza, e su una
mesh grande può richiedere ore. Aumentalo solo quando sai di aver bisogno dei modi
più alti.

Se un'esecuzione richiede più tempo del previsto, il log del solutore indica il
vettore del sottospazio su cui sta lavorando, così l'avanzamento all'interno di
un'iterazione è visibile, e dopo ogni iterazione stampa il tasso di convergenza
raggiunto accanto al valore richiesto.

### 2.3 Scheda Materiale

La scheda `Materiale` elenca il materiale assegnato all'entità selezionata.
L'analisi delle tensioni richiede **tutte e quattro** queste proprietà:

| Proprietà | Unità | Commento |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, circa `2.1e11` per l'acciaio |
| Poisson ratio | `-` | `v`, circa `0.3` per l'acciaio; deve restare sotto `0.5` |
| Density | `kg/m^3` | necessaria per peso proprio, analisi dinamica e modale |
| Thermal expansion coefficient | `1/K` | impostalo a `0` se non c'è carico termico |

Un'entità che ne manca una non viene risolta, e il controllo della
configurazione avvisa prima dell'avvio dell'esecuzione.

Due ulteriori proprietà sono facoltative e vengono lette solo da un'analisi
dinamica transitoria:

| Proprietà | Unità | Commento |
|---|---|---|
| Rayleigh mass damping coefficient | `1/s` | `alpha_M`, vedi 1.7; zero se assente |
| Rayleigh stiffness damping coefficient | `s` | `beta_K`, vedi 1.7; zero se assente |

Ogni proprietà è una tabella in funzione della temperatura, quindi un modulo
dipendente dalla temperatura viene rispettato quando prima viene eseguito un
solutore termico.

### 2.4 Scheda Condizioni al contorno

La scheda è suddivisa in un elenco gestore delle condizioni al contorno
disponibili in alto e in un editor per quella selezionata in basso. Seleziona
prima un'entità nell'albero `Modello`; il gestore offre allora le condizioni
valide per quel tipo di entità e per il problema selezionato. Attivane una per
assegnarla, poi modificane le componenti nell'albero inferiore.

Vengono offerte le condizioni della sezione 1.5. Note sul loro uso:

- **Displacement** mostra una casella di controllo accanto a ciascuna delle sue
  tre componenti. Vengono vincolate solo le componenti selezionate, quindi una
  faccia può essere bloccata in X e lasciata libera in Y e Z. Il campo del valore
  di una componente non selezionata è disattivato.
- **Normal displacement** e **Roller displacement** prendono per impostazione
  predefinita il loro sistema locale dalla geometria - le normali medie degli
  elementi di una superficie, la direzione degli elementi di una linea. Su
  un'entità **punto** non c'è geometria da seguire, quindi l'editor *Direzione
  locale* è sempre mostrato e sempre usato.
- Su una superficie o una linea l'albero della condizione mostra una casella di
  controllo **Usa la direzione locale inserita**. Attivala per mostrare l'editor
  *Direzione locale* e prendere il sistema dalla direzione inserita anziché dalla
  mesh. Le frecce di direzione disegnate nella vista 3D seguono quella delle due
  che è in vigore. Se un vincolo su una superficie si comporta in modo strano con
  la casella disattivata, verifica che le normali della superficie siano coerenti
  e sincronizzale con gli strumenti di geometria.
- I vincoli di più entità che si incontrano in un nodo vengono combinati, quindi
  una faccia può essere bloccata in una direzione globale da *Displacement* e
  contemporaneamente vincolata a carrello su un piano inclinato. Se due di essi
  chiedono valori diversi nella stessa direzione, l'esecuzione si arresta con un
  errore che indica il nodo.
- **Pressure** e **Force (unit area)** si applicano solo alle superfici.
- Ogni componente è una tabella in funzione del tempo, quindi a un carico si può
  dare un andamento nel tempo per un'esecuzione transitoria.

### 2.5 Condizioni iniziali e ambientali

La scheda `Condizioni ambientali` è il luogo in cui si attiva la **gravità**.
Assegna *Gravitational acceleration* all'entità che deve portare il proprio peso -
di solito l'intero modello - e imposta il vettore; il valore predefinito è
`-9.80665` in Z. Assegna *Temperature* per impostare la temperatura di
riferimento priva di tensioni per un'analisi delle tensioni termiche.

La scheda `Condizioni iniziali` offre *Displacement*, *Structural velocity* e
*Pressure*, applicate alla prima esecuzione di un'analisi transitoria.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Il solutore strutturale usa la voce **CG**, sia per la
soluzione statica e transitoria sia per le soluzioni interne dell'iterazione agli
autovalori.

Se una soluzione statica non converge, la causa abituale è un modello labile più
che un'impostazione del solutore. Controlla i vincoli prima di aumentare il numero
di iterazioni.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare - tipicamente *Displacement* o *Von Mises
stress*. In un'esecuzione transitoria l'andamento è mostrato da `Report` ->
`Punti di monitoraggio`.

### 2.8 Risultati e record

La scheda `Risultati` elenca le variabili calcolate e controlla la vista 3D. Lo
spostamento è un vettore nodale; le tre misure di tensione sono scalari di
elemento. Un'analisi dinamica transitoria aggiunge i vettori nodali *Structural
velocity* e *Structural acceleration*.

Per un grafico della forma deformata, visualizza *Displacement* e attiva la
scalatura della deformazione nelle impostazioni della vista - gli spostamenti
reali sono di solito troppo piccoli per essere visibili in scala reale.

La scheda `Record` del pannello `Modello` elenca i record dei risultati:

- **analisi statica** - un unico record;
- **analisi transitoria** - un record per ogni passo temporale scritto;
- **analisi modale** - un record per ogni modo estratto, numerati a partire dal
  fondamentale. Scorrere i record significa scorrere i modi, e animarli anima le
  forme modali.

Per un record modale la vista 3D etichetta il record con il numero del modo e la
sua frequenza naturale in Hz.

`Report` -> `File di log del solutore` mostra l'output completo del solutore,
compreso l'autovalore stampato per ogni modo estratto e le statistiche di
spostamento, tensione di von Mises e forza dopo ogni record.

---

## 3. Tutorial - analisi statica delle tensioni

**Obiettivo.** Calcolare la freccia e la tensione di una mensola caricata dal
proprio peso e da un carico in punta, e leggere la reazione all'incastro.

Questo tutorial presuppone un modello di volume meshato - un parallelepipedo
allungato va bene. Usa `File` -> `Nuovo modello`, disegna un parallelepipedo con
`Geometria` -> `Crea elemento` o importa una geometria, e tetraedrizzala; il
tutorial *Disegnare un cubo* tratta questi passaggi. Marca l'estremità incastrata
e quella caricata come entità superficie separate, in modo da potervi assegnare le
condizioni al contorno.

### Passo 1 - selezionare il problema

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungi un task e seleziona **Stress analysis**.
3. Conferma con `OK`.

### Passo 2 - assegnare il materiale

1. Seleziona l'entità volume nell'albero `Modello`.
2. Apri la scheda `Materiale` e assegna un materiale con tutte e quattro le
   proprietà, per l'acciaio da costruzione:
   - **Modulus of elasticity** = `2.1e11` `Pa`
   - **Poisson ratio** = `0.3`
   - **Density** = `7850` `kg/m^3`
   - **Thermal expansion coefficient** = `1.2e-5` `1/K`

### Passo 3 - mantenere l'analisi statica

Apri la scheda `Problema` e lascia **Abilita** disattivato nel riquadro
*Solutore temporale*. Il solutore assembla e risolve quindi `K*u = f` una volta.

### Passo 4 - vincolare il modello

1. Seleziona l'entità superficie all'estremità incastrata.
2. Apri la scheda `Condizioni al contorno` e attiva **Displacement**.
3. Lascia selezionate tutte e tre le componenti e impostate a `0`.

Ciò blocca completamente la faccia e rimuove tutti e sei i modi rigidi. Verifica
che nient'altro resti libero - una parte scollegata senza vincoli rende singolare
il sistema e la soluzione non converge.

Per un appoggio che deve scorrere anziché essere completamente incastrato, usa
invece **Roller displacement** sulla superficie. Blocca solo la direzione normale
della superficie e lascia il nodo libero di muoversi nel piano tangente.

### Passo 5 - applicare i carichi

**Peso proprio.** Seleziona l'entità volume, apri la scheda `Condizioni
ambientali` e attiva **Gravitational acceleration**. Lascia il valore predefinito
`-9.80665` in Z.

**Carico in punta.** Seleziona l'entità superficie all'estremità libera, apri la
scheda `Condizioni al contorno` e attiva **Force**. Inserisci la forza totale in
newton, ad esempio `-1000` in Z. Il valore è il totale sull'entità, quindi non
cambia raffinando la mesh.

Alternative, a seconda di cosa sia fisicamente il carico:

- **Pressure** per una pressione normale uniforme su una faccia, in `Pa`;
- **Force (unit area)** per una trazione orientata in `N/m^2`;
- **Weight** per una massa applicata in `kg` non modellata come geometria.

### Passo 6 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Il controllo della configurazione segnala materiali o condizioni al contorno
   mancanti prima dell'avvio dell'esecuzione.
3. Segui l'avanzamento in `Report` -> `File di log del solutore`.

### Passo 7 - esaminare i risultati

- Scheda `Risultati`: visualizza **Displacement** per vedere la forma deformata, e
  **Von Mises stress** per la distribuzione delle tensioni. Attiva la scalatura
  della deformazione nelle impostazioni della vista per rendere visibile la
  freccia.
- **Normal stress** e **Shear stress** separano i due contributi che compongono il
  valore di von Mises riportato.
- **Force** sulla faccia incastrata è la reazione. La sua somma dovrebbe
  bilanciare il carico applicato più il peso proprio - una verifica rapida ed
  efficace che il modello sia caricato come intendevi.
- Un punto di monitoraggio su *Displacement* in punta dà la freccia come numero
  anziché come colore.

### Passo 8 - facoltativo, tensioni termiche

Per aggiungere un carico termico:

1. Aggiungi un task **Heat transfer** *prima* del task delle tensioni nel flusso
   dei task del problema, e configurane le condizioni al contorno e le proprietà
   del materiale.
2. Assegna all'entità strutturale una condizione ambientale **Temperature** con
   la temperatura di riferimento priva di tensioni, ad esempio `293.15` K.

Il solutore delle tensioni prende dal solutore termico le temperature degli
elementi calcolate e aggiunge automaticamente il termine di dilatazione termica.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La soluzione non converge | il modello è labile; una parte è libera o gli appoggi non eliminano tutti e sei i modi rigidi |
| *Entity has material assigned which is missing required properties* | manca una delle quattro proprietà richieste |
| Gli spostamenti sono nulli ovunque | nessun carico ha raggiunto il modello - verifica che la gravità sia assegnata, o che le componenti di Force siano diverse da zero |
| Gli spostamenti sono enormi | il modulo di elasticità è sbagliato di ordini di grandezza, oppure un'entità superficie ha spessore nullo e quindi nessuna rigidezza |
| Il carico cambia raffinando la mesh | una *Force (unit area)* o una *Pressure* era intesa come forza totale, o viceversa |
| La pressione spinge nel verso sbagliato | le normali della superficie non sono coerenti, oppure la superficie non delimita un volume |

---

## 4. Tutorial - analisi modale

**Obiettivo.** Estrarre i primi modi naturali della stessa mensola.

### Passo 1 - selezionare il problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`), aggiungi un task e
seleziona **Modal analysis**.

L'analisi modale non può essere combinata con altre fisiche, con l'unica eccezione
di un'analisi delle tensioni posta prima di essa, che la rende un'analisi modale
precaricata.

### Passo 2 - assegnare il materiale

Le stesse quattro proprietà del tutorial statico. Qui la **densità** conta più che
altrove: è l'intera matrice di massa, e le frequenze scalano come
`1/sqrt(rho)`.

### Passo 3 - vincolare il modello

Assegna la stessa condizione al contorno **Displacement** all'estremità
incastrata.

I vincoli sono ciò che distingue i modi di una mensola da quelli di una barra
libera. Se il modello resta non vincolato, i primi sei autovalori saranno quasi
nulli - i modi rigidi - e i modi interessanti verranno spinti più in alto
nell'elenco.

### Passo 4 - configurare le impostazioni modali

Apri la scheda `Problema` e in *Impostazioni dell'analisi modale* imposta:

| Campo | Valore |
|---|---|
| Metodo modale | `Multiple modes` |
| Iterazioni | `100` |
| Estrai modi | `6` |
| Valore di convergenza | `1.0e-9` |

Sei modi sono un punto di partenza sensato, e meno dei dieci predefiniti. Ogni
modo in più aggiunge una soluzione lineare a ogni iterazione, quindi un numero
elevato di modi su una mesh grande è lento.

Scegli `Only most dominant mode` quando serve un solo modo; usa l'iterazione
inversa delle potenze su un singolo vettore ed è molto più economico.

### Passo 5 - non servono carichi

Un'analisi modale non ha un vettore dei carichi - il problema agli autovalori è
omogeneo. I carichi assegnati al modello vengono ignorati, con un'eccezione: se
prima è stata eseguita un'analisi delle tensioni, il suo risultato di spostamento
deforma la mesh per l'assemblaggio modale.

### Passo 6 - risolvere

`Soluzione` -> `Avvia solutore` (`Ctrl+R`). Il log riporta le iterazioni nel
sottospazio e il loro tasso di convergenza, e poi un blocco per ogni modo estratto:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

I modi vengono elaborati dall'indice estratto più alto fino al fondamentale,
quindi il fondamentale è l'ultimo calcolato e viene memorizzato come record 1.

### Passo 7 - esaminare i modi

- Pannello `Modello`, scheda `Record`: un record per modo, numerati a partire dal
  fondamentale. Scorrili per vedere ogni forma modale.
- Scheda `Risultati`: visualizza **Displacement**. Una forma modale è definita
  solo a meno di un fattore di scala, quindi i moduli sono arbitrari - ciò che
  conta è la forma. Attiva la scalatura della deformazione e anima il record per
  visualizzare il modo.
- L'autovalore di ogni modo è stampato nel log del solutore e mostrato come
  etichetta del record nella vista 3D.

### Passo 8 - leggere le frequenze naturali

La vista 3D etichetta ogni record modale con il numero del modo e la sua frequenza
naturale in Hz. Il log del solutore elenca la stessa frequenza per ogni modo
estratto insieme all'autovalore sottostante:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

Un modo che l'iterazione agli autovalori non è riuscita a risolvere viene
riportato come `0` Hz e porta un avviso nel log.

Come per qualsiasi risultato, conviene verificare la frequenza fondamentale con un
calcolo manuale la prima volta che si imposta un modello - la sezione 5 dà la
formula per una mensola.

### Passo 9 - facoltativo, modi precaricati

1. Nel flusso dei task del problema, metti un task **Stress analysis** *prima* del
   task **Modal analysis**.
2. Imposta i carichi statici e i vincoli come nel tutorial 3.

L'analisi modale viene quindi eseguita sulla mesh deformata. È il modo per vedere
come un carico statico modifica la forma in cui vibra una struttura.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| I primi autovalori sono quasi nulli | il modello è labile, oppure una parte è scollegata - un vero meccanismo |
| Il solutore agli autovalori non converge | troppe poche iterazioni, oppure una matrice di massa mal condizionata perché un'entità non ha densità o misura geometrica |
| L'estrazione dei modi è molto lenta | troppi modi richiesti; ogni modo costa una soluzione lineare completa |
| L'esecuzione dura ore, o sembra non finire mai | troppi modi richiesti - ciascuno costa una soluzione lineare a ogni iterazione, e l'esecuzione non può fermarsi in anticipo finché tutti non si sono stabilizzati. Osserva nel log il conteggio dei vettori del sottospazio per vedere l'avanzamento all'interno di un'iterazione |
| Una frequenza sembra assurda | l'iterazione nel sottospazio non è convergente - il log lo indica alla fine di un'esecuzione non convergente; estrai meno modi, aumenta il numero di iterazioni o controlla densità e vincoli |
| Le forme modali sembrano identiche | modi ravvicinati di una struttura simmetrica, oppure troppo poche iterazioni nel sottospazio per separarli |
| Meno record dei modi richiesti | il sottospazio ha perso direzioni indipendenti; il log indica quanti modi sono stati estratti e memorizza solo quelli |

---

## 5. Verifica di un modello

Il solutore non ha una suite di verifica integrata, quindi conviene validare un
nuovo modello rispetto a qualcosa che puoi calcolare a mano prima di fidartene.
Tre verifiche economiche, in ordine di utilità:

**Bilancio delle reazioni.** Somma il risultato *Force* sui nodi vincolati. Deve
bilanciare i carichi applicati più il peso proprio `rho * V * g`. Ciò rileva
moduli di carico sbagliati, unità sbagliate e direzioni di pressione sbagliate.

**Freccia in punta di una mensola.** Per una trave di lunghezza `L`, momento
d'inerzia della sezione `I` e carico in punta `F`, la teoria di Euler-Bernoulli dà

```
delta = F * L^3 / (3 * E * I)
```

Modella la trave con elementi di volume e confronta. Aspettati che il risultato
agli elementi finiti sia leggermente più rigido su una mesh grossolana, e che
converga dal basso al raffinarsi della mesh. Non usare elementi linea per questa
verifica - non hanno rigidezza flessionale.

**Frequenza fondamentale di una mensola.** Per la stessa trave con area della
sezione `A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Confronta con `sqrt(lambda_0) / (2*pi)` dall'analisi modale. Ciò valida
contemporaneamente la matrice di massa, la matrice di rigidezza e la tua
conversione degli autovalori.

**Barra caricata improvvisamente.** Una barra incastrata a un'estremità e caricata
all'altra da una forza assiale `F` applicata a `t = 0` è una verifica di
un'analisi dinamica. L'onda di tensione corre fino all'estremità incastrata e
ritorno con la velocità `c = sqrt(E/rho)`, quindi l'estremità libera si sposta
linearmente fino al doppio della freccia statica, `2 * F * L / (E * A)`, a
`t = 2*L/c` e torna a zero a `t = 4*L/c`. Senza smorzamento il ciclo si ripete.
Con lo smorzamento di Rayleigh si estingue e la barra si ferma alla freccia
statica. Usa un coefficiente di Poisson nullo, così che la barra sia davvero
monodimensionale.

Per ognuna di queste verifiche, raffina la mesh almeno una volta e conferma che la
risposta converge anziché variare.

---

## 6. Limitazioni

### Limitazioni di modellazione

- **Solo elasticità lineare.** Piccole deformazioni, piccoli spostamenti,
  materiale isotropo, nessuna plasticità, nessun contatto, nessuna grande
  deformazione e nessuna instabilità. I risultati hanno significato solo finché la
  struttura resta nel suo campo lineare.
- **Lo smorzamento è solo di Rayleigh.** La matrice di smorzamento è
  `alpha_M*M + beta_K*K` per materiale, quindi il rapporto di smorzamento segue la
  curva di 1.7 e non può essere impostato modo per modo. Non ci sono smorzatori
  discreti, né smorzamento indipendente dalla frequenza (isteretico), né una
  condizione al contorno di smorzamento.
- **Solo integrazione temporale implicita.** Ogni passo è una soluzione lineare.
  Non c'è uno schema esplicito alle differenze centrali, quindi eventi molto
  brevi come un impatto balistico richiedono molti passi costosi.
- **Gli elementi di superficie sono in deformazione piana.** Una piastra sottile
  caricata nel proprio piano sarà più rigida di un'idealizzazione in tensione
  piana. Modella le strutture sottili con elementi di volume quando la differenza
  è importante.
- **Gli elementi linea sono aste.** Viene assemblata solo la rigidezza assiale
  `E*A`. Non c'è flessione, taglio né torsione, comunque si chiami il tipo di
  elemento, quindi un telaio modellato con elementi linea si comporterà come una
  travatura reticolare a nodi incernierati.
- **Il precarico è solo geometrico.** Un'analisi modale precaricata rivaluta la
  rigidezza lineare sulla mesh spostata. Non aggiunge un termine di rigidezza
  geometrica, quindi l'irrigidimento di un cavo teso o l'ammorbidimento di un
  puntone compresso non vengono colti.
- **Nessun fattore di partecipazione modale né massa efficace.** Il solutore
  riporta solo autovalori e forme modali, quindi non c'è un'indicazione diretta di
  quali modi contino per una data direzione di eccitazione.
