# Exporter le modèle RAW

Exporte le modèle sélectionné dans un fichier de type **RAW**.


Chaque ligne d'un fichier **RAW** contient les coordonnées de tous les nœuds d'exactement un élément (simplexe). Les lignes vides et les commentaires commençant par **#** sont acceptés.


Les simplexes suivants sont acceptés :


**Point :**
```x1 y1 z1```


**Segment :**
```x1 y1 z1 x2 y2 z2```


**Triangle :**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Quadrilatère :**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Exemple
L'exemple suivant représente un cube composé de quadrilatères, avec un point à chaque sommet.

```# --------------------------------------------------
# X1  Y2  Z1   X2  Y2  Z2   X3  Y3  Z3   X4  Y4  Z4
# --------------------------------------------------
# Points
  0.0 0.0 0.0
  1.0 0.0 0.0
  0.0 1.0 0.0
  1.0 1.0 0.0
  0.0 0.0 1.0
  1.0 0.0 1.0
  0.0 1.0 1.0
  1.0 1.0 1.0
# Quadrilatères
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Face inférieure
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Face supérieure
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Face avant
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Face arrière
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Face gauche
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Face droite```
