# Générer un maillage tétraédrique

Génère un maillage volumique (tétraédrique).


Un **maillage volumique** est nécessaire pour la plupart des **types de problèmes**.


Le générateur de maillage 3D ne peut générer un maillage que si :
1. le modèle contient une surface fermée (étanche),
2. les éléments de surface ne se coupent pas entre eux.


Pour générer plusieurs maillages volumiques représentant un modèle de matériau composite, une surface de séparation doit être incluse.


Dans certains cas, il peut être nécessaire de remplacer les paramètres par défaut et d'indiquer directement les paramètres de TetGen.

## Interface

![Fenêtre Générer le maillage](image-Generate_mesh_dialog.png)

- **Vérifier l'intégrité de la surface** – utile lors de la génération d'un maillage 3D à partir d'un modèle surfacique. Détecte les erreurs et les problèmes potentiels du maillage de surface.
- **Reconstruire le maillage (réutiliser les nœuds actuels)** – conserve les nœuds actuels et n'en ajoute de nouveaux qu'en cas de besoin.
- **Conserver les résultats calculés** – tous les résultats calculés sont projetés/interpolés sur les nouveaux nœuds et éléments.
- **Maillage de qualité** – génère un maillage 3D conforme aux critères de qualité.
    - **Volume maximal des éléments** – volume maximal de chaque élément généré. Aucun élément ne doit avoir un volume supérieur à celui indiqué.
    - **Générer une fonction de taille de maillage** – à utiliser pour raffiner le maillage aux endroits où les résultats varient fortement.
- **Paramètres TetGen** – paramètres transmis au générateur de maillage TetGen.

L'image suivante montre la différence de résolution du maillage lorsque le maillage adaptatif est utilisé.

![Exemple de maillage adaptatif](image-Adaptive_mesh-example.png)

## Syntaxe des paramètres

`pYrq_Aa_miO_S_T_XMwcdzfenvgkJBNEFICQVh`


Les traits de soulignement indiquent que certains commutateurs peuvent être suivis de nombres facultatifs. Ne laissez aucun espace entre un commutateur et son paramètre numérique.


Voici un aperçu de tous les commutateurs de la ligne de commande avec une brève description.

- **p** - Tétraédrise un complexe linéaire par morceaux (PLC).
- **r** - Reconstruit un maillage généré précédemment.
- **q** - Raffine le maillage (pour en améliorer la qualité).
- **R** - Déraffinement du maillage (pour réduire le nombre d'éléments).
- **a** - Applique une contrainte de volume maximal des tétraèdres.
- **O** - Indique le niveau d'optimisation du maillage.
- **S** - Indique le nombre maximal de points ajoutés.
- **T** - Définit une tolérance pour le test de coplanarité (10−8 par défaut).
- **X** - Désactive l'utilisation de l'arithmétique exacte.
- **M** - Pas de fusion des facettes coplanaires ni des sommets très proches.
- **w** - Génère une triangulation de Delaunay pondérée (régulière).
- **c** - Conserve l'enveloppe convexe du PLC.
- **d** - Détecte les auto-intersections des facettes du PLC.
- **n** - Fournit les voisins des tétraèdres.
- **C** - Vérifie la cohérence du maillage final.
- **Q** - Silencieux : aucune sortie sur le terminal sauf les erreurs.
- **V** - Détaillé : informations détaillées, davantage de sortie sur le terminal.
