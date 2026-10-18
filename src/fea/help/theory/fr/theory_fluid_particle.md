# Dispersion de contaminants - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur de dispersion de contaminants de Range FEA -
`RSolverFluidParticle`, qui traite le type de problème **Dispersion de
contaminants** (*Contaminant dispersion*) : l'équation qu'il résout, la
signification de chaque donnée d'entrée qu'il accepte, les parties de
l'interface graphique qui le pilotent, et un tutoriel complet.

La dispersion de contaminants est résolue sur le champ d'écoulement d'une tâche
**Écoulement visqueux incompressible** (*Incompressible viscous flow*). Le
solveur d'écoulement, ses conditions aux limites, son itération non linéaire et
sa marche en temps sont décrits dans le manuel
[Écoulement des fluides](theory_fluid.md), sur lequel celui-ci s'appuie.

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - écoulement transitoire avec dispersion de contaminants](#3-tutoriel---écoulement-transitoire-avec-dispersion-de-contaminants)
4. [Vérifier un modèle](#4-vérifier-un-modèle)
5. [Limitations](#5-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnue principale - la concentration de particules

L'inconnue principale est la **concentration de particules** `C` en `kg/m^3`,
un degré de liberté par nœud.

| Type de problème | Solveur | Inconnue principale | Unités | Degrés de liberté par nœud |
|---|---|---|---|---|
| Contaminant dispersion | `RSolverFluidParticle` | concentration de particules | `kg/m^3` | 1 |

Ce type de problème **nécessite** une tâche *Incompressible viscous flow* dans
le même problème : il lit le champ de vitesse dans son résultat et ne le calcule
jamais lui-même.

### 1.2 Équation fondamentale

**Contaminant dispersion** transporte une concentration scalaire sur le champ
d'écoulement,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

(`for s > 0 and a maximum saturation set` = pour `s > 0` et une saturation
maximale définie ; `otherwise` = sinon), avec la stabilisation SUPG et le
schéma temporel theta du solveur d'écoulement, la vitesse étant lue dans le
résultat de l'écoulement.

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `C` | concentration de particules | `kg/m^3` | inconnue résolue |
| `v` | vitesse | `m/s` | résultat de l'écoulement |
| `D` | coefficient de diffusion | `m^2/s` | *Paramètres de la dispersion de contaminants*, `0` par défaut |
| `C_sat` | saturation maximale | `kg/m^3` | *Paramètres de la dispersion de contaminants*, `0` (illimitée) par défaut |
| `s` | taux de source de particules | `kg/(m^3*s)` | condition Particle rate |

### 1.3 Paramètres de la dispersion de contaminants

Les deux paramètres de ce type de problème se définissent une fois par modèle
dans le groupe *Paramètres de la dispersion de contaminants* de l'onglet
`Problème` (section 2.2). Tous deux valent zéro par défaut, ce qui reproduit une
advection pure avec une source illimitée.

#### Coefficient de diffusion

Le **Coefficient de diffusion** `D` répand le contaminant dans le sens
décroissant de son gradient de concentration. Avec `D = 0`, le transport est
purement advectif : le contaminant est entraîné par l'écoulement et ne se répand
que par la diffusion numérique de la stabilisation ; le solveur répond donc à la
question « où l'écoulement l'emmène-t-il, et en combien de temps », et non
« quelle est la largeur du panache ». Un `D` positif fait de la largeur du
panache un résultat physique.

Il n'y a pas de modèle de turbulence (section 5) ; `D` est donc une diffusivité
**effective** : le mélange d'un écoulement réel est dominé par la turbulence,
que l'on peut estimer par `D ~ nu_t / Sc_t` avec le nombre de Schmidt turbulent
`Sc_t ~ 0.7`.

| Diffusion moléculaire (la propriété physique) | `D [m^2/s]` |
|---|---|
| gaz et vapeurs dans l'air (vapeur d'eau `2.5e-5`, CO2 `1.6e-5`, vapeurs de solvants environ `1e-5`) | `1e-6` - `1e-4` |
| espèces dissoutes dans l'eau (O2 `2e-9`, sels environ `1.5e-9`) | `1e-10` - `1e-8` |
| particules d'aérosol dans l'air, mouvement brownien (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

Aux échelles de l'ingénierie, la diffusion moléculaire est presque toujours
négligeable devant le transport par l'écoulement. La valeur saisie normalement
est une valeur effective :

| Diffusion effective (turbulente) | `D_eff [m^2/s]` |
|---|---|
| pièce ventilée, mélange de l'air intérieur | `1e-3` - `1e-2` |
| écoulement en conduite ou en canal, environ `D ~ 0.01 - 0.05 * U * L` (`U` vitesse moyenne, `L` dimension de la conduite) | `1e-3` - `1e-1` |
| rivières, mélange transversal | `1e-2` - `1` |
| atmosphère, mélange horizontal | `1` - `100` |

L'importance d'une valeur dépend du maillage. Avec la vitesse moyenne `U`, la
longueur d'écoulement `L`, la taille d'élément `h` et le pas de temps `dt` :

- **quand `D` a un effet visible** - le contaminant s'étale d'environ
  `sqrt( 2 * D * L / U )` pendant qu'il traverse le domaine. Si c'est inférieur
  à `h`, `D` n'a aucun effet ; `D` ne compte donc qu'au-dessus d'environ
  `U * h^2 / ( 2 * L )`. Pour un canal avec `U ~ 1 m/s`, `L ~ 15 m` et
  `h ~ 0.1 m`, cela fait environ `3e-4 m^2/s` ;
- **quand `D` domine** - au-dessus d'environ `U * L` (environ `15 m^2/s` pour le
  même canal), la diffusion étale tout et l'écoulement ne joue presque plus
  aucun rôle ;
- **pas de temps** - si `D * dt / h^2` dépasse environ `1`, passez à *Backward
  difference (stable)* ; le schéma aux différences centrées n'amortit pas les
  modes diffusifs les plus rapides et oscille.

#### Saturation maximale

La **Saturation maximale** `C_sat` est la plus forte concentration que le
contaminant peut atteindre dans le fluide - une concentration de vapeur
saturante ou une solubilité. Lorsqu'elle est définie, un *Particle rate*
positif s'affaiblit à mesure que la concentration s'en approche et s'annule à
saturation, `s * ( 1 - C / C_sat )` ; cela modélise l'évaporation ou la
dissolution, et `s` est alors le taux dans un fluide propre, environ
`k_m * C_sat * A / V`. Les puits (`s < 0`) ne sont pas limités. La limite
n'agit que sur la source : une concentration imposée par une condition aux
limites ou initiale n'est pas limitée, et le journal du solveur avertit
lorsqu'elle dépasse `C_sat`. La concentration n'est pas non plus écrêtée ; un
léger dépassement près d'un front raide reste donc possible.

Une limite de saturation n'existe que là où il y a une limite thermodynamique :
une pression de vapeur ou une solubilité. Pour une vapeur dans l'air, la
concentration de saturation découle de la pression de vapeur saturante,
`C_sat = p_sat * M / ( R * T )` :

| Vapeur dans l'air | Température | `C_sat [kg/m^3]` |
|---|---|---|
| vapeur d'eau | 0 °C | `0.0048` |
| vapeur d'eau | 20 °C | `0.017` |
| vapeur d'eau | 40 °C | `0.051` |
| vapeur d'eau | 100 °C | `0.60` |
| éthanol ou toluène | 20 °C | environ `0.11` |
| méthanol | 20 °C | environ `0.17` |
| benzène | 20 °C | environ `0.32` |
| acétone | 20 °C | environ `0.59` |

Pour une substance dissoute dans l'eau, la limite est sa solubilité :

| Dissous dans l'eau | `C_sat [kg/m^3]` |
|---|---|
| oxygène en équilibre avec l'air, 20 °C | `0.009` |
| dioxyde de carbone à 1 atm | `1.7` |
| gypse | `2.4` |
| chlorure de sodium | environ `360` |

Les particules, les poussières et la fumée n'ont pas de saturation ; laissez la
valeur à `0` (illimitée). À titre d'ordre de grandeur seulement, les valeurs
limites d'exposition professionnelle sont d'environ
`1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) et les limites inférieures d'explosivité
des poussières d'environ `0.02` - `0.06 kg/m^3`.
La saturation dépend fortement de la température, alors que le solveur utilise
une seule valeur constante : pour un cas non isotherme, prenez la valeur à la
température pertinente la plus froide, là où la condensation commencerait.

### 1.4 Stabilisation

Le paramètre SUPG du solveur de dispersion est limité par le pas de temps et
inclut la diffusion,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

le terme en `dt` étant supprimé dans une analyse stationnaire. Il se réduit à
`h / ( 2 * |v| )` là où l'écoulement est rapide, ne dépasse jamais `dt / 2`, et
tend vers `h^2 / ( 12 * D )` - la limite du paramètre unidimensionnel exact aux
nœuds - là où la diffusion domine. La limite due au pas de temps compte dans les
zones lentes et de recirculation : le nombre de Courant de l'élément
`|v| * dt / h` y est minuscule, et un `tau` non limité transformerait la
stabilisation en un bruit qui crée de la concentration en avant du front. Ce
paramètre est propre au solveur de dispersion ; le solveur d'écoulement utilise
celui décrit dans le manuel [Écoulement des fluides](theory_fluid.md).

Le terme d'advection de Galerkin est assemblé sous **forme advective avec un
puits de divergence**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, avec la vitesse
nodale continue. Une vitesse issue du solveur d'écoulement n'est jamais
exactement à divergence nulle - sur un maillage grossier, un `div(v)` de
plusieurs dizaines de `1/s` est courant près des parois et des coins. Là où
l'écoulement se dilate (`div(v) > 0`), l'advection simple créerait de la
concentration d'elle-même, un motif en damier qui croît d'un pas à l'autre dès
que le pas de temps est petit ; le puits l'annule. Là où l'écoulement converge,
aucun terme n'est ajouté : une correction symétrique `0.5 * div(v) * C` y
agirait comme une source et accumulerait le contaminant près des parois. Pour
une vitesse exactement à divergence nulle, le puits est nul.

Avec un coefficient de diffusion négligeable, le transport est une advection
presque pure, que Galerkin avec SUPG ne maintient pas monotone : attendez-vous à
des dépassements par excès et par défaut, les plus forts aux parois adhérentes
où la vitesse est nulle. Un coefficient de diffusion effectif (section 1.3) en
supprime la plupart.

Contrairement au solveur d'écoulement, le solveur de dispersion applique la
pondération theta de manière cohérente ; *Central difference (accurate)* est
donc ici un véritable schéma de Crank-Nicolson.

### 1.5 Conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Particle concentration | explicite (Dirichlet) | point, ligne, surface | Particle concentration `[kg/m^3]` |
| Particle rate | naturelle, source | point, ligne, surface, volume | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | naturelle (Neumann) | surface | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | naturelle (Robin) | surface | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** (concentration de particules) maintient ses nœuds à
la concentration donnée. Affectée à une surface d'entrée avec une table en
fonction du temps, c'est ainsi que l'on modélise un rejet : zéro avant
l'événement, la concentration de rejet pendant celui-ci, zéro ensuite. C'est le
procédé le plus utile de ce type de problème, et celui sur lequel s'appuie le
tutoriel de la section 3.

**Particle rate** (taux de particules) est une source, exprimée en **densité**
en `kg/(m^3*s)` et non en total - contrairement à la condition *Heat*, elle
n'est pas répartie sur la mesure de l'entité. Elle modélise une source qui émet
en continu à l'intérieur du domaine.

**Particle flux (unit area)** (flux de particules par unité de surface) impose le
flux massique `q` qui entre dans le domaine à travers une surface,
`D * grad(C) . n = q`, `n` étant la normale sortante. Contrairement à *Particle
rate*, c'est un flux par unité de surface ; le rejet total vaut donc `q * A`.
Un `q` positif est une émission - un film qui s'évapore, une paroi qui dégaze -
et il est limité par la saturation maximale de la même manière qu'un taux de
particules positif, `q * ( 1 - C / C_sat )`. Un `q` négatif est un taux
d'extraction fixe, indépendant de la concentration locale, qui peut la faire
passer sous zéro ; un dépôt proportionnel à la concentration correspond à
*Particle mass transfer* avec `C_inf = 0`. Sur une paroi, le flux entre par
diffusion seule ; sur une partie de la frontière où l'écoulement entre, le
fluide entrant est traité comme un fluide propre qui transporte le flux ; `q`
devient alors le flux massique entrant total et `C ~ q / |v . n|`. Affectée à
une surface interne entre deux volumes, elle émet `q` par unité de surface comme
une source en nappe.

**Particle mass transfer** (transfert de masse de particules) échange du
contaminant avec un réservoir ambiant, `q = k_m * ( C_inf - C )`, où `k_m` est
le *Mass transfer coefficient* (coefficient de transfert de masse) et `C_inf` la
*Ambient particle concentration* (concentration ambiante de particules). C'est
l'équivalent, pour le contaminant, de la convection simple en transfert de
chaleur. Avec `C_inf = 0`, elle modélise un dépôt sur une surface avec une
vitesse de dépôt `k_m` ; avec `C_inf > 0`, un échange avec l'air ambiant à
travers une ouverture, ou une surface en dissolution maintenue à `C_inf`. Les
valeurs typiques de `k_m` sont `1e-4` - `1e-2 m/s` pour le dépôt de particules
dans les pièces et `1e-3` - `1e-1 m/s` pour l'évaporation dans un gaz en
mouvement. Un `k_m` élevé amène la surface vers `C_inf` et se rapproche d'une
condition *Particle concentration*.

Chaque composante est une **table en fonction du temps** ; un rejet peut donc
commencer et s'arrêter au cours d'un calcul transitoire.

**Frontières sans condition.** Les parois et les sorties sans condition
*Particle concentration*, *Particle flux* ou *Particle mass transfer* gardent la
condition naturelle de **flux diffusif nul** : le contaminant ne diffuse pas à
travers une paroi et ne quitte une sortie qu'avec l'écoulement.

Là où l'écoulement **entre** par une telle frontière - typiquement un
refoulement sur une partie d'une sortie *Pressure (implicit)* - il est traité
comme un fluide propre : le solveur ajoute le terme d'entrée faible
`max(-v . n, 0) * C` sur cette partie de la surface, réparti sur la face de
manière à amortir aussi les nœuds de bord où la sortie rejoint une paroi
adhérente.
Sans cela, la frontière alimenterait la solution et la concentration à la
sortie croîtrait sans limite. Si le fluide qui rentre n'est pas propre,
prolongez le domaine pour que la sortie ne voie que de l'écoulement sortant.

Les conditions de l'écoulement - parois, entrées et sorties - appartiennent à la
tâche d'écoulement et sont décrites dans le manuel
[Écoulement des fluides](theory_fluid.md).

### 1.6 Conditions initiales

| Condition initiale | Effet |
|---|---|
| Particle concentration | champ de concentration initial |

Les conditions initiales ne sont lues **qu'au premier calcul** ; ensuite, le
champ avance à partir du résultat précédent. Une condition aux limites
l'emporte sur une condition initiale là où les deux se rencontrent.

### 1.7 Stationnaire ou transitoire ?

La dispersion de contaminants est avant tout un problème **transitoire** : un
problème de transport dans le temps, sans réponse stationnaire pertinente pour
un événement de rejet. Une dispersion stationnaire répond à une autre question -
le champ de concentration vers lequel tend une source continue - et consiste en
une seule résolution linéaire sur le champ d'écoulement (voir la section 2.6 sur
son solveur matriciel).

Le déroulement en deux étapes du tutoriel de la section 3 est la méthode de
référence : faire converger l'écoulement en stationnaire, puis redémarrer avec
le solveur temporel et la tâche de dispersion activés.

### 1.8 Résultats

| Résultat | S'applique à | Signification |
|---|---|---|
| Particle concentration `[kg/m^3]` | nœud | le champ de concentration résolu |
| Relative saturation `[-]` | nœud | `C / C_sat`, uniquement si une saturation maximale est définie |

La saturation relative est affichée de `0` à `1` par défaut et est supprimée
lorsque la saturation maximale est remise à zéro.

Seule la **masse volumique** du matériau est requise pour ce type de problème.

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`, `Matériau`, `Résultats`), le
menu **Problème** et le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Placez une tâche **Contaminant
dispersion** dans le même groupe de tâches qu'une tâche **Incompressible viscous
flow** - la dispersion sur l'écoulement, l'association transitoire classique. Ce
type de problème nécessite la présence de la tâche d'écoulement.

Les valeurs du groupe de tâches - **Nb d'itérations** et **Convergence** -
pilotent l'itération non linéaire de l'écoulement et sont décrites dans le
manuel [Écoulement des fluides](theory_fluid.md). Dans un calcul transitoire, ce
sont les itérations *par pas de temps*.

### 2.2 Onglet Problème

**Solveur temporel** - laissez-le désactivé pour une dispersion stationnaire,
activez-le pour un événement de rejet.

| Champ | Signification |
|---|---|
| Activer | active ou désactive la marche en temps transitoire |
| Approximation | theta du schéma temporel : arrière, centré ou avant |
| Temps de début | première valeur de temps en secondes |
| Temps de fin | `start time + time-step size * number of time-steps` (temps de début + taille du pas de temps × nombre de pas de temps), lecture seule |
| Taille du pas de temps | `dt` en secondes |
| Nombre de pas de temps | nombre de pas à calculer |
| Fréquence de sortie | écrit un fichier de résultats tous les N pas ; `0` n'écrit que le dernier pas |

**Paramètres de la dispersion de contaminants** - affiché dès qu'une tâche
*Contaminant dispersion* figure dans l'enchaînement des tâches. Il contient les
deux paramètres de la section 1.3 :

| Champ | Signification |
|---|---|
| Saturation maximale `[kg/m^3]` | plus forte concentration possible ; un taux de particules positif s'y arrête et *Relative saturation* est calculée. `0` signifie illimitée |
| Coefficient de diffusion `[m^2/s]` | diffusivité effective du contaminant dans le fluide. `0` signifie advection pure |

Les valeurs typiques des deux sont indiquées dans la section 1.3.

### 2.3 Onglet Matériau

| Propriété | Unités | Commentaire |
|---|---|---|
| Density | `kg/m^3` | `rho`, environ `1000` pour l'eau, `1.2` pour l'air |

La tâche d'écoulement du même problème a aussi besoin de la viscosité
dynamique. La base de données de matériaux fournie contient **Water** (eau) et
**Air**, ce qui constitue généralement toute cette étape.

### 2.4 Onglet Conditions aux limites

Les conditions de la section 1.5 sont proposées dès qu'une tâche *Contaminant
dispersion* figure dans l'enchaînement des tâches. Sélectionnez d'abord une
entité dans l'arbre `Modèle` ; le gestionnaire propose alors les conditions
valables pour ce type d'entité. Cochez-en une pour l'affecter, puis modifiez ses
composantes dans l'arbre inférieur.

Chaque composante est une table en fonction du temps. Cliquez sur **Modifier les
valeurs dépendant du temps** pour ouvrir l'éditeur de composantes, où les
valeurs se saisissent en fonction du temps ; entre deux instants donnés, la
valeur est interpolée linéairement, et après le dernier instant, la dernière
valeur est conservée. C'est ainsi que l'on modélise un événement de rejet.

### 2.5 Conditions initiales

L'onglet `Conditions initiales` propose *Particle concentration*. Elle définit
le champ de départ du premier calcul.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur de dispersion utilise l'entrée **GMRES**, car sa
matrice n'est pas symétrique.

Une **dispersion de contaminants stationnaire avec diffusion** demande ici de
l'attention. C'est une seule résolution linéaire, il n'y a donc pas
d'itérations du groupe de tâches sur lesquelles se rabattre, et un système
dominé par la diffusion converge lentement avec la limite GMRES par défaut de 10
itérations internes fois 10 itérations externes. Le solveur matriciel n'avertit
pas lorsqu'il atteint sa limite : lisez son tableau d'itérations dans le journal
du solveur et, s'il va jusqu'à la dernière itération externe avec un résidu
encore bien au-dessus de la valeur de convergence du solveur, augmentez le
nombre d'itérations externes. Une résolution non convergée donne une réponse
fausse sans aucun autre signe.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer - *Particle
concentration*, *Particle rate* ou *Relative saturation*. Dans un calcul
transitoire, l'historique est affiché par `Rapport` -> `Points de surveillance`.

Un point de surveillance à l'endroit qui vous intéresse donne la réponse sous la
forme généralement souhaitée : la concentration en fonction du temps en un lieu
donné, lue sur un graphique plutôt que dans une suite d'images.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste les variables calculées et contrôle la vue 3D. La
concentration de particules et la saturation relative sont des scalaires aux
nœuds.

L'onglet `Enregistrements` du dock `Modèle` liste les enregistrements de
résultats - un par pas de temps écrit d'un calcul transitoire. L'animation des
enregistrements montre la dispersion, et le bouton rouge d'enregistrement en bas
de l'arbre écrit l'animation dans un fichier vidéo.

---

## 3. Tutoriel - écoulement transitoire avec dispersion de contaminants

**Objectif.** Rejeter un contaminant dans un écoulement convergé et suivre le
panache à travers un canal dans le temps.

C'est le déroulement en deux étapes de la section 1.7, et il reprend le
tutoriel fourni *Dispersion de contaminants dans les fluides*.

### Étape 1 - partir d'un écoulement convergé

Faites d'abord converger le tutoriel du canal stationnaire du manuel
[Écoulement des fluides](theory_fluid.md). Un calcul de dispersion démarré à
partir du repos gaspille ses premiers pas de temps à établir le champ
d'écoulement au lieu de transporter quoi que ce soit.

### Étape 2 - ajouter la tâche de dispersion

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Conservez le groupe de tâches et la tâche **Incompressible viscous flow**.
3. Ajoutez une tâche **Contaminant dispersion** dans le même groupe.
4. Réduisez le **Nb d'itérations** du groupe à une valeur comme `20` - ce sont
   désormais les itérations non linéaires *par pas de temps*, et non les
   itérations d'une résolution stationnaire. Ne modifiez pas la valeur
   **Convergence** ; la plupart des pas se stabiliseront avant le vingtième
   passage et se termineront plus tôt, et ceux qui ne le font pas sont ceux qui
   méritent les vingt passages.

*Contaminant dispersion* nécessite la présence de la tâche d'écoulement ; elle
lit le champ de vitesse et ne le calcule jamais.

### Étape 3 - configurer le solveur temporel

Ouvrez l'onglet `Problème` et, dans le groupe *Solveur temporel*, réglez :

| Champ | Valeur |
|---|---|
| Activer | coché |
| Approximation | `Backward difference (stable)` |
| Temps de début | `0` |
| Taille du pas de temps | d'après la règle de Courant ci-dessous |
| Nombre de pas de temps | suffisant pour faire traverser le domaine au panache |
| Fréquence de sortie | `1` pour une animation fluide, davantage pour garder un fichier petit |

Choisissez le pas de temps d'après le maillage et la vitesse de l'écoulement,
de sorte que le fluide traverse environ un élément par pas :

```
dt ~ h / |v|
```

Choisissez ensuite le nombre de pas d'après le temps nécessaire au panache pour
traverser le domaine, `L / |v|`, divisé par ce pas.

Dans le groupe *Paramètres de la dispersion de contaminants* du même onglet,
laissez les deux valeurs à `0` pour une advection pure, ou saisissez un
**Coefficient de diffusion** effectif d'après le tableau de la section 1.3 si la
largeur du panache compte. Une **Saturation maximale** n'a de sens que pour une
vapeur ou une substance dissoute ; elle ne doit pas être inférieure à la
concentration de rejet de l'étape 4.

### Étape 4 - rejeter le contaminant

1. Sélectionnez l'entité surface d'entrée.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Particle concentration**.
3. Au lieu de saisir une valeur unique, cliquez sur **Modifier les valeurs
   dépendant du temps**.
4. Dans l'éditeur de composantes, saisissez un profil de rejet - par exemple `0`
   au temps `0`, la concentration de rejet au moment où le rejet commence, et
   `0` à nouveau quand il se termine.

Entre deux instants donnés, la valeur est interpolée linéairement, et après le
dernier instant, la dernière valeur est conservée ; un rejet qui doit commencer
brusquement nécessite donc deux entrées rapprochées. C'est cette condition
dépendant du temps qui transforme une entrée stationnaire en événement de rejet,
et c'est le cœur d'un modèle de dispersion.

### Étape 5 - conserver les conditions de l'écoulement

La paroi **Wall**, la condition d'entrée et la sortie **Pressure (implicit)**
restent inchangées. L'écoulement est toujours résolu à chaque pas de temps - la
dispersion s'appuie sur lui.

### Étape 6 - ajouter un point de surveillance

`Problème` -> `Définir les points de surveillance`, placez un point en aval et
réglez sa variable sur **Particle concentration**. Le temps d'arrivée et la
concentration maximale en un lieu donné se lisent bien plus facilement sur un
historique que dans une suite d'images colorées.

### Étape 7 - résoudre avec redémarrage

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Cochez **Redémarrer le solveur / continuer**.

C'est le redémarrage qui fait du champ stationnaire convergé le point de départ
de la marche en temps. Sans lui, le calcul part du repos et les premiers pas
sont gaspillés.

### Étape 8 - examiner les résultats

- Dock `Modèle`, onglet `Enregistrements` : un enregistrement par pas écrit.
  Parcourez-les ou animez-les pour voir le panache se déplacer.
- Onglet `Résultats` : affichez **Particle concentration**. Fixez la plage
  d'affichage pour tous les enregistrements afin que les couleurs aient la même
  signification sur chaque image - sinon l'animation se remet à l'échelle
  d'elle-même et le panache semble ne pas décroître.
- `Rapport` -> `Points de surveillance` trace l'historique de la concentration
  à la sonde. Relevez-y le temps d'arrivée et comparez-le avec `L / |v|`, le
  temps de parcours qu'implique l'écoulement moyen - c'est la validation la
  moins coûteuse de tout le modèle.
- Avec un coefficient de diffusion nul, le panache ne s'étale que par diffusion
  numérique ; sa **largeur** est donc une propriété du maillage et du pas de
  temps, et non de la physique. Avec un coefficient de diffusion effectif, la
  largeur est un résultat - vérifiez-la sur un maillage plus fin avant de vous y
  fier.

### Dépannage

| Symptôme | Cause |
|---|---|
| La concentration ne quitte jamais l'entrée | le champ d'écoulement est nul - la tâche d'écoulement manque dans le groupe, ou le calcul n'a pas été redémarré à partir d'un écoulement convergé |
| Le panache s'étale presque immédiatement | le nombre de Courant est très supérieur à 1, ou le maillage est trop grossier le long du trajet |
| La concentration devient négative ou dépasse | le pas de temps est trop grand pour le maillage, l'advection stabilisée oscille. De légers dépassements par défaut près d'un front raide sont normaux ; la solution n'est pas écrêtée à zéro, car l'écrêtage ajouterait du contaminant |
| Un seul enregistrement a été écrit | la fréquence de sortie vaut `0`, ce qui n'écrit que le dernier pas |
| Le résidu de chaque pas reste élevé | trop peu d'itérations par pas de temps - l'écoulement n'est pas convergé à l'intérieur de chaque pas |
| Le panache ne s'étale jamais latéralement | le coefficient de diffusion est nul, l'étalement latéral ne vient donc que du champ d'écoulement - saisissez un coefficient de diffusion effectif, voir la section 1.3 |
| Le panache s'étale d'un coup sur tout le domaine | le coefficient de diffusion est beaucoup trop grand - comparez-le avec `U * L` |
| Le journal avertit que la concentration imposée dépasse la saturation maximale | une condition aux limites ou initiale est au-dessus de la saturation maximale ; la limite n'agit que sur le taux de particules, vérifiez donc les deux valeurs |
| Une résolution stationnaire avec diffusion semble fausse | le solveur matriciel s'est arrêté sur sa limite d'itérations - augmentez les itérations externes de GMRES, voir la section 2.6 |
| La concentration dépasse la valeur d'entrée ou devient négative près des parois, mais ne continue pas de croître | advection presque pure - le coefficient de diffusion est proche de zéro. Saisissez un coefficient de diffusion effectif ; sur un maillage grossier, vérifiez aussi que le champ d'écoulement est convergé |
| La concentration dépasse la valeur d'entrée et croît en un motif bruité | l'écoulement n'est pas convergé - `Residual ratio` reste proche de `1` à chaque pas. Sa vitesse est loin d'être à divergence nulle et aucun transport sur celle-ci n'est fiable ; faites d'abord converger l'écoulement (maillage, nombre de Reynolds, itérations par pas) |

---

## 4. Vérifier un modèle

**Temps de parcours.** Le temps qu'un traceur met pour parcourir une distance
`L` à la vitesse moyenne `V` vaut `L/V`. Relevez le temps d'arrivée sur
l'historique d'un point de surveillance et comparez. Cela confirme que la tâche
de dispersion s'appuie bien sur le champ d'écoulement attendu, et cela détecte un
calcul qui n'a pas été redémarré à partir d'un écoulement convergé.

Il vaut mieux vérifier d'abord le champ d'écoulement lui-même, avec le bilan de
masse et les écoulements analytiques indiqués dans le manuel
[Écoulement des fluides](theory_fluid.md). Un modèle d'écoulement non convergé
échoue à toutes les vérifications pour la même raison, et aucun transport sur
celui-ci n'est fiable.

---

## 5. Limitations

- **Pas de modèle de turbulence.** L'écoulement est laminaire, voir le manuel
  [Écoulement des fluides](theory_fluid.md) ; le mélange d'un écoulement
  turbulent doit donc être saisi sous forme de coefficient de diffusion
  effectif.
- **Diffusion constante et isotrope.** Le coefficient de diffusion est une
  valeur unique pour tout le modèle ; il ne peut pas varier dans l'espace ou le
  temps, suivre la direction de l'écoulement, ni provenir d'un modèle de
  turbulence. Il doit être estimé comme une diffusivité effective - voir la
  section 1.3.
- **Saturation maximale constante.** La saturation ne suit pas la température
  d'un résultat *Heat transfer in fluids*, et elle ne limite qu'un taux de
  particules ou un flux de particules positif. Les concentrations imposées par
  des conditions ne sont pas limitées, et le champ n'est pas écrêté.
- **Le couplage avec l'écoulement est à sens unique.** L'écoulement entraîne le
  transport ; le transport ne modifie jamais l'écoulement.
- **Non monotone pour une advection presque pure.** Galerkin avec SUPG produit
  des dépassements par excès et par défaut lorsque le coefficient de diffusion
  est proche de zéro, les plus forts aux parois adhérentes - voir la
  section 1.4.
