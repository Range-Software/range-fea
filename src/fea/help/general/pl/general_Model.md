# Model

**Model** to wirtualna geometryczna reprezentacja obiektu trójwymiarowego. Model składa się z geometrycznych **encji**.


**Encja** to zbiór elementów (simpleksów geometrycznych) podobnego typu; wyróżnia się następujące typy:

- **Punkt** – zbiór elementów 0D (węzeł).
- **Linia** – zbiór elementów 1D (odcinek).
- **Powierzchnia** – zbiór elementów 2D (trójkąt, czworokąt).
- **Objętość** – zbiór elementów 3D (czworościan, sześciościan).

Niezależnie od tego, czy elementy są 0D, 1D, 2D czy 3D, wszystkie są reprezentowane w przestrzeni 3D. Na przykład punkt (węzeł) nie ma wymiarów, ma tylko położenie, natomiast linia ma jeden wymiar (długość).


Do każdej **encji** z wyjątkiem **objętości** można przypisać właściwości **geometryczne**, które uzupełniają jej reprezentację 3D. Są to:

- **Punkt** – objętość w metrach sześciennych.
- **Linia** – pole przekroju poprzecznego w metrach kwadratowych.
- **Powierzchnia** – grubość w metrach.
