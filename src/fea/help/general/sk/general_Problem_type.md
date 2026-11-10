# Typ úlohy

**Typ úlohy** je jednoznačný identifikátor fyzikálneho javu, ktorý sa má riešiť.

Typy úloh sa vyberajú v dialógu **Vybrať úlohu** (**Menu:** _Úloha -> Vybrať úlohu_). Kliknutím na dlaždicu typ úlohy vyberiete alebo jeho výber zrušíte, kliknutím na **OK** výber potvrdíte. Tlačidlá v pravom hornom rohu prepínajú medzi **Zobrazením dlaždíc** a **Zobrazením zoznamu**, ktoré ukazuje aj krátky popis každého typu úlohy.

![Vybrať úlohu](image-Select_problem.png)

Zoznam podporovaných typov úloh:

- **Acoustics** (akustika) – prechodový a harmonický (vo frekvenčnej oblasti) akustický tlak, rýchlosť častíc, intenzita a hladina akustického tlaku.
- **Electro-statics** (elektrostatika) – elektrické pole, prúdová hustota a generované Joulovo teplo.
- **Magneto-statics** (magnetostatika) – magnetické pole.
- **Incompressible viscous flow** (nestlačiteľné viskózne prúdenie) – ustálené a prechodové prúdenie newtonovských tekutín.
- **Contaminant dispersion** (šírenie kontaminantu) – rozloženie „kontaminantu“ v prúdovom poli.
- **Heat transfer in fluids** (prestup tepla v tekutinách) – rozloženie teploty v prúdiacej tekutine.
- **Heat transfer** (prestup tepla) – ustálené a prechodové rozloženie teploty a tepelný tok, s jednoduchou, nútenou a prirodzenou konvekciou.
- **Radiative heat transfer** (prestup tepla žiarením) – prestup tepla medzi viacerými povrchmi schopnými vyžarovať a/alebo prijímať teplo.
- **Stress analysis** (analýza napätia) – statická analýza napätia a lineárna elastodynamika vrátane tepelnej rozťažnosti a zaťaženia tlakom tekutiny.
- **Modal analysis** (modálna analýza) – vlastné frekvencie konštrukcie a ich vlastné tvary.
- **Flow through porous media** (prúdenie pórovitým prostredím) – tlak a rýchlosť tekutiny prúdiacej priepustným materiálom.
- **Mesh reconstruction** (rekonštrukcia siete) – rekonštrukcia siete podľa zadaných kritérií.

## Kombinovanie typov úloh

Je možné kombinovať viacero rôznych **typov úloh** (napr. **Heat transfer** a **Stress analysis** alebo **Incompressible viscous flow** a **Contaminant dispersion**).

- Niektoré typy úloh vyžadujú iný typ úlohy. Výberom takého typu sa vyberie aj ten vyžadovaný: **Magneto-statics** vyžaduje **Electro-statics**, **Contaminant dispersion** a **Heat transfer in fluids** vyžadujú **Incompressible viscous flow** a **Radiative heat transfer** vyžaduje **Heat transfer**.
- Typy úloh, ktoré nemožno kombinovať s aktuálnym výberom, sú zašednuté a označené ako **Nekompatibilná** (napr. **Modal analysis** nemožno kombinovať so žiadnym iným typom úlohy).

## Poradie krokov úlohy

Vybrané typy úloh sa riešia jeden po druhom ako jednotlivé kroky. Ich poradie sa nastaví automaticky tak, aby výsledky jedného kroku mohol využiť nasledujúci krok (napr. **Heat transfer** sa rieši pred **Stress analysis**, aby sa zohľadnila tepelná rozťažnosť).

Kliknutím na **Prispôsobiť…** možno poradie krokov zobraziť alebo zmeniť. Kroky sa vykonávajú zhora nadol. Tlačidlami **Hore** a **Dole** zmeníte ich poradie, tlačidlom **>** vložíte krok do skupiny, ktorá sa opakuje (s vlastným počtom iterácií a hodnotou konvergencie), a tlačidlom **<** ho zo skupiny vyberiete. Dvojklikom na hodnotu ju upravíte. **Použiť automatické poradie** zahodí vlastné poradie krokov.

![Vybrať úlohu – vlastné poradie krokov](image-Select_problem-customize.png)

Ak sa z modelu, ktorý už obsahuje výsledky, odoberie typ úlohy, aplikácia ponúkne vymazanie týchto výsledkov z modelu.
