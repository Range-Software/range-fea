# Électrostatique - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur d'électrostatique (`RSolverElectrostatics`) de
Range FEA, qui traite le type de problème **Électrostatique**
(*Electro-statics*) : les équations qu'il résout, la signification de chaque
donnée d'entrée qu'il accepte, les parties de l'interface graphique qui le
pilotent, et deux tutoriels complets.

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - potentiel et courant dans un conducteur](#3-tutoriel---potentiel-et-courant-dans-un-conducteur)
4. [Tutoriel - échauffement résistif](#4-tutoriel---échauffement-résistif)
5. [Vérifier un modèle](#5-vérifier-un-modèle)
6. [Limitations](#6-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnue principale - le champ de potentiel électrique

L'inconnue principale est le **potentiel électrique** `V` en chaque nœud, en
volts. Chaque nœud porte un seul degré de liberté ; un maillage de `N` nœuds
donne donc un système de `N` équations. Le champ électrique, la densité de
courant et tout ce qui en dérive sont obtenus à partir du champ de potentiel
après la résolution.

Le potentiel est un scalaire, ce qui rend un modèle électrostatique aussi peu
coûteux qu'un modèle thermique sur le même maillage : une inconnue par nœud au
lieu de trois, et une matrice comptant un neuvième des termes d'un modèle
structurel.

Le potentiel par défaut d'un nœud qu'aucune condition ni aucun résultat
précédent n'atteint est de `0 V`.

Un champ de potentiel n'est défini **qu'à une constante additive près**. Rien
dans les équations ne fixe le niveau zéro ; chaque modèle a donc besoin d'au
moins une condition aux limites *Electric potential* - voir 1.7. C'est
l'équivalent électrostatique d'une structure non bridée, ou d'un modèle
thermique sans température imposée.

### 1.2 Équation fondamentale

Le solveur met en œuvre l'**équation du champ électrostatique** - l'équation de
Poisson pour le potentiel d'un diélectrique linéaire et isotrope :

```
div( e0 * er * grad(V) ) = -rho
```

et, en l'absence de toute densité de charge, elle se réduit à l'équation de
Laplace

```
div( e0 * er * grad(V) ) = 0
```

Le champ électrique est l'opposé du gradient du potentiel, et la densité de
courant découle de la loi d'Ohm :

```
E = -grad(V)
J = sigma * E
```

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `V` | potentiel électrique | `V` | inconnue résolue |
| `rho` | densité de charge | `C/m^3`, voir 1.5 | condition Charge density |
| `e0` | permittivité du vide | `F/m` | intégrée, `8.854187817e-12` |
| `er` | permittivité relative | `-` | propriété du matériau |
| `sigma` | conductivité électrique | `S/m` | propriété du matériau |
| `E` | champ électrique | `V/m` | obtenu, voir 1.8 |
| `J` | densité de courant | `A/m^2` | obtenue, voir 1.8 |

La permittivité du vide est une constante du solveur, ni une propriété de
matériau ni un paramètre - seule la permittivité **relative** est saisie avec le
matériau.

**Le champ est piloté par la permittivité, et non par la conductivité.** C'est
la chose la plus importante à savoir sur ce solveur. La matrice de rigidité est
construite à partir de `e0*er` seul ; la conductivité n'entre jamais dans le
système d'équations. Elle sert ensuite à transformer le champ calculé en densité
de courant, en résistivité et en chaleur Joule. La conséquence est détaillée en
1.9 : dans un corps fait d'un seul matériau, cela ne change rien, mais dans un
corps fait de **deux** matériaux, le potentiel se répartit proportionnellement
aux permittivités et non proportionnellement aux résistances.

**Propriétés de matériau requises.** Un problème électrostatique en requiert
**trois** :

| Propriété | Unités | Utilisée pour |
|---|---|---|
| Relative permittivity | `-` | la matrice de rigidité |
| Electrical conductivity | `S/m` | densité de courant, résistivité, chaleur Joule |
| Density | `kg/m^3` | rien - requise, jamais lue |

Une entité dont le matériau n'a pas l'une des trois **n'est pas résolue**, bien
que la masse volumique ne soit jamais lue par ce solveur. Le contrôle de la
configuration le signale avant le début du calcul. La seule exception est une
entité qui porte une condition aux limites électrostatique : cette entité est
incluse dans le système quel que soit son matériau, et sa permittivité et sa
conductivité valent alors simplement zéro.

Une permittivité nulle n'est pas une valeur par défaut inoffensive - un élément
avec `er = 0` n'a pas de rigidité ; il n'apporte donc rien au système et laisse
ses nœuds flottants. Si une entité doit être résolue, donnez-lui une vraie
permittivité : `1` pour le vide ou, à peu de chose près, pour l'air.

Chaque propriété est une table en fonction de la température, mais une tâche
d'électrostatique seule n'a pas de champ de température pour l'évaluer ; chaque
table est donc évaluée à `293.15 K`. Les tables ne sont prises en compte que
lorsqu'une tâche *Heat transfer* placée plus tôt dans le même enchaînement de
tâches a calculé des températures aux éléments, partagées avec les tâches qui
la suivent.

La permittivité relative est **sans dimension** - elle est affichée comme `N/A`
dans l'interface, comme l'émissivité et le coefficient de Poisson - et la valeur
est utilisée comme simple multiplicateur de `e0`. Saisissez `1` pour le vide,
`1.00059` pour l'air, `2.1` pour le PTFE, environ `4` pour le verre.

### 1.3 Discrétisation par éléments finis

La forme faible de l'équation du champ donne, pour chaque élément,

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(`stiffness` = rigidité, `charge source` = source de charge), où `B` est
construite à partir des dérivées des fonctions de forme et `N` sont les
fonctions de forme elles-mêmes. Chaque matrice élémentaire est `n x n` pour un
élément à `n` nœuds - une ligne par nœud, et non trois. Il n'y a pas de matrice
de masse ou de capacité : le problème est statique, et 1.10 explique ce que cela
signifie pour l'enchaînement des tâches.

**Mesures des éléments.** Comme ailleurs dans Range FEA, les entités de
dimension inférieure reçoivent une mesure géométrique afin de pouvoir
représenter des corps réels :

| Entité | Mesure | Où elle est définie |
|---|---|---|
| Volume | 1 | - |
| Surface | épaisseur de la surface | propriétés géométriques de l'entité |
| Ligne | section de la ligne | propriétés géométriques de l'entité |
| Point | volume du point | propriétés géométriques de l'entité |

Une **surface** porte le champ dans son propre plan, à travers une section égale
à son épaisseur. Il n'y a pas de gradient de potentiel dans l'épaisseur : un
élément surface a une épaisseur d'un seul nœud, il est donc par construction
équipotentiel dans son épaisseur. Une surface **d'épaisseur nulle** n'a donc
aucune rigidité et ne relie rien - contrairement au solveur thermique, où une
surface d'épaisseur nulle porte encore la convection, une surface d'épaisseur
nulle est ici simplement inerte. Donnez une épaisseur à une entité surface dès
qu'elle doit conduire, et n'oubliez pas qu'une face d'épaisseur nulle n'obtient
pas non plus de **résultats dérivés** (1.8).

Une **ligne** porte le champ selon sa propre direction, à travers sa section. Un
**point** n'a pas de direction selon laquelle le porter ; il n'apporte donc
**aucune rigidité** : il ne contient qu'une source de charge. Un élément point
dont le nœud n'est partagé avec aucun autre élément calculable produit donc une
ligne vide dans la matrice, que le solveur à gradient conjugué ne sait pas
traiter - un point sert à injecter une charge ou à imposer un potentiel, ce n'est
pas un conducteur.

### 1.4 Sources et conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Electric potential | explicite (Dirichlet) | point, ligne, surface, volume | Electric potential `[V]` |
| Charge density | naturelle, source | point, ligne, surface, volume | Charge density, voir 1.5 |

Ces deux conditions constituent tout le vocabulaire de ce type de problème. Il
n'y a ni flux imposé, ni charge surfacique, ni condition de champ lointain ou de
frontière ouverte, ni condition de symétrie - voir la section 6.

**Electric potential** (potentiel électrique) est la seule condition explicite
du problème électrostatique. Chaque nœud de l'entité à laquelle elle est
affectée est retiré du système d'équations et maintenu à la valeur donnée ; sa
contribution est reportée au second membre des équations voisines. Chaque modèle
en a besoin d'au moins une. Une condition nouvellement affectée commence à
`1 V`.

Là où des entités se chevauchent, **l'entité de dimension inférieure
l'emporte** : un potentiel imposé sur une surface remplace, aux nœuds communs,
celui imposé sur le volume situé derrière elle ; une ligne l'emporte sur une
surface, et un point sur tout le reste. Les potentiels ne sont pas combinés - il
n'y a qu'un degré de liberté par nœud à maintenir, la dernière condition lue
prévaut donc simplement.

**Charge density** (densité de charge) est le terme source de l'équation de
Poisson. Contrairement à la condition *Heat* du solveur thermique, sa valeur
n'est **pas** répartie sur la mesure de l'entité : le nombre saisi est appliqué
tel quel à chaque élément de l'entité, comme une densité. La charge totale
placée dans une entité volume est donc la valeur multipliée par le volume de
cette entité, et elle augmente si l'entité est agrandie - elle ne reste pas fixe
comme le fait une valeur *Heat* en watts.

| Entité | Signification de la valeur |
|---|---|
| Volume | charge par unité de volume |
| Surface | charge par unité de volume de la section pondérée par l'épaisseur |
| Ligne | charge par unité de volume de la section |
| Point | une source de charge nodale |

Les deux composantes acceptent une table en fonction du temps. Comme le problème
lui-même est statique, un historique temporel n'a d'effet que si une autre tâche,
dépendant du temps, partage l'enchaînement des tâches - et, comme l'explique
1.10, cette combinaison est elle-même restreinte.

### 1.5 La valeur de la densité de charge

La composante *Charge density* est une **densité volumique de charge** en
`C/m^3`, comme l'indique son étiquette dans l'interface. C'est une densité et
non un total, ce qui la rend indépendante du maillage - et, comme le note 1.4, ce
qui la distingue de la condition *Heat* du solveur thermique, qui prend un total
en watts et le répartit pour vous.

Le signe suit la convention habituelle. L'intégration par parties de
`div(e0*er*grad(V)) = -rho` laisse la charge au second membre avec un signe
positif,

```
every entity type      f = +INT( N^T * rho )
```

(`every entity type` = tout type d'entité), si bien qu'une densité de charge
**positive** élève le potentiel autour d'elle et qu'une densité négative
l'abaisse, sur les entités volume, surface, ligne et point indifféremment.

N'oubliez pas qu'une source de charge seule ne suffit pas à rendre un modèle
soluble. Le potentiel reste défini à une constante près tant que rien ne le
fixe ; un modèle piloté par des charges a donc aussi besoin d'au moins une
condition *Electric potential* - sur la frontière extérieure de la région
maillée, à défaut d'ailleurs. Voir 1.7.

### 1.6 Conditions d'environnement, initiales et de couplage

#### Conditions d'environnement

| Condition d'environnement | Composantes | Effet |
|---|---|---|
| Temperature | Temperature `[K]` | proposée, mais non lue - voir ci-dessous |

La condition d'environnement *Temperature* est proposée pour un problème
électrostatique parce que les propriétés des matériaux sont des tables en
fonction de la température. Le solveur d'électrostatique ne la lit **pas** : il
ne prend les températures aux éléments que dans les données partagées laissées
par une tâche *Heat transfer*, et évalue sinon chaque table à `293.15 K`.
L'affecter à un modèle purement électrostatique ne change rien.

#### Conditions initiales

| Condition initiale | Effet |
|---|---|
| Electric potential | champ de potentiel initial, appliqué au premier calcul |

Le potentiel initial n'est lu **qu'au premier calcul**. Comme la résolution est
directe et non une marche en temps, il ne modifie pas la réponse convergée - il
ne fixe que le point de départ de l'itération du gradient conjugué et la valeur
de tout nœud que le solveur exclut du système.

Les conditions d'environnement, les conditions initiales et les conditions aux
limites sont lues dans cet ordre et chacune écrase la précédente ; ainsi, **une
condition aux limites l'emporte sur une condition initiale, qui l'emporte sur
une condition d'environnement**.

#### Couplage avec d'autres solveurs

Le solveur d'électrostatique produit deux résultats utilisés par d'autres
solveurs.

| Sens | Variable | Effet |
|---|---|---|
| Électrostatique -> thermique | Joule heat | ajoutée au terme source de chaque élément |
| Électrostatique -> thermique des fluides | Joule heat | ajoutée au terme source de chaque élément |
| Électrostatique -> magnétostatique | Current density | source du champ magnétique |
| Thermique -> électrostatique | Temperature | sélectionne la ligne de chaque table de matériau dépendant de la température |

La **chaleur Joule** est reprise par une tâche *Heat transfer* placée **après**
la tâche d'électrostatique dans l'enchaînement des tâches du problème, où elle
est ajoutée au terme source des éléments exactement comme le serait une
condition *Heat rate (unit volume)*. L'échauffement résistif ne nécessite donc
aucune configuration au-delà de l'ordre des deux tâches - le tutoriel 4 le
détaille.

La **magnétostatique** est un type de problème distinct qui *nécessite* une
tâche d'électrostatique : elle lit la densité de courant aux éléments stockée
par ce solveur - sur les entités volume, surface et ligne indifféremment - et
évalue le champ magnétique de ce courant avec la loi de Biot-Savart. Ajoutez
d'abord la tâche d'électrostatique.

Comme le solveur se déclare convergé sans condition, un groupe de tâches qui ne
contient qu'une tâche d'électrostatique s'arrête toujours après une itération.
Itérer n'a d'intérêt que lorsqu'une grandeur dont dépend la résolution est
elle-même décalée - une permittivité fortement dépendante de la température
alimentée par une tâche thermique, par exemple.

### 1.7 Résolution du système

Le solveur assemble et résout

```
K * V = f
```

`K` est symétrique définie positive dès qu'au moins un potentiel a été imposé ;
elle est donc résolue par la méthode du **gradient conjugué** avec un
préconditionnement de Jacobi. L'assemblage est parallélisé sur les éléments,
avec des tampons par thread fusionnés une seule fois à la fin, si bien que le
résultat ne dépend pas du nombre de threads.

Un modèle **sans condition aux limites Electric potential** est singulier : le
potentiel n'est défini qu'à une constante près, la matrice a un noyau non
trivial et la résolution ne convergera pas. Chaque modèle a besoin d'au moins un
potentiel imposé, et un modèle destiné à porter un courant en a besoin de deux,
de valeurs différentes.

Trois autres façons d'aboutir à un système singulier ou vide méritent d'être
connues, car toutes trois sont silencieuses :

- une entité dont le matériau n'a pas l'une des trois propriétés requises n'est
  pas résolue, et ses nœuds sortent du système ;
- une entité avec `er = 0` n'a pas de rigidité ; ses nœuds restent donc dans le
  système mais ne sont reliés à rien ;
- une surface d'épaisseur nulle, ou une ligne de section nulle, n'a pas non plus
  de rigidité.

### 1.8 Résultats dérivés

Après la résolution, les résultats aux éléments sont obtenus à partir du champ
de potentiel aux nœuds. Le champ électrique est l'opposé du gradient du
potentiel, moyenné sur les points d'intégration et tourné dans le repère global
pour les éléments lignes et surfaces :

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`electric energy` = énergie électrique, `electrical resistivity` = résistivité
électrique, `Joule heat` = chaleur Joule.)

| Résultat | S'applique à | Signification |
|---|---|---|
| Electric potential `[V]` | nœud | le champ de potentiel résolu |
| Electric field `[V/m]` | élément | vecteur champ |
| Current density `[A/m^2]` | élément | `sigma*E`, nulle là où la conductivité est nulle |
| Electric energy `[J/m^3]` | élément | densité d'énergie du champ |
| Electrical resistivity `[V*m/A]` | élément | `1/sigma` partout où un courant circule, `0` ailleurs |
| Joule heat `[W/m^3]` | élément | dissipation résistive, transmise au solveur thermique |

Ce sont toutes des **densités** : les dérivées des fonctions de forme sont
moyennées sur les points d'intégration de l'élément et ne sont pas pondérées par
son déterminant jacobien, et l'épaisseur de surface et la section de ligne
restent dans la rigidité, où elles ont leur place, au lieu d'entrer dans le
champ obtenu. Le champ, la densité de courant et l'énergie électrique sont donc
indépendants de la taille de l'élément dans lequel ils ont été calculés ; on
peut ainsi les comparer d'un élément à l'autre et avec un calcul manuel, et ils
convergent à mesure que le maillage est raffiné.

La **résistivité électrique** est le résultat à consulter pour vérifier un
modèle, car les deux facteurs dont elle est construite se compensent :
`|E|/|sigma*E|` vaut exactement `1/sigma`, quoi que fasse le champ. C'est la
confirmation la plus rapide que la conductivité prévue pour une entité a bien
atteint ses éléments. Elle est stockée à `0` sur tout élément où la densité de
courant passe sous la tolérance numérique, ce qui inclut tout élément de
conductivité nulle.

La **chaleur Joule** est la densité de dissipation `sigma*|E|^2`, en `W/m^3`.
C'est exactement ce qu'attend une tâche *Heat transfer* lorsqu'elle ajoute la
valeur à son terme source et l'intègre sur l'élément ; la puissance transmise à
un calcul thermique couplé est donc la puissance réellement dissipée par le
modèle. Comme les autres résultats, c'est une densité ; elle est donc
indépendante de la taille des éléments et converge par raffinement.

Deux autres cas où les résultats dérivés restent simplement à zéro : les
**éléments points** ne sont jamais post-traités, et une **surface d'épaisseur
nulle** ou une **ligne de section nulle** est ignorée ; les deux gardent donc un
champ nul, quelle que soit l'importance du gradient de potentiel qui les
traverse.

Une fois l'enregistrement écrit, le journal du solveur affiche les statistiques
des six variables et les valeurs aux éventuels points de surveillance.

### 1.9 Ce qu'est le solveur, et ce qu'il n'est pas

Le nom est exact : c'est un solveur **électrostatique**. Il calcule le champ
d'une répartition de charges et de potentiels dans un diélectrique. La densité
de courant qu'il indique est ce que donne la loi d'Ohm pour ce champ - une
grandeur dérivée, calculée après coup, qui n'influence jamais le champ
lui-même.

Un problème de **courant continu**, en revanche, est régi par

```
div( sigma * grad(V) ) = 0
```

avec la conductivité à la place où ce solveur met la permittivité. Les deux
donnent le même champ de potentiel dans un corps homogène, car tous deux se
réduisent à l'équation de Laplace et la constante se simplifie. Ils divergent
dès que le corps est fait de plus d'un matériau :

| | Ce solveur | Un solveur de courant continu |
|---|---|---|
| Le potentiel se répartit proportionnellement à | `1/er` de chaque région | `1/sigma` de chaque région |
| Adapté à | un condensateur, un diélectrique, un isolant | un réseau de résistances, une barre omnibus, un chemin conducteur |

Un conducteur d'un seul matériau avec des potentiels imposés à ses extrémités
est donc modélisé correctement : le potentiel est linéaire, et le diviser par la
longueur donne le champ. Deux conducteurs de conductivités différentes en série
ne le sont pas - la répartition suit les permittivités, ce qui n'est
probablement pas ce qui était voulu. Modélisez un tel cas avec un seul
matériau, ou calculez la répartition à la main.

### 1.10 Analyse statique et solveur temporel

L'électrostatique est déclarée comme un type de problème **sans** solveur
temporel. Cela a deux conséquences visibles.

Dans un modèle purement électrostatique, le groupe *Solveur temporel*
n'apparaît pas du tout dans l'onglet `Problème`, le calcul produit un seul
enregistrement, et il n'y a rien à configurer.

Plus surprenant, ajouter une tâche d'électrostatique à un enchaînement de tâches
**désactive le solveur temporel pour tout l'enchaînement**. Le solveur temporel
n'est proposé que si *tous* les types de problèmes de l'enchaînement en
prennent un en charge ; une analyse thermique transitoire à laquelle on ajoute
une tâche d'électrostatique devient donc une analyse stationnaire, sans autre
avertissement que la disparition du groupe. Si une réponse thermique transitoire
est nécessaire pour un élément chauffant résistif, exécutez la tâche
d'électrostatique séparément et reportez sa dissipation dans le modèle thermique
transitoire sous forme de condition *Heat*, plutôt que de placer les deux tâches
dans un même enchaînement.

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`,
`Conditions d'environnement`, `Matériau`, `Résultats`), le menu **Problème**
et le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Ajoutez une tâche et choisissez
**Electro-statics** - « Calculate electric field, current density and generated
Joule's heat » (calculer le champ électrique, la densité de courant et la
chaleur Joule produite).

L'électrostatique se combine librement avec toutes les autres physiques sauf
*Modal analysis*. Les chaînes à connaître sont :

- *Electro-statics* puis *Heat transfer* - échauffement résistif ;
- *Electro-statics* puis *Magneto-statics* - le champ magnétique du courant
  calculé, seule manière d'effectuer une analyse magnétostatique ;
- *Electro-statics* puis *Heat transfer* puis *Stress analysis* - contraintes
  thermiques dues à l'échauffement résistif.

Rappelez-vous 1.10 : tout enchaînement contenant une tâche d'électrostatique est
résolu en stationnaire.

### 2.2 Onglet Problème

Pour un modèle purement électrostatique, cet onglet est **vide**. Il n'y a pas
de solveur temporel, car ce type de problème n'en prend pas en charge, et
l'électrostatique n'a pas de groupe de paramètres propre. Tout s'affecte par
entité dans les onglets des conditions.

Si l'onglet affiche un groupe *Solveur temporel*, c'est une autre tâche de
l'enchaînement qui l'a placé là - et, d'après 1.10, il disparaîtra de nouveau
dès que la tâche d'électrostatique sera ajoutée.

### 2.3 Onglet Matériau

L'onglet `Matériau` liste le matériau affecté à l'entité sélectionnée.
L'électrostatique requiert **ces trois** propriétés :

| Propriété | Unités | Commentaire |
|---|---|---|
| Relative permittivity | `N/A`, sans dimension | `er`, `1` pour le vide, `1.00059` pour l'air, environ `4` pour le verre |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` pour le cuivre, `5e-15` pour l'air |
| Density | `kg/m^3` | requise pour que l'entité soit résolue, jamais lue |

Une entité à laquelle il en manque une n'est pas résolue, et le contrôle de la
configuration avertit avant le début du calcul par *Entity ... has material
assigned which is missing required properties* (le matériau affecté à l'entité
n'a pas toutes les propriétés requises). Comme la masse volumique n'est jamais
utilisée par ce solveur, n'importe quelle valeur positive satisfait l'exigence.

La bibliothèque de matériaux fournie possède déjà les trois pour les métaux,
pour l'air et pour les verres ; affecter *Copper* ou *Air* suffit donc pour
commencer.

Chaque propriété est une table en fonction de la température, mais la
dépendance à la température n'est prise en compte que lorsqu'une tâche
thermique placée plus tôt dans l'enchaînement a produit des températures aux
éléments - voir 1.6.

### 2.4 Onglet Conditions aux limites

L'onglet est divisé en une liste de gestion des conditions aux limites
disponibles en haut et un éditeur de la condition sélectionnée en bas.
Sélectionnez d'abord une entité dans l'arbre `Modèle` ; le gestionnaire propose
alors les conditions valables pour ce type d'entité et pour le problème
sélectionné. Cochez-en une pour l'affecter, puis modifiez ses composantes dans
l'arbre inférieur.

Seules les deux conditions de la section 1.4 sont proposées. Remarques sur leur
utilisation :

- **Electric potential** est la condition qui maintient une valeur au lieu de la
  piloter, et elle est proposée pour tous les types d'entités. Affectez-la à la
  face, l'arête ou le point dont le potentiel est connu. Un modèle sans aucune
  de ces conditions ne se résoudra pas.
- **Charge density** est le terme source, et une valeur positive élève le
  potentiel autour d'elle. Lisez 1.5 avant de l'utiliser : la valeur est une
  densité et, contrairement à la condition *Heat* du solveur thermique, elle
  n'est pas répartie sur l'entité.
- Les deux composantes sont des tables en fonction du temps, ce qui ne compte
  que dans un enchaînement de tâches piloté par une autre tâche dépendant du
  temps.

### 2.5 Conditions initiales et d'environnement

L'onglet `Conditions initiales` propose *Electric potential*, qui définit le
champ de départ de la résolution itérative. Elle ne modifie pas la réponse
convergée.

L'onglet `Conditions d'environnement` propose *Temperature*, que ce solveur
ignore - voir 1.6.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur d'électrostatique utilise l'entrée **CG**.

Si la résolution ne converge pas, la cause habituelle est un modèle sans
potentiel imposé, ou une entité de permittivité nulle, plutôt qu'un paramètre du
solveur. Vérifiez les conditions aux limites et le matériau avant d'augmenter le
nombre d'itérations.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer - pour un problème
électrostatique, *Electric potential* est le choix habituel, et c'est la
manière la plus rapide de lire une valeur sur un modèle sous forme de nombre
plutôt que de couleur. `Rapport` -> `Points de surveillance` affiche les
valeurs.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste les variables calculées et contrôle la vue 3D. Le
potentiel électrique est un scalaire aux nœuds ; le champ électrique et la
densité de courant sont des vecteurs aux éléments ; l'énergie électrique, la
résistivité électrique et la chaleur Joule sont des scalaires aux éléments.

L'onglet `Enregistrements` du dock `Modèle` liste les enregistrements de
résultats. Une analyse électrostatique est statique ; il y en a donc exactement
**un**.

`Rapport` -> `Fichier journal du solveur` affiche la sortie complète du solveur,
y compris les statistiques des six variables après l'écriture de
l'enregistrement.

---

## 3. Tutoriel - potentiel et courant dans un conducteur

**Objectif.** Calculer la répartition du potentiel le long d'une barre de cuivre
maintenue à deux potentiels différents, et en déduire le champ, le courant et la
résistance.

Ce tutoriel suppose un modèle volumique maillé - un parallélépipède allongé
convient bien. Utilisez `Fichier` -> `Nouveau modèle`, dessinez un
parallélépipède avec `Géométrie` -> `Créer un élément` ou importez une
géométrie, et maillez-le en tétraèdres ; le tutoriel *Dessiner un cube* couvre ce
sujet. Marquez les deux faces d'extrémité comme entités surface distinctes afin
de pouvoir leur affecter des conditions.

Prenez une barre de `0.1 m` de long avec une section de `0.01 m` sur `0.01 m`,
ce qui rend les calculs manuels de la fin faciles à suivre.

### Étape 1 - choisir le problème

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez une tâche et sélectionnez **Electro-statics**.
3. Confirmez avec `OK`.

L'onglet `Problème` reste vide - il n'y a rien à configurer.

### Étape 2 - affecter le matériau

1. Sélectionnez l'entité volume dans l'arbre `Modèle`.
2. Ouvrez l'onglet `Matériau` et affectez **Copper** (cuivre), ou un matériau
   possédant les trois propriétés :
   - **Relative permittivity** (permittivité relative) = `2.8`
   - **Electrical conductivity** (conductivité électrique) = `5.882e7` `S/m`
   - **Density** (masse volumique) = `8960` `kg/m^3`

La masse volumique n'est pas utilisée par ce solveur, mais elle doit être
présente pour que l'entité soit résolue. C'est la permittivité qui pilote le
champ ; la conductivité ne fait que transformer ensuite le champ en courant.

### Étape 3 - imposer les deux extrémités

1. Sélectionnez l'entité surface à une extrémité.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Electric potential**.
3. Réglez la valeur sur `0` V - c'est la masse.
4. Sélectionnez l'entité surface à l'autre extrémité, cochez de nouveau
   **Electric potential** et réglez-la sur `10` V.

C'est l'étape qui rend le modèle soluble. Un seul potentiel imposé est le
minimum ; deux potentiels de valeurs différentes créent un champ.

N'ajoutez pas de condition *Charge density*. Un conducteur avec des potentiels
imposés aux extrémités ne porte pas de charge d'espace ; le terme source doit
donc rester nul et le potentiel est une droite entre les deux extrémités - ce
qui rend exacte la vérification manuelle de l'étape 6.

### Étape 4 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Le contrôle de la configuration signale les matériaux manquants avant le
   début du calcul.
3. Suivez la progression dans `Rapport` -> `Fichier journal du solveur`.

Le calcul est une seule résolution par gradient conjugué et se termine en
quelques secondes sur un maillage de cette taille.

### Étape 5 - examiner les résultats

- Onglet `Résultats` : affichez **Electric potential**. Il doit décroître
  linéairement de `10` V à `0` V le long de la barre, avec des plans
  équipotentiels plans perpendiculaires à son axe. Toute courbure dans une barre
  homogène signifie que le maillage n'est pas connecté comme on le suppose.
- **Electrical resistivity** doit indiquer `1/5.882e7 = 1.7e-8` `V*m/A` sur
  chaque élément de la barre. C'est la meilleure vérification unique que le
  matériau a atteint les éléments : elle est exacte, indépendante du maillage,
  et fausse dès que la conductivité l'est.
- **Electric field** est dirigé le long de la barre et doit indiquer
  uniformément `10/0.1 = 100` `V/m`, et **Current density**
  `sigma*E = 5.88e9` `A/m^2`. Les deux sont des densités ; les valeurs sont donc
  valables quelle que soit la taille des éléments - l'étape 6 les établit.
- Un point de surveillance sur *Electric potential* à mi-longueur de la barre
  doit indiquer `5` V.

### Étape 6 - faire le calcul à la main

Les deux premières lignes ci-dessous sont ce que les résultats **Electric
field** et **Current density** doivent déjà afficher ; elles constituent donc
une vérification directe de la résolution. Les trois dernières sont des totaux
et non des densités, qu'aucun résultat aux éléments ne fournit, et doivent être
calculées. Pour une barre homogène de longueur `L` et de section `A` soumise à
une différence de potentiel `U` :

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

Le courant est énorme, car une barre de cuivre massive de cette section est un
très bon conducteur et `10` V à ses bornes constituent un court-circuit - un
rappel utile qu'un modèle électrostatique calculera sans broncher une situation
qui vaporiserait la pièce réelle.

### Étape 7 - facultatif, raffiner le maillage

Raffinez le maillage une fois et résolvez de nouveau. Chaque résultat - le
**potentiel**, le **champ électrique**, la **densité de courant**, l'**énergie
électrique**, la **résistivité** et la **chaleur Joule** - doit conserver sa
valeur. Ce sont toutes des densités ; une valeur qui dérive avec le raffinement
signale donc un problème de modèle plutôt qu'un maillage plus fin, et il vaut la
peine d'en chercher la cause avant d'aller plus loin.

### Dépannage

| Symptôme | Cause |
|---|---|
| La résolution ne converge pas | aucune condition *Electric potential* nulle part, le potentiel n'est donc défini qu'à une constante près |
| *Entity has material assigned which is missing required properties* | l'une des trois propriétés requises est absente - souvent la masse volumique, que le solveur n'utilise jamais |
| Tout le modèle est à un seul potentiel | un seul potentiel a été imposé, ou le second a la même valeur |
| Le potentiel est nul partout sauf sur les faces imposées | l'entité a `er = 0`, elle n'a donc pas de rigidité et ne relie rien |
| La résistivité est nulle partout | la conductivité électrique est nulle, aucun courant ne circule et la résistivité est stockée à zéro |
| Une entité surface n'affiche aucun champ | son épaisseur est nulle, elle n'est donc ni assemblée ni post-traitée |

---

## 4. Tutoriel - échauffement résistif

**Objectif.** Prendre la chaleur Joule calculée à partir du champ électrique et
l'utiliser pour chauffer la barre, en enchaînant une tâche d'électrostatique et
une tâche thermique.

### Étape 1 - construire l'enchaînement des tâches

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez une tâche **Electro-statics**.
3. Ajoutez une tâche **Heat transfer** **après** celle-ci.
4. Confirmez avec `OK`.

L'ordre compte : le solveur thermique lit la chaleur Joule laissée par le
solveur d'électrostatique ; la tâche d'électrostatique doit donc s'exécuter en
premier. Aucune condition ni aucun paramètre n'est nécessaire pour les relier.

Observez ce qui arrive à l'onglet `Problème` : le groupe *Solveur temporel*
qu'afficherait une tâche thermique seule a disparu, car l'électrostatique n'a
pas de solveur temporel et l'enchaînement adopte le réglage le plus restrictif -
voir 1.10. Cette chaîne est uniquement stationnaire.

### Étape 2 - compléter le matériau

La tâche thermique a besoin de quatre propriétés propres en plus des trois
nécessaires à la tâche d'électrostatique ; le matériau de l'entité doit donc
posséder toutes les suivantes :

| Propriété | Valeur pour le cuivre | Requise par |
|---|---|---|
| Relative permittivity | `2.8` | électrostatique |
| Electrical conductivity | `5.882e7` `S/m` | électrostatique |
| Density | `8960` `kg/m^3` | les deux |
| Thermal conductivity | `401` `W/(m*K)` | thermique |
| Heat capacity | `385` `J/(kg*K)` | thermique |
| Emissivity | `0` | thermique |

Le matériau fourni *Copper* les possède déjà. Une entité à laquelle il manque
une propriété requise par **l'une ou l'autre** tâche est ignorée par cette
tâche.

### Étape 3 - conserver les conditions aux limites électriques

Les deux conditions **Electric potential** du tutoriel 3 restent inchangées.
Réduisez le potentiel d'excitation à une valeur moins violente - `0.01` V aux
bornes de la barre produit encore un courant important, et maintient la source
de chaleur dans une plage où la réponse thermique reste lisible.

### Étape 4 - ajouter les conditions aux limites thermiques

La partie thermique du modèle a besoin de sa propre évacuation, exactement comme
si elle était seule :

1. Sélectionnez l'entité surface qui couvre les côtés de la barre.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Simple convection**.
3. Réglez **Convection coefficient** = `10` `W/(m^2*K)` et **Fluid
   temperature** = `293.15` K.

Sans surface de convection ni température imposée, le modèle thermique a une
source et aucune évacuation, et le régime stationnaire n'existe pas. Notez
qu'une face utilisée pour la convection n'a pas besoin d'épaisseur pour le
solveur thermique, mais qu'une face d'épaisseur nulle ne porte pas non plus de
champ électrique - gardez les conditions électriques sur les faces d'extrémité
et la convection sur les côtés, et les deux n'interfèrent pas.

### Étape 5 - résoudre

`Solution` -> `Démarrer le solveur` (`Ctrl+R`). Le journal affiche d'abord la
tâche d'électrostatique, avec les statistiques de ses six variables, puis la
tâche thermique avec les statistiques de la température, du flux de chaleur et
du coefficient de transfert thermique.

### Étape 6 - examiner les résultats

- Onglet `Résultats` : affichez **Joule heat**. C'est le pont entre les deux
  physiques - partout où elle est non nulle, le solveur thermique a vu une
  source.
- Affichez **Temperature**. La barre doit être la plus chaude là où la chaleur
  Joule est la plus forte, et redescendre vers la température du fluide aux
  faces de convection.
- Affichez **Electric potential** pour confirmer que la partie électrique du
  modèle est toujours le champ linéaire du tutoriel 3.

### Étape 7 - vérifier le bilan énergétique

La chaleur Joule est une densité de dissipation en `W/m^3` ; la puissance totale
que la partie électrique du modèle fournit à la partie thermique est donc cette
densité multipliée par le volume sur lequel elle agit. Pour la barre homogène,
elle doit valoir le `P = U^2/R` du tutoriel 3, avec `R = L/(sigma*A)`.

En régime stationnaire, la surface de convection doit évacuer exactement cette
puissance, ce qui donne la même vérification globale que celle décrite dans le
manuel thermique :

```
P = h * A_s * ( Ts_mean - Tf )
```

Lisez la température moyenne de surface dans les statistiques du journal du
solveur et confirmez que le bilan est équilibré. C'est la seule vérification qui
couvre les deux physiques à la fois : elle détecte une conductivité erronée, un
coefficient de convection erroné et une entité restée silencieusement non
résolue dans l'une ou l'autre partie.

### Dépannage

| Symptôme | Cause |
|---|---|
| La température est uniforme à `293.15` K | la chaleur Joule est nulle - vérifiez la conductivité électrique, et que la tâche d'électrostatique s'exécute en premier |
| La résolution thermique ne converge pas | la partie thermique n'a ni température imposée ni surface de convection |
| Le groupe *Solveur temporel* a disparu | attendu - une tâche d'électrostatique rend tout l'enchaînement stationnaire, voir 1.10 |
| L'élévation de température change quand le maillage est raffiné | inattendu - chaque résultat électrostatique est une densité ; vérifiez donc la partie thermique du modèle et la qualité du maillage |
| Seuls les résultats électrostatiques apparaissent | il manque à la tâche thermique l'une de ses quatre propriétés de matériau requises |

---

## 5. Vérifier un modèle

Le solveur n'a pas de suite de vérification intégrée ; il vaut donc la peine de
valider un nouveau modèle par rapport à quelque chose qui se calcule à la main
avant de s'y fier. Les vérifications ci-dessous portent sur le **potentiel**,
qui est à la fois l'inconnue principale et la grandeur dans laquelle toute
solution analytique est exprimée ; les résultats de champ électrique et de
densité de courant doivent concorder avec les gradients qui en découlent.

**Barre homogène.** Pour un corps de longueur `L` maintenu à `V1` et `V2` sur
deux faces opposées, avec un seul matériau et sans charge, le potentiel est
linéaire le long de la barre et

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Placez un point de surveillance dans le plan médian et confirmez qu'il indique
la moyenne des deux valeurs imposées, et un autre au quart de la longueur pour
la valeur au quart. Cela détecte un maillage déconnecté, une condition affectée
à la mauvaise entité et une permittivité nulle. Raffinez le maillage une fois et
confirmez que les valeurs ne bougent pas.

**Charge d'espace uniforme.** La barre ci-dessus ne sollicite que la rigidité.
Pour vérifier aussi le terme source, prenez le même corps, maintenez **les deux**
faces d'extrémité à `0` V et affectez une *Charge density* uniforme `rho` au
volume. Le potentiel est alors une parabole, avec `eps = e0*er` :

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`at the mid-plane` = dans le plan médian.) Un point de surveillance dans le plan
médian donne directement `Vmax`. Deux choses sont vérifiées à la fois :
l'ordre de grandeur, qui confirme que la permittivité et la densité de charge
sont lues comme prévu, et le **signe** - une densité de charge positive doit
produire une bosse de potentiel positive entre deux faces à la masse. Répétez
avec la charge sur une entité surface ou ligne, puis sur un point, et confirmez
que la polarité ne change pas avec le type d'entité.

**Résistivité.** Chaque élément qui porte un courant doit stocker une
résistivité électrique d'exactement `1/sigma`. Relevez-la dans les statistiques
du journal du solveur et comparez-la avec la conductivité du matériau. Elle est
exacte et non approchée, indépendante du maillage, et c'est la confirmation la
plus rapide que le matériau a atteint les éléments prévus.

**Champ coaxial.** Pour une couronne entre les rayons `a` et `b`, la surface
intérieure à `V1` et la surface extérieure à `V2`, le potentiel est
logarithmique :

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Modélisez la couronne, imposez les deux surfaces et comparez des points de
surveillance à quelques rayons. Cela valide le solveur sur un champ non
uniforme, ce que la barre ne permet pas, et montre si le maillage est assez fin
près du conducteur intérieur.

**Diélectriques en série.** Pour deux couches d'épaisseurs `d1`, `d2` et de
permittivités relatives `er1`, `er2` empilées entre deux potentiels imposés, le
potentiel à l'interface vaut

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Comparez avec le potentiel d'interface calculé. Cela confirme directement ce que
décrit la section 1.9 : la répartition suit les **permittivités**. Exécuter le
même modèle avec deux conductivités différentes et la même permittivité, et
constater que le potentiel d'interface ne bouge pas, démontre qu'il s'agit d'un
solveur électrostatique et non d'un solveur de courant continu.

**Résistance et dissipation.** Les résultats aux éléments sont des densités ; un
total - un courant, une résistance, une puissance - doit donc provenir du
potentiel. Pour la barre homogène :

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Utilisez ces formules chaque fois qu'un total doit sortir du modèle - pour
dimensionner un conducteur, ou pour un contrôle de cohérence avec une mesure.
`P` est aussi la valeur à laquelle doit s'intégrer le résultat **Joule heat** sur
le volume de la barre, ce qui vérifie que les parties électrique et thermique
d'un calcul couplé concordent.

Pour chacune de ces vérifications, raffinez le maillage au moins une fois et
confirmez que le potentiel converge au lieu de dériver.

---

## 6. Limitations

### Limitations de modélisation

- **Électrostatique, et non courant continu.** La rigidité est construite à
  partir de la permittivité ; la conductivité n'entre jamais dans le système
  d'équations. Dans un corps de deux matériaux, le potentiel se répartit donc
  proportionnellement aux permittivités, et non aux résistances. Un conducteur
  d'un seul matériau est modélisé correctement ; un réseau de résistances ne
  l'est pas - voir 1.9.
- **Statique uniquement.** Il n'y a ni solveur temporel, ni capacité, ni
  inductance ; les transitoires de charge, les champs alternatifs, le courant de
  déplacement, les courants de Foucault et l'effet de peau sont donc tous hors du
  modèle. Ajouter une tâche d'électrostatique à un enchaînement de tâches
  supprime aussi le solveur temporel de tout l'enchaînement ; une tâche
  thermique placée après elle ne peut donc être que stationnaire - voir 1.10.
- **Diélectriques linéaires et isotropes uniquement.** La permittivité et la
  conductivité d'un élément sont des scalaires et restent constantes pendant une
  résolution. Les matériaux anisotropes, ferroélectriques et dépendant du champ
  ne sont pas modélisés, et il n'y a pas de claquage diélectrique.
- **Seulement deux conditions aux limites.** Un potentiel imposé et une densité
  de charge constituent tout le vocabulaire. Il n'y a ni charge surfacique
  imposée, ni flux normal imposé, ni condition de symétrie, ni traitement par
  frontière ouverte ou éléments infinis - un problème de champ non borné doit
  être tronqué en maillant suffisamment d'air environnant et en imposant un
  potentiel sur sa frontière extérieure.
- **Pas de redistribution des charges libres.** Les charges ne se déplacent pas
  vers la surface d'un conducteur ; le solveur n'a aucune notion de conducteur
  au-delà des propriétés de matériau qui lui sont données. Modélisez un
  conducteur en imposant son potentiel, et non en lui donnant une grande
  conductivité.
- **Pas de rétroaction du courant.** La densité de courant est post-traitée à
  partir du champ ; une conductivité qui varie avec le champ, ou avec la
  température produite par la dissipation, n'a donc aucun effet au sein d'une
  même résolution.
- **Les surfaces sont équipotentielles dans leur épaisseur.** Une entité surface
  ne porte le champ que dans son propre plan. Une surface d'épaisseur nulle ou
  une ligne de section nulle n'a aucune rigidité et est ignorée aussi bien à
  l'assemblage qu'au post-traitement.
- **Les éléments points ne conduisent pas.** Un point porte une source de charge
  et peut maintenir un potentiel imposé, rien de plus, et il n'est jamais
  post-traité. Un point dont le nœud n'appartient à aucun autre élément
  calculable laisse une ligne vide dans la matrice.
- **La dépendance du matériau à la température n'est pas lue.** Les tables sont
  évaluées à `293.15 K`, sauf si une tâche thermique placée plus tôt dans
  l'enchaînement a produit des températures aux éléments ; la condition
  d'environnement *Temperature* n'a aucun effet sur ce solveur.
- **Un modèle a besoin d'un potentiel imposé.** Sans potentiel imposé, la
  matrice a un noyau non trivial et la résolution ne convergera pas.
