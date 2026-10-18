# Dispersione di contaminanti - Manuale teorico e guida utente

Questo documento descrive il solutore di dispersione di contaminanti di Range FEA -
`RSolverFluidParticle`, che gestisce il tipo di problema **Dispersione di
contaminanti** (*Contaminant dispersion*): l'equazione che risolve, il
significato di ogni input che accetta, le parti dell'interfaccia grafica che lo
controllano e un tutorial svolto.

La dispersione di contaminanti viene risolta sul campo di flusso di un task
**Flusso viscoso incomprimibile** (*Incompressible viscous flow*). Il solutore
del flusso, le sue condizioni al contorno, la sua iterazione non lineare e il
suo avanzamento nel tempo sono descritti nel manuale
[Flusso di fluidi](theory_fluid.md), su cui questo manuale si basa.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - flusso transitorio con dispersione di contaminanti](#3-tutorial---flusso-transitorio-con-dispersione-di-contaminanti)
4. [Verifica di un modello](#4-verifica-di-un-modello)
5. [Limitazioni](#5-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - la concentrazione di particelle

L'incognita principale è la **concentrazione di particelle** `C` in `kg/m^3`, un
grado di libertà per nodo.

| Tipo di problema | Solutore | Incognita principale | Unità | Gradi di libertà per nodo |
|---|---|---|---|---|
| Contaminant dispersion | `RSolverFluidParticle` | concentrazione di particelle | `kg/m^3` | 1 |

Il tipo di problema **richiede** un task *Incompressible viscous flow* nello
stesso problema: legge il campo di velocità dal suo risultato e non lo calcola
mai da sé.

### 1.2 Equazione di governo

La **dispersione di contaminanti** trasporta una concentrazione scalare sul
campo di flusso,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

con stabilizzazione SUPG e l'avanzamento nel tempo a theta del solutore del
flusso, leggendo la velocità dal risultato del flusso. (`for s > 0 and a maximum
saturation set` significa «per s > 0 e con saturazione massima impostata»,
`otherwise` «altrimenti».)

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `C` | concentrazione di particelle | `kg/m^3` | incognita |
| `v` | velocità | `m/s` | il risultato del flusso |
| `D` | coefficiente di diffusione | `m^2/s` | *Impostazioni della dispersione di contaminanti*, `0` per impostazione predefinita |
| `C_sat` | saturazione massima | `kg/m^3` | *Impostazioni della dispersione di contaminanti*, `0` (illimitata) per impostazione predefinita |
| `s` | tasso della sorgente di particelle | `kg/(m^3*s)` | condizione Particle rate |

### 1.3 Impostazioni della dispersione di contaminanti

I due parametri del tipo di problema si impostano una volta per modello nel
riquadro *Impostazioni della dispersione di contaminanti* della scheda
`Problema` (sezione 2.2). Entrambi valgono zero per impostazione predefinita, il
che riproduce un'avvezione pura con una sorgente illimitata.

#### Coefficiente di diffusione

Il **coefficiente di diffusione** `D` diffonde il contaminante lungo il suo
gradiente di concentrazione. Con `D = 0` il trasporto è puramente avvettivo: il
contaminante viene trascinato dal flusso e si diffonde solo per la diffusione
numerica della stabilizzazione, quindi il solutore risponde alla domanda «dove
lo porta il flusso e in quanto tempo», non «quanto è larga la nube». Un `D`
positivo rende la larghezza della nube un risultato fisico.

Non c'è un modello di turbolenza (sezione 5), quindi `D` è una diffusività
**efficace**: il mescolamento di un flusso reale è dominato dalla turbolenza, che
si può stimare come `D ~ nu_t / Sc_t` con il numero di Schmidt turbolento
`Sc_t ~ 0.7`.

| Diffusione molecolare (la proprietà fisica) | `D [m^2/s]` |
|---|---|
| gas e vapori in aria (vapore acqueo `2.5e-5`, CO2 `1.6e-5`, vapori di solventi circa `1e-5`) | `1e-6` - `1e-4` |
| specie disciolte in acqua (O2 `2e-9`, sali circa `1.5e-9`) | `1e-10` - `1e-8` |
| particelle di aerosol in aria, moto browniano (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

Alle scale ingegneristiche la diffusione molecolare è quasi sempre trascurabile
rispetto al trasporto dovuto al flusso. Il valore che si inserisce di solito è
quello efficace:

| Diffusione efficace (turbolenta) | `D_eff [m^2/s]` |
|---|---|
| locale ventilato, mescolamento dell'aria interna | `1e-3` - `1e-2` |
| flusso in condotto o canale, circa `D ~ 0.01 - 0.05 * U * L` (`U` velocità media, `L` dimensione del condotto) | `1e-3` - `1e-1` |
| fiumi, mescolamento trasversale | `1e-2` - `1` |
| atmosfera, mescolamento orizzontale | `1` - `100` |

Se un valore conta o meno dipende dalla mesh. Con velocità media `U`, lunghezza
del flusso `L`, dimensione dell'elemento `h` e passo temporale `dt`:

- **quando `D` ha un effetto visibile** - il contaminante si allarga di circa
  `sqrt( 2 * D * L / U )` mentre attraversa il dominio. Se questo è minore di
  `h`, `D` non ha effetto, quindi `D` conta solo sopra circa
  `U * h^2 / ( 2 * L )`. Per un canale con `U ~ 1 m/s`, `L ~ 15 m` e
  `h ~ 0.1 m` sono circa `3e-4 m^2/s`;
- **quando `D` domina** - sopra circa `U * L` (circa `15 m^2/s` per lo stesso
  canale) la diffusione appiattisce tutto e il flusso conta appena;
- **avanzamento nel tempo** - se `D * dt / h^2` supera circa `1`, passa a
  *Backward difference (stable)*; lo schema alle differenze centrali non smorza i
  modi diffusivi più rapidi e oscilla.

#### Saturazione massima

La **saturazione massima** `C_sat` è la massima concentrazione che il
contaminante può raggiungere nel fluido - una concentrazione di vapore saturo o
una solubilità. Quando è impostata, un *Particle rate* positivo si indebolisce
man mano che la concentrazione vi si avvicina e si arresta alla saturazione,
`s * ( 1 - C / C_sat )`; ciò modella evaporazione o dissoluzione, e `s` è allora
il tasso in un fluido pulito, circa `k_m * C_sat * A / V`. I pozzi (`s < 0`) non
sono limitati. Il limite agisce solo sulla sorgente: una concentrazione
prescritta da una condizione al contorno o iniziale non è limitata, e il log del
solutore avvisa quando supera `C_sat`. Nemmeno la concentrazione viene troncata,
quindi una leggera sovraelongazione vicino a un fronte ripido resta possibile.

Un limite di saturazione esiste solo dove ce n'è uno termodinamico: una tensione
di vapore o una solubilità. Per un vapore in aria la concentrazione di
saturazione deriva dalla tensione di vapore saturo,
`C_sat = p_sat * M / ( R * T )`:

| Vapore in aria | Temperatura | `C_sat [kg/m^3]` |
|---|---|---|
| vapore acqueo | 0 degC | `0.0048` |
| vapore acqueo | 20 degC | `0.017` |
| vapore acqueo | 40 degC | `0.051` |
| vapore acqueo | 100 degC | `0.60` |
| etanolo o toluene | 20 degC | circa `0.11` |
| metanolo | 20 degC | circa `0.17` |
| benzene | 20 degC | circa `0.32` |
| acetone | 20 degC | circa `0.59` |

Per una sostanza disciolta in acqua il limite è la sua solubilità:

| Disciolto in acqua | `C_sat [kg/m^3]` |
|---|---|
| ossigeno in equilibrio con l'aria, 20 degC | `0.009` |
| anidride carbonica a 1 atm | `1.7` |
| gesso | `2.4` |
| cloruro di sodio | circa `360` |

Particelle, polveri e fumi non hanno saturazione; lascia il valore a `0`
(illimitata). Solo come ordine di grandezza, i limiti di esposizione
professionale sono circa `1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) e i limiti
inferiori di esplosività delle polveri circa `0.02` - `0.06 kg/m^3`.
La saturazione dipende fortemente dalla temperatura, mentre il solutore usa un
unico valore costante: per un caso non isotermo prendi il valore alla
temperatura rilevante più fredda, quella a cui inizierebbe la condensazione.

### 1.4 Stabilizzazione

Il parametro SUPG del solutore di dispersione è limitato dal passo temporale e
include la diffusione,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

con il termine `dt` omesso in un'analisi stazionaria. Si riduce a
`h / ( 2 * |v| )` dove il flusso è veloce, non supera mai `dt / 2` e tende a
`h^2 / ( 12 * D )` - il limite del parametro monodimensionale esatto nei nodi -
dove domina la diffusione. Il limite sul passo temporale conta nelle zone lente e
di ricircolo: lì il numero di Courant dell'elemento `|v| * dt / h` è minuscolo e
un `tau` illimitato trasformerebbe la stabilizzazione in rumore che crea
concentrazione davanti al fronte. Questo parametro è proprio del solutore di
dispersione; il solutore del flusso usa quello descritto nel manuale
[Flusso di fluidi](theory_fluid.md).

Il termine di avvezione di Galerkin viene assemblato in **forma avvettiva con un
pozzo di divergenza**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, con la velocità
nodale continua. Una velocità proveniente dal solutore del flusso non è mai
esattamente a divergenza nulla - su una mesh grossolana un `div(v)` di decine di
`1/s` vicino a pareti e spigoli è comune. Dove il flusso si espande
(`div(v) > 0`) l'avvezione semplice creerebbe concentrazione da sola, un motivo
a scacchiera che cresce da un passo all'altro quando il passo temporale è
piccolo; il pozzo lo annulla. Dove il flusso converge non si aggiunge alcun
termine: una correzione simmetrica `0.5 * div(v) * C` agirebbe lì come sorgente
e accumulerebbe il contaminante vicino alle pareti. Per una velocità
esattamente a divergenza nulla il pozzo è zero.

Con un coefficiente di diffusione trascurabile il trasporto è quasi avvezione
pura, che Galerkin con SUPG non mantiene monotona: aspettati sovra- e
sottoelongazioni, massime alle pareti con aderenza dove la velocità è nulla. Un
coefficiente di diffusione efficace (sezione 1.3) ne elimina la maggior parte.

A differenza del solutore del flusso, il solutore di dispersione applica la
ponderazione theta in modo coerente, quindi *Central difference (accurate)* è
qui un vero schema di Crank-Nicolson.

### 1.5 Condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componenti |
|---|---|---|---|
| Particle concentration | esplicita (Dirichlet) | punto, linea, superficie | Particle concentration `[kg/m^3]` |
| Particle rate | naturale, sorgente | punto, linea, superficie, volume | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | naturale (Neumann) | superficie | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | naturale (Robin) | superficie | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** mantiene i suoi nodi alla concentrazione data.
Assegnata a una superficie di ingresso con una tabella in funzione del tempo, è
il modo in cui si modella un rilascio: zero prima dell'evento, la concentrazione
di rilascio durante l'evento, zero dopo. È lo schema più utile di questo tipo di
problema, ed è quello su cui si basa il tutorial della sezione 3.

**Particle rate** è una sorgente ed è una **densità** in `kg/(m^3*s)` anziché un
totale - a differenza della condizione *Heat*, non viene distribuita sulla misura
dell'entità. Modella una sorgente che rilascia in modo continuo all'interno del
dominio.

**Particle flux (unit area)** prescrive il flusso di massa `q` che entra nel
dominio attraverso una superficie, `D * grad(C) . n = q` con `n` la normale
uscente. A differenza di *Particle rate* è un flusso per unità di area, quindi il
rilascio totale è `q * A`. Un `q` positivo è un'emissione - un film che evapora,
una parete che rilascia gas - ed è limitato dalla saturazione massima allo stesso
modo di un particle rate positivo, `q * ( 1 - C / C_sat )`. Un `q` negativo è un
tasso di rimozione fisso indipendente dalla concentrazione locale e può portarla
sotto zero; una deposizione proporzionale alla concentrazione è *Particle mass
transfer* con `C_inf = 0`. Su una parete il flusso entra solo per diffusione; su
una parte del contorno in cui il flusso entra, l'afflusso viene trattato come
fluido pulito che trasporta il flusso, quindi `q` diventa il flusso di massa
entrante totale e `C ~ q / |v . n|`. Assegnato a una superficie interna tra due
volumi, rilascia `q` per unità di area come sorgente superficiale.

**Particle mass transfer** scambia contaminante con un serbatoio ambientale,
`q = k_m * ( C_inf - C )`, dove `k_m` è il *Mass transfer coefficient* e `C_inf`
la *Ambient particle concentration*. È l'equivalente, per il contaminante, della
convezione semplice nella trasmissione del calore. Con `C_inf = 0` modella la
deposizione su una superficie con velocità di deposizione `k_m`; con
`C_inf > 0` uno scambio con l'aria ambiente attraverso un'apertura, oppure una
superficie che si dissolve mantenuta a `C_inf`. Valori tipici di `k_m` sono
`1e-4` - `1e-2 m/s` per la deposizione di particelle negli ambienti interni e
`1e-3` - `1e-1 m/s` per l'evaporazione in un gas in movimento. Un `k_m` grande
porta la superficie verso `C_inf` e si avvicina a una condizione *Particle
concentration*.

Ogni componente è una **tabella in funzione del tempo**, quindi un rilascio può
iniziare e terminare durante un'esecuzione transitoria.

**Contorni senza condizione.** Pareti e uscite senza una condizione *Particle
concentration*, *Particle flux* o *Particle mass transfer* mantengono la
condizione naturale di **flusso diffusivo nullo**: il contaminante non diffonde
attraverso una parete ed esce da un'uscita solo con il flusso.

Dove il flusso **entra** attraverso un tale contorno - tipicamente un riflusso su
una parte di un'uscita con *Pressure (implicit)* - viene trattato come fluido
pulito: il solutore aggiunge su quella parte della superficie il termine debole
di afflusso `max(-v . n, 0) * C`, distribuito sulla faccia in modo da smorzare
anche i nodi del bordo dove l'uscita incontra una parete con aderenza.
Senza di esso il contorno alimenterebbe la soluzione e la concentrazione
all'uscita crescerebbe senza limiti. Se il fluido che rientra non è pulito,
estendi il dominio in modo che l'uscita veda solo deflusso.

Le condizioni del flusso - pareti, ingressi e uscite - appartengono al task del
flusso e sono descritte nel manuale [Flusso di fluidi](theory_fluid.md).

### 1.6 Condizioni iniziali

| Condizione iniziale | Effetto |
|---|---|
| Particle concentration | campo di concentrazione iniziale |

Le condizioni iniziali vengono lette **solo alla prima esecuzione**; in seguito
il campo avanza a partire dal risultato precedente. Una condizione al contorno
prevale su una condizione iniziale dove le due si incontrano.

### 1.7 Stazionaria o transitoria?

La dispersione di contaminanti è soprattutto un problema **transitorio**: un
problema di trasporto nel tempo, senza una risposta stazionaria significativa per
un evento di rilascio. Una dispersione stazionaria risponde a una domanda
diversa - il campo di concentrazione a cui si assesta una sorgente continua - ed
è una singola soluzione lineare sul campo di flusso (vedi la sezione 2.6 sul suo
solutore matriciale).

Il flusso di lavoro in due fasi del tutorial della sezione 3 è quello canonico:
far convergere il flusso in regime stazionario, poi riavviare con il solutore
temporale e il task di dispersione attivati.

### 1.8 Risultati

| Risultato | Si applica a | Significato |
|---|---|---|
| Particle concentration `[kg/m^3]` | nodo | il campo di concentrazione risolto |
| Relative saturation `[-]` | nodo | `C / C_sat`, solo quando è impostata una saturazione massima |

La saturazione relativa viene visualizzata da `0` a `1` per impostazione
predefinita e viene rimossa quando la saturazione massima viene riportata a zero.

Per questo tipo di problema è richiesta solo la proprietà del materiale
**densità**.

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Condizioni al contorno`, `Condizioni iniziali`, `Materiale`, `Risultati`), nel
menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Inserisci un task **Contaminant dispersion** nello
stesso gruppo di task di un task **Incompressible viscous flow** - dispersione
sul flusso, la classica coppia transitoria. Il tipo di problema richiede la
presenza del task del flusso.

I valori del gruppo di task - **N. di iterazioni** e **Convergenza** - guidano
l'iterazione non lineare del flusso e sono descritti nel manuale
[Flusso di fluidi](theory_fluid.md). In un'esecuzione transitoria sono le
iterazioni *per passo temporale*.

### 2.2 Scheda Problema

**Solutore temporale** - lascialo disattivato per una dispersione stazionaria,
attivalo per un evento di rilascio.

| Campo | Significato |
|---|---|
| Abilita | attiva e disattiva l'avanzamento transitorio |
| Approssimazione | theta dell'avanzamento nel tempo: all'indietro, centrale o in avanti |
| Tempo iniziale | primo valore di tempo in secondi |
| Tempo finale | `start time + time-step size * number of time-steps`, sola lettura |
| Dimensione del passo temporale | `dt` in secondi |
| Numero di passi temporali | quanti passi calcolare |
| Frequenza di output | scrive un file di risultati ogni N passi; `0` scrive solo l'ultimo passo |

**Impostazioni della dispersione di contaminanti** - mostrato quando nel flusso
dei task è presente un task *Contaminant dispersion*. Contiene i due parametri
della sezione 1.3:

| Campo | Significato |
|---|---|
| Saturazione massima `[kg/m^3]` | massima concentrazione possibile; un particle rate positivo si arresta a questo valore e viene calcolata la *Relative saturation*. `0` significa illimitata |
| Coefficiente di diffusione `[m^2/s]` | diffusività efficace del contaminante nel fluido. `0` significa avvezione pura |

I valori tipici di entrambi sono elencati nella sezione 1.3.

### 2.3 Scheda Materiale

| Proprietà | Unità | Commento |
|---|---|---|
| Density | `kg/m^3` | `rho`, circa `1000` per l'acqua, `1.2` per l'aria |

Il task del flusso nello stesso problema richiede anche la viscosità dinamica. Il
database dei materiali fornito include **Water** e **Air**, il che di solito
esaurisce questo passaggio.

### 2.4 Scheda Condizioni al contorno

Le condizioni della sezione 1.5 vengono offerte quando nel flusso dei task è
presente un task *Contaminant dispersion*. Seleziona prima un'entità
nell'albero `Modello`; il gestore offre allora le condizioni valide per quel
tipo di entità. Attivane una per assegnarla, poi modificane le componenti
nell'albero inferiore.

Ogni componente è una tabella in funzione del tempo. Fai clic su **Modifica
valori dipendenti dal tempo** per aprire l'editor dei componenti, in cui si
inseriscono i valori in funzione del tempo; tra due istanti dati il valore viene
interpolato linearmente, e dopo l'ultimo istante si mantiene l'ultimo valore. È
così che si modella un evento di rilascio.

### 2.5 Condizioni iniziali

La scheda `Condizioni iniziali` offre *Particle concentration*. Imposta il campo
di partenza della prima esecuzione.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Il solutore di dispersione usa la voce **GMRES**, poiché la
sua matrice non è simmetrica.

Una **dispersione di contaminanti stazionaria con diffusione** richiede qui
attenzione. È una singola soluzione lineare, quindi non ci sono iterazioni del
gruppo di task su cui ripiegare, e un sistema dominato dalla diffusione converge
lentamente con il limite predefinito di GMRES di 10 iterazioni interne per 10
esterne. Il solutore matriciale non avvisa quando raggiunge il limite: leggi la
sua tabella delle iterazioni nel log del solutore e, se arriva all'ultima
iterazione esterna con il residuo ancora ben sopra il valore di convergenza del
solutore, aumenta il numero di iterazioni esterne. Una soluzione non convergente
dà una risposta sbagliata senza alcun altro segnale.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare - *Particle concentration*, *Particle
rate* o *Relative saturation*. In un'esecuzione transitoria l'andamento è
mostrato da `Report` -> `Punti di monitoraggio`.

Un punto di monitoraggio nella posizione di interesse dà la risposta nella forma
in cui di solito la si vuole: la concentrazione in funzione del tempo in un dato
luogo, letta da un grafico anziché da una sequenza di immagini.

### 2.8 Risultati e record

La scheda `Risultati` elenca le variabili calcolate e controlla la vista 3D.
Concentrazione di particelle e saturazione relativa sono scalari nodali.

La scheda `Record` del pannello `Modello` elenca i record dei risultati - uno per
ogni passo temporale scritto di un'esecuzione transitoria. L'animazione dei
record riproduce la dispersione, e il pulsante di registrazione in fondo
all'albero scrive l'animazione in un file video.

---

## 3. Tutorial - flusso transitorio con dispersione di contaminanti

**Obiettivo.** Rilasciare un contaminante in un flusso convergente e seguire la
nube nel tempo attraverso un canale.

È il flusso di lavoro in due fasi della sezione 1.7 e corrisponde al tutorial
fornito *Dispersione di contaminanti nei fluidi*.

### Passo 1 - partire da un flusso convergente

Esegui prima fino a convergenza il tutorial del canale stazionario del manuale
[Flusso di fluidi](theory_fluid.md). Un'esecuzione di dispersione avviata da
fermo spreca i primi passi temporali per sviluppare il campo di flusso anziché
trasportare qualcosa.

### Passo 2 - aggiungere il task di dispersione

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Mantieni il gruppo di task e il task **Incompressible viscous flow**.
3. Aggiungi un task **Contaminant dispersion** all'interno dello stesso gruppo.
4. Riduci il **N. di iterazioni** del gruppo a circa `20` - queste sono ora le
   iterazioni non lineari *per passo temporale*, non le iterazioni di una
   soluzione stazionaria. Non modificare il valore di **Convergenza**; la maggior
   parte dei passi si stabilizzerà prima del ventesimo passaggio e terminerà in
   anticipo, e quelli che non lo fanno sono proprio quelli che meritano tutti e
   venti.

*Contaminant dispersion* richiede la presenza del task del flusso; legge il campo
di velocità e non lo calcola mai.

### Passo 3 - configurare il solutore temporale

Apri la scheda `Problema` e nel riquadro *Solutore temporale* imposta:

| Campo | Valore |
|---|---|
| Abilita | attivato |
| Approssimazione | `Backward difference (stable)` |
| Tempo iniziale | `0` |
| Dimensione del passo temporale | secondo l'indicazione di Courant sotto |
| Numero di passi temporali | sufficienti a portare la nube attraverso il dominio |
| Frequenza di output | `1` per un'animazione fluida, di più per un file più piccolo |

Scegli il passo temporale in base alla mesh e alla velocità del flusso, in modo
che il fluido attraversi circa un elemento per passo:

```
dt ~ h / |v|
```

Poi scegli il numero di passi in base al tempo che la nube impiega ad
attraversare il dominio, `L / |v|`, diviso per quel passo.

Nel riquadro *Impostazioni della dispersione di contaminanti* della stessa
scheda lascia entrambi i valori a `0` per l'avvezione pura, oppure inserisci un
**Coefficiente di diffusione** efficace dalla tabella della sezione 1.3 se la
larghezza della nube è importante. Una **Saturazione massima** ha senso solo per
un vapore o una sostanza disciolta; non deve essere inferiore alla
concentrazione di rilascio del passo 4.

### Passo 4 - rilasciare il contaminante

1. Seleziona l'entità superficie di ingresso.
2. Apri la scheda `Condizioni al contorno` e attiva **Particle concentration**.
3. Invece di inserire un singolo valore, fai clic su **Modifica valori
   dipendenti dal tempo**.
4. Nell'editor dei componenti inserisci un profilo di rilascio - ad esempio `0`
   al tempo `0`, la concentrazione di rilascio nell'istante in cui il rilascio
   inizia, e di nuovo `0` quando termina.

Tra due istanti dati il valore viene interpolato linearmente, e dopo l'ultimo
istante si mantiene l'ultimo valore, quindi un rilascio che deve iniziare
bruscamente richiede due voci ravvicinate. Questa condizione dipendente dal tempo
è ciò che trasforma un ingresso stazionario in un evento di rilascio, ed è il
cuore di un modello di dispersione.

### Passo 5 - mantenere le condizioni del flusso

La **Wall**, la condizione di ingresso e l'uscita con **Pressure (implicit)**
restano invariate. Il flusso viene ancora risolto a ogni passo temporale - la
dispersione viaggia su di esso.

### Passo 6 - aggiungere un punto di monitoraggio

`Problema` -> `Definisci punti di monitoraggio`, colloca un punto a valle e
imposta la sua variabile su **Particle concentration**. Il tempo di arrivo e la
concentrazione di picco in un dato luogo si leggono molto più facilmente da un
andamento che da una sequenza di immagini colorate.

### Passo 7 - risolvere con riavvio

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Attiva **Riavvia solutore / continua**.

Il riavvio è ciò che rende il campo stazionario convergente il punto di partenza
dell'avanzamento. Senza di esso l'esecuzione parte da fermo e i primi passi sono
sprecati.

### Passo 8 - esaminare i risultati

- Pannello `Modello`, scheda `Record`: un record per ogni passo scritto.
  Scorrili uno per uno o animali per vedere la nube muoversi.
- Scheda `Risultati`: visualizza **Particle concentration**. Fissa l'intervallo
  di visualizzazione su tutti i record in modo che i colori abbiano lo stesso
  significato in ogni fotogramma - altrimenti l'animazione si riscala da sola e
  la nube sembra non attenuarsi.
- `Report` -> `Punti di monitoraggio` traccia l'andamento della concentrazione
  nella sonda. Leggi il tempo di arrivo e confrontalo con `L / |v|`, il tempo di
  percorrenza implicato dal flusso medio - è la validazione più economica
  dell'intero modello.
- Con un coefficiente di diffusione nullo la nube si allarga solo per diffusione
  numerica, quindi la sua **larghezza** è una proprietà della mesh e del passo
  temporale, non della fisica. Con un coefficiente di diffusione efficace la
  larghezza è un risultato - verificala su una mesh più fine prima di fidartene.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| La concentrazione non lascia mai l'ingresso | il campo di flusso è nullo - il task del flusso manca dal gruppo, oppure l'esecuzione non è stata riavviata da un flusso convergente |
| La nube si appiattisce quasi subito | il numero di Courant è molto superiore a 1, oppure la mesh è troppo grossolana lungo il percorso |
| La concentrazione diventa negativa o ha sovraelongazioni | il passo temporale è troppo grande per la mesh, quindi l'avvezione stabilizzata oscilla. Lievi sottoelongazioni vicino a un fronte ripido sono normali; la soluzione non viene troncata a zero, perché il troncamento aggiungerebbe contaminante |
| È stato scritto un solo record | la frequenza di output è `0`, che scrive solo l'ultimo passo |
| Il residuo di ogni passo resta alto | troppe poche iterazioni per passo temporale - il flusso all'interno di ogni passo non è convergente |
| La nube non si allarga mai lateralmente | il coefficiente di diffusione è nullo, quindi l'allargamento laterale deriva solo dal campo di flusso - inserisci un coefficiente di diffusione efficace, vedi la sezione 1.3 |
| La nube si diffonde subito su tutto il dominio | il coefficiente di diffusione è decisamente troppo grande - confrontalo con `U * L` |
| Il log avvisa che la concentrazione prescritta supera la saturazione massima | una condizione al contorno o iniziale è sopra la saturazione massima; il limite agisce solo sul particle rate, quindi controlla entrambi i valori |
| Una soluzione stazionaria con diffusione sembra sbagliata | il solutore matriciale si è fermato al limite di iterazioni - aumenta le iterazioni esterne di GMRES, vedi la sezione 2.6 |
| La concentrazione supera il valore di ingresso o diventa negativa vicino alle pareti, ma non continua a crescere | avvezione quasi pura - il coefficiente di diffusione è prossimo a zero. Inserisci un coefficiente di diffusione efficace; su una mesh grossolana verifica anche che il campo di flusso sia convergente |
| La concentrazione supera il valore di ingresso e cresce in un motivo rumoroso | il flusso non è convergente - `Residual ratio` resta vicino a `1` a ogni passo. La sua velocità è lontana dall'avere divergenza nulla e nessun trasporto su di essa è affidabile; fai prima convergere il flusso (mesh, numero di Reynolds, iterazioni per passo) |

---

## 4. Verifica di un modello

**Tempo di percorrenza.** Il tempo che un marcatore impiega a percorrere una
distanza `L` alla velocità media `V` è `L/V`. Leggi il tempo di arrivo
dall'andamento di un punto di monitoraggio e confronta. Ciò conferma che il task
di dispersione sta viaggiando sul campo di flusso che pensi, e rileva
un'esecuzione non riavviata da un flusso convergente.

Il campo di flusso stesso è meglio verificarlo prima, con il bilancio di massa e
i flussi analitici elencati nel manuale [Flusso di fluidi](theory_fluid.md). Un
modello di flusso non convergente fallisce ogni verifica per lo stesso motivo, e
nessun trasporto su di esso è affidabile.

---

## 5. Limitazioni

- **Nessun modello di turbolenza.** Il flusso è laminare, vedi il manuale
  [Flusso di fluidi](theory_fluid.md), quindi il mescolamento di un flusso
  turbolento deve essere inserito come coefficiente di diffusione efficace.
- **Diffusione costante e isotropa.** Il coefficiente di diffusione è un unico
  valore per l'intero modello; non può variare nello spazio o nel tempo, seguire
  la direzione del flusso o derivare da un modello di turbolenza. Deve essere
  stimato come diffusività efficace - vedi la sezione 1.3.
- **Saturazione massima costante.** La saturazione non segue la temperatura di un
  risultato *Heat transfer in fluids* e limita solo un particle rate o un
  particle flux positivi. Le concentrazioni prescritte dalle condizioni non sono
  limitate, e il campo non viene troncato.
- **L'accoppiamento con il flusso è unidirezionale.** Il flusso guida il
  trasporto; il trasporto non modifica mai il flusso.
- **Non monotono per avvezione quasi pura.** Galerkin con SUPG produce sovra- e
  sottoelongazioni quando il coefficiente di diffusione è prossimo a zero,
  massime alle pareti con aderenza - vedi la sezione 1.4.
