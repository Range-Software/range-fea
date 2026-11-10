# Risultati

I **risultati** sono prodotti dal **solutore** e sono costituiti da **variabili di risultato**.

## Variabile di risultato

Una **variabile di risultato** è un insieme di valori calcolati dal **solutore**. Una **variabile di risultato** è definita dal suo **tipo** (ad es. **Temperature** o **Displacement**) e da un vettore di valori. In base al **tipo**, una **variabile di risultato** può contenere un vettore di valori scalari o vettoriali.

Ad esempio, la variabile di risultato **Temperature** contiene un vettore di temperature per ogni nodo, mentre **Displacement** contiene un vettore di vettori di spostamento per ogni nodo.

Tutte le variabili di risultato del modello selezionato sono elencate nel menu a discesa nella parte superiore della scheda **Risultati**, insieme al loro genere (**Scalar** o **Vector**).

![Variabili di risultato](image-Results_variables.png)

Una **variabile di risultato** può essere calcolata a livello di nodo o a livello di elemento. Ad esempio **Displacement** è calcolato a livello di nodo, mentre **Von Mises stress** è calcolato a livello di elemento. Dove è memorizzata la variabile selezionata (**Su nodi** o **Su elementi**) è indicato sotto il menu a discesa.

Le variabili a livello di elemento sono visualizzate con un valore per elemento. Per ottenere una visualizzazione continua:

- Le variabili di tensione offrono la casella **Visualizzazione levigata**. Essa media i risultati degli elementi nei nodi solo per la visualizzazione; i risultati stessi restano invariati. La media riduce i valori di picco.
- Le altre variabili a livello di elemento offrono il pulsante **Mostra sui nodi**, che sposta la variabile dagli elementi ai nodi.

## Visualizzazione dei risultati

I risultati sono visualizzati sulle entità selezionate nell'**albero del modello**. I risultati di un problema risolto sui **Volumi** si vedono meglio quando le entità di superficie sono nascoste.

![Risultati](image-Results.png)

- **Mostra sulle entità selezionate** – selezionare **Mappa colori** per colorare le entità selezionate in base al valore della variabile. Le variabili vettoriali possono essere mostrate anche come **Forma deformata**.
- **Intervallo colori** – valori **Max** e **Min** della mappa colori. Possono essere digitati o modificati nella **Regolazione fine**. **Ripristina dai dati** riporta l'intervallo a quello calcolato dai risultati.
- **Colori** – schema di colori della mappa colori.
- **Mostra legenda** – mostra l'intervallo di valori nell'area 3D.
- **Scala di deformazione e frecce** – moltiplica la deformazione delle forme deformate e la lunghezza delle frecce dei campi vettoriali (solo variabili vettoriali).
- **Grafico…** – mostra la distribuzione dei valori e consente di impostare l'intervallo colori su un grafico.
- **Rimuovi** – rimuove la variabile dal modello.

I risultati possono essere visualizzati anche su entità create a tale scopo: **Sezioni**, **ISO**, **Linee di flusso**, **Campi scalari** e **Campi vettoriali**.
