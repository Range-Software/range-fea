# Esporta modello RAW

Esporta il modello selezionato in un file di tipo **RAW**.


Ogni riga di un file **RAW** contiene le coordinate di tutti i nodi di esattamente un elemento (simplesso). Sono ammesse righe vuote e commenti che iniziano con **#**.


Sono accettati i seguenti simplessi:


**Punto:**
```x1 y1 z1```


**Segmento:**
```x1 y1 z1 x2 y2 z2```


**Triangolo:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Quadrilatero:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Esempio
L'esempio seguente rappresenta un cubo composto da quadrilateri con un punto in ogni vertice.

```# --------------------------------------------------
# X1  Y2  Z1   X2  Y2  Z2   X3  Y3  Z3   X4  Y4  Z4
# --------------------------------------------------
# Punti
  0.0 0.0 0.0
  1.0 0.0 0.0
  0.0 1.0 0.0
  1.0 1.0 0.0
  0.0 0.0 1.0
  1.0 0.0 1.0
  0.0 1.0 1.0
  1.0 1.0 1.0
# Quadrilateri
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Lato inferiore
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Lato superiore
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Lato anteriore
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Lato posteriore
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Lato sinistro
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Lato destro```
