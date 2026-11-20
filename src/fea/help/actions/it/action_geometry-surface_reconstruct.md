# Ricostruisci superficie

Questa azione ritriangola le superfici selezionate. Unisce gli elementi complanari, rimuove i nodi che non definiscono la forma (nodi all'interno di regioni piane e su spigoli rettilinei) e rimuove gli elementi minuscoli e allungati.

I nodi esistenti non vengono mai spostati, nuovi nodi vengono creati solo dall'opzione mesh di qualità. I nodi e gli spigoli condivisi con altre entità (punti, linee, superfici non selezionate) vengono preservati, le superfici chiuse restano chiuse e non vengono create nuove autointersezioni. Vengono elaborati solo triangoli lineari e la mesh di volume deve essere rimossa prima.

* **Tolleranza angolare** - angolo massimo tra le normali di elementi complanari e tra le direzioni di spigoli collineari.
* **Tolleranza di distanza** - distanza massima di un nodo da un piano (retta) per essere considerato complanare (collineare).
* **Lunghezza minima dello spigolo** - gli spigoli più corti vengono collassati. I dettagli più piccoli di questo valore possono scomparire.
* **Angolo di scheggia** - gli elementi con l'angolo minimo inferiore a questo valore sono considerati schegge.
* **Triangolazione di Delaunay** - al termine esegue la triangolazione di Delaunay della superficie: gli spigoli i cui angoli opposti sommano a più di 180 gradi vengono ribaltati, purché la forma sia preservata entro la tolleranza di distanza.
* **Mesh di qualità (inserimento nodi)** - inserisce nuovi nodi (raffinamento di Delaunay) in modo che l'angolo minimo degli elementi sia almeno l'**Angolo di qualità** e gli spigoli non siano più lunghi della **Lunghezza massima dello spigolo** (zero significa nessun limite). I nuovi nodi vengono posizionati sulla superficie esistente e sugli spigoli rettilinei, quindi la forma è preservata. Gli spigoli condivisi con entità non selezionate non vengono suddivisi. Le superfici curve composte da faccette inclinate singolarmente possono essere migliorate solo in parte.

Le tolleranze predefinite sono relative alle dimensioni delle superfici selezionate. Tolleranze maggiori producono superfici più semplificate.
