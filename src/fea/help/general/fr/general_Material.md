# Matériau

Un **matériau** est un ensemble de propriétés de matériau pertinentes pour un **type de problème** donné. Les valeurs de chaque propriété peuvent dépendre de la température.

## Affectation d'un matériau

1. Sélectionnez une ou plusieurs entités dans l'**arbre du modèle**.
2. Ouvrez l'onglet **Matériau** à droite de la fenêtre principale.
3. Cochez la case devant le nom du matériau pour l'affecter aux entités sélectionnées.

![Matériau](image-Material.png)

Les matériaux qui contiennent toutes les propriétés requises par les **types de problème** sélectionnés sont affichés en **gras**, tous les autres en _italique_.

_Remarque : un matériau doit être affecté à toutes les entités du modèle._

## Propriétés du matériau

Les propriétés du matériau sélectionné sont listées dans la partie inférieure de l'onglet **Matériau**. Une valeur peut être modifiée directement dans la colonne **Valeur**. Les boutons **Ajouter** et **Supprimer** permettent d'ajouter ou de supprimer une propriété.

Pour rendre une propriété dépendante de la température, développez-la et cliquez sur **Modifier les valeurs dépendant de la température** pour saisir un tableau de valeurs pour des températures données. Une propriété dont les valeurs dépendent de la température est grisée dans la colonne **Valeur** et ne peut être modifiée que dans ce tableau.

## Base de données des matériaux

La liste des matériaux est une base de données partagée par tous les modèles. Chaque matériau est enregistré dans son propre fichier, indiqué dans la colonne **Fichier**. Le bouton **Nouveau** crée un nouveau matériau et le bouton **Supprimer** supprime celui qui est sélectionné. Les modifications des propriétés d'un matériau sont enregistrées dans la base de données.
