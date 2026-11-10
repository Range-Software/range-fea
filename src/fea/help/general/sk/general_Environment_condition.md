# Podmienka prostredia

**Podmienka prostredia** (environmentálna podmienka) je podmienka, ktorú možno zadať na akejkoľvek entite modelu, ako je Bod, Čiara, Povrch alebo Objem. **Podmienka prostredia** určuje hodnoty prostredia špecifické pre **úlohu** (napr. **Temperature** – teplota prostredia alebo **Gravitational acceleration** – gravitačné zrýchlenie).

Dostupné podmienky prostredia závisia od vybraných **typov úloh**:

- **Gravitational acceleration** – zložky v smere X, Y a Z, predvolené hodnoty 0, 0 a -9.80665 `[m/s^2]`.
- **Temperature** – predvolená hodnota 293.15 `[K]`.

## Úprava podmienky prostredia

Na rozdiel od **okrajových podmienok** sa podmienky prostredia nezaškrtávajú. Sú vždy prítomné a majú predvolené hodnoty.

1. Vyberte jednu alebo viac entít v **strome modelu**.
2. Otvorte záložku **Environmentálne podmienky** na pravej strane hlavného okna.
3. Vyberte podmienku prostredia a v zozname nižšie upravte hodnoty jej vlastností.

![Environmentálne podmienky](image-Environment_conditions.png)

Ak pre danú entitu modelu nie je zadaná žiadna **počiatočná podmienka**, použije sa **podmienka prostredia**.
