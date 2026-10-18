# Rozptyl kontaminantu v tekutinách

Tento návod ukáže, ako nastaviť a vyriešiť multifyzikálnu simuláciu, ktorá spája nelineárnu iteratívnu úlohu – **CFD (výpočtovú dynamiku tekutín)** – s prenosom kontaminantu unášaného prúdením.

Potrebné sú dva typy úloh:

1. **Incompressible viscous flow** (nestlačiteľné viskózne prúdenie) – počíta rýchlosť a tlak tekutiny (ustálené alebo prechodové prúdenie newtonovských tekutín).
2. **Contaminant dispersion** (rozptyl kontaminantu) – počíta, ako sa kontaminant v tomto prúdovom poli šíri.

Keďže úloha prúdenia je nelineárna, rieši sa iteratívne. Simulácia preto prebieha v dvoch etapách:

1. **Ustálené prúdenie** – najprv sa vypočíta skonvergované „počiatočné" prúdové pole a rozloženie tlaku, zatiaľ bez kontaminantu.
2. **Prechodová simulácia** – potom sa z tohto prúdového poľa pomocou **časového krokovania** sleduje, ako je impulz kontaminantu vstupujúci do kanála unášaný prúdením.

## 1. Načítať model

**Menu:** _Súbor -> Otvoriť model_

V dialógu **Otvoriť model** vyberte **Channel.tmsh** a kliknite na **Otvoriť**.

Model pozostáva z troch celkov povrchu – **Walls** (steny), **Inflow** (prítok) a **Outflow** (odtok). Keďže modelu zatiaľ nie je priradený žiadny typ úlohy, po dokončení načítania sa automaticky otvorí dialóg **Poradie krokov úlohy**.

## 2. Poradie krokov úlohy (etapa 1 – ustálené prúdenie)

V prvej etape sa rieši iba prúdenie, a to s veľkým počtom nelineárnych iterácií, aby riešenie mohlo skonvergovať.

1. V dialógu **Poradie krokov úlohy** kliknite na **Pridať typ úlohy**.
2. V dialógu **Výber typu úlohy** zaškrtnite **Incompressible viscous flow** a kliknite na **OK**.
3. Dvakrát kliknite na hodnotu **1** vedľa **počet iterácií:**, napíšte **2000** a stlačte _Enter_.
4. Kliknutím na **OK** dialóg zatvorte.

Dialóg možno kedykoľvek znova otvoriť tlačidlom **Poradie krokov úloh** na paneli nástrojov. Teraz by mal vyzerať takto:

![CFD – Poradie krokov úlohy – ustálené prúdenie](image-CFD-Problem_task_flow_steady.png)

## 3. Vygenerovať objemovú sieť

Model je definovaný iba svojimi povrchmi. Aby bolo možné riešiť prúdenie v jeho vnútri, treba najprv vygenerovať objemovú (tetrahedrónovú) sieť.

**Menu:** _Geometria -> Objem -> Generovať tetrahedrónovú sieť_

Predvyplnené hodnoty v dialógu **Vygenerovať 3D sieť** sú postačujúce. Kliknite na **OK**. Po dokončení generovania sa v strome **Model** v skupine **Objemy** objaví nový celok **Volume 1**.

## 4. Priradiť materiál

Všetky celky modelu musia mať priradený materiál.

1. V strome **Model** kliknite na **Walls** a potom so stlačeným klávesom _Shift_ kliknite na **Volume 1**, čím vyberiete všetky celky.
2. Otvorte záložku **Materiál** na pravej strane hlavného okna.
3. Zaškrtnite **Water** (voda).

## 5. Okrajové podmienky (etapa 1)

Okrajové podmienky sa priraďujú v záložke **Okrajové podmienky** celku vybranému v strome **Model**. **Celkom povrchu** priraďte tieto podmienky:

1. **Walls**
    - _Wall_ (stena; hodnota nie je potrebná)
2. **Inflow**
    - _Volumetric flow rate (inflow)_ (objemový prietok – prítok)
        - Volumetric flow rate = 50 `[m^3/s]`
3. **Outflow**
    - _Pressure (implicit)_ (tlak – implicitný)
        - Pressure = 0 `[Pa]` (predvolená hodnota je 1000 `[Pa]`)

Pre každý celok: vyberte ho v strome **Model**, v záložke **Okrajové podmienky** zaškrtnite podmienku, vyberte ju, aby sa pod zoznamom zobrazili jej vlastnosti, potom dvakrát kliknite na hodnotu, napíšte novú a stlačte _Enter_.

## 6. Vyriešiť ustálené prúdenie

**Menu:** _Riešenie -> Spustiť riešič_

