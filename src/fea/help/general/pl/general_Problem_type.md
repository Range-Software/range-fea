# Typ problemu

**Typ problemu** to unikalny identyfikator zjawiska fizycznego, które ma zostać rozwiązane.

Typy problemów wybiera się w oknie dialogowym **Wybierz problem** (**Menu:** _Problem -> Wybierz problem_). Kliknij kafelek, aby zaznaczyć typ problemu lub cofnąć jego zaznaczenie, i kliknij **OK**, aby zatwierdzić. Przyciski w prawym górnym rogu przełączają między **Widokiem kafelków** a **Widokiem listy**, który pokazuje również krótki opis każdego typu problemu.

![Wybierz problem](image-Select_problem.png)

Lista obsługiwanych typów problemów:

- **Acoustics** (akustyka) – przejściowe i harmoniczne (w dziedzinie częstotliwości) ciśnienie akustyczne, prędkość cząstek, natężenie i poziom ciśnienia akustycznego.
- **Electro-statics** (elektrostatyka) – pole elektryczne, gęstość prądu i wydzielane ciepło Joule'a.
- **Magneto-statics** (magnetostatyka) – pole magnetyczne.
- **Incompressible viscous flow** (nieściśliwy przepływ lepki) – ustalony i przejściowy przepływ płynów newtonowskich.
- **Contaminant dispersion** (rozprzestrzenianie zanieczyszczeń) – rozkład „zanieczyszczenia” w polu przepływu.
- **Heat transfer in fluids** (wymiana ciepła w płynach) – rozkład temperatury w płynącym płynie.
- **Heat transfer** (wymiana ciepła) – ustalony i przejściowy rozkład temperatury oraz przepływ ciepła, z konwekcją prostą, wymuszoną i naturalną.
- **Radiative heat transfer** (wymiana ciepła przez promieniowanie) – wymiana ciepła między kilkoma powierzchniami zdolnymi do emitowania i/lub pochłaniania ciepła.
- **Stress analysis** (analiza naprężeń) – statyczna analiza naprężeń i liniowa elastodynamika, w tym rozszerzalność cieplna i obciążenia ciśnieniem płynu.
- **Modal analysis** (analiza modalna) – częstotliwości drgań własnych konstrukcji i ich postacie.
- **Flow through porous media** (przepływ przez ośrodek porowaty) – ciśnienie i prędkość płynu przepływającego przez materiał przepuszczalny.
- **Mesh reconstruction** (rekonstrukcja siatki) – rekonstrukcja siatki według zadanych kryteriów.

## Łączenie typów problemów

Można łączyć wiele różnych **typów problemów** (np. **Heat transfer** i **Stress analysis** albo **Incompressible viscous flow** i **Contaminant dispersion**).

- Niektóre typy problemów wymagają innego typu. Zaznaczenie takiego typu zaznacza również typ wymagany: **Magneto-statics** wymaga **Electro-statics**, **Contaminant dispersion** i **Heat transfer in fluids** wymagają **Incompressible viscous flow**, a **Radiative heat transfer** wymaga **Heat transfer**.
- Typy problemów, których nie można połączyć z bieżącym wyborem, są wyszarzone i oznaczone jako **Niezgodny** (np. **Modal analysis** nie można łączyć z żadnym innym typem problemu).

## Kolejność zadań

Wybrane typy problemów są rozwiązywane jeden po drugim jako zadania. Ich kolejność jest ustalana automatycznie tak, aby wyniki jednego zadania mogły być użyte przez następne (np. **Heat transfer** jest rozwiązywany przed **Stress analysis**, aby uwzględnić rozszerzalność cieplną).

Kliknij **Dostosuj…**, aby zobaczyć lub zmienić kolejność zadań. Zadania są wykonywane od góry do dołu. Przyciski **W górę** i **W dół** zmieniają ich kolejność, przycisk **>** umieszcza zadanie w grupie, która się powtarza (z własną liczbą iteracji i wartością zbieżności), a przycisk **<** wyjmuje je z grupy. Kliknij dwukrotnie wartość, aby ją edytować. **Użyj automatycznej kolejności** odrzuca własną kolejność zadań.

![Wybierz problem – własna kolejność zadań](image-Select_problem-customize.png)

Jeśli typ problemu zostanie usunięty z modelu, który zawiera już jego wyniki, aplikacja zaproponuje usunięcie tych wyników z modelu.
