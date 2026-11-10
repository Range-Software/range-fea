# Modello

Un **modello** è una rappresentazione geometrica virtuale di un oggetto tridimensionale. Un modello è costituito da **entità** geometriche.

![Modello](image-Model.png)

## Entità

Un'**entità** è un insieme di elementi (simplessi geometrici) dello stesso tipo. Si distinguono i seguenti tipi:

- **Punto** – insieme di elementi 0D (nodo).
- **Linea** – insieme di elementi 1D (segmento).
- **Superficie** – insieme di elementi 2D (triangolo, quadrilatero).
- **Volume** – insieme di elementi 3D (tetraedro).

Indipendentemente dal fatto che gli elementi siano 0D, 1D, 2D o 3D, sono tutti rappresentati nello spazio 3D. Ad esempio un punto (nodo) non ha dimensioni, ha solo una posizione, mentre una linea ha una dimensione (lunghezza).

Oltre a queste entità geometriche, un modello può contenere entità create dai **risultati** calcolati: **Sezioni**, **ISO**, **Linee di flusso**, **Campi scalari** e **Campi vettoriali**.

## Albero del modello

Tutte le entità di un modello sono elencate nell'**albero del modello** (scheda **Modello** sul lato sinistro della finestra principale), raggruppate per tipo. La colonna **#** mostra il numero di entità di un gruppo o il numero di elementi di un'entità.

![Albero del modello](image-Model_tree.png)

- La casella davanti al nome mostra o nasconde il modello, il gruppo o l'entità nell'area 3D.
- Facendo clic su una riga la si seleziona. Per selezionare più entità, tenere premuto il tasto _Ctrl_ (_⌘ Cmd_ su macOS) durante la selezione, oppure il tasto _Maiusc_ per selezionare un intervallo. Selezionando un modello o un gruppo si selezionano tutte le entità che contiene.
- **Condizioni al contorno**, **Condizioni iniziali**, **Condizioni ambientali**, **Materiale** e **Risultati** vengono sempre assegnati alle entità selezionate nell'**albero del modello**.

Le **Proprietà di visualizzazione**, sotto l'albero, determinano come vengono disegnate le entità selezionate (ad es. **Come wireframe**, **Bordi degli elementi**, **Nodi degli elementi**, **Condizioni al contorno**) e il loro colore.

## Proprietà geometriche

A ogni **entità**, tranne al **Volume**, può essere assegnata una proprietà **geometrica** che ne completa la rappresentazione 3D. Per le entità selezionate è mostrata nell'elenco **Proprietà geometriche**, sotto l'**albero del modello**:

- **Volume del punto** – volume di un **Punto** in metri cubi.
- **Area della sezione della linea** – area della sezione trasversale di una **Linea** in metri quadrati.
- **Spessore della superficie** – spessore di una **Superficie** in metri.
