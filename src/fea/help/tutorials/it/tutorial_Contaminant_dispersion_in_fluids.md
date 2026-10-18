# Dispersione di contaminanti nei fluidi

Questo tutorial mostra come impostare e risolvere una simulazione multifisica che combina un problema non lineare e iterativo – **CFD (fluidodinamica computazionale)** – con il trasporto di un contaminante trascinato dal flusso.

Sono necessari due tipi di problema:

1. **Incompressible viscous flow** (flusso viscoso incomprimibile) – calcola la velocità e la pressione del fluido (flusso stazionario o transitorio di fluidi newtoniani).
2. **Contaminant dispersion** (dispersione di contaminanti) – calcola come un contaminante si diffonde in quel campo di flusso.

Poiché il problema del flusso è non lineare, viene risolto in modo iterativo. La simulazione si svolge quindi in due fasi:

1. **Flusso stazionario** – prima si calcolano, senza contaminante, un campo di flusso «iniziale» convergente e la distribuzione della pressione.
2. **Simulazione transitoria** – poi, partendo da quel campo di flusso, si usa l'**avanzamento nel tempo** per seguire come un impulso di contaminante che entra nel canale viene trasportato al suo interno.

## 1. Caricare il modello

**Menu:** _File -> Apri modello_

Nella finestra **Apri modello** seleziona **Channel.tmsh** e fai clic su **Apri**.

Il modello è composto da tre entità superficie – **Walls**, **Inflow** e **Outflow**. Poiché al modello non è ancora assegnato alcun tipo di problema, al termine del caricamento si apre automaticamente la finestra **Flusso dei task del problema**.

## 2. Flusso dei task del problema (fase 1 – flusso stazionario)

Nella prima fase si risolve solo il flusso, con molte iterazioni non lineari in modo che possa convergere.

1. Nella finestra **Flusso dei task del problema** fai clic su **Aggiungi tipo di problema**.
2. Nel **Selettore del tipo di problema** attiva **Incompressible viscous flow** e fai clic su **OK**.
3. Fai doppio clic sul valore **1** accanto a **N. di iterazioni:**, digita **2000** e premi _Invio_.
4. Fai clic su **OK** per chiudere la finestra.

La finestra può essere riaperta in qualsiasi momento con il pulsante **Flusso dei task dei problemi** della barra degli strumenti. Ora dovrebbe apparire così:

![CFD – Flusso dei task del problema, stazionario](image-CFD-Problem_task_flow_steady.png)

## 3. Generare la mesh di volume

Il modello è definito solo dalle sue superfici. Per risolvere un flusso al suo interno occorre prima generare una mesh di volume (tetraedrica).

**Menu:** _Geometria -> Volume -> Genera mesh tetraedrica_

I valori precompilati nella finestra **Genera mesh 3D** sono sufficienti. Fai clic su **OK**. Al termine della generazione della mesh, nell'albero **Modello** sotto **Volumi** compare una nuova entità **Volume 1**.

## 4. Assegnare il materiale

A tutte le entità del modello deve essere assegnato un materiale.

1. Nell'albero **Modello** fai clic su **Walls** e poi, tenendo premuto _Maiusc_, fai clic su **Volume 1** per selezionare tutte le entità.
2. Apri la scheda **Materiale** sul lato destro della finestra principale.
3. Attiva **Water** (acqua).

## 5. Condizioni al contorno (fase 1)

Le condizioni al contorno si assegnano nella scheda **Condizioni al contorno** all'entità selezionata nell'albero **Modello**. Assegna alle entità **superficie** le condizioni seguenti:

1. **Walls**
    - _Wall_ (parete; nessun valore richiesto)
2. **Inflow**
    - _Volumetric flow rate (inflow)_ (portata volumetrica, ingresso)
        - Volumetric flow rate = 50 `[m^3/s]`
3. **Outflow**
    - _Pressure (implicit)_ (pressione, implicita)
        - Pressure = 0 `[Pa]` (il valore predefinito è 1000 `[Pa]`)

Per ogni entità: selezionala nell'albero **Modello**, attiva la condizione nella scheda **Condizioni al contorno**, seleziona la condizione per mostrarne le proprietà sotto l'elenco, poi fai doppio clic sul valore, digita quello nuovo e premi _Invio_.

## 6. Risolvere il flusso stazionario

