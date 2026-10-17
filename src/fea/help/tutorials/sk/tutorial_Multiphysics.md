# Multifyzika

Tento návod ukáže, ako na jednom modeli vyriešiť niekoľko previazaných úloh. Opäť sa použije model **Beam** (nosník) z návodu **Analýza napätia a deformácie**. Tentokrát cez nosník preteká elektrický prúd, ktorý ho zohrieva (Joulovo teplo), a teplo spôsobí, že sa nosník roztiahne a zdeformuje.

Potrebné sú tri typy úloh, ktoré sa riešia v tomto poradí:

1. **Electro-statics** (elektrostatika) – počíta elektrický potenciál, elektrické pole, prúdovú hustotu a Joulovo teplo vytvorené prúdom.
2. **Heat transfer** (prestup tepla) – počíta rozloženie teploty spôsobené Joulovým teplom.
3. **Stress analysis** (analýza napätia) – počíta tepelnú rozťažnosť (posunutia) a výsledné napätie.

Každá úloha využíva výsledky predchádzajúcej, preto na poradí úloh záleží.

## 1. Načítať model

**Menu:** _Súbor -> Otvoriť model_

V dialógu **Otvoriť model** vyberte **Beam.tmsh** a kliknite na **Otvoriť**.

Keďže modelu zatiaľ nie je priradený žiadny typ úlohy, po dokončení načítania sa automaticky otvorí dialóg **Poradie krokov úlohy**.

## 2. Poradie krokov úlohy

1. V dialógu **Poradie krokov úlohy** kliknite na **Pridať typ úlohy**.
2. V dialógu **Výber typu úlohy** zaškrtnite **Electro-statics**, **Heat transfer** a **Stress analysis** a kliknite na **OK**.

![Výber typu úlohy – multifyzika](image-Problem_type_selector-Multi.png)

Dialóg **Poradie krokov úlohy** by teraz mal obsahovať tri úlohy v jednej skupine iterácií v poradí, ktoré je zobrazené nižšie. Ak je poradie iné alebo sú v zozname položky navyše, vyberte položku a opravte to tlačidlami **Hore**, **Dole** a **Odstrániť**. Potvrďte kliknutím na **OK**.

![Poradie krokov úlohy – multifyzika](image-Problem_task_flow-Multi.png)

Dialóg možno kedykoľvek znova otvoriť tlačidlom **Poradie krokov úloh** na paneli nástrojov.

## 3. Vygenerovať 3D sieť

Všetky tri úlohy sa riešia na objemovej sieti.

**Menu:** _Geometria -> Objem -> Generovať tetrahedrónovú sieť_

Predvyplnené hodnoty v dialógu **Vygenerovať 3D sieť** sú postačujúce. Kliknite na **OK**. Po dokončení generovania sa v strome **Model** v skupine **Objemy** objaví nový celok **Volume 1**.

## 4. Priradiť materiál

Všetky celky modelu musia mať priradený materiál. Materiál musí obsahovať vlastnosti potrebné pre všetky tri úlohy; **Iron** (železo) ich obsahuje.

1. V strome **Model** kliknite na **Surface** a potom so stlačeným klávesom _Shift_ kliknite na **Volume 1**, čím vyberiete všetky celky.
2. Otvorte záložku **Materiál** na pravej strane hlavného okna.
3. Zaškrtnite **Iron**.

## 5. Priradiť okrajové podmienky

Keďže je vybraných viac typov úloh, záložka **Okrajové podmienky** teraz ponúka podmienky všetkých z nich a jeden celok môže mať priradených viac podmienok – jednu pre každú úlohu. **Celkom povrchu** priraďte tieto podmienky:

1. **Surface** (vonkajší povrch nosníka)
    - _Simple convection_ (jednoduchá konvekcia)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293.15 `[K]`
2. **Load** (plocha v strede nosníka)
    - _Electric potential_ (elektrický potenciál)
        - Electric potential = 0.4 `[V]`
    - _Simple convection_ (hodnoty ako pri **Surface**)
3. **Left** a **Right** (plochy na oboch koncoch nosníka)
    - _Displacement_ (posunutie)
        - posunutie v smere X, Y a Z = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0.4 `[V]`
    - _Simple convection_ (hodnoty ako pri **Surface**)

Prúd teda tečie zo stredu nosníka k obom koncom a všetky plochy odovzdávajú teplo okolitému vzduchu konvekciou.

