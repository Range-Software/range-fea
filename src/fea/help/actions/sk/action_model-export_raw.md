# Exportovať RAW model

Exportuje vybraný model do súboru typu **RAW**.


Každý riadok v súbore **RAW** obsahuje súradnice všetkých uzlov práve jedného prvku (simplexu). Prázdne riadky a komentáre začínajúce znakom **#** sú akceptované.


Akceptované sú nasledujúce simplexy:


**Bod:**
```x1 y1 z1```


**Úsečka:**
```x1 y1 z1 x2 y2 z2```


**Trojuholník:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Štvoruholník:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Príklad
Nasledujúci príklad predstavuje kocku zloženú zo štyroch štvoruholníkov s bodom v každom rohu.

```# --------------------------------------------------
# X1  Y2  Z1   X2  Y2  Z2   X3  Y3  Z3   X4  Y4  Z4
# --------------------------------------------------
# Body
  0.0 0.0 0.0
  1.0 0.0 0.0
  0.0 1.0 0.0
  1.0 1.0 0.0
  0.0 0.0 1.0
  1.0 0.0 1.0
  0.0 1.0 1.0
  1.0 1.0 1.0
# Štvoruholníky
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Spodná strana
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Horná strana
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Predná strana
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Zadná strana
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Ľavá strana
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Pravá strana```
