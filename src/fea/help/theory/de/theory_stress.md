# Spannungsanalyse - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Strukturlöser (`RSolverStress`) von Range FEA, der
sowohl den Problemtyp **Spannungsanalyse** (*Stress analysis*) als auch den
Problemtyp **Modalanalyse** (*Modal analysis*) bedient: die Gleichungen, die er
löst, die Bedeutung jeder Eingabe, die er akzeptiert, die Teile der grafischen
Benutzeroberfläche, die ihn steuern, und zwei ausgearbeitete Tutorials.

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - statische Spannungsanalyse](#3-tutorial---statische-spannungsanalyse)
4. [Tutorial - Modalanalyse](#4-tutorial---modalanalyse)
5. [Überprüfung eines Modells](#5-überprüfung-eines-modells)
6. [Einschränkungen](#6-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - das Verschiebungsfeld

Die primäre Unbekannte ist der **Verschiebungsvektor** `u = (ux, uy, uz)` in
jedem Knoten, in Metern. Jeder Knoten trägt drei Freiheitsgrade, sodass ein Netz
mit `N` Knoten ein System aus `3N` Gleichungen ergibt. Dehnungen, Spannungen und
Reaktionskräfte werden nach der Lösung alle aus dem Verschiebungsfeld gewonnen.

### 1.2 Grundgleichungen

Der Löser implementiert **lineare, isotrope Elastizität kleiner Dehnungen**. Drei
Beziehungen definieren sie.

**Gleichgewicht** eines Körpers unter einer Volumenlast `b` und einer
Oberflächenspannung `t`:

```
div(sigma) + b = 0
```

**Verzerrungs-Verschiebungs**-Beziehung, kleine Dehnungen:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

**Materialgesetz** mit einem thermischen Term:

```
sigma = D * ( eps - alpha * dT )
```

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `u` | Verschiebung | `m` | wird gelöst |
| `sigma` | Cauchy-Spannung | `Pa` | abgeleitet |
| `eps` | technische Dehnung | `-` | abgeleitet |
| `E` | Elastizitätsmodul | `Pa` | Materialeigenschaft |
| `v` | Querkontraktionszahl | `-` | Materialeigenschaft |
| `rho` | Dichte | `kg/m^3` | Materialeigenschaft |
| `alpha` | Wärmeausdehnungskoeffizient | `1/K` | Materialeigenschaft |
| `dT` | Temperaturerhöhung über dem spannungsfreien Zustand | `K` | siehe 1.5 |

Alle vier Materialeigenschaften sind **erforderlich** - eine Entität, der eine
davon fehlt, wird nicht gelöst. Die beiden Rayleigh-Dämpfungskoeffizienten
einer dynamischen Analyse (siehe 1.7) sind optional.

### 1.3 Elastizitätsmatrix

**Volumenelemente** verwenden die vollständige dreidimensionale isotrope
Elastizitätsmatrix in Voigt-Notation, mit dem Spannungsvektor in der
Reihenfolge `[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

**Flächenelemente** verwenden die zweidimensionale Form des **ebenen
Verzerrungszustands** mit dem Spannungsvektor `[sigma_xx, sigma_yy, tau_xy]`,
ausgewertet in der lokalen Elementebene und in globale Koordinaten
zurückgedreht:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Dies ist der ebene **Verzerrungs**zustand, nicht der ebene Spannungszustand.
Eine dünne, in ihrer eigenen Ebene belastete Platte ergibt sich daher steifer,
als es eine Idealisierung im ebenen Spannungszustand vorhersagen würde.

**Linienelemente** werden als **Stäbe** behandelt: Es wird nur die
Axialsteifigkeit `E * A` entlang der Elementrichtung assembliert, wobei `A` die
Querschnittsfläche der Linie ist. Es gibt keine Biege-, Schub- oder
Torsionssteifigkeit, auch nicht bei Elementtypen mit dem Namen „beam“ (Balken).

### 1.4 Finite-Elemente-Diskretisierung

Die schwache Form des Gleichgewichts ergibt für jedes Element

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(`stiffness matrix` = Steifigkeitsmatrix, `consistent mass matrix` =
konsistente Massenmatrix, `load vector, see 1.5` = Lastvektor, siehe 1.5),
wobei `B` die aus den Ableitungen der Formfunktionen aufgebaute
Verzerrungs-Verschiebungs-Matrix ist. Jede Elementmatrix ist für ein Element mit
`n` Knoten `3n x 3n` groß, mit den Freiheitsgraden verschränkt als
`[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Elementmaße.** Wie überall in Range FEA erhalten niederdimensionale Entitäten
ein geometrisches Maß, damit sie reale Körper darstellen können:

| Entität | Maß | Wo es gesetzt wird |
|---|---|---|
| Volumen | 1 | - |
| Fläche | Flächendicke | geometrische Eigenschaften der Entität |
| Linie | Querschnittsfläche der Linie | geometrische Eigenschaften der Entität |
| Punkt | Punktvolumen | geometrische Eigenschaften der Entität |

Eine Fläche mit Dicke null oder eine Linie mit Querschnittsfläche null trägt
weder Steifigkeit noch Masse bei - sie kann aber weiterhin Lasten tragen, was
genau das ist, was für eine lasttragende Seite eines vernetzten Festkörpers
gewünscht ist.

Die Massenmatrix wird nur assembliert, wenn sie benötigt wird, also für eine
dynamische transiente Analyse oder für eine Modalanalyse.

### 1.5 Lasten, Rand- und Umgebungsbedingungen

#### Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Displacement | explizit (Dirichlet) | Punkt, Linie, Fläche, Volumen | Displacement in X, Y, Z `[m]` |
| Normal displacement | explizit, lokales System | Fläche | Displacement `[m]` |
| Roller displacement | explizit, lokales System | Punkt, Linie, Fläche | Displacement `[m]` |
| Force | natürlich | Punkt, Linie, Fläche | Force in X, Y, Z `[N]` |
| Force (unit area) | natürlich | Fläche | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | natürlich | Fläche | Pressure `[Pa]` |
| Weight | natürlich | Punkt, Linie, Fläche | Weight `[kg]` |

**Displacement** ist eine Bedingung mit **optionalen** Komponenten: Jede ihrer
drei Komponenten kann einzeln ein- oder ausgeschaltet werden. Eine
eingeschaltete Komponente schränkt diese globale Richtung jedes Knotens der
Entität auf den angegebenen Wert ein; eine ausgeschaltete Komponente lässt diese
Richtung völlig frei. Das Ausschalten von *Displacement in Y* und *Displacement
in Z* ergibt daher ein Lager, das X festhält und den Knoten in der YZ-Ebene
gleiten lässt.

Verwenden Sie stattdessen *Roller displacement*, wenn die freien Richtungen nicht
die globalen sind, sondern die Tangentialebene einer gekrümmten Fläche.

**Normal displacement** und **Roller displacement** arbeiten in einem **lokalen
Koordinatensystem**, das für jeden eingeschränkten Knoten aufgebaut wird.
Standardmäßig folgt das System der Geometrie:

- auf einer **Fläche** ist die lokale X-Achse die gemittelte Normale der
  angrenzenden Flächenelemente;
- auf einer **Linie** wird das lokale System aus der Elementrichtung aufgebaut;
- auf einem **Punkt** gibt es keine Geometrie, der gefolgt werden könnte, daher
  wird immer die mit der Randbedingung eingegebene Richtung verwendet.

Das Aktivieren von **Eingegebene lokale Richtung verwenden** setzt dies für eine
Fläche oder eine Linie außer Kraft und übernimmt die lokale X-Achse aus der
darunter eingegebenen Richtung. Verwenden Sie es, wenn die Richtung, die Sie
festhalten möchten, nicht diejenige ist, die das Netz zufällig ergibt - etwa bei
einem Loslager, das entlang einer facettierten Fläche entlang einer globalen
Achse gleiten soll.

Randbedingungen verschiedener Entitäten **kombinieren** sich in einem gemeinsamen
Knoten. Der Löser sammelt jede auf einen Knoten wirkende
Verschiebungsrandbedingung, jede eine Aussage `d . u = v` über eine Richtung,
und reduziert die Sammlung mittels Gram-Schmidt auf höchstens drei zueinander
senkrechte festgehaltene Richtungen, wobei die vorgeschriebenen Werte dieselben
Operationen durchlaufen. Diese festgehaltenen Richtungen werden zu den führenden
Achsen des Knotensystems; was übrig bleibt, vervollständigt das System und
bleibt frei. Ein Knoten kann daher in einer Richtung durch ein globales
*Displacement*, in einer anderen durch ein *Roller displacement* auf einer
geneigten Seite festgehalten werden und in der dritten frei bleiben - die beiden
Bedingungen überschreiben einander nicht, und eine global formulierte
Komponente wird nie stillschweigend im lokalen System einer anderen Bedingung
umgedeutet.

Zwei Entitäten, die **unterschiedliche Werte in derselben Richtung**
vorschreiben, können nicht beide erfüllt werden. Der Löser erkennt dies bei der
Orthogonalisierung und hält mit einem Fehler an, der den Knoten nennt, statt den
Widerspruch zugunsten der zufällig zuletzt gelesenen Bedingung aufzulösen.

Die Elementmatrizen und Lastvektoren eines eingeschränkten Knotens werden in
dieses System gedreht, die Randbedingung wird angewendet, und die resultierende
Verschiebung wird zurückgedreht. In diesem System gilt:

- **Normal displacement** sperrt alle drei lokalen Komponenten, und der
  vorgeschriebene Skalar wird entlang der lokalen Normale angewendet. Der Knoten
  bewegt sich genau um den angegebenen Betrag entlang der Normale und ist sonst
  fest.
- **Roller displacement** sperrt auf einer Fläche oder einem Punkt nur die
  lokale Normalkomponente, sodass der Knoten in der Tangentialebene frei gleiten
  kann. Auf einer Linienentität sperrt sie stattdessen die beiden Komponenten
  senkrecht zur Linie, sodass der Knoten frei entlang der Linie gleiten kann.

**Force** ist eine Gesamtkraft in Newton, die über die Entität verteilt wird,
der sie zugewiesen ist - sie wird bei einer Linie durch die Länge der Entität,
bei einer Fläche durch die Fläche der Entität und bei einer Punktentität durch
die Anzahl der Punkte geteilt. Eine Verfeinerung des Netzes ändert die
Gesamtlast daher nicht.

**Force (unit area)** ist eine Flächenlast in `N/m^2`, die in der angegebenen
globalen Richtung aufgebracht und über die Elementfläche integriert wird.

**Pressure** wirkt entlang der **Normale** der Fläche. Der Löser bestimmt für
jede Randfläche, ob ihre Normale in das angrenzende berechenbare Volumenelement
zeigt, und kehrt das Vorzeichen entsprechend um, sodass ein positiver Druck
immer auf den Körper drückt. Eine Fläche, die kein Volumenelement begrenzt, wird
nie als nach innen gerichtet eingestuft, daher wirkt die Last dort entlang der
**negativen** Flächennormale - prüfen Sie in diesem Fall die Richtung der
resultierenden Last.

**Weight** ist eine an der Entität befestigte Masse in Kilogramm. Sie wird mit
der Erdbeschleunigung der Umgebungsbedingung multipliziert und auf dieselbe
Weise wie Force über die Entität verteilt. Verwenden Sie sie für nicht
modellierte angehängte Massen.

#### Umgebungsbedingungen

| Umgebungsbedingung | Komponenten | Wirkung |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | Eigengewicht und die Randbedingung Weight |
| Temperature | Temperature `[K]` | spannungsfreie Bezugstemperatur |

**Eigengewicht** wird automatisch für jede Entität erzeugt, die sowohl eine
Dichte als auch ein geometrisches Maß hat, als über das Element integriertes
`rho * g`. Es erscheint, sobald eine Umgebungsbedingung Erdbeschleunigung
zugewiesen ist; der Standard ist `-9.80665 m/s^2` in Z.

**Thermische Last.** Die Temperaturdifferenz, die den Wärmeausdehnungsterm
antreibt, ist

```
dT = element temperature - environment temperature
```

(`element temperature` = Elementtemperatur, `environment temperature` =
Umgebungstemperatur.) Die **Umgebungstemperatur** stammt aus der
Umgebungsbedingung *Temperature* und ist der spannungsfreie Bezugszustand. Die
**Elementtemperatur** stammt von einem Wärmelöser, der weiter vorne im selben
Aufgabenablauf des Problems läuft, und wird über die gemeinsamen Daten des
Lösers übergeben. *Heat transfer* und anschließend *Stress analysis* in einem
Aufgabenablauf auszuführen ergibt daher ohne weitere Einrichtung eine
thermische Spannungsanalyse. Ohne Wärmelöser bleibt die Elementtemperatur auf
ihrem Standardwert, und der thermische Term ist allein die Differenz zur
Umgebungsbedingung.

#### Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Displacement | Anfangsverschiebungsfeld einer dynamischen transienten Analyse |
| Structural velocity | Anfangsgeschwindigkeitsfeld einer dynamischen transienten Analyse |
| Pressure | anfängliche Druckbelastung der Fläche |

Anfangsbedingungen werden nur beim ersten Lauf einer transienten Analyse
gelesen. Eine dynamische Analyse ohne sie startet aus der Ruhe. Eingeschränkte
Richtungen behalten keine eigene Anfangsgeschwindigkeit, sie folgen der
vorgeschriebenen Verschiebung.

#### Kopplung mit einem Strömungslöser

Der Löser übernimmt das Ergebnis **Pressure** einer vorherigen Lösung und
verwendet es überall dort als Flächenlast, wo keine explizite
Druck-Randbedingung zugewiesen ist. Eine Strömungsaufgabe, gefolgt von einer
Spannungsaufgabe im selben Aufgabenablauf, überträgt den Fluiddruck daher
automatisch auf die Struktur.

In der anderen Richtung verschiebt jeder nicht strukturmechanische Löser das
Netz vor seinem Lauf um das gespeicherte Ergebnis **Displacement** und danach
wieder zurück, sodass eine Strömungs- oder Wärmelösung, die auf eine
Spannungslösung folgt, die verformte Gestalt sieht.

### 1.6 Statische Analyse

Bei deaktiviertem Zeitlöser und auf *Stress analysis* gesetztem Problemtyp
assembliert und löst der Löser

```
K * u = f
```

`K` ist symmetrisch positiv definit, sobald die Starrkörpermoden durch die
Verschiebungsrandbedingungen entfernt sind, und wird daher mit dem Verfahren der
**konjugierten Gradienten** mit Jacobi-Vorkonditionierung und einer Blockgröße
von 3 gelöst.

Ist das Modell unzureichend gelagert, ist die Matrix singulär, und die Lösung
konvergiert nicht. Jedes statische Modell benötigt genug Randbedingungen, um alle
sechs Starrkörpermoden zu unterdrücken - drei Translationen und drei
Rotationen.

### 1.7 Transiente Analyse

Bei aktiviertem Zeitlöser wählen die *Einstellungen der transienten
Spannungsanalyse* einen von zwei Analysetypen.

#### Dynamische Analyse

Der Standard. Der Löser integriert die Gleichungen der linearen Elastodynamik

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

sodass Trägheit, Spannungswellen, Schwingungen, dynamische Überhöhung und
Resonanz alle abgebildet werden.

**Dämpfung.** `C` ist eine Rayleigh-Dämpfung, Element für Element aus zwei
optionalen Materialeigenschaften aufgebaut,

```
C = alpha_M * M + beta_K * K
```

*Rayleigh mass damping coefficient* `alpha_M` `[1/s]` dämpft die tiefen
Frequenzen, *Rayleigh stiffness damping coefficient* `beta_K` `[s]` die hohen.
Der Dämpfungsgrad einer Mode mit der Kreisfrequenz `omega` beträgt

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

sodass ein Zieldämpfungsgrad `zeta` bei zwei Frequenzen `omega_1` und
`omega_2` erreicht wird mit

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Nehmen Sie die beiden Frequenzen aus einer Modalanalyse, um das Band herum, das
Sie interessiert. Typische strukturelle Dämpfungsgrade liegen bei 0.01 bis
0.05. Ohne die beiden Eigenschaften gibt es überhaupt keine physikalische
Dämpfung.

**Zeitintegration.** Das Newmark-Verfahren mit der Modifikation nach
Hilber-Hughes-Taylor (HHT). Mit der Verschiebung `u`, der Geschwindigkeit `v`
und der Beschleunigung `a` zu Beginn des Schritts löst jeder Schritt

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

und aktualisiert dann

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

Die effektive Matrix ist symmetrisch positiv definit und wird daher mit
demselben CG-Verfahren wie eine statische Analyse gelöst.

- Mit **HHT alpha = 0** (Standard) ist das Verfahren das einfache
  Newmark-Verfahren mit `gamma = 1/2` und `beta = max(theta/2, 1/4)`, wobei
  `theta` der Koeffizient der Approximation des Zeitschrittverfahrens ist (`1`
  rückwärts, `0.5` zentral, `0` vorwärts). Zentral und vorwärts ergeben beide
  das Verfahren der mittleren Beschleunigung - unbedingt stabil, genau zweiter
  Ordnung und frei von numerischer Dämpfung. Rückwärts ergibt `beta = 1/2`, was
  ebenfalls unbedingt stabil ist, aber die Perioden verlängert.
- Mit **HHT alpha** zwischen `-1/3` und `0` gilt `gamma = 1/2 - alpha` und
  `beta = (1 - alpha)^2 / 4`. Das Verfahren bleibt unbedingt stabil und genau
  zweiter Ordnung und dämpft die hohen Frequenzen, die ein grobes Netz schlecht
  auflöst, während es die tiefen Moden nahezu unberührt lässt. `-0.05` bis
  `-0.1` ist eine übliche Wahl, wenn die Antwort durch hochfrequentes Rauschen
  verunreinigt ist.

**Anfangsbeschleunigung.** Im ersten Schritt wird die Beschleunigung aus dem
Gleichgewicht gelöst, `M*a = f - K*u - C*v`, sodass eine zu Beginn aufgebrachte
Last die richtige Anfangsbeschleunigung erzeugt. Richtungen ohne Masse erhalten
null.

**Konzentrierte Masse.** Die Option *Konzentrierte Massenmatrix* ersetzt die
konsistente Masse durch eine diagonale mit derselben Gesamtmasse, indem die
Diagonale der konsistenten Matrix skaliert wird (HRZ-Konzentration, die auch für
quadratische Elemente positiv bleibt). Sie senkt die Eigenfrequenzen leicht und
ergibt hinter steilen Wellenfronten eine glattere Antwort.

**Wahl des Zeitschritts.** Das Verfahren ist für jedes `dt` stabil, die
Genauigkeit erfordert jedoch etwa 20 oder mehr Schritte pro Periode der höchsten
Mode, die Sie auflösen möchten. Für eine durch das Netz laufende Welle ist ein
Schritt nahe der Zeit, die die Welle zum Durchqueren eines Elements benötigt
(`dt ~ h/c`, `c = sqrt(E/rho)`), ein guter Anfang.

**Ergebnisse.** Neben Verschiebung und Spannung speichert eine dynamische
Analyse die Knotenvektoren *Structural velocity* und *Structural acceleration*.
Sie tragen auch den Zustand in den nächsten Zeitschritt und über einen Neustart
des Lösers.

#### Quasistatische Analyse

Jeder Zeitschritt ist eine unabhängige statische Lösung

```
K * u(t) = f(t)
```

unter Vernachlässigung von Trägheit und Dämpfung. Verwenden Sie sie für eine
Lastgeschichte, die im Vergleich zu den Eigenperioden der Struktur langsam
aufgebracht wird und bei der nur die Folge von Gleichgewichtszuständen zählt.
Sie benötigt keine Dichte und hat keine Zeitschrittbegrenzung.

### 1.8 Modalanalyse

Bei auf *Modal analysis* gesetztem Problemtyp assembliert der Löser `K` und `M`
und löst das verallgemeinerte Eigenwertproblem

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(`with` = mit.) Jedes Eigenpaar ist eine **Mode**: `phi` ist die Eigenform, die
als Verschiebungsfeld gespeichert und angezeigt wird, und `lambda` ist der
Eigenwert, aus dem sich die Eigenfrequenz ergibt als

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

Der Löser führt diese Umrechnung selbst durch: Die Modaleinstellungen und die
3D-Ansicht geben die Eigenfrequenz in Hz an, während das Löserprotokoll daneben
den zugrunde liegenden Eigenwert ausgibt. Eine Mode, die die
Eigenwertiteration nicht auflösen konnte, wird mit `0` Hz und einer Warnung im
Protokoll gemeldet.

Zwei Methoden stehen zur Verfügung, gewählt über die Einstellung der
Modalmethode:

| Modalmethode | Eigenwertlöser | Extrahiert |
|---|---|---|
| Multiple modes | Unterraumiteration mit Rayleigh-Ritz-Projektion | die angeforderte Anzahl von Moden |
| Only most dominant mode | inverse Vektoriteration mit Rayleigh-Quotient | eine einzige Mode |

Beide Methoden iterieren auf `K^-1 * M` und konvergieren daher gegen die
**tiefsten** Frequenzen - also die, auf die es ankommt. Jeder Schritt ist eine
lineare Lösung mit `K` unter Verwendung der Einstellungen für konjugierte
Gradienten aus der Einrichtung des Matrixlösers, sodass der Aufwand einer
Modalanalyse etwa dem Aufwand einer statischen Lösung mal der Anzahl der
Unterraumvektoren und Iterationen entspricht.

Die Unterraumiteration führt einen Block von Vektoren mit, projiziert `K` und
`M` auf den von ihnen aufgespannten Unterraum und löst das resultierende kleine
dichte Eigenproblem exakt. Einige zusätzliche Vektoren über die angeforderte
Anzahl von Moden hinaus werden mitgeführt, weil sie die gewünschten Moden
deutlich schneller konvergieren lassen. Für `p` angeforderte Moden enthält der
Block `min(2p, p+8)` Vektoren, der übliche Kompromiss zwischen
Konvergenzgeschwindigkeit und Aufwand einer Iteration. Die Eigenwerte ergeben
sich direkt als `lambda` und werden aufsteigend sortiert, daher ist **Mode 0 die
Grundmode**.

**Hier geht die Laufzeit hin.** Eine Unterraumiteration führt pro Vektor des
Blocks eine vollständige lineare Lösung durch, sodass 10 Moden 18 Lösungen pro
Iteration kosten und 100 Moden 108. Die Iteration hält nur dann vorzeitig an,
wenn sich **jeder** angeforderte Eigenwert innerhalb des Konvergenzwerts
eingependelt hat, und die höchste angeforderte Mode pendelt sich immer zuletzt
ein - eine große Anzahl von Moden macht also jede Iteration teuer und den
vorzeitigen Abbruch unwahrscheinlich. Fordern Sie die Moden an, die Sie
tatsächlich betrachten wollen.

Haben sich die angeforderten Moden bis zur letzten Iteration nicht eingependelt,
meldet der Löser dies im Protokoll und gibt die erreichte Änderung an, statt die
nicht konvergierten Werte kommentarlos zu präsentieren. Die tiefsten Moden eines
solchen Laufs sind meist noch brauchbar; die höchsten nicht.

Für eine Modalanalyse wird die Massenmatrix immer assembliert, daher muss die
Dichte auf jeder beteiligten Entität vorhanden sein.

**Vorgespannte Modalanalyse.** Vor dem Assemblieren der Matrizen verschiebt eine
Modalanalyse jeden Knoten um das Verschiebungsergebnis einer vorherigen Lösung
und stellt das Netz danach wieder her. *Stress analysis* und anschließend *Modal
analysis* in einem Aufgabenablauf auszuführen ergibt daher die Moden der
**belasteten, verformten** Struktur statt der unverformten. Die gespeicherte
Eigenform ist der Eigenvektor, der zu dieser Anfangsverschiebung addiert wird.

Es wird nur die Geometrie aktualisiert - die Steifigkeitsmatrix ist weiterhin
die linear-elastische, ausgewertet auf dem verschobenen Netz. Dies erfasst die
Formänderung, nicht die geometrische Versteifung eines gespannten Bauteils.

Die Modalanalyse ist **exklusiv**: Sie kann mit keiner anderen Physik kombiniert
werden außer mit einer vorangehenden Spannungsanalyse.

### 1.9 Abgeleitete Ergebnisse

Nach der Lösung werden die Elementspannungen aus dem Verschiebungsfeld als
Mittelwert über die Integrationspunkte von

```
sigma = D * ( B * u  -  eps_th )
```

gewonnen, mit der thermischen Dehnung `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]`
für Volumenelemente. Flächenelemente befinden sich im ebenen
Verzerrungszustand: Die Ausdehnung aus ihrer Ebene heraus wird unterdrückt, was
die thermische Dehnung in der Ebene auf
`eps_th = (1+v) * alpha * dT * [1, 1, 0]` erhöht. Dieselbe thermische Dehnung
geht in die thermische Last `f_th = INT( B^T * D * eps_th )` ein.
Linienelemente geben `E * (eps - alpha * dT)` aus.

Aus den sechs Komponenten bildet der Löser zwei Invarianten und kombiniert sie
quadratisch zur Vergleichsspannung nach von Mises:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(`normal part` = Normalanteil, `shear part` = Schubanteil), was der klassischen
Definition entspricht

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

Für Flächenelemente werden die zweidimensionalen Entsprechungen verwendet, `QN`
aus den beiden Normalspannungen in der Ebene und `QS = sqrt(3 * tau_xy^2)`. Für
Linienelemente wird nur der axiale Wert ausgegeben, und der Schubanteil ist
null.

**Knotenkräfte** werden Element für Element als innere elastische Kraft
gewonnen

```
f = K * u - f_th
```

und in den Knoten aufsummiert. In einem eingeschränkten Knoten ist dies die
Reaktionskraft; in einem unbelasteten freien Knoten sollte sie bei einer
konvergierten statischen Lösung nahezu null sein, was das Ergebnis Force zu einer
nützlichen Residuumsprüfung macht. Ein Körper, der sich thermisch frei ausdehnen
kann, zeigt daher weder Kräfte noch Spannungen.

| Ergebnis | Anwendbar auf | Bedeutung |
|---|---|---|
| Displacement `[m]` | Knoten | Verschiebungsvektor oder bei einer Modalanalyse die Eigenform |
| Von Mises stress `[Pa]` | Element | `sqrt(QN^2 + QS^2)` wie oben |
| Normal stress `[Pa]` | Element | `QN` |
| Shear stress `[Pa]` | Element | `QS` |
| Normal stress in X, Y, Z `[Pa]` | Element | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | Element | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | Knoten | gewonnene Knotenkraft, Reaktion in eingeschränkten Knoten |

Die sechs Spannungskomponenten werden für Volumenelemente in **globalen**
Koordinaten ausgegeben. Flächen- und Linienelemente werten ihre Spannung in
ihrem eigenen lokalen Elementsystem aus, daher werden ihre Komponenten in diesem
System ausgegeben: Eine Fläche füllt die in der Ebene liegenden `sigma_xx`,
`sigma_yy` und `tau_xy`, und eine Linie füllt nur das axiale `sigma_xx`.

### 1.10 Statisch, transient oder modal?

| | Statisch | Dynamisch transient | Quasistatisch transient | Modal |
|---|---|---|---|---|
| Löst | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` in der Zeit | `K*u = f(t)` zu jedem Zeitpunkt | `K*phi = omega^2*M*phi` |
| Beantwortet | „wie stark verformt es sich und wie hoch ist es beansprucht“ | „wie reagiert es auf eine sich schnell ändernde Last“ | „wie folgt es einer sich langsam ändernden Last“ | „bei welchen Frequenzen und in welcher Form will es schwingen“ |
| Benötigt Masse | nein | ja | nein | ja |
| Benötigt Lagerung gegen Starrkörperbewegung | ja | ja | ja | ja |
| Datensätze | einer | einer pro Zeitschritt | einer pro Zeitschritt | einer pro extrahierter Mode |

**Verwenden Sie eine statische Analyse für** Festigkeits- und
Steifigkeitsnachweise, Durchbiegungsgrenzen, Reaktionskräfte, Eigengewicht,
Druckbehälter und thermische Spannungen. Das ist der Standard und deckt die
meisten Arbeiten ab.

**Verwenden Sie eine dynamische transiente Analyse für** Stöße, plötzlich
aufgebrachte oder entfernte Lasten, Stoß- und Schwingungsverläufe,
Wellenausbreitung und die Antwort auf eine harmonische Anregung nahe einer
Resonanz - überall dort, wo Trägheit eine Rolle spielt.

**Verwenden Sie eine quasistatische transiente Analyse für** eine Last, die sich
über die Zeit so langsam ändert, dass die Struktur ihr im Gleichgewicht folgen
kann.

**Verwenden Sie eine Modalanalyse für** Eigenfrequenzen und Eigenformen, die
Vermeidung von Resonanz mit einer bekannten Anregung, die Beurteilung, ob eine
Struktur dynamisch steif genug ist, und die Suche nach unbeabsichtigten
Mechanismen - ein Eigenwert nahe null bedeutet meist eine fehlende Lagerung oder
ein nicht verbundenes Teil.

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Umgebungsbedingungen`, `Material`,
`Ergebnisse`), im Menü **Problem** und im Panel **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Fügen Sie eine Aufgabe hinzu und wählen Sie
entweder:

- **Stress analysis** - „Stress and linear elasto-dynamics analysis including
  modal analysis“ (Spannungsanalyse und lineare Elastodynamik einschließlich
  Modalanalyse);
- **Modal analysis** - „Stress modal analysis“ (strukturmechanische
  Modalanalyse).

Die Spannungsanalyse lässt sich frei mit anderer Physik kombinieren. Zwei
nützliche Ketten sind *Heat transfer*, dann *Stress analysis* für thermische
Spannungen, und *Incompressible viscous flow*, dann *Stress analysis* für
Druckbelastung.

Die Modalanalyse schließt jeden anderen Problemtyp aus außer einer
vorangehenden Spannungsanalyse, die sie zu einer vorgespannten Modalanalyse
macht.

### 2.2 Registerkarte Problem

**Zeitlöser** - wird für die Spannungsanalyse angezeigt, da sie ein
zeitabhängiger Problemtyp ist. Lassen Sie ihn für eine statische Analyse
deaktiviert.

| Feld | Bedeutung |
|---|---|
| Aktivieren | schaltet das Zeitschrittverfahren ein und aus |
| Approximation | setzt das Newmark-beta einer dynamischen Analyse, siehe 1.7; zentral ist die empfohlene Wahl |
| Startzeit | erster Zeitwert in Sekunden; wird bei einem Neustart des Lösers ignoriert |
| Endzeit | `start time + time-step size * number of time-steps`, schreibgeschützt |
| Zeitschrittweite | `dt` in Sekunden |
| Anzahl der Zeitschritte | wie viele Schritte berechnet werden |
| Ausgabefrequenz | schreibt alle N Schritte eine Ergebnisdatei; `0` schreibt nur den letzten Schritt |

**Einstellungen der transienten Spannungsanalyse** - wird für die
Spannungsanalyse angezeigt und ist nur aktiv, solange der Zeitlöser aktiviert
ist.

| Feld | Bedeutung | Standard |
|---|---|---|
| Analysetyp | *Dynamic* (mit Trägheit) oder *Quasi-static* (eine statische Lösung pro Zeitschritt) | Dynamic |
| Numerische Dämpfung (HHT alpha) | von `-1/3` bis `0`; negative Werte dämpfen hochfrequentes Rauschen | 0 |
| Konzentrierte Massenmatrix | eine diagonale Massenmatrix statt der konsistenten verwenden | aus |

Die letzten beiden gelten nur für die dynamische Analyse und sind sonst
ausgegraut.

**Einstellungen der Modalanalyse** - wird angezeigt, wenn der Problemtyp *Modal
analysis* ist.

| Feld | Bedeutung | Standard |
|---|---|---|
| Modalmethode | *Only most dominant mode* oder *Multiple modes* | Multiple modes |
| Iterationen | Iterationen des Eigenwertlösers; bei der Methode mit dominanter Mode deaktiviert | 100 |
| Moden extrahieren | wie viele Moden extrahiert werden; bei der Methode mit dominanter Mode deaktiviert | 10 |
| Konvergenzwert | Konvergenzschwelle des Eigenwertlösers | 1.0e-9 |

Die beiden Anzahlen sind unabhängig: Die Iterationen verfeinern die gesamte
Menge der Moden gemeinsam, sie erzeugen sie nicht einzeln nacheinander.

**Moden extrahieren ist die Einstellung, die über die Laufzeit entscheidet.**
Jede Mode fügt jeder Iteration eine lineare Lösung hinzu, und der Lauf kann erst
vorzeitig anhalten, wenn sich alle angeforderten Moden eingependelt haben,
sodass der Aufwand schneller wächst als die Anzahl. Zehn ist ein sinnvoller
Ausgangspunkt und der Standard; hundert ist eine Berechnung anderer
Größenordnung und kann auf einem großen Netz Stunden dauern. Erhöhen Sie den
Wert nur, wenn Sie wissen, dass Sie die höheren Moden benötigen.

Dauert ein Lauf länger als erwartet, nennt das Löserprotokoll den
Unterraumvektor, an dem gerade gearbeitet wird, sodass der Fortschritt innerhalb
einer Iteration sichtbar ist, und es gibt nach jeder Iteration die erreichte
Konvergenzrate neben dem geforderten Wert aus.

### 2.3 Registerkarte Material

Die Registerkarte `Material` listet das der ausgewählten Entität zugewiesene
Material auf. Die Spannungsanalyse benötigt **alle vier** dieser Eigenschaften:

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, etwa `2.1e11` für Stahl |
| Poisson ratio | `-` | `v`, etwa `0.3` für Stahl; muss unter `0.5` bleiben |
| Density | `kg/m^3` | benötigt für Eigengewicht, dynamische Analyse und Modalanalyse |
| Thermal expansion coefficient | `1/K` | auf `0` setzen, wenn keine thermische Last vorliegt |

Eine Entität, der eine davon fehlt, wird nicht gelöst, und die
Einstellungsprüfung warnt vor dem Start des Laufs.

Zwei weitere Eigenschaften sind optional und werden nur von einer dynamischen
transienten Analyse gelesen:

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Rayleigh mass damping coefficient | `1/s` | `alpha_M`, siehe 1.7; null, wenn nicht vorhanden |
| Rayleigh stiffness damping coefficient | `s` | `beta_K`, siehe 1.7; null, wenn nicht vorhanden |

Jede Eigenschaft ist eine Tabelle über der Temperatur, sodass ein
temperaturabhängiger Modul berücksichtigt wird, wenn zuvor ein Wärmelöser läuft.

### 2.4 Registerkarte Randbedingungen

Die Registerkarte ist oben in eine Verwaltungsliste der verfügbaren
Randbedingungen und unten in einen Editor für die ausgewählte Bedingung
aufgeteilt. Wählen Sie zuerst eine Entität im Baum `Modell` aus; die Verwaltung
bietet dann die für diesen Entitätstyp und das ausgewählte Problem gültigen
Bedingungen an. Aktivieren Sie eine, um sie zuzuweisen, und bearbeiten Sie dann
ihre Komponenten im unteren Baum.

Es werden die Bedingungen aus Abschnitt 1.5 angeboten. Hinweise zu ihrer
Verwendung:

- **Displacement** zeigt neben jeder ihrer drei Komponenten ein
  Kontrollkästchen. Nur die aktivierten Komponenten werden eingeschränkt, sodass
  eine Seite in X festgehalten und in Y und Z frei gelassen werden kann. Das
  Wertfeld einer deaktivierten Komponente ist ausgegraut.
- **Normal displacement** und **Roller displacement** übernehmen ihr lokales
  System standardmäßig aus der Geometrie - die gemittelten Elementnormalen einer
  Fläche, die Elementrichtung einer Linie. Auf einer **Punkt**entität gibt es
  keine Geometrie, der gefolgt werden könnte, daher wird der Editor *Lokale
  Richtung* immer angezeigt und immer verwendet.
- Auf einer Fläche oder einer Linie zeigt der Bedingungsbaum ein Kontrollkästchen
  **Eingegebene lokale Richtung verwenden**. Aktivieren Sie es, um den Editor
  *Lokale Richtung* einzublenden und das System aus der eingegebenen Richtung
  statt aus dem Netz zu übernehmen. Die in der 3D-Ansicht gezeichneten
  Richtungspfeile folgen der jeweils wirksamen Variante. Verhält sich eine
  Flächenrandbedingung bei deaktiviertem Kontrollkästchen seltsam, prüfen Sie, ob
  die Flächennormalen konsistent sind, und synchronisieren Sie sie mit den
  Geometriewerkzeugen.
- Randbedingungen mehrerer Entitäten, die in einem Knoten zusammentreffen,
  werden kombiniert, sodass eine Seite gleichzeitig durch *Displacement* in einer
  globalen Richtung festgehalten und auf einer geneigten Ebene gleitend gelagert
  werden kann. Verlangen zwei davon unterschiedliche Werte in derselben
  Richtung, hält der Lauf mit einem Fehler an, der den Knoten nennt.
- **Pressure** und **Force (unit area)** gelten nur für Flächen.
- Jede Komponente ist eine Tabelle über der Zeit, sodass einer Last für einen
  transienten Lauf ein Zeitverlauf gegeben werden kann.

### 2.5 Anfangs- und Umgebungsbedingungen

Auf der Registerkarte `Umgebungsbedingungen` wird die **Schwerkraft**
eingeschaltet. Weisen Sie *Gravitational acceleration* der Entität zu, die ihr
Eigengewicht tragen soll - meist das gesamte Modell -, und setzen Sie den
Vektor; der Standard ist `-9.80665` in Z. Weisen Sie *Temperature* zu, um die
spannungsfreie Bezugstemperatur für eine thermische Spannungsanalyse
festzulegen.

Die Registerkarte `Anfangsbedingungen` bietet *Displacement*, *Structural
velocity* und *Pressure* an, die beim ersten Lauf einer transienten Analyse
angewendet werden.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Strukturlöser verwendet den Eintrag **CG**, sowohl für die statische
und transiente Lösung als auch für die inneren Lösungen der Eigenwertiteration.

Konvergiert eine statische Lösung nicht, ist die übliche Ursache ein
unzureichend gelagertes Modell und keine Lösereinstellung. Prüfen Sie die
Randbedingungen, bevor Sie die Anzahl der Iterationen erhöhen.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable - typischerweise
*Displacement* oder *Von Mises stress*. In einem transienten Lauf wird der
Verlauf über `Bericht` -> `Überwachungspunkte` angezeigt.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet die berechneten Variablen auf und steuert
die 3D-Ansicht. Die Verschiebung ist ein Knotenvektor; die drei
Spannungsgrößen sind Elementskalare. Eine dynamische transiente Analyse fügt die
Knotenvektoren *Structural velocity* und *Structural acceleration* hinzu.

Für eine Darstellung der verformten Gestalt zeigen Sie *Displacement* an und
aktivieren Sie in den Ansichtseinstellungen die Skalierung der Verformung -
reale Verschiebungen sind meist viel zu klein, um im wahren Maßstab sichtbar zu
sein.

Die Registerkarte `Datensätze` des Panels `Modell` listet die
Ergebnisdatensätze auf:

- **statische Analyse** - ein einziger Datensatz;
- **transiente Analyse** - ein Datensatz pro geschriebenem Zeitschritt;
- **Modalanalyse** - ein Datensatz pro extrahierter Mode, ab der Grundmode
  nummeriert. Das schrittweise Durchgehen der Datensätze geht die Moden durch,
  und ihre Animation animiert die Eigenformen.

Für einen modalen Datensatz beschriftet die 3D-Ansicht den Datensatz mit der
Nummer der Mode und ihrer Eigenfrequenz in Hz.

`Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige Ausgabe des
Lösers, einschließlich des für jede extrahierte Mode ausgegebenen Eigenwerts und
der Statistik von Verschiebung, Vergleichsspannung nach von Mises und Kraft nach
jedem Datensatz.

---

## 3. Tutorial - statische Spannungsanalyse

**Ziel.** Durchbiegung und Spannung eines Kragarms berechnen, der durch sein
Eigengewicht und eine Last am Ende belastet ist, und die Reaktion am Lager
ablesen.

Dieses Tutorial setzt ein vernetztes Volumenmodell voraus - ein langer Quader
eignet sich gut. Verwenden Sie `Datei` -> `Neues Modell`, zeichnen Sie einen
Quader mit `Geometrie` -> `Element erstellen` oder importieren Sie eine
Geometrie und tetraedrisieren Sie sie; das Tutorial *Würfel zeichnen* behandelt
diese Schritte. Markieren Sie das eingespannte und das belastete Ende als
separate Flächenentitäten, damit ihnen Randbedingungen zugewiesen werden
können.

### Schritt 1 - Problem auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine Aufgabe hinzu und wählen Sie **Stress analysis**.
3. Bestätigen Sie mit `OK`.

### Schritt 2 - Material zuweisen

1. Wählen Sie die Volumenentität im Baum `Modell` aus.
2. Öffnen Sie die Registerkarte `Material` und weisen Sie ein Material mit allen
   vier Eigenschaften zu, für Baustahl:
   - **Modulus of elasticity** = `2.1e11` `Pa`
   - **Poisson ratio** = `0.3`
   - **Density** = `7850` `kg/m^3`
   - **Thermal expansion coefficient** = `1.2e-5` `1/K`

### Schritt 3 - Analyse statisch belassen

Öffnen Sie die Registerkarte `Problem` und lassen Sie im Gruppenfeld
*Zeitlöser* **Aktivieren** deaktiviert. Der Löser assembliert und löst dann
einmal `K*u = f`.

### Schritt 4 - Modell lagern

1. Wählen Sie die Flächenentität am eingespannten Ende aus.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie
   **Displacement**.
3. Lassen Sie alle drei Komponenten aktiviert und auf `0` gesetzt.

Dies spannt die Seite vollständig ein und entfernt alle sechs
Starrkörpermoden. Prüfen Sie, dass nichts anderes frei schwebt - ein nicht
verbundenes Teil ohne Lagerung macht das System singulär, und die Lösung
konvergiert nicht.

Für ein Lager, das gleiten statt vollständig eingespannt sein soll, verwenden
Sie stattdessen **Roller displacement** auf der Fläche. Es sperrt nur die
Richtung der Flächennormale und lässt den Knoten in der Tangentialebene frei.

### Schritt 5 - Lasten aufbringen

**Eigengewicht.** Wählen Sie die Volumenentität aus, öffnen Sie die
Registerkarte `Umgebungsbedingungen` und aktivieren Sie **Gravitational
acceleration**. Lassen Sie den Standardwert `-9.80665` in Z.

**Endlast.** Wählen Sie die Flächenentität am freien Ende aus, öffnen Sie die
Registerkarte `Randbedingungen` und aktivieren Sie **Force**. Geben Sie die
Gesamtkraft in Newton ein, zum Beispiel `-1000` in Z. Der Wert ist die Summe
über die Entität und ändert sich daher nicht, wenn das Netz verfeinert wird.

Alternativen, je nachdem, was die Last physikalisch ist:

- **Pressure** für einen gleichmäßigen Normaldruck auf eine Seite, in `Pa`;
- **Force (unit area)** für eine gerichtete Flächenlast in `N/m^2`;
- **Weight** für eine angehängte Masse in `kg`, die nicht als Geometrie
  modelliert ist.

### Schritt 6 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Die Einstellungsprüfung meldet fehlende Materialien oder fehlende
   Randbedingungen vor dem Start des Laufs.
3. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.

### Schritt 7 - Ergebnisse untersuchen

- Registerkarte `Ergebnisse`: Zeigen Sie **Displacement** an, um die verformte
  Gestalt zu sehen, und **Von Mises stress** für die Spannungsverteilung.
  Aktivieren Sie in den Ansichtseinstellungen die Skalierung der Verformung,
  damit die Durchbiegung sichtbar wird.
- **Normal stress** und **Shear stress** trennen die beiden Beiträge, aus denen
  sich die angegebene Vergleichsspannung nach von Mises zusammensetzt.
- **Force** an der eingespannten Seite ist die Reaktion. Ihre Summe sollte die
  aufgebrachte Last plus das Eigengewicht ausgleichen - eine schnelle und
  wirksame Prüfung, dass das Modell so belastet ist, wie Sie es beabsichtigt
  haben.
- Ein Überwachungspunkt auf *Displacement* am Ende liefert die Durchbiegung als
  Zahl statt als Farbe.

### Schritt 8 - optional, thermische Spannung

Um eine thermische Last hinzuzufügen:

1. Fügen Sie im Aufgabenablauf des Problems *vor* der Spannungsaufgabe eine
   Aufgabe **Heat transfer** hinzu und richten Sie deren Randbedingungen und
   Materialeigenschaften ein.
2. Weisen Sie der Strukturentität eine Umgebungsbedingung **Temperature** mit
   der spannungsfreien Bezugstemperatur zu, zum Beispiel `293.15` K.

Der Spannungslöser übernimmt die vom Wärmelöser berechneten
Elementtemperaturen und fügt den Wärmeausdehnungsterm automatisch hinzu.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die Lösung konvergiert nicht | das Modell ist unzureichend gelagert; ein Teil schwebt frei, oder die Lager entfernen nicht alle sechs Starrkörpermoden |
| *Entity has material assigned which is missing required properties* | eine der vier erforderlichen Eigenschaften fehlt |
| Die Verschiebungen sind überall null | keine Last hat das Modell erreicht - prüfen Sie, ob die Schwerkraft zugewiesen ist oder ob die Komponenten von Force ungleich null sind |
| Die Verschiebungen sind riesig | der Elastizitätsmodul ist um Größenordnungen falsch, oder eine Flächenentität hat Dicke null und damit keine Steifigkeit |
| Die Last ändert sich bei Verfeinerung des Netzes | *Force (unit area)* oder *Pressure* war als Gesamtkraft gemeint, oder umgekehrt |
| Der Druck drückt in die falsche Richtung | die Flächennormalen sind nicht konsistent, oder die Fläche begrenzt kein Volumen |

---

## 4. Tutorial - Modalanalyse

**Ziel.** Die ersten Eigenmoden desselben Kragarms extrahieren.

### Schritt 1 - Problem auswählen

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`), fügen Sie eine Aufgabe
hinzu und wählen Sie **Modal analysis**.

Die Modalanalyse kann nicht mit anderer Physik kombiniert werden, mit der
einzigen Ausnahme einer davor platzierten Spannungsanalyse, die sie zu einer
vorgespannten Modalanalyse macht.

### Schritt 2 - Material zuweisen

Dieselben vier Eigenschaften wie im statischen Tutorial. Die **Dichte** spielt
hier eine größere Rolle als irgendwo sonst: Sie ist die gesamte Massenmatrix,
und die Frequenzen skalieren mit `1/sqrt(rho)`.

### Schritt 3 - Modell lagern

Weisen Sie dieselbe Randbedingung **Displacement** am eingespannten Ende zu.

Die Lagerung unterscheidet die Moden eines Kragarms von denen eines frei
schwebenden Stabs. Bleibt das Modell ungelagert, liegen die ersten sechs
Eigenwerte nahe null - die Starrkörpermoden -, und die interessanten Moden
werden in der Liste nach oben verschoben.

### Schritt 4 - Modaleinstellungen konfigurieren

Öffnen Sie die Registerkarte `Problem` und stellen Sie in *Einstellungen der
Modalanalyse* Folgendes ein:

| Feld | Wert |
|---|---|
| Modalmethode | `Multiple modes` |
| Iterationen | `100` |
| Moden extrahieren | `6` |
| Konvergenzwert | `1.0e-9` |

Sechs Moden sind ein sinnvoller Ausgangspunkt und weniger als die
standardmäßigen zehn. Jede zusätzliche Mode fügt jeder Iteration eine lineare
Lösung hinzu, daher ist eine große Anzahl von Moden auf einem großen Netz
langsam.

Wählen Sie `Only most dominant mode`, wenn nur eine einzige Mode benötigt wird;
diese Methode verwendet die inverse Vektoriteration auf einem einzigen Vektor
und ist viel günstiger.

### Schritt 5 - es werden keine Lasten benötigt

Eine Modalanalyse hat keinen Lastvektor - das Eigenwertproblem ist homogen. Dem
Modell zugewiesene Lasten werden ignoriert, mit einer Ausnahme: Ist zuvor eine
Spannungsanalyse gelaufen, verformt deren Verschiebungsergebnis das Netz für die
modale Assemblierung.

### Schritt 6 - lösen

`Lösung` -> `Löser starten` (`Ctrl+R`). Das Protokoll meldet die
Unterraumiterationen und ihre Konvergenzrate und dann einen Block pro
extrahierter Mode:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

Die Moden werden vom höchsten extrahierten Index abwärts bis zur Grundmode
verarbeitet, sodass die Grundmode als letzte berechnet und als Datensatz 1
gespeichert wird.

### Schritt 7 - Moden untersuchen

- Panel `Modell`, Registerkarte `Datensätze`: ein Datensatz pro Mode, ab der
  Grundmode nummeriert. Gehen Sie sie schrittweise durch, um jede Eigenform zu
  sehen.
- Registerkarte `Ergebnisse`: Zeigen Sie **Displacement** an. Eine Eigenform ist
  nur bis auf einen Skalierungsfaktor definiert, daher sind die Beträge
  willkürlich - entscheidend ist die Form. Aktivieren Sie die Skalierung der
  Verformung und animieren Sie den Datensatz, um die Mode zu visualisieren.
- Der Eigenwert jeder Mode wird im Löserprotokoll ausgegeben und in der
  3D-Ansicht als Beschriftung des Datensatzes angezeigt.

### Schritt 8 - Eigenfrequenzen ablesen

Die 3D-Ansicht beschriftet jeden modalen Datensatz mit seiner Modennummer und
seiner Eigenfrequenz in Hz. Das Löserprotokoll listet für jede extrahierte Mode
dieselbe Frequenz zusammen mit dem zugrunde liegenden Eigenwert auf:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

Eine Mode, die die Eigenwertiteration nicht auflösen konnte, wird mit `0` Hz
gemeldet und trägt eine Warnung im Protokoll.

Wie bei jedem Ergebnis lohnt es sich, die Grundfrequenz beim ersten Einrichten
eines Modells mit einer Handrechnung zu vergleichen - Abschnitt 5 gibt die
Formel für einen Kragarm an.

### Schritt 9 - optional, vorgespannte Moden

1. Platzieren Sie im Aufgabenablauf des Problems eine Aufgabe **Stress
   analysis** *vor* der Aufgabe **Modal analysis**.
2. Richten Sie die statischen Lasten und Lagerungen wie in Tutorial 3 ein.

Die Modalanalyse läuft dann auf dem verformten Netz. So lässt sich sehen, wie
eine statische Last die Form verändert, in der eine Struktur schwingt.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die ersten Eigenwerte liegen nahe null | das Modell ist unzureichend gelagert, oder ein Teil ist nicht verbunden - ein echter Mechanismus |
| Der Eigenwertlöser konvergiert nicht | zu wenige Iterationen oder eine schlecht konditionierte Massenmatrix, weil eine Entität keine Dichte oder kein geometrisches Maß hat |
| Das Extrahieren der Moden ist sehr langsam | zu viele Moden angefordert; jede Mode kostet eine vollständige lineare Lösung |
| Der Lauf dauert Stunden oder scheint nie zu enden | zu viele Moden angefordert - jede kostet in jeder Iteration eine lineare Lösung, und der Lauf kann erst vorzeitig anhalten, wenn sich alle eingependelt haben. Beobachten Sie die Anzahl der Unterraumvektoren im Protokoll, um den Fortschritt innerhalb einer Iteration zu sehen |
| Eine Frequenz wirkt absurd | die Unterraumiteration ist nicht konvergiert - das Protokoll meldet dies am Ende eines nicht konvergierten Laufs; extrahieren Sie weniger Moden, erhöhen Sie die Anzahl der Iterationen oder prüfen Sie Dichte und Lagerung |
| Die Eigenformen sehen identisch aus | dicht beieinander liegende Moden einer symmetrischen Struktur oder zu wenige Unterraumiterationen, um sie zu trennen |
| Weniger Datensätze als angeforderte Moden | der Unterraum hat unabhängige Richtungen verloren; das Protokoll gibt an, wie viele Moden extrahiert wurden, und speichert nur diese |

---

## 5. Überprüfung eines Modells

Der Löser hat keine eingebaute Verifikationssuite, daher lohnt es sich, ein neues
Modell an etwas zu validieren, das Sie von Hand berechnen können, bevor Sie ihm
vertrauen. Drei günstige Prüfungen, nach Nützlichkeit geordnet:

**Reaktionsbilanz.** Summieren Sie das Ergebnis *Force* über die eingeschränkten
Knoten. Es muss die aufgebrachten Lasten plus das Eigengewicht `rho * V * g`
ausgleichen. Dies deckt falsche Lastbeträge, falsche Einheiten und falsche
Druckrichtungen auf.

**Durchbiegung am Ende eines Kragarms.** Für einen Balken der Länge `L` mit dem
Flächenträgheitsmoment `I` und der Endlast `F` ergibt die Euler-Bernoulli-Theorie

```
delta = F * L^3 / (3 * E * I)
```

Modellieren Sie den Balken mit Volumenelementen und vergleichen Sie. Rechnen Sie
damit, dass das Finite-Elemente-Ergebnis auf einem groben Netz etwas steifer ist
und bei Verfeinerung des Netzes von unten konvergiert. Verwenden Sie für diese
Prüfung keine Linienelemente - sie haben keine Biegesteifigkeit.

**Grundfrequenz eines Kragarms.** Für denselben Balken mit der
Querschnittsfläche `A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Vergleichen Sie mit `sqrt(lambda_0) / (2*pi)` aus der Modalanalyse. Dies
validiert gleichzeitig die Massenmatrix, die Steifigkeitsmatrix und Ihre
Umrechnung der Eigenwerte.

**Plötzlich belasteter Stab.** Ein an einem Ende eingespannter Stab, der am
anderen Ende durch eine bei `t = 0` eingeschaltete Axialkraft `F` belastet wird,
ist eine Prüfung einer dynamischen Analyse. Die Spannungswelle läuft mit der
Geschwindigkeit `c = sqrt(E/rho)` zum eingespannten Ende und zurück, sodass sich
das freie Ende linear bis zur doppelten statischen Auslenkung
`2 * F * L / (E * A)` bei `t = 2*L/c` bewegt und bei `t = 4*L/c` zurück auf null.
Ohne Dämpfung wiederholt sich der Zyklus. Mit Rayleigh-Dämpfung klingt er ab, und
der Stab kommt bei der statischen Auslenkung zur Ruhe. Verwenden Sie eine
Querkontraktionszahl von null, damit der Stab wirklich eindimensional ist.

Verfeinern Sie bei jeder dieser Prüfungen das Netz mindestens einmal und
bestätigen Sie, dass das Ergebnis konvergiert und nicht driftet.

---

## 6. Einschränkungen

### Einschränkungen der Modellierung

- **Nur lineare Elastizität.** Kleine Dehnungen, kleine Verschiebungen,
  isotropes Material, keine Plastizität, kein Kontakt, keine großen
  Verformungen und kein Knicken. Die Ergebnisse sind nur aussagekräftig, solange
  die Struktur in ihrem linearen Bereich bleibt.
- **Nur Rayleigh-Dämpfung.** Die Dämpfungsmatrix ist `alpha_M*M + beta_K*K` pro
  Material, daher folgt der Dämpfungsgrad der Kurve aus 1.7 und kann nicht Mode
  für Mode eingestellt werden. Es gibt keine diskreten Dämpfer, keine
  frequenzunabhängige (hysteretische) Dämpfung und keine Dämpfungs-Randbedingung.
- **Nur implizite Zeitintegration.** Jeder Schritt ist eine lineare Lösung. Es
  gibt kein explizites Verfahren mit zentraler Differenz, daher benötigen sehr
  kurze Ereignisse wie ein ballistischer Aufprall viele teure Schritte.
- **Flächenelemente befinden sich im ebenen Verzerrungszustand.** Eine dünne, in
  ihrer eigenen Ebene belastete Platte ist steifer als eine Idealisierung im
  ebenen Spannungszustand. Modellieren Sie dünne Strukturen mit
  Volumenelementen, wenn der Unterschied wichtig ist.
- **Linienelemente sind Stäbe.** Es wird nur die Axialsteifigkeit `E*A`
  assembliert. Es gibt keine Biegung, keinen Schub und keine Torsion, wie auch
  immer der Elementtyp heißt, daher verhält sich ein mit Linienelementen
  modellierter Rahmen wie ein gelenkiges Fachwerk.
- **Die Vorspannung ist nur geometrisch.** Eine vorgespannte Modalanalyse wertet
  die lineare Steifigkeit auf dem verschobenen Netz neu aus. Sie fügt keinen
  geometrischen Steifigkeitsterm hinzu, daher werden die Versteifung eines
  gespannten Seils oder die Entfestigung einer gedrückten Strebe nicht erfasst.
- **Keine modalen Beteiligungsfaktoren oder effektiven Massen.** Der Löser gibt
  nur Eigenwerte und Eigenformen aus, daher gibt es keinen direkten Hinweis
  darauf, welche Moden für eine gegebene Anregungsrichtung von Bedeutung sind.
