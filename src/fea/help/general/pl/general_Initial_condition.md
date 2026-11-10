# Warunek początkowy

**Warunek początkowy** to warunek, który można zadać na dowolnej encji modelu (Punkt, Linia, Powierzchnia, Objętość). Można go traktować jako punkt startowy dla solvera. W **problemach ustalonych** dobrze dobrany **warunek początkowy** poprawia zbieżność solvera. W problemach **przejściowych** (zależnych od czasu) określa stan w chwili początkowej.

Warunki początkowe są specyficzne dla **problemu**: każdy **typ problemu** definiuje własny zestaw **warunków początkowych** (np. **Temperature** dla **Heat transfer**, **Velocity** i **Pressure** dla **Incompressible viscous flow** lub **Displacement** dla **Stress analysis**).

## Przypisywanie warunku początkowego

1. Zaznacz jedną lub więcej encji w **drzewie modelu**.
2. Otwórz zakładkę **Warunki początkowe** po prawej stronie okna głównego.
3. Zaznacz pole wyboru przed nazwą warunku początkowego, aby go przypisać. Usuń zaznaczenie, aby usunąć warunek początkowy z zaznaczonych encji.
4. Zaznacz warunek początkowy, aby edytować wartości jego właściwości na liście poniżej.

![Warunki początkowe](image-Initial_conditions.png)

Warunki początkowe są opcjonalne. Jeśli dla danej encji modelu nie zadano żadnego **warunku początkowego**, używany jest **warunek otoczenia**. Na encjach z **jawnym warunkiem brzegowym** używana jest wartość zadana przez warunek brzegowy.
