# Résultats

Les **résultats** sont produits par le **solveur** et se composent de **variables de résultat**.

## Variable de résultat

Une **variable de résultat** est un ensemble de valeurs calculées par le **solveur**. Une **variable de résultat** est définie par son **type** (par exemple **température** ou **déplacement**) et un vecteur de valeurs. Selon le **type**, une **variable de résultat** peut contenir un vecteur de valeurs scalaires ou de valeurs vectorielles.


Par exemple, la variable de résultat **température** contient un vecteur de températures pour chaque nœud, tandis que le **déplacement** contient un vecteur de vecteurs de déplacement pour chaque nœud.


Une **variable de résultat** peut être calculée au niveau des nœuds ou au niveau des éléments. Par exemple, le **déplacement** est calculé au niveau des nœuds, alors que la **contrainte** l'est au niveau des éléments. Toute variable calculée au niveau des éléments peut être convertie au niveau des nœuds. Cette conversion est une opération unique et irréversible ; pour revenir en arrière, les résultats doivent être recalculés par le **solveur**.
