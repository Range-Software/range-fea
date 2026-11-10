# Condizione ambientale

Una **condizione ambientale** è una condizione che può essere specificata su qualsiasi entità del modello, come Punto, Linea, Superficie o Volume. La **condizione ambientale** specifica valori propri del **problema** per l'ambiente dato (ad es. **Temperature** – temperatura dell'ambiente o **Gravitational acceleration** – accelerazione di gravità).

Le condizioni ambientali disponibili dipendono dai **tipi di problema** selezionati:

- **Gravitational acceleration** – componenti nelle direzioni X, Y e Z, per impostazione predefinita 0, 0 e -9.80665 `[m/s^2]`.
- **Temperature** – per impostazione predefinita 293.15 `[K]`.

## Modifica di una condizione ambientale

A differenza delle **condizioni al contorno**, le condizioni ambientali non vanno selezionate con una casella. Sono sempre presenti e hanno valori predefiniti.

1. Selezionare una o più entità nell'**albero del modello**.
2. Aprire la scheda **Condizioni ambientali** sul lato destro della finestra principale.
3. Selezionare la condizione ambientale e modificare i valori delle sue proprietà nell'elenco sottostante.

![Condizioni ambientali](image-Environment_conditions.png)

Se per una data entità del modello non è specificata alcuna **condizione iniziale**, viene usata la **condizione ambientale**.
