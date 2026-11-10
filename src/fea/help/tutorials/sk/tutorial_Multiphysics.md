# Multifyzika

Tento tutoriál ukáže, ako na jednom modeli vyriešiť niekoľko previazaných úloh. Znova sa použije model **Beam** (nosník) z tutoriálu **Analýza napätia a deformácie**. Tentoraz nosníkom preteká elektrický prúd, ktorý ho zahrieva (Joulovo teplo), a teplo spôsobí, že sa nosník roztiahne a zdeformuje.

Potrebné sú tri typy úloh, ktoré sa riešia v tomto poradí:

1. **Electro-statics** (elektrostatika) – počíta elektrický potenciál, elektrické pole, prúdovú hustotu a Joulovo teplo vytvorené prúdom.
2. **Heat transfer** (prestup tepla) – počíta rozloženie teploty spôsobené Joulovým teplom.
3. **Stress analysis** (analýza napätia) – počíta tepelnú rozťažnosť (posunutia) a výsledné napätie.

Každá úloha využíva výsledky predchádzajúcej, preto na poradí úloh záleží.

## 1. Načítať model

**Menu:** _Súbor -> Otvoriť model_

V dialógu **Otvoriť model** vyberte **Beam.rbm** a kliknite na **Open** (Otvoriť).

Keďže modelu zatiaľ nie je priradený žiadny typ úlohy, po dokončení načítania sa automaticky otvorí dialóg **Vybrať úlohu**.

## 2. Vybrať úlohy

V dialógu **Vybrať úlohu** kliknite na dlaždice **Electro-statics**, **Heat transfer** a **Stress analysis**. Pod dlaždicami dialóg zobrazí **Poradie riešenia**: Electro-statics → Heat transfer → Stress analysis. **Modal analysis** je označená ako **Nekompatibilná**, pretože ju nemožno kombinovať s inými typmi úloh. Potvrďte kliknutím na **OK**.

![Vybrať úlohu – multifyzika](image-Select_problem-Multi.png)

Dialóg možno kedykoľvek znova otvoriť tlačidlom **Vybrať úlohu** na paneli nástrojov.

## 3. Vygenerovať 3D sieť

Všetky tri úlohy sa riešia na objemovej sieti.

**Menu:** _Geometria -> Objem -> Generovať tetrahedrónovú sieť_

Predvyplnené hodnoty v dialógu **Generovať 3D sieť** postačia. Kliknite na **OK**. Po dokončení generovania sa v **strome modelu** pod **Objemy** objaví nový celok **Volume 1**.

## 4. Priradiť materiál

Všetky celky modelu musia mať priradený materiál. Materiál musí obsahovať vlastnosti potrebné pre všetky tri úlohy; **Iron** (železo) ich obsahuje.

1. V **strome modelu** rozbaľte **Objemy**, kliknite na **Surface** a potom so stlačeným klávesom _Shift_ kliknite na **Volume 1**, čím vyberiete všetky celky.
2. Otvorte záložku **Materiál** na pravej strane hlavného okna.
3. Zaškrtnite **Iron**.

## 5. Priradiť okrajové podmienky

Keďže je vybraných viac typov úloh, záložka **Okrajové podmienky** ponúka podmienky všetkých týchto úloh a jeden celok môže mať priradených viac podmienok – jednu pre každú úlohu. **Povrchovým** celkom priraďte tieto podmienky:

1. **Surface** (vonkajší povrch nosníka)
    - _Simple convection_ (jednoduchá konvekcia)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293,15 `[K]`
2. **Load** (plocha v strede nosníka)
    - _Electric potential_ (elektrický potenciál)
        - Electric potential = 0,04 `[V]`
    - _Simple convection_ (hodnoty ako pri **Surface**)
3. **Left** a **Right** (plochy na oboch koncoch nosníka)
    - _Displacement_ (posunutie)
        - Displacement in X, Y and Z direction = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0,04 `[V]`
    - _Simple convection_ (hodnoty ako pri **Surface**)

Prúd teda tečie zo stredu nosníka k obom koncom a všetky plochy odovzdávajú teplo do okolitého vzduchu konvekciou.

Pre každý celok: vyberte ho v **strome modelu** (na výber **Left** aj **Right** podržte _Ctrl_ / _⌘ Cmd_), v záložke **Okrajové podmienky** zaškrtnite podmienku, vyberte ju, aby sa pod zoznamom zobrazili jej vlastnosti, potom dvakrát kliknite na hodnotu, zadajte novú a stlačte _Enter_. Predvolené hodnoty **Simple convection** (100 `[W/(m^2*K)]` a 293,15 `[K]`) a **Displacement** (0 `[m]`) už zodpovedajú, takže treba zmeniť iba **Electric potential**.