Ak model ešte nebol uložený, najprv sa zobrazí dialóg **Uložiť model**. Kliknutím na **Uložiť** prijmite navrhnutý názov súboru (**Channel.rbm**) a potom v dialógu **Spustiť riešič pre model** kliknite na **OK**.

Riešič beží na pozadí a jeho výstup sa zobrazuje v záložke **Výstup procesu**. Výpočet všetkých 2000 iterácií trvá niekoľko minút; výpočet je dokončený, keď výstup skončí riadkom **SOLVER - END**. Konvergenciu riešenia možno kedykoľvek skontrolovať pomocou:

**Menu:** _Výkaz -> Konvergencia riešiča_

## 7. Vytvoriť rez

Prúdenie možno dobre preskúmať zobrazením rýchlosti ako **vektorového poľa**, v ktorom každá šípka ukazuje smer a veľkosť prúdenia v jednom bode. Vektorové pole vykreslené v celom objeme sa však rýchlo zmení na hustý mrak prekrývajúcich sa šípok, ktorý sa ťažko číta. Zvyčajne je prehľadnejšie najprv model rozrezať **rezom** – rovinou prechádzajúcou objemom – a vektorové pole potom zobraziť iba na tejto rovine.

_Tip: pred začatím zaškrtnite v strome **Model** skupinu **Objemy**, aby bol objem počas umiestňovania rezu viditeľný._

**Menu:** _Geometria -> Rez -> Vytvoriť rez_

V dialógu **Editor rezu**:

1. V zozname **Model** vyberte **Volume 1**. Rez sa vytvorí cez tento objem.
2. **Poloha roviny** určuje bod, ktorým rovina rezu prechádza. Predvolené hodnoty ju umiestňujú do stredu modelu (**X** = 7.5, **Y** = 2.5, **Z** = 1.5 `[m]`); ponechajte ich.
3. **Normála roviny** určuje orientáciu roviny. Predvolená normála (**X** = 1) dáva zvislú rovinu; zmeňte ju na **X** = 0, **Y** = 0, **Z** = 1, čím získate vodorovnú rovinu prechádzajúcu kanálom v polovici jeho výšky.

Počas zmeny hodnôt sa rovina rezu priebežne zobrazuje v **3D oblasti modelu**. Kliknutím na **OK** rez vytvoríte. V strome **Model** sa v skupine **Rezy** objaví nový celok **Cut 1**.

![CFD – Vytvoriť rez](image-CFD-Create_cut.png)

## 8. Vytvoriť vektorové pole

Teraz zobrazte vypočítanú rýchlosť ako šípky na rovine rezu.

**Menu:** _Geometria -> Vektorové pole -> Vytvoriť vektorové pole_

V dialógu **Editor vektorového poľa**:

1. V zozname **Model** vyberte **Cut 1**. Šípky sa vykreslia na rovine rezu vytvorenej v predchádzajúcom kroku.
2. V zozname **Premenné** vyberte **Velocity** (rýchlosť).
3. Ponechajte zaškrtnuté **3D vektorové pole**. Šípky potom ukazujú celý vektor rýchlosti. Ak políčko nie je zaškrtnuté, každá šípka sa premietne do roviny rezu a zobrazí sa iba zložka prúdenia ležiaca v rovine.

Kliknutím na **OK** vektorové pole vytvoríte. Objaví sa v strome **Model** v skupine **Vektorové polia**. Zrušte zaškrtnutie skupiny **Objemy**, aby plný objem nezakrýval šípky.

![CFD – Vytvoriť vektorové pole](image-CFD-Create_vector.png)

Šípky môžu byť príliš krátke na to, aby boli dobre viditeľné, pretože ich dĺžka je úmerná rýchlosti prúdenia. Ak ich chcete predĺžiť, vyberte vektorové pole v strome **Model**, otvorte záložku **Výsledky** a zvýšte hodnotu v časti **Mierka**. Mierka je súčinom hodnoty **Mierka** a mocniny desiatich nastavenej v poli **Faktor**; v príklade nižšie dávajú **Mierka** 1 a **Faktor** 2 mierku `1 × 10^2 = 100`.

![CFD – Mierka vektorového poľa](image-CFD-Scale_vector.png)

## 9. Poradie krokov úlohy (etapa 2 – prechodová simulácia)

Keď je k dispozícii skonvergované prúdové pole, možno nastaviť **prechodovú** simuláciu vrátane úlohy **Contaminant dispersion**.

