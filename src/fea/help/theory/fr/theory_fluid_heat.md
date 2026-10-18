# Transfert de chaleur dans les fluides - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur de transfert de chaleur dans les fluides de
Range FEA - `RSolverFluidHeat`, qui traite le type de problème
**Transfert de chaleur dans les fluides** (*Heat transfer in fluids*) :
l'équation qu'il résout, la signification de chaque donnée d'entrée qu'il
accepte, son couplage avec le solveur thermique d'un solide et les parties de
l'interface graphique qui le pilotent.

Le transfert de chaleur dans les fluides est résolu sur le champ d'écoulement
d'une tâche **Écoulement visqueux incompressible** (*Incompressible viscous
flow*). Le solveur d'écoulement, ses conditions aux limites, son itération non
linéaire et sa marche en temps sont décrits dans le manuel
[Écoulement des fluides](theory_fluid.md), et le solveur thermique des solides
dans le manuel [Transfert de chaleur](theory_heat.md).

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Limitations](#3-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnue principale - la température

L'inconnue principale est la **température** `T` en `K`, un degré de liberté
par nœud.

| Type de problème | Solveur | Inconnue principale | Unités | Degrés de liberté par nœud |
|---|---|---|---|---|
| Heat transfer in fluids | `RSolverFluidHeat` | température | `K` | 1 |

Ce type de problème **nécessite** une tâche *Incompressible viscous flow* dans
le même problème : il lit le champ de vitesse dans son résultat et ne le calcule
jamais lui-même.

### 1.2 Équation fondamentale

**Heat transfer in fluids** résout l'advection-diffusion de la température sur
le champ d'écoulement,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

avec la même stabilisation SUPG du terme convectif que le solveur d'écoulement
et le même schéma temporel theta. La vitesse `v` est **reprise du résultat de
l'écoulement** et n'est pas résolue ici - c'est pourquoi ce type de problème
nécessite une tâche *Incompressible viscous flow*.

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `T` | température | `K` | inconnue résolue |
| `v` | vitesse | `m/s` | résultat de l'écoulement |
| `k` | conductivité thermique | `W/(m*K)` | propriété du matériau |
| `c` | capacité thermique | `J/(kg*K)` | propriété du matériau |
| `rho` | masse volumique | `kg/m^3` | propriété du matériau |
| `q` | densité de source de chaleur | `W/m^3` | condition Heat, chaleur Joule, rayonnement |

La différence avec le type de problème simple **Heat transfer** est
précisément le terme `v . grad(T)` : l'énergie est transportée par le fluide en
mouvement et pas seulement conduite. Cela signifie aussi que ce solveur n'a pas
de conditions aux limites de convection - il n'y a rien à corréler, puisque la
chaleur qui passe d'une paroi au fluide est résolue par le maillage et non
modélisée.

### 1.3 Conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Temperature | explicite (Dirichlet) | point, ligne, surface, volume | Temperature `[K]` |
| Heat | naturelle, source | point, ligne, surface, volume | Heat `[W]` |

**Temperature** (température) maintient les nœuds de son entité à la valeur
donnée. Affectez-la à la surface d'entrée pour fixer la température du fluide
qui entre dans le domaine, et à une paroi pour modéliser une frontière chauffée
ou refroidie.

**Heat** (chaleur) est une source et prend la **puissance totale en `W`**
apportée à l'entité, répartie sur la mesure de l'entité avant l'assemblage - la
même convention que pour le solveur thermique. *Heat rate (unit area)* et *Heat
rate (unit volume)* sont proposées par l'interface pour ce type de problème mais
ne sont **pas lues** par le solveur thermique des fluides ; utilisez ici *Heat*
pour une source.

Chaque composante est une **table en fonction du temps**.

Les conditions de l'écoulement - parois, entrées et sorties - appartiennent à la
tâche d'écoulement et sont décrites dans le manuel
[Écoulement des fluides](theory_fluid.md).

### 1.4 Transfert de chaleur conjugué

Une paroi entre un fluide maillé et un solide maillé, portant la condition
**Forced convection** (convection forcée) du solveur simple *Heat transfer*,
couple les deux. Une fois le solveur thermique exécuté, le solveur thermique des
fluides maintient les nœuds de la paroi à la température du solide - jusque-là
la paroi est isolée - et renvoie, pour chaque élément de paroi, le couple
`(h, Tf)` qui reproduit le flux de chaleur que sa solution fait passer à travers
la paroi. Le solveur thermique applique ce couple comme condition de convection.
Le flux est le résidu des équations du fluide aux nœuds de la paroi, cohérent
avec la discrétisation, et reste donc précis dans une couche limite thermique
mince. La température de paroi est relaxée d'un passage à l'autre avec un
facteur d'Aitken, ce qui stabilise l'échange en quelques passages. Le manuel
[Transfert de chaleur](theory_heat.md) décrit le schéma sous *Transfert de
chaleur conjugué*. La chaleur rayonnée et la chaleur Joule provenant d'autres
solveurs sont ajoutées au terme source de la même manière que dans le solveur
thermique.

### 1.5 Convergence

Sans couplage, le solveur thermique des fluides se déclare convergé sans
condition - il est linéaire une fois le champ d'écoulement fixé, une seule
résolution donne donc la réponse finale. Un groupe contenant à la fois la tâche
d'écoulement et la tâche thermique des fluides se termine donc lorsque la tâche
d'écoulement converge. Avec des parois couplées, il indique à la place la
variation relative de son champ de température, si bien que le groupe itère
jusqu'à ce que la température de paroi se soit stabilisée.

### 1.6 Couplage en retour vers l'écoulement - flottabilité

La température produite par ce solveur est renvoyée à l'écoulement. À chaque
passage, le solveur d'écoulement évalue la masse volumique et la viscosité
dynamique de chaque élément à la dernière température ; avec un matériau dont la
masse volumique dépend de la température, la force volumique `rho(T)*g` entraîne
donc une **convection naturelle** - le fluide chaud monte, le fluide froid
descend. Les matériaux fournis *Water* et *Air* ont une masse volumique
dépendant de la température.

Dans un calcul stationnaire, le premier passage de l'écoulement ne voit que la
température initiale ; la flottabilité n'apparaît donc qu'à partir de la
deuxième itération du groupe. Donnez au groupe plusieurs itérations et une
valeur de convergence. Dans un calcul transitoire avec une itération par pas de
temps, l'écoulement voit la température du pas précédent. Le manuel
[Écoulement des fluides](theory_fluid.md) décrit le schéma dans la section 1.9.

### 1.7 Conditions initiales

| Condition initiale | Effet |
|---|---|
| Temperature | champ de température initial |

Les conditions initiales ne sont lues **qu'au premier calcul** ; ensuite, le
champ avance à partir du résultat précédent. Une condition aux limites
l'emporte sur une condition initiale là où les deux se rencontrent.

### 1.8 Résultats

| Résultat | S'applique à | Signification |
|---|---|---|
| Temperature `[K]` | nœud | le champ de température résolu |
| Heat flux `[W/m^2]` | élément | flux conductif `-k*grad(T)` |

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`, `Matériau`, `Résultats`), le
menu **Problème** et le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Les combinaisons à connaître :

- *Incompressible viscous flow* et *Heat transfer in fluids* dans un même
  groupe : un écoulement chauffé ou refroidi. Donnez au groupe plusieurs
  itérations lorsque l'écoulement est entraîné ou sensiblement modifié par la
  flottabilité, afin que l'écoulement et la température se stabilisent
  ensemble (voir 1.6) ;
- *Heat transfer in fluids* et une tâche *Heat transfer* séparée sur un solide
  maillé, la tâche thermique des fluides en premier, dans un même groupe de
  plusieurs itérations : transfert de chaleur conjugué à travers les parois
  portant *Forced convection*.

Ce type de problème nécessite la présence d'une tâche d'écoulement. Les valeurs
du groupe de tâches - **Nb d'itérations** et **Convergence** - sont décrites
dans le manuel [Écoulement des fluides](theory_fluid.md).

### 2.2 Onglet Problème

**Solveur temporel** - affiché car ce type de problème dépend du temps.
Laissez-le désactivé pour un calcul stationnaire. Ses champs sont décrits dans
le manuel [Écoulement des fluides](theory_fluid.md).

Ce type de problème n'a pas de groupe de paramètres propre. Tout le reste
s'affecte par entité dans les onglets des conditions.

### 2.3 Onglet Matériau

| Propriété | Unités | Commentaire |
|---|---|---|
| Density | `kg/m^3` | `rho`, environ `1000` pour l'eau, `1.2` pour l'air |
| Dynamic viscosity | `kg/(m*s)` | `mu`, environ `1.0e-3` pour l'eau, `1.8e-5` pour l'air |
| Thermal conductivity | `W/(m*K)` | `k`, environ `0.6` pour l'eau, `0.025` pour l'air |
| Heat capacity | `J/(kg*K)` | `c`, environ `4180` pour l'eau, `1005` pour l'air |

Une entité à laquelle il manque une propriété requise n'est pas résolue, et le
contrôle de la configuration avertit avant le début du calcul. La base de
données de matériaux fournie contient **Water** (eau) et **Air**, ce qui
constitue généralement toute cette étape. Chaque propriété est une table en
fonction de la température, évaluée à la température de l'élément du calcul
précédent.

### 2.4 Onglet Conditions aux limites

**Temperature** et **Heat** apparaissent dès qu'une tâche *Heat transfer in
fluids* figure dans l'enchaînement des tâches - voir la section 1.3. Chaque
composante est une table en fonction du temps ; cliquez sur **Modifier les
valeurs dépendant du temps** pour saisir des valeurs en fonction du temps.

### 2.5 Conditions initiales

L'onglet `Conditions initiales` propose *Temperature*. Elle définit le champ de
départ du premier calcul.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur thermique des fluides utilise l'entrée
**GMRES**, car sa matrice n'est pas symétrique.

### 2.7 Points de surveillance et résultats

`Problème` -> `Définir les points de surveillance` peut enregistrer
*Temperature*. L'onglet `Résultats` affiche la température comme scalaire aux
nœuds et le flux de chaleur comme vecteur aux éléments.

---

## 3. Limitations

- **Flottabilité sans dilatation du fluide.** La température modifie
  l'écoulement par `rho(T)` et `mu(T)` (voir 1.6), mais la dilatation thermique
  du fluide n'intervient jamais dans le bilan de masse `div(v) = 0`. C'est exact
  pour les faibles différences de masse volumique de la convection naturelle,
  mais pas pour un gaz chauffé de plusieurs centaines de kelvins. L'écoulement
  par flottabilité est laminaire, comme tout écoulement dans Range FEA.
- **Le couplage est séquentiel, non monolithique.** L'écoulement et la
  température sont résolus l'un après l'autre et échangent des données entre les
  passages. Un écoulement stationnaire avec flottabilité nécessite un groupe de
  plusieurs itérations ; avec une itération par pas de temps, un écoulement
  transitoire a un pas de retard sur la température.
- **Les conditions `Heat rate` sont ignorées.** *Heat rate (unit area)* et
  *Heat rate (unit volume)* sont proposées par l'interface pour *Heat transfer in
  fluids* mais ne sont pas lues par `RSolverFluidHeat` ; seule la condition
  *Heat* lui parvient. Le type de problème simple *Heat transfer* lit les trois.
- **Pas de conditions aux limites de convection.** La chaleur qui passe d'une
  paroi à un fluide maillé est résolue et non corrélée ; les conditions
  *Simple*, *Forced* et *Natural convection* appartiennent donc au solveur
  thermique simple. Le transfert de chaleur conjugué se configure comme une
  tâche *Heat transfer* sur le solide, dont les parois *Forced convection*
  lisent l'état calculé du fluide.
- **Pas de modèle de turbulence.** L'écoulement est laminaire, voir le manuel
  [Écoulement des fluides](theory_fluid.md) ; le transfert de chaleur turbulent
  n'est donc pas représenté.
