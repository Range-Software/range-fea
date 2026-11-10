# Condizione al contorno

Una **condizione al contorno** è una condizione specificata sul contorno, dove il contorno può essere qualsiasi entità del modello (Punto, Linea, Superficie, Volume).

Le condizioni al contorno sono:
- **Specifiche del problema** – Ogni **tipo di problema** definisce il proprio insieme di **condizioni al contorno**.
- **Specifiche dell'entità** – Non tutte le condizioni al contorno possono essere applicate a tutti i tipi di entità (ad es. **Pressure** può essere applicata solo a una **Superficie**).
- **Esplicite** – Sul contorno è prescritta la soluzione esatta (ad es. **Temperature** per **Heat transfer**). Sono mostrate in grassetto nell'elenco **Condizioni al contorno**.
- **Implicite** – Implicano una soluzione sul contorno (ad es. **Heat** per **Heat transfer**).

## Assegnazione di una condizione al contorno

1. Selezionare una o più entità nell'**albero del modello**.
2. Aprire la scheda **Condizioni al contorno** sul lato destro della finestra principale. L'elenco contiene solo le condizioni al contorno valide per i **tipi di problema** selezionati e per le entità selezionate.
3. Selezionare la casella davanti al nome della condizione al contorno per assegnarla. Deselezionarla per rimuovere la condizione al contorno dalle entità selezionate.
4. Selezionare la condizione al contorno per modificare i valori delle sue proprietà nell'elenco sottostante.

![Condizioni al contorno](image-Boundary_conditions.png)

Alcune proprietà hanno una propria casella. Ad esempio ogni componente della condizione al contorno **Displacement** può essere deselezionata per lasciare libero lo spostamento in quella direzione.

Le condizioni al contorno che agiscono in una direzione data dalla geometria (**Normal displacement**, **Roller displacement**) ricavano la propria direzione locale dall'entità. Selezionare **Usa la direzione locale inserita** per inserire la direzione a mano.

Per mostrare le condizioni al contorno assegnate nell'area 3D, selezionare **Condizioni al contorno** nelle **Proprietà di visualizzazione**.

## Valori dipendenti dal tempo

Il valore di ogni proprietà può dipendere dal tempo. Espandere la proprietà e fare clic su **Modifica valori dipendenti dal tempo** per inserire una tabella di valori per determinati istanti. I valori dipendenti dal tempo vengono usati quando il problema è **transitorio** (dipendente dal tempo), cioè quando nella scheda **Problema** è attivato il **Solutore temporale**.