## 6. Skontrolovať podmienky prostredia

Podmienky prostredia platia pre celý model a nezaškrtávajú sa ako okrajové podmienky. Otvorte záložku **Environmentálne podmienky** a vyberte každú podmienku, aby ste overili jej hodnoty:

- _Gravitational acceleration_ (gravitačné zrýchlenie)
    - 0, 0 a -9,80665 `[m/s^2]` v smere X, Y a Z
- _Temperature_ (teplota)
    - Temperature = 293,15 `[K]`

## 7. Vyriešiť úlohu

**Menu:** _Riešenie -> Spustiť riešič_

Pred spustením riešiča musí byť model uložený. Model **Beam** bol otvorený zo súboru, takže sa doň uloží automaticky; model, ktorý ešte nikdy nebol uložený, najprv zobrazí dialóg **Uložiť model**. Potom sa zobrazí dialóg **Spustiť riešič**.

Dialóg vypíše tri upozornenia: **Electro-statics: No implicit boundary condition assigned.**, **Heat transfer: No explicit boundary condition assigned.** a **Stress analysis: No implicit boundary condition assigned.** Pre toto nastavenie sú očakávané a možno ich ignorovať. Kliknutím na **OK** spustíte riešič.

Riešič beží na pozadí a jeho výstup sa zobrazuje v záložke **Výstup procesu**. Úlohy sa riešia jedna po druhej v poradí krokov úlohy; výpočet je dokončený, keď výstup končí riadkom **SOLVER - END**.

## 8. Vytvoriť vektorové pole

**Vektorové pole** je zobrazovací celok, ktorý vykresľuje vektorovú premennú ako šípky. Tu sa použije na zobrazenie elektrického poľa vo vnútri nosníka.

**Menu:** _Geometria -> Vektorové pole -> Vytvoriť vektorové pole_

V dialógu **Editor vektorového poľa** vyberte v zozname **Model** položku **Volume 1** a v zozname **Premenné** položku **Electric field** a ponechajte zaškrtnuté **3D vektorové pole**. Zoznam **Premenné** obsahuje vektorové premenné vypočítané pre vybraný celok (**Electric field**, **Current density**, **Heat flux**, **Displacement** a **Force**).

![Vytvoriť vektorové pole](image-Create_vector_field.png)

Kliknutím na **OK** vektorové pole vytvoríte. Pridá sa do **stromu modelu** pod **Vektorové polia** s názvom **Electric field** a vypočítané výsledky možno naň použiť ako na akýkoľvek iný celok modelu.

## 9. Zobraziť výsledky

Výsledky sa zobrazujú v záložke **Výsledky** na celku vybranom v **strome modelu**, rovnako ako v tutoriáli **Analýza napätia a deformácie**.

### 9.1. Teplota a deformácia objemu

1. V **strome modelu** zaškrtnite **Objemy** (povrchy ponechajte nezaškrtnuté) a potom vyberte **Volume 1**.
2. V záložke **Výsledky** vyberte v rozbaľovacej ponuke **Temperature - Scalar**, zaškrtnite **Farebná mapa** a **Zobraziť legendu**. Ak zobrazený rozsah nezodpovedá výsledkom, kliknite na **Obnoviť podľa dát**.
3. V rozbaľovacej ponuke vyberte **Displacement - Vector [3]**, zaškrtnite **Deformovaný tvar** a nastavte **Mierka deformácie a šípok** na **20**, aby bola deformácia viditeľná (vypočítané posunutia sú iba asi 1,2 mm).
4. Znova vyberte **Temperature - Scalar**, aby legenda zobrazovala teplotu.

Teplota sa pohybuje približne od 468 K do 558 K; nosník je najteplejší v strede oboch vonkajších častí a prehýba sa nahor, pretože upevnené konce mu nedovolia voľne sa roztiahnuť.

![Teplota na objeme](image-Applied_temperature_on_volume.png)

### 9.2. Teplota na vektorovom poli

1. V **strome modelu** zrušte zaškrtnutie **Objemy**, zaškrtnite **Vektorové polia** a vyberte **Electric field**.
2. V záložke **Výsledky** vyberte **Temperature - Scalar**, zaškrtnite **Farebná mapa** a **Zobraziť legendu** (ak ešte nie sú zaškrtnuté). Šípky sú teraz zafarbené podľa teploty v mieste, kde sa nachádzajú.
3. V **Možnostiach zobrazenia** pod stromom zaškrtnite **Hroty šípiek**, aby bol vidieť smer elektrického poľa. Elektrické pole smeruje zo stredu nosníka (vyšší potenciál) k obom koncom.

![Teplota na vektorovom poli](image-Applied_temperature_on_vector_field.png)
