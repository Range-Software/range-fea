# Počáteční podmínka

**Počáteční podmínka** je podmínka, kterou lze zadat na jakékoli entitě modelu (Bod, Čára, Povrch, Objem), a lze ji chápat jako výchozí bod pro řešič. U **ustálených úloh** dobře zvolená **počáteční podmínka** zlepšuje konvergenci řešiče. U **přechodových** (časově závislých) úloh určuje stav v počátečním čase.

Počáteční podmínky jsou specifické pro **úlohu**, každý **typ úlohy** definuje vlastní sadu **počátečních podmínek** (např. **Temperature** pro **Heat transfer**, **Velocity** a **Pressure** pro **Incompressible viscous flow** nebo **Displacement** pro **Stress analysis**).

## Přiřazení počáteční podmínky

1. Vyberte jednu nebo více entit ve **stromu modelu**.
2. Otevřete záložku **Počáteční podmínky** na pravé straně hlavního okna.
3. Zaškrtnutím políčka před názvem počáteční podmínky ji přiřadíte. Zrušením zaškrtnutí počáteční podmínku z vybraných entit odeberete.
4. Vyberte počáteční podmínku a v seznamu níže upravte hodnoty jejích vlastností.

![Počáteční podmínky](image-Initial_conditions.png)

Počáteční podmínky jsou nepovinné. Není-li pro danou entitu modelu zadána žádná **počáteční podmínka**, použije se **environmentální podmínka**. Na entitách s **explicitní okrajovou podmínkou** se použije hodnota předepsaná okrajovou podmínkou.
