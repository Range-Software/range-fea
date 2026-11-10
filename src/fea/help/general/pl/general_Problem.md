# Problem

**Problem** to ogólne określenie jednego lub wielu zjawisk fizycznych zastosowanych do geometrii. Jest zdefiniowany przez **typ problemu**, **ustawienia specyficzne dla problemu**, **warunki brzegowe**, **warunki początkowe**, **warunki otoczenia** i **ustawienia materiału**.

Problem definiuje się osobno dla każdego modelu. Jego części ustawia się w zakładkach po prawej stronie okna głównego: **Problem**, **Warunki brzegowe**, **Warunki początkowe**, **Warunki otoczenia** i **Materiał**.

## Wybór problemu

**Menu:** _Problem -> Wybierz problem_

Okno dialogowe **Wybierz problem** służy do wyboru tego, co ma zostać rozwiązane (zob. [Typ problemu](general_Problem_type.md)). Jest ono także wyświetlane automatycznie po wczytaniu modelu bez żadnego problemu. Dopóki problem nie zostanie wybrany, zakładka **Problem** zawiera tylko przycisk **Wybierz problem**.

## Ustawienia specyficzne dla problemu

Po wybraniu problemu zakładka **Problem** zawiera ustawienia specyficzne dla wybranych **typów problemów**.

![Zakładka Problem](image-Problem_tab.png)

- **Solver czasowy** – zaznacz, aby rozwiązać problem jako **przejściowy** (zależny od czasu). Po zaznaczeniu można ustawić metodę aproksymacji, czas początkowy, wielkość kroku czasowego, liczbę kroków czasowych i częstotliwość zapisu wyników; czas końcowy jest z nich obliczany (_czas końcowy = czas początkowy + wielkość kroku czasowego * liczba kroków czasowych_). Bez zaznaczenia problem jest rozwiązywany jako **ustalony**. Solver czasowy jest dostępny tylko dla typów problemów, które go obsługują.
- Dodatkowe sekcje zależą od wybranych **typów problemów** (np. **Ustawienia przejściowej analizy naprężeń**, ustawienia promieniowania, akustyki lub analizy modalnej).

Inne akcje związane z problemem znajdują się w menu **Problem**: _Konfiguruj solver macierzowy problemów_, _Zdefiniuj punkty monitorowania_ i _Resetuj ustawienia_.
