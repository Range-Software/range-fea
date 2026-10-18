# Wyniki

**Wyniki** są tworzone przez **solver** i składają się ze **zmiennych wynikowych**.

## Zmienna wynikowa

**Zmienna wynikowa** to zbiór wartości obliczonych przez **solver**. **Zmienna wynikowa** jest określona przez swój **typ** (np. **temperatura** lub **przemieszczenie**) oraz wektor wartości. W zależności od **typu** **zmienna wynikowa** może zawierać wektor wartości skalarnych lub wektorowych.


Na przykład zmienna wynikowa **temperatura** zawiera wektor temperatur dla każdego węzła, natomiast **przemieszczenie** zawiera wektor wektorów przemieszczeń dla każdego węzła.


**Zmienną wynikową** można obliczać na poziomie węzłów lub elementów. Na przykład **przemieszczenie** jest obliczane na poziomie węzłów, a **naprężenie** na poziomie elementów. Każdą obliczoną zmienną na poziomie elementów można przekształcić na poziom węzłów. Takie przekształcenie jest operacją jednorazową i nie można go cofnąć; aby to zrobić, wyniki muszą zostać ponownie obliczone przez **solver**.
