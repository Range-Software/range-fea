# Problema

**Problema** è un termine generale che indica uno o più fenomeni fisici applicati a una geometria. È definito dal **tipo di problema**, dalla **configurazione specifica del problema**, dalle **condizioni al contorno**, dalle **condizioni iniziali**, dalle **condizioni ambientali** e dalle **impostazioni del materiale**.

Il problema viene definito separatamente per ogni modello. Le sue parti si impostano nelle schede sul lato destro della finestra principale: **Problema**, **Condizioni al contorno**, **Condizioni iniziali**, **Condizioni ambientali** e **Materiale**.

## Selezione di un problema

**Menu:** _Problema -> Seleziona problema_

La finestra di dialogo **Seleziona problema** serve a scegliere che cosa deve essere risolto (vedere [Tipo di problema](general_Problem_type.md)). Viene mostrata anche automaticamente quando si carica un modello senza alcun problema. Finché non viene selezionato un problema, la scheda **Problema** contiene solo il pulsante **Seleziona problema**.

## Configurazione specifica del problema

Una volta selezionato un problema, la scheda **Problema** contiene la configurazione specifica dei **tipi di problema** selezionati.

![Scheda Problema](image-Problem_tab.png)

- **Solutore temporale** – selezionare per risolvere il problema come **transitorio** (dipendente dal tempo). Una volta selezionato, si possono impostare il metodo di approssimazione, il tempo iniziale, l'ampiezza del passo temporale, il numero di passi temporali e la frequenza di output; il tempo finale viene calcolato da questi valori (_tempo finale = tempo iniziale + ampiezza del passo temporale * numero di passi temporali_). Se non è selezionato, il problema viene risolto come **stazionario**. Il solutore temporale è disponibile solo per i tipi di problema che lo supportano.
- Le altre sezioni dipendono dai **tipi di problema** selezionati (ad es. **Impostazioni dell'analisi transitoria delle tensioni**, impostazioni di irraggiamento, acustica o analisi modale).

Altre azioni relative al problema si trovano nel menu **Problema**: _Configura il solutore matriciale dei problemi_, _Definisci punti di monitoraggio_ e _Reimposta configurazione_.