**Menu:** _Soluzione -> Avvia solutore_

Se il modello non è ancora stato salvato, viene mostrata prima la finestra **Salva modello**. Fai clic su **Salva** per accettare il nome file proposto (**Channel.rbm**), poi fai clic su **OK** nella finestra **Avvia solutore per il modello**.

Il solutore viene eseguito in background e il suo output è mostrato nella scheda **Output del processo**. Il calcolo di tutte le 2000 iterazioni richiede alcuni minuti; l'esecuzione è terminata quando l'output si conclude con **SOLVER - END**. La convergenza della soluzione può essere controllata in qualsiasi momento con:

**Menu:** _Report -> Convergenza del solutore_

## 7. Creare una sezione

Un buon modo per esaminare un flusso è visualizzare la velocità come **campo vettoriale**, in cui ogni freccia mostra direzione e intensità del flusso in un punto. Un campo vettoriale disegnato in tutto il volume, però, diventa presto una nuvola fitta di frecce sovrapposte, difficile da leggere. Di solito è più chiaro tagliare prima il modello con una **sezione** – un piano attraverso il volume – e poi visualizzare il campo vettoriale solo su quel piano.

_Suggerimento: prima di iniziare attiva **Volumi** nell'albero **Modello**, così il volume è visibile mentre posizioni la sezione._

**Menu:** _Geometria -> Sezione -> Crea sezione_

Nella finestra **Editor della sezione**:

1. Seleziona **Volume 1** nell'elenco **Modello**. La sezione verrà creata attraverso questo volume.
2. **Posizione del piano** imposta un punto per cui passa il piano di sezione. I valori predefiniti lo collocano al centro del modello (**X** = 7.5, **Y** = 2.5, **Z** = 1.5 `[m]`); mantienili.
3. **Normale del piano** imposta l'orientamento del piano. La normale predefinita (**X** = 1) dà un piano verticale; cambiala in **X** = 0, **Y** = 0, **Z** = 1 per ottenere un piano orizzontale che taglia il canale a metà altezza.

Mentre modifichi i valori, il piano di sezione viene mostrato in anteprima nell'**area 3D del modello**. Fai clic su **OK** per creare la sezione. Nell'albero **Modello** sotto **Sezioni** compare una nuova entità **Cut 1**.

![CFD – Crea entità sezione](image-CFD-Create_cut.png)

## 8. Creare un campo vettoriale

Ora visualizza la velocità calcolata come frecce sul piano di sezione.

**Menu:** _Geometria -> Campo vettoriale -> Crea campo vettoriale_

Nella finestra **Editor del campo vettoriale**:

1. Seleziona **Cut 1** nell'elenco **Modello**. Le frecce verranno disegnate sul piano di sezione creato nel passo precedente.
2. Seleziona **Velocity** (velocità) nell'elenco **Variabili**.
3. Lascia attivo **Campo vettoriale 3D**. Le frecce mostrano così il vettore velocità completo. Se è disattivato, ogni freccia viene proiettata sul piano di sezione e viene mostrata solo la parte del flusso che giace nel piano.

Fai clic su **OK** per creare il campo vettoriale. Compare nell'albero **Modello** sotto **Campi vettoriali**. Disattiva **Volumi**, in modo che il volume pieno non nasconda le frecce.

![CFD – Crea entità campo vettoriale](image-CFD-Create_vector.png)

Le frecce potrebbero essere troppo corte per essere ben visibili, perché la loro lunghezza è proporzionale alla velocità del flusso. Per allungarle, seleziona il campo vettoriale nell'albero **Modello**, apri la scheda **Risultati** e aumenta il valore nella sezione **Scala**. La scala è il prodotto del valore **Scala** per la potenza di dieci impostata in **Fattore**; nell'esempio seguente **Scala** 1 e **Fattore** 2 danno una scala di `1 × 10^2 = 100`.

