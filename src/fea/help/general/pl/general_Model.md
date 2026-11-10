# Model

**Model** to wirtualna geometryczna reprezentacja trójwymiarowego obiektu. Model składa się z geometrycznych **encji**.

![Model](image-Model.png)

## Encje

**Encja** to zbiór elementów (sympleksów geometrycznych) tego samego typu. Rozróżnia się następujące typy:

- **Punkt** – zbiór elementów 0D (węzeł).
- **Linia** – zbiór elementów 1D (odcinek).
- **Powierzchnia** – zbiór elementów 2D (trójkąt, czworokąt).
- **Objętość** – zbiór elementów 3D (czworościan).

Niezależnie od tego, czy elementy są 0D, 1D, 2D czy 3D, wszystkie są reprezentowane w przestrzeni 3D. Na przykład punkt (węzeł) nie ma wymiarów, ma jedynie położenie, natomiast linia ma jeden wymiar (długość).

Oprócz tych encji geometrycznych model może zawierać encje utworzone z obliczonych **wyników**: **Przekroje**, **Izopowierzchnie**, **Linie prądu**, **Pola skalarne** i **Pola wektorowe**.

## Drzewo modelu

Wszystkie encje modelu są wymienione w **drzewie modelu** (zakładka **Model** po lewej stronie okna głównego) i pogrupowane według typu. Kolumna **#** pokazuje liczbę encji w grupie lub liczbę elementów w encji.

![Drzewo modelu](image-Model_tree.png)

- Pole wyboru przed nazwą pokazuje lub ukrywa model, grupę albo encję w obszarze 3D.
- Kliknięcie wiersza zaznacza go. Aby zaznaczyć wiele encji, przytrzymaj klawisz _Ctrl_ (_⌘ Cmd_ w macOS) podczas zaznaczania albo klawisz _Shift_, aby zaznaczyć zakres. Zaznaczenie modelu lub grupy zaznacza wszystkie zawarte w nich encje.
- **Warunki brzegowe**, **Warunki początkowe**, **Warunki otoczenia**, **Materiał** i **Wyniki** są zawsze przypisywane do encji zaznaczonych w **drzewie modelu**.

**Właściwości wyświetlania** poniżej drzewa określają sposób rysowania zaznaczonych encji (np. **Jako siatka krawędziowa**, **Krawędzie elementów**, **Węzły elementów**, **Warunki brzegowe**) oraz ich kolor.

## Właściwości geometryczne

Każdej **encji** z wyjątkiem **Objętości** można przypisać właściwość **geometryczną**, która uzupełnia jej reprezentację 3D. Dla zaznaczonych encji jest ona pokazana na liście **Właściwości geometryczne** poniżej **drzewa modelu**:

- **Objętość punktu** – objętość **Punktu** w metrach sześciennych.
- **Pole przekroju linii** – pole przekroju poprzecznego **Linii** w metrach kwadratowych.
- **Grubość powierzchni** – grubość **Powierzchni** w metrach.
