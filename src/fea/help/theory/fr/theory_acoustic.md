# Analyse acoustique - Manuel théorique et guide de l'utilisateur

Ce document décrit le solveur acoustique (`RSolverAcoustic`) de Range FEA : les
équations qu'il résout, la signification de chaque donnée d'entrée qu'il
accepte, les parties de l'interface graphique qui le pilotent, et deux
tutoriels complets.

**Sommaire**

1. [Fondements théoriques](#1-fondements-théoriques)
2. [Interface graphique](#2-interface-graphique)
3. [Tutoriel - analyse acoustique transitoire](#3-tutoriel---analyse-acoustique-transitoire)
4. [Tutoriel - analyse acoustique harmonique](#4-tutoriel---analyse-acoustique-harmonique)
5. [Vérification](#5-vérification)
6. [Limitations](#6-limitations)

---

## 1. Fondements théoriques

### 1.1 Inconnue principale - le potentiel des vitesses

Le solveur acoustique ne résout pas directement la pression. Son inconnue
principale est le **potentiel des vitesses** `phi`, un champ scalaire en
`m^2/s`. Les deux grandeurs physiques intéressantes s'en déduisent :

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`particle velocity` = vitesse particulaire, `acoustic pressure` = pression
acoustique.)

Utiliser un potentiel plutôt que la pression présente deux avantages pratiques.
Le problème reste scalaire alors que la vitesse particulaire est un vecteur, et
une vitesse de paroi imposée - la manière dont la plupart des sources
acoustiques sont décrites - intervient comme une condition aux limites naturelle
et non comme une contrainte.

La convention de signe ci-dessus (`u = -grad(phi)`, `p = +rho * dphi/dt`) est
utilisée de façon cohérente dans tout le solveur et dans les résultats stockés.

### 1.2 Équation fondamentale

Le son de faible amplitude dans un fluide homogène au repos obéit à l'équation
des ondes amortie pour le potentiel des vitesses :

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Symbole | Signification | Unités | Origine |
|---|---|---|---|
| `phi` | potentiel des vitesses | `m^2/s` | inconnue résolue |
| `c` | vitesse du son | `m/s` | propriété du matériau |
| `rho` | masse volumique du fluide | `kg/m^3` | propriété du matériau |
| `beta` | facteur d'amortissement volumique | `1/s` | propriété du matériau, `0` par défaut |

L'équation est linéaire. Par conséquent, une seule résolution par pas de temps,
ou par fréquence, donne la réponse finale - aucune itération non linéaire
externe n'est effectuée et `hasConverged()` indique toujours un succès une fois
le système linéaire résolu.

**Vitesse du son.** `c` peut être donnée directement par la propriété de
matériau *Speed of sound*. Si elle est absente ou nulle, elle est déduite de la
rigidité volumique et de la masse volumique :

```
c = sqrt(E / rho)
```

où `E` est la propriété de matériau *Modulus of elasticity*. Pour un gaz, `E`
est le module de compressibilité adiabatique (`gamma * p0`, environ
`1.42e5 Pa` pour l'air à la pression atmosphérique), qui avec
`rho = 1.2 kg/m^3` donne la valeur bien connue `c = 344 m/s`. Donner `c`
directement est généralement moins source d'erreurs.

Les éléments sans matériau propre - une surface frontière qui délimite un volume
maillé, par exemple - héritent des valeurs moyennes de `c` et `rho` du domaine
calculable, de sorte que les termes d'impédance et de rayonnement y sont
évalués correctement.

### 1.3 Discrétisation par éléments finis

En multipliant par une fonction de pondération `w`, en intégrant sur le domaine,
en appliquant le théorème de la divergence et en multipliant le tout par `c^2`,
on obtient la forme faible

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

qui, après substitution des fonctions de forme des éléments `N`, devient le
système du second ordre

```
M * phi'' + C * phi' + K * phi = f
```

avec

| Matrice | Définition | Commentaire |
|---|---|---|
| `M` | `INT( N_m * N_n )` | matrice de masse |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | matrice de rigidité |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | amortissement volumique plus impédance de frontière |
| `f` | `c^2 * SURF( v_n * N_m )` | source de vitesse normale imposée |

`M` et `K` sont semi-définies positives ; `C` est semi-définie positive tant que
le rapport d'impédance `zeta` est positif.

**Mesures des éléments.** Les termes de domaine (`M`, `K`, la partie volumique
de `C`) sont pondérés par la mesure géométrique de l'entité à laquelle ils
appartiennent :

| Entité | Mesure | Où elle est définie |
|---|---|---|
| Volume | 1 | - |
| Surface | épaisseur de la surface | propriétés géométriques de l'entité |
| Ligne | section de la ligne | propriétés géométriques de l'entité |
| Point | volume du point | propriétés géométriques de l'entité |

Les termes de frontière (la partie impédance de `C` et la source `f`) agissent
sur la mesure simple de l'élément - l'aire d'un élément surface, la longueur
d'un élément ligne, une aire unité pour un élément point. Une surface qui
délimite un volume maillé n'apporte donc que ses termes de frontière, car son
épaisseur est nulle par défaut.

Les éléments points n'ont pas de points d'intégration. Leurs contributions sont
concentrées directement sur leur nœud unique, ce qui permet aux conditions aux
limites ponctuelles de fonctionner aux extrémités des conduits
unidimensionnels.

### 1.4 Conditions aux limites

| Condition aux limites | Type | S'applique à | Composante |
|---|---|---|---|
| Velocity potential | explicite (Dirichlet) | point, ligne, surface | Potential `[m^2/s]` |
| Velocity | naturelle (source) | point, ligne, surface | Velocity `[m/s]` |
| Absorbing boundary | naturelle (amortissement) | point, ligne, surface | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | naturelle (amortissement) | point, ligne, surface | Acoustic impedance `[Pa*s/m]` |

**Velocity** (vitesse). La source acoustique classique : une surface qui vibre
avec une vitesse normale imposée, comme une membrane de haut-parleur ou un
piston. Une valeur positive entraîne le fluide, c'est-à-dire qu'elle est
dirigée *vers l'intérieur* du domaine acoustique. Elle intervient au second
membre sous la forme `f = c^2 * SURF(v_n * N)`.

Un nœud sans aucune condition aux limites est une **paroi rigide** idéale - la
condition naturelle de la forme faible est une vitesse normale nulle. Il n'y a
rien à affecter pour obtenir une cavité à parois rigides.

**Acoustic impedance** (impédance acoustique). Impose le rapport entre la
pression et la vitesse particulaire normale sortante sur la frontière,
`p = Z * u_n`. En interne, elle est exprimée par le **rapport d'impédance
spécifique** sans dimension

```
zeta = Z / (rho * c)
```

et apporte la matrice d'amortissement de frontière
`(c/zeta) * SURF(N_m * N_n)`. `Z = rho*c` (`zeta = 1`) est l'impédance
caractéristique du fluide et donne une terminaison parfaitement non
réfléchissante. Pour l'air dans les conditions ambiantes, `rho*c` est proche de
`415 Pa*s/m`, la valeur proposée par défaut par la condition aux limites.

**Absorbing boundary** (frontière absorbante). Le même terme, mais paramétré
par le **coefficient d'absorption** en incidence normale `alpha` dans
`[0, 1]`, qui est la valeur indiquée par les fiches techniques des matériaux. Il
est converti en rapport d'impédance par le coefficient de réflexion d'onde plane
`R = (zeta - 1) / (zeta + 1)` et `alpha = 1 - |R|^2` :

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Comportement |
|---|---|---|
| `1.0` | `1` | entièrement absorbante, terminaison anéchoïque, aucune réflexion |
| `0.5` | `5.83` | partiellement absorbante |
| `0.1` | `38.0` | faiblement absorbante |
| `0.0` | infini | paroi rigide, aucun amortissement |

Utilisez `alpha = 1` sur toute coupe artificielle d'un domaine non borné pour
émuler un rayonnement libre, et un `alpha` mesuré sur une paroi revêtue. Si une
condition *Acoustic impedance* et une condition *Absorbing boundary* sont
affectées à la même entité, l'impédance l'emporte.

### 1.5 Conditions initiales

| Condition initiale | Définit | Commentaire |
|---|---|---|
| Velocity potential | `phi` à `t = 0` | rarement non nul à lui seul |
| Velocity potential / s | `dphi/dt` à `t = 0` | c'est ce qui définit la **pression** initiale, `p = rho * dphi/dt` |

Les deux ne sont appliquées qu'au premier pas de temps. Pour démarrer un calcul
transitoire à partir d'une région sous pression, affectez à cette entité
*Velocity potential / s* `= p0 / rho`. Les conditions initiales n'ont aucun
effet sur une analyse harmonique.

### 1.6 Analyse transitoire

Le système du second ordre est intégré avec le schéma de **Newmark**, avec
`gamma = 1/2` et

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

(`time-march approximation coefficient` = coefficient d'approximation du schéma
temporel.) La borne garantit `beta_N >= 1/4`, c'est-à-dire au moins la variante
de l'accélération moyenne, inconditionnellement stable, et évite une division
par zéro pour l'approximation avant, dont le coefficient vaut `0`.

Chaque pas résout

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

avec les coefficients habituels

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

suivis de la mise à jour de l'état

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

La matrice du système est symétrique définie positive ; elle est donc résolue
par la méthode du **gradient conjugué** avec un préconditionnement de Jacobi.

`phi'` et `phi''` sont écrits dans le fichier de résultats sous les noms
*Potential / s* et *Potential / s^2*. Ils constituent l'état de l'intégrateur
temporel ; ils doivent donc être présents pour qu'un redémarrage ou le pas de
temps suivant se poursuive correctement.

**Une analyse transitoire nécessite un solveur temporel activé.** Sans lui, les
termes de masse et d'amortissement disparaissent et le problème dégénère en un
problème de Laplace dépourvu de sens ; le solveur le signale comme une erreur
au lieu de produire un résultat absurde.

**Choix du pas de temps.** Le schéma est inconditionnellement stable ; `dt` est
donc dicté par la précision et non par la stabilité. Une règle utile consiste à
résoudre la plus haute fréquence d'intérêt `f_max` avec au moins 20 pas par
période :

```
dt <= 1 / (20 * f_max)
```

**Choix du maillage.** Résolvez la plus courte longueur d'onde
`lambda_min = c / f_max` avec au moins 10 éléments linéaires :

```
h <= c / (10 * f_max)
```

### 1.7 Analyse harmonique

Pour une réponse stationnaire à une pulsation unique `omega = 2*pi*f`, on écrit
`phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Alors `dphi/dt -> i*omega*PHI` et
`d2phi/dt2 -> -omega^2*PHI`, et le système devient le problème complexe de
**Helmholtz**

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

Les solveurs matriciels de Range FEA sont réels ; le système complexe est donc
résolu comme un système réel par blocs équivalent, de taille double. Avec
`S = K - omega^2*M` et `D = omega*C` :

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Cette matrice par blocs est non symétrique et, au-delà de la limite statique,
indéfinie ; elle est donc résolue avec **GMRES** et un préconditionnement de
Jacobi. GMRES avec redémarrage stagne sur les systèmes indéfinis dès que
l'espace de Krylov est beaucoup plus petit que le système ; le solveur élargit
donc automatiquement le redémarrage de GMRES jusqu'à la taille du système, dans
la limite d'un budget fixe de 256 Mo pour les deux bases de Krylov. Le
redémarrage configuré n'est jamais qu'augmenté, jamais réduit, et la
modification est signalée dans le journal du solveur :

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

(« Augmentation du redémarrage de GMRES de 10 à 403 pour le système dans le
domaine fréquentiel. ») Une analyse harmonique parcourt une liste de fréquences

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(`first frequency` = première fréquence, `frequency step` = pas de fréquence,
`number of frequencies` = nombre de fréquences.) Chaque fréquence est une
résolution indépendante et produit son propre enregistrement de résultats, de
la même manière qu'une analyse transitoire produit un enregistrement par pas de
temps. Le solveur temporel ne joue aucun rôle ; s'il se trouve activé, il est
ignoré avec un avertissement, et ses paramètres restent inchangés afin qu'un
retour à une analyse transitoire ne les perde pas.

### 1.8 Résultats dérivés

| Résultat | S'applique à | Transitoire | Harmonique |
|---|---|---|---|
| Potential `[m^2/s]` | nœud | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | nœud | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | nœud | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | nœud | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | nœud | `rho * dphi/dt`, instantanée | `omega * rho * abs(PHI)`, amplitude |
| Acoustic phase `[Deg]` | nœud | - | `arg(P)` |
| Sound pressure level `[dB]` | nœud | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | élément | `-grad(phi)` | amplitude de `-grad(PHI)`, par composante |
| Acoustic intensity `[W/m^2]` | élément | `p * u`, instantanée | `0.5 * Re{P * conj(U)}`, moyennée dans le temps |

Le **niveau de pression acoustique** (*Sound pressure level*) est calculé par
rapport à la **Pression de référence** des paramètres acoustiques, `2e-5 Pa` par
défaut - le seuil d'audition dans l'air. Dans une analyse harmonique, on utilise
la valeur efficace du signal harmonique, c'est-à-dire l'amplitude divisée par
`sqrt(2)` ; dans une analyse transitoire, on utilise la valeur instantanée, si
bien que le niveau indiqué oscille au cours d'une période.

L'**intensité acoustique** dans une analyse harmonique est l'intensité active
moyennée dans le temps, c'est-à-dire le flux net de puissance acoustique. Elle
est identiquement nulle dans une onde stationnaire pure et vaut
`0.5 * |p| * |u|` dans une onde plane progressive.

### 1.9 Transitoire ou harmonique ?

| | Transitoire | Harmonique |
|---|---|---|
| Résout | `M*phi'' + C*phi' + K*phi = f` dans le temps | `(K - w^2*M + i*w*C)*PHI = F` par fréquence |
| Répond à | « que se passe-t-il quand ... » | « quelle est la réponse stationnaire à ... » |
| Excitation | tout historique temporel, y compris une impulsion unique | une sinusoïde à la fois |
| Coût | une résolution SPD par pas de temps | une résolution indéfinie de taille double par fréquence |
| Enregistrements | un par pas de temps | un par fréquence |
| Information de phase | implicite dans l'historique temporel | explicite, comme champ de résultat |

**Utilisez une analyse transitoire pour**

- les études de réponse impulsionnelle, d'écho et de réverbération ;
- les événements transitoires - une détonation, une fermeture de vanne, un
  claquement de porte ;
- une excitation large bande, où un seul calcul suivi d'une transformée de
  Fourier d'un point de surveillance couvre tout le spectre d'un coup ;
- tout cas où l'historique temporel lui-même est la réponse.

**Utilisez une analyse harmonique pour**

- les fonctions de réponse en fréquence et les courbes de perte par
  transmission ;
- les résonances de cavités et de conduits, et la répartition de la pression à
  une résonance ;
- les performances des silencieux et des absorbeurs sur une bande de
  fréquences ;
- les cartes de niveau de pression acoustique pour un son pur ;
- tout cas où seul le régime établi compte. Un calcul harmonique est
  généralement bien moins coûteux qu'une marche transitoire assez longue pour
  que les transitoires s'éteignent.

Un compromis pratique consiste à localiser les résonances par un balayage
harmonique grossier, puis à n'affiner que la bande intéressante.

---

## 2. Interface graphique

Tout ce qui concerne l'acoustique se trouve dans le dock **Solveur** (onglets
`Problème`, `Conditions aux limites`, `Conditions initiales`, `Matériau`,
`Résultats`), le menu **Problème** et le dock **Modèle**.

### 2.1 Choix du problème acoustique

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`) ouvre la
fenêtre d'enchaînement des tâches du problème. Ajoutez une tâche et choisissez
**Acoustics** - « Customized wave equation for solving acoustic problems »
(équation des ondes adaptée à la résolution des problèmes acoustiques).

Acoustics est mutuellement exclusif avec **Wave - general** et avec **Modal
analysis**. Il peut être combiné avec d'autres physiques ; l'enchaînement des
tâches détermine alors l'ordre d'exécution.

### 2.2 Onglet Problème

Une fois l'acoustique sélectionnée, l'onglet `Problème` du dock Solveur affiche
deux groupes.

**Solveur temporel** - affiché car l'acoustique est un type de problème
dépendant du temps.

| Champ | Signification |
|---|---|
| Activer | active ou désactive la marche en temps transitoire |
| Approximation | approximation du schéma temporel ; correspond au `beta_N` de Newmark comme décrit en 1.6 |
| Temps de début | première valeur de temps en secondes ; ignorée lors d'un redémarrage du solveur |
| Temps de fin | `start time + time-step size * number of time-steps` (temps de début + taille du pas de temps × nombre de pas de temps), lecture seule |
| Taille du pas de temps | `dt` en secondes |
| Nombre de pas de temps | nombre de pas à calculer |
| Fréquence de sortie | écrit un fichier de résultats tous les N pas ; `0` n'écrit que le dernier pas |

**Paramètres de l'analyse acoustique** - le groupe ajouté pour l'acoustique.

| Champ | Signification |
|---|---|
| Type d'analyse | *Transient* (transitoire) ou *Harmonic* (harmonique) |
| Première fréquence `[Hz]` | première fréquence du balayage harmonique |
| Pas de fréquence `[Hz]` | incrément entre deux fréquences consécutives |
| Nombre de fréquences | nombre de fréquences à résoudre ; chacune produit son propre enregistrement |
| Balayage | étiquette en lecture seule indiquant la plage de fréquences obtenue |
| Pression de référence `[Pa]` | référence utilisée pour le niveau de pression acoustique, `2e-5 Pa` par défaut |

Les trois champs de fréquence sont grisés tant que *Transient* est sélectionné.
La pression de référence s'applique aux deux types d'analyse.

### 2.3 Onglet Matériau

L'onglet `Matériau` liste le matériau affecté à l'entité sélectionnée et les
propriétés qu'il contient. Pour l'acoustique, les propriétés pertinentes sont :

| Propriété | Unités | Requise | Commentaire |
|---|---|---|---|
| Density | `kg/m^3` | oui | `rho` |
| Speed of sound | `m/s` | l'une des deux | `c`, utilisée directement si non nulle |
| Modulus of elasticity | `Pa` | l'une des deux | solution de repli, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | non | `beta`, atténuation volumique, `0` par défaut |

Une entité est résolue par le solveur acoustique lorsqu'elle possède une masse
volumique **et** soit une vitesse du son, soit un module d'élasticité. Comme ces
deux propriétés sont des alternatives, aucune n'est signalée comme manquante par
le contrôle de la configuration ; un modèle qui n'a ni l'une ni l'autre est
rejeté par le solveur avec un message explicite.

Chaque propriété est une table en fonction de la température ; une vitesse du
son dépendant de la température peut donc être saisie et sera interpolée si un
problème de transfert de chaleur est résolu en parallèle.

### 2.4 Onglet Conditions aux limites

L'onglet est divisé en une liste de gestion des conditions aux limites
disponibles en haut et un éditeur de la condition sélectionnée en bas.
Sélectionnez d'abord une entité dans l'arbre `Modèle` ; le gestionnaire propose
alors les conditions aux limites valables pour ce type d'entité et pour le
problème sélectionné. Cochez-en une pour l'affecter, puis modifiez ses
composantes dans l'arbre inférieur.

Pour l'acoustique, le gestionnaire propose :

- **Velocity potential** - explicite, points, lignes et surfaces. Composante :
  *Potential* `[m^2/s]`.
- **Velocity** - points, lignes et surfaces. Composante : *Velocity* `[m/s]`.
- **Absorbing boundary** - points, lignes et surfaces. Composante : *Acoustic
  absorption coefficient*, sans dimension, `1.0` par défaut.
- **Acoustic impedance** - points, lignes et surfaces. Composante : *Acoustic
  impedance* `[Pa*s/m]`, `415` par défaut.

Chaque composante est une table en fonction du temps ; on peut donc donner un
historique temporel à une source : ajoutez des lignes pour augmenter
progressivement une vitesse ou pour émettre une brève impulsion. Dans une
analyse harmonique, on utilise la valeur au temps courant du solveur, ce qui,
pour une table à une seule valeur, est simplement cette valeur.

Laissez une paroi sans aucune condition aux limites pour la rendre
acoustiquement rigide.

### 2.5 Onglet Conditions initiales

Même disposition que l'onglet des conditions aux limites. Pour l'acoustique, il
propose **Velocity potential** et **Velocity potential / s**. C'est la seconde
qui définit le champ de pression initial.

### 2.6 Configuration du solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes` configure les
solveurs itératifs. Une analyse acoustique transitoire utilise l'entrée **CG**,
une analyse harmonique l'entrée **GMRES**.

Le redémarrage par défaut de GMRES, `10`, est bien trop petit pour le système
par blocs dans le domaine fréquentiel. Le solveur l'augmente automatiquement,
mais le nombre d'itérations *externes* et la valeur de convergence sont repris
tels que configurés ; augmentez-les donc ici si un calcul harmonique ne converge
pas.

### 2.7 Points de surveillance

`Problème` -> `Définir les points de surveillance` place des sondes aux
coordonnées données et choisit la variable à enregistrer. Un point de
surveillance sur *Acoustic pressure* est le microphone naturel : dans un calcul
transitoire, il produit l'historique de la pression à cet endroit, que l'on peut
consulter par `Rapport` -> `Points de surveillance`.

### 2.8 Résultats

Après le calcul, l'onglet `Résultats` liste les variables calculées et contrôle
ce que la vue 3D affiche. Les variables acoustiques sont celles de la
section 1.8.

Les variables aux nœuds (pression, niveau de pression acoustique, phase,
potentiel) sont tracées comme des champs continus ; les variables aux éléments
(vitesse particulaire, intensité) sont des vecteurs et se montrent au mieux par
un champ vectoriel ou une coupe.

L'onglet `Enregistrements` du dock `Modèle` liste les différents enregistrements
de résultats - un par pas de temps pour un calcul transitoire, un par fréquence
pour un balayage harmonique - et permet de les parcourir ou de les animer.

`Rapport` -> `Fichier journal du solveur` affiche la sortie complète du solveur,
y compris le bloc de statistiques imprimé pour chaque variable acoustique après
chaque enregistrement.

---

## 3. Tutoriel - analyse acoustique transitoire

**Objectif.** Calculer la réponse impulsionnelle d'une cavité d'air fermée et
lire l'historique de la pression à la position d'un microphone.

Ce tutoriel suppose un modèle volumique maillé. Utilisez `Fichier` ->
`Nouveau modèle`, dessinez un parallélépipède avec `Géométrie` ->
`Créer un élément` ou importez une géométrie, et maillez-la en tétraèdres - le
tutoriel *Dessiner un cube* couvre ce sujet.

### Étape 1 - choisir le problème

1. `Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`).
2. Ajoutez une tâche et sélectionnez **Acoustics**.
3. Confirmez avec `OK`.

L'onglet `Problème` du dock Solveur affiche maintenant les groupes *Solveur
temporel* et *Paramètres de l'analyse acoustique*.

### Étape 2 - affecter le matériau

1. Sélectionnez l'entité volume dans l'arbre `Modèle`.
2. Ouvrez l'onglet `Matériau`.
3. Affectez un matériau, ou créez-en un, et donnez-lui :
   - **Density** (masse volumique) = `1.2` `kg/m^3`
   - **Speed of sound** (vitesse du son) = `344` `m/s`
4. Laissez **Acoustic damping factor** à `0` pour une cavité non amortie, ou
   donnez-lui une petite valeur comme `1.0` `1/s` pour que la réponse décroisse.

### Étape 3 - choisir le type d'analyse et le pas de temps

1. Ouvrez l'onglet `Problème`.
2. Dans *Paramètres de l'analyse acoustique*, réglez **Type d'analyse** =
   `Transient`.
3. Dans *Solveur temporel* :
   - cochez **Activer** ;
   - **Temps de début** = `0` ;
   - **Taille du pas de temps** = `2e-5` s. D'après `dt <= 1/(20*f_max)`, cela
     résout les fréquences jusqu'à environ `2.5 kHz` ;
   - **Nombre de pas de temps** = `500`, soit `10 ms` de temps simulé ;
   - **Fréquence de sortie** = `5`, ce qui écrit 100 enregistrements de
     résultats.

Vérifiez le maillage avec `h <= c/(10*f_max)` : pour `f_max = 2.5 kHz` et
`c = 344 m/s`, la taille des éléments ne doit pas dépasser environ `14 mm`.

### Étape 4 - exciter la cavité

Deux possibilités, choisissez-en une.

**A - une paroi mobile.** Sélectionnez la surface qui représente la source,
ouvrez l'onglet `Conditions aux limites` et cochez **Velocity**. Saisissez un
historique temporel dans la table de la composante, par exemple une brève
impulsion :

| Temps `[s]` | Vitesse `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - une région initialement sous pression.** Sélectionnez l'entité qui doit
démarrer sous pression, ouvrez l'onglet `Conditions initiales` et cochez
**Velocity potential / s**. Saisissez `p0 / rho` ; pour `p0 = 1 Pa` et
`rho = 1.2 kg/m^3`, cela donne `0.833`.

Laissez toutes les autres surfaces sans condition aux limites pour que la
cavité ait des parois rigides, ou cochez **Absorbing boundary** avec un
coefficient d'absorption correspondant au revêtement des parois pour laisser
l'énergie s'échapper.

### Étape 5 - ajouter un microphone

1. `Problème` -> `Définir les points de surveillance`.
2. Ajoutez un point à l'endroit qui vous intéresse.
3. Sélectionnez **Acoustic pressure** comme variable.

### Étape 6 - résoudre

1. `Solution` -> `Démarrer le solveur` (`Ctrl+R`).
2. Le contrôle de la configuration signale les matériaux ou conditions aux
   limites manquants avant le début du calcul.
3. Suivez la progression dans `Rapport` -> `Fichier journal du solveur`. Chaque
   pas de temps affiche la convergence du CG et les statistiques de chaque
   variable acoustique.

### Étape 7 - examiner les résultats

- Dock `Modèle`, onglet `Enregistrements` : parcourez ou animez les
  enregistrements temporels.
- Onglet `Résultats` : affichez **Acoustic pressure** ou **Sound pressure
  level** sur le modèle, et **Acoustic particle velocity** comme champ
  vectoriel.
- `Rapport` -> `Points de surveillance` : l'historique de la pression au
  microphone. Son taux de décroissance donne le temps de réverbération ; sa
  transformée de Fourier donne les résonances de la cavité.

### Dépannage

| Symptôme | Cause |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* (l'analyse acoustique transitoire nécessite un solveur temporel activé) | *Activer* est décoché dans *Solveur temporel*, ou le type d'analyse devrait être *Harmonic* |
| *"Acoustic solver requires a positive time step size"* (le solveur acoustique nécessite une taille de pas de temps positive) | la taille du pas de temps est nulle |
| *"No element with a valid speed of sound was found"* (aucun élément avec une vitesse du son valide n'a été trouvé) | aucune entité n'a une masse volumique avec une vitesse du son ou un module d'élasticité |
| La réponse ne décroît jamais | aucun amortissement nulle part - normal pour une cavité parfaitement rigide et sans pertes ; ajoutez une frontière absorbante ou un facteur d'amortissement acoustique |
| Oscillations en avant du front d'onde | le maillage ou le pas de temps est trop grossier pour le contenu fréquentiel ; affinez les deux |

---

## 4. Tutoriel - analyse acoustique harmonique

**Objectif.** Calculer la réponse en fréquence d'un conduit à terminaison
anéchoïque, et une carte du niveau de pression acoustique à chaque fréquence.

### Étape 1 - choisir le problème

`Problème` -> `Enchaînement des tâches des problèmes` (`Ctrl+P`), ajoutez une
tâche, sélectionnez **Acoustics**.

### Étape 2 - affecter le matériau

Comme dans le tutoriel transitoire : **Density** `1.2 kg/m^3` et **Speed of
sound** `344 m/s` sur l'entité du fluide.

Dimensionnez le maillage pour la fréquence la **plus haute** du balayage :
`h <= c / (10 * f_max)`.

### Étape 3 - configurer le balayage

Ouvrez l'onglet `Problème` et, dans *Paramètres de l'analyse acoustique*,
réglez :

| Champ | Valeur |
|---|---|
| Type d'analyse | `Harmonic` |
| Première fréquence | `50` Hz |
| Pas de fréquence | `50` Hz |
| Nombre de fréquences | `20` |
| Pression de référence | `2e-5` Pa |

L'étiquette *Balayage* confirme `50 - 1000 [Hz]`, soit 20 résolutions et 20
enregistrements de résultats.

Le groupe *Solveur temporel* peut rester activé - une analyse harmonique l'ignore
et écrit un avertissement dans le journal. Ses paramètres sont conservés.

### Étape 4 - exciter le conduit

Sélectionnez la surface d'entrée, ouvrez l'onglet `Conditions aux limites` et
cochez **Velocity**. Saisissez une seule ligne avec la vitesse du piston, par
exemple `0.01` `m/s`. L'amplitude est interprétée comme l'amplitude de
l'excitation harmonique à chaque fréquence du balayage.

### Étape 5 - terminer le conduit

Sélectionnez la surface de sortie et cochez au choix :

- **Absorbing boundary** avec **Acoustic absorption coefficient** = `1.0` pour
  une terminaison parfaitement non réfléchissante ; ou
- **Acoustic impedance** avec **Acoustic impedance** = `rho * c` = `413`
  `Pa*s/m`, ce qui revient au même exprimé autrement.

Utilisez un coefficient d'absorption plus petit pour modéliser une terminaison
partiellement réfléchissante, et laissez la sortie sans aucune condition pour
modéliser une extrémité fermée et rigide.

Les parois du conduit n'ont besoin d'aucune condition aux limites - elles sont
rigides par défaut.

### Étape 6 - vérifier le solveur matriciel

`Problème` -> `Configurer le solveur matriciel des problèmes`, entrée **GMRES**.
Le redémarrage est augmenté automatiquement, mais si le journal montre un résidu
qui stagne, augmentez le nombre d'itérations externes et relâchez ou resserrez
la valeur de convergence selon les besoins.

### Étape 7 - résoudre

`Solution` -> `Démarrer le solveur` (`Ctrl+R`). Le journal affiche un bloc par
fréquence :

```
Frequency:         1 of 20        | f =  5.000000e+01 [Hz]
{
    Increasing GMRES restart from 10 to ... for the frequency domain system.
    ...
    Acoustic pressure [Pa]
    Sound pressure level [dB]
    ...
}
```

(`Frequency: 1 of 20` = fréquence 1 sur 20.)

### Étape 8 - examiner les résultats

- Dock `Modèle`, onglet `Enregistrements` : un enregistrement par fréquence. Les
  parcourir revient à suivre le balayage en fréquence.
- Onglet `Résultats` : affichez **Sound pressure level** pour une carte en
  décibels, **Acoustic pressure** pour l'amplitude, et **Acoustic phase** pour
  voir les fronts d'onde.
- **Acoustic intensity** montre où circule la puissance acoustique. Dans un
  conduit bien terminé, elle est uniforme et dirigée vers la sortie ; dans une
  cavité résonante, elle est presque nulle.
- Un point de surveillance sur **Acoustic pressure** enregistre l'amplitude à
  chaque fréquence, c'est-à-dire la fonction de réponse en fréquence à cet
  endroit.

### Interprétation du résultat

Pour un conduit droit excité à `v` et à terminaison anéchoïque, la solution
analytique d'onde plane est

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`constant along the duct` = constante le long du conduit, `with` = avec.) Pour
`rho = 1.2`, `c = 344`, `v = 0.01`, cela donne `|p| = 4.13 Pa`, un niveau de
pression acoustique de `103.3 dB` et une intensité de `0.0206 W/m^2`. Des
écarts par rapport à un `|p|` constant le long du conduit signifient que la
terminaison réfléchit - soit volontairement, soit parce que le coefficient
d'absorption n'est pas égal à `1`.

Avec une terminaison rigide, le même conduit devient résonant. L'amplitude de la
pression présente des pics aux résonances quart d'onde `f = (2n+1) * c / (4L)`
et la phase est constante à `+/-90` degrés, avec une intensité nette nulle.

### Dépannage

| Symptôme | Cause |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* (l'analyse acoustique harmonique nécessite une fréquence positive) | la première fréquence est nulle et le pas de fréquence est nul |
| Le résidu stagne, les résultats semblent bruités | GMRES n'a pas convergé ; augmentez les itérations externes dans la configuration du solveur matriciel, ou utilisez un maillage plus grossier |
| L'amplitude explose à une fréquence | le balayage a rencontré une résonance non amortie ; ajoutez une frontière absorbante ou un facteur d'amortissement acoustique |
| La pression varie fortement le long d'un conduit censé être anéchoïque | la terminaison réfléchit ; vérifiez le coefficient d'absorption ou la valeur de l'impédance |

---

## 5. Vérification

Le test unitaire `tst_solver_acoustic` (dans `range-solver-lib/tests`, compilé
avec `-DRANGE_BUILD_TESTS=ON`) vérifie le solveur par rapport à des solutions
analytiques sur un conduit unidimensionnel de `1 m` discrétisé en 200 éléments
barres, avec les paramètres standard du solveur matriciel.

**Harmonique, terminaison anéchoïque** - `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz` :

| Grandeur | Analytique | Calculée |
|---|---|---|
| Amplitude de la pression | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` en chaque nœud |
| Amplitude du potentiel | `v/k = 1.0823e-3 m^2/s` | à 1 % près |
| Vitesse particulaire | `0.01 m/s` | `0.009999 m/s` |
| Niveau de pression acoustique | `103.18 dB` | à `0.1 dB` près |
| Intensité | `0.0204 W/m^2` | à 2 % près |
| Gradient de phase | `k * dx` | à 5 degrés près |

**Harmonique, terminaison rigide** - l'onde stationnaire donne une phase
d'exactement `+/-90` degrés et une intensité nette nulle, et l'amplitude
maximale dépasse l'amplitude de l'onde progressive.

**Transitoire** - un conduit excité à terminaison anéchoïque reste borné,
l'état de Newmark est publié afin que l'intégration se poursuive correctement
d'un pas à l'autre, et le champ excité est non nul et fini partout.

---

## 6. Limitations

- **Pas de couplage vibro-acoustique.** L'acoustique ne peut pas être combinée
  avec l'analyse modale, et il n'y a pas d'interface fluide-structure avec
  `RSolverStress`. Une structure vibrante doit être représentée par une
  condition aux limites *Velocity* imposée.
- **Acoustique linéaire uniquement.** Faible amplitude, milieu au repos. Pas
  d'écoulement moyen, pas d'effets convectifs, pas de propagation non linéaire.
- **Impédance réelle uniquement.** Les conditions aux limites absorbante et
  d'impédance prennent une valeur réelle. Une impédance de surface complexe et
  dépendant de la fréquence n'est pas prise en charge.
- **Données de matériau indépendantes de la fréquence.** La vitesse du son et
  le facteur d'amortissement sont tabulés en fonction de la température, et non
  de la fréquence ; ils sont donc constants sur tout un balayage.
- **Les termes de frontière utilisent la mesure simple de l'élément.** Pour un
  modèle à une ou deux dimensions dont la section ou l'épaisseur n'est pas égale
  à `1`, les termes de source et d'impédance ne sont pas pondérés par celle-ci,
  contrairement aux termes de domaine. Utilisez une section unité ou un modèle
  tridimensionnel complet pour éviter cette incohérence.
- **L'excitation harmonique est réelle.** Une vitesse imposée intervient comme
  partie réelle de la charge complexe ; des sources avec un déphasage relatif ne
  peuvent pas être définies.
