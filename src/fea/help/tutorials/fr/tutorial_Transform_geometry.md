# Transformer la géométrie

Ce tutoriel fait suite au tutoriel **Dessiner un cube** et montre comment mettre à l'échelle, faire pivoter et translater toute la géométrie ou seulement certaines de ses parties.

## 1. Charger le modèle

Si le modèle **Cube** n'est pas encore chargé, vous pouvez le faire avec l'action de menu **_Ouvrir un modèle_**.

**Menu :** _Fichier -> Ouvrir un modèle_

La fenêtre **Ouvrir un modèle** s'ouvre. Sélectionnez le fichier **Cube.rbm** et cliquez sur **Ouvrir**.

Une fois le modèle prêt, sa géométrie (entière ou en partie) peut être modifiée (transformée).

## 2. Préparation

**Menu :** _Géométrie -> Mettre à l'échelle, translater, pivoter_

Une fois l'élément de menu activé, le panneau **Transformer la géométrie** s'affiche sur le côté droit de la fenêtre principale, avec trois onglets : **Échelle**, **Pivoter** et **Translater**.

![Transformer la géométrie](image-Transform_geometry.png)

Dans ce tutoriel, toutes les transformations sont appliquées successivement à une face du cube.

Pour l'instant, toutes les faces du modèle **Cube** sont regroupées dans la même **entité surface**. Une face doit donc être **marquée** comme **entité surface** distincte, afin que les transformations s'appliquent à cette face seulement et non aux autres. Cela se fait avec l'action de menu **_Marquer la surface_**. Mais il faut d'abord pointer la face du cube.

## 3. Pointer une face

Maintenez la touche **_Ctrl_** (**_⌘ Cmd_** sous macOS) enfoncée et cliquez avec le **_bouton gauche de la souris_** sur une face du cube. L'élément pointé est mis en évidence. Comme le modèle **Cube** est composé de 6 éléments rectangulaires (quadrilatères), un élément correspond à une face du cube.

## 4. Marquer une surface

**Menu :** _Géométrie -> Surface -> Marquer la surface_

![Fenêtre Marquer la surface](image-Mark_surface_dialog.png)

Cette action ouvre la fenêtre **Marquer l'entité (surface)**. Comme un élément a été **pointé** à l'étape précédente, l'option **_Marquer uniquement les éléments sélectionnés et associés_** est présélectionnée. En cliquant sur **Ok**, une face du modèle **Cube** est marquée comme **entité surface** distincte.

_**Remarque :** toute entité peut être renommée par un **double-clic** du **bouton gauche de la souris** sur son nom dans l'**arbre du modèle**, sur le côté gauche de la fenêtre principale._

![Surface marquée](image-Marked_surface.png)

## 5. Mettre à l'échelle l'entité pointée (face du cube)

Pointez la surface marquée (**_Ctrl_** / **_⌘ Cmd_** + **_bouton gauche de la souris_**) et ouvrez le panneau **Transformer la géométrie**.

**Menu :** _Géométrie -> Mettre à l'échelle, translater, pivoter_

1. Cochez **Même échelle dans toutes les directions**.
2. Réglez la valeur **Échelle** sur 0.5 (utilisez le séparateur décimal de votre système, par ex. 0,5).
3. Cliquez sur le bouton **Définir à partir de l'élément/nœud pointé**.
4. Dans le groupe **Appliquer à**, choisissez **Entités pointées**.
5. Vérifiez que **Inclure les nœuds partagés** est coché, ainsi que **toutes** ses cases subordonnées.

La configuration correcte est visible sur la capture d'écran suivante.

![Transformer la géométrie – échelle](image-Transform_geometry-scale.png)

Cliquez sur **Ok**. Le résultat est visible sur la capture d'écran suivante.

![Surface mise à l'échelle](image-Scaled_surface.png)

## 6. Faire pivoter l'entité pointée (face du cube mise à l'échelle)

En faisant pivoter la surface pointée, on peut créer une surface extrudée. Ouvrez le panneau **Transformer la géométrie**.

**Menu :** _Géométrie -> Mettre à l'échelle, translater, pivoter_

Passez à l'onglet **Pivoter** et saisissez les valeurs comme sur la capture d'écran suivante :

1. Dans **Angles de rotation**, réglez **X** sur -90.
2. Cliquez sur le bouton **Définir à partir de l'élément/nœud pointé**, puis changez **Z** du **Centre de rotation** en 0.
3. Dans le groupe **Appliquer à**, choisissez **Entités pointées**.
4. Vérifiez que **Inclure les nœuds partagés**, **Détacher/séparer les nœuds partagés** et **Extruder les nœuds** sont cochés, et réglez **Pas d'extrusion** sur 6.

_Remarque : **Pas d'extrusion : 6** produit six nouveaux segments._

![Transformer la géométrie – rotation](image-Transform_geometry-rotate.png)

Cliquez sur **Ok**. Le résultat est visible sur la capture d'écran suivante.

![Surface pivotée](image-Rotated_surface.png)
