# Výsledky

**Výsledky** sú produkované **Riešičom** a skladajú sa z **Výsledkových premenných**.

## Výsledková premenná

**Výsledková premenná** je sada hodnôt vypočítaných **Riešičom**. **Výsledková premenná** je definovaná **typom** (napr. **Teplota** alebo **Posunutie**) a vektorom hodnôt. Na základe **typu** môže **Výsledková premenná** obsahovať vektor skalárnych hodnôt alebo vektor vektorových hodnôt.


Napríklad výsledková premenná **Teplota** bude obsahovať vektor teplôt pre každý uzol, zatiaľ čo **Posunutie** bude obsahovať vektor vektorov posunutia pre každý uzol.


**Výsledková premenná** môže byť vypočítaná buď na úrovni uzlov, alebo na úrovni prvkov. Napríklad **Posunutie** je vypočítané na úrovni uzlov, zatiaľ čo **Napätie** je vypočítané na úrovni prvkov. Každú premennú na úrovni prvkov možno po výpočte previesť na úroveň uzlov. Prevod premennej z úrovne prvkov na úroveň uzlov je jednorazová operácia a nie je možné ju vrátiť späť. Na opätovný výpočet je potrebné, aby výsledky prepočítal **Riešič**.
