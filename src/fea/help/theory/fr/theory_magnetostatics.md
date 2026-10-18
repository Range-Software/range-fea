# Magnétostatique - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur de magnétostatique (`RSolverMagnetostatics`) de
Range FEA, qui traite le type de problème **Magnétostatique**
(*Magneto-statics*) : la loi qu'il évalue, les données d'entrée qu'il accepte,
les parties de l'interface graphique qui le pilotent, un tutoriel complet et la
valeur de son résultat.

> **État.** Le solveur évalue directement, avec la **loi de Biot-Savart**, le
> champ magnétique du courant calculé par la tâche d'électrostatique. Il n'y a
> ni système d'équations ni condition aux limites : le champ est déterminé par le
> seul courant, et la condition selon laquelle il s'annule loin du conducteur est
> intégrée à la loi. Pour la densité de courant qui lui est fournie, le résultat
> est exact à une erreur de quadrature contrôlée près, bien inférieure à `1e-4`
> du champ maximal. Le solveur travaille dans **le vide** - les matériaux
> magnétiques ne peuvent pas être représentés - et ne calcule que le champ du
> courant modélisé ; le trajet du courant doit donc être fermé à l'intérieur du
> modèle. La section 5 liste les limitations.

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - le champ d'un conducteur parcouru par un courant](#3-tutoriel---le-champ-dun-conducteur-parcouru-par-un-courant)
4. [Vérifier un modèle](#4-vérifier-un-modèle)
5. [Limitations](#5-limitations)

---

## 1. Fondements théoriques

### 1.1 Le résultat - le champ magnétique

Le résultat est l'**induction magnétique** `B` en chaque nœud du modèle, en
tesla, sous forme de vecteur à trois composantes.

Elle est **évaluée, et non résolue**. Une fois la densité de courant connue, le
champ magnétique dans le vide s'en déduit par une intégrale ; le solveur
n'assemble donc aucune matrice et n'effectue aucune itération. Chaque nœud du
modèle reçoit une valeur, qu'il se trouve dans le conducteur, à sa surface ou
n'importe où ailleurs dans le maillage.

### 1.2 Loi fondamentale

Dans le vide, le champ magnétique statique satisfait le théorème d'Ampère et n'a
pas de sources :

```
curl(B) = u0 * J
div(B)  = 0
```

avec la condition que `B` s'annule loin de tous les courants. L'unique champ qui
satisfait ces trois conditions est donné par la **loi de Biot-Savart**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `B` | induction magnétique | `T` | calculée |
| `J` | densité de courant | `A/m^2` | de la tâche d'électrostatique, voir 1.4 |
| `u0` | perméabilité du vide | `H/m` | intégrée, `1.25663706212e-6` |
| `x` | point de calcul - un nœud du modèle | `m` | |
| `y` | point source - à l'intérieur d'un élément conducteur | `m` | |

Trois propriétés découlent de la loi elle-même et non de ce que fait le
solveur :

- **`div(B) = 0` est vérifiée exactement**, partout, quel que soit le maillage.
- **Le champ lointain est correct sans frontière.** `B` décroît avec la distance
  comme le prescrit la loi ; il n'y a ni surface extérieure, ni région d'air à
  mailler, ni condition à y définir.
- **`curl(B) = u0 * J` est vérifiée partout où le courant est fermé**,
  c'est-à-dire là où `div(J) = 0` et où le trajet du courant forme des boucles.
  La section 1.4 explique ce que cela signifie pour un modèle électrostatique,
  dont le courant entre dans le corps et en sort.

La perméabilité du matériau n'intervient nulle part. Il n'y a pas de propriété
de matériau de perméabilité relative, pas d'aimantation et pas de distinction
entre `B` et `H` - le solveur travaille dans le vide, quel que soit le matériau
du modèle. La section 1.9 explique pourquoi cette restriction ne peut pas être
levée avec la même approche.

**Aucune propriété de matériau n'est requise.** Le type de problème
magnétostatique n'en déclare aucune, et le contrôle de la configuration n'a rien
à vérifier pour lui. La tâche d'électrostatique dont dépend ce type de problème
a ses trois exigences propres - voir le manuel d'électrostatique.

### 1.3 Discrétisation

Le solveur d'électrostatique produit une densité de courant **constante dans
chaque élément**. L'intégrale de 1.2 se décompose donc en une intégrale par
élément, chacune avec un `J_e` constant qui peut être sorti de l'intégrale :

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` ne dépend que de la géométrie de l'élément et du point de calcul. Tous les
types d'éléments acceptés par le solveur ont des faces planes, et pour ceux-ci
`G_e` a une forme analytique ; le solveur intègre donc chaque élément
**exactement** et non de manière approchée :

- **Tétraèdre.** L'intégrande est le gradient de `1/|x - y|` par rapport à `y`,
  l'intégrale de volume devient donc une somme sur les quatre faces,
  `G = SUM_f n_f * INT_f 1/R dA`. L'intégrale de `1/R` sur un triangle plan est
  connue sous forme analytique : un logarithme le long de chaque arête et l'angle
  solide sous lequel le triangle est vu depuis `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Triangle** d'une entité surface d'épaisseur `t`. Le courant est un courant
  surfacique `K = J*t` en `A/m`. La partie tangentielle de `G` est de nouveau une
  somme de logarithmes d'arêtes, `SUM_e m_e * I_e`, et la partie normale est
  l'angle solide signé.
- **Quadrilatère.** Divisé en deux triangles.
- **Segment** d'une entité ligne de section `A`. Le courant vaut
  `I = A * (J . t)` le long du segment, et le champ d'un fil droit fini est
  classique :

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  `a` et `b` étant les vecteurs allant du point de calcul aux deux extrémités.

Ici, `I_e` est l'intégrale de `1/R` le long de l'arête `e`, `P_e` la distance
dans le plan du point de calcul à cette arête, `m_e` la normale sortante de
l'arête dans le plan, `d` la hauteur du point de calcul au-dessus du plan de la
face et `omega` l'angle solide de la face vu depuis le point de calcul.

Les **éléments éloignés** n'ont pas besoin de la forme analytique. L'intégrande
y est régulière, et une règle de quadrature est pratiquement exacte pour une
fraction du coût :

| Distance du point de calcul au centre de l'élément | Règle |
|---|---|
| moins de 2 x la plus longue arête de l'élément | forme analytique |
| 2 à 6 x | règle de degré deux - 3 points sur un triangle, 4 sur un tétraèdre |
| au-delà de 6 x | règle du point milieu - l'élément comme courant ponctuel |

Les segments utilisent toujours la forme analytique, aussi peu coûteuse qu'un
point de quadrature. L'erreur combinée des règles est mesurée par les tests
unitaires par rapport à la forme analytique sommée sur tous les éléments d'un
conducteur entier, et reste inférieure à `1e-4` du champ maximal en chaque
nœud - dans le conducteur, à sa surface et au-delà.

**Le champ sur le conducteur lui-même.**

- Pour un **courant volumique**, le champ est continu et fini partout, y compris
  aux nœuds, arêtes et faces des éléments conducteurs. Un nœud à la surface d'un
  conducteur massif reçoit le véritable champ de surface.
- Une entité **ligne** ou **surface** représente un conducteur de section nulle,
  et le champ d'un tel conducteur est singulier : il croît comme `1/r` vers un
  fil et de façon logarithmique vers le bord d'une nappe, et il subit un saut de
  `u0 * K x n` à travers une nappe. En un nœud situé sur un tel conducteur, le
  solveur renvoie une valeur finie : un segment n'apporte rien à un point situé
  sur sa propre droite, une nappe apporte la moyenne des champs de ses deux
  côtés, et le logarithme d'arête d'une nappe est omis en un nœud situé sur
  cette arête. Ces valeurs sont régularisées et non physiques. Lisez le champ
  d'un conducteur mince à quelques tailles de section de distance, là où le
  modèle mince est valable.

### 1.4 La source - la densité de courant

Le champ magnétique est piloté par une seule donnée : la **densité de courant**
laissée par une tâche *Electro-statics* placée plus tôt dans l'enchaînement des
tâches du problème. Il n'y a rien à saisir et rien à affecter.

Le solveur lit la densité de courant **aux éléments** stockée par la tâche
d'électrostatique et l'utilise telle quelle - il n'y a pas de moyenne aux nœuds.
Les éléments porteurs de courant sont :

| Entité | Condition | Source |
|---|---|---|
| Volume | tétraèdres | `J` |
| Surface | épaisseur `> 0` | courant surfacique `J * thickness` (`thickness` = épaisseur) |
| Ligne | section `> 0` | courant `J * cross area` (`cross area` = section) |

Les éléments dont la densité de courant est nulle sont ignorés, ce qui concerne
tous les éléments que la tâche d'électrostatique n'a pas calculés. Il en va de
même des éléments dont la densité de courant est inférieure à `1e-10` de la plus
grande du modèle : une région mal conductrice - de l'air d'une conductivité de
`5e-15 S/m` à côté du cuivre - ajouterait sinon une source par élément et rien
de mesurable au champ. Si aucun élément ne porte de courant - la tâche
d'électrostatique n'a pas été exécutée, les potentiels sont égaux, ou aucun
matériau n'a de conductivité électrique - le journal affiche un avertissement et
le champ est nul partout.

**Le courant doit se refermer.** La loi de Biot-Savart donne le champ du courant
qui lui est fourni, et rien d'autre. Un courant physique circule en boucles
fermées, mais un modèle électrostatique ne contient généralement qu'une partie
de la boucle : une barre entre deux électrodes, le courant entrant par une
extrémité et sortant par l'autre. Les conducteurs d'alimentation et le reste du
circuit ne figurent pas dans le modèle, ni leur champ. Le solveur calcule alors
le champ du conducteur modélisé comme si le courant apparaissait à une extrémité
et disparaissait à l'autre :

- **près du conducteur**, c'est généralement une excellente approximation, car le
  champ y est dominé par le courant voisin ;
- **loin de lui**, à des distances comparables à la taille du conducteur, la
  partie manquante de la boucle compte autant que la partie modélisée, et le
  résultat n'est le champ d'aucun circuit réel.

Lorsque le champ lointain compte, incluez le chemin de retour dans le modèle
électrostatique, de sorte que le courant forme une boucle. Un conducteur de
retour mince peut être représenté par une entité ligne avec une section.

### 1.5 Conditions aux limites, initiales et d'environnement

Il n'y en a **aucune**, et aucune n'est nécessaire.

| Type de condition | Proposé pour la magnétostatique |
|---|---|
| Conditions aux limites | aucune |
| Conditions initiales | aucune |
| Conditions d'environnement | aucune |
| Propriétés du matériau | aucune |

Le champ est entièrement déterminé par le courant et par l'exigence qu'il
s'annule à grande distance, que la loi de Biot-Savart porte en elle. Il n'y a
pas de frontière extérieure à fermer ni rien à y imposer.

Lorsqu'une tâche d'électrostatique et une tâche de magnétostatique partagent un
enchaînement de tâches - ce qui est toujours le cas, voir 1.6 - les onglets des
conditions proposent tout de même les conditions **électrostatiques**, car les
onglets sont remplis à partir des types de problèmes de tout l'enchaînement.
*Electric potential* et *Charge density* appartiennent à la tâche
d'électrostatique et agissent sur le champ magnétique par la densité de courant
qu'elles produisent.

### 1.6 Couplage avec d'autres solveurs

| Sens | Variable | Effet |
|---|---|---|
| Électrostatique -> magnétostatique | Current density | la seule source du champ magnétique |

**La magnétostatique nécessite l'électrostatique.** La dépendance est déclarée
dans le type de problème, et l'interface l'impose : cocher *Magneto-statics* dans
le sélecteur de problème coche automatiquement *Electro-statics*, et décocher
*Electro-statics* décoche aussi *Magneto-statics*. L'enchaînement des tâches par
défaut place la tâche d'électrostatique en premier, ce qui est l'ordre requis
par le couplage.

Rien n'utilise le champ magnétique. Il est en bout de chaîne - aucun solveur ne
le lit, et il n'y a ni force de Lorentz, ni courant induit, ni contribution
magnétique à une autre physique.

Comme le solveur se déclare convergé sans condition, un groupe de tâches
contenant une tâche de magnétostatique s'arrête toujours après une itération.

### 1.7 Évaluation du champ

Chaque élément porteur de courant contribue à chaque nœud ; le travail croît
donc comme le **nombre de nœuds multiplié par le nombre d'éléments porteurs de
courant**. L'évaluation s'exécute en parallèle sur les nœuds, traités par blocs,
de sorte que chaque élément est lu en mémoire une fois par bloc et non une fois
par nœud.

À titre indicatif, une barre de `120 000` tétraèdres évaluée en ses `24 000`
nœuds prend environ cinq secondes sur 14 threads. Doubler le maillage dans
chaque direction multiplie par huit le nombre de nœuds et celui des éléments, et
le temps par environ soixante-quatre ; un modèle de quelques centaines de
milliers d'éléments prend des minutes plutôt que des secondes.

Seuls les éléments porteurs de courant comptent. Un grand maillage autour d'un
petit conducteur - une région d'air, voir 2.8 - ajoute des nœuds mais pas de
sources.

La configuration du solveur matriciel n'est pas utilisée.

### 1.8 Résultats

| Résultat | S'applique à | Signification |
|---|---|---|
| Magnetic field `[T]` | nœud | le vecteur champ |

C'est toute la sortie. `process()` ne fait rien ; il n'y a donc pas de grandeur
dérivée - pas de norme du champ stockée séparément, pas de flux, pas de densité
d'énergie. La vue 3D peut afficher la norme et les composantes de la variable
vectorielle de la manière habituelle, et `Magnetic field in X direction` ainsi
que ses équivalents `Y` et `Z` sont disponibles pour l'affichage des
composantes.

Le champ magnétique est stocké comme variable **aux nœuds**, contrairement aux
résultats du champ électrostatique, qui sont des variables aux éléments.
**Chaque nœud** du modèle a une valeur, y compris les nœuds des entités qui ne
portent pas de courant ; un maillage autour du conducteur montre donc le champ
dans l'espace qui l'entoure.

Une fois l'enregistrement écrit, le journal du solveur affiche les statistiques
du champ magnétique et les valeurs aux éventuels points de surveillance.

### 1.9 Ce qu'est le solveur, et ce qu'il n'est pas

**C'est une intégrale, pas une équation de champ.** L'évaluation de Biot-Savart
ne nécessite ni frontière ni maillage hors du conducteur, et elle est exacte
pour le courant fourni. La contrepartie est qu'elle ne fonctionne que dans un
milieu **homogène**. Avec des matériaux magnétiques, le champ d'un courant
dépend de l'emplacement du fer, et aucune intégrale du seul courant ne peut
l'exprimer ; ce problème se résout par une formulation en potentiel -
généralement un potentiel vecteur magnétique sur des éléments d'arête - qui est
un autre solveur, et non une extension de celui-ci.

**Il est piloté par un courant de conduction calculé.** Il n'est pas possible
d'indiquer directement un courant, ni d'avoir un aimant permanent ou un champ
imposé ; le courant doit être produit par une résolution électrostatique sur un
corps conducteur, et sa précision détermine celle du résultat magnétique. Le
manuel d'électrostatique décrit ce qu'est et ce que n'est pas cette densité de
courant. Une densité de courant constante par élément est une approximation du
premier ordre de la densité réelle, et elle converge à mesure que le maillage est
raffiné.

**Il calcule le champ du courant modélisé.** Voir 1.4 - le courant doit se
refermer dans le modèle lorsque le champ loin du conducteur compte.

### 1.10 Analyse statique et solveur temporel

La magnétostatique est déclarée comme un type de problème **sans** solveur
temporel, exactement comme l'électrostatique. Le groupe *Solveur temporel*
n'apparaît pas dans l'onglet `Problème`, le calcul produit un seul
enregistrement, et il n'y a rien à configurer.

Comme un enchaînement de magnétostatique contient toujours aussi une tâche
d'électrostatique, et qu'aucune des deux ne prend en charge de solveur temporel,
l'ajout de l'une ou de l'autre à un enchaînement supprime le solveur temporel de
tout l'enchaînement. Une tâche thermique placée après elles ne peut être que
stationnaire. Le manuel d'électrostatique traite ce point dans sa propre
section 1.10.

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Matériau`, `Résultats`), le menu **Problème** et le dock **Modèle**. Les
onglets des conditions ne contiennent rien pour ce type de problème.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Ajoutez une tâche et choisissez
**Magneto-statics** - « Calculate magnetic field » (calculer le champ
magnétique).

**Electro-statics est coché automatiquement.** Le sélecteur impose la
dépendance de 1.6 dans les deux sens : cocher *Magneto-statics* coche
*Electro-statics*, et décocher *Electro-statics* décoche *Magneto-statics*.
L'enchaînement des tâches par défaut place la tâche d'électrostatique en
premier.

La magnétostatique se combine avec toutes les autres physiques sauf *Modal
analysis*. En pratique, la seule chaîne qui ait un sens est celle sur laquelle
elle repose :

- *Electro-statics* puis *Magneto-statics* - le champ magnétique d'un courant de
  conduction calculé.

### 2.2 Onglet Problème

Cet onglet est **vide** pour un enchaînement électrostatique plus
magnétostatique. Aucun des deux types de problèmes n'a de solveur temporel ni de
groupe de paramètres propre.

### 2.3 Onglet Matériau

La magnétostatique n'exige **rien** du matériau. La tâche d'électrostatique dont
elle dépend exige trois propriétés - permittivité relative, conductivité
électrique et masse volumique - et une entité dont le matériau n'a pas l'une
d'elles est ignorée par *cette* tâche, ce qui la laisse sans densité de courant
et donc sans contribution au champ magnétique.

Le matériau compte donc tout de même, entièrement par la moitié électrostatique
de la chaîne. Affectez un matériau possédant les trois, comme le matériau fourni
*Copper* (cuivre), et consultez le manuel d'électrostatique pour l'usage de
chacune.

Il n'y a pas de propriété de perméabilité relative à affecter. Le solveur
travaille dans le vide.

### 2.4 Onglet Conditions aux limites

Rien dans cet onglet n'appartient à la tâche de magnétostatique. Les conditions
qu'il propose - *Electric potential* et *Charge density* - sont celles de
l'électrostatique, et elles pilotent la densité de courant qui produit à son tour
le champ magnétique. Affectez-les comme le décrit le manuel d'électrostatique.

Aucune condition aux limites magnétostatique n'existe ni n'est nécessaire - voir
1.5.

### 2.5 Conditions initiales et d'environnement

Aucun de ces onglets ne propose quoi que ce soit pour la magnétostatique. La
condition initiale *Electric potential* et la condition d'environnement
*Temperature* affichées appartiennent à la tâche d'électrostatique.

### 2.6 Configuration du solveur matriciel

Le solveur de magnétostatique ne résout aucun système d'équations et n'utilise
pas la configuration du solveur matriciel. L'entrée **CG** ne configure que le
solveur d'électrostatique.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer. *Magnetic field* est
disponible dans la liste, avec les variables électrostatiques, et lit le champ
aux nœuds à l'emplacement de la sonde.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste **Magnetic field** comme vecteur aux nœuds, à côté
des résultats électrostatiques produits par la première tâche. L'onglet
`Enregistrements` du dock `Modèle` contient exactement un enregistrement : les
deux types de problèmes sont statiques.

`Rapport` -> `Fichier journal du solveur` affiche la sortie complète : d'abord
les statistiques de l'électrostatique, puis le nombre d'éléments porteurs de
courant et de nœuds pour lesquels l'intégrale de Biot-Savart a été évaluée, puis
les statistiques du champ magnétique.

**Voir le champ autour d'un conducteur.** Le champ est calculé en chaque nœud ;
tout ce qui est maillé autour du conducteur le montre donc. Maillez l'espace
environnant comme une entité volume distincte - une région d'air - et donnez-lui
un matériau sans conductivité électrique : la tâche d'électrostatique l'ignore,
elle ne porte donc pas de courant, mais ses nœuds reçoivent le champ magnétique.
La région coûte des nœuds mais pas de sources (1.7) et ne nécessite ni taille
particulière ni frontière, puisque rien n'y est résolu.

Le matériau fourni *Air* possède une conductivité électrique, très faible ; la
tâche d'électrostatique inclurait donc la région dans sa propre résolution.
Utilisez-en une copie dont la propriété de conductivité électrique a été
supprimée. Le contrôle de la configuration avertit alors qu'il manque au
matériau de l'entité des propriétés requises ; pour la région d'air, c'est
attendu, et le calcul se poursuit.

---

## 3. Tutoriel - le champ d'un conducteur parcouru par un courant

**Objectif.** Exécuter de bout en bout la chaîne électrostatique et
magnétostatique sur une barre de cuivre parcourue par un courant, vérifier le
champ magnétique par un calcul manuel, et voir où les approximations de 1.4 se
manifestent.

Ce tutoriel suppose la barre maillée du manuel d'électrostatique - un
parallélépipède de `0.1 m` de long à section carrée de `0.01 m`, maillé en
tétraèdres, avec les deux faces d'extrémité marquées comme entités surface
distinctes. Si ce modèle est encore configuré, seul l'enchaînement des tâches
change.

### Étape 1 - choisir les problèmes

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Cochez **Magneto-statics**. **Electro-statics** se coche de lui-même.
3. Confirmez avec `OK`.

L'enchaînement des tâches contient maintenant deux tâches, l'électrostatique en
premier. L'onglet `Problème` reste vide.

### Étape 2 - affecter le matériau

Sélectionnez l'entité volume et affectez **Copper** dans l'onglet `Matériau`, ou
un matériau portant les trois propriétés nécessaires à la tâche
d'électrostatique :

- **Relative permittivity** (permittivité relative) = `2.8`
- **Electrical conductivity** (conductivité électrique) = `5.882e7` `S/m`
- **Density** (masse volumique) = `8960` `kg/m^3`

Rien ici n'est lu par la tâche de magnétostatique. C'est la tâche
d'électrostatique qui en a besoin, et sans densité de courant il n'y a pas de
champ magnétique.

### Étape 3 - faire circuler un courant

1. Sélectionnez l'entité surface à une extrémité, ouvrez l'onglet
   `Conditions aux limites` et cochez **Electric potential**. Réglez-le sur
   `0` V.
2. Sélectionnez l'entité surface à l'autre extrémité, cochez **Electric
   potential** et réglez-le sur `0.01` V.

Le courant dans la barre vaut

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Une barre sans différence de potentiel ne porte aucun courant et donne un champ
magnétique exactement nul - c'est la première chose à vérifier si le résultat
est vide.

### Étape 4 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Suivez la progression dans `Rapport` -> `Fichier journal du solveur`.

Le journal affiche la tâche d'électrostatique avec ses variables, puis la tâche
de magnétostatique : le nombre d'éléments porteurs de courant, le nombre de
nœuds et les statistiques du champ magnétique. Les deux s'exécutent une fois.

### Étape 5 - examiner le résultat

- Onglet `Résultats` : affichez **Magnetic field**. C'est un vecteur aux nœuds ;
  la vue 3D propose donc sa norme et ses trois composantes.
- Le champ **circule autour de la barre**, perpendiculairement au courant, dans
  le sens de la règle de la main droite.
- Il est **nul sur l'axe** de la barre et croît vers la surface, où il est
  maximal - au milieu de chaque face longue plutôt qu'aux coins.
- Il est **plus faible près des extrémités** de la barre qu'en son milieu. C'est
  le trajet de courant ouvert de 1.4 : près d'une extrémité, il manque la moitié
  de ce qui serait un conducteur plus long.

### Étape 6 - vérifier l'ordre de grandeur

Pour un conducteur rond de rayon `R`, le champ de surface vaut
`u0*I/(2*pi*R)`. Une barre carrée de côté `a` n'est pas ronde, mais le milieu
d'une face, à une distance `a/2` de l'axe, en est proche : avec `a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

Le champ calculé au milieu d'une face longue, à mi-longueur de la barre, vaut
environ `0.020 T`, soit quelque 15 % de moins - la section carrée place une
grande partie du courant plus loin du point que ne le ferait une section ronde.
À un coin, à `a/sqrt(2)` de l'axe, il est encore plus faible, environ
`0.019 T`.

Une vérification plus fine nécessite un point de calcul éloigné de la barre, où
la section n'a plus d'importance. Ajoutez une région d'air comme décrit en 2.8
et lisez le champ à `r = 0.02 m` de l'axe, à mi-longueur. Pour un conducteur
droit fini de longueur `L`,

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

et la valeur calculée doit concorder à environ `0.1 %` près, la différence
venant de la section carrée de la barre. Cette valeur est le champ de la barre
seule, comme l'explique 1.4 - un circuit réel y ajouterait le champ de ses
conducteurs d'alimentation.

### Dépannage

| Symptôme | Cause |
|---|---|
| Le champ magnétique est nul partout et le journal avertit qu'aucun élément ne porte de courant | pas de densité de courant - vérifiez que la tâche d'électrostatique a été exécutée en premier, que les potentiels diffèrent et que le matériau possède une conductivité électrique |
| **Magneto-statics** ne peut pas être décoché seul | attendu - il nécessite *Electro-statics*, et décocher celui-ci décoche les deux |
| Le groupe *Solveur temporel* est absent | attendu - aucun des deux types de problèmes n'a de solveur temporel, voir 1.10 |
| Le champ est plus faible aux extrémités d'un conducteur | attendu - le trajet du courant est ouvert, voir 1.4 |
| Le champ loin du conducteur ne correspond pas au circuit | les conducteurs d'alimentation et le reste du circuit ne figurent pas dans le modèle, voir 1.4 |
| Le champ sur un conducteur ligne ou surface mince est erratique | attendu - le champ d'un conducteur de section nulle y est singulier, voir 1.3 |
| Le calcul est long | le coût est le nombre de nœuds multiplié par le nombre d'éléments porteurs de courant, voir 1.7 |

---

## 4. Vérifier un modèle

**Conducteur droit.** Pour un long conducteur de rayon `R` parcouru
uniformément par un courant total `I`, le champ est orthoradial et

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(`outside` = à l'extérieur, `inside` = à l'intérieur), avec `u0 = 1.2566e-6`.
Le champ est maximal à la surface du conducteur, `B = u0*I/(2*pi*R)`, et décroît
en `1/r` au-delà. Pour un courant de `1 A` dans un conducteur de `5 mm` de
rayon, ce maximum vaut `4e-5 T`, à peu près la grandeur du champ terrestre.

**Conducteur fini.** Pour un conducteur droit de longueur `L`, à une distance
`r` de son axe et à mi-longueur,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

qui tend vers la formule du conducteur long quand `L` croît. Loin du conducteur,
la forme de la section n'a plus d'importance ; toute section peut donc être
vérifiée avec cette formule.

**Fil.** Une entité ligne avec une section porte son courant le long des
segments, et le solveur utilise pour chacun d'eux la forme analytique d'un fil
droit. Une entité ligne droite reproduit donc exactement la formule du
conducteur fini, aux arrondis près, à toute distance.

**Solénoïde.** Pour un long solénoïde de `n` spires par mètre parcouru par `I`,
le champ intérieur est uniforme et dirigé selon l'axe,

```
B = u0 * n * I
```

et presque nul à l'extérieur. Un enroulement peut être modélisé par une entité
ligne qui suit les spires, avec la section du fil, et la tâche d'électrostatique
y fait circuler le courant à partir des potentiels imposés à ses deux
extrémités.

**Divergence.** Pour tout champ magnétique, `div(B) = 0` partout. La loi de
Biot-Savart la satisfait exactement ; elle ne teste donc pas ce solveur, elle ne
testerait que l'interpolation entre les nœuds.

**Convergence avec le maillage.** L'intégration elle-même est exacte (1.3) ; ce
qui dépend du maillage, c'est la densité de courant, que la tâche
d'électrostatique calcule constante par élément. Raffiner le maillage dans le
conducteur et comparer le champ au même point est la vérification de cette
partie.

---

## 5. Limitations

### Limitations de modélisation

- **Vide uniquement.** Aucune perméabilité n'intervient dans la formulation. Il
  n'y a pas de propriété de matériau de perméabilité relative, pas de
  distinction entre `B` et `H`, pas d'aimantation et pas de saturation ; le fer,
  les ferrites et les blindages ne peuvent donc pas du tout être représentés.
  Voir 1.9 pour comprendre pourquoi cela nécessite un autre solveur.
- **Uniquement le champ du courant modélisé.** Le champ des conducteurs
  d'alimentation et du reste du circuit n'est pas inclus. Lorsque le trajet du
  courant dans le modèle est ouvert, le champ loin du conducteur n'est pas celui
  d'un circuit réel - voir 1.4.
- **Pas d'aimants permanents ni de champ imposé.** La seule source est un
  courant de conduction calculé par une tâche d'électrostatique. Un aimant, un
  champ de fond et une excitation extérieure n'ont aucune représentation.
- **Aucun courant ne peut être indiqué directement.** Un courant est produit par
  une résolution électrostatique sur un corps conducteur. Une bobine peut être
  modélisée par une entité ligne qui suit l'enroulement, mais son courant reste
  défini par des potentiels.
- **Les conducteurs minces sont singuliers.** Le champ en un nœud d'une entité
  ligne ou sur le bord d'une entité surface est une valeur régularisée et non
  physique - voir 1.3.
- **Types d'éléments.** Les tétraèdres, triangles, quadrilatères et segments à
  deux nœuds portent du courant ; les autres types d'éléments n'apportent rien.
- **Coût.** Le travail croît comme le nombre de nœuds multiplié par le nombre
  d'éléments porteurs de courant - voir 1.7.
- **Pas de résultats dérivés.** `process()` ne calcule rien ; le vecteur champ
  magnétique constitue donc toute la sortie. Il n'y a ni flux, ni énergie, ni
  force, ni norme du champ stockée comme variable propre.
- **Rien n'utilise le résultat.** Aucun autre solveur ne lit le champ
  magnétique. Il n'y a ni force de Lorentz sur le conducteur, ni courant induit,
  ni couplage en retour vers la physique électrostatique, thermique ou
  structurelle.
- **Statique uniquement.** Il n'y a pas de solveur temporel, et l'ajout d'une
  tâche de magnétostatique - qui amène avec elle une tâche d'électrostatique -
  supprime le solveur temporel de tout l'enchaînement des tâches ; toute autre
  physique enchaînée après elle est donc elle aussi stationnaire.
