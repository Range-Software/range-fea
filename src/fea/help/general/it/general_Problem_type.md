# Tipo di problema

Il **tipo di problema** è un identificatore univoco del fenomeno fisico da risolvere.

I tipi di problema si selezionano nella finestra di dialogo **Seleziona problema** (**Menu:** _Problema -> Seleziona problema_). Fare clic su un riquadro per selezionare o deselezionare un tipo di problema e fare clic su **OK** per confermare. I pulsanti nell'angolo in alto a destra passano dalla **Vista a riquadri** alla **Vista elenco**, che mostra anche una breve descrizione di ogni tipo di problema.

![Seleziona problema](image-Select_problem.png)

Elenco dei tipi di problema supportati:

- **Acoustics** (acustica) – pressione acustica, velocità delle particelle, intensità e livello di pressione sonora, in regime transitorio e armonico (dominio della frequenza).
- **Electro-statics** (elettrostatica) – campo elettrico, densità di corrente e calore generato per effetto Joule.
- **Magneto-statics** (magnetostatica) – campo magnetico.
- **Incompressible viscous flow** (flusso viscoso incomprimibile) – flusso stazionario e transitorio di fluidi newtoniani.
- **Contaminant dispersion** (dispersione di contaminante) – distribuzione di un «contaminante» nel campo di flusso.
- **Heat transfer in fluids** (trasmissione del calore nei fluidi) – distribuzione della temperatura in un fluido in movimento.
- **Heat transfer** (trasmissione del calore) – distribuzione della temperatura e flusso di calore stazionari e transitori, con convezione semplice, forzata e naturale.
- **Radiative heat transfer** (trasmissione del calore per irraggiamento) – trasmissione del calore tra più superfici in grado di emettere e/o ricevere calore.
- **Stress analysis** (analisi delle tensioni) – analisi statica delle tensioni ed elastodinamica lineare, comprese la dilatazione termica e i carichi dovuti alla pressione di un fluido.
- **Modal analysis** (analisi modale) – frequenze naturali della struttura e relative forme modali.
- **Flow through porous media** (flusso attraverso mezzi porosi) – pressione e velocità di un fluido che attraversa un materiale permeabile.
- **Mesh reconstruction** (ricostruzione della mesh) – ricostruzione della mesh in base ai criteri forniti.

## Combinazione dei tipi di problema

È possibile combinare più **tipi di problema** diversi (ad es. **Heat transfer** e **Stress analysis** oppure **Incompressible viscous flow** e **Contaminant dispersion**).

- Alcuni tipi di problema ne richiedono un altro. Selezionando uno di essi viene selezionato anche quello richiesto: **Magneto-statics** richiede **Electro-statics**, **Contaminant dispersion** e **Heat transfer in fluids** richiedono **Incompressible viscous flow** e **Radiative heat transfer** richiede **Heat transfer**.
- I tipi di problema che non possono essere combinati con la selezione corrente sono disattivati e contrassegnati come **Incompatibile** (ad es. **Modal analysis** non può essere combinato con nessun altro tipo di problema).

## Flusso delle attività

I tipi di problema selezionati vengono risolti uno dopo l'altro come attività. Il loro ordine viene impostato automaticamente, in modo che i risultati di un'attività possano essere usati dalla successiva (ad es. **Heat transfer** viene risolto prima di **Stress analysis** per tenere conto della dilatazione termica).

Fare clic su **Personalizza…** per vedere o modificare il flusso delle attività. Le attività vengono eseguite dall'alto verso il basso. Usare i pulsanti **Su** e **Giù** per cambiarne l'ordine, **>** per inserire un'attività in un gruppo che si ripete (con un proprio numero di iterazioni e un proprio valore di convergenza) e **<** per toglierla dal gruppo. Fare doppio clic su un valore per modificarlo. **Usa ordine automatico** annulla il flusso delle attività personalizzato.

![Seleziona problema – flusso delle attività personalizzato](image-Select_problem-customize.png)

Se un tipo di problema viene rimosso da un modello che ne contiene già i risultati, l'applicazione propone di eliminare tali risultati dal modello.
