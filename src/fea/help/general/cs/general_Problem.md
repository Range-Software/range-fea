# Úloha

**Úloha** je obecný pojem pro jeden nebo více fyzikálních jevů aplikovaných na geometrii a je definována **typem úlohy**, **specifickým nastavením úlohy**, **okrajovými podmínkami**, **počátečními podmínkami**, **environmentálními podmínkami** a **nastavením materiálu**.

Úloha se definuje pro každý model zvlášť. Její části se nastavují v záložkách na pravé straně hlavního okna: **Úloha**, **Okrajové podmínky**, **Počáteční podmínky**, **Environmentální podmínky** a **Materiál**.

## Výběr úlohy

**Menu:** _Úloha -> Vybrat úlohu_

Dialog **Vybrat úlohu** slouží k výběru toho, co se má řešit (viz [Typ úlohy](general_Problem_type.md)). Zobrazí se také automaticky po načtení modelu, který nemá žádnou úlohu. Dokud není úloha vybrána, obsahuje záložka **Úloha** pouze tlačítko **Vybrat úlohu**.

## Specifické nastavení úlohy

Po výběru úlohy obsahuje záložka **Úloha** nastavení, které je specifické pro vybrané **typy úloh**.

![Záložka Úloha](image-Problem_tab.png)

- **Řešič v čase** – zaškrtněte, má-li se úloha řešit jako **přechodová** (časově závislá). Po zaškrtnutí lze nastavit metodu aproximace, počáteční čas, velikost časového kroku, počet časových kroků a frekvenci výstupu; koncový čas se z nich vypočítá (_koncový čas = počáteční čas + velikost časového kroku * počet časových kroků_). Není-li zaškrtnuto, řeší se úloha jako **ustálená**. Řešič v čase je dostupný pouze pro typy úloh, které jej podporují.
- Další sekce závisí na vybraných **typech úloh** (např. **Nastavení přechodové analýzy napětí**, nastavení radiace, akustiky nebo modální analýzy).

Další akce související s úlohou najdete v menu **Úloha**: _Nastavit maticový řešič úloh_, _Definovat monitorovací body_ a _Vymazat nastavení_.
