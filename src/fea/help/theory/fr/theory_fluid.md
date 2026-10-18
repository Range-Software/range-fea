# Écoulement des fluides - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur d'écoulement des fluides de Range FEA -
`RSolverFluid`, qui traite le type de problème **Écoulement visqueux
incompressible** (*Incompressible viscous flow*) : les équations qu'il résout,
la signification de chaque donnée d'entrée qu'il accepte, les parties de
l'interface graphique qui le pilotent, et un tutoriel complet.

Les deux types de problèmes résolus sur le champ d'écoulement ont leurs propres
manuels : [Transfert de chaleur dans les fluides](theory_fluid_heat.md)
(`RSolverFluidHeat`) et [Dispersion de contaminants](theory_fluid_particle.md)
(`RSolverFluidParticle`).

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - écoulement stationnaire dans un canal](#3-tutoriel---écoulement-stationnaire-dans-un-canal)
4. [Vérifier un modèle](#4-vérifier-un-modèle)
5. [Limitations](#5-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnues principales - les champs de vitesse et de pression

Les inconnues principales de **Incompressible viscous flow** sont le vecteur
**vitesse** `v` en `m/s` et la **pression** `p` en `Pa`, toutes deux en chaque
nœud. Chaque nœud porte **quatre** degrés de liberté - `vx`, `vy`, `vz` et
`p` - ; un maillage de `N` nœuds donne donc un système de `4N` équations.

Cela fait d'un modèle d'écoulement le type de physique le plus coûteux de
Range FEA : quatre inconnues par nœud contre trois pour un modèle structurel et
une pour un modèle thermique, une matrice ni symétrique ni définie positive, et
l'itération non linéaire de la section 1.6 autour de chaque résolution
linéaire.

Les deux types de problèmes associés ajoutent chacun un scalaire, résolu sur le
champ d'écoulement produit par le solveur de fluide :

| Type de problème | Solveur | Inconnue principale | Unités | Degrés de liberté par nœud |
|---|---|---|---|---|
| Incompressible viscous flow | `RSolverFluid` | vitesse et pression | `m/s`, `Pa` | 4 |
| Heat transfer in fluids | `RSolverFluidHeat` | température | `K` | 1 |
| Contaminant dispersion | `RSolverFluidParticle` | concentration de particules | `kg/m^3` | 1 |

Les deux types associés **nécessitent** une tâche *Incompressible viscous flow*
dans le même problème : ils lisent le champ de vitesse dans son résultat et ne
le calculent jamais eux-mêmes. Chacun est décrit dans son propre manuel -
[Transfert de chaleur dans les fluides](theory_fluid_heat.md) et
[Dispersion de contaminants](theory_fluid_particle.md).

L'état par défaut d'un nœud qu'aucune condition ni aucun résultat précédent
n'atteint est une vitesse nulle et une pression nulle.

### 1.2 Équations fondamentales

Le solveur met en œuvre les **équations de Navier-Stokes incompressibles** pour
un fluide newtonien de masse volumique `rho` et de viscosité dynamique `mu`. Les
deux sont des propriétés de matériau évaluées par élément à sa température -
voir 1.9 pour l'origine de cette température et la façon dont elle rend
l'écoulement soumis à la flottabilité. Les bilans de quantité de mouvement et de
masse s'écrivent

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

et, avec le solveur temporel désactivé, le terme transitoire disparaît, laissant
le bilan stationnaire

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `v` | vitesse | `m/s` | inconnue résolue |
| `p` | pression | `Pa` | inconnue résolue |
| `rho` | masse volumique | `kg/m^3` | propriété du matériau |
| `mu` | viscosité dynamique | `kg/(m*s)`, c'est-à-dire `Pa*s` | propriété du matériau |
| `g` | accélération de la pesanteur | `m/s^2` | condition d'environnement Gravitational acceleration |

La pression n'est pas ici une grandeur thermodynamique. L'incompressibilité en
fait le multiplicateur de Lagrange qui impose `div(v) = 0`, ce qui a deux
conséquences pratiques à bien assimiler avant de lire le moindre résultat :

- Seules les **différences de pression** ont un sens. Dans un modèle où aucune
  condition ne fixe une pression nulle part, le champ de pression n'est
  déterminé qu'à une constante additive près, et la matrice est singulière par
  rapport à cette constante - voir 1.7.
- La pression réagit instantanément dans tout le domaine. Il n'y a ni onde
  acoustique ni vitesse du son dans cette formulation ; une pression appliquée à
  une sortie est donc ressentie à l'entrée au cours de la même résolution.

Le **nombre de Reynolds** construit à partir d'une longueur caractéristique `L`
et d'une vitesse `V`,

```
Re = rho * V * L / mu
```

est le nombre qui décide à lui seul si un modèle d'écoulement va bien se
comporter. En dessous d'environ `Re = 1`, le terme convectif est négligeable et
le problème est presque linéaire ; jusqu'à quelques milliers, l'écoulement est
laminaire et le solveur est dans son élément ; bien au-delà, l'écoulement réel
est turbulent, et ce solveur ne modélise pas la turbulence - voir la section 5.

### 1.3 Discrétisation par éléments finis et stabilisation

La vitesse et la pression sont interpolées avec les **mêmes** fonctions de forme
linéaires sur chaque élément - interpolation d'ordre égal `P1/P1` et `Q1/Q1`.
Cette combinaison ne satisfait pas à elle seule la condition inf-sup
(Babuška-Brezzi) et produirait un champ de pression en damier ; la formulation
est donc **stabilisée**. Trois termes de stabilisation sont ajoutés, chacun avec
son propre paramètre au niveau de l'élément :

| Terme | Rôle |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | stabilise le terme convectif, qui produirait sinon des oscillations d'un nœud à l'autre dans un écoulement dominé par la convection |
| **PSPG** - pressure stabilising Petrov-Galerkin | rend admissibles une vitesse et une pression de même ordre, en supprimant le mode de pression en damier |
| **LSIC** - least-squares on the incompressibility constraint | amortit l'erreur de divergence, améliorant la conservation de la masse élément par élément |

Les paramètres SUPG et PSPG sont construits à partir d'un nombre de Reynolds
d'élément

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`element length along the flow direction` = longueur de l'élément dans la
direction de l'écoulement, `element Reynolds number` = nombre de Reynolds
d'élément, `for` = pour), si bien que le décentrement est fort là où l'élément
est dominé par la convection et s'estompe là où il est dominé par la diffusion.
Le paramètre LSIC vaut `|v| * h / 2`. Ces paramètres appartiennent au solveur
d'écoulement ; le solveur de dispersion utilise son propre paramètre SUPG,
limité par le pas de temps, voir le manuel
[Dispersion de contaminants](theory_fluid_particle.md).

Le paramètre SUPG utilise la vitesse et la longueur **locales** de l'élément ; il
s'adapte donc élément par élément. Le paramètre PSPG utilise à la place une
**vitesse d'écoulement globale** et une échelle de longueur d'élément globale,
ce qui maintient une stabilisation de la pression uniforme sur tout le maillage.
La vitesse d'écoulement est tirée des conditions d'entrée au premier passage de
chaque résolution - à chaque pas de temps dans un calcul transitoire - et non du
champ en cours de résolution,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

(`over inflow surfaces` = sur les surfaces d'entrée, `area of inflow surfaces` =
aire des surfaces d'entrée), c'est-à-dire la vitesse d'entrée moyenne pondérée
par l'aire, avec un repli sur `1 m/s` lorsque le modèle n'a aucune condition
d'entrée. L'échelle de longueur d'élément est déduite du volume de l'élément :
`cbrt(6*V/pi)` pour un tétraèdre, `cbrt(V)` pour un hexaèdre.

**Éléments pris en charge.** Les équations de l'écoulement ne sont assemblées
que sur les **éléments volumiques**, et uniquement sur les **tétraèdres
linéaires** (`TETRA1`) et les **hexaèdres linéaires** (`HEXA1`). Tout autre type
d'élément volumique arrête le calcul avec une erreur nommant le type. Les
éléments surfaces n'interviennent que là où ils portent une condition *Pressure
(implicit)*, où ils apportent une traction de pression et rien d'autre. Les
entités points et lignes n'apportent rien du tout - contrairement à un modèle
thermique, ce ne sont pas des conducteurs de dimension inférieure, elles sont
simplement absentes.

C'est la différence pratique la plus nette avec les autres physiques de
Range FEA : un modèle de fluide doit être un **volume maillé**. Générez le
maillage tétraédrique avec `Géométrie` -> `Volume` ->
`Générer un maillage tétraédrique` avant d'attendre quoi que ce soit d'une
résolution d'écoulement.

### 1.4 Conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Wall | explicite | point, ligne, surface | aucune |
| Wall (frictionless) | explicite | point, ligne, surface | aucune |
| Velocity (inflow) | explicite | surface | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | explicite | surface | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | explicite | surface | Pressure `[Pa]` |
| Pressure (implicit) | naturelle, traction | surface | Pressure `[Pa]` |

**Wall** (paroi) est la condition d'adhérence. Pour chaque nœud de l'entité, les
trois composantes de la vitesse sont retirées du système et maintenues à zéro.
Elle n'a pas de composantes - il n'y a rien à saisir, la cocher constitue toute
la configuration. C'est la condition de toute frontière solide touchée par le
fluide.

**Wall (frictionless)** (paroi sans frottement) est la condition de glissement,
ou de symétrie : le fluide peut se déplacer le long de la surface mais pas la
traverser. Elle est appliquée en recherchant l'**axe global dominant** de la
normale de chaque élément surface et en ne maintenant à zéro que cette seule
composante de la vitesse, les deux autres restant libres. Notez ce que cela
implique : la condition n'est exacte que pour une surface dont la normale est
dirigée selon `x`, `y` ou `z`. Sur une surface inclinée par rapport aux axes
globaux, elle bloque la mauvaise direction, et plus la normale se rapproche de
45 degrés entre deux axes, plus l'approximation est mauvaise. Utilisez-la pour
les plans de symétrie plans d'un modèle aligné sur les axes globaux, et *Wall*
ailleurs.

**Velocity (inflow)** (vitesse d'entrée) impose une vitesse de la norme donnée,
dirigée selon la **normale moyenne** de l'entité surface, vers l'intérieur du
fluide. Le signe est déduit de l'orientation des éléments de l'entité par
rapport au volume situé derrière eux ; une valeur positive signifie donc un
écoulement qui entre dans le domaine, et une valeur négative un écoulement qui
en sort. Vous saisissez une vitesse scalaire, pas un vecteur - la direction
provient de la géométrie.

**Volumetric flow rate (inflow)** (débit volumique d'entrée) impose la même
chose à partir du débit total : le solveur divise le débit par l'aire de
l'entité et applique la vitesse moyenne obtenue selon la même normale moyenne.

```
v = Q / A          A = area of the entity
```

(`area of the entity` = aire de l'entité.) Utilisez-la chaque fois que la
grandeur réellement connue est un débit - les caractéristiques d'une pompe, une
conduite en `m^3/s` - et *Velocity (inflow)* lorsque vous connaissez la vitesse.
Les deux maintiennent les trois composantes de la vitesse en chaque nœud de
l'entité ; toutes deux font donc de l'entrée un écoulement uniforme en bouchon,
et aucune ne peut produire un profil établi sur la face d'entrée. Donnez au
modèle une longueur d'établissement si le profil y compte.

**Pressure (explicit)** (pression explicite) retire le degré de liberté de
pression de chaque nœud de l'entité et le maintient à la valeur donnée. C'est
l'équivalent en pression d'une vitesse imposée, et l'un des moyens de fixer la
constante additive de la section 1.2.

**Pressure (implicit)** (pression implicite) en est le pendant naturel : au lieu
de fixer la pression nodale, elle applique une **traction de pression** sur les
éléments surfaces et y laisse la vitesse libre ; le fluide décide ainsi de la
quantité qui sort et du profil de sortie. La traction comprend une charge
hydrostatique,

```
t = -( p + rho * g * h ) * n
```

où `h` est la hauteur du nœud mesurée **selon la direction de la pesanteur à
partir du point le plus bas** du modèle et `n` est la normale à la surface. La
direction de la pesanteur est la moyenne des conditions d'environnement
*Gravitational acceleration* affectées aux surfaces qui portent cette condition.
Sans pesanteur, le terme de charge disparaît et la traction se réduit à la seule
pression saisie.

C'est la condition de sortie à utiliser dans presque tous les modèles : elle
laisse le profil de sortie s'établir au lieu de l'imposer plat, et saisir `0`
fait de la sortie une référence à la pression atmosphérique. *Pressure
(explicit)* est le choix plus rigide et plus contraignant.

Chaque composante de chacune de ces conditions est une **table en fonction du
temps** ; on peut donc faire monter progressivement une entrée pendant les
premières secondes d'un calcul transitoire, ou donner un cycle de fonctionnement
à un débit.

**Priorité.** Les conditions de vitesse et les conditions de pression sont
indépendantes - elles bloquent des degrés de liberté différents ; une surface
d'entrée peut donc porter une condition de vitesse et une sortie une condition
de pression sans interférence. Parmi les conditions de même nature sur la même
entité, la dernière lue prévaut, et une *Wall* annule la vitesse de ses nœuds
après l'application de toutes les autres conditions de vitesse. Une entité qui
est à la fois une entrée et une paroi est donc une paroi.

### 1.5 Conditions d'environnement et initiales

#### Conditions d'environnement

| Condition d'environnement | Composantes | Effet |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | force volumique `rho*g` sur l'entité, et terme de charge de *Pressure (implicit)* |
| Temperature | Temperature `[K]` | température de fond d'une tâche *Heat transfer in fluids* ; non lue par le solveur d'écoulement lui-même - voir 1.9 |

**Gravitational acceleration** vaut par défaut `(0, 0, -9.80665) m/s^2`,
c'est-à-dire une pesanteur selon `Z` négatif. Affectez-la à l'entité volume pour
la force volumique et à la surface de sortie pour que la charge de *Pressure
(implicit)* soit mesurée dans la bonne direction.

La pesanteur entre dans l'équation de quantité de mouvement sous forme de force
volumique `rho*g`. Dans un domaine fermé de masse volumique uniforme, un champ de
pesanteur uniforme produit un gradient de pression hydrostatique et aucun
mouvement - ce qui est la bonne réponse. Là où la masse volumique varie avec la
température, la force volumique varie avec elle et la différence entraîne un
**écoulement par flottabilité** - la convection naturelle, voir 1.9. Le
résultat de pression contient toujours la partie hydrostatique
`rho*(g . x)` ; rien n'est soustrait.

#### Conditions initiales

| Condition initiale | Effet |
|---|---|
| Velocity | champ de vitesse initial, appliqué au premier calcul |
| Pressure | champ de pression initial, appliqué au premier calcul |

Les champs initiaux de température et de concentration des types de problèmes
associés sont décrits dans leurs propres manuels.

Les conditions initiales ne sont lues **qu'au premier calcul** ; ensuite, les
champs avancent à partir du résultat précédent. C'est ce qui fait fonctionner
le déroulement avec redémarrage de la section 1.7 : un calcul transitoire lancé
avec **Redémarrer le solveur / continuer** reprend le champ stationnaire convergé
au lieu de repartir du repos.

Une condition aux limites l'emporte sur une condition initiale là où les deux
se rencontrent.

### 1.6 L'itération non linéaire

Le terme convectif `(v . grad) v` est quadratique en l'inconnue ; les équations
de l'écoulement sont donc **non linéaires** et ne peuvent pas être résolues en
un seul passage. Le solveur les linéarise et itère : chaque passage assemble le
résidu des bilans de quantité de mouvement et de masse pour le champ courant,
résout un **incrément**, et l'ajoute,

```
J * dx = -R(x)
x <- x + dx
```

où `x` contient les vitesses et pressions nodales. Le premier membre est un
jacobien approché et non exact, ce qui fait du schéma une itération de Newton
modifiée : elle converge plus lentement qu'un Newton complet près de la
solution, mais la solution vers laquelle elle converge est fixée par le
**résidu** `R`, qui est l'ensemble des équations de Navier-Stokes discrétisées
elles-mêmes.

**Le pas est amorti.** Comme la matrice n'est pas la dérivée exacte du résidu,
un pas complet `dx` peut dépasser la cible et laisser le champ plus loin d'une
solution qu'au départ - le résidu du passage suivant est alors plus élevé que le
précédent. Le solveur surveille précisément ce cas et n'applique qu'une fraction
du pas calculé,

```
x <- x + omega * dx
```

en partant de `omega = 1`. Un passage qui a aggravé le résidu le divise par
deux, jusqu'à un plancher de `0.1` ; un passage qui l'a amélioré laisse `omega`
remonter d'un quart à la fois, si bien que le recul est rapide et la reprise
prudente. La valeur en vigueur est affichée comme `Relaxation` dans le journal,
et elle reste à `1` pour un calcul qui ne dépasse jamais - un modèle
d'écoulement qui se comporte bien ne remarque jamais la présence du mécanisme.
`omega` est remis à `1` au début de chaque résolution, ce qui, dans un calcul
transitoire, signifie à chaque pas de temps.

Deux conséquences en découlent, et elles dictent tout le déroulement d'un modèle
d'écoulement :

- **Un passage ne suffit jamais.** Une seule itération d'une tâche d'écoulement
  renvoie ce qu'a donné le premier incrément, qui n'est la solution de rien. Une
  tâche d'écoulement doit être placée dans un **groupe de tâches du problème**
  avec un nombre d'itérations - des centaines pour un calcul stationnaire, des
  dizaines par pas de temps pour un calcul transitoire.
- **Le nombre est une borne supérieure, pas un objectif.** Le groupe porte
  aussi une **valeur de convergence**, et il termine ses itérations dès que la
  tâche d'écoulement indique une convergence inférieure à celle-ci. Réglez le
  nombre assez haut pour le pas le plus difficile et laissez la valeur de
  convergence arrêter chacun une fois stabilisé.

Après chaque itération, le solveur écrit quatre nombres dans le fichier de
convergence et dans le journal :

| Grandeur | Signification |
|---|---|
| `Residual` | norme du vecteur résidu, la distance à une solution |
| `Convergence-R` | variation du résidu depuis l'itération précédente |
| `Convergence-V` | taille relative de l'incrément de vitesse, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | taille relative de l'incrément de pression, `\|\|dp\|\| / \|\|p\|\|` |

`Convergence-V` et `Convergence-P` sont celles que l'on compare à la valeur de
convergence. Toutes deux sont sans dimension et toutes deux diminuent à mesure
que l'itération se stabilise : elles mesurent la taille du pas que fait encore le
champ par rapport à la taille du champ lui-même ; une valeur de `1e-5` signifie
donc que la solution a bougé d'un cent-millième d'elle-même lors de ce passage.

Un petit pas ne constitue cependant pas à lui seul une solution. Un système
presque singulier - un modèle sans référence de pression, ou un maillage
incapable de porter le nombre de Reynolds demandé - fait de tout petits pas
parce qu'il **ne peut pas bouger**, et non parce qu'il est arrivé, et son résidu
stagne ou augmente. Le test comporte donc deux parties, et le groupe ne s'arrête
que lorsque les deux sont satisfaites :

| Partie | Condition |
|---|---|
| Le champ a cessé de bouger | `Convergence-V` et `Convergence-P` sont toutes deux inférieures à la valeur de convergence |
| Le champ a réellement diminué | le résidu est tombé au **dixième** de ce qu'il était au premier passage de cette résolution |

La seconde partie est fixe et non configurable - c'est un contrôle de cohérence
de la première, et non un second réglage. Le journal l'affiche comme
`Residual ratio` à côté de sa cible ; on voit ainsi à quel point un calcul qui
ne s'est pas arrêté plus tôt était près de le faire. Dans un calcul transitoire,
chaque pas de temps est une résolution à part entière ; le rapport est donc
mesuré de nouveau à partir du début de chaque pas.

`Rapport` -> `Convergence du solveur` trace les quatre nombres. Un calcul
stationnaire convergé montre un résidu qui chute de plusieurs ordres de grandeur
puis se stabilise, avec `Convergence-V` et `Convergence-P` qui décroissent de
façon monotone vers zéro. Un résidu qui se stabilise à un niveau élevé, oscille
ou augmente signifie que le calcul n'a pas convergé - et le groupe ne s'arrêtera
pas plus tôt dans un tel calcul, quoi que fassent les incréments.

Une valeur de convergence de `0` désactive l'arrêt anticipé et exécute le nombre
complet d'itérations, comme le faisait le solveur avant l'existence de cette
valeur. Utilisez-la lorsque vous voulez une quantité de travail fixe par pas,
quel que soit le comportement du résidu.

### 1.7 Analyse stationnaire

Avec le solveur temporel désactivé, le solveur itère les bilans stationnaires de
quantité de mouvement et de masse jusqu'à un point fixe. Le système linéaire de
chaque itération n'est pas symétrique ; il est donc résolu par **GMRES** avec un
préconditionnement de Jacobi plutôt que par la méthode du gradient conjugué
utilisée par les physiques symétriques.

Un modèle d'écoulement stationnaire nécessite au minimum :

- un **volume** maillé avec une masse volumique et une viscosité dynamique
  affectées ;
- **Wall** sur chaque frontière solide ;
- une condition d'**entrée**, ou une différence de pression, pour entraîner
  l'écoulement ;
- une **référence de pression** - une surface *Pressure (explicit)* ou *Pressure
  (implicit)*. Sans elle, la pression n'est déterminée qu'à une constante près
  et l'itération ne se stabilisera pas.

Un modèle dont les frontières ne sont que des parois et des vitesses imposées
comporte un piège supplémentaire : les débits imposés en entrée et en sortie
doivent s'**équilibrer**, car un fluide incompressible ne peut pas s'accumuler.
Imposer 5 m^3/s en entrée et 4 m^3/s en sortie demande l'impossible et la
résolution ne convergera pas. Laisser la sortie en *Pressure (implicit)* évite
entièrement la question, ce qui est la principale raison de la préférer.

Le déroulement habituel consiste à faire d'abord converger un calcul
stationnaire et à l'utiliser comme champ de départ d'un calcul transitoire, car
un calcul transitoire démarré à partir du repos consacre ses premiers pas de
temps au transitoire de démarrage plutôt qu'à la physique qui vous intéresse.

### 1.8 Analyse transitoire

Avec le solveur temporel activé, le solveur intègre en temps les équations
instationnaires avec un schéma theta, où `alpha` est le coefficient
d'approximation du schéma temporel :

| Approximation | `alpha` | Schéma obtenu | Stabilité |
|---|---|---|---|
| Backward difference (stable) | `1` | pas entièrement implicite | inconditionnelle |
| Central difference (accurate) | `0.5` | Crank-Nicolson | inconditionnelle, peut osciller |
| Forward difference (fast) | `0` | pas explicite | conditionnelle |

**Backward difference** est le réglage à utiliser pour un modèle d'écoulement.
C'est l'itération non linéaire à l'intérieur de chaque pas de temps qui coûte du
temps, et un schéma implicite permet de choisir le pas de temps d'après la
physique plutôt que d'après le maillage.

Même avec le schéma implicite, le pas de temps d'un modèle d'écoulement est
limité par la précision, à travers le **nombre de Courant**

```
C = |v| * dt / h
```

où `h` est la taille de l'élément dans la direction de l'écoulement. Un nombre
de Courant proche de `1` signifie que le fluide traverse environ un élément par
pas, ce qui est le bon ordre de grandeur pour résoudre un transitoire ; bien
au-delà, le solveur étale le transport qu'il est censé résoudre, quelle que soit
la stabilité du schéma. Choisissez `dt` d'après le maillage et la vitesse
attendue :

```
dt ~ h / |v|
```

Chaque pas de temps exécute l'itération non linéaire complète de la
section 1.6 ; le coût d'un calcul transitoire est donc le nombre de pas de temps
multiplié par les itérations par pas. Dix à cinquante itérations par pas est une
plage typique une fois le champ d'écoulement déjà convergé par un calcul
stationnaire. Un pas dont le résidu n'a pas diminué à la fin de ses itérations
est un pas dont la réponse n'est pas convergée, et l'erreur se reporte sur tous
les pas qui suivent.

### 1.9 Transfert de chaleur et dispersion de contaminants

Deux types de problèmes sont résolus sur le champ d'écoulement produit par ce
solveur, chacun avec son propre manuel :

- **Heat transfer in fluids** (`RSolverFluidHeat`) transporte la température
  avec l'écoulement et se couple au solveur thermique d'un solide maillé - voir
  [Transfert de chaleur dans les fluides](theory_fluid_heat.md) ;
- **Contaminant dispersion** (`RSolverFluidParticle`) transporte une
  concentration de contaminant avec l'écoulement - voir
  [Dispersion de contaminants](theory_fluid_particle.md).

Les deux lisent la vitesse dans le résultat de l'écoulement. La dispersion de
contaminants ne modifie jamais l'écoulement. Le transfert de chaleur dans les
fluides le modifie, à travers les propriétés du matériau.

#### Propriétés dépendant de la température et flottabilité

À **chaque passage**, le solveur d'écoulement évalue la masse volumique et la
viscosité dynamique de chaque élément à partir des tables du matériau, à la
température de l'élément laissée par le dernier passage du transfert de
chaleur. Sans tâche de transfert de chaleur dans le problème, cette température
vaut partout la valeur par défaut `293.15 K`. La condition d'environnement
*Temperature* n'est pas lue directement par le solveur d'écoulement ; elle
n'atteint l'écoulement qu'à travers une tâche de transfert de chaleur.

C'est la masse volumique dans la force volumique `rho(T)*g` qui crée la
flottabilité : le fluide chaud est plus léger que le fluide qui l'entoure et
monte, le fluide froid descend. La convection naturelle, les panaches thermiques
et la stratification sont donc obtenus avec un groupe composé d'une tâche
*Incompressible viscous flow* et d'une tâche *Heat transfer in fluids*, à
condition que le matériau ait une **table de masse volumique dépendant de la
température**. C'est le cas des matériaux fournis *Water* et *Air*. Un matériau
dont la masse volumique est une valeur unique ne donne aucune flottabilité, quel
que soit l'écart de température.

Comment les deux tâches échangent leurs données :

- **Régime stationnaire.** Placez les deux tâches dans un même groupe, la tâche
  d'écoulement en premier, avec plusieurs itérations et une valeur de
  convergence. Le premier passage de l'écoulement voit la température initiale -
  la condition initiale *Temperature*, ou `293.15 K` - et n'a donc pas encore de
  flottabilité. À partir de la deuxième itération, chaque passage de
  l'écoulement voit la température du passage thermique précédent. Un modèle
  entraîné par la seule flottabilité nécessite assez d'itérations pour que les
  deux se stabilisent ensemble ; un groupe à une seule itération ne les couple
  jamais.
- **Régime transitoire.** Chaque pas de temps exécute de nouveau le groupe, et
  chaque passage de l'écoulement voit la dernière température. Avec une
  itération par pas de temps, l'écoulement utilise la température du pas
  précédent - un décalage d'un pas, acceptable tant que le pas de temps résout le
  transitoire thermique. Plusieurs itérations par pas le suppriment.

Le groupe se termine lorsque la tâche d'écoulement converge, car une tâche
*Heat transfer in fluids* non couplée se déclare convergée à chaque passage. Une
variation de la température modifie le résidu de l'écoulement par `rho(T)` et
`mu(T)` ; un écoulement convergé est donc aussi un écoulement que la
température ne pousse plus.

La formulation conserve `rho(T)` dans tous les termes du bilan de quantité de
mouvement et `div(v) = 0` pour la masse. Pour les faibles différences de masse
volumique de la convection naturelle dans un liquide, ou dans un gaz sur
quelques dizaines de kelvins, cela donne la même réponse que l'approximation de
Boussinesq. Ce n'est pas une formulation compressible : la dilatation thermique
du fluide n'entre pas dans le bilan de masse, si bien qu'un gaz chauffé de
plusieurs centaines de kelvins sort de son domaine de validité.

### 1.10 Résultats dérivés

| Résultat | S'applique à | Produit par | Signification |
|---|---|---|---|
| Velocity `[m/s]` | nœud | écoulement | le champ de vitesse résolu, trois composantes |
| Pressure `[Pa]` | nœud | écoulement | le champ de pression résolu |

Les résultats des types de problèmes associés sont listés dans leurs propres
manuels.

La vitesse est stockée comme **vecteur** aux nœuds ; l'onglet `Résultats` peut
donc afficher sa norme sous forme de couleur, ses composantes une à une, ou des
flèches. Deux outils d'affichage méritent d'être connus spécifiquement pour un
résultat d'écoulement :

- un **plan de coupe** à travers le volume, car la structure intéressante d'un
  écoulement se trouve à l'intérieur du domaine plutôt qu'à sa surface ;
- les **lignes de courant**, créées par `Géométrie` -> `Ligne de courant`, qui
  tracent le champ de vitesse et sont la manière la plus rapide de voir une
  recirculation ou une zone morte.

Après chaque enregistrement, le journal du solveur affiche les statistiques de
la vitesse et de la pression, et les valeurs aux éventuels points de
surveillance.

### 1.11 Stationnaire ou transitoire ?

| | Stationnaire | Transitoire |
|---|---|---|
| Résout | le bilan stationnaire, itéré | les équations instationnaires, intégrées en temps |
| Répond à | « à quoi ressemble l'écoulement établi » | « comment l'écoulement y parvient-il, et que transporte-t-il » |
| Itérations | des centaines, dans un groupe de tâches | des dizaines par pas de temps |
| Enregistrements | un | un par pas de temps écrit |
| Nécessite une référence de pression | oui | oui |

**Utilisez une analyse stationnaire pour** le champ d'écoulement établi d'une
conduite, d'un collecteur ou d'un passage d'échangeur de chaleur, la perte de
charge à travers un composant, et comme champ de départ de tout calcul
transitoire. C'est par là que commencent presque tous les modèles
d'écoulement.

**Utilisez une analyse transitoire pour** le démarrage et l'arrêt, une entrée
pulsée ou cyclique, le détachement tourbillonnaire et - avant tout - pour la
**dispersion de contaminants**, qui est un problème de transport dans le temps
sans réponse stationnaire pertinente pour un événement de rejet.

Le déroulement en deux étapes du tutoriel *Dispersion de contaminants dans les
fluides* est la méthode de référence : faire converger l'écoulement en
stationnaire, puis redémarrer avec le solveur temporel et la tâche de dispersion
activés - voir le manuel [Dispersion de contaminants](theory_fluid_particle.md).

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`,
`Conditions d'environnement`, `Matériau`, `Résultats`), le menu **Problème**
et le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Ajoutez une tâche et choisissez
**Incompressible viscous flow** - « Steady-state and transient flow of
newtonian fluids » (écoulement stationnaire et transitoire de fluides
newtoniens).

Une tâche d'écoulement doit se trouver dans un **groupe de tâches**, qui porte
les deux valeurs pilotant l'itération non linéaire de 1.6. Double-cliquez sur
l'une ou l'autre pour la modifier :

| Valeur | Signification |
|---|---|
| **Nb d'itérations** | le nombre maximal de passages qu'une résolution - ou un pas de temps - peut effectuer |
| **Convergence** | le groupe termine ses passages une fois que la tâche d'écoulement s'est stabilisée en dessous de cette valeur **et** que son résidu a diminué ; `0` exécute toujours le nombre complet |

La valeur de convergence vaut par défaut `1e-5`, ce qui est une taille de pas
relative sûre pour un modèle d'écoulement. Placer une tâche d'écoulement hors
d'un groupe, où elle ne s'exécute qu'une fois, n'est pas utile - voir 1.6.

Les combinaisons à connaître :

- *Incompressible viscous flow* seul, dans un groupe de plusieurs centaines
  d'itérations : un champ d'écoulement stationnaire ;
- *Incompressible viscous flow* et *Contaminant dispersion* dans un même
  groupe : la dispersion sur l'écoulement, l'association transitoire classique,
  voir [Dispersion de contaminants](theory_fluid_particle.md) ;
- *Incompressible viscous flow* et *Heat transfer in fluids* dans un même
  groupe : un écoulement chauffé ou refroidi, voir
  [Transfert de chaleur dans les fluides](theory_fluid_heat.md).

L'écoulement exclut *Flow through porous media* et *Modal analysis* ; les deux
types de problèmes associés nécessitent chacun la présence d'une tâche
d'écoulement.

### 2.2 Onglet Problème

**Solveur temporel** - affiché pour les trois types de problèmes de fluides, car
tous dépendent du temps. Laissez-le désactivé pour un calcul d'écoulement
stationnaire.

| Champ | Signification |
|---|---|
| Activer | active ou désactive la marche en temps transitoire |
| Approximation | theta du schéma temporel : arrière, centré ou avant, voir 1.8 |
| Temps de début | première valeur de temps en secondes ; un redémarrage à partir d'un résultat stationnaire place ce résultat au temps de début, un redémarrage d'un calcul transitoire interrompu reprend à partir de son dernier pas |
| Temps de fin | `start time + time-step size * number of time-steps` (temps de début + taille du pas de temps × nombre de pas de temps), lecture seule |
| Taille du pas de temps | `dt` en secondes, voir la règle de Courant en 1.8 |
| Nombre de pas de temps | nombre de pas à calculer |
| Fréquence de sortie | écrit un fichier de résultats tous les N pas ; `0` n'écrit que le dernier pas |

Le type de problème d'écoulement n'a pas de groupe de paramètres propre ; le
groupe *Paramètres de la dispersion de contaminants* est décrit dans le manuel
[Dispersion de contaminants](theory_fluid_particle.md).
Tout le reste s'affecte par entité dans les onglets des conditions, et le nombre
d'itérations - qui compte ici plus que partout ailleurs dans Range FEA - se
trouve dans la fenêtre d'enchaînement des tâches du problème et non dans cet
onglet.

### 2.3 Onglet Matériau

L'onglet `Matériau` liste le matériau affecté à l'entité sélectionnée. Le type
de problème d'écoulement requiert :

| Propriété | Unités | Commentaire |
|---|---|---|
| Density | `kg/m^3` | `rho`, environ `1000` pour l'eau, `1.2` pour l'air |
| Dynamic viscosity | `kg/(m*s)` | `mu`, environ `1.0e-3` pour l'eau, `1.8e-5` pour l'air |

Les types de problèmes associés listent leurs propres propriétés dans leurs
manuels. Une entité à laquelle il manque une propriété requise par un type de
problème de l'enchaînement des tâches n'est pas résolue, et le contrôle de la
configuration avertit avant le début du calcul. La base de données de matériaux
fournie contient **Water** (eau) et **Air**, ce qui constitue généralement toute
cette étape.

Notez que la viscosité attendue est la viscosité **dynamique** `mu` en `Pa*s`,
et non la viscosité cinématique `nu = mu/rho` en `m^2/s`. Saisir `1.0e-6` pour
l'eau parce que c'est la valeur familière est l'erreur de matériau la plus
courante dans un modèle d'écoulement, et elle rend le fluide mille fois moins
visqueux qu'il ne devrait l'être - ce qui se manifeste généralement par un
calcul qui ne converge pas.

Chaque propriété est une table en fonction de la température, évaluée à la
température de l'élément du calcul précédent.

### 2.4 Onglet Conditions aux limites

L'onglet est divisé en une liste de gestion des conditions aux limites
disponibles en haut et un éditeur de la condition sélectionnée en bas.
Sélectionnez d'abord une entité dans l'arbre `Modèle` ; le gestionnaire propose
alors les conditions valables pour ce type d'entité et pour les types de
problèmes sélectionnés. Cochez-en une pour l'affecter, puis modifiez ses
composantes dans l'arbre inférieur.

Les conditions de la section 1.4 sont proposées. Remarques sur leur
utilisation :

- **Wall** se place sur chaque frontière solide et ne prend aucune valeur. Une
  entité surface d'un volume maillé qui ne porte aucune condition n'est *pas*
  une paroi - c'est une frontière non contrainte, ce qui est rarement ce que
  l'on veut. Les parois sont la première chose à vérifier dans un modèle qui se
  comporte bizarrement.
- **Wall (frictionless)** est destinée aux plans de symétrie alignés sur les
  axes globaux. Sur une surface inclinée, elle bloque la mauvaise direction -
  voir 1.4.
- **Velocity (inflow)** et **Volumetric flow rate (inflow)** font toutes deux
  entrer l'écoulement par une surface selon sa normale moyenne. Choisissez celle
  qui correspond à la valeur dont vous disposez réellement. Une valeur négative
  entraîne l'écoulement dans l'autre sens, ce qui est une manière de définir une
  sortie.
- **Pressure (implicit)** est la condition de sortie à privilégier : elle laisse
  le profil de sortie s'établir et, saisie à `0`, fait de la sortie la référence
  de pression. **Pressure (explicit)** fixe à la place la pression nodale.
- Les conditions de *Heat transfer in fluids* et de *Contaminant dispersion*
  apparaissent dès que leur tâche figure dans l'enchaînement des tâches, et
  sont décrites dans leurs propres manuels.
- Chaque composante est une table en fonction du temps. Cliquez sur **Modifier
  les valeurs dépendant du temps** pour ouvrir l'éditeur de composantes, où les
  valeurs se saisissent en fonction du temps ; entre deux instants donnés, la
  valeur est interpolée linéairement, et après le dernier instant, la dernière
  valeur est conservée. C'est ainsi que l'on modélise un événement de rejet, une
  entrée à montée progressive ou un cycle de fonctionnement.

### 2.5 Conditions initiales et d'environnement

L'onglet `Conditions initiales` propose *Velocity* et *Pressure* pour un
problème d'écoulement. Elles définissent le champ de départ du premier calcul et
permettent à un calcul transitoire de partir d'autre chose que du repos.

L'onglet `Conditions d'environnement` propose *Gravitational acceleration*, qui
vaut par défaut `(0, 0, -9.80665) m/s^2`. Affectez-la au volume pour la force
volumique, et à toute surface portant *Pressure (implicit)* pour que la charge
hydrostatique de cette condition soit mesurée dans la bonne direction.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur d'écoulement utilise l'entrée **GMRES**, car sa
matrice n'est pas symétrique.

Distinguez les deux itérations imbriquées avant de modifier quoi que ce soit
ici : les itérations du **solveur matriciel** résolvent un système linéarisé,
tandis que les itérations du **groupe de tâches** pilotent la non-linéarité de
la section 1.6. Un modèle d'écoulement qui ne converge pas a presque toujours
besoin de plus d'itérations du groupe de tâches, d'un meilleur maillage ou d'un
jeu de conditions aux limites corrigé - et non d'un nombre d'itérations GMRES
plus élevé.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer - *Velocity* ou
*Pressure* pour l'écoulement. Dans un calcul transitoire, l'historique est
affiché par `Rapport` -> `Points de surveillance`.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste les variables calculées et contrôle la vue 3D. La
vitesse est un vecteur aux nœuds et la pression un scalaire aux nœuds.

L'onglet `Enregistrements` du dock `Modèle` liste les enregistrements de
résultats :

- **analyse stationnaire** - un seul enregistrement ;
- **analyse transitoire** - un enregistrement par pas de temps écrit.
  L'animation des enregistrements montre l'écoulement, et le bouton rouge
  d'enregistrement en bas de l'arbre écrit l'animation dans un fichier vidéo.

`Rapport` -> `Convergence du solveur` trace l'historique de convergence décrit
en 1.6, et c'est le rapport à ouvrir en premier après tout calcul d'écoulement.
`Rapport` -> `Fichier journal du solveur` affiche la sortie complète du solveur,
y compris les statistiques de vitesse et de pression après chaque
enregistrement.

---

## 3. Tutoriel - écoulement stationnaire dans un canal

**Objectif.** Calculer le champ d'écoulement établi et la perte de charge d'un
fluide entraîné à travers un canal, et confirmer que le résultat est convergé.

Ce tutoriel suppose un modèle volumique maillé - le modèle fourni
**Channel.tmsh** correspond exactement à ce cas, et le tutoriel *Dispersion de
contaminants dans les fluides* l'utilise également. Si vous construisez le
vôtre, dessinez un parallélépipède, maillez-le en tétraèdres, et marquez la face
d'entrée, la face de sortie et les faces restantes comme trois entités surface
distinctes afin de pouvoir leur affecter des conditions.

### Étape 1 - choisir le problème

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez un **groupe de tâches** et réglez son **Nb d'itérations** sur `2000`
   en double-cliquant sur la valeur.
3. Laissez sa **Convergence** à la valeur par défaut `1e-5`.
4. Ajoutez une tâche **Incompressible viscous flow** dans le groupe.
5. Confirmez avec `OK`.

Les deux valeurs sont tout l'enjeu de cette étape. Le calcul effectue jusqu'à
2000 passages et s'arrête dès que le champ se stabilise en dessous de `1e-5` -
voir 1.6. Réglez le nombre généreusement : c'est un plafond, et c'est la valeur
de convergence qui termine normalement le calcul.

### Étape 2 - générer le maillage

`Géométrie` -> `Volume` -> `Générer un maillage tétraédrique`.

Un modèle d'écoulement n'est résolu que sur des éléments volumiques. Sans
maillage volumique, le solveur n'a rien à assembler.

### Étape 3 - affecter le matériau

1. Sélectionnez l'entité volume dans l'arbre `Modèle`.
2. Ouvrez l'onglet `Matériau` et affectez **Water** (eau), ou saisissez :
   - **Density** (masse volumique) = `1000` `kg/m^3`
   - **Dynamic viscosity** (viscosité dynamique) = `1.0e-3` `kg/(m*s)`

Vérifiez que la viscosité est bien la viscosité dynamique en `Pa*s` et non la
viscosité cinématique - voir 2.3.

### Étape 4 - garder l'analyse stationnaire

Ouvrez l'onglet `Problème` et laissez **Activer** décoché dans le groupe
*Solveur temporel*.

### Étape 5 - affecter les parois

1. Sélectionnez l'entité surface qui couvre les frontières solides du canal.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Wall**.

Il n'y a aucune valeur à saisir. C'est la condition d'adhérence, et c'est elle
qui fait que l'écoulement développe un profil au lieu de glisser en bloc.

### Étape 6 - entraîner l'écoulement

1. Sélectionnez l'entité surface d'entrée.
2. Cochez **Volumetric flow rate (inflow)** et saisissez le débit en `m^3/s` -
   ou cochez plutôt **Velocity (inflow)** et saisissez une vitesse en `m/s`.

Avant de continuer, estimez le nombre de Reynolds à partir de la vitesse moyenne
et de la largeur du canal :

```
Re = rho * V * L / mu
```

En dessous de quelques milliers, le modèle est dans le domaine que ce solveur
traite. Bien au-delà, attendez-vous à ce que le calcul ne converge pas, et lisez
la section 5 avant d'y consacrer du temps.

### Étape 7 - ouvrir la sortie

1. Sélectionnez l'entité surface de sortie.
2. Cochez **Pressure (implicit)** et réglez **Pressure** = `0` `Pa`.

C'est l'étape qui rend le modèle soluble. Elle donne au champ de pression sa
référence et laisse le profil de sortie s'établir au lieu de l'imposer plat. Un
modèle avec une entrée et des parois mais sans aucune condition de pression a
une pression singulière et ne convergera pas.

### Étape 8 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Le contrôle de la configuration signale les matériaux ou les conditions aux
   limites manquants avant le début du calcul.
3. Suivez la progression dans `Rapport` -> `Fichier journal du solveur`.

Cela peut prendre un certain temps - un modèle d'écoulement de quelque taille
que ce soit représente un vrai travail, et le journal affiche un bloc par
itération. Surveillez `Convergence target` dans le journal : le passage qui
l'atteint est signalé, puis le groupe indique *All sub-tasks have converged*
(toutes les sous-tâches ont convergé) et s'arrête.

### Étape 9 - vérifier la convergence

`Rapport` -> `Convergence du solveur`.

Faites-le avant de regarder la moindre image. Le résidu doit chuter de plusieurs
ordres de grandeur puis se stabiliser, et `Convergence-V` et `Convergence-P`
doivent tendre vers zéro. Si le résidu chute encore fortement à la dernière
itération, le calcul était trop court - augmentez le nombre d'itérations et
redémarrez. S'il oscille ou augmente, la réponse n'est pas une solution et c'est
la section 5 qu'il faut consulter.

### Étape 10 - examiner les résultats

- Onglet `Résultats` : affichez **Velocity**. Le profil à travers le canal doit
  être régulier, nul aux parois et maximal près du centre.
- Coupez le volume par un plan - la structure intéressante d'un écoulement se
  trouve à l'intérieur du domaine, pas à sa surface.
- Affichez **Pressure**. Elle doit décroître de façon monotone de l'entrée vers
  la sortie, et valoir `0` à la sortie, où vous l'avez fixée.
- Créez des **lignes de courant** avec `Géométrie` -> `Ligne de courant` pour
  voir d'un coup les recirculations ou les zones mortes.
- Vérifiez le bilan de masse : la vitesse moyenne de sortie multipliée par
  l'aire de sortie doit être égale au débit volumique imposé à l'entrée. C'est la
  vérification globale la plus rapide que le modèle fait ce que vous lui avez
  demandé - voir la section 4.

### Dépannage

| Symptôme | Cause |
|---|---|
| Le résidu ne diminue jamais | aucune condition de pression nulle part, la pression est donc indéterminée |
| Le résidu oscille ou augmente | le nombre de Reynolds est trop élevé pour le maillage, ou la viscosité a été saisie en viscosité cinématique |
| L'écoulement glisse sans profil | pas de condition *Wall* sur les frontières solides |
| Le résultat ressemble à la première itération | la tâche n'a pas été placée dans un groupe de tâches, elle ne s'est donc exécutée qu'une fois |
| Le calcul s'est arrêté bien avant le nombre d'itérations | il a convergé - les incréments et le résidu ont tous deux satisfait le test de 1.6 |
| Le calcul ne s'arrête jamais plus tôt, quelle que soit la souplesse de la valeur de convergence | le résidu ne diminue pas - lisez `Residual ratio` dans le journal, et considérez le champ d'écoulement comme non convergé |
| *Failed to calculate element scales. Unsupported element type* (impossible de calculer les échelles des éléments, type d'élément non pris en charge) | le maillage contient des éléments volumiques autres que des tétraèdres ou hexaèdres linéaires |
| Rien n'a été résolu | aucun maillage volumique n'a été généré, ou il manque au matériau la masse volumique ou la viscosité |
| Les débits imposés en entrée et en sortie ne s'équilibrent pas | un fluide incompressible ne peut pas s'accumuler - utilisez *Pressure (implicit)* à la sortie |

---

## 4. Vérifier un modèle

Le solveur n'a pas de suite de vérification intégrée ; il vaut donc la peine de
valider un nouveau modèle par rapport à quelque chose que l'on peut calculer à
la main avant de s'y fier. Quatre vérifications peu coûteuses, par ordre
d'utilité :

**Bilan de masse.** Pour tout modèle incompressible convergé, ce qui entre doit
sortir :

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(`over the inlets` = sur les entrées, `over the outlets` = sur les sorties.)
Relevez la vitesse moyenne sur la sortie dans les statistiques du journal du
solveur, multipliez-la par l'aire de sortie et comparez avec le débit d'entrée
imposé. Cette seule vérification globale détecte un calcul non convergé, une
paroi manquante et une condition aux limites sur la mauvaise entité, et elle ne
coûte rien.

**Hydrostatique.** Désactivez toutes les entrées, laissez le domaine fermé par
des parois, affectez une condition d'environnement *Gravitational acceleration*
et résolvez. Le fluide doit s'immobiliser et la pression doit varier
linéairement avec la profondeur :

```
p(h) = p_ref + rho * g * h
```

Tout mouvement résiduel est d'origine numérique. Cela valide en un seul calcul
la force volumique de pesanteur, la référence de pression et les unités du
matériau, et c'est le premier modèle le plus rapide à construire dans une
nouvelle configuration.

**Écoulement de Poiseuille plan.** Pour un écoulement laminaire stationnaire
entre deux plaques parallèles distantes de `H`, le profil est parabolique, la
vitesse sur l'axe vaut `1.5` fois la vitesse moyenne, et le gradient de pression
vaut

```
dp/dx = 12 * mu * V_mean / H^2
```

Modélisez un canal droit avec des parois en haut et en bas, une entrée imposée
et une sortie *Pressure (implicit)*, et comparez avec ces valeurs la vitesse
calculée sur l'axe et la perte de charge. C'est la vérification la plus fine
disponible : elle valide d'un coup le terme visqueux, la paroi adhérente et les
unités de la viscosité. Donnez au modèle une longueur d'établissement suffisante
pour que le profil se développe avant la mesure, car la condition d'entrée
impose un profil plat.

**Écoulement de Hagen-Poiseuille.** La même vérification dans un tube rond de
diamètre `D` et de longueur `L`, où la vitesse sur l'axe vaut `2` fois la
vitesse moyenne et

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

valable tant que l'écoulement est laminaire, c'est-à-dire en dessous d'environ
`Re = 2300`.

Pour chacune de ces vérifications, raffinez le maillage au moins une fois et
vérifiez que la réponse converge au lieu de dériver - et vérifiez que
l'historique de convergence s'est stabilisé avant de comparer quoi que ce soit.
Un modèle d'écoulement non convergé échouera à toutes ces vérifications pour la
même raison.

---

## 5. Limitations

### Limitations de modélisation

- **Pas de modèle de turbulence.** Le solveur intègre les équations de
  Navier-Stokes laminaires. Les termes SUPG, PSPG et LSIC sont une
  stabilisation numérique, et non une fermeture de turbulence - ils ne
  représentent ni le mélange ni la dissipation supplémentaire d'un écoulement
  turbulent. Au-delà d'environ `Re = 2300` dans un tube, l'écoulement réel est
  turbulent et ce solveur ne le modélise pas : le calcul peut tout de même
  converger, mais la réponse décrit un écoulement laminaire qui n'existe pas. Il
  n'y a ni modèle `k`-`e`, ni loi de paroi, ni formulation des grandes échelles.
- **Incompressible et newtonien uniquement.** Le bilan de masse est
  `div(v) = 0` et la contrainte visqueuse est linéaire en fonction du taux de
  déformation. La masse volumique et la viscosité peuvent dépendre de la
  température, mais pas de la pression. Les écoulements compressibles, les
  effets du nombre de Mach, les surfaces libres, les écoulements multiphasiques
  et le comportement non newtonien sont hors de la formulation.
- **Flottabilité sans dilatation du fluide.** Une masse volumique dépendant de
  la température entraîne la convection naturelle par `rho(T)*g` (voir 1.9),
  mais la dilatation thermique n'entre jamais dans le bilan de masse. C'est
  exact pour les faibles différences de masse volumique de la convection
  naturelle, mais pas pour un gaz chauffé de plusieurs centaines de kelvins.
  L'écoulement par flottabilité est laminaire comme tout autre écoulement ici -
  un panache ou une cavité au-delà d'un nombre de Rayleigh d'environ `1e8` à
  `1e9` est turbulent en réalité.
- **La température n'atteint l'écoulement que par une tâche de transfert de
  chaleur.** Sans elle, les propriétés du matériau sont évaluées à `293.15 K` ;
  la condition d'environnement *Temperature* seule ne les modifie pas.
- **La dispersion de contaminants est à sens unique.** L'écoulement transporte
  le contaminant ; la concentration ne modifie jamais l'écoulement. Les
  limitations des types de problèmes associés sont listées dans leurs propres
  manuels.
- **Éléments volumiques uniquement, tétraèdres et hexaèdres linéaires
  uniquement.** Un modèle d'écoulement doit être un volume maillé d'éléments
  `TETRA1` ou `HEXA1` ; tout autre élément arrête le calcul. Les surfaces
  n'interviennent que pour porter une traction *Pressure (implicit)*, et les
  entités points et lignes n'apportent rien.
- **La paroi sans frottement est alignée sur les axes.** Elle bloque la seule
  composante globale dominante de la normale de chaque élément ; elle n'est donc
  exacte que sur une surface dont la normale est dirigée selon `x`, `y` ou `z`.
- **Les entrées sont uniformes.** *Velocity (inflow)* et *Volumetric flow rate
  (inflow)* imposent toutes deux un profil plat selon la normale moyenne de la
  surface. Un profil d'entrée établi doit être obtenu par une longueur
  d'établissement dans le maillage.

### Limitations du solveur

- **L'itération est un schéma de Newton modifié** ; la convergence près de la
  solution est donc linéaire et non quadratique. Attendez-vous à des centaines
  d'itérations pour un calcul stationnaire, et réglez le nombre d'itérations du
  groupe de tâches assez haut pour que ce soit la valeur de convergence, et non
  le nombre, qui termine le calcul. L'amortissement de 1.6 maintient un tel
  calcul en descente, mais il ne peut pas le rendre quadratique.
- **Le schéma aux différences centrées du solveur d'écoulement n'est pas un
  Crank-Nicolson.** La pondération theta est appliquée à la matrice mais pas au
  résidu ; *Central difference (accurate)* résout donc un résidu d'Euler
  implicite avec une matrice de demi-rigidité. La réponse est celle des
  différences arrière, obtenue plus lentement. Utilisez *Backward difference
  (stable)* pour l'écoulement jusqu'à ce que ce point soit corrigé. Le solveur
  de dispersion de contaminants n'est pas concerné - il est linéaire et son
  schéma est un véritable Crank-Nicolson.
- **Les paramètres de stabilisation ne sont pas dérivés.** `Tsupg`, `Tpspg`,
  `Tlsic` et la longueur d'élément dépendent tous de la vitesse, et tous entrent
  dans la matrice comme des constantes évaluées pour le champ courant. C'est la
  pratique habituelle pour une formulation stabilisée, et c'est l'une des
  raisons pour lesquelles le jacobien n'est pas exact.
- **La partie résidu du test de convergence est fixe.** Une résolution n'est
  considérée comme convergée qu'une fois que son résidu est tombé au dixième de
  ce qu'il était au début de cette résolution, et ce dixième ne peut pas être
  modifié. C'est volontairement prudent : un modèle qui converge lentement peut
  ne jamais le satisfaire et exécutera alors son nombre complet d'itérations. Le
  `Residual ratio` du journal indique à quel point un tel calcul en était proche.
- **Une référence de pression est requise.** Sans surface *Pressure (explicit)*
  ni *Pressure (implicit)*, la pression n'est déterminée qu'à une constante près
  et l'itération ne se stabilisera pas.
- **Les débits imposés doivent s'équilibrer.** Un modèle dont toutes les
  frontières sont des parois et des vitesses imposées doit avoir un débit
  d'entrée exactement égal à son débit de sortie ; un fluide incompressible ne
  peut pas absorber la différence.
- **Coût.** Quatre inconnues par nœud, une matrice non symétrique et une
  itération non linéaire autour de chaque résolution linéaire font d'un modèle
  d'écoulement, de loin, le type de problème le plus coûteux de Range FEA.
  Dimensionnez le maillage en conséquence, et faites converger un champ
  stationnaire avant de lancer tout calcul transitoire.
