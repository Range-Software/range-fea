# Okrajová podmienka

**Okrajová podmienka** je podmienka zadaná na hranici, pričom hranicou môže byť akákoľvek entita modelu (Bod, Čiara, Povrch, Objem).

Okrajové podmienky sú:
- **Špecifické pre úlohu** – Každý **typ úlohy** definuje vlastnú sadu **okrajových podmienok**.
- **Špecifické pre entitu** – Nie každú okrajovú podmienku možno použiť na každý typ entity (napr. **Pressure** možno použiť iba na **Povrch**).
- **Explicitné** – Na hranici je predpísané presné riešenie (napr. **Temperature** pre **Heat transfer**). V zozname **Okrajové podmienky** sú zobrazené tučným písmom.
- **Implicitné** – Riešenie na hranici z nich vyplýva (napr. **Heat** pre **Heat transfer**).

## Priradenie okrajovej podmienky

1. Vyberte jednu alebo viac entít v **strome modelu**.
2. Otvorte záložku **Okrajové podmienky** na pravej strane hlavného okna. Zoznam obsahuje iba okrajové podmienky, ktoré sú platné pre vybrané **typy úloh** a vybrané entity.
3. Zaškrtnutím políčka pred názvom okrajovej podmienky ju priradíte. Zrušením zaškrtnutia okrajovú podmienku z vybraných entít odoberiete.
4. Vyberte okrajovú podmienku a v zozname nižšie upravte hodnoty jej vlastností.

![Okrajové podmienky](image-Boundary_conditions.png)

Niektoré vlastnosti majú vlastné zaškrtávacie políčko. Napríklad pri každej zložke okrajovej podmienky **Displacement** možno zaškrtnutie zrušiť a ponechať tak posun v danom smere voľný.

Okrajové podmienky, ktoré pôsobia v smere danom geometriou (**Normal displacement**, **Roller displacement**), odvodzujú svoj lokálny smer z entity. Zaškrtnutím **Použiť zadaný lokálny smer** možno smer zadať ručne.

Ak chcete priradené okrajové podmienky zobraziť v 3D oblasti, zaškrtnite **Okrajové podmienky** v **Možnostiach zobrazenia**.

## Časovo závislé hodnoty

Hodnota každej vlastnosti môže byť časovo závislá. Rozbaľte vlastnosť a kliknutím na **Upraviť časovo závislé hodnoty** zadajte tabuľku hodnôt pre dané časy. Časovo závislé hodnoty sa použijú, ak je úloha **prechodová** (časovo závislá), teda ak je v záložke **Úloha** zapnutý **Riešič v čase**.
