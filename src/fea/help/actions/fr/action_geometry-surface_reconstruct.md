# Reconstruire la surface

Cette action retriangule les surfaces sélectionnées. Elle fusionne les éléments coplanaires, supprime les nœuds qui ne définissent pas la forme (nœuds à l'intérieur des régions planes et sur les arêtes droites) et supprime les éléments minuscules et effilés.

Les nœuds existants ne sont jamais déplacés, seule l'option de maillage de qualité crée de nouveaux nœuds. Les nœuds et arêtes partagés avec d'autres entités (points, lignes, surfaces non sélectionnées) sont conservés, les surfaces fermées restent fermées et aucune nouvelle auto-intersection n'est créée. Seuls les triangles linéaires sont traités et le maillage volumique doit d'abord être supprimé.

* **Tolérance angulaire** - angle maximal entre les normales des éléments coplanaires et entre les directions des arêtes colinéaires.
* **Tolérance de distance** - distance maximale d'un nœud à un plan (une droite) pour être considéré comme coplanaire (colinéaire).
* **Longueur minimale d'arête** - les arêtes plus courtes sont fusionnées. Les détails plus petits que cette valeur peuvent disparaître.
* **Angle d'élément effilé** - les éléments dont le plus petit angle est inférieur à cette valeur sont considérés comme effilés.
* **Triangulation de Delaunay** - termine par une triangulation de Delaunay de la surface : les arêtes dont la somme des angles opposés dépasse 180 degrés sont basculées, à condition que la forme soit conservée dans la tolérance de distance.
* **Maillage de qualité (insertion de nœuds)** - insère de nouveaux nœuds (raffinement de Delaunay) afin que le plus petit angle des éléments soit au moins l'**Angle de qualité** et que les arêtes ne dépassent pas la **Longueur d'arête maximale** (zéro signifie sans limite). Les nouveaux nœuds sont placés sur la surface existante et sur les arêtes droites, la forme est donc conservée. Les arêtes partagées avec des entités non sélectionnées ne sont pas divisées. Les surfaces courbes constituées de facettes inclinées individuellement ne peuvent être améliorées que partiellement.

Les tolérances par défaut sont relatives à la taille des surfaces sélectionnées. Des tolérances plus grandes donnent des surfaces plus simplifiées.
