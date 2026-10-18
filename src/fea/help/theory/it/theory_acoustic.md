# Analisi acustica - Manuale teorico e guida utente

Questo documento descrive il solutore acustico (`RSolverAcoustic`) di Range FEA:
le equazioni che risolve, il significato di ogni input che accetta, le parti
dell'interfaccia grafica che lo controllano e due tutorial svolti.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - analisi acustica transitoria](#3-tutorial---analisi-acustica-transitoria)
4. [Tutorial - analisi acustica armonica](#4-tutorial---analisi-acustica-armonica)
5. [Verifica](#5-verifica)
6. [Limitazioni](#6-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Incognita principale - il potenziale di velocità

Il solutore acustico non risolve direttamente la pressione. La sua incognita
principale è il **potenziale di velocità** `phi`, un campo scalare con unità
`m^2/s`. Entrambe le grandezze fisiche di interesse si ricavano da esso:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`particle velocity` = velocità delle particelle, `acoustic pressure` = pressione
acustica.) Usare un potenziale anziché la pressione ha due vantaggi pratici. Il
problema resta scalare anche se la velocità delle particelle è un vettore, e una
velocità di parete prescritta - il modo in cui si descrive la maggior parte delle
sorgenti acustiche - entra come condizione al contorno naturale anziché come
vincolo.

La convenzione di segno sopra (`u = -grad(phi)`, `p = +rho * dphi/dt`) è usata in
modo coerente in tutto il solutore e nei risultati memorizzati.

### 1.2 Equazione di governo

Il suono di piccola ampiezza in un fluido omogeneo in quiete obbedisce
all'equazione delle onde smorzata per il potenziale di velocità:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `phi` | potenziale di velocità | `m^2/s` | incognita |
| `c` | velocità del suono | `m/s` | proprietà del materiale |
| `rho` | densità del fluido | `kg/m^3` | proprietà del materiale |
| `beta` | fattore di smorzamento volumetrico | `1/s` | proprietà del materiale, predefinito `0` |

L'equazione è lineare. Di conseguenza una sola soluzione per passo temporale, o
per frequenza, è la risposta finale - non viene eseguita alcuna iterazione non
lineare esterna e `hasConverged()` riporta sempre successo una volta risolto il
sistema lineare.

**Velocità del suono.** `c` può essere data direttamente tramite la proprietà del
materiale *Speed of sound*. Se manca o è nulla, viene ricavata dalla rigidezza
volumetrica e dalla densità:

```
c = sqrt(E / rho)
```

dove `E` è la proprietà del materiale *Modulus of elasticity*. Per un gas, `E` è il
modulo di compressibilità adiabatico (`gamma * p0`, circa `1.42e5 Pa` per l'aria
a pressione atmosferica), che insieme a `rho = 1.2 kg/m^3` dà il noto
`c = 344 m/s`. Indicare `c` direttamente è di solito meno soggetto a errori.

Gli elementi privi di un materiale proprio - ad esempio una superficie di
contorno che delimita un volume meshato - ereditano i valori medi di `c` e `rho`
del dominio calcolabile, in modo che i termini di impedenza e di radiazione vi
vengano valutati correttamente.

### 1.3 Discretizzazione agli elementi finiti

Moltiplicando per una funzione peso `w`, integrando sul dominio, applicando il
teorema della divergenza e moltiplicando per `c^2` si ottiene la forma debole

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

che, sostituendo le funzioni di forma dell'elemento `N`, diventa il sistema del
secondo ordine

```
M * phi'' + C * phi' + K * phi = f
```

con

| Matrice | Definizione | Commento |
|---|---|---|
| `M` | `INT( N_m * N_n )` | matrice di massa |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | matrice di rigidezza |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | smorzamento volumetrico più impedenza di contorno |
| `f` | `c^2 * SURF( v_n * N_m )` | sorgente di velocità normale prescritta |

`M` e `K` sono semidefinite positive; `C` è semidefinita positiva finché il
rapporto di impedenza `zeta` è positivo.

**Misure degli elementi.** I termini di dominio (`M`, `K`, la parte volumetrica
di `C`) sono scalati per la misura geometrica dell'entità a cui appartengono:

| Entità | Misura | Dove si imposta |
|---|---|---|
| Volume | 1 | - |
| Superficie | spessore della superficie | proprietà geometriche dell'entità |
| Linea | area della sezione della linea | proprietà geometriche dell'entità |
| Punto | volume del punto | proprietà geometriche dell'entità |

I termini di contorno (la parte di impedenza di `C` e la sorgente `f`) agiscono
sulla misura semplice dell'elemento - l'area di un elemento di superficie, la
lunghezza di un elemento linea, l'area unitaria per un elemento punto. Una
superficie che delimita un volume meshato contribuisce quindi solo con i suoi
termini di contorno, poiché il suo spessore è nullo per impostazione predefinita.

Gli elementi punto non hanno punti di integrazione. I loro contributi vengono
concentrati direttamente sul loro unico nodo, il che consente il funzionamento
delle condizioni al contorno puntuali alle estremità di condotti monodimensionali.

### 1.4 Condizioni al contorno

| Condizione al contorno | Tipo | Si applica a | Componente |
|---|---|---|---|
| Velocity potential | esplicita (Dirichlet) | punto, linea, superficie | Potential `[m^2/s]` |
| Velocity | naturale (sorgente) | punto, linea, superficie | Velocity `[m/s]` |
| Absorbing boundary | naturale (smorzamento) | punto, linea, superficie | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | naturale (smorzamento) | punto, linea, superficie | Acoustic impedance `[Pa*s/m]` |

**Velocity.** La classica sorgente acustica: una superficie che vibra con una
velocità normale prescritta, come il cono di un altoparlante o un pistone. Un
valore positivo spinge il fluido, cioè punta *verso* il dominio acustico. Entra
nel termine noto come `f = c^2 * SURF(v_n * N)`.

Un nodo privo di qualsiasi condizione al contorno è una **parete rigida** ideale -
la condizione naturale della forma debole è una velocità normale nulla. Per
ottenere una cavità a pareti rigide non occorre assegnare nulla.

**Acoustic impedance.** Prescrive il rapporto tra la pressione e la velocità
normale uscente delle particelle sul contorno, `p = Z * u_n`. Internamente è
espressa tramite il **rapporto di impedenza specifica** adimensionale

```
zeta = Z / (rho * c)
```

e contribuisce con la matrice di smorzamento di contorno
`(c/zeta) * SURF(N_m * N_n)`. `Z = rho*c` (`zeta = 1`) è l'impedenza
caratteristica del fluido e dà una terminazione perfettamente non riflettente.
Per l'aria in condizioni ambiente `rho*c` è vicino a `415 Pa*s/m`, il valore che
la condizione al contorno propone per impostazione predefinita.

**Absorbing boundary.** Lo stesso termine, ma parametrizzato dal **coefficiente
di assorbimento** a incidenza normale `alpha` in `[0, 1]`, quello riportato nelle
schede tecniche dei materiali. Viene convertito in un rapporto di impedenza
tramite il coefficiente di riflessione dell'onda piana
`R = (zeta - 1) / (zeta + 1)` e `alpha = 1 - |R|^2`:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Comportamento |
|---|---|---|
| `1.0` | `1` | completamente assorbente, terminazione anecoica, nessuna riflessione |
| `0.5` | `5.83` | parzialmente assorbente |
| `0.1` | `38.0` | debolmente assorbente |
| `0.0` | infinito | parete rigida, nessuno smorzamento |

Usa `alpha = 1` su qualsiasi taglio artificiale di un dominio illimitato per
emulare la radiazione libera, e un `alpha` misurato su una parete rivestita. Se
alla stessa entità sono assegnate sia una condizione *Acoustic impedance* sia una
*Absorbing boundary*, prevale l'impedenza.

### 1.5 Condizioni iniziali

| Condizione iniziale | Imposta | Commento |
|---|---|---|
| Velocity potential | `phi` a `t = 0` | raramente diversa da zero da sola |
| Velocity potential / s | `dphi/dt` a `t = 0` | è ciò che imposta la **pressione** iniziale, `p = rho * dphi/dt` |

Entrambe vengono applicate solo al primo passo temporale. Per avviare
un'esecuzione transitoria da una regione pressurizzata, assegna a quell'entità
*Velocity potential / s* `= p0 / rho`. Le condizioni iniziali non hanno effetto
su un'analisi armonica.

### 1.6 Analisi transitoria

Il sistema del secondo ordine viene integrato con lo schema di **Newmark** con
`gamma = 1/2` e

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

(`time-march approximation coefficient` è il coefficiente dell'approssimazione
dell'avanzamento nel tempo). La limitazione garantisce `beta_N >= 1/4`, cioè
almeno la variante ad accelerazione media, incondizionatamente stabile, ed evita
una divisione per zero nell'approssimazione in avanti, il cui coefficiente è `0`.

Ogni passo risolve

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

con i consueti coefficienti

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

seguito dall'aggiornamento dello stato

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

La matrice del sistema è simmetrica definita positiva, quindi viene risolta con
il metodo del **gradiente coniugato** e precondizionamento di Jacobi.

`phi'` e `phi''` vengono scritti nel file dei risultati come *Potential / s* e
*Potential / s^2*. Sono lo stato dell'integratore temporale, quindi devono essere
presenti per un riavvio o perché il passo temporale successivo prosegua
correttamente.

**Un'analisi transitoria richiede un solutore temporale attivo.** Senza di esso i
termini di massa e di smorzamento scompaiono e il problema degenera in un
problema di Laplace privo di significato; il solutore lo segnala come errore
anziché produrre risultati senza senso.

**Scelta del passo temporale.** Lo schema è incondizionatamente stabile, quindi
`dt` è determinato dall'accuratezza, non dalla stabilità. Una regola utile è
risolvere la frequenza di interesse più alta `f_max` con almeno 20 passi per
periodo:

```
dt <= 1 / (20 * f_max)
```

**Scelta della mesh.** Risolvi la lunghezza d'onda più corta
`lambda_min = c / f_max` con almeno 10 elementi lineari:

```
h <= c / (10 * f_max)
```

### 1.7 Analisi armonica

Per una risposta stazionaria a una singola pulsazione `omega = 2*pi*f`, si scrive
`phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Allora `dphi/dt -> i*omega*PHI` e
`d2phi/dt2 -> -omega^2*PHI`, e il sistema diventa il problema complesso di
**Helmholtz**

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

I solutori matriciali di Range FEA sono reali, quindi il sistema complesso viene
risolto come sistema reale a blocchi equivalente di dimensione doppia. Con
`S = K - omega^2*M` e `D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Questa matrice a blocchi non è simmetrica e, sopra il limite statico, è
indefinita, quindi viene risolta con **GMRES** e precondizionamento di Jacobi.
GMRES con riavvio ristagna sui sistemi indefiniti ogni volta che lo spazio di
Krylov è molto più piccolo del sistema, quindi il solutore allarga
automaticamente il riavvio di GMRES alla dimensione del sistema, entro un budget
fisso di 256 MB per le due basi di Krylov. Il riavvio configurato viene solo
aumentato, mai ridotto, e la modifica è riportata nel log del solutore:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

Un'analisi armonica percorre un elenco di frequenze

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(`first frequency` = prima frequenza, `frequency step` = passo di frequenza,
`number of frequencies` = numero di frequenze.) Ogni frequenza è una soluzione
indipendente e produce un proprio record di risultati, così come un'analisi
transitoria produce un record per passo temporale. Il solutore temporale non ha
alcun ruolo; se è attivo viene saltato con un avviso, e le sue impostazioni
restano invariate, così da non perderle tornando a un'analisi transitoria.

### 1.8 Risultati derivati

| Risultato | Si applica a | Transitoria | Armonica |
|---|---|---|---|
| Potential `[m^2/s]` | nodo | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | nodo | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | nodo | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | nodo | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | nodo | `rho * dphi/dt`, istantanea | `omega * rho * abs(PHI)`, ampiezza |
| Acoustic phase `[Deg]` | nodo | - | `arg(P)` |
| Sound pressure level `[dB]` | nodo | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | elemento | `-grad(phi)` | ampiezza di `-grad(PHI)`, per componente |
| Acoustic intensity `[W/m^2]` | elemento | `p * u`, istantanea | `0.5 * Re{P * conj(U)}`, media nel tempo |

Il **livello di pressione sonora** è calcolato rispetto alla *Pressione di
riferimento* delle impostazioni acustiche, `2e-5 Pa` per impostazione
predefinita - la soglia di udibilità in aria. In un'analisi armonica si usa il
valore efficace del segnale armonico, cioè l'ampiezza divisa per `sqrt(2)`; in
un'analisi transitoria si usa il modulo istantaneo, quindi il livello riportato
oscilla all'interno di un periodo.

L'**intensità acustica** in un'analisi armonica è l'intensità attiva mediata nel
tempo, cioè il flusso netto di potenza acustica. È identicamente nulla in un'onda
stazionaria pura ed è pari a `0.5 * |p| * |u|` in un'onda piana progressiva.

### 1.9 Transitoria o armonica?

| | Transitoria | Armonica |
|---|---|---|
| Risolve | `M*phi'' + C*phi' + K*phi = f` nel tempo | `(K - w^2*M + i*w*C)*PHI = F` per frequenza |
| Risponde a | «che cosa succede quando ...» | «qual è la risposta stazionaria a ...» |
| Eccitazione | qualsiasi andamento nel tempo, compreso un singolo impulso | una sinusoide alla volta |
| Costo | una soluzione SPD per passo temporale | una soluzione indefinita di dimensione doppia per frequenza |
| Record | uno per passo temporale | uno per frequenza |
| Informazione di fase | implicita nell'andamento temporale | esplicita, come campo di risultato |

**Usa un'analisi transitoria per**

- studi di risposta all'impulso, eco e riverberazione;
- eventi transitori - uno scoppio, la chiusura di una valvola, una porta che
  sbatte;
- eccitazione a banda larga, dove una sola esecuzione seguita da una trasformata
  di Fourier di un punto di monitoraggio copre l'intero spettro in una volta;
- qualsiasi caso in cui l'andamento nel tempo stesso è la risposta.

**Usa un'analisi armonica per**

- funzioni di risposta in frequenza e curve di perdita di trasmissione;
- risonanze di cavità e condotti, e la distribuzione della pressione a una
  risonanza;
- prestazioni di marmitte, silenziatori e assorbitori su una banda di frequenze;
- mappe del livello di pressione sonora a un singolo tono;
- qualsiasi caso in cui conta solo lo stato stazionario. Un'esecuzione armonica è
  di solito molto più economica che far avanzare un transitorio abbastanza a lungo
  da estinguere i transitori.

Un compromesso pratico consiste nell'individuare le risonanze con una scansione
armonica grossolana e poi raffinare solo la banda interessante.

---

## 2. Interfaccia grafica

Tutto ciò che riguarda l'acustica si trova nel pannello **Solutore** (schede
`Problema`, `Condizioni al contorno`, `Condizioni iniziali`, `Materiale`,
`Risultati`), nel menu **Problema** e nel pannello **Modello**.

### 2.1 Selezione del problema acustico

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Aggiungi un task e scegli **Acoustics** -
"Customized wave equation for solving acoustic problems" (equazione delle onde
personalizzata per la risoluzione di problemi acustici).

L'acustica è mutuamente esclusiva con **Wave - general** e con **Modal
analysis**. Può essere combinata con altre fisiche, nel qual caso il flusso dei
task determina l'ordine di esecuzione.

### 2.2 Scheda Problema

Una volta selezionata l'acustica, la scheda `Problema` del pannello Solutore
mostra due riquadri.

**Solutore temporale** - mostrato perché l'acustica è un tipo di problema
dipendente dal tempo.

| Campo | Significato |
|---|---|
| Abilita | attiva e disattiva l'avanzamento transitorio nel tempo |
| Approssimazione | approssimazione dell'avanzamento nel tempo; corrisponde al `beta_N` di Newmark come descritto in 1.6 |
| Tempo iniziale | primo valore di tempo in secondi; ignorato al riavvio del solutore |
| Tempo finale | `start time + time-step size * number of time-steps`, sola lettura |
| Dimensione del passo temporale | `dt` in secondi |
| Numero di passi temporali | quanti passi calcolare |
| Frequenza di output | scrive un file di risultati ogni N passi; `0` scrive solo l'ultimo passo |

**Impostazioni dell'analisi acustica** - il riquadro aggiunto per l'acustica.

| Campo | Significato |
|---|---|
| Tipo di analisi | *Transient* o *Harmonic* |
| Prima frequenza `[Hz]` | prima frequenza della scansione armonica |
| Passo di frequenza `[Hz]` | incremento tra due frequenze consecutive |
| Numero di frequenze | quante frequenze risolvere; ciascuna produce un proprio record |
| Scansione | etichetta di sola lettura che mostra l'intervallo di frequenze risultante |
| Pressione di riferimento `[Pa]` | riferimento usato per il livello di pressione sonora, `2e-5 Pa` per impostazione predefinita |

I tre campi di frequenza sono disattivati mentre è selezionato *Transient*. La
pressione di riferimento si applica a entrambi i tipi di analisi.

### 2.3 Scheda Materiale

La scheda `Materiale` elenca il materiale assegnato all'entità selezionata e le
sue proprietà. Per l'acustica le proprietà rilevanti sono:

| Proprietà | Unità | Richiesta | Commento |
|---|---|---|---|
| Density | `kg/m^3` | sì | `rho` |
| Speed of sound | `m/s` | una delle due | `c`, usata direttamente se non nulla |
| Modulus of elasticity | `Pa` | una delle due | alternativa, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | no | `beta`, attenuazione volumetrica, `0` per impostazione predefinita |

Un'entità viene risolta dal solutore acustico quando ha una densità **e** una
velocità del suono o un modulo di elasticità. Poiché le due sono alternative,
nessuna viene segnalata come mancante dal controllo della configurazione; un
modello che non ha nessuna delle due viene rifiutato dal solutore con un
messaggio esplicito.

Ogni proprietà è una tabella in funzione della temperatura, quindi si può
inserire una velocità del suono dipendente dalla temperatura, che verrà
interpolata se in parallelo viene eseguito un problema di trasmissione del
calore.

### 2.4 Scheda Condizioni al contorno

La scheda è suddivisa in un elenco gestore delle condizioni al contorno
disponibili in alto e in un editor per quella selezionata in basso. Seleziona
prima un'entità nell'albero `Modello`; il gestore offre allora le condizioni al
contorno valide per quel tipo di entità e per il problema selezionato. Attivane
una per assegnarla, poi modificane le componenti nell'albero inferiore.

Per l'acustica il gestore offre:

- **Velocity potential** - esplicita, punti, linee e superfici. Componente:
  *Potential* `[m^2/s]`.
- **Velocity** - punti, linee e superfici. Componente: *Velocity* `[m/s]`.
- **Absorbing boundary** - punti, linee e superfici. Componente: *Acoustic
  absorption coefficient*, adimensionale, predefinito `1.0`.
- **Acoustic impedance** - punti, linee e superfici. Componente: *Acoustic
  impedance* `[Pa*s/m]`, predefinito `415`.

Ogni componente è una tabella in funzione del tempo, quindi a una sorgente si può
dare un andamento nel tempo: aggiungi righe per aumentare gradualmente una
velocità o per generare un breve impulso. In un'analisi armonica si usa il valore
al tempo corrente del solutore, che per una tabella con un solo valore è
semplicemente quel valore.

Lascia una parete senza alcuna condizione al contorno per renderla acusticamente
rigida.

### 2.5 Scheda Condizioni iniziali

Stessa struttura della scheda delle condizioni al contorno. Per l'acustica offre
**Velocity potential** e **Velocity potential / s**. Quest'ultima è quella che
imposta il campo di pressione iniziale.

### 2.6 Configurazione del solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi` configura i
solutori iterativi. Un'analisi acustica transitoria usa la voce **CG**,
un'analisi armonica usa la voce **GMRES**.

Il riavvio predefinito di GMRES pari a `10` è decisamente troppo piccolo per il
sistema a blocchi nel dominio della frequenza. Il solutore lo aumenta
automaticamente, ma il numero di iterazioni *esterne* e il valore di convergenza
vengono presi come configurati, quindi aumentali qui se un'esecuzione armonica
non converge.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare. Un punto di monitoraggio su *Acoustic
pressure* è il microfono naturale: in un'esecuzione transitoria produce
l'andamento nel tempo della pressione in quel punto, che si può esaminare tramite
`Report` -> `Punti di monitoraggio`.

### 2.8 Risultati

Dopo l'esecuzione, la scheda `Risultati` elenca le variabili calcolate e
controlla ciò che mostra la vista 3D. Le variabili acustiche sono quelle della
sezione 1.8.

Le variabili nodali (pressione, livello di pressione sonora, fase, potenziale)
vengono disegnate come campi continui; le variabili di elemento (velocità delle
particelle, intensità) sono vettori e si mostrano al meglio tramite un campo
vettoriale o una sezione.

La scheda `Record` del pannello `Modello` elenca i singoli record di risultati -
uno per passo temporale in un'esecuzione transitoria, uno per frequenza in una
scansione armonica - e permette di scorrerli o animarli.

`Report` -> `File di log del solutore` mostra l'output completo del solutore,
compreso il blocco di statistiche stampato per ogni variabile acustica dopo ogni
record.

---

## 3. Tutorial - analisi acustica transitoria

**Obiettivo.** Calcolare la risposta all'impulso di una cavità d'aria chiusa e
leggere l'andamento della pressione nella posizione di un microfono.

Questo tutorial presuppone un modello di volume meshato. Usa `File` -> `Nuovo
modello`, disegna un parallelepipedo con `Geometria` -> `Crea elemento` o importa
una geometria, e tetraedrizzala - il tutorial *Disegnare un cubo* tratta questi
passaggi.

### Passo 1 - selezionare il problema

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Aggiungi un task e seleziona **Acoustics**.
3. Conferma con `OK`.

La scheda `Problema` del pannello Solutore mostra ora i riquadri *Solutore
temporale* e *Impostazioni dell'analisi acustica*.

### Passo 2 - assegnare il materiale

1. Seleziona l'entità volume nell'albero `Modello`.
2. Apri la scheda `Materiale`.
3. Assegna un materiale, o creane uno, e assegnagli:
   - **Density** = `1.2` `kg/m^3`
   - **Speed of sound** = `344` `m/s`
4. Lascia **Acoustic damping factor** a `0` per una cavità non smorzata, oppure
   impostalo a un valore piccolo come `1.0` `1/s` per far decadere la risposta.

### Passo 3 - scegliere il tipo di analisi e il passo temporale

1. Apri la scheda `Problema`.
2. In *Impostazioni dell'analisi acustica* imposta **Tipo di analisi** =
   `Transient`.
3. In *Solutore temporale*:
   - **Abilita**: attivato;
   - **Tempo iniziale** = `0`;
   - **Dimensione del passo temporale** = `2e-5` s. Secondo
     `dt <= 1/(20*f_max)` ciò risolve frequenze fino a circa `2.5 kHz`;
   - **Numero di passi temporali** = `500`, che dà `10 ms` di tempo simulato;
   - **Frequenza di output** = `5`, che scrive 100 record di risultati.

Verifica la mesh con `h <= c/(10*f_max)`: per `f_max = 2.5 kHz` e
`c = 344 m/s` la dimensione degli elementi non dovrebbe superare circa `14 mm`.

### Passo 4 - eccitare la cavità

Due opzioni, scegline una.

**A - una parete mobile.** Seleziona la superficie che rappresenta la sorgente,
apri la scheda `Condizioni al contorno` e attiva **Velocity**. Inserisci un
andamento nel tempo nella tabella dei componenti, ad esempio un breve impulso:

| Tempo `[s]` | Velocity `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - una regione inizialmente pressurizzata.** Seleziona l'entità che deve
partire pressurizzata, apri la scheda `Condizioni iniziali` e attiva **Velocity
potential / s**. Inserisci `p0 / rho`; per `p0 = 1 Pa` e `rho = 1.2 kg/m^3` sono
`0.833`.

Lascia tutte le altre superfici senza condizione al contorno, così la cavità ha
pareti rigide, oppure attiva **Absorbing boundary** con un coefficiente di
assorbimento corrispondente al rivestimento della parete per lasciare uscire
energia.

### Passo 5 - aggiungere un microfono

1. `Problema` -> `Definisci punti di monitoraggio`.
2. Aggiungi un punto nella posizione di interesse.
3. Seleziona **Acoustic pressure** come sua variabile.

### Passo 6 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Il controllo della configurazione segnala materiali o condizioni al contorno
   mancanti prima dell'avvio dell'esecuzione.
3. Segui l'avanzamento in `Report` -> `File di log del solutore`. Ogni passo
   temporale stampa la convergenza del CG e le statistiche di ogni variabile
   acustica.

### Passo 7 - esaminare i risultati

- Pannello `Modello`, scheda `Record`: scorri o anima i record nel tempo.
- Scheda `Risultati`: visualizza **Acoustic pressure** o **Sound pressure level**
  sul modello, e **Acoustic particle velocity** come campo vettoriale.
- `Report` -> `Punti di monitoraggio`: l'andamento della pressione al microfono.
  Il suo tasso di decadimento dà il tempo di riverberazione; la sua trasformata
  di Fourier dà le risonanze della cavità.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* | *Abilita* non è attivo in *Solutore temporale*, oppure il tipo di analisi dovrebbe essere *Harmonic* |
| *"Acoustic solver requires a positive time step size"* | la dimensione del passo temporale è nulla |
| *"No element with a valid speed of sound was found"* | nessuna entità ha una densità insieme a una velocità del suono o a un modulo di elasticità |
| La risposta non decade mai | nessuno smorzamento - previsto per una cavità perfettamente rigida e senza perdite; aggiungi un contorno assorbente o un fattore di smorzamento acustico |
| Oscillazioni davanti al fronte d'onda | la mesh o il passo temporale sono troppo grossolani per il contenuto in frequenza; raffina entrambi |

---

## 4. Tutorial - analisi acustica armonica

**Obiettivo.** Calcolare la risposta in frequenza di un condotto con terminazione
anecoica e una mappa del livello di pressione sonora a ogni frequenza.

### Passo 1 - selezionare il problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`), aggiungi un task e
seleziona **Acoustics**.

### Passo 2 - assegnare il materiale

Come nel tutorial transitorio: **Density** `1.2 kg/m^3` e **Speed of sound**
`344 m/s` sull'entità fluida.

Dimensiona la mesh per la frequenza **più alta** della scansione:
`h <= c / (10 * f_max)`.

### Passo 3 - configurare la scansione

Apri la scheda `Problema` e in *Impostazioni dell'analisi acustica* imposta:

| Campo | Valore |
|---|---|
| Tipo di analisi | `Harmonic` |
| Prima frequenza | `50` Hz |
| Passo di frequenza | `50` Hz |
| Numero di frequenze | `20` |
| Pressione di riferimento | `2e-5` Pa |

L'etichetta *Scansione* conferma `50 - 1000 [Hz]`, cioè 20 soluzioni e 20 record
di risultati.

Il riquadro *Solutore temporale* può restare attivo - un'analisi armonica lo
ignora e scrive un avviso nel log. Le sue impostazioni vengono conservate.

### Passo 4 - eccitare il condotto

Seleziona la superficie di ingresso, apri la scheda `Condizioni al contorno` e
attiva **Velocity**. Inserisci un'unica riga con la velocità del pistone, ad
esempio `0.01` `m/s`. L'ampiezza viene interpretata come ampiezza
dell'eccitazione armonica a ogni frequenza della scansione.

### Passo 5 - terminare il condotto

Seleziona la superficie di uscita e attiva una delle due:

- **Absorbing boundary** con **Acoustic absorption coefficient** = `1.0` per una
  terminazione perfettamente non riflettente; oppure
- **Acoustic impedance** con **Acoustic impedance** = `rho * c` = `413`
  `Pa*s/m`, che è la stessa cosa espressa diversamente.

Usa un coefficiente di assorbimento più piccolo per modellare una terminazione
parzialmente riflettente, e lascia l'uscita senza alcuna condizione per
modellare un'estremità chiusa e rigida.

Le pareti del condotto non richiedono condizioni al contorno - sono rigide per
impostazione predefinita.

### Passo 6 - controllare il solutore matriciale

`Problema` -> `Configura il solutore matriciale dei problemi`, voce **GMRES**. Il
riavvio viene aumentato automaticamente, ma se il log mostra un residuo che
ristagna, aumenta il numero di iterazioni esterne e allenta o restringi il
valore di convergenza secondo necessità.

### Passo 7 - risolvere

`Soluzione` -> `Avvia solutore` (`Ctrl+R`). Il log mostra un blocco per
frequenza:

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

### Passo 8 - esaminare i risultati

- Pannello `Modello`, scheda `Record`: un record per frequenza. Scorrerli
  equivale alla scansione in frequenza.
- Scheda `Risultati`: visualizza **Sound pressure level** per una mappa in
  decibel, **Acoustic pressure** per l'ampiezza e **Acoustic phase** per vedere i
  fronti d'onda.
- **Acoustic intensity** mostra dove fluisce la potenza acustica. In un condotto
  ben terminato è uniforme e diretta verso l'uscita; in una cavità risonante è
  quasi nulla.
- Un punto di monitoraggio su **Acoustic pressure** registra l'ampiezza a ogni
  frequenza, cioè la funzione di risposta in frequenza in quel punto.

### Interpretazione del risultato

Per un condotto rettilineo eccitato con `v` e con terminazione anecoica, la
soluzione analitica per onde piane è

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`constant along the duct` = costante lungo il condotto, `with` = con.) Per
`rho = 1.2`, `c = 344`, `v = 0.01` si ottiene `|p| = 4.13 Pa`, un livello di
pressione sonora di `103.3 dB` e un'intensità di `0.0206 W/m^2`. Scostamenti da
un `|p|` costante lungo il condotto indicano che la terminazione riflette - per
scelta, oppure perché il coefficiente di assorbimento non è `1`.

