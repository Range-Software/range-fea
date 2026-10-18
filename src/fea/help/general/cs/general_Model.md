# Model

**Model** je virtuální geometrická reprezentace trojrozměrného objektu. Model se skládá z geometrických **entit**.


**Entita** je soubor prvků (geometrických simplexů) stejného druhu a dělí se na tyto typy:

- **Bod** – soubor 0D prvků (uzel).
- **Čára** – soubor 1D prvků (úsečka).
- **Povrch** – soubor 2D prvků (trojúhelník, čtyřúhelník).
- **Objem** – soubor 3D prvků (čtyřstěn, šestistěn).

Bez ohledu na to, zda jsou prvky 0D, 1D, 2D nebo 3D, jsou všechny zobrazeny ve 3D prostoru. Například bod (uzel) nemá žádné rozměry, má pouze polohu, kdežto čára má jeden rozměr (délku).


Každé **entitě** kromě **objemu** lze přiřadit **geometrické** vlastnosti, které doplní její 3D reprezentaci. Jsou to:

- **Bod** – objem v metrech krychlových.
- **Čára** – plocha průřezu v metrech čtverečních.
- **Povrch** – tloušťka v metrech.
