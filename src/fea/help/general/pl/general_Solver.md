# Solver

**Solver** to narzędzie, które przetwarza ustawienia **problemu** i oblicza **wyniki** dla danego **modelu**.

W zależności od rozmiaru **modelu**, **typu problemu**, ustawień zbieżności i ustawień przejściowych czas rozwiązania **problemu** może wynosić od sekund do godzin, a nawet dłużej.

## Uruchamianie solvera

**Menu:** _Rozwiązanie -> Uruchom solver_

Przed uruchomieniem solvera model musi zostać zapisany. Jeśli nie został jeszcze zapisany, najpierw wyświetlane jest okno dialogowe **Zapisz model**. Następnie wyświetlane jest okno dialogowe **Uruchom solver**.

![Uruchom solver](image-Start_solver.png)

Przed uruchomieniem solvera sprawdzane są ustawienia problemu. Znalezione **Błędy** (np. encja bez materiału) i **Ostrzeżenia** są wymienione w oknie **Uruchom solver**. Jeśli model zawiera już wyniki, można zaznaczyć **Uruchom solver ponownie / kontynuuj**, aby kontynuować od tych wyników zamiast zaczynać od początku.

Solver działa jako osobny proces w tle, dzięki czemu można kontynuować pracę podczas obliczeń. Całe jego wyjście jest wyświetlane w zakładce **Wyjście procesu** poniżej obszaru 3D. Po zakończeniu pracy solvera obliczone **wyniki** są automatycznie wczytywane do modelu.

## Zatrzymywanie solvera

- _Rozwiązanie -> Zatrzymaj solver_ – prosi wszystkie działające solvery o zatrzymanie.
- _Rozwiązanie -> Przerwij solver_ – natychmiast kończy proces solvera.

## Monitorowanie solvera

- _Raport -> Zbieżność solvera_ – pokazuje wykres zbieżności solvera.
- _Raport -> Punkty monitorowania_ – pokazuje wartości obliczone w punktach monitorowania zdefiniowanych przez _Problem -> Zdefiniuj punkty monitorowania_.
- _Raport -> Plik dziennika solvera_ – pokazuje plik dziennika solvera.
