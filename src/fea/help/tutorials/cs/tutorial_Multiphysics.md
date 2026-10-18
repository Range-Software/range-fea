# Multifyzika

Tento návod ukáže, jak na jednom modelu vyřešit několik provázaných úloh. Znovu se použije model **Beam** (nosník) z návodu **Analýza napětí a deformace**. Tentokrát nosníkem protéká elektrický proud, který ho zahřívá (Jouleovo teplo), a teplo způsobí, že se nosník roztáhne a zdeformuje.

Potřebné jsou tři typy úloh, které se řeší v tomto pořadí:

1. **Electro-statics** (elektrostatika) – počítá elektrický potenciál, elektrické pole, proudovou hustotu a Jouleovo teplo vytvořené proudem.
2. **Heat transfer** (přenos tepla) – počítá rozložení teploty způsobené Jouleovým teplem.
3. **Stress analysis** (analýza napětí) – počítá tepelnou roztažnost (posunutí) a výsledné napětí.

Každá úloha využívá výsledky předchozí, proto na pořadí úloh záleží.

## 1. Načíst model

**Nabídka:** _Soubor -> Otevřít model_

V dialogu **Otevřít model** vyberte **Beam.tmsh** a klikněte na **Otevřít**.

Protože modelu zatím není přiřazen žádný typ úlohy, po dokončení načítání se automaticky otevře dialog **Pořadí kroků úlohy**.

## 2. Pořadí kroků úlohy

1. V dialogu **Pořadí kroků úlohy** klikněte na **Přidat typ úlohy**.
2. V dialogu **Výběr typu úlohy** zaškrtněte **Electro-statics**, **Heat transfer** a **Stress analysis** a klikněte na **OK**.

![Výběr typu úlohy – multifyzika](image-Problem_type_selector-Multi.png)

Dialog **Pořadí kroků úlohy** by nyní měl obsahovat tři úlohy v jedné skupině iterací v pořadí, které je zobrazeno níže. Pokud je pořadí jiné nebo jsou v seznamu položky navíc, vyberte položku a opravte to tlačítky **Nahoru**, **Dolů** a **Odstranit**. Potvrďte kliknutím na **OK**.

![Pořadí kroků úlohy – multifyzika](image-Problem_task_flow-Multi.png)

Dialog lze kdykoli znovu otevřít tlačítkem **Pořadí kroků úloh** na panelu nástrojů.

## 3. Vygenerovat 3D síť

Všechny tři úlohy se řeší na objemové síti.

**Nabídka:** _Geometrie -> Objem -> Generovat čtyřstěnnou síť_

Předvyplněné hodnoty v dialogu **Generovat 3D síť** postačují. Klikněte na **OK**. Po dokončení generování se ve stromu **Model** ve skupině **Objemy** objeví nová entita **Volume 1**.

## 4. Přiřadit materiál

Všechny entity modelu musí mít přiřazený materiál. Materiál musí obsahovat vlastnosti potřebné pro všechny tři úlohy; **Iron** (železo) je obsahuje.

1. Ve stromu **Model** klikněte na **Surface** a poté se stisknutou klávesou _Shift_ klikněte na **Volume 1**, čímž vyberete všechny entity.
2. Otevřete záložku **Materiál** na pravé straně hlavního okna.
3. Zaškrtněte **Iron**.

## 5. Přiřadit okrajové podmínky

Protože je vybráno více typů úloh, záložka **Okrajové podmínky** nyní nabízí podmínky všech z nich a jedna entita může mít přiřazeno více podmínek – jednu pro každou úlohu. **Povrchovým entitám** přiřaďte tyto podmínky:

1. **Surface** (vnější povrch nosníku)
    - _Simple convection_ (jednoduchá konvekce)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293.15 `[K]`
2. **Load** (plocha uprostřed nosníku)
    - _Electric potential_ (elektrický potenciál)
        - Electric potential = 0.4 `[V]`
    - _Simple convection_ (hodnoty jako u **Surface**)
3. **Left** a **Right** (plochy na obou koncích nosníku)
    - _Displacement_ (posunutí)
        - posunutí ve směru X, Y a Z = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0.4 `[V]`
    - _Simple convection_ (hodnoty jako u **Surface**)

Proud tedy teče ze středu nosníku k oběma koncům a všechny plochy odevzdávají teplo okolnímu vzduchu konvekcí.

