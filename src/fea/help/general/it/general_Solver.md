# Solutore

Il **solutore** è uno strumento che elabora le impostazioni del **problema** e calcola i **risultati** su un dato **modello**.

A seconda delle dimensioni del **modello**, del **tipo di problema**, delle impostazioni di convergenza e della configurazione transitoria, il tempo necessario per risolvere il **problema** può variare da secondi a ore e oltre.

## Avvio del solutore

**Menu:** _Soluzione -> Avvia solutore_

Il modello deve essere salvato prima che il solutore possa essere avviato. Se non è ancora stato salvato, viene mostrata prima la finestra di dialogo **Salva modello**. Viene poi mostrata la finestra di dialogo **Avvia solutore**.

![Avvia solutore](image-Start_solver.png)

Prima dell'avvio del solutore viene controllata la configurazione del problema. Gli **Errori** (ad es. un'entità senza materiale) e gli **Avvisi** trovati sono elencati nella finestra di dialogo **Avvia solutore**. Se il modello contiene già dei risultati, si può selezionare **Riavvia solutore / continua** per proseguire da tali risultati invece di ricominciare dall'inizio.

Il solutore viene eseguito in background come processo separato, quindi è possibile continuare a lavorare durante il calcolo. Tutto il suo output è mostrato nella scheda **Output del processo**, sotto l'area 3D. Al termine del solutore, i **risultati** calcolati vengono caricati automaticamente nel modello.

## Arresto del solutore

- _Soluzione -> Arresta solutore_ – chiede a tutti i solutori in esecuzione di arrestarsi.
- _Soluzione -> Termina solutore_ – termina immediatamente il processo del solutore.

## Monitoraggio del solutore

- _Report -> Convergenza del solutore_ – mostra un grafico della convergenza del solutore.
- _Report -> Punti di monitoraggio_ – mostra i valori calcolati nei punti di monitoraggio definiti con _Problema -> Definisci punti di monitoraggio_.
- _Report -> File di log del solutore_ – mostra il file di log del solutore.
