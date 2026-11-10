# Multifisica

Questo tutorial mostra come risolvere più problemi accoppiati su un unico modello. Viene riutilizzato il modello **Beam** (trave) del tutorial **Analisi delle tensioni e delle deformazioni**. Questa volta una corrente elettrica attraversa la trave, la corrente riscalda la trave (effetto Joule) e il calore fa dilatare e deformare la trave.

Sono necessari tre tipi di problema, risolti in questo ordine:

1. **Electro-statics** (elettrostatica) – calcola il potenziale elettrico, il campo elettrico, la densità di corrente e il calore di Joule prodotto dalla corrente.
2. **Heat transfer** (trasmissione del calore) – calcola la distribuzione di temperatura causata dal calore di Joule.
3. **Stress analysis** (analisi delle tensioni) – calcola la dilatazione termica (spostamenti) e la tensione risultante.

Ogni problema usa i risultati del precedente, quindi l'ordine dei problemi è importante.

## 1. Caricare il modello

**Menu:** _File -> Apri modello_

Nella finestra di dialogo **Apri modello** selezionare **Beam.rbm** e fare clic su **Open** (Apri).

Poiché al modello non è ancora assegnato alcun tipo di problema, la finestra di dialogo **Seleziona problema** si apre automaticamente al termine del caricamento.

## 2. Selezionare i problemi

Nella finestra di dialogo **Seleziona problema** fare clic sui riquadri **Electro-statics**, **Heat transfer** e **Stress analysis**. Sotto i riquadri la finestra mostra l'**Ordine di risoluzione**: Electro-statics → Heat transfer → Stress analysis. **Modal analysis** è contrassegnato come **Incompatibile**, perché non può essere combinato con altri tipi di problema. Confermare con **OK**.

![Seleziona problema – multifisica](image-Select_problem-Multi.png)

La finestra di dialogo può essere riaperta in qualsiasi momento con il pulsante **Seleziona problema** della barra degli strumenti.

## 3. Generare la mesh 3D

Tutti e tre i problemi vengono risolti su una mesh di volume.

**Menu:** _Geometria -> Volume -> Genera mesh tetraedrica_

I valori precompilati nella finestra di dialogo **Genera mesh 3D** sono sufficienti. Fare clic su **OK**. Al termine della generazione, nell'**albero del modello** appare una nuova entità **Volume 1** sotto **Volumi**.

## 4. Assegnare il materiale

A tutte le entità del modello deve essere assegnato un materiale. Il materiale deve fornire le proprietà necessarie a tutti e tre i problemi; **Iron** (ferro) le fornisce.

1. Nell'**albero del modello** espandere **Volumi**, fare clic su **Surface** e poi fare clic su **Volume 1** tenendo premuto _Maiusc_ per selezionare tutte le entità.
2. Aprire la scheda **Materiale** sul lato destro della finestra principale.
3. Selezionare **Iron**.

## 5. Assegnare le condizioni al contorno

Poiché sono selezionati più tipi di problema, la scheda **Condizioni al contorno** offre ora le condizioni di tutti, e a un'entità possono essere assegnate più condizioni, una per ogni problema. Assegnare le seguenti condizioni alle entità **superficie**:

1. **Surface** (la superficie esterna della trave)
    - _Simple convection_ (convezione semplice)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293,15 `[K]`
2. **Load** (la faccia al centro della trave)
    - _Electric potential_ (potenziale elettrico)
        - Electric potential = 0,04 `[V]`
    - _Simple convection_ (valori come per **Surface**)
3. **Left** e **Right** (le facce alle due estremità della trave)
    - _Displacement_ (spostamento)
        - Displacement in X, Y and Z direction = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0,04 `[V]`
    - _Simple convection_ (valori come per **Surface**)

La corrente scorre quindi dal centro della trave verso le due estremità, e tutte le facce cedono calore all'aria circostante per convezione.

Per ogni entità: selezionarla nell'**albero del modello** (tenere premuto _Ctrl_ / _⌘ Cmd_ per selezionare **Left** e **Right**), spuntare la condizione nella scheda **Condizioni al contorno**, selezionare la condizione per mostrarne le proprietà sotto l'elenco, quindi fare doppio clic sul valore, digitare quello nuovo e premere _Invio_. I valori predefiniti di **Simple convection** (100 `[W/(m^2*K)]` e 293,15 `[K]`) e di **Displacement** (0 `[m]`) sono già corretti, quindi va modificato solo **Electric potential**.

## 6. Verificare le condizioni ambientali

