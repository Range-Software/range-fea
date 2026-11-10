# Modèle

Un **modèle** est une représentation géométrique virtuelle d'un objet tridimensionnel. Un modèle est constitué d'**entités** géométriques.

![Modèle](image-Model.png)

## Entités

Une **entité** est un ensemble d'éléments (simplexes géométriques) de même type. On distingue les types suivants :

- **Point** – ensemble d'éléments 0D (nœud).
- **Ligne** – ensemble d'éléments 1D (segment).
- **Surface** – ensemble d'éléments 2D (triangle, quadrilatère).
- **Volume** – ensemble d'éléments 3D (tétraèdre).

Que les éléments soient 0D, 1D, 2D ou 3D, ils sont tous représentés dans l'espace 3D. Par exemple, un point (nœud) n'a pas de dimension, il n'a qu'une position, alors qu'une ligne a une dimension (longueur).

Outre ces entités géométriques, un modèle peut contenir des entités créées à partir des **résultats** calculés : **Coupes**, **ISO**, **Lignes de courant**, **Champs scalaires** et **Champs vectoriels**.

## Arbre du modèle

Toutes les entités d'un modèle sont listées dans l'**arbre du modèle** (onglet **Modèle** à gauche de la fenêtre principale), regroupées par type. La colonne **#** indique le nombre d'entités d'un groupe ou le nombre d'éléments d'une entité.

![Arbre du modèle](image-Model_tree.png)

- La case à cocher devant un nom affiche ou masque le modèle, le groupe ou l'entité dans la zone 3D.
- Un clic sur une ligne la sélectionne. Pour sélectionner plusieurs entités, maintenez la touche _Ctrl_ (_⌘ Cmd_ sous macOS) pendant la sélection, ou la touche _Maj_ pour sélectionner une plage. La sélection d'un modèle ou d'un groupe sélectionne toutes les entités qu'il contient.
- Les **Conditions aux limites**, les **Conditions initiales**, les **Conditions d'environnement**, le **Matériau** et les **Résultats** sont toujours affectés aux entités sélectionnées dans l'**arbre du modèle**.

Les **Propriétés d'affichage**, sous l'arbre, déterminent la manière dont les entités sélectionnées sont dessinées (p. ex. **En fil de fer**, **Arêtes des éléments**, **Nœuds des éléments**, **Conditions aux limites**) ainsi que leur couleur.

## Propriétés géométriques

Chaque **entité**, à l'exception du **Volume**, peut recevoir une propriété **géométrique** qui complète sa représentation 3D. Elle est affichée pour les entités sélectionnées dans la liste **Propriétés géométriques**, sous l'**arbre du modèle** :

- **Volume du point** – volume d'un **Point** en mètres cubes.
- **Section de la ligne** – aire de la section d'une **Ligne** en mètres carrés.
- **Épaisseur de la surface** – épaisseur d'une **Surface** en mètres.
