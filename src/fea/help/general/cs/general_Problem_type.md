# Typ úlohy

**Typ úlohy** je jednoznačný identifikátor fyzikálního jevu, který se má řešit.

Typy úloh se vybírají v dialogu **Vybrat úlohu** (**Menu:** _Úloha -> Vybrat úlohu_). Kliknutím na dlaždici typ úlohy vyberete nebo jeho výběr zrušíte, kliknutím na **OK** výběr potvrdíte. Tlačítka v pravém horním rohu přepínají mezi **Zobrazením dlaždic** a **Zobrazením seznamu**, které ukazuje i krátký popis každého typu úlohy.

![Vybrat úlohu](image-Select_problem.png)

Seznam podporovaných typů úloh:

- **Acoustics** (akustika) – přechodový a harmonický (ve frekvenční oblasti) akustický tlak, rychlost částic, intenzita a hladina akustického tlaku.
- **Electro-statics** (elektrostatika) – elektrické pole, proudová hustota a generované Jouleovo teplo.
- **Magneto-statics** (magnetostatika) – magnetické pole.
- **Incompressible viscous flow** (nestlačitelné viskózní proudění) – ustálené a přechodové proudění newtonských tekutin.
- **Contaminant dispersion** (šíření kontaminantu) – rozložení „kontaminantu“ v proudovém poli.
- **Heat transfer in fluids** (přenos tepla v tekutinách) – rozložení teploty v proudící tekutině.
- **Heat transfer** (přenos tepla) – ustálené a přechodové rozložení teploty a tepelný tok, s jednoduchou, nucenou a přirozenou konvekcí.
- **Radiative heat transfer** (přenos tepla zářením) – přenos tepla mezi několika povrchy schopnými vyzařovat a/nebo přijímat teplo.
- **Stress analysis** (analýza napětí) – statická analýza napětí a lineární elastodynamika, včetně tepelné roztažnosti a zatížení tlakem tekutiny.
- **Modal analysis** (modální analýza) – vlastní frekvence konstrukce a jejich vlastní tvary.
- **Flow through porous media** (proudění porézním prostředím) – tlak a rychlost tekutiny proudící propustným materiálem.
- **Mesh reconstruction** (rekonstrukce sítě) – rekonstrukce sítě podle zadaných kritérií.

## Kombinování typů úloh

Je možné kombinovat více různých **typů úloh** (např. **Heat transfer** a **Stress analysis** nebo **Incompressible viscous flow** a **Contaminant dispersion**).

- Některé typy úloh vyžadují jiný typ úlohy. Výběrem takového typu se vybere i ten vyžadovaný: **Magneto-statics** vyžaduje **Electro-statics**, **Contaminant dispersion** a **Heat transfer in fluids** vyžadují **Incompressible viscous flow** a **Radiative heat transfer** vyžaduje **Heat transfer**.
- Typy úloh, které nelze kombinovat s aktuálním výběrem, jsou zašedlé a označené jako **Nekompatibilní** (např. **Modal analysis** nelze kombinovat s žádným jiným typem úlohy).

## Pořadí kroků úlohy

Vybrané typy úloh se řeší jeden po druhém jako jednotlivé kroky. Jejich pořadí se nastaví automaticky tak, aby výsledky jednoho kroku mohl využít krok následující (např. **Heat transfer** se řeší před **Stress analysis**, aby se zohlednila tepelná roztažnost).

Kliknutím na **Přizpůsobit…** lze pořadí kroků zobrazit nebo změnit. Kroky se provádějí shora dolů. Tlačítky **Nahoru** a **Dolů** změníte jejich pořadí, tlačítkem **>** vložíte krok do skupiny, která se opakuje (s vlastním počtem iterací a hodnotou konvergence), a tlačítkem **<** jej ze skupiny vyjmete. Dvojklikem na hodnotu ji upravíte. **Použít automatické pořadí** zahodí vlastní pořadí kroků.

![Vybrat úlohu – vlastní pořadí kroků](image-Select_problem-customize.png)

Je-li z modelu, který již obsahuje výsledky, odebrán typ úlohy, aplikace nabídne vymazání těchto výsledků z modelu.
