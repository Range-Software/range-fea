# Magnetostatica - Manuale teorico e guida utente

Questo documento descrive il solutore di magnetostatica (`RSolverMagnetostatics`)
di Range FEA, che gestisce il tipo di problema **Magnetostatica**
(*Magneto-statics*): la legge che valuta, gli input che accetta, le parti
dell'interfaccia grafica che lo controllano, un tutorial svolto e il valore del
suo risultato.

> **Stato.** Il solutore valuta direttamente, con la **legge di Biot-Savart**, il
> campo magnetico della corrente calcolata dal task di elettrostatica. Non c'è
> alcun sistema di equazioni né alcuna condizione al contorno: il campo è
> determinato dalla sola corrente, e la condizione che si annulli lontano dal
> conduttore è contenuta nella legge. Per la densità di corrente data, il
> risultato è esatto a meno di un errore di quadratura controllato ben al di
> sotto di `1e-4` del campo di picco. Il solutore lavora nello **spazio libero** -
> i materiali magnetici non possono essere rappresentati - e calcola solo il
> campo della corrente modellata, quindi il percorso della corrente dovrebbe
> essere chiuso all'interno del modello. La sezione 5 elenca le limitazioni.

**Indice**

1. [Fondamenti teorici](#1-fondamenti-teorici)
2. [Interfaccia grafica](#2-interfaccia-grafica)
3. [Tutorial - il campo di un conduttore percorso da corrente](#3-tutorial---il-campo-di-un-conduttore-percorso-da-corrente)
4. [Verifica di un modello](#4-verifica-di-un-modello)
5. [Limitazioni](#5-limitazioni)

---

## 1. Fondamenti teorici

### 1.1 Il risultato - il campo magnetico

Il risultato è la **densità di flusso magnetico** `B` in ogni nodo del modello,
in tesla, come vettore a tre componenti.

Viene **valutata, non risolta**. Una volta nota la densità di corrente, il campo
magnetico nello spazio libero ne deriva tramite un integrale, quindi il solutore
non assembla alcuna matrice e non esegue alcuna iterazione. Ogni nodo del
modello riceve un valore, che si trovi nel conduttore, sulla sua superficie o in
qualsiasi altro punto della mesh.

### 1.2 Legge di governo

Nello spazio libero il campo magnetico statico soddisfa la legge di Ampère e non
ha sorgenti:

```
curl(B) = u0 * J
div(B)  = 0
```

insieme alla condizione che `B` si annulli lontano da tutte le correnti. L'unico
campo che soddisfa tutte e tre è dato dalla **legge di Biot-Savart**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Simbolo | Significato | Unità | Origine |
|---|---|---|---|
| `B` | densità di flusso magnetico | `T` | calcolata |
| `J` | densità di corrente | `A/m^2` | dal task di elettrostatica, vedi 1.4 |
| `u0` | permeabilità del vuoto | `H/m` | integrata, `1.25663706212e-6` |
| `x` | punto di campo - un nodo del modello | `m` | |
| `y` | punto sorgente - all'interno di un elemento conduttore | `m` | |

Tre proprietà derivano dalla legge stessa e non da qualcosa che fa il solutore:

- **`div(B) = 0` vale esattamente**, ovunque, qualunque sia la mesh.
- **Il campo lontano è corretto senza contorno.** `B` decresce con la distanza
  come prescrive la legge; non c'è una superficie esterna, nessuna regione d'aria
  da meshare e nessuna condizione da impostare lì.
- **`curl(B) = u0 * J` vale ovunque la corrente sia chiusa**, cioè dove
  `div(J) = 0` e il percorso della corrente forma anelli. La sezione 1.4 spiega
  cosa significa per un modello elettrostatico, la cui corrente entra ed esce dal
  corpo.

La permeabilità del materiale non entra da nessuna parte. Non esiste una
proprietà del materiale permeabilità relativa, né magnetizzazione, né distinzione
tra `B` e `H` - il solutore lavora nello spazio libero, qualunque sia il
materiale del modello. La sezione 1.9 spiega perché ciò non può essere superato
con lo stesso approccio.

**Non è richiesta alcuna proprietà del materiale.** Il tipo di problema
magnetostatica non ne dichiara nessuna, e il controllo della configurazione non
ha nulla da verificare per esso. Il task di elettrostatica da cui dipende il tipo
di problema ha i propri tre requisiti - vedi il manuale di elettrostatica.

### 1.3 Discretizzazione

Il solutore di elettrostatica produce una densità di corrente **costante in ogni
elemento**. L'integrale di 1.2 si scompone quindi in un integrale per elemento,
ciascuno con un `J_e` costante che può essere portato fuori:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` dipende solo dalla geometria dell'elemento e dal punto di campo. Ogni tipo
di elemento accettato dal solutore ha facce piane, e per questi `G_e` ha una
forma chiusa, quindi il solutore integra ogni elemento **esattamente** anziché in
modo approssimato:

- **Tetraedro.** L'integrando è il gradiente di `1/|x - y|` rispetto a `y`,
  quindi l'integrale di volume diventa una somma sulle quattro facce,
  `G = SUM_f n_f * INT_f 1/R dA`. L'integrale di `1/R` su un triangolo piano è
  noto in forma chiusa: un logaritmo lungo ogni lato e l'angolo solido sotteso
  dal triangolo visto da `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Triangolo** di un'entità superficie con spessore `t`. La corrente è una
  corrente superficiale `K = J*t` in `A/m`. La parte tangenziale di `G` è di nuovo
  una somma di logaritmi dei lati, `SUM_e m_e * I_e`, e la parte normale è
  l'angolo solido con segno.
- **Quadrilatero.** Viene suddiviso in due triangoli.
- **Segmento** di un'entità linea con area della sezione `A`. La corrente è
  `I = A * (J . t)` lungo il segmento, e il campo di un filo rettilineo finito è
  classico:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  con `a` e `b` i vettori dal punto di campo alle due estremità.

Qui `I_e` è l'integrale di `1/R` lungo il lato `e`, `P_e` la distanza nel piano
del punto di campo da quel lato, `m_e` la normale uscente del lato nel piano, `d`
l'altezza del punto di campo sopra il piano della faccia e `omega` l'angolo
solido della faccia vista dal punto di campo.

**Gli elementi lontani** non richiedono la forma chiusa. Lì l'integrando è
regolare, e una regola di quadratura è praticamente esatta a una frazione del
costo:

| Distanza del punto di campo dal centro dell'elemento | Regola |
|---|---|
| meno di 2 x il lato più lungo dell'elemento | forma chiusa |
| da 2 a 6 x | regola di grado due - 3 punti su un triangolo, 4 su un tetraedro |
| oltre 6 x | regola del punto medio - l'elemento come corrente puntiforme |

I segmenti usano sempre la forma chiusa, che costa quanto un punto di
quadratura. L'errore combinato delle regole è misurato dai test unitari rispetto
alla forma chiusa sommata su tutti gli elementi di un intero conduttore, e resta
sotto `1e-4` del campo di picco in ogni nodo - dentro il conduttore, sulla sua
superficie e oltre.

**Il campo sul conduttore stesso.**

- Per una **corrente di volume** il campo è continuo e finito ovunque, anche nei
  nodi, sui lati e sulle facce degli elementi conduttori. Un nodo sulla superficie
  di un conduttore massiccio riceve il vero campo superficiale.
- Un'entità **linea** o **superficie** rappresenta un conduttore di sezione
  nulla, e il campo di un tale conduttore è singolare: cresce come `1/r` verso un
  filo e in modo logaritmico verso il bordo di una lamina, e salta di
  `u0 * K x n` attraverso una lamina. In un nodo su un tale conduttore il
  solutore restituisce un valore finito: un segmento non contribuisce a un punto
  sulla propria retta, una lamina contribuisce con la media dei campi sui suoi
  due lati, e il logaritmo del bordo di una lamina viene omesso in un nodo che
  giace su quel bordo. Questi valori sono regolarizzati, non fisici. Leggi il
  campo di un conduttore sottile a qualche dimensione della sezione di distanza,
  dove il modello sottile è valido.

### 1.4 La densità di corrente come sorgente

Il campo magnetico è generato da un solo input: la **densità di corrente**
lasciata da un task *Electro-statics* che lo precede nel flusso dei task del
problema. Non c'è nulla da inserire né da assegnare.

Il solutore legge la densità di corrente **di elemento** memorizzata dal task di
elettrostatica e la usa così com'è - non c'è media sui nodi. Gli elementi
percorsi da corrente sono:

| Entità | Condizione | Sorgente |
|---|---|---|
| Volume | tetraedri | `J` |
| Superficie | spessore `> 0` | corrente superficiale `J * thickness` |
| Linea | area della sezione `> 0` | corrente `J * cross area` |

Gli elementi con densità di corrente nulla vengono saltati, cioè ogni elemento
che il task di elettrostatica non ha calcolato. Lo stesso vale per gli elementi
la cui densità di corrente è inferiore a `1e-10` della massima del modello: una
regione poco conduttiva - aria con una conducibilità di `5e-15 S/m` accanto al
rame - aggiungerebbe altrimenti una sorgente per elemento e nulla di misurabile
al campo. Se nessun elemento è percorso da corrente - il task di elettrostatica
non è stato eseguito, i potenziali sono uguali o nessun materiale ha una
conducibilità elettrica - il log stampa un avviso e il campo è nullo ovunque.

**La corrente deve chiudersi.** La legge di Biot-Savart dà il campo della
corrente fornita e nient'altro. Una corrente fisica scorre in anelli chiusi, ma
un modello elettrostatico di solito contiene solo una parte dell'anello: una
barra tra due elettrodi, con la corrente che entra da un'estremità ed esce
dall'altra. I conduttori di alimentazione e il resto del circuito non fanno parte
del modello, e nemmeno il loro campo. Ciò che il solutore calcola è quindi il
campo del conduttore modellato come se la corrente nascesse a un'estremità e
sparisse all'altra:

- **vicino al conduttore** questa è di solito un'ottima approssimazione, poiché
  lì il campo è dominato dalla corrente vicina;
- **lontano da esso**, a distanze paragonabili al conduttore stesso, la parte
  mancante dell'anello conta quanto quella modellata, e il risultato non è il
  campo di alcun circuito reale.

Quando il campo lontano è importante, includi il percorso di ritorno nel modello
elettrostatico, in modo che la corrente formi un anello. Un conduttore di ritorno
sottile può essere rappresentato da un'entità linea con un'area della sezione.

### 1.5 Condizioni al contorno, iniziali e ambientali

Non ce ne sono **nessuna**, e non ne servono.

| Tipo di condizione | Offerte per la magnetostatica |
|---|---|
| Condizioni al contorno | nessuna |
| Condizioni iniziali | nessuna |
| Condizioni ambientali | nessuna |
| Proprietà del materiale | nessuna |

Il campo è interamente determinato dalla corrente e dal requisito che si annulli
a grande distanza, che la legge di Biot-Savart porta con sé. Non c'è alcun
contorno esterno da chiudere né nulla da prescrivervi.

Quando un task di elettrostatica e uno di magnetostatica condividono un flusso
di task - cosa che avviene sempre, vedi 1.6 - le schede delle condizioni offrono
comunque le condizioni **elettrostatiche**, perché le schede vengono popolate dai
tipi di problema dell'intero flusso. *Electric potential* e *Charge density*
appartengono al task di elettrostatica e raggiungono il campo magnetico tramite
la densità di corrente che producono.

### 1.6 Accoppiamento con altri solutori

| Direzione | Variabile | Effetto |
|---|---|---|
| Elettrostatica -> magnetostatica | Current density | l'unica sorgente del campo magnetico |

**La magnetostatica richiede l'elettrostatica.** La dipendenza è dichiarata nel
tipo di problema e l'interfaccia la impone: attivando *Magneto-statics* nel
selettore dei problemi si attiva automaticamente *Electro-statics*, e
disattivando *Electro-statics* si disattiva con esso *Magneto-statics*. Il flusso
dei task predefinito mette per primo il task di elettrostatica, che è l'ordine
richiesto dall'accoppiamento.

Nulla utilizza il campo magnetico. È la fine della sua catena - nessun solutore
lo legge, e non c'è forza di Lorentz, né corrente indotta, né contributo
magnetico ad alcun'altra fisica.

Poiché il solutore si dichiara sempre convergente, un gruppo di task che contiene
un task di magnetostatica si ferma sempre dopo un'iterazione.

### 1.7 Valutazione del campo

Ogni elemento percorso da corrente contribuisce a ogni nodo, quindi il lavoro
cresce come il **numero di nodi per il numero di elementi percorsi da corrente**.
La valutazione è eseguita in parallelo sui nodi, che vengono elaborati a blocchi
in modo che ogni elemento venga letto dalla memoria una volta per blocco anziché
una volta per nodo.

A titolo indicativo, una barra di `120 000` tetraedri valutata nei suoi
`24 000` nodi richiede circa cinque secondi su 14 thread. Raddoppiare la mesh in
ogni direzione moltiplica per otto sia il numero di nodi sia quello di elementi,
e il tempo per circa sessantaquattro; un modello di qualche centinaio di migliaia
di elementi richiede minuti anziché secondi.

Contano solo gli elementi percorsi da corrente. Una mesh grande attorno a un
piccolo conduttore - una regione d'aria, vedi 2.8 - aggiunge nodi ma non
sorgenti.

La configurazione del solutore matriciale non viene usata.

### 1.8 Risultati

| Risultato | Si applica a | Significato |
|---|---|---|
| Magnetic field `[T]` | nodo | il vettore del campo |

Questo è l'intero output. `process()` non fa nulla, quindi non ci sono grandezze
derivate - nessun modulo del campo memorizzato separatamente, nessun flusso,
nessuna densità di energia. La vista 3D può mostrare il modulo e le componenti
della variabile vettoriale nel modo consueto, e per la visualizzazione delle
componenti sono disponibili `Magnetic field in X direction` e gli equivalenti
`Y` e `Z`.

Il campo magnetico è memorizzato come variabile **nodale**, a differenza dei
risultati del campo elettrostatico, che sono variabili di elemento. **Ogni nodo**
del modello ha un valore, compresi i nodi di entità non percorse da corrente,
quindi una mesh attorno al conduttore mostra il campo nello spazio circostante.

Dopo la scrittura del record il log del solutore stampa le statistiche del campo
magnetico e i valori in tutti i punti di monitoraggio.

### 1.9 Cosa è il solutore e cosa non è

**È un integrale, non un'equazione di campo.** La valutazione di Biot-Savart non
richiede né un contorno né una mesh fuori dal conduttore, ed è esatta per la
corrente fornita. Il prezzo è che funziona solo in un mezzo **uniforme**. Con i
materiali magnetici il campo di una corrente dipende da dove si trova il ferro, e
nessun integrale della sola corrente può esprimerlo; quel problema si risolve con
una formulazione a potenziale - di solito un potenziale vettore magnetico su
elementi di spigolo - che è un solutore diverso, non un'estensione di questo.

**È generato da una corrente di conduzione calcolata.** Non c'è modo di
specificare direttamente una corrente, nessun magnete permanente e nessun campo
imposto; la corrente deve essere prodotta da una soluzione elettrostatica su un
corpo conduttore, e la sua accuratezza determina quella del risultato magnetico.
Il manuale di elettrostatica descrive cosa è e cosa non è quella densità di
corrente. Una densità di corrente costante per elemento è un'approssimazione del
primo ordine di quella reale, e converge al raffinarsi della mesh.

**Calcola il campo della corrente modellata.** Vedi 1.4 - la corrente dovrebbe
chiudersi all'interno del modello quando il campo lontano dal conduttore è
importante.

### 1.10 Analisi statica e solutore temporale

La magnetostatica è dichiarata come tipo di problema **senza** solutore
temporale, esattamente come l'elettrostatica. Il riquadro *Solutore temporale*
non compare nella scheda `Problema`, l'esecuzione produce un unico record e non
c'è nulla da configurare.

Poiché un flusso di magnetostatica contiene sempre anche un task di
elettrostatica, e nessuno dei due supporta un solutore temporale, aggiungere uno
dei due a un flusso rimuove il solutore temporale dal flusso intero. Un task
termico concatenato dopo di essi può essere solo stazionario. Il manuale di
elettrostatica tratta questo aspetto nella propria sezione 1.10.

---

## 2. Interfaccia grafica

Tutto ciò che serve si trova nel pannello **Solutore** (schede `Problema`,
`Materiale`, `Risultati`), nel menu **Problema** e nel pannello **Modello**. Le
schede delle condizioni non contengono nulla per questo tipo di problema.

### 2.1 Selezione del problema

`Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`) apre la finestra del
flusso dei task del problema. Aggiungi un task e scegli **Magneto-statics** -
"Calculate magnetic field" (calcola il campo magnetico).

**Electro-statics viene attivato per te.** Il selettore impone la dipendenza di
1.6 in entrambe le direzioni: attivando *Magneto-statics* si attiva
*Electro-statics*, e disattivando *Electro-statics* si disattiva
*Magneto-statics*. Il flusso dei task predefinito mette per primo il task di
elettrostatica.

La magnetostatica si combina con ogni altra fisica tranne *Modal analysis*. In
pratica l'unica catena significativa è quella su cui si basa:

- *Electro-statics* e poi *Magneto-statics* - il campo magnetico di una corrente
  di conduzione calcolata.

### 2.2 Scheda Problema

Questa scheda è **vuota** per un flusso di elettrostatica più magnetostatica.
Nessuno dei due tipi di problema ha un solutore temporale e nessuno ha un
riquadro di impostazioni proprio.

### 2.3 Scheda Materiale

La magnetostatica non richiede **nulla** dal materiale. Il task di elettrostatica
da cui dipende richiede tre proprietà - permittività relativa, conducibilità
elettrica e densità - e un'entità il cui materiale ne manca una viene saltata da
*quel* task, il che la lascia senza densità di corrente e quindi senza
contributo al campo magnetico.

Il materiale conta quindi comunque, interamente attraverso la metà
elettrostatica della catena. Assegna un materiale con tutte e tre le proprietà,
come il *Copper* fornito, e consulta il manuale di elettrostatica per sapere a
cosa serve ciascuna.

Non c'è alcuna proprietà permeabilità relativa da assegnare. Il solutore lavora
nello spazio libero.

### 2.4 Scheda Condizioni al contorno

Nulla in questa scheda appartiene al task di magnetostatica. Le condizioni
offerte - *Electric potential* e *Charge density* - sono quelle
elettrostatiche, e generano la densità di corrente che a sua volta produce il
campo magnetico. Assegnale come descritto nel manuale di elettrostatica.

Non esiste alcuna condizione al contorno magnetostatica, né serve - vedi 1.5.

### 2.5 Condizioni iniziali e ambientali

Nessuna delle due schede offre qualcosa per la magnetostatica. La condizione
iniziale *Electric potential* e la condizione ambientale *Temperature* mostrate
appartengono al task di elettrostatica.

### 2.6 Configurazione del solutore matriciale

Il solutore di magnetostatica non risolve alcun sistema di equazioni e non usa la
configurazione del solutore matriciale. La voce **CG** configura solo il
solutore di elettrostatica.

### 2.7 Punti di monitoraggio

`Problema` -> `Definisci punti di monitoraggio` colloca sonde in coordinate date
e seleziona la variabile da registrare. *Magnetic field* è disponibile
nell'elenco insieme alle variabili elettrostatiche e legge il campo nodale nella
sonda.

### 2.8 Risultati e record

La scheda `Risultati` elenca **Magnetic field** come vettore nodale, accanto ai
risultati elettrostatici prodotti dal primo task. La scheda `Record` del pannello
`Modello` contiene esattamente un record: entrambi i tipi di problema sono
statici.

`Report` -> `File di log del solutore` mostra l'output completo: prima le
statistiche dell'elettrostatica, poi il numero di elementi percorsi da corrente
e di nodi per cui è stato valutato l'integrale di Biot-Savart, infine le
statistiche del campo magnetico.

**Vedere il campo attorno a un conduttore.** Il campo viene calcolato in ogni
nodo, quindi tutto ciò che è meshato attorno al conduttore lo mostra. Mesha lo
spazio circostante come entità volume separata - una regione d'aria - e
assegnale un materiale senza conducibilità elettrica: il task di elettrostatica
la salta, quindi non è percorsa da corrente, ma i suoi nodi ricevono il campo
magnetico. La regione costa nodi ma non sorgenti (1.7) e non richiede né una
dimensione particolare né un contorno, poiché su di essa non si risolve nulla.

Il materiale *Air* fornito ha una conducibilità elettrica, seppur minima, quindi
il task di elettrostatica includerebbe la regione nella propria soluzione. Usane
una copia da cui sia stata rimossa la proprietà conducibilità elettrica. Il
controllo della configurazione avvisa allora che al materiale dell'entità
mancano proprietà richieste; per la regione d'aria è previsto, e l'esecuzione
prosegue.

---

## 3. Tutorial - il campo di un conduttore percorso da corrente

**Obiettivo.** Eseguire dall'inizio alla fine la catena elettrostatica e
magnetostatica su una barra di rame percorsa da corrente, verificare il campo
magnetico con un calcolo manuale e vedere dove si manifestano le approssimazioni
di 1.4.

Questo tutorial presuppone la barra meshata del manuale di elettrostatica - un
parallelepipedo lungo `0.1 m` con sezione quadrata di `0.01 m`, tetraedrizzato,
con le due facce terminali marcate come entità superficie separate. Se quel
modello è ancora configurato, cambia solo il flusso dei task.

### Passo 1 - selezionare i problemi

1. `Problema` -> `Flusso dei task dei problemi` (`Ctrl+P`).
2. Attiva **Magneto-statics**. Nota che **Electro-statics** si attiva da solo.
3. Conferma con `OK`.

Il flusso dei task contiene ora due task, prima l'elettrostatica. La scheda
`Problema` resta vuota.

### Passo 2 - assegnare il materiale

Seleziona l'entità volume e assegna **Copper** nella scheda `Materiale`, oppure
un materiale con le tre proprietà richieste dal task di elettrostatica:

- **Relative permittivity** = `2.8`
- **Electrical conductivity** = `5.882e7` `S/m`
- **Density** = `8960` `kg/m^3`

Nulla di tutto ciò viene letto dal task di magnetostatica. È il task
elettrostatico a richiederle, e senza densità di corrente non c'è campo
magnetico.

### Passo 3 - far scorrere una corrente

1. Seleziona l'entità superficie a un'estremità, apri la scheda `Condizioni al
   contorno` e attiva **Electric potential**. Impostalo a `0` V.
2. Seleziona l'entità superficie all'altra estremità, attiva **Electric
   potential** e impostalo a `0.01` V.

La corrente nella barra è

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Una barra senza differenza di potenziale non è percorsa da corrente e dà un campo
magnetico esattamente nullo - è la prima cosa da controllare se il risultato
risulta vuoto.

### Passo 4 - risolvere

1. `Soluzione` -> `Avvia solutore` (`Ctrl+R`).
2. Segui l'avanzamento in `Report` -> `File di log del solutore`.

Il log mostra il task di elettrostatica con le sue variabili, poi il task di
magnetostatica: il numero di elementi percorsi da corrente, il numero di nodi e
le statistiche del campo magnetico. Entrambi vengono eseguiti una volta.

### Passo 5 - esaminare il risultato

- Scheda `Risultati`: visualizza **Magnetic field**. È un vettore nodale, quindi
  la vista 3D ne offre il modulo e le tre componenti.
- Il campo **circola attorno alla barra**, perpendicolare alla corrente, nel
  verso della regola della mano destra.
- È **nullo sull'asse** della barra e cresce verso la superficie, dove è massimo -
  al centro di ogni faccia lunga anziché negli spigoli.
- È **più debole vicino alle estremità** della barra che al centro. È il
  percorso di corrente aperto di 1.4: vicino a un'estremità manca metà di quello
  che sarebbe un conduttore più lungo.

### Passo 6 - verificare il modulo

Per un conduttore circolare di raggio `R` il campo superficiale è
`u0*I/(2*pi*R)`. Una barra quadrata di lato `a` non è circolare, ma il centro di
una faccia a distanza `a/2` dall'asse vi si avvicina: con `a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

Il campo calcolato al centro di una faccia lunga, a metà della barra, è circa
`0.020 T`, circa il 15 % in meno - la sezione quadrata mette gran parte della
corrente più lontano dal punto di quanto farebbe una sezione circolare. In uno
spigolo, a `a/sqrt(2)` dall'asse, è ancora più basso, circa `0.019 T`.

Una verifica più precisa richiede un punto di campo lontano dalla barra, dove la
sezione non conta più. Aggiungi una regione d'aria come descritto in 2.8 e leggi
il campo a `r = 0.02 m` dall'asse, a metà lunghezza. Per un conduttore
rettilineo finito di lunghezza `L`,

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

e il valore calcolato dovrebbe coincidere entro circa lo `0.1 %`, con la
differenza dovuta alla sezione quadrata della barra. Questo valore è il campo
della sola barra, come spiega 1.4 - un circuito reale aggiungerebbe il campo dei
suoi conduttori di alimentazione.

### Risoluzione dei problemi

| Sintomo | Causa |
|---|---|
| Il campo magnetico è nullo ovunque e il log avvisa che nessun elemento è percorso da corrente | nessuna densità di corrente - verifica che il task di elettrostatica sia stato eseguito per primo, che i potenziali siano diversi e che il materiale abbia una conducibilità elettrica |
| **Magneto-statics** non può essere disattivato da solo | previsto - richiede *Electro-statics*, e disattivare quest'ultimo li disattiva entrambi |
| Il riquadro *Solutore temporale* manca | previsto - nessuno dei due tipi di problema ha un solutore temporale, vedi 1.10 |
| Il campo è più debole alle estremità di un conduttore | previsto - il percorso della corrente è aperto, vedi 1.4 |
| Il campo lontano dal conduttore non corrisponde al circuito | i conduttori di alimentazione e il resto del circuito non sono nel modello, vedi 1.4 |
| Il campo su un conduttore lineare o su una superficie sottile è irregolare | previsto - il campo di un conduttore di sezione nulla è lì singolare, vedi 1.3 |
| L'esecuzione richiede molto tempo | il costo è nodi per elementi percorsi da corrente, vedi 1.7 |

---

## 4. Verifica di un modello

**Conduttore rettilineo.** Per un conduttore lungo di raggio `R` che porta
uniformemente una corrente totale `I`, il campo è circonferenziale e

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(`outside` = all'esterno, `inside` = all'interno) con `u0 = 1.2566e-6`. Il campo
è massimo sulla superficie del conduttore, `B = u0*I/(2*pi*R)`, e oltre decresce
come `1/r`. Per una corrente di `1 A` in un conduttore di `5 mm` di raggio il
picco è `4e-5 T`, all'incirca l'intensità del campo terrestre.

**Conduttore finito.** Per un conduttore rettilineo di lunghezza `L`, a distanza
`r` dal suo asse e a metà lunghezza,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

che tende alla formula del conduttore lungo al crescere di `L`. Lontano dal
conduttore la forma della sezione non conta più, quindi qualsiasi sezione può
essere verificata con questa formula.

**Filo.** Un'entità linea con area della sezione porta la sua corrente lungo i
segmenti, e il solutore usa per ciascuno la forma chiusa di un filo rettilineo.
Un'entità linea rettilinea riproduce quindi esattamente la formula del
conduttore finito, a meno degli arrotondamenti, a qualsiasi distanza.

**Solenoide.** Per un solenoide lungo con `n` spire per metro percorso da `I`, il
campo interno è uniforme e diretto lungo l'asse,

```
B = u0 * n * I
```

e quasi nullo all'esterno. Un avvolgimento può essere modellato come entità
linea che segue le spire, con l'area della sezione del filo, e il task di
elettrostatica vi fa scorrere la corrente tramite potenziali alle sue due
estremità.

**Divergenza.** Per qualsiasi campo magnetico vale ovunque `div(B) = 0`. La legge
di Biot-Savart la soddisfa esattamente, quindi non mette alla prova questo
solutore; metterebbe alla prova solo l'interpolazione tra i nodi.

**Convergenza con la mesh.** L'integrazione in sé è esatta (1.3); ciò che
dipende dalla mesh è la densità di corrente, che il task di elettrostatica
calcola costante per elemento. Raffinare la mesh nel conduttore e confrontare il
campo nello stesso punto è la verifica di questa parte.

---

## 5. Limitazioni

### Limitazioni di modellazione

- **Solo spazio libero.** Nella formulazione non entra alcuna permeabilità. Non
  esiste una proprietà del materiale permeabilità relativa, né distinzione tra
  `B` e `H`, né magnetizzazione né saturazione, quindi ferro, ferriti e
  schermature non possono essere rappresentati affatto. Vedi 1.9 per sapere
  perché ciò richiede un solutore diverso.
- **Solo il campo della corrente modellata.** Il campo dei conduttori di
  alimentazione e del resto del circuito non è incluso. Quando nel modello il
  percorso della corrente è aperto, il campo lontano dal conduttore non è quello
  di un circuito reale - vedi 1.4.
- **Nessun magnete permanente e nessun campo imposto.** L'unica sorgente è una
  corrente di conduzione calcolata da un task di elettrostatica. Un magnete, un
  campo di fondo e un'eccitazione esterna non hanno rappresentazione.
- **Una corrente non può essere specificata direttamente.** Una corrente viene
  prodotta da una soluzione elettrostatica su un corpo conduttore. Una bobina può
  essere modellata come entità linea che segue l'avvolgimento, ma la sua
  corrente si imposta comunque tramite potenziali.
- **I conduttori sottili sono singolari.** Il campo in un nodo su un'entità linea
  o sul bordo di un'entità superficie è un valore regolarizzato, non fisico -
  vedi 1.3.
- **Tipi di elemento.** Tetraedri, triangoli, quadrilateri e segmenti a due nodi
  portano corrente; gli altri tipi di elemento non contribuiscono.
- **Costo.** Il lavoro cresce come il numero di nodi per il numero di elementi
  percorsi da corrente - vedi 1.7.
- **Nessun risultato derivato.** `process()` non calcola nulla, quindi il vettore
  del campo magnetico è l'intero output. Non ci sono flusso, energia, forza né un
  modulo del campo memorizzato come variabile a sé.
- **Nulla utilizza il risultato.** Nessun altro solutore legge il campo
  magnetico. Non c'è forza di Lorentz sul conduttore, né corrente indotta, né
  accoppiamento di ritorno con la fisica elettrostatica, termica o strutturale.
- **Solo statica.** Non c'è un solutore temporale, e aggiungere un task di
  magnetostatica - che porta con sé un task di elettrostatica - rimuove il
  solutore temporale dall'intero flusso dei task, quindi anche qualsiasi altra
  fisica concatenata dopo di esso è stazionaria.