V prechodovej simulácii sa úlohy riešia raz v každom časovom kroku. **Incompressible viscous flow** je stále nelineárna úloha, preto v každom časovom kroku potrebuje vlastné nelineárne iterácie, zatiaľ čo **Contaminant dispersion** sa rieši raz za časový krok po vyriešení prúdenia. Znova otvorte dialóg **Poradie krokov úlohy** (tlačidlo **Poradie krokov úloh** na paneli nástrojov) a nastavte ho takto:

1. Vyberte **Incompressible viscous flow** a kliknite na tlačidlo **>**. Úloha sa tým presunie do novej, vnorenej skupiny iterácií.
2. Dvakrát kliknite na hodnotu vonkajšieho **počet iterácií:** a zmeňte ju z **2000** na **1**. Potom zmeňte vnútorný (vnorený) **počet iterácií:** z **1** na **10**.
3. Kliknite na **Pridať typ úlohy**, zaškrtnite **Contaminant dispersion** a kliknite na **OK**. Automaticky sa zaškrtne aj **Incompressible viscous flow**, pretože rozptyl kontaminantu potrebuje prúdové pole. Ak sa na vonkajšej úrovni pridá druhá položka **Incompressible viscous flow**, vyberte ju a kliknite na **Odstrániť**.

Poradie krokov by malo vyzerať ako na snímke obrazovky nižšie. Potvrďte kliknutím na **OK**.

![CFD – Poradie krokov úlohy](image-CFD-Problem_task_flow.png)

## 10. Nastavenie riešiča v čase

Otvorte záložku **Úloha** na pravej strane hlavného okna a nastavte:

1. Zaškrtnite **Riešič v čase**.
2. **Aproximácia**: **Backward difference (stable)** (spätná diferencia – stabilná).
3. **Veľkosť časového kroku**: **0.01** `[s]`.
4. **Počet časových krokov**: **2000**.
5. **Frekvencia výstupu**: **10** – záznam s výsledkami sa zapíše každý 10. časový krok.

**Konečný čas** sa vypočíta automaticky (2000 × 0.01 s = 20 s).

V časti **Nastavenia rozptylu kontaminantu** nastavte **Maximálne nasýtenie** na **2.4** `[kg/m^3]` a **Koeficient difúzie** na **0.01** `[m^2/s]`. Viac informácií nájdete v teoretickej príručke.

![CFD – Riešič v čase](image-CFD-Problem_time_solver.png)

## 11. Okrajové podmienky (etapa 2)

Kontaminant vstupuje do kanála cez prítok. V strome **Model** vyberte **Inflow** a v záložke **Okrajové podmienky** zaškrtnite **Particle concentration** (koncentrácia častíc); **Volumetric flow rate (inflow)** ponechajte zaškrtnuté.

Namiesto konštantnej hodnoty sa použije hodnota závislá od času. Rozbaľte vlastnosť **Particle concentration** (šípka pred jej názvom) a kliknite na **Upraviť časovo závislé hodnoty**.

![CFD – Okrajové podmienky](image-CFD-Boundary_conditions.png)

V dialógu **Editor zložiek** nastavte **Počet hodnôt** na **3**, zadajte hodnoty uvedené nižšie (bunku upravíte dvojitým kliknutím) a kliknite na **Ok**.

| Čas `[s]` | Koncentrácia častíc `[kg/m^3]` |
| --------- | ------------------------------ |
| 0         | 0                              |
| 5         | 1                              |
| 10        | 0                              |

Medzi dvoma zadanými časmi sa hodnota lineárne interpoluje a po poslednom zadanom čase zostáva posledná hodnota. Pri týchto hodnotách koncentrácia na prítoku počas prvých 5 sekúnd stúpne z 0 na 1 `[kg/m^3]` a medzi 5. a 10. sekundou klesne späť na 0.

![CFD – Podmienka koncentrácie častíc](image-CFD-Particle_concentration_condition.png)

## 12. Vyriešiť prechodovú úlohu (reštart)

**Menu:** _Riešenie -> Spustiť riešič_

V dialógu **Spustiť riešič pre model** zaškrtnite **Reštartovať riešič / pokračovať** a kliknite na **OK**. Riešič potom ako východiskový bod použije už vypočítané ustálené prúdenie a pokračuje simuláciou s časovým krokovaním.

Upozornenie **Contaminant dispersion: No implicit boundary condition assigned.** (nie je priradená žiadna implicitná okrajová podmienka) je očakávané a možno ho ignorovať.

![CFD – Reštart riešiča](image-CFD-Solver_restart.png)

## 13. Zobraziť výsledky počas behu riešiča