![CFD – Scala dell'entità campo vettoriale](image-CFD-Scale_vector.png)

## 9. Flusso dei task del problema (fase 2 – transitoria)

Con un campo di flusso convergente disponibile, si può configurare la simulazione **transitoria** che include **Contaminant dispersion**.

In una simulazione transitoria i problemi vengono risolti una volta per ogni passo temporale. **Incompressible viscous flow** è ancora non lineare, quindi richiede proprie iterazioni non lineari all'interno di ogni passo temporale, mentre **Contaminant dispersion** viene risolto una volta per passo temporale dopo il flusso. Riapri la finestra **Flusso dei task del problema** (pulsante **Flusso dei task dei problemi** della barra degli strumenti) e configurala come segue:

1. Seleziona **Incompressible viscous flow** e fai clic sul pulsante **>**. Questo lo sposta in un nuovo gruppo di iterazioni annidato.
2. Fai doppio clic sul valore del **N. di iterazioni:** esterno e cambialo da **2000** a **1**. Poi cambia il **N. di iterazioni:** interno (annidato) da **1** a **10**.
3. Fai clic su **Aggiungi tipo di problema**, attiva **Contaminant dispersion** e fai clic su **OK**. Anche **Incompressible viscous flow** viene attivato automaticamente, perché la dispersione di contaminanti richiede un campo di flusso. Se al livello esterno viene aggiunta una seconda voce **Incompressible viscous flow**, selezionala e fai clic su **Rimuovi**.

Il flusso dei task dovrebbe apparire come nello screenshot seguente. Conferma con **OK**.

![CFD – Flusso dei task del problema](image-CFD-Problem_task_flow.png)

## 10. Impostazioni del solutore temporale

Apri la scheda **Problema** sul lato destro della finestra principale e imposta:

1. Attiva **Solutore temporale**.
2. **Approssimazione**: **Backward difference (stable)** (differenze all'indietro – stabile).
3. **Dimensione del passo temporale**: **0.01** `[s]`.
4. **Numero di passi temporali**: **2000**.
5. **Frequenza di output**: **10** – un record di risultati viene scritto ogni 10 passi temporali.

Il **Tempo finale** viene calcolato automaticamente (2000 × 0.01 s = 20 s).

Nella sezione **Impostazioni della dispersione di contaminanti** imposta la **Saturazione massima** a **2.4** `[kg/m^3]` e il **Coefficiente di diffusione** a **0.01** `[m^2/s]`. Per ulteriori informazioni consulta il manuale di teoria.

![CFD – Solutore temporale del problema](image-CFD-Problem_time_solver.png)

## 11. Condizioni al contorno (fase 2)

Il contaminante entra nel canale attraverso l'ingresso. Seleziona **Inflow** nell'albero **Modello** e nella scheda **Condizioni al contorno** attiva **Particle concentration** (concentrazione di particelle); lascia attiva **Volumetric flow rate (inflow)**.

Invece di un valore costante si userà un valore dipendente dal tempo. Espandi la proprietà **Particle concentration** (freccia davanti al suo nome) e fai clic su **Modifica valori dipendenti dal tempo**.

![CFD – Condizioni al contorno](image-CFD-Boundary_conditions.png)

Nella finestra **Editor dei componenti** imposta **Numero di valori** a **3** e inserisci i valori seguenti (doppio clic su una cella per modificarla), poi fai clic su **Ok**.

| Tempo `[s]` | Particle concentration `[kg/m^3]` |
| ----------- | --------------------------------- |
| 0           | 0                                 |
| 5           | 1                                 |
| 10          | 0                                 |

Tra due istanti specificati il valore viene interpolato linearmente, e dopo l'ultimo istante specificato si mantiene l'ultimo valore. Con questi valori la concentrazione all'ingresso sale da 0 a 1 `[kg/m^3]` nei primi 5 secondi e torna a 0 tra 5 e 10 secondi.

![CFD – Condizione Particle concentration](image-CFD-Particle_concentration_condition.png)

## 12. Risolvere il problema transitorio (riavvio)

**Menu:** _Soluzione -> Avvia solutore_

Nella finestra **Avvia solutore per il modello** attiva **Riavvia solutore / continua** e fai clic su **OK**. Il solutore usa quindi il flusso stazionario già calcolato come punto di partenza e prosegue con la simulazione ad avanzamento nel tempo.

L'avviso **Contaminant dispersion: No implicit boundary condition assigned.** è previsto e può essere ignorato.

![CFD – Riavvio del solutore](image-CFD-Solver_restart.png)

## 13. Visualizzare i risultati durante l'esecuzione del solutore

Il solutore transitorio viene eseguito in background, quindi i risultati possono essere esaminati mentre il calcolo è ancora in corso. Altre entità di visualizzazione come sezioni, ISO, linee di flusso, campi vettoriali o campi scalari possono essere create in qualsiasi momento e su di esse si possono visualizzare i risultati calcolati. Qui si riutilizzano la sezione e il campo vettoriale dei passi 7 e 8.

### 13.1 Concentrazione del contaminante sulla sezione

1. Nell'albero **Modello** espandi **Sezioni**, attiva **Cut 1** in modo che sia visibile e selezionala.
2. Apri la scheda **Risultati** e scegli **Particle concentration - Scalar** nell'elenco a discesa in alto.
3. In **Applica come ...** attiva **Scalar**. La sezione viene ora colorata in base alla concentrazione del contaminante.
4. In **Intervallo di valori e scala dei colori** imposta **Massimo** a **1** e **Minimo** a **0** (digita il valore e premi _Invio_). Corrisponde all'intervallo della concentrazione in ingresso, così gli stessi colori indicano la stessa concentrazione in ogni passo temporale.
5. Attiva **Mostra intervallo di valori** per visualizzare la scala dei colori nell'**area 3D del modello**.

![Risultati della concentrazione di particelle](image-CFD-Particle_concentration_results.png)

### 13.2 Frecce del campo vettoriale

Per rendere più leggibile la direzione del flusso sopra la sezione colorata, disegna le frecce con le punte e in bianco:

1. Seleziona il campo vettoriale (**Velocity** sotto **Campi vettoriali**) nell'albero **Modello**.
2. In **Proprietà di visualizzazione** sotto l'albero attiva **Disegna punte delle frecce**.
3. Fai clic sul selettore di colore sotto le caselle di controllo e scegli il bianco (**#ffffffff**).

Le altre proprietà di visualizzazione stabiliscono se tutte le frecce hanno la stessa lunghezza (**Disegna frecce di uguale lunghezza**) e se le frecce partono dal punto in cui viene valutata la velocità (**Disegna frecce uscenti dal punto**).

![CFD – Disegna punte delle frecce](image-CFD-Draw_arrow_heads.png)

## 14. Record del modello (risultati nel tempo)

Ogni 10 passi temporali (la **Frequenza di output**) il solutore scrive un record del modello contenente la soluzione per quell'istante. I record sono elencati nella scheda **Record** sul lato sinistro della finestra principale; la colonna **Record** mostra il numero del passo temporale (il record **500** contiene la soluzione a 500 × 0.01 s = 5 s). Fai doppio clic su un record per caricare i risultati del suo passo temporale; i risultati applicati nella scheda **Risultati** vengono mantenuti e ridisegnati per il record caricato.

Il calcolo di tutti i 2000 passi temporali richiede un po' di tempo (circa 20 minuti su un computer attuale). Il solutore può essere arrestato in qualsiasi momento e i record scritti fino a quel momento vengono conservati:

**Menu:** _Soluzione -> Arresta solutore_ (conferma con **Yes**)

## 15. Registrare un video

Un video viene creato da ciò che è attualmente mostrato nell'**area 3D del modello**, quindi imposta prima la vista e i risultati visualizzati (vedi il passo 13). Le impostazioni di visualizzazione non vengono salvate con il modello e dopo averlo riaperto devono essere impostate di nuovo. Poi apri la scheda **Record**, usa i campi **Da** e **A** in basso per scegliere l'intervallo di record e fai clic sul pulsante rosso di registrazione.

![CFD – Record del modello](image-CFD-Model_records.png)

Compare la finestra **Impostazioni video**, in cui si possono impostare il codec video, il formato del file, i fotogrammi al secondo e i fotogrammi per record; sotto viene mostrata la durata stimata del video. Con 202 record, 24 fotogrammi al secondo e 1 fotogramma per record il video dura circa 8.4 secondi. Fai clic su **OK** per avviare la registrazione.

![CFD – Impostazioni video](image-CFD-Video_settings.png)

La registrazione può richiedere del tempo, perché ogni record del modello viene caricato a turno e uno screenshot dell'**area 3D del modello** viene acquisito come fotogramma del video.

Il video e tutti i fotogrammi (un'immagine PNG per record) si trovano nella scheda **Documenti** sotto **Channel**; il video **Channel.mp4** è alla fine dell'elenco. Fai doppio clic su di esso per riprodurlo.

![CFD – Documenti](image-CFD-Documents.png)
