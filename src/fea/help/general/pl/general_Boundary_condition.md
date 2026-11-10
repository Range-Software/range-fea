# Warunek brzegowy

**Warunek brzegowy** to warunek zadany na brzegu, przy czym brzegiem może być dowolna encja modelu (Punkt, Linia, Powierzchnia, Objętość).

Warunki brzegowe są:
- **Specyficzne dla problemu** – Każdy **typ problemu** definiuje własny zestaw **warunków brzegowych**.
- **Specyficzne dla encji** – Nie każdy warunek brzegowy można zastosować do każdego typu encji (np. **Pressure** można zastosować tylko do **Powierzchni**).
- **Jawne** – Na brzegu zadane jest dokładne rozwiązanie (np. **Temperature** dla **Heat transfer**). Na liście **Warunki brzegowe** są wyświetlane pogrubioną czcionką.
- **Niejawne** – Rozwiązanie na brzegu z nich wynika (np. **Heat** dla **Heat transfer**).

## Przypisywanie warunku brzegowego

1. Zaznacz jedną lub więcej encji w **drzewie modelu**.
2. Otwórz zakładkę **Warunki brzegowe** po prawej stronie okna głównego. Lista zawiera tylko warunki brzegowe poprawne dla wybranych **typów problemów** i zaznaczonych encji.
3. Zaznacz pole wyboru przed nazwą warunku brzegowego, aby go przypisać. Usuń zaznaczenie, aby usunąć warunek brzegowy z zaznaczonych encji.
4. Zaznacz warunek brzegowy, aby edytować wartości jego właściwości na liście poniżej.

![Warunki brzegowe](image-Boundary_conditions.png)

Niektóre właściwości mają własne pole wyboru. Na przykład przy każdej składowej warunku brzegowego **Displacement** można usunąć zaznaczenie, aby pozostawić przemieszczenie w danym kierunku swobodne.

Warunki brzegowe działające w kierunku wynikającym z geometrii (**Normal displacement**, **Roller displacement**) wyznaczają swój kierunek lokalny na podstawie encji. Zaznacz **Użyj wprowadzonego kierunku lokalnego**, aby wprowadzić kierunek ręcznie.

Aby wyświetlić przypisane warunki brzegowe w obszarze 3D, zaznacz **Warunki brzegowe** we **Właściwościach wyświetlania**.

## Wartości zależne od czasu

Wartość każdej właściwości może zależeć od czasu. Rozwiń właściwość i kliknij **Edytuj wartości zależne od czasu**, aby wprowadzić tabelę wartości dla zadanych chwil czasu. Wartości zależne od czasu są używane, gdy problem jest **przejściowy** (zależny od czasu), czyli gdy w zakładce **Problem** włączony jest **Solver czasowy**.
