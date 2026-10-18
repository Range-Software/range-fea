# Przekształcanie geometrii

Ten samouczek jest kontynuacją samouczka **Rysowanie sześcianu** i pokazuje, jak skalować, obracać i przesuwać całą geometrię lub tylko jej części.

## 1. Wczytaj model

Jeśli model **Cube** nie jest jeszcze wczytany, można go wczytać akcją menu **_Otwórz model_**.

**Menu:** _Plik -> Otwórz model_

Pojawi się okno **Otwórz model**. Wybierz plik **Cube.rbm** i kliknij **Otwórz**.

Gdy model jest gotowy, jego geometrię (całą lub część) można modyfikować (przekształcać).

## 2. Przygotowanie

**Menu:** _Geometria -> Skaluj, przesuń, obróć_

Po wybraniu tej pozycji menu po prawej stronie okna głównego pojawi się panel **Przekształć geometrię** z trzema kartami: **Skala**, **Obróć** i **Przesuń**.

![Przekształć geometrię](image-Transform_geometry.png)

W tym samouczku wszystkie przekształcenia będą kolejno stosowane do jednej ściany sześcianu.

W tej chwili wszystkie ściany modelu **Cube** są zgrupowane w tej samej **encji powierzchni**. Dlatego jedną ścianę trzeba **oznaczyć** jako oddzielną **encję powierzchni**, aby przekształcenia dotyczyły tylko tej ściany, a nie pozostałych. Służy do tego akcja menu **_Oznacz powierzchnię_**. Najpierw jednak należy wskazać ścianę sześcianu.

## 3. Wskaż ścianę

Przytrzymaj klawisz **_Ctrl_** (**_⌘ Cmd_** w systemie macOS) i kliknij **_lewym przyciskiem myszy_** jedną ze ścian sześcianu. Wskazany element zostanie podświetlony. Ponieważ model **Cube** składa się z 6 prostokątnych (czworokątnych) elementów, jeden element odpowiada jednej ścianie **sześcianu**.

## 4. Oznacz powierzchnię

**Menu:** _Geometria -> Powierzchnia -> Oznacz powierzchnię_

![Okno Oznacz powierzchnię](image-Mark_surface_dialog.png)

Ta akcja otworzy okno **Oznacz encję (powierzchnia)**. Ponieważ w poprzednim kroku **wskazano** jeden element, opcja **_Oznacz tylko wybrane i powiązane elementy_** jest wybrana domyślnie. Po kliknięciu **OK** jedna ściana modelu **Cube** zostanie oznaczona jako oddzielna **encja powierzchni**.

_**Uwaga:** nazwę każdej encji można zmienić, **klikając dwukrotnie** **lewym przyciskiem myszy** jej nazwę w **drzewie modelu** po lewej stronie okna głównego._

![Oznaczona powierzchnia](image-Marked_surface.png)

## 5. Przeskaluj wskazaną encję (ścianę sześcianu)

Wskaż oznaczoną powierzchnię (**_Ctrl_** / **_⌘ Cmd_** + **_lewy przycisk myszy_**) i otwórz panel **Przekształć geometrię**.

**Menu:** _Geometria -> Skaluj, przesuń, obróć_

1. Zaznacz **Ta sama skala we wszystkich kierunkach**.
2. Ustaw wartość **Skala** na 0.5 (użyj separatora dziesiętnego swojego systemu, np. 0,5).
3. Kliknij przycisk **Ustaw ze wskazanego elementu/węzła**.
4. W grupie **Zastosuj do** wybierz **Wskazane encje**.
5. Upewnij się, że zaznaczono **Uwzględnij węzły wspólne** oraz **wszystkie** jego podrzędne pola wyboru.

Prawidłowe ustawienia widać na poniższym zrzucie ekranu.

![Przekształć geometrię – skala](image-Transform_geometry-scale.png)

Kliknij **OK**. Wynik widać na poniższym zrzucie ekranu.

![Przeskalowana powierzchnia](image-Scaled_surface.png)

## 6. Obróć wskazaną encję (przeskalowaną ścianę sześcianu)

Obracając wskazaną powierzchnię, można utworzyć powierzchnię wyciągniętą. Otwórz panel **Przekształć geometrię**.

**Menu:** _Geometria -> Skaluj, przesuń, obróć_

Przejdź na kartę **Obróć** i wprowadź wartości jak na poniższym zrzucie ekranu:

1. W **Kąty obrotu** ustaw **X** na -90.
2. Kliknij przycisk **Ustaw ze wskazanego elementu/węzła**, a następnie zmień **Z** w **Środek obrotu** na 0.
3. W grupie **Zastosuj do** wybierz **Wskazane encje**.
4. Upewnij się, że zaznaczono **Uwzględnij węzły wspólne**, **Odłącz/rozdziel węzły wspólne** i **Przeciągaj węzły**, oraz ustaw **Kroki przeciągania** na 6.

_Uwaga: **Kroki przeciągania: 6** dadzą sześć nowych segmentów._

![Przekształć geometrię – obrót](image-Transform_geometry-rotate.png)

Kliknij **OK**. Wynik widać na poniższym zrzucie ekranu.

![Obrócona powierzchnia](image-Rotated_surface.png)