Pre každý celok: vyberte ho v strome **Model** (na výber **Left** aj **Right** podržte _Ctrl_ / _⌘ Cmd_), v záložke **Okrajové podmienky** zaškrtnite podmienku, vyberte ju, aby sa pod zoznamom zobrazili jej vlastnosti, potom dvakrát kliknite na hodnotu, napíšte novú a stlačte _Enter_. Predvolené hodnoty podmienok **Simple convection** (100 `[W/(m^2*K)]` a 293.15 `[K]`) a **Displacement** (0 `[m]`) už zodpovedajú zadaniu, takže meniť treba iba **Electric potential**.

## 6. Skontrolovať environmentálne podmienky

Environmentálne podmienky platia pre celý model a nezaškrtávajú sa ako okrajové podmienky. Otvorte záložku **Environmentálne podmienky** a vyberte jednotlivé podmienky, aby ste skontrolovali ich hodnoty:

- _Gravitational acceleration_ (gravitačné zrýchlenie)
    - 0, 0 a -9.80665 `[m/s^2]` v smere X, Y a Z
- _Temperature_ (teplota)
    - Temperature = 293.15 `[K]`

## 7. Vyriešiť úlohu

**Menu:** _Riešenie -> Spustiť riešič_

Pred spustením riešiča musí byť model uložený. Ak ešte nebol uložený, najprv sa zobrazí dialóg **Uložiť model**; potvrďte ho kliknutím na **Uložiť** (ak súbor s rovnakým názvom už existuje, potvrďte jeho nahradenie). Potom sa zobrazí dialóg **Spustiť riešič pre model**.

Dialóg uvádza niekoľko upozornení, napríklad **Heat transfer: No explicit boundary condition assigned.** Pri tomto nastavení sú očakávané a možno ich ignorovať. Kliknutím na **OK** spustíte riešič.

Riešič beží na pozadí a jeho výstup sa zobrazuje v záložke **Výstup procesu**. Úlohy sa riešia jedna po druhej v poradí krokov úlohy; výpočet je dokončený, keď výstup skončí riadkom **SOLVER - END**.

## 8. Vytvoriť vektorové pole

**Vektorové pole** je zobrazovací celok, ktorý vykresľuje vektorovú premennú ako šípky. Tu sa použije na zobrazenie elektrického poľa vo vnútri nosníka.

**Menu:** _Geometria -> Vektorové pole -> Vytvoriť vektorové pole_

V dialógu **Editor vektorového poľa** vyberte v zozname **Model** položku **Volume 1** a v zozname **Premenné** položku **Electric field** (elektrické pole); **3D vektorové pole** ponechajte zaškrtnuté.

![Vytvoriť vektorové pole](image-Create_vector_field.png)

Kliknutím na **OK** vektorové pole vytvoríte. Pridá sa do stromu **Model** do skupiny **Vektorové polia** a vypočítané výsledky naň možno použiť rovnako ako na ktorýkoľvek iný celok modelu.

## 9. Použiť výsledky

Výsledky sa používajú v záložke **Výsledky** na celok vybraný v strome **Model**, rovnako ako v návode **Analýza napätia a deformácie**.

### 9.1. Teplota a deformácia objemu

1. V strome **Model** zaškrtnite **Objemy** (povrchy nechajte nezaškrtnuté) a vyberte **Volume 1**.
2. V záložke **Výsledky** vyberte v rozbaľovacom zozname **Temperature - Scalar**, zaškrtnite **Scalar** a **Zobraziť rozsah hodnôt**. Ak zobrazený rozsah nezodpovedá výsledkom, kliknite na **Nastaviť východzie min-max hodnoty**.
3. V rozbaľovacom zozname vyberte **Displacement - Vector [3]**, zaškrtnite **Displacement** a **Mierka** nastavte na **20**, aby bola deformácia viditeľná (vypočítané posunutia sú iba asi 1,3 mm).

Teplota sa pohybuje približne od 505 K do 620 K; nosník je najteplejší v strede svojich dvoch krajných častí a prehne sa nahor, pretože upevnené konce mu nedovolia voľne sa roztiahnuť.

![Teplota na objeme](image-Applied_temperature_on_volume.png)

### 9.2. Teplota na vektorovom poli

1. V strome **Model** zrušte zaškrtnutie **Objemy**, zaškrtnite **Vektorové polia** a vyberte **Electric field**.
2. V záložke **Výsledky** vyberte **Temperature - Scalar**, zaškrtnite **Scalar** a **Zobraziť rozsah hodnôt**. Šípky sú teraz zafarbené podľa teploty v mieste, kde sa nachádzajú.
3. V časti **Možnosti zobrazenia** pod stromom zaškrtnite **Vykresliť hroty šípiek**, aby bol viditeľný smer elektrického poľa.

![Teplota na vektorovom poli](image-Applied_temperature_on_vector_field.png)
