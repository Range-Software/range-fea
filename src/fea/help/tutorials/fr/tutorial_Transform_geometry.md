# Transformer la géométrie

Ce tutoriel fait suite au tutoriel **Dessiner un cube** et montre comment mettre à l'échelle, faire pivoter et translater toute la géométrie ou seulement certaines de ses parties.

## 1. Charger le modèle

Si le modèle **Cube** n'est pas encore chargé, utilisez l'action **_Ouvrir un modèle_**.

**Menu :** _Fichier -> Ouvrir un modèle_

La boîte de dialogue **Ouvrir un modèle** s'affiche. Sélectionnez le fichier **Cube.rbm** et cliquez sur **Open** (Ouvrir).

Une fois le modèle prêt, sa géométrie (en tout ou en partie) peut être modifiée (transformée).

## 2. Préparation

**Menu :** _Géométrie -> Mettre à l'échelle, translater, pivoter_

Une fois l'élément de menu activé, le panneau **Transformer la géométrie** s'affiche à droite de la fenêtre principale avec trois onglets : **Échelle**, **Pivoter** et **Translater**. Le groupe **Appliquer à** sous les onglets détermine quelle partie du modèle est transformée.

![Transformer la géométrie](image-Transform_geometry.png)

Dans ce tutoriel, toutes les transformations sont appliquées successivement à une face du cube. Fermez le panneau pour l'instant en cliquant sur **Annuler**.

Pour le moment, toutes les faces du modèle **Cube** sont regroupées dans la même **entité surfacique**. Il faut donc **marquer** une face comme **entité surfacique** distincte afin que les transformations ne s'appliquent qu'à cette face et non aux autres. C'est le rôle de l'action **_Marquer la surface_**. Mais il faut d'abord pointer la face du cube.

## 3. Pointer une face

Maintenez la touche **_Ctrl_** (**_⌘ Cmd_** sous macOS) enfoncée et cliquez avec le **_bouton gauche de la souris_** sur la face du cube orientée vers l'axe **Y**. L'élément pointé est mis en évidence et listé sous **Picked entities** dans l'encart en haut à gauche de la zone 3D. Comme le modèle **Cube** est composé de 6 éléments rectangulaires (quadrilatères), un élément correspond à une face du **Cube**.

## 4. Marquer une surface

**Menu :** _Géométrie -> Surface -> Marquer la surface_

![Boîte de dialogue Marquer la surface](image-Mark_surface_dialog.png)

Cette action affiche la boîte de dialogue **Mark entity (surface)**. Comme un élément a été **pointé** à l'étape précédente, l'option **_Marquer uniquement les éléments sélectionnés et associés_** est présélectionnée. En cliquant sur le bouton **Ok**, une face du modèle **Cube** est marquée comme **entité surfacique** distincte nommée **Surface 2**.

_**Remarque :** toute entité peut être renommée par un **double-clic** du **bouton gauche de la souris** sur son nom dans l'**arbre du modèle**, à gauche de la fenêtre principale._

![Surface marquée](image-Marked_surface.png)

## 5. Mettre à l'échelle l'entité pointée (face du cube)

Pointez la surface marquée (**_Ctrl_** / **_⌘ Cmd_** + **_bouton gauche de la souris_**) et ouvrez le panneau **Transformer la géométrie**.

**Menu :** _Géométrie -> Mettre à l'échelle, translater, pivoter_

1. Sur l'onglet **Échelle**, laissez **Même échelle dans toutes les directions** coché.
2. Réglez la valeur **Échelle** sur 0,5 (utilisez le séparateur décimal de votre système, p. ex. 0,5 ou 0.5).
3. Cliquez sur le bouton **Définir à partir de l'élément/nœud pointé**. Le **Centre de mise à l'échelle** est placé au centre de la face pointée (0,5 ; 1 ; 0,5).
4. Dans le groupe **Appliquer à**, sélectionnez **Entités pointées**.
5. Vérifiez que **Inclure les nœuds partagés** est coché ainsi que toutes ses cases filles.

Le réglage correct est visible sur la capture d'écran suivante.

![Transformer la géométrie – échelle](image-Transform_geometry-scale.png)

Cliquez sur **Ok**. La face pointée est réduite de moitié et l'espace entre la face réduite et le reste du cube est rempli par une nouvelle entité surfacique (**Surface 3**). Le résultat est visible sur la capture d'écran suivante.

![Surface mise à l'échelle](image-Scaled_surface.png)

## 6. Faire pivoter l'entité pointée (face réduite du cube)

En faisant pivoter la surface pointée, on peut créer une surface extrudée. Ouvrez le panneau **Transformer la géométrie**.

**Menu :** _Géométrie -> Mettre à l'échelle, translater, pivoter_

Passez à l'onglet **Pivoter** et saisissez les valeurs comme sur la capture d'écran suivante :

1. Réglez **Angles de rotation** **X** sur -90.
2. Cliquez sur le bouton **Définir à partir de l'élément/nœud pointé**, puis remplacez **Centre de rotation** **Z** par 0.
3. Dans le groupe **Appliquer à**, sélectionnez **Entités pointées**.
4. Vérifiez que **Inclure les nœuds partagés**, **Détacher/séparer les nœuds partagés** et **Extruder les nœuds** sont cochés et réglez **Pas d'extrusion** sur 6.

_Remarque : **Pas d'extrusion : 6** produit six nouveaux segments._

![Transformer la géométrie – rotation](image-Transform_geometry-rotate.png)

Cliquez sur **Ok**. Les segments extrudés sont ajoutés sous forme d'une nouvelle entité surfacique (**Surface 4**). Le résultat est visible sur la capture d'écran suivante.

![Surface pivotée](image-Rotated_surface.png)
