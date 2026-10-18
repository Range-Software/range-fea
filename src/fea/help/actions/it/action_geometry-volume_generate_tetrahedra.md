# Genera mesh tetraedrica

Genera una mesh di volume (tetraedrica).


Una **mesh di volume** è necessaria per la maggior parte dei **tipi di problema**.


Il generatore di mesh 3D può generare una mesh solo se:
1. il modello contiene una superficie chiusa (a tenuta stagna),
2. gli elementi di superficie non si intersecano tra loro.


Per generare più mesh di volume che rappresentino un modello di materiale composito, deve essere inclusa una superficie di separazione.


In alcuni casi può essere necessario sovrascrivere le impostazioni predefinite e specificare direttamente i parametri di TetGen.

## Interfaccia utente

![Finestra Genera mesh](image-Generate_mesh_dialog.png)

- **Verifica integrità della superficie** – utile quando si genera una mesh 3D da un modello di superficie. Rileva errori e potenziali problemi della mesh di superficie.
- **Ricostruisci mesh (riusa i nodi attuali)** – conserva i nodi attuali e ne aggiunge di nuovi solo se necessario.
- **Mantieni i risultati calcolati** – tutti i risultati calcolati vengono mappati/interpolati sui nuovi nodi ed elementi.
- **Mesh di qualità** – genera una mesh 3D conforme ai criteri di qualità.
    - **Volume massimo dell'elemento** – volume massimo di ogni elemento generato. Nessun elemento dovrebbe avere un volume superiore a quello specificato.
    - **Genera funzione di dimensione della mesh** – da usare per raffinare la mesh nei punti in cui i risultati variano fortemente.
- **Parametri TetGen** – parametri passati al generatore di mesh TetGen.

L'immagine seguente mostra la differenza nella risoluzione della mesh quando si usa la generazione adattiva.

![Esempio di mesh adattiva](image-Adaptive_mesh-example.png)

## Sintassi dei parametri

`pYrq_Aa_miO_S_T_XMwcdzfenvgkJBNEFICQVh`


I trattini bassi indicano che alcuni interruttori possono essere seguiti facoltativamente da numeri. Non lasciare spazi tra un interruttore e il suo parametro numerico.


Segue una panoramica di tutti gli interruttori della riga di comando con una breve descrizione.

- **p** - Tetraedrizza un complesso lineare a tratti (PLC).
- **r** - Ricostruisce una mesh generata in precedenza.
- **q** - Raffina la mesh (per migliorarne la qualità).
- **R** - Semplificazione della mesh (per ridurre il numero di elementi).
- **a** - Applica un vincolo sul volume massimo dei tetraedri.
- **O** - Specifica il livello di ottimizzazione della mesh.
- **S** - Specifica il numero massimo di punti aggiunti.
- **T** - Imposta una tolleranza per il test di complanarità (predefinita 10−8).
- **X** - Disattiva l'uso dell'aritmetica esatta.
- **M** - Nessuna unione di facce complanari o di vertici molto vicini.
- **w** - Genera una triangolazione di Delaunay pesata (regolare).
- **c** - Conserva l'inviluppo convesso del PLC.
- **d** - Rileva le autointersezioni delle facce del PLC.
- **n** - Restituisce i vicini dei tetraedri.
- **C** - Verifica la consistenza della mesh finale.
- **Q** - Silenzioso: nessun output sul terminale tranne gli errori.
- **V** - Dettagliato: informazioni dettagliate, più output sul terminale.
