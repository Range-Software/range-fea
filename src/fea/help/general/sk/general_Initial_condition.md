# Počiatočná podmienka

**Počiatočná podmienka** je podmienka, ktorú možno zadať na akejkoľvek entite modelu (Bod, Čiara, Povrch, Objem), a možno ju chápať ako východiskový bod pre riešič. Pri **ustálených úlohách** dobre zvolená **počiatočná podmienka** zlepšuje konvergenciu riešiča. Pri **prechodových** (časovo závislých) úlohách určuje stav v počiatočnom čase.

Počiatočné podmienky sú špecifické pre **úlohu**, každý **typ úlohy** definuje vlastnú sadu **počiatočných podmienok** (napr. **Temperature** pre **Heat transfer**, **Velocity** a **Pressure** pre **Incompressible viscous flow** alebo **Displacement** pre **Stress analysis**).

## Priradenie počiatočnej podmienky

1. Vyberte jednu alebo viac entít v **strome modelu**.
2. Otvorte záložku **Počiatočné podmienky** na pravej strane hlavného okna.
3. Zaškrtnutím políčka pred názvom počiatočnej podmienky ju priradíte. Zrušením zaškrtnutia počiatočnú podmienku z vybraných entít odoberiete.
4. Vyberte počiatočnú podmienku a v zozname nižšie upravte hodnoty jej vlastností.

![Počiatočné podmienky](image-Initial_conditions.png)

Počiatočné podmienky sú nepovinné. Ak pre danú entitu modelu nie je zadaná žiadna **počiatočná podmienka**, použije sa **podmienka prostredia**. Na entitách s **explicitnou okrajovou podmienkou** sa použije hodnota predpísaná okrajovou podmienkou.