Con una terminazione rigida lo stesso condotto diventa risonante. L'ampiezza
della pressione ha picchi alle risonanze in quarto d'onda
`f = (2n+1) * c / (4L)` e la fase è costante a `+/-90` gradi, con intensità netta
nulla.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* | la prima frequenza è nulla e il passo di frequenza è nullo |
| Il residuo ristagna, i risultati sembrano rumorosi | GMRES non è converso; aumenta le iterazioni esterne nella configurazione del solutore matriciale, oppure rendi più grossolana la mesh |
| L'ampiezza esplode a una frequenza | la scansione ha colpito una risonanza non smorzata; aggiungi un contorno assorbente o un fattore di smorzamento acustico |
| La pressione varia fortemente lungo un condotto che dovrebbe essere anecoico | la terminazione riflette; controlla il coefficiente di assorbimento o il valore di impedenza |

---

## 5. Verifica

Il test unitario `tst_solver_acoustic` (in `range-solver-lib/tests`, compilato
con `-DRANGE_BUILD_TESTS=ON`) verifica il solutore rispetto a soluzioni in forma
chiusa su un condotto monodimensionale di `1 m` discretizzato con 200 elementi
asta, usando le impostazioni standard del solutore matriciale.

**Armonica, terminazione anecoica** - `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz`:

| Grandezza | Analitica | Calcolata |
|---|---|---|
| Ampiezza della pressione | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` in ogni nodo |
| Ampiezza del potenziale | `v/k = 1.0823e-3 m^2/s` | entro l'1 % |
| Velocità delle particelle | `0.01 m/s` | `0.009999 m/s` |
| Livello di pressione sonora | `103.18 dB` | entro `0.1 dB` |
| Intensità | `0.0204 W/m^2` | entro il 2 % |
| Gradiente di fase | `k * dx` | entro 5 gradi |

**Armonica, terminazione rigida** - l'onda stazionaria dà una fase esattamente di
`+/-90` gradi e un'intensità netta nulla, e l'ampiezza di picco supera l'ampiezza
dell'onda progressiva.

**Transitoria** - un condotto eccitato con terminazione anecoica resta limitato,
lo stato di Newmark viene pubblicato in modo che l'integrazione prosegua
correttamente tra i passi, e il campo eccitato è non nullo e finito ovunque.

---

## 6. Limitazioni

- **Nessun accoppiamento struttura-acustica.** L'acustica non può essere
  combinata con l'analisi modale, e non esiste un'interfaccia fluido-struttura con
  `RSolverStress`. Una struttura vibrante deve essere rappresentata da una
  condizione al contorno *Velocity* prescritta.
- **Solo acustica lineare.** Piccola ampiezza, mezzo in quiete. Nessun flusso
  medio, nessun effetto convettivo, nessuna propagazione non lineare.
- **Solo impedenza reale.** Le condizioni al contorno assorbente e di impedenza
  accettano un valore reale. Un'impedenza superficiale complessa e dipendente
  dalla frequenza non è supportata.
- **Dati del materiale indipendenti dalla frequenza.** La velocità del suono e il
  fattore di smorzamento sono tabulati in funzione della temperatura, non della
  frequenza, quindi sono costanti lungo una scansione.
- **I termini di contorno usano la misura semplice dell'elemento.** Per un modello
  mono- o bidimensionale con area della sezione o spessore diversi da `1`, i
  termini di sorgente e di impedenza non vengono scalati di conseguenza, mentre i
  termini di dominio sì. Usa un'area della sezione unitaria o un modello
  tridimensionale completo per evitare la discrepanza.
- **L'eccitazione armonica è reale.** Una velocità prescritta entra come parte
  reale del carico complesso; non è possibile definire sorgenti con uno
  sfasamento relativo.
