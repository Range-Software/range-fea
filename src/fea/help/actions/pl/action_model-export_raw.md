# Eksportuj model RAW

Eksportuje wybrany model do pliku typu **RAW**.


Każdy wiersz pliku **RAW** zawiera współrzędne wszystkich węzłów dokładnie jednego elementu (simpleksu). Dozwolone są puste wiersze oraz komentarze zaczynające się od **#**.


Akceptowane są następujące simpleksy:


**Punkt:**
```x1 y1 z1```


**Odcinek:**
```x1 y1 z1 x2 y2 z2```


**Trójkąt:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Czworokąt:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Przykład
Poniższy przykład przedstawia sześcian złożony z czworokątów, z punktem w każdym narożniku.

```# --------------------------------------------------
# X1  Y2  Z1   X2  Y2  Z2   X3  Y3  Z3   X4  Y4  Z4
# --------------------------------------------------
# Punkty
  0.0 0.0 0.0
  1.0 0.0 0.0
  0.0 1.0 0.0
  1.0 1.0 0.0
  0.0 0.0 1.0
  1.0 0.0 1.0
  0.0 1.0 1.0
  1.0 1.0 1.0
# Czworokąty
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Ściana dolna
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Ściana górna
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Ściana przednia
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Ściana tylna
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Ściana lewa
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Ściana prawa```
