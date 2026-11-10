# Résultats

Les **résultats** sont produits par le **solveur** et se composent de **variables de résultat**.

## Variable de résultat

Une **variable de résultat** est un ensemble de valeurs calculées par le **solveur**. Une **variable de résultat** est définie par son **type** (p. ex. **Temperature** ou **Displacement**) et par un vecteur de valeurs. Selon son **type**, une **variable de résultat** peut contenir un vecteur de valeurs scalaires ou vectorielles.

Par exemple, la variable de résultat **Temperature** contient un vecteur de températures pour chaque nœud, alors que **Displacement** contient un vecteur de vecteurs de déplacement pour chaque nœud.

Toutes les variables de résultat du modèle sélectionné sont listées dans le menu déroulant en haut de l'onglet **Résultats**, avec leur nature (**Scalar** ou **Vector**).

![Variables de résultat](image-Results_variables.png)

Une **variable de résultat** peut être calculée au niveau des nœuds ou au niveau des éléments. Par exemple, **Displacement** est calculé au niveau des nœuds, alors que **Von Mises stress** est calculé au niveau des éléments. L'emplacement de la variable sélectionnée (**Sur les nœuds** ou **Sur les éléments**) est indiqué sous le menu déroulant.

Les variables au niveau des éléments sont affichées avec une valeur par élément. Pour obtenir un affichage continu :

- Les variables de contrainte proposent la case à cocher **Affichage lissé**. Elle moyenne les résultats des éléments aux nœuds pour l'affichage uniquement, les résultats eux-mêmes restent inchangés. Le moyennage abaisse les valeurs de crête.
- Les autres variables au niveau des éléments proposent le bouton **Afficher sur les nœuds**, qui fait passer la variable des éléments aux nœuds.

## Affichage des résultats

Les résultats sont affichés sur les entités sélectionnées dans l'**arbre du modèle**. Les résultats d'un problème résolu sur des **Volumes** se voient mieux lorsque les entités surfaciques sont masquées.

![Résultats](image-Results.png)

- **Afficher sur les entités sélectionnées** – cochez **Carte de couleurs** pour colorer les entités sélectionnées selon la valeur de la variable. Les variables vectorielles peuvent aussi être affichées sous forme de **Forme déformée**.
- **Plage de couleurs** – valeurs **Max** et **Min** de la carte de couleurs. Elles peuvent être saisies ou modifiées dans le **Réglage fin**. **Réinitialiser selon les données** rétablit la plage calculée à partir des résultats.
- **Couleurs** – palette de la carte de couleurs.
- **Afficher la légende** – affiche la plage de valeurs dans la zone 3D.
- **Échelle de déformation et des flèches** – multiplie la déformation des formes déformées et la longueur des flèches des champs vectoriels (variables vectorielles uniquement).
- **Graphique…** – affiche la distribution des valeurs et permet de régler la plage de couleurs sur un graphique.
- **Supprimer** – supprime la variable du modèle.

Les résultats peuvent aussi être affichés sur des entités créées à cet effet : **Coupes**, **ISO**, **Lignes de courant**, **Champs scalaires** et **Champs vectoriels**.
