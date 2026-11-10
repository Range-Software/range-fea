# Condition d'environnement

Une **condition d'environnement** est une condition qui peut être imposée à n'importe quelle entité du modèle, telle que Point, Ligne, Surface ou Volume. Une **condition d'environnement** définit des valeurs spécifiques au **problème** pour l'environnement considéré (p. ex. **Temperature** – température de l'environnement ou **Gravitational acceleration** – accélération de la pesanteur).

Les conditions d'environnement disponibles dépendent des **types de problème** sélectionnés :

- **Gravitational acceleration** – composantes dans les directions X, Y et Z, par défaut 0, 0 et -9.80665 `[m/s^2]`.
- **Temperature** – par défaut 293.15 `[K]`.

## Modification d'une condition d'environnement

Contrairement aux **conditions aux limites**, les conditions d'environnement ne se cochent pas. Elles sont toujours présentes et ont des valeurs par défaut.

1. Sélectionnez une ou plusieurs entités dans l'**arbre du modèle**.
2. Ouvrez l'onglet **Conditions d'environnement** à droite de la fenêtre principale.
3. Sélectionnez la condition d'environnement et modifiez les valeurs de ses propriétés dans la liste située en dessous.

![Conditions d'environnement](image-Environment_conditions.png)

Si aucune **condition initiale** n'est définie pour une entité du modèle, la **condition d'environnement** est utilisée.
