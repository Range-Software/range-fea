# Exportovat model RAW

Exportuje vybraný model do souboru typu **RAW**.


Každý řádek souboru **RAW** obsahuje souřadnice všech uzlů právě jednoho prvku (simplexu). Prázdné řádky a komentáře začínající znakem **#** jsou povoleny.


Podporovány jsou tyto simplexy:


**Bod:**
```x1 y1 z1```


**Úsečka:**
```x1 y1 z1 x2 y2 z2```


**Trojúhelník:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Čtyřúhelník:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Příklad
Následující příklad představuje krychli složenou ze čtyřúhelníků s bodem v každém rohu.

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
# Čtyřúhelníky
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Spodní stěna
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Horní stěna
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Přední stěna
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Zadní stěna
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Levá stěna
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Pravá stěna```