Pro každou entitu: vyberte ji ve stromu **Model** (k výběru **Left** i **Right** podržte _Ctrl_ / _⌘ Cmd_), v záložce **Okrajové podmínky** zaškrtněte podmínku, vyberte ji, aby se pod seznamem zobrazily její vlastnosti, poté dvakrát klikněte na hodnotu, napište novou a stiskněte _Enter_. Výchozí hodnoty podmínek **Simple convection** (100 `[W/(m^2*K)]` a 293.15 `[K]`) a **Displacement** (0 `[m]`) již odpovídají zadání, takže měnit je potřeba pouze **Electric potential**.

## 6. Zkontrolovat environmentální podmínky

Environmentální podmínky platí pro celý model a nezaškrtávají se jako okrajové podmínky. Otevřete záložku **Environmentální podmínky** a vyberte jednotlivé podmínky, abyste zkontrolovali jejich hodnoty:

- _Gravitational acceleration_ (gravitační zrychlení)
    - 0, 0 a -9.80665 `[m/s^2]` ve směru X, Y a Z
- _Temperature_ (teplota)
    - Temperature = 293.15 `[K]`

## 7. Vyřešit úlohu

**Nabídka:** _Řešení -> Spustit řešič_

Před spuštěním řešiče musí být model uložen. Pokud ještě nebyl uložen, nejprve se zobrazí dialog **Uložit model**; potvrďte ho kliknutím na **Uložit** (pokud soubor se stejným názvem již existuje, potvrďte jeho nahrazení). Poté se zobrazí dialog **Spustit řešič pro model**.

Dialog uvádí několik upozornění, například **Heat transfer: No explicit boundary condition assigned.** Při tomto nastavení jsou očekávaná a lze je ignorovat. Kliknutím na **OK** spustíte řešič.

Řešič běží na pozadí a jeho výstup se zobrazuje v záložce **Výstup procesu**. Úlohy se řeší jedna po druhé v pořadí kroků úlohy; výpočet je dokončen, když výstup skončí řádkem **SOLVER - END**.

## 8. Vytvořit vektorové pole

**Vektorové pole** je zobrazovací entita, která vykresluje vektorovou proměnnou jako šipky. Zde se použije k zobrazení elektrického pole uvnitř nosníku.

**Nabídka:** _Geometrie -> Vektorové pole -> Vytvořit vektorové pole_

V dialogu **Editor vektorového pole** vyberte v seznamu **Model** položku **Volume 1** a v seznamu **Proměnné** položku **Electric field** (elektrické pole); **3D vektorové pole** ponechte zaškrtnuté.

![Vytvořit vektorové pole](image-Create_vector_field.png)

Kliknutím na **OK** vektorové pole vytvoříte. Přidá se do stromu **Model** do skupiny **Vektorová pole** a vypočtené výsledky na něj lze použít stejně jako na kteroukoli jinou entitu modelu.

## 9. Použít výsledky

Výsledky se používají v záložce **Výsledky** na entitu vybranou ve stromu **Model**, stejně jako v návodu **Analýza napětí a deformace**.

### 9.1. Teplota a deformace objemu

1. Ve stromu **Model** zaškrtněte **Objemy** (povrchy nechte nezaškrtnuté) a vyberte **Volume 1**.
2. V záložce **Výsledky** vyberte v rozbalovacím seznamu **Temperature - Scalar**, zaškrtněte **Scalar** a **Zobrazit rozsah hodnot**. Pokud zobrazený rozsah neodpovídá výsledkům, klikněte na **Nastavit výchozí min/max hodnoty**.
3. V rozbalovacím seznamu vyberte **Displacement - Vector [3]**, zaškrtněte **Displacement** a **Měřítko** nastavte na **20**, aby byla deformace viditelná (vypočtená posunutí jsou jen asi 1,3 mm).

Teplota se pohybuje přibližně od 505 K do 620 K; nosník je nejteplejší uprostřed svých dvou krajních částí a prohne se nahoru, protože upevněné konce mu nedovolí volně se roztáhnout.

![Teplota na objemu](image-Applied_temperature_on_volume.png)

### 9.2. Teplota na vektorovém poli

1. Ve stromu **Model** zrušte zaškrtnutí **Objemy**, zaškrtněte **Vektorová pole** a vyberte **Electric field**.
2. V záložce **Výsledky** vyberte **Temperature - Scalar**, zaškrtněte **Scalar** a **Zobrazit rozsah hodnot**. Šipky jsou nyní obarveny podle teploty v místě, kde se nacházejí.
3. V části **Možnosti zobrazení** pod stromem zaškrtněte **Vykreslit hroty šipek**, aby byl vidět směr elektrického pole.

![Teplota na vektorovém poli](image-Applied_temperature_on_vector_field.png)
