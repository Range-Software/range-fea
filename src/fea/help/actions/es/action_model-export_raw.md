# Exportar modelo RAW

Exporta el modelo seleccionado a un archivo de tipo **RAW**.


Cada línea de un archivo **RAW** contiene las coordenadas de todos los nodos de exactamente un elemento (símplex). Se admiten líneas vacías y comentarios que empiezan por **#**.


Se admiten los siguientes símplex:


**Punto:**
```x1 y1 z1```


**Segmento:**
```x1 y1 z1 x2 y2 z2```


**Triángulo:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3```


**Cuadrilátero:**
```x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4```

## Ejemplo
El siguiente ejemplo representa un cubo formado por cuadriláteros, con un punto en cada vértice.

```# --------------------------------------------------
# X1  Y2  Z1   X2  Y2  Z2   X3  Y3  Z3   X4  Y4  Z4
# --------------------------------------------------
# Puntos
  0.0 0.0 0.0
  1.0 0.0 0.0
  0.0 1.0 0.0
  1.0 1.0 0.0
  0.0 0.0 1.0
  1.0 0.0 1.0
  0.0 1.0 1.0
  1.0 1.0 1.0
# Cuadriláteros
  0.0 0.0 0.0  0.0 1.0 0.0  1.0 1.0 0.0  1.0 0.0 0.0  # Cara inferior
  0.0 0.0 1.0  1.0 0.0 1.0  1.0 1.0 1.0  0.0 1.0 1.0  # Cara superior
  0.0 0.0 0.0  1.0 0.0 0.0  1.0 0.0 1.0  0.0 0.0 1.0  # Cara frontal
  0.0 1.0 0.0  0.0 1.0 1.0  1.0 1.0 1.0  1.0 1.0 0.0  # Cara posterior
  0.0 0.0 0.0  0.0 0.0 1.0  0.0 1.0 1.0  0.0 1.0 0.0  # Cara izquierda
  1.0 0.0 0.0  1.0 1.0 0.0  1.0 1.0 1.0  1.0 0.0 1.0  # Cara derecha```
