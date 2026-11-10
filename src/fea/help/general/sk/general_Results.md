# Výsledky

**Výsledky** vytvára **riešič** a skladajú sa z **výsledkových premenných**.

## Výsledková premenná

**Výsledková premenná** je sada hodnôt vypočítaných **riešičom**. **Výsledková premenná** je definovaná svojím **typom** (napr. **Temperature** alebo **Displacement**) a vektorom hodnôt. Podľa **typu** môže **výsledková premenná** obsahovať vektor skalárnych alebo vektorových hodnôt.

Napríklad výsledková premenná **Temperature** obsahuje vektor teplôt pre každý uzol, kým **Displacement** obsahuje vektor vektorov posunu pre každý uzol.

Všetky výsledkové premenné vybraného modelu sú uvedené v rozbaľovacej ponuke v hornej časti záložky **Výsledky** spolu so svojím druhom (**Scalar** alebo **Vector**).

![Výsledkové premenné](image-Results_variables.png)

**Výsledková premenná** môže byť vypočítaná buď na úrovni uzlov, alebo na úrovni prvkov. Napríklad **Displacement** sa počíta na úrovni uzlov, kým **Von Mises stress** na úrovni prvkov. Kde je vybraná premenná uložená (**Na uzloch** alebo **Na prvkoch**), je uvedené pod rozbaľovacou ponukou.

Premenné na úrovni prvkov sa zobrazujú s jednou hodnotou na prvok. Pre spojité zobrazenie:

- Premenné napätia ponúkajú zaškrtávacie políčko **Vyhladené zobrazenie**. To priemeruje výsledky prvkov v uzloch iba na účely zobrazenia, samotné výsledky zostávajú nezmenené. Priemerovanie znižuje špičkové hodnoty.
- Ostatné premenné na úrovni prvkov ponúkajú tlačidlo **Zobraziť na uzloch**, ktoré premennú prevedie z prvkov na uzly.

## Zobrazenie výsledkov

Výsledky sa zobrazujú na entitách, ktoré sú vybrané v **strome modelu**. Výsledky úlohy riešenej na **Objemoch** sú najlepšie viditeľné, keď sú povrchové entity skryté.

![Výsledky](image-Results.png)

- **Zobraziť na vybraných entitách** – zaškrtnutím **Farebná mapa** sa vybrané entity zafarbia podľa hodnoty premennej. Vektorové premenné možno zobraziť aj ako **Deformovaný tvar**.
- **Farebný rozsah** – hodnoty **Max** a **Min** farebnej mapy. Možno ich zadať ručne alebo zmeniť v **Jemnom nastavení**. **Obnoviť podľa dát** nastaví rozsah späť na rozsah vypočítaný z výsledkov.
- **Farby** – farebná schéma farebnej mapy.
- **Zobraziť legendu** – zobrazí rozsah hodnôt v 3D oblasti.
- **Mierka deformácie a šípok** – násobí deformáciu deformovaných tvarov a dĺžku šípok vektorových polí (iba vektorové premenné).
- **Graf…** – zobrazí rozloženie hodnôt a umožní nastaviť farebný rozsah na grafe.
- **Odstrániť** – odstráni premennú z modelu.

Výsledky možno zobraziť aj na entitách vytvorených na tento účel: **Rezy**, **ISO**, **Prúdnicové čiary**, **Skalárové polia** a **Vektorové polia**.
