# Écoulement en milieu poreux - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur d'écoulement en milieu poreux (`RSolverPotential`)
de Range FEA, qui traite le type de problème **Écoulement en milieu poreux**
(*Flow through porous media*) : les équations qu'il résout, la signification de
chaque donnée qu'il accepte, les parties de l'interface graphique qui le pilotent
et deux tutoriels détaillés.

**Sommaire**

1. [Bases théoriques](#1-bases-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - écoulement sous pression à travers un filtre à sable](#3-tutoriel---écoulement-sous-pression-à-travers-un-filtre-à-sable)
4. [Tutoriel - infiltration ascendante dans une colonne verticale](#4-tutoriel---infiltration-ascendante-dans-une-colonne-verticale)
5. [Vérification d'un modèle](#5-vérification-dun-modèle)
6. [Limitations](#6-limitations)

---

## 1. Bases théoriques

### 1.1 Inconnue principale - le champ de pression

L'inconnue principale est la **pression** `p` du fluide interstitiel en chaque
nœud, en pascals. Chaque nœud porte un seul degré de liberté, de sorte qu'un
maillage de `N` nœuds donne un système de `N` équations - aussi économique qu'un
modèle thermique ou électrostatique du même maillage. La vitesse du fluide est
reconstruite à partir du champ de pression après la résolution.

Un champ de pression régi par cette équation n'est défini **qu'à une constante
additive près** : ajouter la même valeur à la pression partout ne change aucune
vitesse. Tout modèle a donc besoin d'au moins une condition aux limites
*Pressure (explicit)*, voir 1.7. Que la pression soit lue comme absolue ou
relative est laissé à l'utilisateur - le solveur ne travaille qu'avec son gradient.

### 1.2 Équations fondamentales

Le solveur met en œuvre la **loi de Darcy** pour l'écoulement lent d'un fluide
unique incompressible qui remplit les pores d'un corps poreux rigide :

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

avec la conservation de la masse d'un fluide incompressible

```
div( v ) = 0
```

ce qui donne une équation de potentiel (de type Laplace) pour la pression :

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Symbole | Signification | Unités | Source |
|---|---|---|---|
| `p` | pression du fluide interstitiel | `Pa` | inconnue résolue |
| `v` | vitesse de Darcy (vitesse de filtration) | `m/s` | reconstruite, voir 1.8 |
| `k` | perméabilité intrinsèque du corps poreux | `m^2` | propriété du matériau *Permeability to fluids* |
| `mu` | viscosité dynamique du fluide interstitiel | `kg/(m*s)` | propriété du matériau *Dynamic viscosity* |
| `rho` | masse volumique du fluide interstitiel | `kg/m^3` | propriété du matériau *Density* |
| `g` | accélération de la pesanteur | `m/s^2` | condition d'environnement, voir 1.6 |

Le rapport `k/mu` joue le rôle que joue la conductivité dans un modèle thermique.
Sans pesanteur, l'équation est l'équation de Laplace pour la pression et
l'écoulement suit directement le gradient de pression. Avec la pesanteur, le terme
`rho*g` fait qu'un fluide au repos porte un gradient de pression hydrostatique :
`grad(p) = rho*g` donne `v = 0`.

**La vitesse est la vitesse de Darcy.** C'est le débit volumique par unité de
section **totale**, solide et pores ensemble - et non la vitesse du fluide dans les
pores. La vitesse moyenne dans les pores est plus grande de l'inverse de la
porosité `n` :

```
v_pore = v / n
```

La porosité n'intervient pas dans un écoulement permanent, le solveur n'en a donc
pas besoin et ne l'accepte pas. Utilisez-la à la main pour estimer des temps de
transit.

**Perméabilité et conductivité hydraulique.** La mécanique des sols et
l'hydrogéologie donnent souvent la *conductivité hydraulique* (coefficient de
perméabilité) `K` en `m/s` - le coefficient de la loi de Darcy écrite pour la
charge hydraulique - au lieu de la perméabilité intrinsèque. Les deux sont liées
par les propriétés du fluide :

```
k = K * mu / ( rho * |g| )
```

Pour l'eau à 20 °C, `K = 1e-4 m/s` correspond à `k = 1.02e-11 m^2`. L'ingénierie
pétrolière donne la perméabilité en darcy : `1 D = 9.869e-13 m^2`.

**Propriétés de matériau requises.** Un problème d'écoulement en milieu poreux en
requiert **trois** :

| Propriété | Unités | Utilisée pour |
|---|---|---|
| Permeability to fluids | `m^2` | la matrice de rigidité - perméabilité du solide |
| Dynamic viscosity | `kg/(m*s)` | la matrice de rigidité - viscosité du **fluide** |
| Density | `kg/m^3` | le terme de pesanteur - masse volumique du **fluide** |

**La masse volumique et la viscosité sont celles du fluide interstitiel, pas du
corps poreux.** Un matériau pour ce type de problème décrit donc le milieu
*saturé* : la perméabilité du squelette solide et la masse volumique et la
viscosité du fluide qui le remplit. Les matériaux fournis nommés *... (water
saturated)* sont construits ainsi (voir 2.3). Un matériau donnant la masse
volumique apparente d'un sol sec fausse la pression hydrostatique dans le rapport
des deux masses volumiques.

Une entité dont le matériau ne possède pas l'une des trois propriétés **n'est pas
résolue**. Le vérificateur de configuration le signale avant le lancement du
calcul. La seule exception est une entité portant une condition aux limites de ce
type de problème : elle est incluse dans le système quel que soit son matériau, et
sa perméabilité est alors simplement nulle. C'est ainsi qu'une face de bord sans
matériau reçoit un débit entrant.

Chaque propriété est une table en fonction de la température. Une tâche
d'écoulement en milieu poreux seule n'a pas de champ de température, donc chaque
table est évaluée à `293.15 K`. Les tables ne sont prises en compte que si une
tâche *Heat transfer* placée plus tôt dans le même enchaînement a calculé des
températures d'éléments - c'est ainsi qu'un fluide plus chaud, moins visqueux,
s'écoule plus vite.

### 1.3 Discrétisation par éléments finis

La forme faible de l'équation de pression donne, pour chaque élément,

```
K = INT( B^T * ( k / mu ) * B )                  rigidité
f = INT( B^T * ( k / mu ) * rho * g )            charge de pesanteur
  + INT_S( N^T * v_in )                          débit entrant par le bord
```

où `B` est construite à partir des dérivées des fonctions de forme, `N` sont les
fonctions de forme et `v_in` est la vitesse d'entrée imposée sur une surface de
bord. Chaque matrice élémentaire est `n x n` pour un élément à `n` nœuds. Il n'y a
pas de matrice de masse ni de stockage : l'écoulement est permanent, voir 1.10.

**Mesures des éléments.** Comme ailleurs dans Range FEA, les entités de dimension
inférieure reçoivent une mesure géométrique pour représenter des corps réels :

| Entité | Mesure | Où elle est définie |
|---|---|---|
| Volume | 1 | - |
| Surface | épaisseur de la surface | propriétés géométriques de l'entité |
| Ligne | section de la ligne | propriétés géométriques de l'entité |
| Point | - | les points ne conduisent pas d'écoulement |

Une **surface** épaisse conduit l'écoulement dans son propre plan, ce qui permet de
construire un modèle bidimensionnel (coupe d'un barrage ou d'un aquifère). Sa
charge de pesanteur utilise la composante de `g` contenue dans le plan de la
surface. Une surface d'**épaisseur nulle** - la face de bord habituelle d'un
modèle volumique - n'a pas de rigidité et ne reçoit qu'un débit entrant.

Une **ligne** avec une section conduit l'écoulement le long de sa direction, pour
un modèle unidimensionnel d'une colonne ou d'un tube garni. Un **point** n'apporte
rien. Un élément point dont le nœud n'appartient à aucun autre élément calculable
laisse une ligne vide dans la matrice - n'affectez pas de matériau poreux aux
entités point.

### 1.4 Conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Pressure (explicit) | explicite (Dirichlet) | surface | Pressure `[Pa]` |
| Velocity (inflow) | naturelle (flux imposé) | surface | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | naturelle (flux imposé) | surface | Volumetric flow rate `[m^3/s]` |

Une surface de bord sans aucune d'elles est **imperméable** : aucun fluide ne la
traverse. C'est la condition naturelle de l'équation, donc les parois, le
substratum d'un aquifère et les plans de symétrie n'ont besoin d'aucune condition.

**Pressure (explicit)** est la seule condition explicite. Chaque nœud de la surface
est retiré du système d'équations et maintenu à la pression donnée. C'est là que
le fluide quitte le modèle (une sortie, un drain, l'eau libre d'un réservoir) ou y
entre sous une pression connue. Tout modèle en a besoin d'au moins une. Une
condition nouvellement affectée démarre à `1000 Pa`.

**Velocity (inflow)** impose la vitesse de Darcy normale à travers la surface. Une
valeur **positive** est une entrée dans le corps poreux, une valeur **négative**
une sortie (prélèvement). La valeur est un flux par unité de surface, le débit
total croît donc avec l'aire de la surface.

**Volumetric flow rate (inflow)** impose le débit **total** à travers la surface en
`m^3/s`. Le solveur le divise par l'aire de toute l'entité et applique le résultat
comme vitesse d'entrée uniforme, de sorte que le débit reste inchangé lorsque la
surface est remaillée ou agrandie. Positif signifie à nouveau une entrée.

Aucune des conditions d'entrée ne fixe la pression : la pression à l'entrée fait
partie de la réponse. Là où une surface d'entrée et une surface *Pressure
(explicit)* partagent des nœuds, la pression l'emporte en ces nœuds.

Une surface d'entrée **n'a besoin ni de matériau ni d'épaisseur** : la condition
seule l'intègre au système. Si une surface porte un matériau poreux et une
épaisseur, elle conduit en plus l'écoulement dans son plan.

La condition *Pressure (implicit)* du solveur d'écoulement incompressible n'est pas
proposée pour ce type de problème.

Toutes les composantes acceptent une table en fonction du temps, qui prend effet
lorsque le solveur temporel est activé, voir 1.10.

### 1.5 Pesanteur

La pesanteur intervient par la condition d'environnement *Gravitational
acceleration*. **Chaque entité la porte par défaut avec
`g = (0, 0, -9.80665) m/s^2`**, la pesanteur est donc dirigée selon l'axe Z négatif
sauf modification. Deux conséquences sont à connaître :

- Dans un modèle dont l'axe Z est dirigé vers le haut, un fluide au repos reçoit
  automatiquement une pression hydrostatique qui croît de `rho*|g|` par mètre de
  profondeur. C'est en général le comportement voulu.
- Un modèle construit avec un autre axe vers le haut doit avoir la condition
  modifiée sur chaque entité poreuse, sinon le fluide « tombe » de côté. Mettez
  les trois composantes à zéro pour supprimer la pesanteur - par exemple pour un
  modèle horizontal où seule la différence de pression importe.

La pesanteur combinée à une surface *Pressure (explicit)* non horizontale demande
de l'attention : une pression uniforme sur une face verticale **n'est pas**
hydrostatique, le fluide circule donc près de cette face. Donnez la pression en
fonction de la hauteur (découpez la face en plusieurs entités) ou supprimez la
pesanteur lorsqu'elle ne fait pas partie de la question.

### 1.6 Conditions d'environnement, initiales et de couplage

#### Conditions d'environnement

| Condition d'environnement | Composantes | Effet |
|---|---|---|
| Gravitational acceleration | g en X, Y, Z `[m/s^2]` | terme de pesanteur de la loi de Darcy, voir 1.5 |
| Temperature | Temperature `[K]` | proposée, mais non lue - voir ci-dessous |

La condition d'environnement *Temperature* est proposée parce que les propriétés
des matériaux sont des tables en fonction de la température. Le solveur **ne la
lit pas** : il prend les températures d'éléments uniquement dans les données
partagées laissées par une tâche *Heat transfer*, et sinon évalue chaque table à
`293.15 K`.

#### Conditions initiales

| Condition initiale | Effet |
|---|---|
| Pressure | proposée, non lue |
| Velocity | proposée, non lue |

L'écoulement est permanent, la réponse ne dépend donc pas d'un champ initial et le
solveur ne lit aucune des deux conditions initiales. Les deux sont proposées parce
que le type de problème les partage avec le problème d'écoulement incompressible.

#### Couplage avec d'autres solveurs

| Sens | Variable | Effet |
|---|---|---|
| Thermique -> écoulement en milieu poreux | Temperature | sélectionne la ligne de chaque table de matériau dépendant de la température |

La pression et la vitesse ne sont utilisées par aucun autre solveur. L'écoulement
en milieu poreux se combine avec toute autre physique sauf *Incompressible viscous
flow* et *Modal analysis*.

Le solveur se déclare convergé sans condition, donc un groupe de tâches ne
contenant qu'une tâche d'écoulement en milieu poreux s'arrête toujours après une
itération.

### 1.7 Résolution du système

Le solveur assemble et résout

```
K * p = f
```

`K` est symétrique définie positive dès qu'au moins une pression est imposée ; elle
est donc résolue par la méthode du **gradient conjugué** avec préconditionnement
de Jacobi. L'assemblage est parallélisé sur les éléments avec des tampons par
thread fusionnés une fois à la fin, de sorte que le résultat ne dépend pas du
nombre de threads.

Les perméabilités des matériaux réels vont d'environ `1e-9 m^2` (gravier) jusqu'à
`1e-20 m^2` (argile intacte), ce qui placerait les termes de la matrice près des
limites d'arrondi du solveur itératif. Avant la résolution, chaque équation est
donc divisée par le plus grand `k/mu` du modèle. Diviser toutes les équations par
le même nombre ne change pas la solution ; cela garde seulement l'itération bien
conditionnée.

Un modèle **sans condition aux limites Pressure (explicit)** n'a pas de solution
unique. Le solveur le détecte et s'arrête avec l'erreur *Flow through porous media
requires at least one 'Pressure (explicit)' boundary condition*.

Autres façons d'aboutir à un système singulier ou vide, toutes silencieuses :

- une entité dont le matériau ne possède pas l'une des trois propriétés requises
  n'est pas résolue, et ses nœuds sortent du système ;
- une entité de perméabilité ou de viscosité nulle n'a pas de rigidité, ses nœuds
  restent donc dans le système mais ne sont reliés à rien ;
- une surface d'épaisseur nulle ou une ligne de section nulle ne conduit pas
  d'écoulement.

### 1.8 Résultats dérivés

Après la résolution, la vitesse de chaque élément est reconstruite à partir du
champ de pression nodal. Le gradient de pression est moyenné sur les points
d'intégration de l'élément et combiné au terme de pesanteur ; pour les éléments
lignes et surfaces, il est tourné dans le repère global :

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Résultat | Appliqué à | Signification |
|---|---|---|
| Pressure `[Pa]` | nœud | le champ de pression interstitielle résolu |
| Velocity `[m/s]` | élément | vecteur vitesse de Darcy |

La vitesse est une **densité** au même sens que les résultats de champ des autres
solveurs : elle ne dépend pas de la taille de l'élément, est constante dans un
élément linéaire et converge lorsque le maillage est raffiné. Les faces de bord
d'épaisseur nulle affichent une vitesse nulle - affichez le résultat sur le volume.

Le débit total n'est pas stocké comme résultat. Calculez-le à la main à partir de
la vitesse et de l'aire d'une section, ou à partir des conditions d'entrée, voir 5.

Après l'écriture de l'enregistrement, le journal du solveur affiche les
statistiques des deux variables et les valeurs aux points de surveillance.

### 1.9 Validité de la loi de Darcy

La loi de Darcy vaut pour un écoulement lent et laminaire, dans lequel les forces
visqueuses dominent l'inertie du fluide. Un critère courant utilise le nombre de
Reynolds construit avec le diamètre moyen des grains `d` :

```
Re = rho * |v| * d / mu  <  1 ... 10
```

L'écoulement des eaux souterraines dans les sols, l'infiltration à travers les
barrages et les digues, l'écoulement à travers les filtres, les lits garnis et la
plupart des matériaux poreux techniques le satisfont largement. L'écoulement à
travers du gravier grossier, des enrochements ou des mousses très ouvertes à grande
vitesse non ; la loi de Darcy sous-estime alors la perte de charge (régime de
Forchheimer), et ce solveur donnera une vitesse trop grande.

### 1.10 Écoulement permanent et solveur temporel

Le type de problème autorise le solveur temporel, mais l'écoulement lui-même est
**permanent** : le fluide et le corps poreux sont incompressibles et il n'y a pas
de terme de stockage. Avec le solveur temporel activé, chaque pas de temps résout un
état permanent distinct avec les valeurs des conditions à cet instant. C'est le bon
modèle lorsque les conditions aux limites varient lentement par rapport au temps
nécessaire à la pression pour s'établir - un niveau de réservoir qui change sur
plusieurs heures au-dessus d'un filtre à sable - et cela permet à une tâche
d'écoulement en milieu poreux de partager un enchaînement avec une tâche thermique
transitoire.

La consolidation, le drainage d'un sol non saturé, l'écoulement de gaz
compressible et tout autre phénomène dans lequel la quantité de fluide stockée dans
les pores varie dans le temps sont hors du modèle, voir la section 6.

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`, `Conditions d'environnement`,
`Matériau`, `Résultats`), dans le menu **Problème** et dans le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la boîte de
dialogue de l'enchaînement des tâches. Ajoutez une tâche et choisissez **Flow
through porous media**.

Le type de problème ne peut pas être combiné avec *Incompressible viscous flow* ni
avec *Modal analysis*. Une tâche *Heat transfer* placée avant lui fait suivre aux
tables de matériau la température calculée.

### 2.2 Onglet Problème

L'onglet affiche le groupe *Solveur temporel*, car le type de problème prend en
charge le solveur temporel - ce que signifie ici un calcul transitoire est
expliqué en 1.10. L'écoulement en milieu poreux n'a pas de groupe de réglages
propre. Tout est affecté par entité dans les onglets des conditions.

### 2.3 Onglet Matériau

L'onglet `Matériau` affiche le matériau affecté à l'entité sélectionnée.
L'écoulement en milieu poreux requiert **les trois** propriétés suivantes :

| Propriété | Unités | Commentaire |
|---|---|---|
| Permeability to fluids | `m^2` | perméabilité intrinsèque du solide, voir le tableau ci-dessous |
| Dynamic viscosity | `kg/(m*s)` | du fluide interstitiel, `1.003e-3` pour l'eau à 20 °C |
| Density | `kg/m^3` | du fluide interstitiel, `998.2` pour l'eau à 20 °C |

La bibliothèque fournie contient des matériaux poreux saturés d'eau prêts à
l'emploi. Chacun porte la masse volumique et la viscosité de l'eau dépendant de la
température et une perméabilité typique :

| Matériau | Perméabilité `[m^2]` | Conductivité hydraulique à 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

La perméabilité d'un sol réel peut s'écarter de ces valeurs d'un ordre de grandeur
ou plus - ce sont des points de départ, pas des données d'un site particulier. Pour
modéliser un autre fluide, copiez l'un d'eux et remplacez la masse volumique et la
viscosité. Le matériau simple *Sand* est un matériau granulaire sec sans viscosité,
il est donc marqué non valide pour ce type de problème.

### 2.4 Onglet Conditions aux limites

L'onglet est partagé entre une liste des conditions aux limites disponibles en haut
et un éditeur de la condition sélectionnée en bas. Sélectionnez d'abord une entité
surface dans l'arbre `Modèle` ; la liste propose alors les conditions de la
section 1.4. Cochez-en une pour l'affecter, puis modifiez ses composantes dans
l'arbre inférieur.

- **Pressure (explicit)** maintient la pression à la sortie. Un modèle sans aucune
  ne se résout pas.
- **Velocity (inflow)** et **Volumetric flow rate (inflow)** injectent un débit
  connu dans le modèle. Les valeurs positives entrent, les négatives sortent.
- Les faces sans condition sont imperméables.

### 2.5 Conditions initiales et d'environnement

L'onglet `Conditions d'environnement` propose *Gravitational acceleration*, que
chaque entité porte déjà avec `(0, 0, -9.80665) m/s^2` (voir 1.5), et
*Temperature*, que ce solveur ignore.

L'onglet `Conditions initiales` propose *Pressure* et *Velocity*, que ce solveur ne
lit pas.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur d'écoulement en milieu poreux utilise l'entrée
**CG**. Si la résolution ne converge pas, cherchez d'abord une entité de
perméabilité nulle ou un élément point isolé plutôt que dans les réglages du
solveur.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes à des
coordonnées données et choisit la variable enregistrée - *Pressure* ou *Velocity*.
`Rapport` -> `Points de surveillance` affiche les valeurs.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste les variables calculées et pilote la vue 3D. La pression
est un scalaire nodal, la vitesse un vecteur d'élément. Affichez la vitesse comme
champ de vecteurs sur le volume pour voir les trajectoires de l'écoulement.

L'onglet `Enregistrements` du dock `Modèle` liste les enregistrements de résultats :
un pour un calcul permanent, un par pas de temps de sortie avec le solveur temporel
activé.

`Rapport` -> `Fichier journal du solveur` affiche la sortie complète du solveur, y
compris les statistiques des deux variables.

---

## 3. Tutoriel - écoulement sous pression à travers un filtre à sable

**Objectif.** Calculer l'écoulement de l'eau à travers un filtre à sable horizontal
sous l'effet d'une différence de pression connue, et vérifier le débit avec la loi
de Darcy.

Ce tutoriel suppose un modèle volumique maillé d'un parallélépipède - utilisez
`Fichier` -> `Nouveau modèle`, dessinez un parallélépipède avec `Géométrie` ->
`Créer un élément` ou importez une géométrie, et maillez-la en tétraèdres ; le
tutoriel *Draw cube* couvre ce point. Marquez les deux faces d'extrémité comme
entités surface distinctes pour pouvoir leur affecter des conditions.

Prenez un filtre de `1 m` de long selon l'axe X avec une section de `0.1 m` sur
`0.1 m`.

### Étape 1 - choisir le problème

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez une tâche et choisissez **Flow through porous media**.
3. Validez avec `OK`.

Laissez le *Solveur temporel* désactivé - la réponse est un unique état permanent.

### Étape 2 - affecter le matériau

1. Sélectionnez l'entité volume dans l'arbre `Modèle`.
2. Ouvrez l'onglet `Matériau` et affectez **Coarse Sand (water saturated)**.

Le matériau porte la perméabilité `1e-10 m^2` du sable ainsi que la masse
volumique et la viscosité de l'eau.

### Étape 3 - supprimer la pesanteur

Le filtre est horizontal et seule la différence de pression importe, supprimez donc
la pesanteur par défaut :

1. L'entité volume étant sélectionnée, ouvrez l'onglet
   `Conditions d'environnement`.
2. Sélectionnez **Gravitational acceleration** et mettez les trois composantes à
   `0`.

Avec la pesanteur conservée, l'eau porterait en plus une pression hydrostatique sur
les `0.1 m` de hauteur du filtre, et les pressions uniformes sur les faces
d'extrémité verticales la feraient circuler près d'elles - voir 1.5.

### Étape 4 - imposer les deux extrémités

1. Sélectionnez l'entité surface en `x = 0`.
2. Ouvrez l'onglet `Conditions aux limites`, cochez **Pressure (explicit)** et
   saisissez `10000` Pa - environ un mètre de colonne d'eau.
3. Sélectionnez l'entité surface en `x = 1 m`, cochez **Pressure (explicit)** et
   saisissez `0` Pa.

Les quatre faces latérales ne reçoivent aucune condition : ce sont des parois
imperméables.

### Étape 5 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Suivez l'avancement dans `Rapport` -> `Fichier journal du solveur`.

Le calcul est une unique résolution par gradient conjugué.

### Étape 6 - examiner les résultats

- Onglet `Résultats` : affichez **Pressure**. Elle décroît linéairement de
  `10000` Pa à `0` Pa le long du filtre, avec des isobares planes perpendiculaires
  à son axe.
- Affichez **Velocity**. Elle est dirigée selon +X et identique dans chaque
  élément.
- Un point de surveillance sur *Pressure* à mi-longueur du filtre indique
  `5000` Pa.

### Étape 7 - refaire le calcul à la main

Avec `k = 1e-10 m^2` et `mu = 1.003e-3 kg/(m*s)` pour l'eau à 20 °C :

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

Le résultat **Velocity** doit indiquer `9.97e-4 m/s`. Le nombre de Reynolds pour
des grains de `1 mm` vaut `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, à la limite du
régime de Darcy (1.9). Avec une porosité de `0.35`, l'eau elle-même se déplace à
`9.97e-4 / 0.35 = 2.8e-3 m/s` et met environ six minutes à traverser le filtre.

### Dépannage

| Symptôme | Cause |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | aucune pression de sortie n'est imposée |
| *Entity has material assigned which is missing required properties* | le matériau n'a pas de viscosité - souvent un matériau de sol sec comme *Sand* |
| La pression varie sur la hauteur et la vitesse a une composante Z | la pesanteur par défaut est toujours active, voir l'étape 3 |
| La vitesse est nulle partout | les deux extrémités ont la même pression ou la perméabilité est nulle |

---

## 4. Tutoriel - infiltration ascendante dans une colonne verticale

**Objectif.** Pousser de l'eau vers le haut à travers une colonne de sable
verticale avec un débit connu, et voir comment la pression d'entrée se répartit
entre une part hydrostatique et une part de frottement.

Utilisez un parallélépipède de `0.1 m` sur `0.1 m` de base et `1 m` de haut selon
l'axe **Z**, avec la face inférieure (`z = 0`) et la face supérieure (`z = 1 m`)
comme entités surface distinctes.

### Étape 1 - problème et matériau

1. Choisissez **Flow through porous media** comme dans le tutoriel 3.
2. Affectez **Coarse Sand (water saturated)** au volume.

### Étape 2 - conserver la pesanteur

La colonne est dressée selon Z, la pesanteur par défaut `(0, 0, -9.80665) m/s^2`
est donc dans le bon sens. Laissez la condition *Gravitational acceleration* telle
quelle.

### Étape 3 - conditions aux limites

1. Sélectionnez la face supérieure, cochez **Pressure (explicit)** et saisissez
   `0` Pa : l'eau libre au-dessus de la colonne.
2. Sélectionnez la face inférieure, cochez **Volumetric flow rate (inflow)** et
   saisissez `1e-6` m^3/s. Sur les `0.01 m^2` de la face, cela correspond à une
   vitesse d'entrée de `1e-4 m/s`.

La face supérieure est horizontale, une pression uniforme y est donc compatible
avec la pesanteur.

### Étape 4 - résoudre et examiner

`Solution` -> `Démarrer le solveur` (`Ctrl+R`), puis affichez **Pressure** et
**Velocity**.

- La vitesse vaut `1e-4 m/s` vers le haut (+Z) dans chaque élément.
- La pression croît linéairement de `0` Pa en haut jusqu'à son maximum en bas.

### Étape 5 - décomposer la pression d'entrée

La pression au fond doit soulever la colonne d'eau et la pousser à travers le
sable :

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Un point de surveillance sur la face inférieure confirme la somme. Mettez le débit
à zéro et résolvez à nouveau : la vitesse disparaît et la pression au fond retombe
aux `9789 Pa` purement hydrostatiques - le solveur reproduit exactement un fluide
au repos.

Augmentez le débit jusqu'à ce que la part de frottement atteigne le poids déjaugé
du sable, environ `(rho_s - rho) * g * (1 - n) * L` : c'est le début de la
fluidisation (sables mouvants), que le squelette rigide de ce modèle ne peut pas
représenter - le solveur continue simplement à calculer une pression plus grande.

### Dépannage

| Symptôme | Cause |
|---|---|
| La pression au fond n'est que la part de frottement | la pesanteur est supprimée sur le volume |
| La pression au fond n'est que la part hydrostatique | l'entrée est sur la mauvaise face ou sa valeur est nulle |
| La vitesse est dirigée de côté | la colonne a été construite selon un autre axe que Z ; modifiez la condition de pesanteur |

---

## 5. Vérification d'un modèle

**Colonne uniforme.** Pour un corps de longueur `L` maintenu à `p1` et `p2` sur
deux faces opposées, d'un seul matériau et sans pesanteur, la pression est linéaire
le long du corps et

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

Le résultat de vitesse doit concorder à la tolérance du solveur près, et la
pression dans le plan médian doit être la moyenne des deux valeurs imposées.

**Hydrostatique.** Un corps soumis à la pesanteur, avec une condition *Pressure
(explicit)* seulement sur sa face supérieure horizontale et des parois imperméables
ailleurs, doit montrer une vitesse nulle et une pression croissant de `rho*|g|` par
mètre de profondeur. Cela vérifie à la fois la masse volumique et la condition de
pesanteur.

**Bilan de débit.** Dans un modèle permanent, tout le fluide qui entre doit
ressortir. Pour une entrée imposée par *Volumetric flow rate (inflow)*, la vitesse
dans une section multipliée par son aire doit redonner le débit imposé. Lisez la
vitesse dans la colonne avec un point de surveillance.

**Couches en série.** Pour deux couches de longueur `L1`, `L2` et de perméabilité
`k1`, `k2` en série entre deux pressions imposées, l'écoulement résiste comme deux
résistances en série :

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          pression à l'interface
```

Contrairement au solveur électrostatique, celui-ci répartit la pression selon les
**perméabilités**, comme le ferait un réseau de résistances.

**Écoulement radial vers un puits.** Pour un aquifère captif d'épaisseur `H` entre
un puits de rayon `rw` maintenu à `pw` et un rayon extérieur `R` maintenu à `pR`,
la pression est logarithmique et le débit du puits vaut

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Cela vérifie le solveur sur un champ non uniforme et montre si le maillage est
assez fin près du puits.

Raffinez le maillage au moins une fois et vérifiez que la pression converge au lieu
de dériver.

---

## 6. Limitations

- **Écoulement permanent, incompressible et monophasique.** Il n'y a pas de terme
  de stockage, donc la consolidation, l'écoulement de gaz compressible et la
  réponse transitoire d'un aquifère captif ne sont pas modélisés. Un calcul
  transitoire est une suite d'états permanents, voir 1.10.
- **Milieux entièrement saturés uniquement.** Il n'y a pas de surface libre (ligne
  phréatique) ni de zone non saturée. L'infiltration à travers une digue dont la
  nappe doit être déterminée est hors du modèle, sauf si la zone saturée est
  maillée explicitement.
- **Corps poreux rigide.** Le squelette ne se déforme pas, il n'y a donc pas de
  couplage avec l'analyse des contraintes, ni contrainte effective, ni
  consolidation, ni fluidisation.
- **Perméabilité isotrope.** La perméabilité est un scalaire. Les sols stratifiés
  dont la perméabilité horizontale diffère de la verticale doivent être modélisés
  en couches distinctes.
- **Régime de Darcy uniquement.** L'écoulement inertiel (Forchheimer) et turbulent
  dans les milieux très grossiers n'est pas modélisé, voir 1.9.
- **Conditions aux limites sur les surfaces uniquement.** Pressions et débits
  entrants sont imposés sur des entités surface ; il n'y a pas de surface de
  suintement, ni de puits ou de source linéique, ni de pression imposée sur une
  ligne ou un point.
- **Pas de résultat de débit total.** Les débits à travers une section doivent être
  calculés à la main à partir de la vitesse.
- **Les conditions initiales ne sont pas lues** et la condition d'environnement
  *Temperature* est sans effet ; les tables de matériau ne suivent la température
  qu'à travers une tâche thermique placée plus tôt dans l'enchaînement.
- **Un modèle a besoin d'une pression imposée.** Sans elle, le solveur s'arrête sur
  une erreur.
