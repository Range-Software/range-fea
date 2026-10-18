# Fluidströmung - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Strömungslöser von Range FEA - `RSolverFluid`,
der den Problemtyp **Inkompressible viskose Strömung** (*Incompressible viscous
flow*) bedient: die Gleichungen, die er löst, die Bedeutung jeder Eingabe, die
er akzeptiert, die Teile der grafischen Benutzeroberfläche, die ihn steuern, und
ein ausgearbeitetes Tutorial.

Die beiden auf dem Strömungsfeld gelösten Problemtypen haben eigene Handbücher:
[Wärmeübertragung in Fluiden](theory_fluid_heat.md) (`RSolverFluidHeat`) und
[Schadstoffausbreitung](theory_fluid_particle.md) (`RSolverFluidParticle`).

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - stationäre Strömung durch einen Kanal](#3-tutorial---stationäre-strömung-durch-einen-kanal)
4. [Überprüfung eines Modells](#4-überprüfung-eines-modells)
5. [Einschränkungen](#5-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - Geschwindigkeits- und Druckfeld

Die primären Unbekannten der **inkompressiblen viskosen Strömung** sind der
**Geschwindigkeits**vektor `v` in `m/s` und der **Druck** `p` in `Pa`, beide in
jedem Knoten. Jeder Knoten trägt **vier** Freiheitsgrade - `vx`, `vy`, `vz` und
`p` -, sodass ein Netz mit `N` Knoten ein System aus `4N` Gleichungen ergibt.

Damit ist ein Strömungsmodell der aufwendigste Physiktyp in Range FEA: vier
Unbekannte pro Knoten gegenüber drei bei einem Strukturmodell und einer bei
einem Wärmemodell, eine Matrix, die weder symmetrisch noch positiv definit ist,
und die nichtlineare Iteration aus Abschnitt 1.6 um jede lineare Lösung herum.

Die beiden begleitenden Problemtypen fügen jeweils einen Skalar hinzu, der auf
dem vom Strömungslöser erzeugten Strömungsfeld gelöst wird:

| Problemtyp | Löser | Primäre Unbekannte | Einheiten | Freiheitsgrade pro Knoten |
|---|---|---|---|---|
| Incompressible viscous flow | `RSolverFluid` | Geschwindigkeit und Druck | `m/s`, `Pa` | 4 |
| Heat transfer in fluids | `RSolverFluidHeat` | Temperatur | `K` | 1 |
| Contaminant dispersion | `RSolverFluidParticle` | Partikelkonzentration | `kg/m^3` | 1 |

Beide Begleiter **erfordern** eine Aufgabe *Incompressible viscous flow* im
selben Problem: Sie lesen das Geschwindigkeitsfeld aus deren Ergebnis und
berechnen es nie selbst. Jeder wird in einem eigenen Handbuch beschrieben -
[Wärmeübertragung in Fluiden](theory_fluid_heat.md) und
[Schadstoffausbreitung](theory_fluid_particle.md).

Der Standardzustand eines Knotens, den weder eine Bedingung noch ein vorheriges
Ergebnis berührt, ist Geschwindigkeit null und Druck null.

### 1.2 Grundgleichungen

Der Löser implementiert die **inkompressiblen Navier-Stokes-Gleichungen** für ein
newtonsches Fluid mit der Dichte `rho` und der dynamischen Viskosität `mu`.
Beide sind Materialeigenschaften, die pro Element bei seiner Temperatur
ausgewertet werden - siehe 1.9, woher diese Temperatur stammt und wie sie die
Strömung auftriebsbehaftet macht. Impuls- und Massenbilanz lauten

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

und bei deaktiviertem Zeitlöser entfällt der transiente Term, sodass die
stationäre Bilanz bleibt

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `v` | Geschwindigkeit | `m/s` | wird gelöst |
| `p` | Druck | `Pa` | wird gelöst |
| `rho` | Dichte | `kg/m^3` | Materialeigenschaft |
| `mu` | dynamische Viskosität | `kg/(m*s)`, also `Pa*s` | Materialeigenschaft |
| `g` | Erdbeschleunigung | `m/s^2` | Umgebungsbedingung Gravitational acceleration |

Der Druck ist hier keine thermodynamische Größe. Die Inkompressibilität lässt ihn
als Lagrange-Multiplikator übrig, der `div(v) = 0` erzwingt, was zwei praktische
Folgen hat, die man verinnerlichen sollte, bevor man ein Ergebnis liest:

- Nur **Druckdifferenzen** sind aussagekräftig. Ein Modell, in dem keine
  Bedingung irgendwo einen Druck festlegt, bestimmt das Druckfeld nur bis auf
  eine additive Konstante, und die Matrix ist in dieser Konstante singulär -
  siehe 1.7.
- Der Druck reagiert sofort im gesamten Gebiet. In dieser Formulierung gibt es
  keine Schallwelle und keine Schallgeschwindigkeit, sodass ein an einem Auslass
  aufgebrachter Druck innerhalb derselben Lösung am Einlass spürbar ist.

Die aus einer charakteristischen Länge `L` und einer Geschwindigkeit `V`
gebildete **Reynolds-Zahl**,

```
Re = rho * V * L / mu
```

ist die eine Zahl, die entscheidet, ob sich ein Strömungsmodell gutartig
verhalten wird. Unterhalb von etwa `Re = 1` ist der konvektive Term
vernachlässigbar und das Problem nahezu linear; bis zu einigen Tausend ist die
Strömung laminar und der Löser in seinem Element; weit darüber ist die reale
Strömung turbulent, und dieser Löser modelliert keine Turbulenz - siehe
Abschnitt 5.

### 1.3 Finite-Elemente-Diskretisierung und Stabilisierung

Geschwindigkeit und Druck werden auf jedem Element mit **denselben** linearen
Formfunktionen interpoliert - gleichordnige Interpolation `P1/P1` und `Q1/Q1`.
Diese Kombination erfüllt die Inf-Sup-Bedingung (Babuska-Brezzi) allein nicht
und würde ein schachbrettartiges Druckfeld erzeugen, daher ist die Formulierung
**stabilisiert**. Es werden drei Stabilisierungsterme hinzugefügt, jeder mit
eigenem Parameter auf Elementebene:

| Term | Zweck |
|---|---|
| **SUPG** - Streamline Upwind Petrov-Galerkin | stabilisiert den konvektiven Term, der sonst in einer konvektionsdominierten Strömung Oszillationen von Knoten zu Knoten erzeugen würde |
| **PSPG** - Pressure Stabilizing Petrov-Galerkin | macht gleichordnige Geschwindigkeit und Druck zulässig und beseitigt die schachbrettartige Druckmode |
| **LSIC** - Least Squares on Incompressibility Constraint | dämpft den Divergenzfehler und verbessert die Massenerhaltung Element für Element |

Die Parameter von SUPG und PSPG werden aus einer Element-Reynolds-Zahl gebildet

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`element length along the flow direction` = Elementlänge in
Strömungsrichtung, `element Reynolds number` = Element-Reynolds-Zahl, `for` =
für), sodass die Upwind-Gewichtung stark ist, wo das Element
konvektionsdominiert ist, und dort abklingt, wo es diffusionsdominiert ist. Der
Parameter von LSIC ist `|v| * h / 2`. Diese Parameter gehören zum
Strömungslöser; der Ausbreitungslöser verwendet einen eigenen, durch den
Zeitschritt begrenzten SUPG-Parameter, siehe Handbuch
[Schadstoffausbreitung](theory_fluid_particle.md).

Der SUPG-Parameter verwendet die **lokale** Elementgeschwindigkeit und
Elementlänge und passt sich daher Element für Element an. Der PSPG-Parameter
verwendet stattdessen eine **globale Strömungsgeschwindigkeit** und eine globale
Elementlängenskala, wodurch die Druckstabilisierung über das Netz gleichmäßig
bleibt. Die Strömungsgeschwindigkeit wird beim ersten Durchlauf jeder Lösung -
bei einem transienten Lauf jedes Zeitschritts - aus den Zuflussbedingungen
übernommen und nicht aus dem zu lösenden Feld,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

(`over inflow surfaces` = über die Zuflussflächen, `area of inflow surfaces` =
Fläche der Zuflussflächen), also die flächengewichtete mittlere
Zuflussgeschwindigkeit, mit `1 m/s` als Ersatzwert, wenn das Modell überhaupt
keine Zuflussbedingung hat. Die Elementlängenskala wird aus dem Elementvolumen
abgeleitet: `cbrt(6*V/pi)` für einen Tetraeder, `cbrt(V)` für einen Hexaeder.

**Unterstützte Elemente.** Die Strömungsgleichungen werden **nur auf
Volumenelementen** assembliert, und nur auf **linearen Tetraedern** (`TETRA1`)
und **linearen Hexaedern** (`HEXA1`). Jeder andere Volumenelementtyp hält den
Lauf mit einem Fehler an, der den Typ nennt. Flächenelemente sind nur beteiligt,
wo sie eine Bedingung *Pressure (implicit)* tragen; dort tragen sie eine
Druckspannung bei und sonst nichts. Punkt- und Linienentitäten tragen überhaupt
nichts bei - anders als in einem Wärmemodell sind sie keine niederdimensionalen
Leiter, sie sind einfach nicht vorhanden.

Dies ist der deutlichste praktische Unterschied zu den anderen Physiken in Range
FEA: Ein Fluidmodell muss ein **vernetztes Volumen** sein. Erzeugen Sie das
Tetraedernetz mit `Geometrie` -> `Volumen` -> `Tetraedernetz erzeugen`, bevor
Sie erwarten, dass eine Strömungslösung etwas bewirkt.

### 1.4 Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Wall | explizit | Punkt, Linie, Fläche | keine |
| Wall (frictionless) | explizit | Punkt, Linie, Fläche | keine |
| Velocity (inflow) | explizit | Fläche | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | explizit | Fläche | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | explizit | Fläche | Pressure `[Pa]` |
| Pressure (implicit) | natürlich, Spannung | Fläche | Pressure `[Pa]` |

**Wall** ist die Haftbedingung. In jedem Knoten der Entität werden alle drei
Geschwindigkeitskomponenten aus dem System entfernt und auf null gehalten. Sie
hat keine Komponenten - es gibt nichts einzugeben, das Aktivieren ist die
gesamte Einrichtung. Dies ist die Bedingung für jeden festen Rand, den das Fluid
berührt.

**Wall (frictionless)** ist die Gleit- oder Symmetriebedingung: Das Fluid darf
sich entlang der Fläche bewegen, aber nicht hindurch. Sie wird angewendet, indem
für jede Normale eines Flächenelements die **dominante globale Achse** bestimmt
und nur diese eine Geschwindigkeitskomponente auf null gehalten wird, während die
beiden anderen frei bleiben. Beachten Sie, was das bedeutet: Die Bedingung ist
nur für eine Fläche exakt, deren Normale entlang `x`, `y` oder `z` liegt. Auf
einer gegenüber den globalen Achsen geneigten Fläche schränkt sie die falsche
Richtung ein, und je näher die Normale an 45 Grad zwischen zwei Achsen liegt,
desto schlechter ist die Näherung. Verwenden Sie sie für die ebenen
Symmetrieebenen eines an den globalen Achsen ausgerichteten Modells und
anderswo *Wall*.

**Velocity (inflow)** schreibt eine Geschwindigkeit des angegebenen Betrags
entlang der **mittleren Normale** der Flächenentität vor, in das Fluid
gerichtet. Das Vorzeichen ergibt sich aus der Orientierung der Elemente der
Entität gegenüber dem dahinterliegenden Volumen, sodass ein positiver Wert in das
Gebiet eintretende Strömung bedeutet und ein negativer Wert austretende
Strömung. Sie geben eine Geschwindigkeit als Betrag ein, keinen Vektor - die
Richtung ergibt sich aus der Geometrie.

**Volumetric flow rate (inflow)** schreibt dasselbe stattdessen über den
gesamten Volumenstrom vor: Der Löser teilt den Volumenstrom durch die Fläche der
Entität und wendet die resultierende mittlere Geschwindigkeit entlang derselben
mittleren Normale an.

```
v = Q / A          A = area of the entity
```

(`area of the entity` = Fläche der Entität.) Greifen Sie zu ihr, wann immer die
Größe, die Sie tatsächlich kennen, ein Durchsatz ist - eine Pumpenleistung, ein
Kanal in `m^3/s` -, und zu *Velocity (inflow)*, wenn Sie die Geschwindigkeit
kennen. Beide halten in jedem Knoten der Entität alle drei
Geschwindigkeitskomponenten fest, sodass beide den Einlass zu einer
gleichförmigen Kolbenströmung machen; keine kann an der Einlassfläche ein
ausgebildetes Profil erzeugen. Geben Sie dem Modell eine Einlaufstrecke, wenn das
Profil dort wichtig ist.

**Pressure (explicit)** entfernt in jedem Knoten der Entität den
Druckfreiheitsgrad und hält ihn auf dem angegebenen Wert. Dies ist das
Druck-Gegenstück zu einer vorgeschriebenen Geschwindigkeit und eine Möglichkeit,
die additive Konstante aus Abschnitt 1.2 festzulegen.

**Pressure (implicit)** ist das natürliche Gegenstück: Statt den Knotendruck
festzulegen, bringt sie eine **Druckspannung** auf die Flächenelemente auf und
lässt die Geschwindigkeit dort frei, sodass das Fluid entscheidet, wie viel und
mit welchem Profil es ausströmt. Die Spannung enthält eine hydrostatische
Druckhöhe,

```
t = -( p + rho * g * h ) * n
```

wobei `h` die Höhe des Knotens ist, gemessen **entlang der Schwerkraftrichtung
vom tiefsten Punkt** des Modells, und `n` die Flächennormale. Die
Schwerkraftrichtung ist der Mittelwert der Umgebungsbedingungen *Gravitational
acceleration*, die den Flächen mit dieser Bedingung zugewiesen sind. Ohne
Schwerkraft verschwindet der Term der Druckhöhe, und die Spannung ist allein der
eingegebene Druck.

Dies ist die Auslassbedingung der Wahl in fast jedem Modell: Sie lässt das
Ausströmprofil sich ausbilden, statt es flach zu erzwingen, und die Eingabe von
`0` macht den Auslass zu einer Referenz bei Atmosphärendruck. *Pressure
(explicit)* ist die steifere, stärker vorschreibende Wahl.

Jede Komponente jeder dieser Bedingungen ist eine **Tabelle über der Zeit**,
sodass ein Einlass in den ersten Sekunden eines transienten Laufs hochgefahren
oder einem Volumenstrom ein Lastzyklus gegeben werden kann.

**Vorrang.** Geschwindigkeits- und Druckbedingungen sind unabhängig - sie
schränken unterschiedliche Freiheitsgrade ein, sodass eine Einlassfläche eine
Geschwindigkeitsbedingung und ein Auslass eine Druckbedingung tragen kann, ohne
sich gegenseitig zu stören. Unter Bedingungen derselben Art auf derselben
Entität setzt sich die zuletzt gelesene durch, und eine *Wall* setzt die
Geschwindigkeit ihrer Knoten auf null, nachdem alle anderen
Geschwindigkeitsbedingungen angewendet wurden. Eine Entität, die sowohl Einlass
als auch Wand ist, ist daher eine Wand.

### 1.5 Umgebungs- und Anfangsbedingungen

#### Umgebungsbedingungen

| Umgebungsbedingung | Komponenten | Wirkung |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | Volumenkraft `rho*g` auf die Entität und der Term der Druckhöhe von *Pressure (implicit)* |
| Temperature | Temperature `[K]` | Hintergrundtemperatur einer Aufgabe *Heat transfer in fluids*; vom Strömungslöser selbst nicht gelesen - siehe 1.9 |

**Gravitational acceleration** ist standardmäßig `(0, 0, -9.80665) m/s^2`, also
Schwerkraft entlang der negativen `Z`-Achse. Weisen Sie sie der Volumenentität
für die Volumenkraft und der Auslassfläche zu, damit die Druckhöhe von
*Pressure (implicit)* entlang der richtigen Richtung gemessen wird.

Die Schwerkraft geht als Volumenkraft `rho*g` in die Impulsgleichung ein. In
einem geschlossenen Gebiet gleichförmiger Dichte erzeugt ein gleichförmiges
Schwerefeld einen hydrostatischen Druckgradienten und keine Bewegung - was die
richtige Antwort ist. Wo die Dichte mit der Temperatur variiert, variiert die
Volumenkraft mit ihr, und der Unterschied treibt eine **Auftriebsströmung** an -
natürliche Konvektion, siehe 1.9. Das Druckergebnis enthält immer den
hydrostatischen Anteil `rho*(g . x)`; es wird nichts abgezogen.

#### Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Velocity | Anfangsgeschwindigkeitsfeld, beim ersten Lauf angewendet |
| Pressure | Anfangsdruckfeld, beim ersten Lauf angewendet |

Die Anfangsfelder für Temperatur und Konzentration der begleitenden
Problemtypen werden in deren eigenen Handbüchern beschrieben.

Anfangsbedingungen werden **nur beim ersten Lauf** gelesen; danach schreiten die
Felder vom vorherigen Ergebnis aus fort. Darauf beruht der Arbeitsablauf mit
Neustart aus Abschnitt 1.7: Ein mit *Löser neu starten / fortsetzen*
gestarteter transienter Lauf übernimmt das konvergierte stationäre Feld, statt
erneut aus der Ruhe zu starten.

Eine Randbedingung überschreibt eine Anfangsbedingung dort, wo beide
aufeinandertreffen.

### 1.6 Die nichtlineare Iteration

Der konvektive Term `(v . grad) v` ist in der Unbekannten quadratisch, daher
sind die Strömungsgleichungen **nichtlinear** und können nicht in einem
Durchlauf gelöst werden. Der Löser linearisiert sie und iteriert: Jeder
Durchlauf assembliert das Residuum der Impuls- und Massenbilanz beim aktuellen
Feld, löst nach einem **Inkrement** und addiert es,

```
J * dx = -R(x)
x <- x + dx
```

wobei `x` die Knotengeschwindigkeiten und -drücke enthält. Die linke Seite ist
eine genäherte und nicht die exakte Jacobi-Matrix, was das Verfahren zu einer
modifizierten Newton-Iteration macht: Sie konvergiert in der Nähe der Lösung
langsamer als das volle Newton-Verfahren, aber die Lösung, gegen die sie
konvergiert, wird durch das **Residuum** `R` bestimmt, also die diskretisierten
Navier-Stokes-Gleichungen selbst.

**Der Schritt wird gedämpft.** Da die Matrix nicht die exakte Ableitung des
Residuums ist, kann ein voller Schritt `dx` überschießen und das Feld weiter
von einer Lösung entfernt zurücklassen, als es begonnen hat - das Residuum des
nächsten Durchlaufs fällt dann höher aus als das vorherige. Der Löser achtet
genau darauf und wendet nur einen Anteil des berechneten Schritts an,

```
x <- x + omega * dx
```

beginnend mit `omega = 1`. Ein Durchlauf, der das Residuum verschlechtert hat,
halbiert ihn, bis zu einer Untergrenze von `0.1`; ein Durchlauf, der es
verbessert hat, lässt `omega` jeweils um ein Viertel wieder wachsen, sodass der
Rückzug schnell und die Erholung bedächtig erfolgt. Der geltende Wert wird im
Protokoll als `Relaxation` ausgegeben und bleibt bei einem Lauf, der nie
überschießt, bei `1` - ein gutartiges Strömungsmodell bemerkt den Mechanismus
nie. `omega` wird zu Beginn jeder Lösung auf `1` zurückgesetzt, bei einem
transienten Lauf also zu Beginn jedes Zeitschritts.

Daraus folgen zwei Konsequenzen, die den gesamten Arbeitsablauf eines
Strömungsmodells bestimmen:

- **Ein Durchlauf genügt nie.** Eine einzelne Iteration einer Strömungsaufgabe
  liefert, was auch immer das erste Inkrement gerade ergeben hat, und das ist
  keine Lösung von irgendetwas. Eine Strömungsaufgabe muss in eine
  **Aufgabengruppe** mit einer Anzahl von Iterationen eingebettet sein -
  Hunderte für einen stationären Lauf, Dutzende pro Zeitschritt für einen
  transienten.
- **Die Anzahl ist eine Obergrenze, kein Ziel.** Die Gruppe trägt außerdem
  einen **Konvergenzwert** und beendet ihre Iterationen, sobald die
  Strömungsaufgabe eine Konvergenz darunter meldet. Setzen Sie die Anzahl hoch
  genug für den schwierigsten Schritt und lassen Sie den Konvergenzwert jeden
  Schritt beenden, sobald er sich eingependelt hat.

Nach jeder Iteration schreibt der Löser vier Zahlen in die Konvergenzdatei und
ins Protokoll:

| Größe | Bedeutung |
|---|---|
| `Residual` | Norm des Residuenvektors, der Abstand von einer Lösung |
| `Convergence-R` | Änderung des Residuums seit der vorherigen Iteration |
| `Convergence-V` | relative Größe des Geschwindigkeitsinkrements, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | relative Größe des Druckinkrements, `\|\|dp\|\| / \|\|p\|\|` |

`Convergence-V` und `Convergence-P` werden mit dem Konvergenzwert verglichen.
Beide sind dimensionslos, und beide fallen, wenn sich die Iteration einpendelt:
Sie messen, wie groß der Schritt ist, den das Feld noch macht, im Verhältnis zur
Größe des Felds selbst, sodass ein Wert von `1e-5` bedeutet, dass sich die
Lösung in diesem Durchlauf um ein Hunderttausendstel ihrer selbst bewegt hat.

Ein kleiner Schritt allein ist jedoch noch keine Lösung. Ein nahezu singuläres
System - ein Modell ohne Druckreferenz oder ein Netz, das die geforderte
Reynolds-Zahl nicht tragen kann - macht winzige Schritte, weil es **sich nicht
bewegen kann**, nicht weil es angekommen ist, und sein Residuum bleibt, wo es
war, oder steigt. Die Prüfung hat daher zwei Teile, und die Gruppe hält nur an,
wenn beide erfüllt sind:

| Teil | Bedingung |
|---|---|
| Das Feld bewegt sich nicht mehr | `Convergence-V` und `Convergence-P` liegen beide unter dem Konvergenzwert |
| Das Feld ist tatsächlich gesunken | das Residuum ist auf ein **Zehntel** dessen gefallen, was es beim ersten Durchlauf dieser Lösung war |

Der zweite Teil ist fest und nicht konfigurierbar - er ist eine
Plausibilitätsprüfung des ersten, kein zweiter Stellknopf. Das Protokoll gibt
ihn als `Residual ratio` gegenüber seinem Zielwert aus, sodass erkennbar ist, wie
nahe ein Lauf, der nicht vorzeitig angehalten hat, dem Anhalten kam. In einem
transienten Lauf ist jeder Zeitschritt eine eigene Lösung, daher wird das
Verhältnis zu Beginn jedes Schritts neu gemessen.

`Bericht` -> `Konvergenz des Lösers` zeichnet alle vier Zahlen. Ein
konvergierter stationärer Lauf zeigt, wie das Residuum um mehrere
Größenordnungen fällt und dann abflacht, während `Convergence-V` und
`Convergence-P` monoton gegen null fallen. Ein Residuum, das hoch abflacht,
oszilliert oder steigt, bedeutet, dass der Lauf nicht konvergiert ist - und die
Gruppe hält bei einem solchen Lauf nicht vorzeitig an, was auch immer die
Inkremente tun.

Ein Konvergenzwert von `0` deaktiviert den vorzeitigen Abbruch und führt die
volle Anzahl aus, so wie sich der Löser verhielt, bevor es diesen Wert gab.
Greifen Sie dazu, wenn Sie pro Schritt eine feste Menge Arbeit wollen,
unabhängig davon, was das Residuum tut.

### 1.7 Stationäre Analyse

Bei deaktiviertem Zeitlöser iteriert der Löser die stationäre Impuls- und
Massenbilanz bis zu einem Fixpunkt. Das lineare System jeder Iteration ist nicht
symmetrisch und wird daher mit **GMRES** mit Jacobi-Vorkonditionierung gelöst
statt mit dem Verfahren der konjugierten Gradienten, das die symmetrischen
Physiken verwenden.

Ein stationäres Strömungsmodell benötigt mindestens:

- ein vernetztes **Volumen** mit zugewiesener Dichte und dynamischer Viskosität;
- **Wall** auf jedem festen Rand;
- eine **Zufluss**bedingung oder eine Druckdifferenz, um die Strömung
  anzutreiben;
- eine **Druckreferenz** - eine Fläche mit *Pressure (explicit)* oder *Pressure
  (implicit)*. Ohne sie ist der Druck nur bis auf eine Konstante bestimmt, und
  die Iteration pendelt sich nicht ein.

Ein Modell, dessen Ränder ausschließlich Wände und vorgeschriebene
Geschwindigkeiten sind, hat eine weitere Falle: Vorgeschriebener Zu- und Abfluss
müssen sich **ausgleichen**, da sich ein inkompressibles Fluid nicht anstauen
kann. 5 m^3/s Zufluss und 4 m^3/s Abfluss vorzuschreiben verlangt das
Unmögliche, und die Lösung konvergiert nicht. Den Auslass als *Pressure
(implicit)* zu belassen umgeht diese Frage vollständig, was der Hauptgrund ist,
diese Bedingung zu bevorzugen.

Der übliche Arbeitsablauf besteht darin, zuerst einen stationären Lauf
konvergieren zu lassen und ihn als Ausgangsfeld eines transienten Laufs zu
verwenden, weil ein aus der Ruhe gestarteter transienter Lauf seine ersten
Zeitschritte mit dem Anfahrvorgang statt mit der interessierenden Physik
verbringt.

### 1.8 Transiente Analyse

Bei aktiviertem Zeitlöser schreitet der Löser die instationären Gleichungen mit
einem Theta-Verfahren fort, wobei `alpha` der Koeffizient der Approximation des
Zeitschrittverfahrens ist:

| Approximation | `alpha` | Resultierendes Verfahren | Stabilität |
|---|---|---|---|
| Backward difference (stable) | `1` | voll impliziter Schritt | unbedingt |
| Central difference (accurate) | `0.5` | Crank-Nicolson | unbedingt, kann oszillieren |
| Forward difference (fast) | `0` | expliziter Schritt | bedingt |

**Backward difference** ist die Einstellung für ein Strömungsmodell. Die
nichtlineare Iteration innerhalb jedes Zeitschritts kostet die Zeit, und ein
implizites Verfahren erlaubt es, den Zeitschritt nach der Physik statt nach dem
Netz zu wählen.

Auch mit dem impliziten Verfahren ist der Zeitschritt eines Strömungsmodells
durch die Genauigkeit begrenzt, über die **Courant-Zahl**

```
C = |v| * dt / h
```

wobei `h` die Elementgröße in Strömungsrichtung ist. Eine Courant-Zahl nahe `1`
bedeutet, dass das Fluid pro Schritt etwa ein Element durchquert, was die
richtige Größenordnung zur Auflösung eines Übergangs ist; bei deutlich mehr
verschmiert der Löser den Transport, den er auflösen soll, unabhängig von der
Stabilität des Verfahrens. Wählen Sie `dt` anhand des Netzes und der erwarteten
Geschwindigkeit:

```
dt ~ h / |v|
```

Jeder Zeitschritt führt die volle nichtlineare Iteration aus Abschnitt 1.6 aus,
daher ist der Aufwand eines transienten Laufs die Anzahl der Zeitschritte mal
die Iterationen pro Schritt. Zehn bis fünfzig Iterationen pro Schritt sind ein
typischer Bereich, sobald das Strömungsfeld bereits aus einem stationären Lauf
konvergiert ist. Ein Schritt, dessen Residuum bis zum Ende seiner Iterationen
nicht gefallen ist, ist ein Schritt, dessen Ergebnis nicht konvergiert ist, und
der Fehler überträgt sich auf jeden folgenden Schritt.

### 1.9 Wärmeübertragung und Schadstoffausbreitung

Auf dem von diesem Löser erzeugten Strömungsfeld werden zwei Problemtypen
gelöst, jeder mit eigenem Handbuch:

- **Wärmeübertragung in Fluiden** (`RSolverFluidHeat`) transportiert die
  Temperatur mit der Strömung und koppelt an den Wärmelöser eines vernetzten
  Festkörpers - siehe [Wärmeübertragung in Fluiden](theory_fluid_heat.md);
- **Schadstoffausbreitung** (`RSolverFluidParticle`) transportiert eine
  Schadstoffkonzentration mit der Strömung - siehe
  [Schadstoffausbreitung](theory_fluid_particle.md).

Beide lesen die Geschwindigkeit aus dem Strömungsergebnis. Die
Schadstoffausbreitung verändert die Strömung nie. Die Wärmeübertragung in
Fluiden tut es, über die Materialeigenschaften.

#### Temperaturabhängige Eigenschaften und Auftrieb

Bei **jedem Durchlauf** wertet der Strömungslöser Dichte und dynamische
Viskosität jedes Elements aus den Materialtabellen bei der Elementtemperatur
aus, die der letzte Durchlauf der Wärmeübertragung hinterlassen hat. Ohne
Wärmeübertragungsaufgabe im Problem ist diese Temperatur überall der Standardwert
`293.15 K`. Die Umgebungsbedingung *Temperature* wird vom Strömungslöser nicht
direkt gelesen; sie erreicht die Strömung nur über eine
Wärmeübertragungsaufgabe.

Die Dichte in der Volumenkraft `rho(T)*g` macht die Strömung
auftriebsbehaftet: Warmes Fluid ist leichter als das umgebende Fluid und steigt,
kaltes Fluid sinkt. Natürliche Konvektion, thermische Fahnen und Schichtung
ergeben sich daher aus einer Gruppe aus einer Aufgabe *Incompressible viscous
flow* und einer Aufgabe *Heat transfer in fluids*, sofern das Material eine
**temperaturabhängige Dichtetabelle** hat. Die mitgelieferten Materialien
*Water* und *Air* haben eine. Ein Material, dessen Dichte ein einzelner Wert
ist, ergibt bei keiner Temperaturdifferenz Auftrieb.

Wie die beiden Aufgaben Daten austauschen:

- **Stationär.** Legen Sie beide Aufgaben in eine Gruppe, die Strömungsaufgabe
  zuerst, mit mehreren Iterationen und einem Konvergenzwert. Der erste
  Strömungsdurchlauf sieht die Anfangstemperatur - die Anfangsbedingung
  *Temperature* oder `293.15 K` -, hat also noch keinen Auftrieb. Ab der zweiten
  Iteration sieht jeder Strömungsdurchlauf die Temperatur des vorangegangenen
  Wärmedurchlaufs. Ein allein durch Auftrieb angetriebenes Modell benötigt genug
  Iterationen, damit sich beide gemeinsam einpendeln; eine Gruppe mit einer
  einzigen Iteration koppelt sie nie.
- **Transient.** Jeder Zeitschritt führt die Gruppe erneut aus, und jeder
  Strömungsdurchlauf sieht die neueste Temperatur. Bei einer Iteration pro
  Zeitschritt verwendet die Strömung die Temperatur des vorherigen Schritts -
  eine Verzögerung um einen Schritt, die akzeptabel ist, solange der Zeitschritt
  den thermischen Übergang auflöst. Mehrere Iterationen pro Schritt beseitigen
  sie.

Die Gruppe endet, wenn die Strömungsaufgabe konvergiert, da eine ungekoppelte
Aufgabe *Heat transfer in fluids* sich bei jedem Durchlauf als konvergiert
meldet. Eine Änderung der Temperatur verschiebt das Residuum der Strömung über
`rho(T)` und `mu(T)`, sodass eine konvergierte Strömung auch eine ist, die die
Temperatur nicht mehr verschiebt.

Die Formulierung behält `rho(T)` in jedem Term der Impulsbilanz und
`div(v) = 0` für die Masse bei. Für die kleinen Dichteunterschiede der
natürlichen Konvektion in einer Flüssigkeit oder in einem Gas über einige
Dutzend Kelvin ergibt dies dasselbe Ergebnis wie die Boussinesq-Näherung. Es ist
keine kompressible Formulierung: Die Wärmeausdehnung des Fluids geht nicht in
die Massenbilanz ein, daher liegt ein um Hunderte Kelvin erhitztes Gas außerhalb
ihres Anwendungsbereichs.

### 1.10 Abgeleitete Ergebnisse

| Ergebnis | Anwendbar auf | Erzeugt von | Bedeutung |
|---|---|---|---|
| Velocity `[m/s]` | Knoten | Strömung | das gelöste Geschwindigkeitsfeld, drei Komponenten |
| Pressure `[Pa]` | Knoten | Strömung | das gelöste Druckfeld |

Die Ergebnisse der begleitenden Problemtypen sind in deren eigenen Handbüchern
aufgeführt.

Die Geschwindigkeit wird als Knoten**vektor** gespeichert, sodass die
Registerkarte `Ergebnisse` ihren Betrag als Farbe, ihre Komponenten einzeln oder
Pfeile anzeigen kann. Zwei Anzeigewerkzeuge sind speziell für ein
Strömungsergebnis wissenswert:

- eine **Schnittebene** durch das Volumen, da die interessante Struktur einer
  Strömung im Inneren des Gebiets liegt und nicht an seiner Oberfläche;
- **Stromlinien**, erstellt über `Geometrie` -> `Stromlinie`, die das
  Geschwindigkeitsfeld nachzeichnen und der schnellste Weg sind, eine
  Rezirkulation oder ein Totwassergebiet zu erkennen.

Nach jedem Datensatz gibt das Löserprotokoll die Statistik von Geschwindigkeit
und Druck und die Werte an allen Überwachungspunkten aus.

### 1.11 Stationär oder transient?

| | Stationär | Transient |
|---|---|---|
| Löst | die stationäre Bilanz, iteriert | die instationären Gleichungen, in der Zeit fortgeschritten |
| Beantwortet | „wie sieht die ausgebildete Strömung aus“ | „wie kommt die Strömung dorthin, und was transportiert sie“ |
| Iterationen | Hunderte, in einer Aufgabengruppe | Dutzende pro Zeitschritt |
| Datensätze | einer | einer pro geschriebenem Zeitschritt |
| Benötigt eine Druckreferenz | ja | ja |

**Verwenden Sie eine stationäre Analyse für** das ausgebildete Strömungsfeld
eines Kanals, eines Verteilers oder eines Wärmetauscherdurchgangs, den
Druckabfall über ein Bauteil und als Ausgangsfeld für jeden transienten Lauf.
Hier beginnt fast jedes Strömungsmodell.

**Verwenden Sie eine transiente Analyse für** Anfahren und Abschalten, einen
pulsierenden oder zyklischen Einlass, Wirbelablösung und - vor allem - für die
**Schadstoffausbreitung**, die ein Transportproblem in der Zeit ist und für ein
Freisetzungsereignis keine sinnvolle stationäre Antwort hat.

Der zweistufige Arbeitsablauf des Tutorials *Schadstoffausbreitung in Fluiden*
ist der kanonische: die Strömung stationär konvergieren lassen und dann mit
aktiviertem Zeitlöser und aktivierter Ausbreitungsaufgabe neu starten - siehe
Handbuch [Schadstoffausbreitung](theory_fluid_particle.md).

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Umgebungsbedingungen`, `Material`,
`Ergebnisse`), im Menü **Problem** und im Panel **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Fügen Sie eine Aufgabe hinzu und wählen Sie
**Incompressible viscous flow** - „Steady-state and transient flow of newtonian
fluids“ (stationäre und transiente Strömung newtonscher Fluide).

Eine Strömungsaufgabe gehört in eine **Aufgabengruppe**, die die beiden Werte
trägt, die die nichtlineare Iteration aus 1.6 steuern. Doppelklicken Sie auf
einen der beiden, um ihn zu bearbeiten:

| Wert | Bedeutung |
|---|---|
| **Anzahl Iterationen** | die maximale Anzahl Durchläufe, die eine einzelne Lösung - oder ein einzelner Zeitschritt - benötigen darf |
| **Konvergenz** | die Gruppe beendet ihre Durchläufe, sobald sich die Strömungsaufgabe unter diesem Wert eingependelt hat **und** ihr Residuum gesunken ist; `0` führt immer die volle Anzahl aus |

Der Konvergenzwert ist standardmäßig `1e-5`, eine sichere relative Schrittweite
für ein Strömungsmodell. Eine Strömungsaufgabe außerhalb einer Gruppe, wo sie
genau einmal läuft, ist nicht sinnvoll - siehe 1.6.

Wissenswerte Kombinationen:

- *Incompressible viscous flow* allein, in einer Gruppe mit mehreren hundert
  Iterationen - ein stationäres Strömungsfeld;
- *Incompressible viscous flow* und *Contaminant dispersion* in einer Gruppe -
  Ausbreitung auf der Strömung, die klassische transiente Kombination, siehe
  [Schadstoffausbreitung](theory_fluid_particle.md);
- *Incompressible viscous flow* und *Heat transfer in fluids* in einer Gruppe -
  eine beheizte oder gekühlte Strömung, siehe
  [Wärmeübertragung in Fluiden](theory_fluid_heat.md).

Strömung schließt *Flow through porous media* und *Modal analysis* aus; die
beiden begleitenden Problemtypen erfordern jeweils, dass eine Strömungsaufgabe
vorhanden ist.

### 2.2 Registerkarte Problem

**Zeitlöser** - wird für alle drei Fluid-Problemtypen angezeigt, da alle
zeitabhängig sind. Lassen Sie ihn für einen stationären Strömungslauf
deaktiviert.

| Feld | Bedeutung |
|---|---|
| Aktivieren | schaltet das Zeitschrittverfahren ein und aus |
| Approximation | Theta des Zeitschrittverfahrens: rückwärts, zentral oder vorwärts, siehe 1.8 |
| Startzeit | erster Zeitwert in Sekunden; ein Neustart von einem stationären Ergebnis setzt dieses Ergebnis auf die Startzeit, ein Neustart eines unterbrochenen transienten Laufs setzt ab seinem letzten Schritt fort |
| Endzeit | `start time + time-step size * number of time-steps`, schreibgeschützt |
| Zeitschrittweite | `dt` in Sekunden, siehe die Courant-Empfehlung in 1.8 |
| Anzahl der Zeitschritte | wie viele Schritte berechnet werden |
| Ausgabefrequenz | schreibt alle N Schritte eine Ergebnisdatei; `0` schreibt nur den letzten Schritt |

Der Strömungs-Problemtyp hat kein eigenes Einstellungs-Gruppenfeld; das
Gruppenfeld *Einstellungen der Schadstoffausbreitung* wird im Handbuch
[Schadstoffausbreitung](theory_fluid_particle.md) beschrieben.
Alles andere wird pro Entität auf den Registerkarten der Bedingungen zugewiesen,
und die Anzahl der Iterationen - die hier wichtiger ist als irgendwo sonst in
Range FEA - befindet sich im Dialog für den Aufgabenablauf des Problems und
nicht auf dieser Registerkarte.

### 2.3 Registerkarte Material

Die Registerkarte `Material` listet das der ausgewählten Entität zugewiesene
Material auf. Der Strömungs-Problemtyp benötigt:

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Density | `kg/m^3` | `rho`, etwa `1000` für Wasser, `1.2` für Luft |
| Dynamic viscosity | `kg/(m*s)` | `mu`, etwa `1.0e-3` für Wasser, `1.8e-5` für Luft |

Die begleitenden Problemtypen führen ihre eigenen Eigenschaften in ihren
Handbüchern auf. Eine Entität, der eine von einem Problemtyp im Aufgabenablauf
benötigte Eigenschaft fehlt, wird nicht gelöst, und die Einstellungsprüfung
warnt vor dem Start des Laufs. Die mitgelieferte Materialdatenbank enthält
**Water** und **Air**, womit dieser Schritt meist bereits erledigt ist.

Beachten Sie, dass die gesuchte Viskosität die **dynamische** Viskosität `mu` in
`Pa*s` ist, nicht die kinematische Viskosität `nu = mu/rho` in `m^2/s`. Für
Wasser `1.0e-6` einzugeben, weil das die vertraute Zahl ist, ist der häufigste
Materialfehler in einem Strömungsmodell und macht das Fluid tausendmal weniger
viskos, als es sein sollte - was sich meist als Lauf zeigt, der nicht
konvergiert.

Jede Eigenschaft ist eine Tabelle über der Temperatur und wird bei der
Elementtemperatur des vorherigen Laufs ausgewertet.

### 2.4 Registerkarte Randbedingungen

Die Registerkarte ist oben in eine Verwaltungsliste der verfügbaren
Randbedingungen und unten in einen Editor für die ausgewählte Bedingung
aufgeteilt. Wählen Sie zuerst eine Entität im Baum `Modell` aus; die Verwaltung
bietet dann die für diesen Entitätstyp und die ausgewählten Problemtypen gültigen
Bedingungen an. Aktivieren Sie eine, um sie zuzuweisen, und bearbeiten Sie dann
ihre Komponenten im unteren Baum.

Es werden die Bedingungen aus Abschnitt 1.4 angeboten. Hinweise zu ihrer
Verwendung:

- **Wall** gehört auf jeden festen Rand und nimmt überhaupt keinen Wert an. Eine
  Flächenentität eines vernetzten Volumens ohne Bedingung ist *keine* Wand - sie
  ist ein uneingeschränkter Rand, was selten gemeint ist. Wände sind das Erste,
  was man bei einem sich seltsam verhaltenden Modell prüfen sollte.
- **Wall (frictionless)** ist für Symmetrieebenen gedacht, die an den globalen
  Achsen ausgerichtet sind. Auf einer geneigten Fläche schränkt sie die falsche
  Richtung ein - siehe 1.4.
- **Velocity (inflow)** und **Volumetric flow rate (inflow)** treiben beide
  Strömung entlang der mittleren Normale durch eine Fläche hinein. Wählen Sie
  diejenige, die zu der Zahl passt, die Sie tatsächlich haben. Ein negativer
  Wert treibt die Strömung in die andere Richtung, was eine Möglichkeit ist,
  einen Auslass anzugeben.
- **Pressure (implicit)** ist die bevorzugte Auslassbedingung: Sie lässt das
  Ausströmprofil sich ausbilden und macht, mit `0` eingegeben, den Auslass zur
  Druckreferenz. **Pressure (explicit)** legt stattdessen den Knotendruck fest.
- Die Bedingungen von *Heat transfer in fluids* und *Contaminant dispersion*
  erscheinen, sobald deren Aufgabe im Aufgabenablauf enthalten ist, und werden
  in deren eigenen Handbüchern beschrieben.
- Jede Komponente ist eine Tabelle über der Zeit. Klicken Sie auf
  **Zeitabhängige Werte bearbeiten**, um den Komponenteneditor zu öffnen, in dem
  Werte über der Zeit eingegeben werden; zwischen zwei angegebenen Zeitpunkten
  wird der Wert linear interpoliert, und nach dem letzten Zeitpunkt bleibt der
  letzte Wert erhalten. So werden ein Freisetzungsereignis, ein hochgefahrener
  Einlass oder ein Lastzyklus modelliert.

### 2.5 Anfangs- und Umgebungsbedingungen

Die Registerkarte `Anfangsbedingungen` bietet für ein Strömungsproblem
*Velocity* und *Pressure* an. Sie legen das Ausgangsfeld des ersten Laufs fest
und ermöglichen, dass ein transienter Lauf von etwas anderem als der Ruhe aus
startet.

Die Registerkarte `Umgebungsbedingungen` bietet *Gravitational acceleration* an,
standardmäßig `(0, 0, -9.80665) m/s^2`. Weisen Sie sie dem Volumen für die
Volumenkraft zu und jeder Fläche mit *Pressure (implicit)*, damit die
hydrostatische Druckhöhe dieser Bedingung entlang der richtigen Richtung
gemessen wird.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Strömungslöser verwendet den Eintrag **GMRES**, da seine Matrix nicht
symmetrisch ist.

Unterscheiden Sie die beiden verschachtelten Iterationen, bevor Sie hier etwas
ändern: Die Iterationen des **Matrixlösers** lösen ein linearisiertes System,
während die Iterationen der **Aufgabengruppe** die Nichtlinearität aus
Abschnitt 1.6 treiben. Ein Strömungsmodell, das nicht konvergiert, benötigt fast
immer mehr Iterationen der Aufgabengruppe, ein besseres Netz oder korrigierte
Randbedingungen - nicht eine höhere Anzahl von GMRES-Iterationen.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable - *Velocity* oder
*Pressure* für die Strömung. In einem transienten Lauf wird der Verlauf über
`Bericht` -> `Überwachungspunkte` angezeigt.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet die berechneten Variablen auf und steuert
die 3D-Ansicht. Die Geschwindigkeit ist ein Knotenvektor und der Druck ein
Knotenskalar.

Die Registerkarte `Datensätze` des Panels `Modell` listet die
Ergebnisdatensätze auf:

- **stationäre Analyse** - ein einziger Datensatz;
- **transiente Analyse** - ein Datensatz pro geschriebenem Zeitschritt. Das
  Animieren der Datensätze spielt die Strömung ab, und die Schaltfläche
  **Datensatz** unter dem Baum schreibt die Animation in eine Videodatei.

`Bericht` -> `Konvergenz des Lösers` zeichnet den in 1.6 beschriebenen
Konvergenzverlauf und ist der Bericht, den man nach jedem Strömungslauf zuerst
öffnen sollte. `Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige
Ausgabe des Lösers, einschließlich der Statistik von Geschwindigkeit und Druck
nach jedem Datensatz.

---

## 3. Tutorial - stationäre Strömung durch einen Kanal

**Ziel.** Das ausgebildete Strömungsfeld und den Druckabfall eines durch einen
Kanal getriebenen Fluids berechnen und bestätigen, dass das Ergebnis konvergiert
ist.

Dieses Tutorial setzt ein vernetztes Volumenmodell voraus - das mitgelieferte
Modell **Channel.tmsh** ist genau dieser Fall, und das Tutorial
*Schadstoffausbreitung in Fluiden* verwendet es ebenfalls. Wenn Sie ein eigenes
erstellen, zeichnen Sie einen Quader, tetraedrisieren Sie ihn und markieren Sie
die Einlassfläche, die Auslassfläche und die übrigen Seiten als drei separate
Flächenentitäten, damit ihnen Bedingungen zugewiesen werden können.

### Schritt 1 - Problem auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine **Aufgabengruppe** hinzu und setzen Sie ihre **Anzahl
   Iterationen** per Doppelklick auf den Wert auf `2000`.
3. Lassen Sie ihre **Konvergenz** auf dem Standardwert `1e-5`.
4. Fügen Sie innerhalb der Gruppe eine Aufgabe **Incompressible viscous flow**
   hinzu.
5. Bestätigen Sie mit `OK`.

Die beiden Werte sind der eigentliche Sinn dieses Schritts. Der Lauf benötigt
bis zu 2000 Durchläufe und hält an, sobald sich das Feld unter `1e-5`
eingependelt hat - siehe 1.6. Setzen Sie die Anzahl großzügig: Sie ist eine
Obergrenze, und normalerweise beendet der Konvergenzwert den Lauf.

### Schritt 2 - Netz erzeugen

`Geometrie` -> `Volumen` -> `Tetraedernetz erzeugen`.

Ein Strömungsmodell wird nur auf Volumenelementen gelöst. Ohne Volumennetz hat
der Löser nichts zu assemblieren.

### Schritt 3 - Material zuweisen

1. Wählen Sie die Volumenentität im Baum `Modell` aus.
2. Öffnen Sie die Registerkarte `Material` und weisen Sie **Water** zu, oder
   geben Sie ein:
   - **Density** = `1000` `kg/m^3`
   - **Dynamic viscosity** = `1.0e-3` `kg/(m*s)`

Prüfen Sie, dass die Viskosität die dynamische in `Pa*s` ist und nicht die
kinematische - siehe 2.3.

### Schritt 4 - Analyse stationär belassen

Öffnen Sie die Registerkarte `Problem` und lassen Sie im Gruppenfeld
*Zeitlöser* **Aktivieren** deaktiviert.

### Schritt 5 - Wände zuweisen

1. Wählen Sie die Flächenentität aus, die die festen Ränder des Kanals bedeckt.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie **Wall**.

Es ist kein Wert einzugeben. Dies ist die Haftbedingung, und sie bewirkt, dass
die Strömung ein Profil ausbildet, statt als Kolben hindurchzugleiten.

### Schritt 6 - Strömung antreiben

1. Wählen Sie die Einlass-Flächenentität aus.
2. Aktivieren Sie **Volumetric flow rate (inflow)** und geben Sie den Durchsatz
   in `m^3/s` ein - oder aktivieren Sie stattdessen **Velocity (inflow)** und
   geben Sie eine Geschwindigkeit in `m/s` ein.

Schätzen Sie, bevor Sie weitermachen, die Reynolds-Zahl aus der mittleren
Geschwindigkeit und der Kanalbreite ab:

```
Re = rho * V * L / mu
```

Unterhalb einiger Tausend liegt das Modell im Bereich, den dieser Löser
beherrscht. Weit darüber ist damit zu rechnen, dass der Lauf nicht konvergiert;
lesen Sie Abschnitt 5, bevor Sie Zeit darauf verwenden.

### Schritt 7 - Auslass öffnen

1. Wählen Sie die Auslass-Flächenentität aus.
2. Aktivieren Sie **Pressure (implicit)** und setzen Sie **Pressure** = `0`
   `Pa`.

Dieser Schritt macht das Modell lösbar. Er gibt dem Druckfeld seine Referenz und
lässt das Ausströmprofil sich ausbilden, statt es flach zu erzwingen. Ein Modell
mit Einlass und Wänden, aber ohne Druckbedingung, hat einen singulären Druck und
konvergiert nicht.

### Schritt 8 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Die Einstellungsprüfung meldet fehlende Materialien oder fehlende
   Randbedingungen vor dem Start des Laufs.
3. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.

Das kann eine Weile dauern - ein Strömungsmodell jeder Größe ist echte Arbeit,
und das Protokoll gibt einen Block pro Iteration aus. Achten Sie im Protokoll
auf `Convergence target`: Der Durchlauf, der es erreicht, wird markiert, und die
Gruppe meldet dann *All sub-tasks have converged* und hält an.

### Schritt 9 - Konvergenz prüfen

`Bericht` -> `Konvergenz des Lösers`.

Tun Sie dies, bevor Sie irgendein Bild betrachten. Das Residuum sollte um
mehrere Größenordnungen fallen und dann abflachen, und `Convergence-V` und
`Convergence-P` sollten sich null nähern. Fällt das Residuum bei der letzten
Iteration noch steil, war der Lauf zu kurz - erhöhen Sie die Anzahl der
Iterationen und starten Sie neu. Oszilliert oder steigt es, ist das Ergebnis
keine Lösung, und Abschnitt 5 ist die richtige Anlaufstelle.

### Schritt 10 - Ergebnisse untersuchen

- Registerkarte `Ergebnisse`: Zeigen Sie **Velocity** an. Das Profil über den
  Kanal sollte glatt sein, an den Wänden null und nahe der Mitte am schnellsten.
- Legen Sie eine Schnittebene durch das Volumen - die interessante Struktur
  einer Strömung liegt im Inneren des Gebiets, nicht an seiner Oberfläche.
- Zeigen Sie **Pressure** an. Er sollte vom Einlass zum Auslass monoton fallen
  und am Auslass, wo Sie ihn gesetzt haben, `0` anzeigen.
- Erstellen Sie **Stromlinien** mit `Geometrie` -> `Stromlinie`, um
  Rezirkulation oder Totwassergebiete sofort zu sehen.
- Prüfen Sie die Massenbilanz: Die mittlere Auslassgeschwindigkeit mal die
  Auslassfläche sollte dem am Einlass vorgeschriebenen Volumenstrom
  entsprechen. Dies ist die schnellste globale Prüfung, dass das Modell tut, was
  Sie verlangt haben - siehe Abschnitt 4.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Das Residuum fällt nie | nirgends eine Druckbedingung, daher ist der Druck unbestimmt |
| Das Residuum oszilliert oder steigt | die Reynolds-Zahl ist für das Netz zu hoch, oder die Viskosität wurde als kinematische eingegeben |
| Die Strömung gleitet ohne Profil hindurch | keine Bedingung *Wall* auf den festen Rändern |
| Das Ergebnis sieht aus wie die erste Iteration | die Aufgabe wurde nicht in eine Aufgabengruppe gelegt und lief daher einmal |
| Der Lauf hielt weit vor Erreichen der Iterationsanzahl an | er ist konvergiert - sowohl die Inkremente als auch das Residuum haben die Prüfung aus 1.6 erfüllt |
| Der Lauf hält nie vorzeitig an, wie locker der Konvergenzwert auch ist | das Residuum sinkt nicht - lesen Sie `Residual ratio` im Protokoll und betrachten Sie das Strömungsfeld als nicht konvergiert |
| *Failed to calculate element scales. Unsupported element type* | das Netz enthält andere Volumenelemente als lineare Tetraeder oder Hexaeder |
| Es wurde überhaupt nichts gelöst | es wurde kein Volumennetz erzeugt, oder dem Material fehlen Dichte oder Viskosität |
| Vorgeschriebener Zu- und Abfluss gleichen sich nicht aus | ein inkompressibles Fluid kann sich nicht anstauen - verwenden Sie am Auslass *Pressure (implicit)* |

---

## 4. Überprüfung eines Modells

Der Löser hat keine eingebaute Verifikationssuite, daher lohnt es sich, ein neues
Modell an etwas zu validieren, das Sie von Hand berechnen können, bevor Sie ihm
vertrauen. Vier günstige Prüfungen, nach Nützlichkeit geordnet:

**Massenbilanz.** Für jedes konvergierte inkompressible Modell gilt: Was
hineingeht, muss herauskommen:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(`over the inlets` = über die Einlässe, `over the outlets` = über die Auslässe.)
Lesen Sie die mittlere Geschwindigkeit am Auslass aus der Statistik im
Löserprotokoll ab, multiplizieren Sie sie mit der Auslassfläche und vergleichen
Sie mit dem vorgeschriebenen Zufluss. Diese eine globale Prüfung deckt einen
nicht konvergierten Lauf, eine fehlende Wand und eine Randbedingung auf der
falschen Entität auf, und sie kostet nichts.

**Hydrostatik.** Schalten Sie jeden Zufluss ab, lassen Sie das Gebiet mit Wänden
geschlossen, weisen Sie eine Umgebungsbedingung *Gravitational acceleration* zu
und lösen Sie. Das Fluid muss zur Ruhe kommen, und der Druck muss linear mit der
Tiefe variieren:

```
p(h) = p_ref + rho * g * h
```

Jede verbleibende Bewegung ist numerisch. Dies validiert die
Schwerkraft-Volumenkraft, die Druckreferenz und die Einheiten des Materials in
einem Lauf und ist das schnellste erste Modell, das man in einer neuen Umgebung
aufbauen kann.

**Ebene Poiseuille-Strömung.** Für eine stationäre laminare Strömung zwischen
zwei parallelen Platten im Abstand `H` ist das Profil parabolisch, die
Geschwindigkeit auf der Mittellinie das `1.5`-fache des Mittelwerts, und der
Druckgradient beträgt

```
dp/dx = 12 * mu * V_mean / H^2
```

Modellieren Sie einen geraden Kanal mit Wänden oben und unten, einem
vorgeschriebenen Zufluss und einem Auslass mit *Pressure (implicit)*, und
vergleichen Sie die berechnete Geschwindigkeit auf der Mittellinie und den
Druckabfall damit. Dies ist die schärfste verfügbare Prüfung: Sie validiert den
viskosen Term, die Wand mit Haftbedingung und die Einheiten der Viskosität
zugleich. Geben Sie dem Modell genug Einlaufstrecke, damit sich das Profil vor
der Messung ausbilden kann, da die Einlassbedingung ein flaches Profil aufprägt.

**Hagen-Poiseuille-Strömung.** Dieselbe Prüfung in einem runden Rohr mit dem
Durchmesser `D` und der Länge `L`, wobei die Geschwindigkeit auf der Mittellinie
das `2`-fache des Mittelwerts beträgt und

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

gültig, solange die Strömung laminar ist, also unterhalb von etwa `Re = 2300`.

Verfeinern Sie bei jeder dieser Prüfungen das Netz mindestens einmal und
bestätigen Sie, dass das Ergebnis konvergiert und nicht driftet - und
bestätigen Sie, dass der Konvergenzverlauf abgeflacht ist, bevor Sie überhaupt
etwas vergleichen. Ein nicht konvergiertes Strömungsmodell besteht aus demselben
Grund keine dieser Prüfungen.

---

## 5. Einschränkungen

### Einschränkungen der Modellierung

- **Kein Turbulenzmodell.** Der Löser integriert die laminaren
  Navier-Stokes-Gleichungen. Die Terme SUPG, PSPG und LSIC sind numerische
  Stabilisierung, keine Turbulenzschließung - sie bilden weder die Durchmischung
  noch die zusätzliche Dissipation einer turbulenten Strömung ab. Oberhalb von
  etwa `Re = 2300` in einem Rohr ist die reale Strömung turbulent, und dieser
  Löser modelliert sie nicht: Der Lauf mag trotzdem konvergieren, aber das
  Ergebnis beschreibt eine laminare Strömung, die es nicht gibt. Es gibt kein
  `k`-`e`-Modell, keine Wandfunktion und keine Grobstruktur-Formulierung.
- **Nur inkompressibel und newtonsch.** Die Massenbilanz ist `div(v) = 0`, und
  die viskose Spannung ist linear in der Verzerrungsrate. Dichte und Viskosität
  dürfen von der Temperatur abhängen, aber nicht vom Druck. Kompressible
  Strömung, Effekte der Mach-Zahl, freie Oberflächen, mehrere Phasen und
  nichtnewtonsches Verhalten liegen außerhalb der Formulierung.
- **Auftrieb ohne Ausdehnung des Fluids.** Eine temperaturabhängige Dichte
  treibt über `rho(T)*g` die natürliche Konvektion an (siehe 1.9), aber die
  Wärmeausdehnung geht nie in die Massenbilanz ein. Das ist für die kleinen
  Dichteunterschiede der natürlichen Konvektion genau, nicht jedoch für ein um
  Hunderte Kelvin erhitztes Gas. Die Auftriebsströmung ist laminar wie jede
  andere Strömung hier - eine Fahne oder ein Hohlraum oberhalb einer
  Rayleigh-Zahl von etwa `1e8` bis `1e9` ist in Wirklichkeit turbulent.
- **Die Temperatur erreicht die Strömung nur über eine
  Wärmeübertragungsaufgabe.** Ohne eine solche werden die Materialeigenschaften
  bei `293.15 K` ausgewertet; die Umgebungsbedingung *Temperature* allein ändert
  sie nicht.
- **Die Schadstoffausbreitung ist einseitig.** Die Strömung trägt den Schadstoff;
  die Konzentration verändert die Strömung nie. Die Einschränkungen der
  begleitenden Problemtypen sind in deren eigenen Handbüchern aufgeführt.
- **Nur Volumenelemente, nur lineare Tetraeder und Hexaeder.** Ein
  Strömungsmodell muss ein vernetztes Volumen aus Elementen `TETRA1` oder
  `HEXA1` sein; alles andere hält den Lauf an. Flächen sind nur beteiligt, um
  eine Spannung *Pressure (implicit)* zu tragen, und Punkt- und Linienentitäten
  tragen nichts bei.
- **Die reibungsfreie Wand ist achsenausgerichtet.** Sie schränkt die einzelne
  dominante globale Komponente jeder Elementnormale ein und ist daher nur auf
  einer Fläche exakt, deren Normale entlang `x`, `y` oder `z` liegt.
- **Einlässe sind gleichförmig.** *Velocity (inflow)* und *Volumetric flow rate
  (inflow)* prägen beide ein flaches Profil entlang der mittleren
  Flächennormale auf. Ein ausgebildetes Einlassprofil muss durch eine
  Einlaufstrecke im Netz erzeugt werden.

### Einschränkungen des Lösers

- **Die Iteration ist ein modifiziertes Newton-Verfahren**, daher ist die
  Konvergenz in der Nähe der Lösung linear statt quadratisch. Rechnen Sie bei
  einem stationären Lauf mit Hunderten von Iterationen und setzen Sie die Anzahl
  der Iterationen der Aufgabengruppe so hoch, dass der Konvergenzwert und nicht
  die Anzahl den Lauf beendet. Die Dämpfung aus 1.6 hält einen solchen Lauf auf
  Abstiegskurs, kann ihn aber nicht quadratisch machen.
- **Das Verfahren mit zentraler Differenz des Strömungslösers ist kein
  Crank-Nicolson-Verfahren.** Die Theta-Gewichtung wird auf die Matrix, aber
  nicht auf das Residuum angewendet, sodass *Central difference (accurate)* ein
  Residuum des Rückwärts-Euler-Verfahrens mit einer Matrix halber Steifigkeit
  löst. Das Ergebnis ist das der Rückwärtsdifferenz, nur langsamer erreicht.
  Verwenden Sie für die Strömung *Backward difference (stable)*, bis dies
  behoben ist. Der Löser für Schadstoffausbreitung ist nicht betroffen - er ist
  linear, und sein Verfahren ist ein echtes Crank-Nicolson-Verfahren.
- **Die Stabilisierungsparameter werden nicht differenziert.** `Tsupg`,
  `Tpspg`, `Tlsic` und die Elementlänge hängen alle von der Geschwindigkeit ab
  und gehen alle als beim aktuellen Feld ausgewertete Konstanten in die Matrix
  ein. Das ist bei einer stabilisierten Formulierung übliche Praxis und einer der
  Gründe, warum die Jacobi-Matrix nicht die exakte ist.
- **Der Residuumsteil der Konvergenzprüfung ist fest.** Eine Lösung gilt erst
  als konvergiert, wenn ihr Residuum auf ein Zehntel dessen gefallen ist, was es
  zu Beginn dieser Lösung war, und dieses Zehntel lässt sich nicht ändern. Es ist
  bewusst konservativ: Ein langsam konvergierendes Modell erfüllt es
  möglicherweise nie und führt dann seine volle Iterationsanzahl aus. Der Wert
  `Residual ratio` im Protokoll zeigt, wie nahe ein solcher Lauf kam.
- **Eine Druckreferenz ist erforderlich.** Ohne Fläche mit *Pressure (explicit)*
  oder *Pressure (implicit)* ist der Druck nur bis auf eine Konstante bestimmt,
  und die Iteration pendelt sich nicht ein.
- **Vorgeschriebene Strömungen müssen sich ausgleichen.** Bei einem Modell,
  dessen Ränder alle Wände und vorgeschriebene Geschwindigkeiten sind, muss der
  Zufluss genau dem Abfluss entsprechen; ein inkompressibles Fluid kann die
  Differenz nicht aufnehmen.
- **Aufwand.** Vier Unbekannte pro Knoten, eine nicht symmetrische Matrix und
  eine nichtlineare Iteration um jede lineare Lösung machen ein Strömungsmodell
  mit großem Abstand zum aufwendigsten Problemtyp in Range FEA. Dimensionieren
  Sie das Netz entsprechend und lassen Sie ein stationäres Feld konvergieren,
  bevor Sie einen transienten Lauf starten.
