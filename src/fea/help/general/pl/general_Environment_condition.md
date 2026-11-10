# Warunek otoczenia

**Warunek otoczenia** to warunek, który można zadać na dowolnej encji modelu, takiej jak Punkt, Linia, Powierzchnia lub Objętość. **Warunek otoczenia** określa wartości otoczenia specyficzne dla **problemu** (np. **Temperature** – temperatura otoczenia lub **Gravitational acceleration** – przyspieszenie grawitacyjne).

Dostępne warunki otoczenia zależą od wybranych **typów problemów**:

- **Gravitational acceleration** – składowe w kierunkach X, Y i Z, domyślnie 0, 0 i -9.80665 `[m/s^2]`.
- **Temperature** – domyślnie 293.15 `[K]`.

## Edycja warunku otoczenia

W odróżnieniu od **warunków brzegowych** warunków otoczenia się nie zaznacza. Są zawsze obecne i mają wartości domyślne.

1. Zaznacz jedną lub więcej encji w **drzewie modelu**.
2. Otwórz zakładkę **Warunki otoczenia** po prawej stronie okna głównego.
3. Zaznacz warunek otoczenia i edytuj wartości jego właściwości na liście poniżej.

![Warunki otoczenia](image-Environment_conditions.png)

Jeśli dla danej encji modelu nie zadano żadnego **warunku początkowego**, używany jest **warunek otoczenia**.
