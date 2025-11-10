# Export RAW model

Export selected model to **RAW** type file.


Each line in **RAW** file contains coordinates of all nodes of exactly one element (simplex). Empty lines and comments that begin with **#** are accepted.


Following simplexes are accepted:


**Point:**
```x1 y1 z1```


**Segment:**
```x1 y1 z1 x2 y2 z2```


**Triangle:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Quadrilateral:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Example
Following example represents cube made of four quadrilaterals with point in each corner.

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
# Quadrilaterals
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Bottom side
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Top side
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Front side
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Back side
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Left side
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Right side```

