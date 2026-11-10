# Riešič

**Riešič** je nástroj, ktorý spracuje nastavenia **úlohy** a vypočíta **výsledky** na zadanom **modeli**.

V závislosti od veľkosti **modelu**, **typu úlohy**, nastavení konvergencie a prechodového nastavenia sa čas riešenia **úlohy** môže pohybovať od sekúnd až po hodiny aj dlhšie.

## Spustenie riešiča

**Menu:** _Riešenie -> Spustiť riešič_

Pred spustením riešiča musí byť model uložený. Ak ešte uložený nebol, zobrazí sa najprv dialóg **Uložiť model**. Potom sa zobrazí dialóg **Spustiť riešič**.

![Spustiť riešič](image-Start_solver.png)

Pred spustením riešiča sa skontroluje nastavenie úlohy. Nájdené **Chyby** (napr. entita bez materiálu) a **Upozornenia** sú vypísané v dialógu **Spustiť riešič**. Ak model už obsahuje výsledky, možno zaškrtnúť **Reštartovať riešič / pokračovať** a pokračovať z týchto výsledkov namiesto výpočtu od začiatku.

Riešič beží ako samostatný proces na pozadí, takže počas výpočtu možno pokračovať v práci. Celý jeho výstup sa zobrazuje v záložke **Výstup procesu** pod 3D oblasťou. Po dokončení riešiča sa vypočítané **výsledky** automaticky načítajú do modelu.

## Zastavenie riešiča

- _Riešenie -> Zastaviť riešič_ – požiada všetky bežiace riešiče o zastavenie.
- _Riešenie -> Zabiť riešič_ – okamžite ukončí proces riešiča.

## Sledovanie riešiča

- _Výkaz -> Konvergencia riešiča_ – zobrazí graf konvergencie riešiča.
- _Výkaz -> Monitorovacie body_ – zobrazí hodnoty vypočítané v monitorovacích bodoch definovaných pomocou _Úloha -> Definovať monitorovacie body_.
- _Výkaz -> Log súbor riešiča_ – zobrazí log súbor riešiča.
