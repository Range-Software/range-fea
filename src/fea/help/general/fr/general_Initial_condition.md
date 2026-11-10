# Condition initiale

Une **condition initiale** est une condition qui peut être imposée à n'importe quelle entité du modèle (Point, Ligne, Surface, Volume) et peut être vue comme un point de départ pour le solveur. Pour les **problèmes en régime permanent**, une **condition initiale** bien choisie améliore la convergence du solveur. Pour les problèmes **transitoires** (dépendant du temps), elle définit l'état au temps initial.

Les conditions initiales sont spécifiques au **problème** : chaque **type de problème** définit son propre ensemble de **conditions initiales** (p. ex. **Temperature** pour **Heat transfer**, **Velocity** et **Pressure** pour **Incompressible viscous flow**, ou **Displacement** pour **Stress analysis**).

## Affectation d'une condition initiale

1. Sélectionnez une ou plusieurs entités dans l'**arbre du modèle**.
2. Ouvrez l'onglet **Conditions initiales** à droite de la fenêtre principale.
3. Cochez la case devant le nom de la condition initiale pour l'affecter. Décochez-la pour retirer la condition initiale des entités sélectionnées.
4. Sélectionnez la condition initiale pour modifier les valeurs de ses propriétés dans la liste située en dessous.

![Conditions initiales](image-Initial_conditions.png)

Les conditions initiales sont facultatives. Si aucune **condition initiale** n'est définie pour une entité du modèle, la **condition d'environnement** est utilisée. Sur les entités portant une **condition aux limites explicite**, c'est la valeur imposée par la condition aux limites qui est utilisée.