Le condizioni ambientali si applicano a tutto il modello e non si spuntano come le condizioni al contorno. Aprire la scheda **Condizioni ambientali** e selezionare ogni condizione per verificarne i valori:

- _Gravitational acceleration_ (accelerazione di gravità)
    - 0, 0 e -9,80665 `[m/s^2]` nelle direzioni X, Y e Z
- _Temperature_ (temperatura)
    - Temperature = 293,15 `[K]`

## 7. Risolvere il problema

**Menu:** _Soluzione -> Avvia solutore_

Il modello deve essere salvato prima che il solutore possa essere avviato. Il modello **Beam** è stato aperto da un file, quindi viene salvato automaticamente in esso; un modello mai salvato mostra prima la finestra di dialogo **Salva modello**. Viene poi mostrata la finestra di dialogo **Avvia solutore**.

La finestra elenca tre avvisi: **Electro-statics: No implicit boundary condition assigned.**, **Heat transfer: No explicit boundary condition assigned.** e **Stress analysis: No implicit boundary condition assigned.** Sono previsti per questa impostazione e possono essere ignorati. Fare clic su **OK** per avviare il solutore.

Il solutore viene eseguito in background e il suo output è mostrato nella scheda **Output del processo**. I problemi vengono risolti uno dopo l'altro nell'ordine del flusso delle attività; il calcolo è terminato quando l'output finisce con **SOLVER - END**.

## 8. Creare un campo vettoriale

Un **campo vettoriale** è un'entità di visualizzazione che disegna una variabile vettoriale come frecce. Qui viene usato per mostrare il campo elettrico all'interno della trave.

**Menu:** _Geometria -> Campo vettoriale -> Crea campo vettoriale_

Nella finestra di dialogo **Editor del campo vettoriale** selezionare **Volume 1** nell'elenco **Modello** ed **Electric field** nell'elenco **Variabili**, e lasciare selezionato **Campo vettoriale 3D**. L'elenco **Variabili** mostra le variabili vettoriali calcolate per l'entità selezionata (**Electric field**, **Current density**, **Heat flux**, **Displacement** e **Force**).

![Crea campo vettoriale](image-Create_vector_field.png)

Fare clic su **OK** per creare il campo vettoriale. Viene aggiunto all'**albero del modello** sotto **Campi vettoriali** con il nome **Electric field**, e i risultati calcolati possono esservi applicati come a qualsiasi altra entità del modello.

## 9. Visualizzare i risultati

I risultati vengono visualizzati nella scheda **Risultati** sull'entità selezionata nell'**albero del modello**, come nel tutorial **Analisi delle tensioni e delle deformazioni**.

### 9.1. Temperatura e deformazione del volume

1. Nell'**albero del modello** selezionare **Volumi** (lasciando deselezionate le superfici), quindi selezionare **Volume 1**.
2. Nella scheda **Risultati** scegliere **Temperature - Scalar** nel menu a discesa, selezionare **Mappa colori** e **Mostra legenda**. Se l'intervallo mostrato non corrisponde ai risultati, fare clic su **Ripristina dai dati**.
3. Scegliere **Displacement - Vector [3]** nel menu a discesa, selezionare **Forma deformata** e impostare **Scala di deformazione e frecce** a **20** in modo che la deformazione sia visibile (gli spostamenti calcolati sono solo di circa 1,2 mm).
4. Scegliere di nuovo **Temperature - Scalar** in modo che la legenda mostri la temperatura.

La temperatura va da circa 468 K a 558 K; la trave è più calda al centro dei suoi due tratti esterni e si inflette verso l'alto perché le estremità vincolate non le permettono di dilatarsi liberamente.

![Temperatura sul volume](image-Applied_temperature_on_volume.png)

### 9.2. Temperatura sul campo vettoriale

1. Nell'**albero del modello** deselezionare **Volumi**, selezionare **Campi vettoriali** e selezionare **Electric field**.
2. Nella scheda **Risultati** scegliere **Temperature - Scalar**, selezionare **Mappa colori** e **Mostra legenda** (se non sono già selezionati). Le frecce sono ora colorate in base alla temperatura nella loro posizione.
3. Nelle **Proprietà di visualizzazione** sotto l'albero selezionare **Punte delle frecce** per mostrare la direzione del campo elettrico. Il campo elettrico è diretto dal centro della trave (potenziale più alto) verso le due estremità.

![Temperatura sul campo vettoriale](image-Applied_temperature_on_vector_field.png)
