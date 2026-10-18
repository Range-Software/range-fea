# Transfert de chaleur - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur thermique (`RSolverHeat`) de Range FEA, qui
traite le type de problème **Transfert de chaleur** (*Heat transfer*) : les
équations qu'il résout, la signification de chaque donnée d'entrée qu'il
accepte, les parties de l'interface graphique qui le pilotent, et deux tutoriels
complets.

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - transfert de chaleur stationnaire](#3-tutoriel---transfert-de-chaleur-stationnaire)
4. [Tutoriel - transfert de chaleur transitoire](#4-tutoriel---transfert-de-chaleur-transitoire)
5. [Vérifier un modèle](#5-vérifier-un-modèle)
6. [Limitations](#6-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnue principale - le champ de température

L'inconnue principale est la **température** `T` en chaque nœud, en kelvins.
Chaque nœud porte un seul degré de liberté ; un maillage de `N` nœuds donne donc
un système de `N` équations. Le flux de chaleur est obtenu à partir du champ de
température après la résolution.

La température est un scalaire, ce qui rend un modèle thermique nettement moins
coûteux qu'un modèle structurel sur le même maillage : une inconnue par nœud au
lieu de trois, et une matrice comptant un neuvième des termes.

La température par défaut d'un nœud qu'aucune condition ni aucun résultat
précédent n'atteint est de `293.15 K`.

### 1.2 Équation fondamentale

Le solveur met en œuvre la **conduction thermique linéaire** avec des frontières
convectives. Le bilan d'énergie d'un corps de conductivité `k`, de masse
volumique `rho` et de capacité thermique massique `c` s'écrit

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

et, avec le solveur temporel désactivé, le terme transitoire disparaît, laissant
le bilan stationnaire

```
div( k * grad(T) ) + q = 0
```

La chaleur quitte une surface par convection selon la loi de refroidissement de
Newton,

```
qs = h * ( T - Tf )
```

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `T` | température | `K` | inconnue résolue |
| `q` | densité de source de chaleur | `W/m^3` | condition Heat, chaleur Joule, rayonnement |
| `k` | conductivité thermique | `W/(m*K)` | propriété du matériau |
| `c` | capacité thermique | `J/(kg*K)` | propriété du matériau |
| `rho` | masse volumique | `kg/m^3` | propriété du matériau |
| `h` | coefficient de convection | `W/(m^2*K)` | condition de convection, voir 1.5 |
| `Tf` | température du fluide | `K` | condition de convection |
| `Q` | flux de chaleur | `W/m^2` | obtenu, voir 1.9 |

Il n'y a pas de terme d'advection. Un problème de transfert de chaleur déplace
l'énergie par conduction à l'intérieur du corps et par convection à travers sa
frontière ; il ne transporte pas d'énergie avec un milieu en écoulement.
Utilisez pour cela *Heat transfer in fluids*.

**Propriétés de matériau requises.** Un problème thermique en requiert
**quatre** :

| Propriété | Unités | Utilisée pour |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | la matrice de conduction |
| Heat capacity | `J/(kg*K)` | la matrice de capacité, en transitoire uniquement |
| Density | `kg/m^3` | la matrice de capacité, en transitoire uniquement |
| Emissivity | `-` | transfert de chaleur par rayonnement uniquement |

Une entité dont le matériau n'a pas l'une des quatre **n'est pas résolue**, même
pour un calcul de conduction purement stationnaire qui n'utilise jamais la masse
volumique ni l'émissivité. Le contrôle de la configuration le signale avant le
début du calcul. La seule exception est une entité qui porte une condition aux
limites thermique : cette entité est incluse dans le système quel que soit son
matériau, et sa conductivité et sa capacité valent alors simplement zéro.

Chaque propriété est une table en fonction de la température. La valeur utilisée
pour un élément est lue dans la table à la **température de l'élément du calcul
précédent** ; une conductivité dépendant de la température a donc une itération
de retard. La section 1.6 explique comment la faire converger.

### 1.3 Discrétisation par éléments finis

La forme faible du bilan d'énergie donne, pour chaque élément,

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(`capacity matrix` = matrice de capacité), où `B` est construite à partir des
dérivées des fonctions de forme et `N` sont les fonctions de forme elles-mêmes.
Chaque matrice élémentaire est `n x n` pour un élément à `n` nœuds - une ligne
par nœud, et non trois.

Les termes de convection ne sont assemblés que sur les **éléments surfaces**, et
uniquement là où une condition aux limites de convection est affectée. La
rigidité de convection est la forme cohérente `INT( N^T * h * N )`, tandis que
la charge de convection est répartie à parts égales sur les nœuds de l'élément.
Les deux sont suffisamment cohérentes sur un maillage raisonnable, et la
différence disparaît à mesure que le maillage est raffiné.

**Mesures des éléments.** Comme ailleurs dans Range FEA, les entités de
dimension inférieure reçoivent une mesure géométrique afin de pouvoir
représenter des corps réels :

| Entité | Mesure | Où elle est définie |
|---|---|---|
| Volume | 1 | - |
| Surface | épaisseur de la surface | propriétés géométriques de l'entité |
| Ligne | section de la ligne | propriétés géométriques de l'entité |
| Point | volume du point | propriétés géométriques de l'entité |

Une **surface** conduit la chaleur dans son propre plan, à travers une section
égale à son épaisseur, et stocke de l'énergie proportionnellement à cette
épaisseur. Il n'y a pas de gradient de température dans l'épaisseur : un élément
surface a une épaisseur d'un seul nœud, il est donc par construction isotherme
dans son épaisseur. Une surface **d'épaisseur nulle** ne conduit donc rien et ne
stocke rien, mais porte encore la convection - ce qui est exactement ce que l'on
veut pour la face frontière d'un volume maillé, où la face n'existe que pour
porter la condition de convection.

Une **ligne** conduit selon sa propre direction, à travers sa section. Un
**point** n'a pas de direction selon laquelle conduire ; il n'apporte donc
aucune conduction : il ne porte qu'une capacité, proportionnelle à son volume de
point, et une source de chaleur. Un point est une masse thermique concentrée, et
non un conducteur.

La matrice de capacité n'est assemblée que lorsqu'elle est nécessaire,
c'est-à-dire pour une analyse transitoire.

### 1.4 Sources et conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Temperature | explicite (Dirichlet) | point, ligne, surface, volume | Temperature `[K]` |
| Heat | naturelle, source | point, ligne, surface, volume | Heat `[W]`, voir ci-dessous |
| Simple convection | naturelle (Robin) | surface | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | naturelle, corrélation | surface | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | naturelle, corrélation | surface | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | naturelle, source | surface | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | naturelle, source | volume | Heat per unit volume `[W/m^3]` |

**Temperature** (température) est la seule condition explicite du problème
thermique. Chaque nœud de l'entité à laquelle elle est affectée est retiré du
système d'équations et maintenu à la valeur donnée ; sa contribution est
reportée au second membre des équations voisines. Un modèle stationnaire en a
besoin d'au moins une, ou d'au moins une surface de convection - voir 1.7.

Là où des entités se chevauchent, **l'entité de dimension inférieure
l'emporte** : une température imposée sur une surface remplace, aux nœuds
communs, celle imposée sur le volume situé derrière elle ; une ligne l'emporte
sur une surface, et un point sur tout le reste. Contrairement aux contraintes de
déplacement du solveur structurel, les températures ne sont pas combinées - il
n'y a qu'un degré de liberté par nœud à maintenir, la dernière condition lue
prévaut donc simplement.

**Heat** (chaleur) est une source de chaleur, et sa valeur est la **puissance
totale en `W`** apportée à l'entité à laquelle elle est affectée, quelle que
soit la dimension de cette entité. Le solveur a besoin d'une densité de source ;
le total est donc réparti sur la mesure de l'entité avant l'assemblage :

| Entité | Densité appliquée par le solveur |
|---|---|
| Volume | valeur / volume de l'entité, `W/m^3` |
| Surface | valeur / aire de l'entité, `W/m^2` |
| Ligne | valeur / longueur de l'entité, `W/m` |
| Point | valeur / nombre d'éléments points, `W` chacun |

La puissance apportée est donc exactement le nombre saisi - elle ne change ni
avec la taille de l'entité, ni avec le raffinement du maillage, ni avec
l'échelle du modèle. Seuls les éléments que le solveur peut calculer en reçoivent
une part ; une entité sans propriétés de matériau est donc ignorée et sa
condition signalée. Une source de chaleur surfacique n'est pas pondérée par
l'épaisseur de la surface.

**Heat rate (unit area)** et **Heat rate (unit volume)** imposent le même terme
source sous forme de densité au lieu d'un total : `W/m^2` sur l'aire d'une
entité surface et `W/m^3` sur le volume d'une entité volume. Utilisez-les
lorsque la source est naturellement une densité - un élément chauffant spécifié
par mètre carré - et *Heat* lorsqu'elle est naturellement un total, comme un
élément de puissance connue. Toutes s'additionnent sur une entité qui en porte
plusieurs, et toutes acceptent une table en fonction du temps ; un élément
chauffant peut donc être allumé et éteint au cours d'un calcul transitoire.

Les conditions de **convection** s'appliquent aux surfaces et ajoutent à la fois
un terme de rigidité et une charge ; elles agissent donc comme une condition aux
limites de Robin et non comme un flux fixe : élever la température de la paroi
augmente la chaleur qu'elle perd. Les trois variantes aboutissent au même couple
`(h, Tf)` ; elles ne diffèrent que par l'origine de `h` :

- **Simple convection** (convection simple) prend `h` et `Tf` directement dans
  les deux valeurs saisies. Utilisez-la chaque fois que le coefficient de
  convection est connu par un manuel, une mesure ou un calcul antérieur. C'est
  la seule des trois que vous maîtrisez entièrement.
- **Forced convection** (convection forcée) a deux modes. Sur une paroi entre un
  solide maillé et un fluide maillé, une fois qu'une tâche *Heat transfer in
  fluids* a produit un résultat, elle couple directement les deux résolutions -
  aucune corrélation n'intervient, puisque l'écoulement résolu porte déjà la
  convection. Le solveur thermique des fluides transmet, pour chaque élément de
  la paroi, le couple `(h, Tf)` qui reproduit le flux de chaleur que sa propre
  solution fait passer à travers la paroi ; voir *Transfert de chaleur conjugué*
  en 1.6. Partout ailleurs - une surface qui ne borde aucun fluide maillé, ou le
  premier passage d'un calcul couplé avant l'exécution de la tâche thermique des
  fluides - `h` est calculé à partir d'une corrélation pour un écoulement externe
  le long d'une plaque plane, en utilisant les propriétés du fluide, le diamètre
  hydraulique, la *Fluid temperature* et la *Velocity* saisies. Le journal
  indique lequel des deux modes est utilisé.
- **Natural convection** (convection naturelle) calcule `h` à partir d'une
  corrélation pour des plaques horizontales, en utilisant les propriétés du
  fluide, le diamètre hydraulique et l'écart de température entre la paroi et le
  fluide.

Si plusieurs conditions de convection sont affectées à la même surface, elles ne
s'additionnent pas : la convection simple est lue en premier, la convection
forcée l'écrase, et la convection naturelle écrase les deux.

### 1.5 Les corrélations de convection

Les deux formes calculées construisent les nombres sans dimension habituels à
partir des propriétés du fluide saisies avec la condition aux limites, le
diamètre hydraulique `d` servant de longueur caractéristique :

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

et convertissent le nombre de Nusselt en coefficient de convection par

```
h = Nu * k / d
```

| Forme | Corrélation | Nombre de Nusselt |
|---|---|---|
| Forcée | écoulement externe, plaque plane | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Naturelle | plaque horizontale, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Naturelle | plaque horizontale, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

Les `k`, `c`, `rho`, `mu` et `beta` de ces expressions sont les propriétés du
**fluide** saisies avec la condition aux limites, et non les propriétés du
matériau du solide.

Les deux conditions sont créées avec les propriétés de **l'air sec à
20 degrés Celsius et 101.325 kPa** ; le fluide est donc décrit pour vous et il ne
reste à saisir que l'écoulement et la géométrie :

| Composante | Valeur par défaut |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, la valeur du gaz parfait `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Remplacez-les pour tout autre fluide - ce sont les propriétés du fluide, et non
le matériau de la paroi. *Natural convection* est donc prête à être résolue telle
qu'affectée ; *Forced convection* a encore besoin d'une **vitesse**, qu'aucune
valeur par défaut ne peut deviner - sauf si la surface borde un fluide maillé,
auquel cas aucune de ces valeurs n'est lue.

Chacun de ces nombres divise par `mu`, `k` ou `d`, et `Re` et `Pr` tombent à zéro
si `rho`, `c` ou `v` sont nuls, ce qui donnerait `h = 0` et une surface qui ne se
refroidit jamais. Une valeur nulle est donc traitée comme une erreur au lieu
d'être masquée : le solveur s'arrête et nomme la composante et l'entité, par
exemple

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

(« La valeur de 'Velocity' configurée dans la condition aux limites 'Forced
convection' sur l'entité 'Surface' est nulle - la corrélation de convection ne
peut pas être évaluée. »)

La vitesse calculée par le solveur d'écoulement n'est jamais injectée dans la
corrélation. Une paroi adhérente maintient le fluide au repos, la vitesse à son
voisinage est donc nulle, et la corrélation aurait besoin de la vitesse de
l'écoulement libre et de la température de mélange, qu'un écoulement
tridimensionnel général ne définit pas. Une paroi qui borde un fluide maillé est
couplée à la résolution du fluide à la place.

Trois conséquences sont à garder à l'esprit :

- La corrélation est **fixe**. Le solveur ne tient compte ni de l'orientation de
  la surface, ni du caractère interne ou externe de l'écoulement, ni du régime
  laminaire ou turbulent de la couche limite. Tout passe par le diamètre
  hydraulique, qui est la longueur caractéristique de la corrélation - c'est
  votre seul levier sur la géométrie.
- Un diamètre hydraulique nul donnerait `h = 0`, c'est-à-dire une paroi isolée ;
  le solveur s'arrête donc au lieu de l'accepter.
- La convection naturelle dépend de la **température de paroi**, prise à la
  température de l'élément de la résolution **précédente**. Au premier calcul
  d'une analyse stationnaire, c'est la température initiale ; le coefficient est
  donc évalué avec un écart de température erroné. Placez la tâche dans un
  groupe de tâches de problème avec plusieurs itérations pour la faire
  converger, ou utilisez *Simple convection* avec un coefficient que vous avez
  calculé vous-même. Un écart de température nul donne `Nu = 1`.

### 1.6 Conditions d'environnement, initiales et de couplage

#### Conditions d'environnement

| Condition d'environnement | Composantes | Effet |
|---|---|---|
| Temperature | Temperature `[K]` | température de fond de l'entité, au premier calcul uniquement |

#### Conditions initiales

| Condition initiale | Effet |
|---|---|
| Temperature | champ de température initial, appliqué au premier calcul |

Les conditions d'environnement, les conditions initiales et les conditions aux
limites sont lues dans cet ordre et chacune écrase la précédente ; ainsi, **une
condition aux limites l'emporte sur une condition initiale, qui l'emporte sur
une condition d'environnement**. Les valeurs d'environnement et initiales ne
sont lues **qu'au premier calcul** ; ensuite, le champ avance à partir du
résultat précédent.

#### Couplage avec d'autres solveurs

Le solveur thermique utilise et produit des résultats partagés avec le reste de
l'enchaînement des tâches.

| Sens | Variable | Effet |
|---|---|---|
| Rayonnement -> thermique | Heat - radiation part | ajoutée au terme source des éléments **surfaces** |
| Électrostatique -> thermique | Joule heat | ajoutée au terme source de chaque élément |
| Contraintes -> thermique | Displacement | le maillage est déformé pour la résolution thermique puis rétabli |
| Thermique -> contraintes | Temperature | pilote le terme de dilatation thermique du solveur structurel |
| Thermique -> tout solveur | Temperature | sélectionne la ligne de chaque table de matériau dépendant de la température |
| Thermique des fluides -> thermique | `(h, Tf)` de paroi | pilote les parois *Forced convection* qui bordent un fluide maillé |
| Thermique -> thermique des fluides | température du solide | maintient ces mêmes parois à la température du solide dans la résolution du fluide |

Le **transfert de chaleur par rayonnement** est un type de problème distinct
avec son propre solveur. Il nécessite une tâche *Heat transfer*, résout un
système de radiosité d'enceinte sur des zones de surface à l'aide des facteurs
de forme et de l'émissivité du matériau, et renvoie le résultat sous forme de
source de chaleur surfacique. Ajoutez les deux tâches à un même groupe de tâches
de problème et laissez le groupe itérer : le solveur de rayonnement indique la
convergence et termine le groupe lorsque la chaleur de ses zones cesse de
varier.

La **chaleur Joule** arrive de la même manière d'une tâche *Electro-statics*
placée avant la tâche thermique ; l'échauffement résistif ne nécessite donc
aucune configuration supplémentaire.

Sauf s'il est couplé à un fluide, le solveur thermique se déclare convergé sans
condition ; un groupe de tâches qui ne contient qu'une tâche thermique s'arrête
donc toujours après une itération. Itérer n'a d'intérêt que lorsqu'une grandeur
dont dépend la résolution thermique est elle-même décalée - chaleur rayonnée,
coefficient de convection naturelle ou conductivité fortement dépendante de la
température.

#### Transfert de chaleur conjugué

Le solveur thermique ne résout **que les solides**. Un volume dont le matériau
est un fluide - un gaz ou un liquide, ou un matériau d'état non spécifié qui
possède une viscosité dynamique, ce qui couvre tous les fluides de la base de
données de matériaux - ne fait jamais partie de la résolution thermique, quelles
que soient les propriétés ou les conditions qu'il porte. Il en va de même d'une
entité point, ligne ou surface située dans le fluide, comme une entrée ; une
condition portée par celle-ci et que seul le solveur thermique lit est signalée
dans le journal. Le domaine fluide relève du type de problème *Heat transfer in
fluids*, qui transporte la chaleur avec l'écoulement.

Les deux se rejoignent sur les parois portant la condition **Forced
convection**. Avec les deux tâches dans un même groupe de tâches de problème -
la tâche thermique des fluides en premier - chaque passage du groupe procède
ainsi :

1. Le solveur thermique des fluides maintient les nœuds de paroi à la
   température que le solveur thermique a calculée en dernier dans le solide (au
   tout premier passage, lorsque cette température n'existe pas encore, la paroi
   est isolée), résout le fluide et relève le flux de chaleur qui entre dans le
   fluide en chaque nœud de paroi. Ce flux est le résidu des équations du fluide
   au nœud, celui que verrait une résolution unique des deux domaines ; une
   couche limite thermique mince ne coûte donc aucune précision.
2. Pour chaque élément de paroi, il transmet le couple `(h, Tf)`. `h = k * G` est
   la conductance du premier élément fluide derrière la paroi - `k` divisée par
   la hauteur de l'élément au-dessus de la paroi, pour un tétraèdre - et `Tf` est
   choisie de sorte que `q = h * (Tw - Tf)` soit le flux de l'étape 1.
3. Le solveur thermique les applique à la paroi comme il le ferait d'une
   condition *Simple convection*, résout le solide et renvoie sa température de
   paroi.

Le couple `(h, Tf)` est un artifice numérique qui fait concorder les deux
résolutions - `h` n'est pas le coefficient de transfert thermique d'un manuel,
et il change avec le maillage. Ce qui converge, ce sont la température et le
flux à travers la paroi, qui sont ceux du problème couplé. Rien n'est nécessaire
sur la vitesse du fluide près de la paroi, ce qui explique qu'une paroi
adhérente - où l'écoulement est au repos - transfère la chaleur exactement comme
il se doit.

La température de paroi transmise au fluide est relaxée avec un **facteur
d'Aitken** calculé à partir des deux derniers passages. Sans lui, un solide peu
conducteur à côté d'un fluide bien résolu peut nécessiter des centaines de
passages ; avec lui, le groupe se stabilise généralement en quelques passages.
Pendant le couplage, les deux solveurs indiquent comme convergence la variation
relative de leur champ de température ; définissez donc la valeur de
convergence du groupe de tâches - `1e-6` est un bon point de départ - et
donnez-lui suffisamment d'itérations. Une valeur de convergence nulle les
exécute toutes.

### 1.7 Analyse stationnaire

Avec le solveur temporel désactivé, le solveur assemble et résout

```
K * T = f
```

`K` est symétrique définie positive dès qu'au moins une température a été
imposée ; elle est donc résolue par la méthode du **gradient conjugué** avec un
préconditionnement de Jacobi.

Un modèle stationnaire **sans condition aux limites Temperature ni surface de
convection** est l'équivalent thermique d'une structure non bridée : de
l'énergie peut y être injectée mais ne peut s'échapper nulle part, la matrice
est singulière et la résolution ne convergera pas. Chaque modèle stationnaire a
besoin d'au moins un chemin vers une température connue.

### 1.8 Analyse transitoire

Avec le solveur temporel activé, le solveur intègre en temps

```
M * dT/dt + K * T = f
```

avec un schéma theta, où `alpha` est le coefficient d'approximation du schéma
temporel :

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

Le paramètre *Approximation* du solveur temporel choisit `alpha` :

| Approximation | `alpha` | Schéma obtenu | Stabilité |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, un pas implicite | inconditionnelle |
| Central difference (accurate) | `0.5` | Crank-Nicolson | inconditionnelle, peut osciller |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, un pas explicite | conditionnelle |

**Backward difference** (différences arrière) est la valeur par défaut sûre pour
un modèle thermique. Elle coûte une résolution matricielle par pas, mais le pas
est limité par la précision souhaitée et non par la taille du plus petit
élément. **Central difference** (différences centrées) est précise au second
ordre et constitue le meilleur choix dès que le pas de temps est assez petit pour
résoudre le transitoire, bien qu'elle puisse osciller lors d'une mise en marche
brutale. **Forward difference** (différences avant) place toute la matrice de
conduction sur l'ancien pas de temps, ce qui rend chaque pas peu coûteux et le
calcul fragile.

La variante avant n'est stable que tant que le pas de temps reste inférieur à
environ

```
dt < rho * c * h^2 / ( 2 * k )
```

où `h` est la plus petite taille d'élément. Comme la diffusivité thermique
`k/(rho*c)` d'un métal est de l'ordre de `1e-5 m^2/s`, un élément de la taille
d'un millimètre impose déjà un pas de temps bien inférieur à un dixième de
seconde. Sauf pour une résolution temporelle très fine, utilisez le schéma
implicite.

Un calcul transitoire instable se reconnaît facilement : le champ de température
oscille d'un pas à l'autre avec une amplitude croissante, souvent entre des
valeurs physiquement impossibles, au lieu de se stabiliser. Le remède est un pas
de temps plus petit ou le schéma implicite.

### 1.9 Résultats dérivés

Après la résolution, le flux de chaleur aux éléments est obtenu à partir du
champ de température comme la moyenne sur les points d'intégration de

```
Q = -k * grad(T)
```

évalué dans l'élément et tourné dans le repère global pour les éléments lignes
et surfaces. Sur une surface qui porte une convection, le flux convectif

```
Qh = h * ( Tf - T )
```

est ajouté selon la normale à la surface ; le vecteur flux d'une face convective
pointe donc dans le sens où la chaleur s'échappe réellement.

Le flux est une **densité** en `W/m^2`, indépendante de la taille de l'élément
dans lequel elle a été calculée ; on peut donc la comparer d'un élément à
l'autre et avec un calcul manuel, et elle converge à mesure que le maillage est
raffiné.

Le **coefficient de transfert thermique** est également stocké. C'est le
coefficient que le solveur a réellement utilisé sur chaque élément surface, ce
qui est le seul moyen de voir ce qu'ont produit les corrélations *Forced
convection* et *Natural convection* - et sur une surface en convection
naturelle, il varie d'un élément à l'autre, puisqu'il dépend de la température
locale de la paroi. Sur une paroi couplée à un fluide, c'est la conductance du
premier élément fluide, qui dépend du maillage - voir *Transfert de chaleur
conjugué* en 1.6. Il est nul sur tout élément qui ne porte pas de condition de
convection.

| Résultat | S'applique à | Signification |
|---|---|---|
| Temperature `[K]` | nœud | le champ de température résolu |
| Heat flux `[W/m^2]` | élément | vecteur flux conductif, plus la partie convective sur les surfaces convectives |
| Heat transfer coefficient `[W/(m^2*K)]` | élément | le coefficient de convection utilisé, nul en l'absence de convection |

Après chaque enregistrement, le journal du solveur affiche les statistiques des
trois et les valeurs aux éventuels points de surveillance.

### 1.10 Stationnaire ou transitoire ?

| | Stationnaire | Transitoire |
|---|---|---|
| Résout | `K*T = f` | `M*T' + K*T = f` dans le temps |
| Répond à | « jusqu'à quelle température cela chauffe-t-il au final » | « combien de temps faut-il pour y arriver » |
| Utilise la masse volumique et la capacité thermique | non, mais elles doivent être présentes | oui |
| Nécessite une température imposée ou une surface convective | oui | non, une capacité suffit |
| Enregistrements | un | un par pas de temps écrit |

**Utilisez une analyse stationnaire pour** la répartition finale de température
d'une pièce en fonctionnement continu, le dimensionnement d'un dissipateur, un
pont thermique dans un mur, ou le champ de température qui alimente une analyse
des contraintes thermiques. C'est le choix par défaut, qui couvre la plupart des
travaux.

**Utilisez une analyse transitoire pour** une montée ou une descente en
température, un cycle de fonctionnement, un choc thermique, une trempe, ou tout
cas où c'est la constante de temps de la pièce qui vous intéresse.

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`,
`Conditions d'environnement`, `Matériau`, `Résultats`), le menu **Problème**
et le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Ajoutez une tâche et choisissez
**Heat transfer** - « Steady-state and transient Heat-transfer » (transfert de
chaleur stationnaire et transitoire).

Le transfert de chaleur se combine librement avec toutes les autres physiques
sauf *Modal analysis*. Les chaînes à connaître sont :

- *Heat transfer* puis *Stress analysis* - contraintes thermiques ;
- *Heat transfer* et *Radiative heat transfer* dans un même groupe de tâches
  itératif - conduction plus rayonnement d'enceinte ;
- *Electro-statics* puis *Heat transfer* - échauffement résistif.

Pour faire itérer un couple de tâches couplées, ajoutez un groupe de tâches et
réglez son nombre d'itérations ; le groupe s'arrête plus tôt lorsque toutes ses
tâches indiquent la convergence.

### 2.2 Onglet Problème

**Solveur temporel** - affiché pour le transfert de chaleur, car c'est un type
de problème dépendant du temps. Laissez-le désactivé pour une analyse
stationnaire.

| Champ | Signification |
|---|---|
| Activer | active ou désactive la marche en temps transitoire |
| Approximation | theta du schéma temporel : arrière, centré ou avant, voir 1.8 |
| Temps de début | première valeur de temps en secondes ; ignorée lors d'un redémarrage du solveur |
| Temps de fin | `start time + time-step size * number of time-steps` (temps de début + taille du pas de temps × nombre de pas de temps), lecture seule |
| Taille du pas de temps | `dt` en secondes |
| Nombre de pas de temps | nombre de pas à calculer |
| Fréquence de sortie | écrit un fichier de résultats tous les N pas ; `0` n'écrit que le dernier pas |

Un problème thermique n'a pas de groupe de paramètres propre. Tout le reste
s'affecte par entité dans les onglets des conditions.

### 2.3 Onglet Matériau

L'onglet `Matériau` liste le matériau affecté à l'entité sélectionnée. Le
transfert de chaleur requiert **ces quatre** propriétés :

| Propriété | Unités | Commentaire |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, environ `50` pour l'acier de construction, `15` pour l'acier inoxydable, `200` pour l'aluminium |
| Heat capacity | `J/(kg*K)` | `c`, environ `460` pour l'acier |
| Density | `kg/m^3` | `rho`, environ `7850` pour l'acier |
| Emissivity | `-` | utilisée uniquement par le transfert de chaleur par rayonnement, mais requise malgré tout |

Une entité à laquelle il en manque une n'est pas résolue, et le contrôle de la
configuration avertit avant le début du calcul. Si vous ne faites pas de
transfert de chaleur par rayonnement, une émissivité de `0` suffit à satisfaire
l'exigence.

Le matériau doit aussi être un **solide**. Un volume dont le matériau est un gaz
ou un liquide - ou n'a pas d'état défini et possède une viscosité dynamique -
est laissé au type de problème *Heat transfer in fluids*, même s'il possède les
quatre propriétés.

Chaque propriété est une table en fonction de la température ; une conductivité
dépendant de la température est donc prise en compte - évaluée à la température
de l'élément du calcul précédent, ce qui justifie de faire itérer la tâche
lorsque la dépendance est forte.

### 2.4 Onglet Conditions aux limites

L'onglet est divisé en une liste de gestion des conditions aux limites
disponibles en haut et un éditeur de la condition sélectionnée en bas.
Sélectionnez d'abord une entité dans l'arbre `Modèle` ; le gestionnaire propose
alors les conditions valables pour ce type d'entité et pour le problème
sélectionné. Cochez-en une pour l'affecter, puis modifiez ses composantes dans
l'arbre inférieur.

Les conditions de la section 1.4 sont proposées. Remarques sur leur
utilisation :

- **Temperature** est la seule condition qui maintient une valeur au lieu de la
  piloter. Affectez-la à la face, l'arête ou le point dont vous connaissez la
  température.
- **Heat** est proposée pour tous les types d'entités et prend la puissance
  totale en `W` ; le solveur la répartit sur l'entité pour vous. C'est la
  condition à utiliser lorsque vous connaissez la puissance de ce qui produit la
  chaleur.
- **Heat rate (unit volume)** sur un volume et **Heat rate (unit area)** sur une
  surface imposent la même source sous forme de densité, pour une source
  naturellement spécifiée par unité de volume ou de surface.
- Les trois conditions de **convection** ne s'appliquent qu'aux surfaces.
  Affectez *Simple convection*, sauf raison particulière de laisser le solveur
  corréler le coefficient pour vous ; c'est la plus prévisible. *Forced
  convection* sur une paroi qui borde un fluide maillé couple le solide à la
  résolution thermique du fluide et ne lit aucune de ses composantes ; ailleurs,
  elle en déduit le coefficient par corrélation.
- Chaque composante est une table en fonction du temps ; on peut donc donner un
  historique temporel à une température imposée ou à une source de chaleur pour
  un calcul transitoire. C'est ainsi que l'on modélise un cycle de
  fonctionnement ou un élément chauffant à montée progressive.

### 2.5 Conditions initiales et d'environnement

L'onglet `Conditions initiales` propose *Temperature*, qui définit le champ de
départ d'un calcul transitoire. Affectez-la à tout le modèle et donnez-lui la
température de départ de la pièce.

L'onglet `Conditions d'environnement` propose également *Temperature*, comme
valeur de fond pour une entité. Les deux ne sont lues qu'au premier calcul, et
une condition aux limites l'emporte sur les deux.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur thermique utilise l'entrée **CG**.

Si une résolution stationnaire ne converge pas, la cause habituelle est un
modèle sans température imposée ni surface convective plutôt qu'un paramètre du
solveur. Vérifiez les conditions aux limites avant d'augmenter le nombre
d'itérations.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer - pour un problème
thermique, *Temperature*. Dans un calcul transitoire, l'historique est affiché
par `Rapport` -> `Points de surveillance`, ce qui est la manière la plus rapide
de lire une courbe de montée en température ou une constante de temps sur un
modèle.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste les variables calculées et contrôle la vue 3D. La
température est un scalaire aux nœuds, le flux de chaleur un vecteur aux
éléments, et le coefficient de transfert thermique un scalaire aux éléments, nul
en dehors des surfaces convectives.

L'onglet `Enregistrements` du dock `Modèle` liste les enregistrements de
résultats :

- **analyse stationnaire** - un seul enregistrement ;
- **analyse transitoire** - un enregistrement par pas de temps écrit. L'animation
  des enregistrements montre la montée en température.

`Rapport` -> `Fichier journal du solveur` affiche la sortie complète du solveur,
y compris les statistiques de la température, du flux de chaleur et du
coefficient de transfert thermique après chaque enregistrement.

---

## 3. Tutoriel - transfert de chaleur stationnaire

**Objectif.** Calculer la répartition de température d'une barre maintenue
chaude à une extrémité et refroidie par l'air ambiant, et lire la température à
l'extrémité froide.

Ce tutoriel suppose un modèle volumique maillé - un parallélépipède allongé
convient bien. Utilisez `Fichier` -> `Nouveau modèle`, dessinez un
parallélépipède avec `Géométrie` -> `Créer un élément` ou importez une
géométrie, et maillez-le en tétraèdres ; le tutoriel *Dessiner un cube* couvre ce
sujet. Marquez la face de l'extrémité chaude et les faces restantes comme
entités surface distinctes afin de pouvoir leur affecter des conditions.

### Étape 1 - choisir le problème

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez une tâche et sélectionnez **Heat transfer**.
3. Confirmez avec `OK`.

### Étape 2 - affecter le matériau

1. Sélectionnez l'entité volume dans l'arbre `Modèle`.
2. Ouvrez l'onglet `Matériau` et affectez un matériau possédant les quatre
   propriétés, pour l'acier de construction :
   - **Thermal conductivity** (conductivité thermique) = `50` `W/(m*K)`
   - **Heat capacity** (capacité thermique) = `460` `J/(kg*K)`
   - **Density** (masse volumique) = `7850` `kg/m^3`
   - **Emissivity** (émissivité) = `0`

La capacité thermique et la masse volumique ne sont pas utilisées par une
résolution stationnaire, mais elles doivent être présentes pour que l'entité
soit résolue.

### Étape 3 - garder l'analyse stationnaire

Ouvrez l'onglet `Problème` et laissez **Activer** décoché dans le groupe
*Solveur temporel*. Le solveur assemble et résout alors `K*T = f` une seule
fois.

### Étape 4 - imposer l'extrémité chaude

1. Sélectionnez l'entité surface de l'extrémité chaude.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Temperature**.
3. Réglez la valeur sur `353.15` K, soit 80 degrés Celsius.

N'oubliez pas que toutes les températures dans Range FEA sont absolues, en
kelvins.

### Étape 5 - refroidir le reste

1. Sélectionnez l'entité surface qui couvre les faces restantes.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Simple convection**.
3. Réglez **Convection coefficient** = `10` `W/(m^2*K)` et **Fluid
   temperature** = `293.15` K.

Un coefficient d'environ `10` est représentatif d'une surface dans de l'air
calme ; l'air soufflé est plus proche de `50`, et l'eau se situe dans les
centaines ou les milliers.

C'est l'étape qui rend le modèle soluble. Sans elle, la barre a un apport de
chaleur et aucune évacuation, et un régime stationnaire n'existe pas.

### Étape 6 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Le contrôle de la configuration signale les matériaux ou les conditions aux
   limites manquants avant le début du calcul.
3. Suivez la progression dans `Rapport` -> `Fichier journal du solveur`.

### Étape 7 - examiner les résultats

- Onglet `Résultats` : affichez **Temperature**. Le champ doit décroître
  régulièrement de l'extrémité chaude imposée vers la température du fluide.
- Le point le plus froid doit rester au-dessus de la température du fluide. Une
  température inférieure à `Tf` où que ce soit, ou supérieure aux `353.15` K
  imposés, signifie que quelque chose apporte de l'énergie dans le mauvais
  sens - vérifiez le signe de toute source de chaleur.
- **Heat flux** montre où circule l'énergie, en `W/m^2`. Multipliez la valeur
  sur les faces convectives par leur aire et comparez-la à la puissance qui
  entre dans la barre.
- **Heat transfer coefficient** doit indiquer `10` sur les faces convectives et
  `0` partout ailleurs. C'est la manière la plus rapide de confirmer qu'une
  condition de convection a atteint les éléments voulus, et le seul moyen de
  voir ce qu'une condition corrélée a calculé.
- Un point de surveillance sur *Temperature* à l'extrémité froide donne la
  réponse sous forme de nombre plutôt que de couleur.

### Étape 8 - facultatif, ajouter une source de chaleur

Pour modéliser un élément chauffant intégré à la barre, sélectionnez l'entité
volume, ouvrez l'onglet `Conditions aux limites` et cochez **Heat**. Pour
apporter un total de `P` watts dans la barre, saisissez `P` - le solveur le
répartit sur le volume de l'entité.

Utilisez plutôt **Heat rate (unit volume)** lorsque la source est spécifiée par
unité de volume ; saisissez `P / V` `W/m^3` pour apporter le même total - voir
1.4.

La température stationnaire doit alors s'élever jusqu'à ce que la surface
convective puisse évacuer exactement cette puissance - c'est la vérification du
bilan d'énergie de la section 5.

### Étape 9 - facultatif, contraintes thermiques

Pour reporter le résultat dans une analyse structurelle :

1. Ajoutez une tâche **Stress analysis** *après* la tâche thermique dans
   l'enchaînement des tâches du problème, et donnez à l'entité les propriétés
   structurelles nécessaires.
2. Affectez à l'entité structurelle une condition d'environnement
   **Temperature** égale à la température de référence sans contrainte, par
   exemple `293.15` K.

Le solveur de contraintes reprend les températures calculées aux éléments et
ajoute automatiquement le terme de dilatation thermique.

### Dépannage

| Symptôme | Cause |
|---|---|
| La résolution ne converge pas | pas de condition Temperature ni de surface convective, le régime stationnaire est donc indéfini |
| *Entity has material assigned which is missing required properties* | l'une des quatre propriétés requises est absente - souvent l'émissivité |
| Tout le modèle est à une seule température | aucune chaleur n'entre, ou rien ne crée de gradient par rapport à la valeur imposée |
| Les températures sont absurdement élevées | une densité en `W/m^3` a été saisie dans une condition *Heat*, qui prend le total en watts, ou le coefficient de convection est beaucoup trop petit |
| Une paroi entre solide et fluide ne refroidit pas | la tâche thermique des fluides s'exécute après la tâche thermique, ou le groupe n'exécute qu'une itération - placez la tâche thermique des fluides en premier et laissez le groupe itérer |
| Le fluide est ignoré par la résolution thermique | voulu - un matériau fluide n'est résolu que par *Heat transfer in fluids* |
| Les températures sont proches de `293.15` K partout | l'entité n'a pas été résolue - recherchez dans le journal une propriété de matériau manquante |

---

## 4. Tutoriel - transfert de chaleur transitoire

**Objectif.** Démarrer la même barre à température ambiante, mettre en marche
l'extrémité chaude et déterminer combien de temps il lui faut pour chauffer.

### Étape 1 - choisir le problème

La même tâche **Heat transfer** que dans le tutoriel 3. Si le modèle est encore
configuré, il suffit d'ajouter les paramètres du solveur temporel et la
condition initiale.

### Étape 2 - affecter le matériau

Les quatre mêmes propriétés. La **capacité thermique** et la **masse volumique**
comptent ici : avec la conductivité, elles fixent la diffusivité thermique
`k/(rho*c)`, qui détermine la vitesse de progression du front.

### Étape 3 - définir la température initiale

1. Sélectionnez l'entité volume.
2. Ouvrez l'onglet `Conditions initiales` et cochez **Temperature**.
3. Réglez-la sur `293.15` K.

Sans elle, le modèle part tout de même des `293.15` K par défaut, mais
l'indiquer explicitement rend le modèle auto-documenté et permet de partir de
n'importe quel autre état.

### Étape 4 - configurer le solveur temporel

Ouvrez l'onglet `Problème` et, dans le groupe *Solveur temporel*, réglez :

| Champ | Valeur |
|---|---|
| Activer | coché |
| Approximation | `Backward difference (stable)` |
| Temps de début | `0` |
| Taille du pas de temps | `1` |
| Nombre de pas de temps | `300` |
| Fréquence de sortie | `10` |

*Backward difference* est le schéma implicite - voir le tableau de 1.8 - et
c'est celui à utiliser ici. Il reste stable avec un pas de temps choisi pour la
physique et non pour le maillage.

Choisissez le pas de temps d'après la constante de temps attendue, en visant
quelque chose comme cinquante pas sur la partie intéressante de la courbe. La
section 5 donne une formule pour une première estimation. Écrire un pas sur dix
garde un fichier de résultats petit sans perdre la forme de la courbe.

### Étape 5 - conserver les conditions aux limites

La **Temperature** imposée à l'extrémité chaude et la **Simple convection** sur
les faces restantes restent inchangées. Chaque composante est une table en
fonction du temps ; on peut donc donner plutôt une rampe à l'extrémité chaude :
ouvrez l'éditeur de composantes et ajoutez des instants pour faire monter la
température pendant les premières secondes au lieu d'un saut instantané.

### Étape 6 - ajouter un point de surveillance

`Problème` -> `Définir les points de surveillance`, placez un point à
l'extrémité froide et réglez sa variable sur **Temperature**. La courbe de
montée en température se lit bien plus facilement sous forme d'historique que
dans une suite d'images colorées.

### Étape 7 - résoudre

`Solution` -> `Démarrer le solveur` (`Ctrl+R`). Le journal affiche un bloc par
pas de temps :

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

(`time step: 1 of 300` = pas de temps 1 sur 300), suivi des itérations du
solveur matriciel et des statistiques de température de ce pas.

### Étape 8 - examiner les résultats

- Dock `Modèle`, onglet `Enregistrements` : un enregistrement par pas écrit.
  Parcourez-les ou animez-les pour voir le front de chaleur pénétrer dans la
  barre.
- Onglet `Résultats` : affichez **Temperature**. Fixez la plage d'affichage pour
  tous les enregistrements afin que les couleurs aient la même signification sur
  chaque image, sinon l'animation se remet à l'échelle d'elle-même et la
  progression est invisible.
- `Rapport` -> `Points de surveillance` trace la courbe de montée en température
  à l'extrémité froide. Relevez-y la constante de temps : le temps nécessaire
  pour parcourir environ 63 pour cent de l'élévation totale.
- Le dernier enregistrement doit se rapprocher du résultat stationnaire du
  tutoriel 3. Si ce n'est pas le cas, le calcul était trop court.

### Dépannage

| Symptôme | Cause |
|---|---|
| La température oscille et croît d'un pas à l'autre | schéma explicite avec un pas de temps trop grand - passez l'approximation à *Backward difference (stable)* ou réduisez `dt` |
| Le résultat varie à peine sur tout le calcul | le pas de temps est très inférieur à la constante de temps, ou le calcul est trop court |
| Le premier pas saute directement à la réponse finale | le pas de temps est très supérieur à la constante de temps, le schéma est donc en pratique stationnaire |
| Un seul enregistrement a été écrit | la fréquence de sortie vaut `0`, ce qui n'écrit que le dernier pas |
| Le transitoire n'atteint jamais le régime stationnaire | c'est souvent correct - comparez avec le calcul stationnaire avant de soupçonner le solveur |

---

## 5. Vérifier un modèle

Le solveur n'a pas de suite de vérification intégrée ; il vaut donc la peine de
valider un nouveau modèle par rapport à quelque chose que l'on peut calculer à
la main avant de s'y fier. Quatre vérifications peu coûteuses, par ordre
d'utilité :

**Paroi plane.** Pour une plaque d'épaisseur `L` et de conductivité `k`
maintenue à `T1` et `T2` sur ses deux faces, la température est linéaire dans
l'épaisseur et le flux vaut

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Modélisez la plaque avec des éléments volumiques, imposez les deux faces,
vérifiez que le plan médian se trouve exactement à mi-chemin et que le **Heat
flux** calculé correspond à `q`. Cela détecte des unités erronées, une
conductivité erronée et un maillage qui n'est pas connecté comme vous le pensez.
Raffinez le maillage une fois et vérifiez que les deux valeurs ne bougent pas.

**Paroi avec convection.** Pour la même plaque maintenue à `T1` sur une face et
refroidie sur l'autre par un fluide à `Tf` avec un coefficient `h`, la surface
refroidie se stabilise à

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Comparez avec la température de surface calculée. Cela valide la condition de
convection, qui est la partie d'un modèle thermique la plus facile à rater.

**Constante de temps globale.** Pour une vérification transitoire, prenez un
corps de volume `V` et de surface `A` sans température imposée, partant de `T0`
et refroidi uniquement par convection. Tant que le nombre de Biot `h*V/(A*k)`
reste inférieur à environ `0.1`, le corps est presque isotherme et

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Relevez `tau` sur l'historique du point de surveillance et comparez. Cela valide
à la fois la matrice de capacité, la condition de convection et votre pas de
temps - et c'est la manière la plus rapide de choisir un pas de temps pour un
modèle réel, puisque `tau/50` est une bonne première estimation.

**Bilan d'énergie.** Pour un modèle stationnaire avec une puissance d'entrée
connue `P`, la surface convective doit évacuer exactement `P` à la température
finale :

```
P = h * A * ( Ts_mean - Tf )
```

Lisez la température moyenne de surface dans les statistiques du journal du
solveur et vérifiez le bilan. C'est la seule vérification globale qui détecte
une valeur Heat saisie comme un total au lieu d'une densité.

Pour chacune de ces vérifications, raffinez le maillage au moins une fois et
vérifiez que la réponse converge au lieu de dériver.

---

## 6. Limitations

### Limitations de modélisation

- **Conduction linéaire uniquement.** La conductivité, la capacité et la masse
  volumique d'un élément sont constantes pendant une résolution, lues dans la
  table du matériau à la température du calcul précédent. Une propriété
  fortement dépendante de la température nécessite un groupe de tâches itératif
  pour converger.
- **Pas de changement de phase.** Il n'y a ni chaleur latente ni formulation en
  enthalpie ; la fusion, la solidification, l'ébullition et la condensation ne
  sont donc pas modélisées. Une table de matériau ne peut pas représenter le
  palier latent.
- **Pas d'advection.** L'énergie se déplace par conduction à l'intérieur du
  corps et par convection à travers sa frontière. Un milieu en mouvement ne
  transporte pas d'énergie avec lui ; utilisez pour cela *Heat transfer in
  fluids*.
- **La convection est un modèle de frontière.** Les formes corrélées utilisent
  chacune une seule corrélation fixe - plaque plane externe pour l'écoulement
  forcé, plaques horizontales pour la convection naturelle - avec le diamètre
  hydraulique comme longueur caractéristique. L'orientation de la surface, le
  régime d'écoulement et le caractère interne ou externe de l'écoulement ne sont
  pas détectés. Lorsque le coefficient compte, calculez-le vous-même et
  saisissez-le avec *Simple convection*.
- **Le coefficient de convection naturelle est décalé.** Il est évalué à la
  température de paroi de la résolution précédente ; un calcul stationnaire en
  un seul passage utilise donc en pratique l'écart de température initial.
- **Rayonnement uniquement par le type de problème distinct.** Il n'y a pas de
  terme d'émissivité de surface dans le solveur thermique lui-même ; le
  rayonnement d'enceinte nécessite une tâche *Radiative heat transfer*, sa
  matrice de facteurs de forme et une itération entre les deux.
- **Les surfaces sont isothermes dans leur épaisseur.** Une entité surface ne
  conduit que dans son propre plan. Modélisez avec des éléments volumiques une
  paroi dont le gradient dans l'épaisseur compte.
- **Les éléments points ne conduisent pas.** Un point est une capacité
  concentrée et une source de chaleur, rien de plus.
- **Un modèle stationnaire a besoin d'une évacuation.** Sans température imposée
  ni surface convective, la matrice est singulière et la résolution ne
  convergera pas.
