# Przekształcanie geometrii

Ten samouczek jest kontynuacją samouczka **Rysowanie sześcianu** i pokazuje, jak skalować, obracać i przesuwać całą geometrię lub tylko jej części.

## 1. Wczytaj model

Jeśli model **Cube** nie jest jeszcze wczytany, można to zrobić akcją **_Otwórz model_**.

**Menu:** _Plik -> Otwórz model_

Pojawi się okno dialogowe **Otwórz model**. Wybierz plik **Cube.rbm** i kliknij **Open** (Otwórz).

Gdy model jest gotowy, jego geometrię (całą lub część) można modyfikować (przekształcać).

## 2. Przygotowanie

**Menu:** _Geometria -> Skaluj, przesuń, obróć_

Po wybraniu tej pozycji menu po prawej stronie okna głównego pojawi się panel **Przekształć geometrię** z trzema zakładkami: **Skala**, **Obróć** i **Przesuń**. Grupa **Zastosuj do** pod zakładkami określa, która część modelu jest przekształcana.

![Przekształć geometrię](image-Transform_geometry.png)

W tym samouczku wszystkie przekształcenia zostaną kolejno zastosowane do jednej ściany sześcianu. Na razie zamknij panel, klikając **Anuluj**.

W tej chwili wszystkie ściany modelu **Cube** są zgrupowane w jednej **encji powierzchni**. Dlatego jedną ścianę trzeba **oznaczyć** jako osobną **encję powierzchni**, aby przekształcenia zostały zastosowane tylko do niej, a nie do pozostałych. Służy do tego akcja **_Oznacz powierzchnię_**. Najpierw jednak trzeba wskazać ścianę sześcianu.

## 3. Wskaż ścianę

Naciśnij i przytrzymaj klawisz **_Ctrl_** (**_⌘ Cmd_** w macOS) i kliknij **_lewym przyciskiem myszy_** ścianę sześcianu skierowaną w stronę osi **Y**. Wskazany element zostanie podświetlony i wymieniony w sekcji **Picked entities** w nakładce w lewym górnym rogu obszaru 3D. Ponieważ model **Cube** składa się z 6 prostokątnych (czworokątnych) elementów, jeden element odpowiada jednej ścianie sześcianu.

## 4. Oznacz powierzchnię

**Menu:** _Geometria -> Powierzchnia -> Oznacz powierzchnię_

![Okno Oznacz powierzchnię](image-Mark_surface_dialog.png)

Ta akcja wyświetla okno dialogowe **Mark entity (surface)**. Ponieważ w poprzednim kroku jeden element został **wskazany**, wstępnie wybrana jest opcja **_Oznacz tylko wybrane i powiązane elementy_**. Po kliknięciu przycisku **Ok** jedna ściana modelu **Cube** zostanie oznaczona jako osobna **encja powierzchni** o nazwie **Surface 2**.

_**Uwaga:** Nazwę każdej encji można zmienić, klikając **dwukrotnie** **lewym przyciskiem myszy** nazwę encji w **drzewie modelu** po lewej stronie okna głównego._

![Oznaczona powierzchnia](image-Marked_surface.png)

## 5. Skaluj wskazaną encję (ścianę sześcianu)

Wskaż oznaczoną powierzchnię (**_Ctrl_** / **_⌘ Cmd_** + **_lewy przycisk myszy_**) i otwórz panel **Przekształć geometrię**.

**Menu:** _Geometria -> Skaluj, przesuń, obróć_

1. Na zakładce **Skala** pozostaw zaznaczone **Ta sama skala we wszystkich kierunkach**.
2. Ustaw wartość **Skala** na 0,5 (użyj separatora dziesiętnego swojego systemu, np. 0,5 lub 0.5).
3. Kliknij przycisk **Ustaw ze wskazanego elementu/węzła**. **Środek skalowania** zostanie ustawiony na środek wskazanej ściany (0,5; 1; 0,5).
4. W grupie **Zastosuj do** wybierz **Wskazane encje**.
5. Upewnij się, że zaznaczone jest **Uwzględnij węzły wspólne** oraz wszystkie jego podrzędne pola wyboru.

Prawidłowe ustawienia widać na poniższym zrzucie ekranu.

![Przekształć geometrię – skala](image-Transform_geometry-scale.png)

Kliknij **Ok**. Wskazana ściana zostanie zmniejszona o połowę, a przestrzeń między zmniejszoną ścianą a resztą sześcianu zostanie wypełniona nową encją powierzchni (**Surface 3**). Wynik widać na poniższym zrzucie ekranu.

![Przeskalowana powierzchnia](image-Scaled_surface.png)

## 6. Obróć wskazaną encję (przeskalowaną ścianę sześcianu)

Obracając wskazaną powierzchnię, można utworzyć powierzchnię wyciągniętą. Otwórz panel **Przekształć geometrię**.

**Menu:** _Geometria -> Skaluj, przesuń, obróć_

Przejdź na zakładkę **Obróć** i ustaw wartości jak na poniższym zrzucie ekranu:

1. Ustaw **Kąty obrotu** **X** na -90.
2. Kliknij przycisk **Ustaw ze wskazanego elementu/węzła**, a następnie zmień **Środek obrotu** **Z** na 0.
3. W grupie **Zastosuj do** wybierz **Wskazane encje**.
4. Upewnij się, że zaznaczone są **Uwzględnij węzły wspólne**, **Odłącz/rozdziel węzły wspólne** i **Przeciągaj węzły**, a **Kroki przeciągania** ustaw na 6.

_Uwaga: **Kroki przeciągania: 6** utworzą sześć nowych segmentów._

![Przekształć geometrię – obrót](image-Transform_geometry-rotate.png)

Kliknij **Ok**. Wyciągnięte segmenty zostaną dodane jako nowa encja powierzchni (**Surface 4**). Wynik widać na poniższym zrzucie ekranu.

![Obrócona powierzchnia](image-Rotated_surface.png)