Prechodový riešič beží na pozadí, takže výsledky možno prezerať ešte počas výpočtu. Ďalšie zobrazovacie celky, napríklad rezy, izoplochy, prúdnice, vektorové alebo skalárne polia, možno vytvoriť kedykoľvek a zobraziť na nich vypočítané výsledky. Tu sa opäť použijú rez a vektorové pole z krokov 7 a 8.

### 13.1 Koncentrácia kontaminantu na reze

1. V strome **Model** rozbaľte skupinu **Rezy**, zaškrtnite **Cut 1**, aby bol viditeľný, a vyberte ho.
2. Otvorte záložku **Výsledky** a v rozbaľovacom zozname hore vyberte **Particle concentration - Scalar**.
3. V časti **Použiť ako ...** zaškrtnite **Scalar**. Rez je teraz zafarbený podľa koncentrácie kontaminantu.
4. V časti **Rozsah hodnôt a farebná škála** nastavte **Maximum** na **1** a **Minimum** na **0** (napíšte hodnotu a stlačte _Enter_). Zodpovedá to rozsahu koncentrácie na prítoku, takže rovnaká farba znamená rovnakú koncentráciu v každom časovom kroku.
5. Zaškrtnite **Zobraziť rozsah hodnôt**, aby sa v **3D oblasti modelu** zobrazila farebná škála.

![Výsledky koncentrácie častíc](image-CFD-Particle_concentration_results.png)

### 13.2 Šípky vektorového poľa

Aby bol smer prúdenia nad zafarbeným rezom lepšie čitateľný, vykreslite šípky s hrotmi a bielou farbou:

1. V strome **Model** vyberte vektorové pole (**Velocity** v skupine **Vektorové polia**).
2. V časti **Možnosti zobrazenia** pod stromom zaškrtnite **Vykresliť hroty šípiek**.
3. Kliknite na výber farby pod zaškrtávacími políčkami a zvoľte bielu (**#ffffffff**).

Ďalšie možnosti zobrazenia určujú, či majú všetky šípky rovnakú dĺžku (**Vykresliť šípky rovnako dlhé**) a či šípky začínajú v bode, v ktorom sa rýchlosť vyhodnocuje (**Vykresliť šípky smerujúce von**).

![CFD – Vykresliť hroty šípiek](image-CFD-Draw_arrow_heads.png)

## 14. Záznamy modelu (výsledky v čase)

Každý 10. časový krok (podľa **Frekvencie výstupu**) zapíše riešič záznam modelu s riešením pre daný čas. Záznamy sú uvedené v záložke **Záznamy** na ľavej strane hlavného okna; stĺpec **Záznam** udáva číslo časového kroku (záznam **500** obsahuje riešenie v čase 500 × 0.01 s = 5 s). Dvojitým kliknutím na záznam sa načítajú výsledky pre jeho časový krok; výsledky použité v záložke **Výsledky** zostanú zachované a vykreslia sa pre načítaný záznam.

Výpočet všetkých 2000 časových krokov chvíľu trvá (na súčasnom počítači asi 20 minút). Riešič možno kedykoľvek zastaviť a doteraz zapísané záznamy zostanú zachované:

**Menu:** _Riešenie -> Zastaviť riešič_ (potvrďte kliknutím na **Yes**)

## 15. Nahrať video

Video sa vytvára z toho, čo je práve zobrazené v **3D oblasti modelu**, preto najprv nastavte pohľad a zobrazené výsledky (pozri krok 13). Nastavenia zobrazenia sa s modelom neukladajú, takže po jeho opätovnom otvorení ich treba nastaviť znova. Potom otvorte záložku **Záznamy**, poľami **Od** a **Do** v dolnej časti zvoľte rozsah záznamov a kliknite na červené tlačidlo **Záznam**.

![CFD – Záznamy modelu](image-CFD-Model_records.png)

Zobrazí sa dialóg **Nastavenia videa**, v ktorom možno nastaviť kodek videa, formát súboru, počet snímok za sekundu a počet snímok za záznam; pod nimi sa zobrazuje odhadovaná dĺžka videa. Pri 202 záznamoch, 24 snímkach za sekundu a 1 snímke za záznam má video približne 8,4 sekundy. Kliknutím na **OK** spustíte nahrávanie.

![CFD – Nastavenia videa](image-CFD-Video_settings.png)

Nahrávanie môže chvíľu trvať, pretože sa postupne načíta každý záznam modelu a ako snímka videa sa urobí snímka obrazovky **3D oblasti modelu**.

Video a všetky snímky (jeden obrázok PNG na záznam) nájdete v záložke **Dokumenty** v skupine **Channel**; video **Channel.mp4** je na konci zoznamu. Dvojitým kliknutím ho prehráte.

![CFD – Dokumenty](image-CFD-Documents.png)
