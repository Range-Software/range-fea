# Problème

**Problème** est un terme général désignant un ou plusieurs phénomènes physiques appliqués à une géométrie. Il est défini par le **type de problème**, la **configuration spécifique au problème**, les **conditions aux limites**, les **conditions initiales**, les **conditions d'environnement** et les **paramètres de matériau**.

Un problème est défini séparément pour chaque modèle. Ses différentes parties se règlent dans les onglets situés à droite de la fenêtre principale : **Problème**, **Conditions aux limites**, **Conditions initiales**, **Conditions d'environnement** et **Matériau**.

## Sélection d'un problème

**Menu :** _Problème -> Sélectionner le problème_

La boîte de dialogue **Sélectionner le problème** sert à choisir ce qui doit être résolu (voir [Type de problème](general_Problem_type.md)). Elle s'affiche aussi automatiquement lorsqu'un modèle sans problème est chargé. Tant qu'aucun problème n'est sélectionné, l'onglet **Problème** ne contient que le bouton **Sélectionner le problème**.

## Configuration spécifique au problème

Une fois le problème sélectionné, l'onglet **Problème** contient la configuration spécifique aux **types de problème** sélectionnés.

![Onglet Problème](image-Problem_tab.png)

- **Solveur temporel** – cochez pour résoudre le problème en régime **transitoire** (dépendant du temps). Une fois coché, on peut régler la méthode d'approximation, le temps initial, le pas de temps, le nombre de pas de temps et la fréquence de sortie ; le temps final en est déduit (_temps final = temps initial + pas de temps * nombre de pas de temps_). Sinon, le problème est résolu en régime **permanent**. Le solveur temporel n'est disponible que pour les types de problème qui le prennent en charge.
- Les autres sections dépendent des **types de problème** sélectionnés (p. ex. **Paramètres de l'analyse transitoire des contraintes**, paramètres de rayonnement, d'acoustique ou d'analyse modale).

D'autres actions liées au problème se trouvent dans le menu **Problème** : _Configurer le solveur matriciel des problèmes_, _Définir les points de surveillance_ et _Réinitialiser la configuration_.
