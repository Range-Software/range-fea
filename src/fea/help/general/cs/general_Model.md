# Model

**Model** je virtuální geometrická reprezentace trojrozměrného objektu. Model se skládá z geometrických **entit**.

![Model](image-Model.png)

## Entity

**Entita** je kolekce prvků (geometrických simplexů) stejného typu a dělí se na tyto typy:

- **Bod** – kolekce 0D prvků (uzel).
- **Čára** – kolekce 1D prvků (úsečka).
- **Povrch** – kolekce 2D prvků (trojúhelník, čtyřúhelník).
- **Objem** – kolekce 3D prvků (čtyřstěn).

Bez ohledu na to, zda jsou prvky 0D, 1D, 2D nebo 3D, všechny jsou reprezentovány ve 3D prostoru. Například bod (uzel) nemá žádné rozměry, má pouze polohu, zatímco čára má jeden rozměr (délku).

Kromě těchto geometrických entit může model obsahovat entity vytvořené z vypočtených **výsledků**: **Řezy**, **ISO**, **Proudnice**, **Skalární pole** a **Vektorová pole**.

## Strom modelu

Všechny entity modelu jsou uvedeny ve **stromu modelu** (záložka **Model** na levé straně hlavního okna) a jsou seskupeny podle typu. Sloupec **#** zobrazuje počet entit ve skupině nebo počet prvků v entitě.

![Strom modelu](image-Model_tree.png)

- Zaškrtávací políčko před názvem zobrazí nebo skryje model, skupinu nebo entitu ve 3D oblasti.
- Kliknutím se řádek vybere. Chcete-li vybrat více entit, držte při výběru klávesu _Ctrl_ (na macOS _⌘ Cmd_), nebo klávesu _Shift_ pro výběr rozsahu. Výběrem modelu nebo skupiny se vyberou všechny entity, které obsahuje.
- **Okrajové podmínky**, **Počáteční podmínky**, **Environmentální podmínky**, **Materiál** a **Výsledky** se vždy přiřazují entitám, které jsou vybrány ve **stromu modelu**.

**Možnosti zobrazení** pod stromem určují, jak se vybrané entity vykreslují (např. **Jako drátový model**, **Hrany prvků**, **Uzly prvků**, **Okrajové podmínky**), a jejich barvu.

## Geometrické vlastnosti

Každé **entitě** kromě **Objemu** lze přiřadit **geometrickou** vlastnost, která doplňuje její 3D reprezentaci. Pro vybrané entity je zobrazena v seznamu **Geometrické vlastnosti** pod **stromem modelu**:

- **Objem bodu** – objem **Bodu** v metrech krychlových.
- **Plocha průřezu čáry** – plocha průřezu **Čáry** v metrech čtverečních.
- **Tloušťka povrchu** – tloušťka **Povrchu** v metrech.
