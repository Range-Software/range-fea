# RAW-Modell exportieren

Exportiert das ausgewählte Modell in eine Datei vom Typ **RAW**.


Jede Zeile einer **RAW**-Datei enthält die Koordinaten aller Knoten genau eines Elements (Simplex). Leere Zeilen und Kommentare, die mit **#** beginnen, sind zulässig.


Folgende Simplexe werden akzeptiert:


**Punkt:**
```x1 y1 z1```


**Segment:**
```x1 y1 z1 x2 y2 z2```


**Dreieck:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Viereck:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Beispiel
Das folgende Beispiel stellt einen Würfel aus Vierecken mit einem Punkt in jeder Ecke dar.

```# --------------------------------------------------
# X1  Y2  Z1   X2  Y2  Z2   X3  Y3  Z3   X4  Y4  Z4
# --------------------------------------------------
# Punkte
  0.0 0.0 0.0
  1.0 0.0 0.0
  0.0 1.0 0.0
  1.0 1.0 0.0
  0.0 0.0 1.0
  1.0 0.0 1.0
  0.0 1.0 1.0
  1.0 1.0 1.0
# Vierecke
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Unterseite
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Oberseite
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Vorderseite
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Rückseite
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Linke Seite
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Rechte Seite```
