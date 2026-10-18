# Risultati

I **risultati** sono prodotti dal **solutore** e sono costituiti da **variabili di risultato**.

## Variabile di risultato

Una **variabile di risultato** è un insieme di valori calcolati dal **solutore**. Una **variabile di risultato** è definita dal suo **tipo** (ad es. **temperatura** o **spostamento**) e da un vettore di valori. In base al **tipo**, una **variabile di risultato** può contenere un vettore di valori scalari o di valori vettoriali.


Ad esempio, la variabile di risultato **temperatura** contiene un vettore di temperature per ogni nodo, mentre lo **spostamento** contiene un vettore di vettori di spostamento per ogni nodo.


Una **variabile di risultato** può essere calcolata a livello di nodi o a livello di elementi. Ad esempio, lo **spostamento** è calcolato a livello di nodi, mentre la **tensione** a livello di elementi. Ogni variabile calcolata a livello di elementi può essere convertita a livello di nodi. La conversione di una variabile dal livello di elementi al livello di nodi è un'operazione una tantum e non può essere annullata; per farlo, i risultati devono essere ricalcolati dal **solutore**.
