# Solveur

Le **solveur** est un outil qui traite les paramètres du **problème** et calcule les **résultats** sur un **modèle** donné.

Selon la taille du **modèle**, le **type de problème**, les paramètres de convergence et la configuration transitoire, le temps de résolution du **problème** peut aller de quelques secondes à plusieurs heures, voire davantage.

## Démarrage du solveur

**Menu :** _Solution -> Démarrer le solveur_

Le modèle doit être enregistré avant que le solveur puisse démarrer. S'il ne l'a pas encore été, la boîte de dialogue **Enregistrer le modèle** s'affiche d'abord. La boîte de dialogue **Démarrer le solveur** s'affiche ensuite.

![Démarrer le solveur](image-Start_solver.png)

Avant le démarrage du solveur, la configuration du problème est vérifiée. Les **Erreurs** (p. ex. une entité sans matériau) et les **Avertissements** trouvés sont listés dans la boîte de dialogue **Démarrer le solveur**. Si le modèle contient déjà des résultats, on peut cocher **Redémarrer le solveur / continuer** pour poursuivre à partir de ces résultats au lieu de repartir du début.

Le solveur s'exécute en arrière-plan dans un processus distinct, ce qui permet de continuer à travailler pendant le calcul. Toute sa sortie est affichée dans l'onglet **Sortie du processus**, sous la zone 3D. Une fois le solveur terminé, les **résultats** calculés sont chargés automatiquement dans le modèle.

## Arrêt du solveur

- _Solution -> Arrêter le solveur_ – demande à tous les solveurs en cours d'exécution de s'arrêter.
- _Solution -> Interrompre le solveur_ – termine immédiatement le processus du solveur.

## Suivi du solveur

- _Rapport -> Convergence du solveur_ – affiche un graphique de la convergence du solveur.
- _Rapport -> Points de surveillance_ – affiche les valeurs calculées aux points de surveillance définis par _Problème -> Définir les points de surveillance_.
- _Rapport -> Fichier journal du solveur_ – affiche le fichier journal du solveur.
