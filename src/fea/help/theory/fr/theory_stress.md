# Analyse des contraintes - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur structurel (`RSolverStress`) de Range FEA, qui
traite à la fois les types de problèmes **Analyse des contraintes** (*Stress
analysis*) et **Analyse modale** (*Modal analysis*) : les équations qu'il
résout, la signification de chaque donnée d'entrée qu'il accepte, les parties de
l'interface graphique qui le pilotent, et deux tutoriels complets.

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - analyse statique des contraintes](#3-tutoriel---analyse-statique-des-contraintes)
4. [Tutoriel - analyse modale](#4-tutoriel---analyse-modale)
5. [Vérifier un modèle](#5-vérifier-un-modèle)
6. [Limitations](#6-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnue principale - le champ de déplacement

L'inconnue principale est le **vecteur déplacement** `u = (ux, uy, uz)` en
chaque nœud, en mètres. Chaque nœud porte trois degrés de liberté ; un maillage
de `N` nœuds donne donc un système de `3N` équations. Les déformations, les
contraintes et les forces de réaction sont toutes obtenues à partir du champ de
déplacement après la résolution.

### 1.2 Équations fondamentales

Le solveur met en œuvre l'**élasticité linéaire isotrope en petites
déformations**. Trois relations la définissent.

**Équilibre** d'un corps soumis à une charge volumique `b` et à une traction
surfacique `t` :

```
div(sigma) + b = 0
```

Relation **déformations-déplacements**, en petites déformations :

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

Loi **de comportement** avec un terme thermique :

```
sigma = D * ( eps - alpha * dT )
```

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `u` | déplacement | `m` | inconnue résolue |
| `sigma` | contrainte de Cauchy | `Pa` | obtenue |
| `eps` | déformation de l'ingénieur | `-` | obtenue |
| `E` | module d'élasticité | `Pa` | propriété du matériau |
| `v` | coefficient de Poisson | `-` | propriété du matériau |
| `rho` | masse volumique | `kg/m^3` | propriété du matériau |
| `alpha` | coefficient de dilatation thermique | `1/K` | propriété du matériau |
| `dT` | élévation de température par rapport à l'état sans contrainte | `K` | voir 1.5 |

Les quatre propriétés de matériau sont **requises** - une entité à laquelle il
en manque une n'est pas résolue. Les deux coefficients d'amortissement de
Rayleigh d'une analyse dynamique (voir 1.7) sont facultatifs.

### 1.3 Matrice d'élasticité

Les **éléments volumiques** utilisent la matrice d'élasticité isotrope
tridimensionnelle complète en notation de Voigt, avec le vecteur des contraintes
ordonné `[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]` :

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

Les **éléments surfaces** utilisent la forme bidimensionnelle en **déformations
planes**, avec le vecteur des contraintes `[sigma_xx, sigma_yy, tau_xy]`,
évaluée dans le plan local de l'élément puis ramenée dans le repère global :

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Il s'agit de **déformations** planes, et non de contraintes planes. Une plaque
mince chargée dans son propre plan sera donc plus rigide que ne le prédirait une
idéalisation en contraintes planes.

Les **éléments lignes** sont traités comme des **barres** : seule la rigidité
axiale `E * A` selon la direction de l'élément est assemblée, `A` étant la
section de la ligne. Il n'y a pas de rigidité en flexion, en cisaillement ni en
torsion, même pour les types d'éléments nommés « beam » (poutre).

### 1.4 Discrétisation par éléments finis

La forme faible de l'équilibre donne, pour chaque élément,

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(`stiffness matrix` = matrice de rigidité, `consistent mass matrix` = matrice de
masse cohérente, `load vector, see 1.5` = vecteur des charges, voir 1.5), où `B`
est la matrice déformations-déplacements construite à partir des dérivées des
fonctions de forme. Chaque matrice élémentaire est `3n x 3n` pour un élément à
`n` nœuds, les degrés de liberté étant entrelacés selon
`[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Mesures des éléments.** Comme ailleurs dans Range FEA, les entités de
dimension inférieure reçoivent une mesure géométrique afin de pouvoir
représenter des corps réels :

| Entité | Mesure | Où elle est définie |
|---|---|---|
| Volume | 1 | - |
| Surface | épaisseur de la surface | propriétés géométriques de l'entité |
| Ligne | section de la ligne | propriétés géométriques de l'entité |
| Point | volume du point | propriétés géométriques de l'entité |

Une surface d'épaisseur nulle ou une ligne de section nulle n'apporte ni
rigidité ni masse - elle peut tout de même porter des charges, ce qui est
exactement ce que l'on veut pour une face chargée d'un solide maillé.

La matrice de masse n'est assemblée que lorsqu'elle est nécessaire, c'est-à-dire
pour une analyse transitoire dynamique ou pour une analyse modale.

### 1.5 Charges, conditions aux limites et d'environnement

#### Conditions aux limites

| Condition aux limites | Type | S'applique à | Composantes |
|---|---|---|---|
| Displacement | explicite (Dirichlet) | point, ligne, surface, volume | Displacement in X, Y, Z `[m]` |
| Normal displacement | explicite, repère local | surface | Displacement `[m]` |
| Roller displacement | explicite, repère local | point, ligne, surface | Displacement `[m]` |
| Force | naturelle | point, ligne, surface | Force in X, Y, Z `[N]` |
| Force (unit area) | naturelle | surface | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | naturelle | surface | Pressure `[Pa]` |
| Weight | naturelle | point, ligne, surface | Weight `[kg]` |

**Displacement** (déplacement) est une condition à composantes **facultatives** :
chacune de ses trois composantes peut être activée ou désactivée
individuellement. Une composante activée bloque cette direction globale de
chaque nœud de l'entité à la valeur donnée ; une composante désactivée laisse
cette direction entièrement libre. Désactiver *Displacement in Y* et
*Displacement in Z* crée donc un appui qui maintient X et laisse le nœud glisser
dans le plan YZ.

Utilisez plutôt *Roller displacement* lorsque les directions libres ne sont pas
les directions globales mais le plan tangent d'une surface courbe.

**Normal displacement** (déplacement normal) et **Roller displacement**
(déplacement d'appui glissant) agissent dans un **repère local** construit pour
chaque nœud bloqué. Par défaut, le repère suit la géométrie :

- sur une **surface**, l'axe X local est la normale moyenne des éléments
  surfaces adjacents ;
- sur une **ligne**, le repère local est construit à partir de la direction de
  l'élément ;
- sur un **point**, il n'y a pas de géométrie à suivre ; la direction saisie
  avec la condition aux limites est donc toujours utilisée.

Cocher **Utiliser la direction locale saisie** remplace ce comportement pour une
surface ou une ligne et prend l'axe X local dans la direction saisie en dessous.
Utilisez cette option lorsque la direction à bloquer n'est pas celle que donne
le maillage - un appui glissant qui doit coulisser le long d'une surface à
facettes selon un axe global, par exemple.

Les blocages provenant de différentes entités se **combinent** sur un nœud
qu'elles partagent. Le solveur rassemble tous les blocages de déplacement qui
agissent sur un nœud, chacun étant une relation `d . u = v` portant sur une
direction, et réduit cet ensemble à au plus trois directions bloquées
mutuellement perpendiculaires par Gram-Schmidt, en appliquant les mêmes
opérations aux valeurs imposées. Ces directions bloquées deviennent les premiers
axes du repère nodal ; ce qui reste complète le repère et demeure libre. Un nœud
peut donc être bloqué dans une direction par un *Displacement* global, dans une
autre par un *Roller displacement* sur une face inclinée, et rester libre dans
la troisième - les deux conditions ne s'écrasent pas mutuellement, et une
composante exprimée dans le repère global n'est jamais relue silencieusement
dans le repère local d'une autre condition.

Deux entités qui imposent **des valeurs différentes dans la même direction** ne
peuvent pas être satisfaites simultanément. Le solveur le détecte pendant
l'orthogonalisation et s'arrête avec une erreur nommant le nœud, au lieu de
trancher en faveur de la condition lue en dernier.

Les matrices élémentaires et les vecteurs de charges d'un nœud bloqué sont
tournés dans ce repère, le blocage est appliqué, et le déplacement obtenu est
ramené dans le repère global. Dans ce repère :

- **Normal displacement** bloque les trois composantes locales, et le scalaire
  imposé est appliqué selon la normale locale. Le nœud se déplace exactement de
  la valeur donnée selon la normale et est fixe par ailleurs.
- **Roller displacement** ne bloque que la composante normale locale sur une
  surface ou un point ; le nœud est donc libre de glisser dans le plan tangent.
  Sur une entité ligne, elle bloque à la place les deux composantes
  perpendiculaires à la ligne ; le nœud est donc libre de glisser le long de la
  ligne.

**Force** est une force totale en newtons répartie sur l'entité à laquelle elle
est affectée - elle est divisée par la longueur de l'entité pour une ligne, par
l'aire de l'entité pour une surface et par le nombre de points d'une entité
point. Raffiner le maillage ne modifie donc pas la charge totale.

**Force (unit area)** (force par unité de surface) est une traction en `N/m^2`
appliquée dans la direction globale donnée, intégrée sur l'aire de l'élément.

**Pressure** (pression) agit selon la **normale** à la surface. Pour chaque
surface frontière, le solveur détermine si sa normale pointe vers l'élément
volume calculable adjacent et inverse le signe en conséquence, de sorte qu'une
pression positive pousse toujours sur le corps. Une surface qui ne délimite
aucun élément volume n'est jamais classée comme orientée vers l'intérieur ; la
charge y agit donc selon l'**opposé** de la normale à la surface - vérifiez dans
ce cas la direction de la charge obtenue.

**Weight** (poids) est une masse en kilogrammes attachée à l'entité. Elle est
multipliée par l'accélération de la pesanteur de la condition d'environnement et
répartie sur l'entité de la même manière que Force. Utilisez-la pour des masses
attachées non modélisées.

#### Conditions d'environnement

| Condition d'environnement | Composantes | Effet |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | poids propre et condition aux limites Weight |
| Temperature | Temperature `[K]` | température de référence sans contrainte |

Le **poids propre** est généré automatiquement pour chaque entité qui possède à
la fois une masse volumique et une mesure géométrique, sous la forme `rho * g`
intégrée sur l'élément. Il apparaît dès qu'une condition d'environnement
d'accélération de la pesanteur est affectée ; la valeur par défaut est
`-9.80665 m/s^2` en Z.

**Charge thermique.** L'écart de température qui pilote le terme de dilatation
thermique vaut

```
dT = element temperature - environment temperature
```

(`element temperature` = température de l'élément, `environment temperature` =
température d'environnement.) La **température d'environnement** provient de la
condition d'environnement *Temperature* et constitue l'état de référence sans
contrainte. La **température de l'élément** provient d'un solveur de transfert
de chaleur exécuté plus tôt dans le même enchaînement des tâches du problème,
transmise par les données partagées des solveurs. Exécuter *Heat transfer* puis
*Stress analysis* dans un même enchaînement de tâches donne donc une analyse des
contraintes thermiques sans autre configuration. Sans solveur thermique, la
température de l'élément reste à sa valeur par défaut et le terme thermique est
l'écart par rapport à la seule condition d'environnement.

#### Conditions initiales

| Condition initiale | Effet |
|---|---|
| Displacement | champ de déplacement initial d'une analyse transitoire dynamique |
| Structural velocity | champ de vitesse initial d'une analyse transitoire dynamique |
| Pressure | charge de pression surfacique initiale |

Les conditions initiales ne sont lues qu'au premier calcul d'une analyse
transitoire. Une analyse dynamique sans elles part du repos. Les directions
bloquées ne gardent pas de vitesse initiale propre, elles suivent le déplacement
imposé.

#### Couplage avec un solveur de fluide

Le solveur reprend le résultat **Pressure** d'une résolution précédente et
l'utilise comme charge surfacique partout où aucune condition aux limites de
pression explicite n'est affectée. Une tâche de fluide suivie d'une tâche de
contraintes dans le même enchaînement de tâches transfère donc automatiquement
la pression du fluide sur la structure.

Dans l'autre sens, chaque solveur non structurel déplace le maillage selon le
résultat **Displacement** stocké avant de s'exécuter, puis le remet en place
ensuite ; une résolution fluide ou thermique qui suit une résolution de
contraintes voit donc la forme déformée.

### 1.6 Analyse statique

Avec le solveur temporel désactivé et le type de problème réglé sur *Stress
analysis*, le solveur assemble et résout

```
K * u = f
```

`K` est symétrique définie positive une fois que les modes de corps rigide ont
été supprimés par les blocages de déplacement ; elle est donc résolue par la
méthode du **gradient conjugué** avec un préconditionnement de Jacobi et une
taille de bloc de 3.

Si le modèle est insuffisamment bridé, la matrice est singulière et la
résolution ne convergera pas. Chaque modèle statique a besoin de suffisamment de
blocages pour supprimer les six modes de corps rigide - trois translations et
trois rotations.

### 1.7 Analyse transitoire

Avec le solveur temporel activé, les *Paramètres de l'analyse transitoire des
contraintes* sélectionnent l'un des deux types d'analyse.

#### Analyse dynamique

Le choix par défaut. Le solveur intègre les équations de l'élastodynamique
linéaire

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

si bien que l'inertie, les ondes de contrainte, les vibrations, l'amplification
dynamique et la résonance sont toutes reproduites.

**Amortissement.** `C` est un amortissement de Rayleigh, construit élément par
élément à partir de deux propriétés de matériau facultatives,

```
C = alpha_M * M + beta_K * K
```

*Rayleigh mass damping coefficient* `alpha_M` `[1/s]` amortit les basses
fréquences, *Rayleigh stiffness damping coefficient* `beta_K` `[s]` les hautes
fréquences. Le taux d'amortissement d'un mode de pulsation `omega` vaut

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

si bien qu'un taux cible `zeta` à deux fréquences `omega_1` et `omega_2` est
obtenu avec

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Prenez les deux fréquences dans une analyse modale, autour de la bande qui vous
intéresse. Les taux d'amortissement structurel typiques vont de 0.01 à 0.05.
Sans ces deux propriétés, il n'y a aucun amortissement physique.

**Intégration temporelle.** La méthode de Newmark avec la modification de
Hilber-Hughes-Taylor (HHT). Avec le déplacement `u`, la vitesse `v` et
l'accélération `a` au début du pas, chaque pas résout

```
( a0*M + (1+alpha)*(K + a1*C) ) * u_n+1 =
      (1+alpha)*f_n+1 - alpha*f_n
    + M * ( a0*u_n + a2*v_n + a3*a_n )
    + (1+alpha) * C * ( a1*u_n + a4*v_n + a5*a_n )
    + alpha * ( C*v_n + K*u_n )
```

```
a0 = 1/(beta*dt^2)   a1 = gamma/(beta*dt)   a2 = 1/(beta*dt)
a3 = 1/(2*beta) - 1  a4 = gamma/beta - 1    a5 = dt/2 * (gamma/beta - 2)
```

puis met à jour

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

La matrice effective est symétrique définie positive ; elle est donc résolue par
la même méthode CG qu'une analyse statique.

- Avec **HHT alpha = 0** (par défaut), le schéma est un Newmark simple avec
  `gamma = 1/2` et `beta = max(theta/2, 1/4)`, où `theta` est le coefficient
  d'approximation du schéma temporel (`1` arrière, `0.5` centré, `0` avant). Le
  centré et l'avant donnent tous deux le schéma de l'accélération moyenne -
  inconditionnellement stable, précis au second ordre et sans amortissement
  numérique. L'arrière donne `beta = 1/2`, également inconditionnellement stable
  mais qui allonge les périodes.
- Avec **HHT alpha** entre `-1/3` et `0`, `gamma = 1/2 - alpha` et
  `beta = (1 - alpha)^2 / 4`. Le schéma reste inconditionnellement stable et
  précis au second ordre, et amortit les hautes fréquences mal résolues par un
  maillage grossier tout en laissant les modes bas presque intacts. `-0.05` à
  `-0.1` est un choix courant lorsque la réponse est polluée par un bruit haute
  fréquence.

**Accélération initiale.** Au premier pas, l'accélération est résolue à partir
de l'équilibre, `M*a = f - K*u - C*v` ; une charge appliquée au départ produit
donc la bonne accélération initiale. Les directions sans masse reçoivent zéro.

**Masse concentrée.** L'option **Matrice de masse concentrée** remplace la masse
cohérente par une masse diagonale de même masse totale, en mettant à l'échelle
la diagonale de la matrice cohérente (concentration HRZ, qui reste positive
aussi pour les éléments quadratiques). Elle abaisse légèrement les fréquences
propres et donne une réponse plus lisse derrière les fronts d'onde raides.

**Choix du pas de temps.** Le schéma est stable pour tout `dt`, mais la
précision exige environ 20 pas ou plus par période du mode le plus élevé que
vous souhaitez résoudre. Pour une onde qui traverse le maillage, un pas proche
du temps que met l'onde à traverser un élément (`dt ~ h/c`, `c = sqrt(E/rho)`)
est un bon point de départ.

**Résultats.** Outre le déplacement et les contraintes, une analyse dynamique
stocke les vecteurs aux nœuds *Structural velocity* et *Structural
acceleration*. Ils portent aussi l'état vers le pas de temps suivant et lors
d'un redémarrage du solveur.

#### Analyse quasi statique

Chaque pas de temps est une solution statique indépendante

```
K * u(t) = f(t)
```

en négligeant l'inertie et l'amortissement. Utilisez-la pour un historique de
charge appliqué lentement par rapport aux périodes de la structure, lorsque
seule compte la suite des états d'équilibre. Elle ne nécessite ni masse
volumique ni limite de pas de temps.

### 1.8 Analyse modale

Avec le type de problème réglé sur *Modal analysis*, le solveur assemble `K` et
`M` et résout le problème aux valeurs propres généralisé

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(`with` = avec.) Chaque couple propre est un **mode** : `phi` est la déformée
modale, stockée et affichée comme un champ de déplacement, et `lambda` est la
valeur propre dont la fréquence propre se déduit par

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

Le solveur effectue lui-même cette conversion : les paramètres modaux et la vue
3D indiquent la fréquence propre en Hz, tandis que le journal du solveur affiche
la valeur propre sous-jacente à côté. Un mode que l'itération aux valeurs
propres n'a pas réussi à résoudre est indiqué à `0` Hz avec un avertissement
dans le journal.

Deux méthodes sont disponibles, choisies par le paramètre de méthode modale :

| Méthode modale | Solveur aux valeurs propres | Extrait |
|---|---|---|
| Multiple modes | itération sur sous-espace avec projection de Rayleigh-Ritz | le nombre de modes demandé |
| Only most dominant mode | itération inverse de la puissance avec quotient de Rayleigh | un seul mode |

Les deux méthodes itèrent sur `K^-1 * M` ; elles convergent donc vers les
fréquences les **plus basses** - qui sont celles qui comptent. Chaque pas est
une résolution linéaire avec `K`, utilisant les paramètres du gradient conjugué
de la configuration du solveur matriciel ; le coût d'une analyse modale est donc
environ celui d'une résolution statique multiplié par le nombre de vecteurs du
sous-espace et d'itérations.

L'itération sur sous-espace porte un bloc de vecteurs, projette `K` et `M` sur le
sous-espace qu'ils engendrent et résout exactement le petit problème aux valeurs
propres dense obtenu. Quelques vecteurs supplémentaires au-delà du nombre de
modes demandé sont conservés, car ils font converger nettement plus vite les
modes recherchés. Pour `p` modes demandés, le bloc contient `min(2p, p+8)`
vecteurs, ce qui est le compromis habituel entre la vitesse de convergence et le
coût d'une itération. Les valeurs propres sortent directement sous forme de
`lambda` et sont triées par ordre croissant ; **le mode 0 est donc le mode
fondamental**.

**C'est là que passe le temps de calcul.** Une itération sur sous-espace
effectue une résolution linéaire complète par vecteur du bloc ; demander 10
modes coûte donc 18 résolutions par itération, et en demander 100 en coûte 108.
L'itération ne s'arrête plus tôt que lorsque **toutes** les valeurs propres
demandées se sont stabilisées à la valeur de convergence près, et le mode
demandé le plus élevé est toujours le dernier à se stabiliser - un grand nombre
de modes rend donc chaque itération coûteuse et un arrêt anticipé peu probable.
Ne demandez que les modes que vous comptez réellement examiner.

Si les modes demandés ne se sont pas stabilisés à la dernière itération, le
solveur l'indique dans le journal et donne la variation atteinte, au lieu de
présenter les valeurs non convergées sans commentaire. Les modes les plus bas
d'un tel calcul sont généralement encore fiables ; les plus élevés ne le sont
pas.

La matrice de masse est toujours assemblée pour une analyse modale ; la masse
volumique doit donc être présente sur chaque entité qui y participe.

**Analyse modale précontrainte.** Avant d'assembler les matrices, une analyse
modale déplace chaque nœud selon le résultat de déplacement d'une résolution
précédente, puis rétablit le maillage ensuite. Exécuter *Stress analysis* puis
*Modal analysis* dans un même enchaînement de tâches donne donc les modes de la
structure **chargée et déformée** plutôt que ceux de la structure non déformée.
La déformée modale stockée est le vecteur propre ajouté à ce déplacement
initial.

Seule la géométrie est mise à jour - la matrice de rigidité reste la matrice
élastique linéaire évaluée sur le maillage déplacé. Cela rend compte du
changement de forme, mais pas de la rigidification géométrique d'un élément
tendu.

L'analyse modale est **exclusive** : elle ne peut être combinée avec aucune
autre physique, sauf une analyse des contraintes qui la précède.

### 1.9 Résultats dérivés

Après la résolution, les contraintes aux éléments sont obtenues à partir du
champ de déplacement comme la moyenne sur les points d'intégration de

```
sigma = D * ( B * u  -  eps_th )
```

avec la déformation thermique `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]` pour
les éléments volumiques. Les éléments surfaces sont en déformations planes : la
dilatation hors de leur plan est empêchée, ce qui porte la déformation thermique
dans le plan à `eps_th = (1+v) * alpha * dT * [1, 1, 0]`. La même déformation
thermique entre dans la charge thermique `f_th = INT( B^T * D * eps_th )`. Les
éléments lignes donnent `E * (eps - alpha * dT)`.

À partir des six composantes, le solveur forme deux invariants et les combine
quadratiquement en contrainte de von Mises :

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(`normal part` = partie normale, `shear part` = partie de cisaillement), ce qui
correspond à la définition classique

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

Pour les éléments surfaces, on utilise les équivalents bidimensionnels, `QN`
calculée à partir des deux contraintes normales dans le plan et
`QS = sqrt(3 * tau_xy^2)`. Pour les éléments lignes, seule la valeur axiale est
donnée et la partie de cisaillement est nulle.

Les **forces nodales** sont obtenues élément par élément comme la force
élastique interne

```
f = K * u - f_th
```

et cumulées aux nœuds. En un nœud bloqué, c'est la force de réaction ; en un
nœud libre non chargé, elle doit être proche de zéro pour une résolution
statique convergée, ce qui fait du résultat Force un contrôle utile du résidu.
Un corps libre de se dilater thermiquement ne présente donc ni forces ni
contraintes.

| Résultat | S'applique à | Signification |
|---|---|---|
| Displacement `[m]` | nœud | vecteur déplacement, ou déformée modale dans une analyse modale |
| Von Mises stress `[Pa]` | élément | `sqrt(QN^2 + QS^2)` comme ci-dessus |
| Normal stress `[Pa]` | élément | `QN` |
| Shear stress `[Pa]` | élément | `QS` |
| Normal stress in X, Y, Z `[Pa]` | élément | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | élément | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | nœud | force nodale obtenue, réaction aux nœuds bloqués |

Les six composantes de contrainte sont données dans le repère **global** pour
les éléments volumiques. Les éléments surfaces et lignes évaluent leur
contrainte dans leur propre repère local d'élément ; leurs composantes sont donc
données dans ce repère : une surface renseigne `sigma_xx`, `sigma_yy` et
`tau_xy` dans le plan, et une ligne uniquement le `sigma_xx` axial.

### 1.10 Statique, transitoire ou modale ?

| | Statique | Transitoire dynamique | Transitoire quasi statique | Modale |
|---|---|---|---|---|
| Résout | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` dans le temps | `K*u = f(t)` à chaque instant | `K*phi = omega^2*M*phi` |
| Répond à | « de combien fléchit-elle et à quel point est-elle sollicitée » | « comment répond-elle à une charge qui varie rapidement » | « comment suit-elle une charge qui varie lentement » | « à quelles fréquences et selon quelle forme tend-elle à vibrer » |
| Nécessite une masse | non | oui | non | oui |
| Nécessite des blocages contre les mouvements de corps rigide | oui | oui | oui | oui |
| Enregistrements | un | un par pas de temps | un par pas de temps | un par mode extrait |

**Utilisez une analyse statique pour** les vérifications de résistance et de
rigidité, les limites de flèche, les forces de réaction, le poids propre, les
réservoirs sous pression, les contraintes thermiques. C'est le choix par défaut,
qui couvre la plupart des travaux.

**Utilisez une analyse transitoire dynamique pour** les impacts, les charges
appliquées ou relâchées brusquement, les historiques de chocs et de vibrations,
la propagation d'ondes et la réponse à une excitation harmonique proche d'une
résonance - partout où l'inertie compte.

**Utilisez une analyse transitoire quasi statique pour** une charge qui varie
dans le temps suffisamment lentement pour que la structure la suive en
équilibre.

**Utilisez une analyse modale pour** les fréquences propres et les déformées
modales, éviter la résonance avec une excitation connue, juger si une structure
est assez rigide dynamiquement, et détecter des mécanismes involontaires - une
valeur propre quasi nulle signifie généralement un blocage manquant ou une
pièce déconnectée.

---

## 2. Interface graphique

Tout ce qui est utile se trouve dans le dock **Solveur** (onglets `Problème`,
`Conditions aux limites`, `Conditions initiales`,
`Conditions d'environnement`, `Matériau`, `Résultats`), le menu **Problème**
et le dock **Modèle**.

### 2.1 Choix du problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Ajoutez une tâche et choisissez
au choix :

- **Stress analysis** - « Stress and linear elasto-dynamics analysis including
  modal analysis » (analyse des contraintes et élastodynamique linéaire, analyse
  modale comprise) ;
- **Modal analysis** - « Stress modal analysis » (analyse modale structurelle).

L'analyse des contraintes se combine librement avec d'autres physiques. Deux
chaînes utiles sont *Heat transfer* puis *Stress analysis* pour les contraintes
thermiques, et *Incompressible viscous flow* puis *Stress analysis* pour le
chargement en pression.

L'analyse modale exclut tous les autres types de problèmes sauf une analyse des
contraintes qui la précède, ce qui en fait une analyse modale précontrainte.

### 2.2 Onglet Problème

**Solveur temporel** - affiché pour l'analyse des contraintes, car c'est un type
de problème dépendant du temps. Laissez-le désactivé pour une analyse statique.

| Champ | Signification |
|---|---|
| Activer | active ou désactive la marche en temps transitoire |
| Approximation | fixe le beta de Newmark d'une analyse dynamique, voir 1.7 ; le centré est le choix recommandé |
| Temps de début | première valeur de temps en secondes ; ignorée lors d'un redémarrage du solveur |
| Temps de fin | `start time + time-step size * number of time-steps` (temps de début + taille du pas de temps × nombre de pas de temps), lecture seule |
| Taille du pas de temps | `dt` en secondes |
| Nombre de pas de temps | nombre de pas à calculer |
| Fréquence de sortie | écrit un fichier de résultats tous les N pas ; `0` n'écrit que le dernier pas |

**Paramètres de l'analyse transitoire des contraintes** - affiché pour l'analyse
des contraintes et actif uniquement lorsque le solveur temporel est activé.

| Champ | Signification | Valeur par défaut |
|---|---|---|
| Type d'analyse | *Dynamic* (avec inertie) ou *Quasi-static* (une résolution statique par pas de temps) | Dynamic |
| Amortissement numérique (alpha HHT) | de `-1/3` à `0` ; les valeurs négatives amortissent le bruit haute fréquence | 0 |
| Matrice de masse concentrée | utilise une matrice de masse diagonale au lieu de la matrice cohérente | désactivé |

Les deux derniers ne s'appliquent qu'à l'analyse dynamique et sont grisés dans
les autres cas.

**Paramètres de l'analyse modale** - affiché lorsque le type de problème est
*Modal analysis*.

| Champ | Signification | Valeur par défaut |
|---|---|---|
| Méthode modale | *Only most dominant mode* (uniquement le mode dominant) ou *Multiple modes* (plusieurs modes) | Multiple modes |
| Itérations | itérations du solveur aux valeurs propres ; désactivé pour la méthode du mode dominant | 100 |
| Extraire les modes | nombre de modes à extraire ; désactivé pour la méthode du mode dominant | 10 |
| Valeur de convergence | seuil de convergence du solveur aux valeurs propres | 1.0e-9 |

Les deux nombres sont indépendants : les itérations affinent l'ensemble des
modes à la fois, elles ne les produisent pas un par un.

**Extraire les modes est le paramètre qui décide de la durée du calcul.** Chaque
mode ajoute une résolution linéaire à chaque itération, et le calcul ne peut
s'arrêter plus tôt qu'une fois tous les modes demandés stabilisés ; le coût
croît donc plus vite que le nombre de modes. Dix est un point de départ
raisonnable et la valeur par défaut ; cent représente un calcul d'un tout autre
ordre de grandeur, et sur un grand maillage il peut prendre des heures.
N'augmentez cette valeur que si vous savez que vous avez besoin des modes plus
élevés.

Si un calcul prend plus de temps que prévu, le journal du solveur indique le
vecteur du sous-espace en cours de traitement, ce qui rend visible la
progression au sein d'une itération, et il affiche après chaque itération le
taux de convergence atteint à côté de la valeur requise.

### 2.3 Onglet Matériau

L'onglet `Matériau` liste le matériau affecté à l'entité sélectionnée. L'analyse
des contraintes requiert **ces quatre** propriétés :

| Propriété | Unités | Commentaire |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, environ `2.1e11` pour l'acier |
| Poisson ratio | `-` | `v`, environ `0.3` pour l'acier ; doit rester inférieur à `0.5` |
| Density | `kg/m^3` | nécessaire pour le poids propre, l'analyse dynamique et l'analyse modale |
| Thermal expansion coefficient | `1/K` | réglez-le sur `0` s'il n'y a pas de charge thermique |

Une entité à laquelle il en manque une n'est pas résolue, et le contrôle de la
configuration avertit avant le début du calcul.

Deux autres propriétés sont facultatives et ne sont lues que par une analyse
transitoire dynamique :

| Propriété | Unités | Commentaire |
|---|---|---|
| Rayleigh mass damping coefficient | `1/s` | `alpha_M`, voir 1.7 ; nul en son absence |
| Rayleigh stiffness damping coefficient | `s` | `beta_K`, voir 1.7 ; nul en son absence | Chaque propriété est une table en fonction de la température ; un module
dépendant de la température est donc pris en compte lorsqu'un solveur thermique
s'exécute en premier.

### 2.4 Onglet Conditions aux limites

L'onglet est divisé en une liste de gestion des conditions aux limites
disponibles en haut et un éditeur de la condition sélectionnée en bas.
Sélectionnez d'abord une entité dans l'arbre `Modèle` ; le gestionnaire propose
alors les conditions valables pour ce type d'entité et pour le problème
sélectionné. Cochez-en une pour l'affecter, puis modifiez ses composantes dans
l'arbre inférieur.

Les conditions de la section 1.5 sont proposées. Remarques sur leur
utilisation :

- **Displacement** affiche une case à cocher à côté de chacune de ses trois
  composantes. Seules les composantes cochées sont bloquées ; une face peut donc
  être maintenue en X et laissée libre en Y et Z. Le champ de valeur d'une
  composante décochée est grisé.
- **Normal displacement** et **Roller displacement** prennent par défaut leur
  repère local dans la géométrie - les normales moyennes des éléments d'une
  surface, la direction des éléments d'une ligne. Sur une entité **point**, il
  n'y a pas de géométrie à suivre ; l'éditeur *Direction locale* est donc
  toujours affiché et toujours utilisé.
- Sur une surface ou une ligne, l'arbre de la condition affiche une case
  **Utiliser la direction locale saisie**. Cochez-la pour faire apparaître
  l'éditeur *Direction locale* et prendre le repère dans la direction saisie au
  lieu du maillage. Les flèches de direction dessinées dans la vue 3D suivent
  celle des deux qui est active. Si un blocage de surface se comporte de manière
  étrange avec la case décochée, vérifiez que les normales de la surface sont
  cohérentes et synchronisez-les avec les outils de géométrie.
- Les blocages de plusieurs entités qui se rejoignent en un nœud sont combinés ;
  une face peut donc être maintenue dans une direction globale par
  *Displacement* et glisser en même temps sur un plan incliné. Si deux d'entre
  eux demandent des valeurs différentes dans la même direction, le calcul
  s'arrête avec une erreur nommant le nœud.
- **Pressure** et **Force (unit area)** ne s'appliquent qu'aux surfaces.
- Chaque composante est une table en fonction du temps ; une charge peut donc
  recevoir un historique temporel pour un calcul transitoire.

### 2.5 Conditions initiales et d'environnement

C'est dans l'onglet `Conditions d'environnement` que l'on active la
**pesanteur**. Affectez *Gravitational acceleration* à l'entité qui doit porter
son poids propre - généralement tout le modèle - et réglez le vecteur ; la
valeur par défaut est `-9.80665` en Z. Affectez *Temperature* pour définir la
température de référence sans contrainte d'une analyse des contraintes
thermiques.

L'onglet `Conditions initiales` propose *Displacement*, *Structural velocity* et
*Pressure*, appliquées au premier calcul d'une analyse transitoire.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Le solveur structurel utilise l'entrée **CG**, aussi bien
pour la résolution statique et transitoire que pour les résolutions internes de
l'itération aux valeurs propres.

Si une résolution statique ne converge pas, la cause habituelle est un modèle
insuffisamment bridé plutôt qu'un paramètre du solveur. Vérifiez les blocages
avant d'augmenter le nombre d'itérations.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer - typiquement
*Displacement* ou *Von Mises stress*. Dans un calcul transitoire, l'historique
est affiché par `Rapport` -> `Points de surveillance`.

### 2.8 Résultats et enregistrements

L'onglet `Résultats` liste les variables calculées et contrôle la vue 3D. Le
déplacement est un vecteur aux nœuds ; les trois mesures de contrainte sont des
scalaires aux éléments. Une analyse transitoire dynamique ajoute les vecteurs
aux nœuds *Structural velocity* et *Structural acceleration*.

Pour un tracé de la déformée, affichez *Displacement* et activez la mise à
l'échelle de la déformation dans les paramètres de la vue - les déplacements
réels sont généralement bien trop petits pour être visibles à l'échelle réelle.

L'onglet `Enregistrements` du dock `Modèle` liste les enregistrements de
résultats :

- **analyse statique** - un seul enregistrement ;
- **analyse transitoire** - un enregistrement par pas de temps écrit ;
- **analyse modale** - un enregistrement par mode extrait, numéroté à partir du
  fondamental. Parcourir les enregistrements revient à parcourir les modes, et
  les animer anime les déformées modales.

Pour un enregistrement modal, la vue 3D étiquette l'enregistrement avec le
numéro du mode et la fréquence propre de ce mode en Hz.

`Rapport` -> `Fichier journal du solveur` affiche la sortie complète du solveur,
y compris la valeur propre affichée pour chaque mode extrait et les statistiques
du déplacement, de la contrainte de von Mises et de la force après chaque
enregistrement.

---

## 3. Tutoriel - analyse statique des contraintes

**Objectif.** Calculer la flèche et les contraintes d'une poutre en porte-à-faux
chargée par son poids propre et une charge en bout, et lire la réaction à
l'appui.

Ce tutoriel suppose un modèle volumique maillé - un parallélépipède allongé
convient bien. Utilisez `Fichier` -> `Nouveau modèle`, dessinez un
parallélépipède avec `Géométrie` -> `Créer un élément` ou importez une
géométrie, et maillez-le en tétraèdres ; le tutoriel *Dessiner un cube* couvre ce
sujet. Marquez l'extrémité encastrée et l'extrémité chargée comme entités
surface distinctes afin de pouvoir leur affecter des conditions aux limites.

### Étape 1 - choisir le problème

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez une tâche et sélectionnez **Stress analysis**.
3. Confirmez avec `OK`.

### Étape 2 - affecter le matériau

1. Sélectionnez l'entité volume dans l'arbre `Modèle`.
2. Ouvrez l'onglet `Matériau` et affectez un matériau possédant les quatre
   propriétés, pour l'acier de construction :
   - **Modulus of elasticity** (module d'élasticité) = `2.1e11` `Pa`
   - **Poisson ratio** (coefficient de Poisson) = `0.3`
   - **Density** (masse volumique) = `7850` `kg/m^3`
   - **Thermal expansion coefficient** (coefficient de dilatation thermique) =
     `1.2e-5` `1/K`

### Étape 3 - garder l'analyse statique

Ouvrez l'onglet `Problème` et laissez **Activer** décoché dans le groupe
*Solveur temporel*. Le solveur assemble et résout alors `K*u = f` une seule
fois.

### Étape 4 - brider le modèle

1. Sélectionnez l'entité surface de l'extrémité encastrée.
2. Ouvrez l'onglet `Conditions aux limites` et cochez **Displacement**.
3. Laissez les trois composantes cochées et réglées sur `0`.

Cela fixe complètement la face et supprime les six modes de corps rigide.
Vérifiez que rien d'autre ne reste flottant - une pièce déconnectée sans blocage
rend le système singulier et la résolution ne convergera pas.

Pour un appui qui doit glisser au lieu d'être entièrement fixe, utilisez plutôt
**Roller displacement** sur la surface. Il ne bloque que la direction normale à
la surface et laisse le nœud libre de se déplacer dans le plan tangent.

### Étape 5 - appliquer les charges

**Poids propre.** Sélectionnez l'entité volume, ouvrez l'onglet
`Conditions d'environnement` et cochez **Gravitational acceleration**. Laissez
la valeur par défaut `-9.80665` en Z.

**Charge en bout.** Sélectionnez l'entité surface de l'extrémité libre, ouvrez
l'onglet `Conditions aux limites` et cochez **Force**. Saisissez la force totale
en newtons, par exemple `-1000` en Z. La valeur est le total sur l'entité ; elle
ne change donc pas quand le maillage est raffiné.

Autres possibilités, selon la nature physique de la charge :

- **Pressure** pour une pression normale uniforme sur une face, en `Pa` ;
- **Force (unit area)** pour une traction orientée en `N/m^2` ;
- **Weight** pour une masse attachée en `kg` qui n'est pas modélisée par la
  géométrie.

### Étape 6 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Le contrôle de la configuration signale les matériaux ou les conditions aux
   limites manquants avant le début du calcul.
3. Suivez la progression dans `Rapport` -> `Fichier journal du solveur`.

### Étape 7 - examiner les résultats

- Onglet `Résultats` : affichez **Displacement** pour voir la forme fléchie, et
  **Von Mises stress** pour la répartition des contraintes. Activez la mise à
  l'échelle de la déformation dans les paramètres de la vue pour rendre la
  flèche visible.
- **Normal stress** et **Shear stress** séparent les deux contributions qui
  composent la valeur de von Mises indiquée.
- **Force** sur la face encastrée est la réaction. Sa somme doit équilibrer la
  charge appliquée plus le poids propre - une vérification rapide et efficace
  que le modèle est chargé comme prévu.
- Un point de surveillance sur *Displacement* en bout donne la flèche sous forme
  de nombre plutôt que de couleur.

### Étape 8 - facultatif, contraintes thermiques

Pour ajouter une charge thermique :

1. Ajoutez une tâche **Heat transfer** *avant* la tâche de contraintes dans
   l'enchaînement des tâches du problème, et configurez ses conditions aux
   limites et ses propriétés de matériau.
2. Affectez à l'entité structurelle une condition d'environnement
   **Temperature** égale à la température de référence sans contrainte, par
   exemple `293.15` K.

Le solveur de contraintes reprend les températures aux éléments calculées par le
solveur thermique et ajoute automatiquement le terme de dilatation thermique.

### Dépannage

| Symptôme | Cause |
|---|---|
| La résolution ne converge pas | le modèle est insuffisamment bridé ; une pièce flotte ou les appuis ne suppriment pas les six modes de corps rigide |
| *Entity has material assigned which is missing required properties* | l'une des quatre propriétés requises est absente |
| Les déplacements sont nuls partout | aucune charge n'a atteint le modèle - vérifiez que la pesanteur est affectée, ou que les composantes de Force sont non nulles |
| Les déplacements sont énormes | le module d'élasticité est faux de plusieurs ordres de grandeur, ou une entité surface a une épaisseur nulle et donc aucune rigidité |
| La charge change quand le maillage est raffiné | une *Force (unit area)* ou une *Pressure* était censée être une force totale, ou l'inverse |
| La pression pousse dans le mauvais sens | les normales de la surface ne sont pas cohérentes, ou la surface ne délimite pas de volume |

---

## 4. Tutoriel - analyse modale

**Objectif.** Extraire les premiers modes propres de la même poutre en
porte-à-faux.

### Étape 1 - choisir le problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`), ajoutez une
tâche, sélectionnez **Modal analysis**.

L'analyse modale ne peut pas être combinée avec d'autres physiques, à la seule
exception d'une analyse des contraintes placée avant elle, qui en fait une
analyse modale précontrainte.

### Étape 2 - affecter le matériau

Les quatre mêmes propriétés que dans le tutoriel statique. La **masse
volumique** compte ici plus que partout ailleurs : elle constitue toute la
matrice de masse, et les fréquences varient comme `1/sqrt(rho)`.

### Étape 3 - brider le modèle

Affectez la même condition aux limites **Displacement** à l'extrémité encastrée.

Ce sont les blocages qui distinguent les modes d'une poutre en porte-à-faux de
ceux d'une barre libre. Si le modèle n'est pas bridé, les six premières valeurs
propres seront proches de zéro - les modes de corps rigide - et les modes
intéressants seront repoussés plus loin dans la liste.

### Étape 4 - configurer les paramètres modaux

Ouvrez l'onglet `Problème` et, dans *Paramètres de l'analyse modale*, réglez :

| Champ | Valeur |
|---|---|
| Méthode modale | `Multiple modes` |
| Itérations | `100` |
| Extraire les modes | `6` |
| Valeur de convergence | `1.0e-9` |

Six modes est un point de départ raisonnable, et moins que les dix par défaut.
Chaque mode supplémentaire ajoute une résolution linéaire à chaque itération ;
un grand nombre de modes sur un grand maillage est donc lent.

Choisissez `Only most dominant mode` lorsqu'un seul mode suffit ; cette méthode
utilise l'itération inverse de la puissance sur un seul vecteur et est beaucoup
moins coûteuse.

### Étape 5 - aucune charge n'est nécessaire

Une analyse modale n'a pas de vecteur des charges - le problème aux valeurs
propres est homogène. Les charges affectées au modèle sont ignorées, à une
exception près : si une analyse des contraintes a été exécutée auparavant, son
résultat de déplacement déforme le maillage pour l'assemblage modal.

### Étape 6 - résoudre

`Solution` -> `Démarrer le solveur` (`Ctrl+R`). Le journal indique les
itérations sur sous-espace et leur taux de convergence, puis un bloc par mode
extrait :

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

(`Extracting mode 6` = extraction du mode 6, `Eigen-value` = valeur propre,
`Storing results` = enregistrement des résultats.) Les modes sont traités de
l'indice extrait le plus élevé jusqu'au fondamental ; le fondamental est donc le
dernier calculé et il est stocké comme enregistrement 1.

### Étape 7 - examiner les modes

- Dock `Modèle`, onglet `Enregistrements` : un enregistrement par mode, numéroté
  à partir du fondamental. Parcourez-les pour voir chaque déformée modale.
- Onglet `Résultats` : affichez **Displacement**. Une déformée modale n'est
  définie qu'à un facteur d'échelle près ; les amplitudes sont donc arbitraires -
  c'est la forme qui compte. Activez la mise à l'échelle de la déformation et
  animez l'enregistrement pour visualiser le mode.
- La valeur propre de chaque mode est affichée dans le journal du solveur et
  comme étiquette de l'enregistrement dans la vue 3D.

### Étape 8 - lire les fréquences propres

La vue 3D étiquette chaque enregistrement modal avec son numéro de mode et sa
fréquence propre en Hz. Le journal du solveur liste la même fréquence pour
chaque mode extrait, avec la valeur propre sous-jacente :

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

(`frequency` = fréquence.) Un mode que l'itération aux valeurs propres n'a pas
pu résoudre est indiqué à `0` Hz et accompagné d'un avertissement dans le
journal.

Comme pour tout résultat, il vaut la peine de vérifier la fréquence
fondamentale par un calcul manuel la première fois qu'un modèle est configuré -
la section 5 donne la formule pour une poutre en porte-à-faux.

### Étape 9 - facultatif, modes précontraints

1. Dans l'enchaînement des tâches du problème, placez une tâche **Stress
   analysis** *avant* la tâche **Modal analysis**.
2. Configurez les charges statiques et les blocages comme dans le tutoriel 3.

L'analyse modale s'exécute alors sur le maillage déformé. C'est la manière de
voir comment une charge statique modifie la forme selon laquelle une structure
vibre.

### Dépannage

| Symptôme | Cause |
|---|---|
| Les premières valeurs propres sont proches de zéro | le modèle est insuffisamment bridé, ou une pièce est déconnectée - un véritable mécanisme |
| Le solveur aux valeurs propres ne converge pas | trop peu d'itérations, ou une matrice de masse mal conditionnée parce qu'une entité n'a pas de masse volumique ou pas de mesure géométrique |
| L'extraction des modes est très lente | trop de modes demandés ; chaque mode coûte une résolution linéaire complète |
| Le calcul prend des heures, ou semble ne jamais finir | trop de modes demandés - chacun coûte une résolution linéaire à chaque itération, et le calcul ne peut pas s'arrêter plus tôt tant qu'ils ne se sont pas tous stabilisés. Suivez le compteur de vecteurs du sous-espace dans le journal pour voir la progression au sein d'une itération |
| Une fréquence semble absurde | l'itération sur sous-espace n'a pas convergé - le journal l'indique à la fin d'un calcul non convergé ; extrayez moins de modes, augmentez le nombre d'itérations, ou vérifiez la masse volumique et les blocages |
| Les déformées modales semblent identiques | modes rapprochés d'une structure symétrique, ou trop peu d'itérations sur sous-espace pour les séparer |
| Moins d'enregistrements que de modes demandés | le sous-espace a perdu des directions indépendantes ; le journal indique combien de modes ont été extraits et ne stocke que ceux-là |

---

## 5. Vérifier un modèle

Le solveur n'a pas de suite de vérification intégrée ; il vaut donc la peine de
valider un nouveau modèle par rapport à quelque chose que l'on peut calculer à
la main avant de s'y fier. Trois vérifications peu coûteuses, par ordre
d'utilité :

**Équilibre des réactions.** Faites la somme du résultat *Force* sur les nœuds
bloqués. Elle doit équilibrer les charges appliquées plus le poids propre
`rho * V * g`. Cela détecte des intensités de charge erronées, des unités
erronées et des directions de pression erronées.

**Flèche en bout de porte-à-faux.** Pour une poutre de longueur `L`, de moment
quadratique `I` et de charge en bout `F`, la théorie d'Euler-Bernoulli donne

```
delta = F * L^3 / (3 * E * I)
```

Modélisez la poutre avec des éléments volumiques et comparez. Attendez-vous à ce
que le résultat éléments finis soit légèrement plus rigide sur un maillage
grossier, et converge par valeurs inférieures à mesure que le maillage est
raffiné. N'utilisez pas d'éléments lignes pour cette vérification - ils n'ont
pas de rigidité en flexion.

**Fréquence fondamentale d'un porte-à-faux.** Pour la même poutre de section
`A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Comparez avec `sqrt(lambda_0) / (2*pi)` issu de l'analyse modale. Cela valide à
la fois la matrice de masse, la matrice de rigidité et votre conversion des
valeurs propres.

**Barre chargée brusquement.** Une barre encastrée à une extrémité et chargée à
l'autre par une force axiale `F` appliquée à `t = 0` constitue une vérification
d'une analyse dynamique. L'onde de contrainte se propage jusqu'à l'extrémité
encastrée et revient à la vitesse `c = sqrt(E/rho)` ; l'extrémité libre se
déplace donc linéairement jusqu'au double de la flèche statique,
`2 * F * L / (E * A)`, à `t = 2*L/c`, et revient à zéro à `t = 4*L/c`. Sans
amortissement, le cycle se répète. Avec un amortissement de Rayleigh, il
s'éteint et la barre s'immobilise à la flèche statique. Utilisez un coefficient
de Poisson nul pour que la barre soit réellement unidimensionnelle.

Pour chacune de ces vérifications, raffinez le maillage au moins une fois et
vérifiez que la réponse converge au lieu de dériver.

---

## 6. Limitations

### Limitations de modélisation

- **Élasticité linéaire uniquement.** Petites déformations, petits
  déplacements, matériau isotrope, pas de plasticité, pas de contact, pas de
  grandes déformations et pas de flambement. Les résultats n'ont de sens que
  tant que la structure reste dans son domaine linéaire.
- **L'amortissement est uniquement de type Rayleigh.** La matrice
  d'amortissement vaut `alpha_M*M + beta_K*K` par matériau ; le taux
  d'amortissement suit donc la courbe de 1.7 et ne peut pas être fixé mode par
  mode. Il n'y a ni amortisseurs discrets, ni amortissement indépendant de la
  fréquence (hystérétique), ni condition aux limites d'amortissement.
- **Intégration temporelle implicite uniquement.** Chaque pas est une
  résolution linéaire. Il n'y a pas de schéma explicite aux différences
  centrées ; les événements très brefs comme un impact balistique nécessitent
  donc de nombreux pas coûteux.
- **Les éléments surfaces sont en déformations planes.** Une plaque mince
  chargée dans son propre plan sera plus rigide qu'une idéalisation en
  contraintes planes. Modélisez les structures minces avec des éléments
  volumiques lorsque la différence compte.
- **Les éléments lignes sont des barres.** Seule la rigidité axiale `E*A` est
  assemblée. Il n'y a ni flexion, ni cisaillement, ni torsion, quel que soit le
  nom du type d'élément ; un portique modélisé avec des éléments lignes se
  comportera donc comme un treillis articulé.
- **La précontrainte est uniquement géométrique.** Une analyse modale
  précontrainte réévalue la rigidité linéaire sur le maillage déplacé. Elle
  n'ajoute pas de terme de rigidité géométrique ; la rigidification d'un câble
  tendu ou l'assouplissement d'une barre comprimée ne sont donc pas pris en
  compte.
- **Pas de facteurs de participation modale ni de masses effectives.** Le
  solveur ne donne que les valeurs propres et les déformées modales ; il n'y a
  donc pas d'indication directe des modes qui comptent pour une direction
  d'excitation donnée.
