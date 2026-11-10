# Wyniki

**Wyniki** są tworzone przez **solver** i składają się ze **zmiennych wynikowych**.

## Zmienna wynikowa

**Zmienna wynikowa** to zbiór wartości obliczonych przez **solver**. **Zmienna wynikowa** jest zdefiniowana przez swój **typ** (np. **Temperature** lub **Displacement**) oraz wektor wartości. W zależności od **typu** **zmienna wynikowa** może zawierać wektor wartości skalarnych lub wektorowych.

Na przykład zmienna wynikowa **Temperature** zawiera wektor temperatur dla każdego węzła, natomiast **Displacement** zawiera wektor wektorów przemieszczeń dla każdego węzła.

Wszystkie zmienne wynikowe zaznaczonego modelu są wymienione w menu rozwijanym u góry zakładki **Wyniki**, wraz z ich rodzajem (**Scalar** lub **Vector**).

![Zmienne wynikowe](image-Results_variables.png)

**Zmienna wynikowa** może być obliczona na poziomie węzłów albo elementów. Na przykład **Displacement** jest obliczane na poziomie węzłów, natomiast **Von Mises stress** na poziomie elementów. Miejsce przechowywania wybranej zmiennej (**Na węzłach** lub **Na elementach**) jest pokazane poniżej menu rozwijanego.

Zmienne na poziomie elementów są wyświetlane z jedną wartością na element. Aby uzyskać ciągłe wyświetlanie:

- Zmienne naprężeń oferują pole wyboru **Wygładzone wyświetlanie**. Uśrednia ono wyniki elementów w węzłach wyłącznie na potrzeby wyświetlania, same wyniki pozostają bez zmian. Uśrednianie obniża wartości szczytowe.
- Pozostałe zmienne na poziomie elementów oferują przycisk **Wyświetl w węzłach**, który przenosi zmienną z elementów na węzły.

## Wyświetlanie wyników

Wyniki są wyświetlane na encjach zaznaczonych w **drzewie modelu**. Wyniki problemu rozwiązanego na **Objętościach** najlepiej widać, gdy encje powierzchniowe są ukryte.

![Wyniki](image-Results.png)

- **Pokaż na wybranych encjach** – zaznacz **Mapa kolorów**, aby pokolorować zaznaczone encje według wartości zmiennej. Zmienne wektorowe można też pokazać jako **Kształt odkształcony**.
- **Zakres kolorów** – wartości **Maks.** i **Min.** mapy kolorów. Można je wpisać lub zmienić w **Dokładnej regulacji**. **Przywróć według danych** przywraca zakres obliczony z wyników.
- **Kolory** – schemat kolorów mapy kolorów.
- **Pokaż legendę** – wyświetla zakres wartości w obszarze 3D.
- **Skala odkształcenia i strzałek** – mnoży odkształcenie kształtów odkształconych i długość strzałek pól wektorowych (tylko zmienne wektorowe).
- **Wykres…** – pokazuje rozkład wartości i pozwala ustawić zakres kolorów na wykresie.
- **Usuń** – usuwa zmienną z modelu.

Wyniki można też wyświetlać na encjach utworzonych w tym celu: **Przekroje**, **Izopowierzchnie**, **Linie prądu**, **Pola skalarne** i **Pola wektorowe**.
