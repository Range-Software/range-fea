# Environmentální podmínka

**Environmentální podmínka** je podmínka, kterou lze zadat na jakékoli entitě modelu, jako je Bod, Čára, Povrch nebo Objem. **Environmentální podmínka** určuje hodnoty prostředí specifické pro **úlohu** (např. **Temperature** – teplota prostředí nebo **Gravitational acceleration** – gravitační zrychlení).

Dostupné environmentální podmínky závisí na vybraných **typech úloh**:

- **Gravitational acceleration** – složky ve směru X, Y a Z, výchozí hodnoty 0, 0 a -9.80665 `[m/s^2]`.
- **Temperature** – výchozí hodnota 293.15 `[K]`.

## Úprava environmentální podmínky

Na rozdíl od **okrajových podmínek** se environmentální podmínky nezaškrtávají. Jsou vždy přítomny a mají výchozí hodnoty.

1. Vyberte jednu nebo více entit ve **stromu modelu**.
2. Otevřete záložku **Environmentální podmínky** na pravé straně hlavního okna.
3. Vyberte environmentální podmínku a v seznamu níže upravte hodnoty jejích vlastností.

![Environmentální podmínky](image-Environment_conditions.png)

Není-li pro danou entitu modelu zadána žádná **počáteční podmínka**, použije se **environmentální podmínka**.
