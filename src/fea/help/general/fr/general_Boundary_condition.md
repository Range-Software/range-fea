# Condition aux limites

Une **condition aux limites** est une condition imposée à la frontière, la frontière pouvant être n'importe quelle entité du modèle (Point, Ligne, Surface, Volume).

Les conditions aux limites sont :
- **Spécifiques au problème** – Chaque **type de problème** définit son propre ensemble de **conditions aux limites**.
- **Spécifiques à l'entité** – Toutes les conditions aux limites ne peuvent pas être appliquées à tous les types d'entité (p. ex. **Pressure** ne peut être appliquée qu'à une **Surface**).
- **Explicites** – La solution exacte est imposée à la frontière (p. ex. **Temperature** pour **Heat transfer**). Affichées en gras dans la liste **Conditions aux limites**.
- **Implicites** – Elles impliquent une solution à la frontière (p. ex. **Heat** pour **Heat transfer**).

## Affectation d'une condition aux limites

1. Sélectionnez une ou plusieurs entités dans l'**arbre du modèle**.
2. Ouvrez l'onglet **Conditions aux limites** à droite de la fenêtre principale. La liste ne contient que les conditions aux limites valables pour les **types de problème** sélectionnés et pour les entités sélectionnées.
3. Cochez la case devant le nom de la condition aux limites pour l'affecter. Décochez-la pour retirer la condition aux limites des entités sélectionnées.
4. Sélectionnez la condition aux limites pour modifier les valeurs de ses propriétés dans la liste située en dessous.

![Conditions aux limites](image-Boundary_conditions.png)

Certaines propriétés possèdent leur propre case à cocher. Par exemple, chaque composante de la condition aux limites **Displacement** peut être décochée pour laisser le déplacement libre dans cette direction.

Les conditions aux limites qui agissent dans une direction donnée par la géométrie (**Normal displacement**, **Roller displacement**) déduisent leur direction locale de l'entité. Cochez **Utiliser la direction locale saisie** pour saisir la direction à la main.

Pour afficher les conditions aux limites affectées dans la zone 3D, cochez **Conditions aux limites** dans les **Propriétés d'affichage**.

## Valeurs dépendant du temps

La valeur de chaque propriété peut dépendre du temps. Développez la propriété et cliquez sur **Modifier les valeurs dépendant du temps** pour saisir un tableau de valeurs pour des instants donnés. Les valeurs dépendant du temps sont utilisées lorsque le problème est **transitoire** (dépendant du temps), c'est-à-dire lorsque le **Solveur temporel** est activé dans l'onglet **Problème**.
