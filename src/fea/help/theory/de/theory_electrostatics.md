# Elektrostatik - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Elektrostatik-Löser (`RSolverElectrostatics`) von
Range FEA, der den Problemtyp **Elektrostatik** (*Electro-statics*) bedient: die
Gleichungen, die er löst, die Bedeutung jeder Eingabe, die er akzeptiert, die
Teile der grafischen Benutzeroberfläche, die ihn steuern, und zwei
ausgearbeitete Tutorials.

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - Potential und Strom in einem Leiter](#3-tutorial---potential-und-strom-in-einem-leiter)
4. [Tutorial - Widerstandserwärmung](#4-tutorial---widerstandserwärmung)
5. [Überprüfung eines Modells](#5-überprüfung-eines-modells)
6. [Einschränkungen](#6-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - das elektrische Potentialfeld

Die primäre Unbekannte ist das **elektrische Potential** `V` in jedem Knoten, in
Volt. Jeder Knoten trägt einen einzigen Freiheitsgrad, sodass ein Netz mit `N`
Knoten ein System aus `N` Gleichungen ergibt. Das elektrische Feld, die
Stromdichte und alles daraus Abgeleitete werden nach der Lösung aus dem
Potentialfeld gewonnen.

Das Potential ist ein Skalar, wodurch ein elektrostatisches Modell genauso
günstig ist wie ein Wärmemodell auf demselben Netz: eine Unbekannte pro Knoten
statt drei und eine Matrix mit einem Neuntel der Einträge eines
Strukturmodells.

Das Standardpotential eines Knotens, den weder eine Bedingung noch ein
vorheriges Ergebnis berührt, ist `0 V`.

Ein Potentialfeld ist nur **bis auf eine additive Konstante** definiert. Nichts in
den Gleichungen legt das Nullniveau fest, daher benötigt jedes Modell mindestens
eine Randbedingung *Electric potential* - siehe 1.7. Das ist das
elektrostatische Gegenstück zu einer ungelagerten Struktur oder zu einem
Wärmemodell ohne vorgeschriebene Temperatur.

### 1.2 Grundgleichung

Der Löser implementiert die **elektrostatische Feldgleichung** - die
Poisson-Gleichung für das Potential eines linearen, isotropen Dielektrikums:

```
div( e0 * er * grad(V) ) = -rho
```

und ohne jegliche Ladungsdichte reduziert sie sich auf die Laplace-Gleichung

```
div( e0 * er * grad(V) ) = 0
```

Das elektrische Feld ist der negative Gradient des Potentials, und die
Stromdichte folgt aus dem Ohmschen Gesetz:

```
E = -grad(V)
J = sigma * E
```

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `V` | elektrisches Potential | `V` | wird gelöst |
| `rho` | Ladungsdichte | `C/m^3`, siehe 1.5 | Bedingung Charge density |
| `e0` | elektrische Feldkonstante | `F/m` | eingebaut, `8.854187817e-12` |
| `er` | relative Permittivität | `-` | Materialeigenschaft |
| `sigma` | elektrische Leitfähigkeit | `S/m` | Materialeigenschaft |
| `E` | elektrisches Feld | `V/m` | abgeleitet, siehe 1.8 |
| `J` | Stromdichte | `A/m^2` | abgeleitet, siehe 1.8 |

Die elektrische Feldkonstante ist eine Konstante des Lösers, keine
Materialeigenschaft und keine Einstellung - nur die **relative** Permittivität
wird mit dem Material eingegeben.

**Das Feld wird von der Permittivität angetrieben, nicht von der
Leitfähigkeit.** Das ist das Wichtigste, was man über diesen Löser wissen muss.
Die Steifigkeitsmatrix wird allein aus `e0*er` aufgebaut; die Leitfähigkeit geht
nie in das Gleichungssystem ein. Sie wird erst danach verwendet, um das
berechnete Feld in eine Stromdichte, einen spezifischen Widerstand und eine
Joulesche Wärme umzurechnen. Die Folge wird in 1.9 ausgeführt: In einem Körper
aus einem Material macht es keinen Unterschied, in einem Körper aus **zwei**
Materialien teilt sich das Potential jedoch im Verhältnis der Permittivitäten
auf und nicht im Verhältnis der Widerstände.

**Erforderliche Materialeigenschaften.** Ein elektrostatisches Problem benötigt
**drei** davon:

| Eigenschaft | Einheiten | Verwendet für |
|---|---|---|
| Relative permittivity | `-` | die Steifigkeitsmatrix |
| Electrical conductivity | `S/m` | Stromdichte, spezifischer Widerstand, Joulesche Wärme |
| Density | `kg/m^3` | nichts - erforderlich, nie gelesen |

Eine Entität, deren Material eine der drei fehlt, wird **nicht gelöst**, obwohl
die Dichte von diesem Löser überhaupt nicht gelesen wird. Die
Einstellungsprüfung warnt vor dem Start des Laufs davor. Die einzige Ausnahme
ist eine Entität mit einer elektrostatischen Randbedingung: Diese Entität wird
unabhängig von ihrem Material in das System aufgenommen, und ihre Permittivität
und Leitfähigkeit sind dann einfach null.

Eine Permittivität von null ist kein harmloser Standardwert - ein Element mit
`er = 0` hat keine Steifigkeit, trägt also nichts zum System bei und lässt
seine Knoten frei schweben. Soll eine Entität gelöst werden, geben Sie ihr eine
echte Permittivität: `1` für Vakuum oder, hinreichend genau, für Luft.

Jede Eigenschaft ist eine Tabelle über der Temperatur, eine
Elektrostatik-Aufgabe allein hat jedoch kein Temperaturfeld, bei dem sie
ausgewertet werden könnte, daher wird jede Tabelle bei `293.15 K` ausgewertet.
Die Tabellen werden nur berücksichtigt, wenn eine Aufgabe *Heat transfer* weiter
vorne im selben Aufgabenablauf Elementtemperaturen berechnet hat, die mit den
nachfolgenden Aufgaben geteilt werden.

Die relative Permittivität ist **dimensionslos** - sie wird in der
Benutzeroberfläche wie der Emissionsgrad und die Querkontraktionszahl als `N/A`
angezeigt -, und der Wert wird als einfacher Faktor von `e0` verwendet. Geben
Sie `1` für Vakuum, `1.00059` für Luft, `2.1` für PTFE und etwa `4` für Glas ein.

### 1.3 Finite-Elemente-Diskretisierung

Die schwache Form der Feldgleichung ergibt für jedes Element

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(`stiffness` = Steifigkeit, `charge source` = Ladungsquelle), wobei `B` aus den
Ableitungen der Formfunktionen aufgebaut ist und `N` die Formfunktionen selbst
sind. Jede Elementmatrix ist für ein Element mit `n` Knoten `n x n` groß - eine
Zeile pro Knoten, nicht drei. Es gibt keine Massen- oder Kapazitätsmatrix: Das
Problem ist statisch, und 1.10 erklärt, was das für den Aufgabenablauf
bedeutet.

**Elementmaße.** Wie überall in Range FEA erhalten niederdimensionale Entitäten
ein geometrisches Maß, damit sie reale Körper darstellen können:

| Entität | Maß | Wo es gesetzt wird |
|---|---|---|
| Volumen | 1 | - |
| Fläche | Flächendicke | geometrische Eigenschaften der Entität |
| Linie | Querschnittsfläche der Linie | geometrische Eigenschaften der Entität |
| Punkt | Punktvolumen | geometrische Eigenschaften der Entität |

Eine **Fläche** führt das Feld in ihrer eigenen Ebene, durch einen Querschnitt
gleich ihrer Dicke. Es gibt keinen Potentialgradienten durch die Dicke: Ein
Flächenelement ist einen Knoten dick und daher konstruktionsbedingt über seine
Dicke äquipotential. Eine Fläche mit **Dicke null** hat daher überhaupt keine
Steifigkeit und verbindet nichts - anders als beim Wärmelöser, wo eine Fläche
mit Dicke null weiterhin Konvektion trägt, ist eine Fläche mit Dicke null hier
einfach wirkungslos. Geben Sie einer Flächenentität immer dann eine Dicke, wenn
sie leiten soll, und denken Sie daran, dass eine Seite mit Dicke null auch
**keine abgeleiteten Ergebnisse** erhält (1.8).

Eine **Linie** führt das Feld entlang ihrer eigenen Richtung, durch ihre
Querschnittsfläche. Ein **Punkt** hat keine Richtung, entlang derer er es führen
könnte, und trägt daher **überhaupt keine Steifigkeit** bei: Er enthält nur
eine Ladungsquelle. Ein Punktelement, dessen Knoten mit keinem anderen
berechenbaren Element geteilt wird, erzeugt daher eine leere Zeile in der
Matrix, die der Löser mit konjugierten Gradienten nicht verarbeiten kann - ein
Punkt ist ein Ort, um Ladung einzubringen oder ein Potential vorzuschreiben,
kein Leiter.

### 1.4 Quellen und Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Electric potential | explizit (Dirichlet) | Punkt, Linie, Fläche, Volumen | Electric potential `[V]` |
| Charge density | natürlich, Quelle | Punkt, Linie, Fläche, Volumen | Charge density, siehe 1.5 |

Diese beiden bilden das gesamte Vokabular des Problemtyps. Es gibt keinen
vorgeschriebenen Fluss, keine Flächenladung, keine Fernfeld- oder offene
Randbedingung und keine Symmetriebedingung - siehe Abschnitt 6.

**Electric potential** ist die einzige explizite Bedingung des
elektrostatischen Problems. Jeder Knoten der Entität, der sie zugewiesen ist,
wird aus dem Gleichungssystem entfernt und auf dem angegebenen Wert gehalten;
sein Beitrag wird auf die rechte Seite der benachbarten Gleichungen übertragen.
Jedes Modell benötigt mindestens eine davon. Eine neu zugewiesene Bedingung
beginnt bei `1 V`.

Wo sich Entitäten überlappen, **gewinnt die niederdimensionale Entität**: Ein auf
einer Fläche vorgeschriebenes Potential überschreibt in den gemeinsamen Knoten
eines, das auf dem dahinterliegenden Volumen vorgeschrieben ist, eine Linie
überschreibt eine Fläche, und ein Punkt überschreibt alles. Potentiale werden
nicht kombiniert - es gibt pro Knoten nur einen zu haltenden Freiheitsgrad,
daher setzt sich einfach die zuletzt gelesene Bedingung durch.

**Charge density** ist der Quellterm der Poisson-Gleichung. Anders als bei der
Bedingung *Heat* des Wärmelösers wird ihr Wert **nicht** über das Maß der
Entität verteilt: Die eingegebene Zahl wird unverändert als Dichte auf jedes
Element der Entität angewendet. Die in eine Volumenentität eingebrachte
Gesamtladung ist daher der Wert multipliziert mit dem Volumen dieser Entität,
und sie wächst, wenn die Entität vergrößert wird - sie bleibt nicht fest, wie es
ein Wert *Heat* in Watt tut.

| Entität | Bedeutung des Werts |
|---|---|
| Volumen | Ladung pro Volumeneinheit |
| Fläche | Ladung pro Volumeneinheit des dickengewichteten Querschnitts |
| Linie | Ladung pro Volumeneinheit des Querschnitts |
| Punkt | eine Knoten-Ladungsquelle |

Beide Komponenten akzeptieren eine Tabelle über der Zeit. Da das Problem selbst
statisch ist, wirkt ein Zeitverlauf nur, wenn eine andere, zeitabhängige Aufgabe
den Aufgabenablauf teilt - und, wie 1.10 erklärt, ist diese Kombination selbst
eingeschränkt.

### 1.5 Der Wert der Ladungsdichte

Die Komponente *Charge density* ist eine **Raumladungsdichte** in `C/m^3`, wie
ihre Beschriftung in der Benutzeroberfläche angibt. Sie ist eine Dichte und
keine Gesamtmenge, was sie vom Netz unabhängig macht - und, wie 1.4 anmerkt, was
sie von der Bedingung *Heat* des Wärmelösers unterscheidet, die eine Gesamtmenge
in Watt annimmt und für Sie verteilt.

Das Vorzeichen folgt der üblichen Konvention. Partielle Integration von
`div(e0*er*grad(V)) = -rho` lässt die Ladung mit positivem Vorzeichen auf der
rechten Seite stehen,

```
every entity type      f = +INT( N^T * rho )
```

(`every entity type` = jeder Entitätstyp), sodass eine **positive**
Ladungsdichte das Potential um sie herum anhebt und eine negative es absenkt,
auf Volumen-, Flächen-, Linien- und Punktentitäten gleichermaßen.

Denken Sie daran, dass eine Ladungsquelle allein kein lösbares Modell ergibt.
Das Potential ist weiterhin nur bis auf eine Konstante definiert, bis etwas es
festlegt, daher benötigt ein ladungsgetriebenes Modell zusätzlich mindestens eine
Bedingung *Electric potential* - am äußeren Rand des vernetzten Bereichs, wenn
nirgendwo sonst. Siehe 1.7.

### 1.6 Umgebungs-, Anfangs- und Kopplungsbedingungen

#### Umgebungsbedingungen

| Umgebungsbedingung | Komponenten | Wirkung |
|---|---|---|
| Temperature | Temperature `[K]` | angeboten, aber nicht gelesen - siehe unten |

Die Umgebungsbedingung *Temperature* wird für ein elektrostatisches Problem
angeboten, weil Materialeigenschaften Tabellen über der Temperatur sind. Der
Elektrostatik-Löser liest sie **nicht**: Er übernimmt Elementtemperaturen nur aus
den gemeinsamen Daten, die eine Aufgabe *Heat transfer* hinterlässt, und wertet
sonst jede Tabelle bei `293.15 K` aus. Sie einem reinen Elektrostatik-Modell
zuzuweisen ändert nichts.

#### Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Electric potential | Anfangspotentialfeld, beim ersten Lauf angewendet |

Das Anfangspotential wird **nur beim ersten Lauf** gelesen. Da die Lösung direkt
und nicht schrittweise erfolgt, ändert es das konvergierte Ergebnis nicht - es
legt nur den Startpunkt der Iteration mit konjugierten Gradienten und den Wert
jedes Knotens fest, den der Löser aus dem System herauslässt.

Umgebungsbedingungen, Anfangsbedingungen und Randbedingungen werden in dieser
Reihenfolge gelesen, und jede überschreibt die vorherige, daher **gewinnt eine
Randbedingung gegenüber einer Anfangsbedingung, die gegenüber einer
Umgebungsbedingung gewinnt**.

#### Kopplung mit anderen Lösern

Der Elektrostatik-Löser erzeugt zwei Ergebnisse, die andere Löser verwenden.

| Richtung | Variable | Wirkung |
|---|---|---|
| Elektrostatik -> Wärme | Joule heat | wird zum Quellterm jedes Elements addiert |
| Elektrostatik -> Wärme in Fluiden | Joule heat | wird zum Quellterm jedes Elements addiert |
| Elektrostatik -> Magnetostatik | Current density | Quelle des Magnetfelds |
| Wärme -> Elektrostatik | Temperature | wählt die Zeile jeder temperaturabhängigen Materialtabelle |

Die **Joulesche Wärme** wird von einer Aufgabe *Heat transfer* aufgenommen, die
im Aufgabenablauf des Problems **nach** der Elektrostatik-Aufgabe steht, und dort
genau wie eine Bedingung *Heat rate (unit volume)* zum Element-Quellterm
addiert. Widerstandserwärmung erfordert daher keine Einstellung außer der
Reihenfolge der beiden Aufgaben - Tutorial 4 geht dies durch.

**Magnetostatik** ist ein eigener Problemtyp, der eine Elektrostatik-Aufgabe
*erfordert*: Er liest die von diesem Löser gespeicherte Element-Stromdichte -
auf Volumen-, Flächen- und Linienentitäten gleichermaßen - und wertet das
Magnetfeld dieses Stroms mit dem Biot-Savart-Gesetz aus. Fügen Sie die
Elektrostatik-Aufgabe zuerst hinzu.

Da der Löser sich bedingungslos als konvergiert meldet, hält eine Aufgabengruppe,
die nur eine Elektrostatik-Aufgabe enthält, immer nach einer Iteration an.
Iterieren lohnt sich nur, wenn etwas, von dem die Lösung abhängt, selbst
nachläuft - etwa eine stark temperaturabhängige Permittivität, die von einer
Wärmeaufgabe gespeist wird.

### 1.7 Lösen des Systems

Der Löser assembliert und löst

```
K * V = f
```

`K` ist symmetrisch positiv definit, sobald mindestens ein Potential
vorgeschrieben ist, und wird daher mit dem Verfahren der **konjugierten
Gradienten** mit Jacobi-Vorkonditionierung gelöst. Die Assemblierung ist über
die Elemente parallelisiert, mit Puffern pro Thread, die am Ende einmal
zusammengeführt werden, sodass das Ergebnis nicht von der Anzahl der Threads
abhängt.

Ein Modell **ohne Randbedingung Electric potential** ist singulär: Das Potential
ist nur bis auf eine Konstante definiert, die Matrix hat einen Nullraum, und die
Lösung konvergiert nicht. Jedes Modell benötigt mindestens ein vorgeschriebenes
Potential, und ein Modell, das einen Strom führen soll, benötigt zwei mit
unterschiedlichen Werten.

Drei weitere Wege zu einem singulären oder leeren System sind wissenswert, weil
alle drei stillschweigend auftreten:

- eine Entität, deren Material eine der drei erforderlichen Eigenschaften
  fehlt, wird nicht gelöst, und ihre Knoten fallen aus dem System heraus;
- eine Entität mit `er = 0` hat keine Steifigkeit, daher bleiben ihre Knoten im
  System, sind aber mit nichts verbunden;
- eine Fläche mit Dicke null oder eine Linie mit Querschnittsfläche null hat
  ebenfalls keine Steifigkeit.

### 1.8 Abgeleitete Ergebnisse

Nach der Lösung werden die Elementergebnisse aus dem Knoten-Potentialfeld
gewonnen. Das elektrische Feld ist der über die Integrationspunkte gemittelte
negative Potentialgradient, für Linien- und Flächenelemente in globale
Koordinaten gedreht:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`electric energy` = elektrische Energie, `electrical resistivity` =
spezifischer elektrischer Widerstand, `Joule heat` = Joulesche Wärme.)

| Ergebnis | Anwendbar auf | Bedeutung |
|---|---|---|
| Electric potential `[V]` | Knoten | das gelöste Potentialfeld |
| Electric field `[V/m]` | Element | Feldvektor |
| Current density `[A/m^2]` | Element | `sigma*E`, null, wo die Leitfähigkeit null ist |
| Electric energy `[J/m^3]` | Element | Energiedichte des Felds |
| Electrical resistivity `[V*m/A]` | Element | `1/sigma` überall, wo Strom fließt, sonst `0` |
| Joule heat `[W/m^3]` | Element | ohmsche Verlustleistung, wird an den Wärmelöser übergeben |

All dies sind **Dichten**: Die Ableitungen der Formfunktionen werden über die
Integrationspunkte des Elements gemittelt und nicht mit seiner
Jacobi-Determinante gewichtet, und die Flächendicke sowie die
Querschnittsfläche der Linie bleiben in der Steifigkeit, wo sie hingehören,
statt in das gewonnene Feld einzugehen. Feld, Stromdichte und elektrische
Energie sind daher unabhängig von der Größe des Elements, in dem sie berechnet
wurden, sodass sie zwischen Elementen und mit einer Handrechnung verglichen
werden können und bei Verfeinerung des Netzes konvergieren.

Der **spezifische elektrische Widerstand** ist das Ergebnis der Wahl zur
Überprüfung eines Modells, weil sich die beiden Faktoren, aus denen er gebildet
wird, aufheben: `|E|/|sigma*E|` ist genau `1/sigma`, was auch immer das Feld
tut. Er ist die schnellste Bestätigung, dass die für eine Entität vorgesehene
Leitfähigkeit tatsächlich ihre Elemente erreicht hat. Er wird auf jedem Element
als `0` gespeichert, auf dem die Stromdichte unter die numerische Toleranz
fällt, was jedes Element mit Leitfähigkeit null einschließt.

**Die Joulesche Wärme** ist die Verlustleistungsdichte `sigma*|E|^2` in `W/m^3`.
Genau das erwartet eine Aufgabe *Heat transfer*, wenn sie den Wert zu ihrem
Quellterm addiert und über das Element integriert, sodass die in einen
gekoppelten thermischen Lauf übergehende Leistung die Leistung ist, die das
Modell tatsächlich abgibt. Wie die anderen ist sie eine Dichte und daher
unabhängig von der Elementgröße und konvergiert bei Verfeinerung.

Zwei weitere Stellen, an denen abgeleitete Ergebnisse einfach auf null bleiben:
**Punktelemente** werden nie nachbearbeitet, und eine **Fläche mit Dicke null**
oder eine **Linie mit Querschnittsfläche null** wird übersprungen, sodass beide
ein Feld von null behalten, wie groß der Potentialgradient über ihnen auch ist.

Nach dem Schreiben des Datensatzes gibt das Löserprotokoll die Statistik aller
sechs Variablen und die Werte an allen Überwachungspunkten aus.

### 1.9 Was der Löser ist und was nicht

Der Name ist genau: Dies ist ein **elektrostatischer** Löser. Er berechnet das
Feld einer Ladungs- und Potentialverteilung in einem Dielektrikum. Die
angegebene Stromdichte ist das, was das Ohmsche Gesetz für dieses Feld ergibt -
eine abgeleitete Größe, nachträglich berechnet, die das Feld selbst nie
beeinflusst.

Ein Problem **stationärer Strömung des elektrischen Stroms** wird dagegen
bestimmt durch

```
div( sigma * grad(V) ) = 0
```

mit der Leitfähigkeit an der Stelle, an der dieser Löser die Permittivität
verwendet. Beide ergeben in einem homogenen Körper dasselbe Potentialfeld, weil
sich beide auf die Laplace-Gleichung reduzieren und die Konstante sich
herauskürzt. Sie unterscheiden sich, sobald der Körper aus mehr als einem
Material besteht:

| | Dieser Löser | Ein Stromflusslöser |
|---|---|---|
| Potential teilt sich auf im Verhältnis zu | `1/er` jedes Bereichs | `1/sigma` jedes Bereichs |
| Richtig für | einen Kondensator, ein Dielektrikum, einen Isolator | ein Widerstandsnetzwerk, eine Stromschiene, einen leitenden Pfad |

Ein Leiter aus einem Material mit vorgeschriebenen Potentialen an seinen Enden
wird also korrekt modelliert: Das Potential ist linear, und die Division durch
die Länge ergibt das Feld. Zwei in Reihe geschaltete Leiter unterschiedlicher
Leitfähigkeit dagegen nicht - die Aufteilung folgt den Permittivitäten, was
kaum beabsichtigt sein dürfte. Modellieren Sie einen solchen Fall als ein
einziges Material, oder berechnen Sie die Aufteilung von Hand.

### 1.10 Statische Analyse und der Zeitlöser

Elektrostatik ist als Problemtyp **ohne** Zeitlöser deklariert. Das hat zwei
sichtbare Folgen.

In einem reinen Elektrostatik-Modell erscheint das Gruppenfeld *Zeitlöser* auf
der Registerkarte `Problem` überhaupt nicht, der Lauf erzeugt einen einzigen
Datensatz, und es gibt nichts zu konfigurieren.

Überraschender ist, dass das Hinzufügen einer Elektrostatik-Aufgabe zu einem
Aufgabenablauf **den Zeitlöser für den gesamten Ablauf deaktiviert**. Der
Zeitlöser wird nur angeboten, wenn *jeder* Problemtyp im Ablauf einen
unterstützt, sodass eine transiente Wärmeanalyse, der eine
Elektrostatik-Aufgabe hinzugefügt wird, zu einer stationären Analyse wird, ohne
eine andere Warnung als das Verschwinden des Gruppenfelds. Wird für ein
Widerstandsheizelement eine transiente thermische Antwort benötigt, führen Sie
die Elektrostatik-Aufgabe separat aus und übertragen Sie ihre Verlustleistung
als Bedingung *Heat* in das transiente Wärmemodell, statt beide Aufgaben in
einen Ablauf zu legen.

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Umgebungsbedingungen`, `Material`,
`Ergebnisse`), im Menü **Problem** und im Panel **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Fügen Sie eine Aufgabe hinzu und wählen Sie
**Electro-statics** - „Calculate electric field, current density and generated
Joule's heat“ (elektrisches Feld, Stromdichte und erzeugte Joulesche Wärme
berechnen).

Elektrostatik lässt sich frei mit jeder anderen Physik außer *Modal analysis*
kombinieren. Wissenswerte Ketten sind:

- *Electro-statics*, dann *Heat transfer* - Widerstandserwärmung;
- *Electro-statics*, dann *Magneto-statics* - das Magnetfeld des berechneten
  Stroms, die einzige Möglichkeit, überhaupt eine magnetostatische Analyse
  durchzuführen;
- *Electro-statics*, dann *Heat transfer*, dann *Stress analysis* - thermische
  Spannungen durch Widerstandserwärmung.

Denken Sie an 1.10: Jeder Ablauf, der eine Elektrostatik-Aufgabe enthält, wird
stationär gelöst.

### 2.2 Registerkarte Problem

Für ein reines Elektrostatik-Modell ist diese Registerkarte **leer**. Es gibt
keinen Zeitlöser, weil der Problemtyp keinen unterstützt, und die Elektrostatik
hat kein eigenes Einstellungs-Gruppenfeld. Alles wird pro Entität auf den
Registerkarten der Bedingungen zugewiesen.

Zeigt die Registerkarte ein Gruppenfeld *Zeitlöser*, hat es eine andere Aufgabe
im Ablauf dorthin gebracht - und gemäß 1.10 verschwindet es wieder, sobald die
Elektrostatik-Aufgabe hinzugefügt wird.

### 2.3 Registerkarte Material

Die Registerkarte `Material` listet das der ausgewählten Entität zugewiesene
Material auf. Elektrostatik benötigt **alle drei** dieser Eigenschaften:

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Relative permittivity | `N/A`, dimensionslos | `er`, `1` für Vakuum, `1.00059` für Luft, etwa `4` für Glas |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` für Kupfer, `5e-15` für Luft |
| Density | `kg/m^3` | erforderlich, damit die Entität gelöst wird, nie gelesen |

Eine Entität, der eine davon fehlt, wird nicht gelöst, und die
Einstellungsprüfung warnt vor dem Start des Laufs mit *Entity ... has material
assigned which is missing required properties*. Da die Dichte von diesem Löser
nie verwendet wird, erfüllt jeder positive Wert die Anforderung.

Die mitgelieferte Materialbibliothek enthält bereits alle drei für die Metalle,
für Luft und für die Gläser, sodass die Zuweisung von *Copper* oder *Air* für
den Anfang genügt.

Jede Eigenschaft ist eine Tabelle über der Temperatur, die
Temperaturabhängigkeit wird jedoch nur berücksichtigt, wenn eine Wärmeaufgabe
weiter vorne im Ablauf Elementtemperaturen erzeugt hat - siehe 1.6.

### 2.4 Registerkarte Randbedingungen

Die Registerkarte ist oben in eine Verwaltungsliste der verfügbaren
Randbedingungen und unten in einen Editor für die ausgewählte Bedingung
aufgeteilt. Wählen Sie zuerst eine Entität im Baum `Modell` aus; die Verwaltung
bietet dann die für diesen Entitätstyp und das ausgewählte Problem gültigen
Bedingungen an. Aktivieren Sie eine, um sie zuzuweisen, und bearbeiten Sie dann
ihre Komponenten im unteren Baum.

Es werden nur die beiden Bedingungen aus Abschnitt 1.4 angeboten. Hinweise zu
ihrer Verwendung:

- **Electric potential** ist die Bedingung, die einen Wert festhält, statt ihn
  anzutreiben, und wird für jeden Entitätstyp angeboten. Weisen Sie sie der
  Seite, Kante oder dem Punkt zu, dessen Potential bekannt ist. Ein Modell ohne
  eine solche Bedingung lässt sich nicht lösen.
- **Charge density** ist der Quellterm, und ein positiver Wert hebt das
  Potential um ihn herum an. Lesen Sie 1.5, bevor Sie ihn verwenden: Der Wert
  ist eine Dichte und wird, anders als bei der Bedingung *Heat* des Wärmelösers,
  nicht über die Entität verteilt.
- Beide Komponenten sind Tabellen über der Zeit, was nur in einem
  Aufgabenablauf eine Rolle spielt, der von einer anderen, zeitabhängigen
  Aufgabe angetrieben wird.

### 2.5 Anfangs- und Umgebungsbedingungen

Die Registerkarte `Anfangsbedingungen` bietet *Electric potential* an, das das
Ausgangsfeld der iterativen Lösung festlegt. Es ändert das konvergierte
Ergebnis nicht.

Die Registerkarte `Umgebungsbedingungen` bietet *Temperature* an, die dieser
Löser ignoriert - siehe 1.6.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Elektrostatik-Löser verwendet den Eintrag **CG**.

Konvergiert die Lösung nicht, ist die übliche Ursache ein Modell ohne
vorgeschriebenes Potential oder eine Entität mit Permittivität null und keine
Lösereinstellung. Prüfen Sie die Randbedingungen und das Material, bevor Sie die
Anzahl der Iterationen erhöhen.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable - für ein
elektrostatisches Problem ist *Electric potential* die übliche Wahl, und es ist
der schnellste Weg, einen Wert als Zahl statt als Farbe aus einem Modell
abzulesen. `Bericht` -> `Überwachungspunkte` zeigt die Werte an.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet die berechneten Variablen auf und steuert
die 3D-Ansicht. Das elektrische Potential ist ein Knotenskalar; das elektrische
Feld und die Stromdichte sind Elementvektoren; die elektrische Energie, der
spezifische elektrische Widerstand und die Joulesche Wärme sind
Elementskalare.

Die Registerkarte `Datensätze` des Panels `Modell` listet die
Ergebnisdatensätze auf. Eine elektrostatische Analyse ist statisch, daher gibt
es genau **einen**.

`Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige Ausgabe des
Lösers, einschließlich der Statistik aller sechs Variablen nach dem Schreiben
des Datensatzes.

---

## 3. Tutorial - Potential und Strom in einem Leiter

**Ziel.** Die Potentialverteilung entlang eines Kupferstabs berechnen, der auf
zwei unterschiedlichen Potentialen gehalten wird, und Feld, Strom und Widerstand
ablesen.

Dieses Tutorial setzt ein vernetztes Volumenmodell voraus - ein langer Quader
eignet sich gut. Verwenden Sie `Datei` -> `Neues Modell`, zeichnen Sie einen
Quader mit `Geometrie` -> `Element erstellen` oder importieren Sie eine
Geometrie und tetraedrisieren Sie sie; das Tutorial *Würfel zeichnen* behandelt
diese Schritte. Markieren Sie die beiden Stirnflächen als separate
Flächenentitäten, damit ihnen Bedingungen zugewiesen werden können.

Nehmen Sie für den Stab eine Länge von `0.1 m` und einen Querschnitt von
`0.01 m` mal `0.01 m` an, damit die Handrechnungen am Ende leicht
nachzuvollziehen sind.

### Schritt 1 - Problem auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine Aufgabe hinzu und wählen Sie **Electro-statics**.
3. Bestätigen Sie mit `OK`.

Die Registerkarte `Problem` bleibt leer - es gibt nichts einzurichten.

### Schritt 2 - Material zuweisen

1. Wählen Sie die Volumenentität im Baum `Modell` aus.
2. Öffnen Sie die Registerkarte `Material` und weisen Sie **Copper** zu, oder ein
   Material mit allen drei Eigenschaften:
   - **Relative permittivity** = `2.8`
   - **Electrical conductivity** = `5.882e7` `S/m`
   - **Density** = `8960` `kg/m^3`

Die Dichte wird von diesem Löser nicht verwendet, muss aber vorhanden sein,
damit die Entität überhaupt gelöst wird. Die Permittivität treibt das Feld an;
die Leitfähigkeit rechnet das Feld nur nachträglich in einen Strom um.

### Schritt 3 - beide Enden vorschreiben

1. Wählen Sie die Flächenentität an einem Ende aus.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie **Electric
   potential**.
3. Setzen Sie den Wert auf `0` V - das ist die Masse.
4. Wählen Sie die Flächenentität am anderen Ende aus, aktivieren Sie erneut
   **Electric potential** und setzen Sie es auf `10` V.

Dieser Schritt macht das Modell lösbar. Ein einziges vorgeschriebenes Potential
ist das Minimum; zwei mit unterschiedlichen Werten erzeugen ein Feld.

Fügen Sie keine Bedingung *Charge density* hinzu. Ein Leiter mit
vorgeschriebenen Endpotentialen trägt keine Raumladung, daher gehört der
Quellterm auf null, und das Potential ist eine Gerade zwischen den beiden
Enden - was die Handrechnung in Schritt 6 exakt macht.

### Schritt 4 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Die Einstellungsprüfung meldet fehlende Materialien vor dem Start des Laufs.
3. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.

Der Lauf ist eine einzige Lösung mit konjugierten Gradienten und ist bei einem
Netz dieser Größe in Sekunden beendet.

### Schritt 5 - Ergebnisse untersuchen

- Registerkarte `Ergebnisse`: Zeigen Sie **Electric potential** an. Es sollte
  entlang des Stabs linear von `10` V auf `0` V abfallen, mit ebenen
  Äquipotentialflächen senkrecht zu seiner Achse. Jede Krümmung in einem
  gleichförmigen Stab bedeutet, dass das Netz nicht so verbunden ist, wie
  angenommen.
- **Electrical resistivity** sollte auf jedem Element des Stabs
  `1/5.882e7 = 1.7e-8` `V*m/A` anzeigen. Das ist die beste einzelne Prüfung,
  dass das Material die Elemente erreicht hat: Sie ist exakt, netzunabhängig
  und falsch, sobald die Leitfähigkeit falsch ist.
- **Electric field** zeigt entlang des Stabs und sollte gleichmäßig
  `10/0.1 = 100` `V/m` anzeigen, und **Current density** `sigma*E = 5.88e9`
  `A/m^2`. Beide sind Dichten, daher gelten die Zahlen unabhängig von der
  Elementgröße - Schritt 6 leitet sie her.
- Ein Überwachungspunkt auf *Electric potential* auf halber Länge des Stabs
  sollte `5` V anzeigen.

### Schritt 6 - Zahlen von Hand nachrechnen

Die ersten beiden Zeilen unten sollten die Ergebnisse **Electric field** und
**Current density** bereits anzeigen, sie sind also eine direkte Prüfung der
Lösung. Die letzten drei sind Gesamtgrößen statt Dichten, die kein
Elementergebnis liefert, und müssen berechnet werden. Für einen gleichförmigen
Stab der Länge `L` und mit dem Querschnitt `A`, der auf einer Potentialdifferenz
`U` gehalten wird:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

Der Strom ist enorm, weil ein massiver Kupferstab dieses Querschnitts ein sehr
guter Leiter ist und `10` V darüber einem Kurzschluss entsprechen - eine
nützliche Erinnerung daran, dass ein elektrostatisches Modell bereitwillig eine
Situation berechnet, die das reale Bauteil verdampfen würde.

### Schritt 7 - optional, Netz verfeinern

Verfeinern Sie das Netz einmal und lösen Sie erneut. Jedes Ergebnis - das
**Potential**, das **elektrische Feld**, die **Stromdichte**, die **elektrische
Energie**, der **spezifische Widerstand** und die **Joulesche Wärme** - muss
seinen Wert behalten. Sie sind alle Dichten, daher ist ein Wert, der sich bei
Verfeinerung verschiebt, ein Zeichen für ein Modellproblem und nicht für ein
feineres Netz, und es lohnt sich, dem nachzugehen, bevor man weitermacht.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die Lösung konvergiert nicht | nirgends eine Bedingung *Electric potential*, daher ist das Potential nur bis auf eine Konstante definiert |
| *Entity has material assigned which is missing required properties* | eine der drei erforderlichen Eigenschaften fehlt - oft die Dichte, die der Löser nie verwendet |
| Das gesamte Modell liegt auf einem Potential | es wurde nur ein Potential vorgeschrieben, oder das zweite hat denselben Wert |
| Das Potential ist überall null außer auf den vorgeschriebenen Seiten | die Entität hat `er = 0`, daher hat sie keine Steifigkeit und verbindet nichts |
| Der spezifische Widerstand ist überall null | die elektrische Leitfähigkeit ist null, daher fließt kein Strom, und der spezifische Widerstand wird als null gespeichert |
| Eine Flächenentität zeigt überhaupt kein Feld | ihre Dicke ist null, daher wird sie weder assembliert noch nachbearbeitet |

---

## 4. Tutorial - Widerstandserwärmung

**Ziel.** Die aus dem elektrischen Feld berechnete Joulesche Wärme verwenden, um
den Stab zu erwärmen, indem eine Elektrostatik-Aufgabe und eine Wärmeaufgabe
verkettet werden.

### Schritt 1 - Aufgabenablauf aufbauen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine Aufgabe **Electro-statics** hinzu.
3. Fügen Sie **danach** eine Aufgabe **Heat transfer** hinzu.
4. Bestätigen Sie mit `OK`.

Die Reihenfolge ist wichtig: Der Wärmelöser liest die vom Elektrostatik-Löser
hinterlassene Joulesche Wärme, daher muss die Elektrostatik-Aufgabe zuerst
laufen. Für ihre Verbindung ist keine Bedingung und keine Einstellung nötig.

Beachten Sie, was mit der Registerkarte `Problem` geschieht: Das Gruppenfeld
*Zeitlöser*, das eine Wärmeaufgabe allein anzeigen würde, ist nun verschwunden,
weil die Elektrostatik keinen Zeitlöser hat und der Ablauf die restriktivste
Einstellung übernimmt - siehe 1.10. Diese Kette ist nur stationär.

### Schritt 2 - Material vervollständigen

Die Wärmeaufgabe benötigt zusätzlich zu den drei Eigenschaften der
Elektrostatik-Aufgabe vier eigene, daher muss das Material der Entität alle
folgenden enthalten:

| Eigenschaft | Wert für Kupfer | Benötigt von |
|---|---|---|
| Relative permittivity | `2.8` | Elektrostatik |
| Electrical conductivity | `5.882e7` `S/m` | Elektrostatik |
| Density | `8960` `kg/m^3` | beiden |
| Thermal conductivity | `401` `W/(m*K)` | Wärme |
| Heat capacity | `385` `J/(kg*K)` | Wärme |
| Emissivity | `0` | Wärme |

Das mitgelieferte Material *Copper* enthält sie bereits. Eine Entität, der eine
von **einer der beiden** Aufgaben benötigte Eigenschaft fehlt, wird von dieser
Aufgabe übersprungen.

### Schritt 3 - elektrische Randbedingungen beibehalten

Die beiden Bedingungen **Electric potential** aus Tutorial 3 werden unverändert
übernommen. Senken Sie das antreibende Potential auf einen weniger heftigen Wert -
`0.01` V über dem Stab erzeugt immer noch einen großen Strom und hält die
Wärmequelle in einem Bereich, in dem die thermische Antwort lesbar ist.

### Schritt 4 - thermische Randbedingungen hinzufügen

Die thermische Hälfte des Modells benötigt einen eigenen Wärmeabfluss, genau wie
sie es allein bräuchte:

1. Wählen Sie die Flächenentität aus, die die Seiten des Stabs bedeckt.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie **Simple
   convection**.
3. Setzen Sie **Convection coefficient** = `10` `W/(m^2*K)` und **Fluid
   temperature** = `293.15` K.

Ohne konvektive Fläche oder vorgeschriebene Temperatur hat das Wärmemodell eine
Quelle und keinen Abfluss, und der stationäre Zustand existiert nicht. Beachten
Sie, dass eine für Konvektion verwendete Seite für den Wärmelöser keine Dicke
benötigt, eine Seite mit Dicke null aber auch kein elektrisches Feld führt -
belassen Sie die elektrischen Bedingungen auf den Stirnflächen und die
Konvektion auf den Seiten, dann stören sich beide nicht.

### Schritt 5 - lösen

`Lösung` -> `Löser starten` (`Ctrl+R`). Das Protokoll zeigt zuerst die
Elektrostatik-Aufgabe mit der Statistik ihrer sechs Variablen und dann die
Wärmeaufgabe mit der Statistik von Temperatur, Wärmestrom und
Wärmeübergangskoeffizient.

### Schritt 6 - Ergebnisse untersuchen

- Registerkarte `Ergebnisse`: Zeigen Sie **Joule heat** an. Sie ist die Brücke
  zwischen den beiden Physiken - überall, wo sie ungleich null ist, hat der
  Wärmelöser eine Quelle gesehen.
- Zeigen Sie **Temperature** an. Der Stab sollte dort am wärmsten sein, wo die
  Joulesche Wärme am größten ist, und an den konvektiven Seiten zur
  Fluidtemperatur hin abfallen.
- Zeigen Sie **Electric potential** an, um zu bestätigen, dass die elektrische
  Hälfte des Modells weiterhin das lineare Feld aus Tutorial 3 ist.

### Schritt 7 - Energiebilanz prüfen

Die Joulesche Wärme ist eine Verlustleistungsdichte in `W/m^3`, daher ist die
Gesamtleistung, die die elektrische Hälfte des Modells an die thermische Hälfte
abgibt, diese Dichte mal das Volumen, auf das sie wirkt. Für den gleichförmigen
Stab sollte sie das `P = U^2/R` aus Tutorial 3 ergeben, mit `R = L/(sigma*A)`.

Im stationären Zustand muss die konvektive Fläche genau diese Leistung abgeben,
was dieselbe globale Prüfung ergibt, die das Handbuch Wärmeübertragung
beschreibt:

```
P = h * A_s * ( Ts_mean - Tf )
```

Lesen Sie die mittlere Oberflächentemperatur aus der Statistik im
Löserprotokoll ab und bestätigen Sie, dass die Bilanz aufgeht. Dies ist die eine
Prüfung, die beide Physiken zugleich umfasst: Sie deckt eine falsche
Leitfähigkeit, einen falschen Konvektionskoeffizienten und eine Entität auf, die
in einer der beiden Hälften stillschweigend ungelöst blieb.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die Temperatur ist gleichmäßig `293.15` K | die Joulesche Wärme ist null - prüfen Sie die elektrische Leitfähigkeit und ob die Elektrostatik-Aufgabe zuerst läuft |
| Die Wärmelösung konvergiert nicht | die thermische Hälfte hat keine vorgeschriebene Temperatur und keine konvektive Fläche |
| Das Gruppenfeld *Zeitlöser* ist verschwunden | erwartet - eine Elektrostatik-Aufgabe macht den gesamten Ablauf stationär, siehe 1.10 |
| Der Temperaturanstieg ändert sich bei Verfeinerung des Netzes | nicht erwartet - jedes elektrostatische Ergebnis ist eine Dichte, prüfen Sie daher die thermische Hälfte des Modells und die Netzqualität |
| Es erscheinen nur die Ergebnisse der Elektrostatik | der Wärmeaufgabe fehlt eine ihrer vier erforderlichen Materialeigenschaften |

---

## 5. Überprüfung eines Modells

Der Löser hat keine eingebaute Verifikationssuite, daher lohnt es sich, ein neues
Modell an etwas zu validieren, das sich von Hand berechnen lässt, bevor man ihm
vertraut. Die folgenden Prüfungen beziehen sich auf das **Potential**, das
sowohl die primäre Unbekannte als auch die Größe ist, in der jede geschlossene
Lösung ausgedrückt wird; die Ergebnisse elektrisches Feld und Stromdichte
sollten mit den daraus folgenden Gradienten übereinstimmen.

**Gleichförmiger Stab.** Für einen Körper der Länge `L`, der auf zwei
gegenüberliegenden Seiten auf `V1` und `V2` gehalten wird, mit einem Material und
ohne Ladung, ist das Potential entlang des Stabs linear und

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Platzieren Sie einen Überwachungspunkt in der Mittelebene und bestätigen Sie,
dass er den Mittelwert der beiden vorgeschriebenen Werte anzeigt, und einen
weiteren bei einem Viertel der Länge für den Viertelwert. Dies deckt ein nicht
verbundenes Netz, eine der falschen Entität zugewiesene Bedingung und eine
Permittivität von null auf. Verfeinern Sie das Netz einmal und bestätigen Sie,
dass die Zahlen gleich bleiben.

**Gleichförmige Raumladung.** Der obige Stab prüft nur die Steifigkeit. Um auch
den Quellterm zu prüfen, nehmen Sie denselben Körper, halten Sie **beide**
Stirnflächen auf `0` V und weisen Sie dem Volumen eine gleichförmige *Charge
density* `rho` zu. Das Potential ist dann eine Parabel, mit `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`at the mid-plane` = in der Mittelebene.) Ein Überwachungspunkt in der
Mittelebene liefert `Vmax` direkt. Es werden zwei Dinge zugleich geprüft: der
Betrag, der bestätigt, dass Permittivität und Ladungsdichte wie erwartet gelesen
werden, und das **Vorzeichen** - eine positive Ladungsdichte muss zwischen zwei
geerdeten Seiten einen positiven Potentialbuckel erzeugen. Wiederholen Sie dies
mit der Ladung auf einer Flächen- oder Linienentität und auf einem Punkt, und
bestätigen Sie, dass sich die Polarität mit dem Entitätstyp nicht ändert.

**Spezifischer Widerstand.** Jedes stromführende Element muss einen
spezifischen elektrischen Widerstand von genau `1/sigma` speichern. Lesen Sie
ihn aus der Statistik im Löserprotokoll ab und vergleichen Sie ihn mit der
Leitfähigkeit im Material. Er ist exakt statt näherungsweise, netzunabhängig und
die schnellste Bestätigung, dass das Material die vorgesehenen Elemente erreicht
hat.

**Koaxiales Feld.** Für einen Ring zwischen den Radien `a` und `b` mit der
inneren Fläche auf `V1` und der äußeren auf `V2` ist das Potential
logarithmisch:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Modellieren Sie den Ring, schreiben Sie beide Flächen vor und vergleichen Sie
Überwachungspunkte bei einigen Radien. Dies validiert den Löser an einem nicht
gleichförmigen Feld, was der Stab nicht kann, und zeigt, ob das Netz in der Nähe
des Innenleiters fein genug ist.

**Dielektrika in Reihe.** Für zwei Schichten der Dicke `d1`, `d2` und der
relativen Permittivität `er1`, `er2`, gestapelt zwischen zwei vorgeschriebenen
Potentialen, beträgt das Potential an der Grenzfläche

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Vergleichen Sie mit dem berechneten Grenzflächenpotential. Dies bestätigt
direkt, was Abschnitt 1.9 beschreibt: Die Aufteilung folgt den
**Permittivitäten**. Dasselbe Modell mit zwei unterschiedlichen Leitfähigkeiten
und gleicher Permittivität zu rechnen und zu sehen, dass sich das
Grenzflächenpotential nicht bewegt, ist der Nachweis, dass dies ein
elektrostatischer Löser und kein Stromflusslöser ist.

**Widerstand und Verlustleistung.** Die Elementergebnisse sind Dichten, daher
muss eine Gesamtgröße - ein Strom, ein Widerstand, eine Leistung - aus dem
Potential kommen. Für den gleichförmigen Stab:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Verwenden Sie diese, wann immer eine Gesamtgröße das Modell verlassen muss - für
die Dimensionierung eines Leiters oder für eine Plausibilitätsprüfung gegen eine
Messung. `P` ist auch das, worauf sich das Ergebnis **Joule heat** über das
Volumen des Stabs integrieren muss, was die Prüfung ist, dass die elektrische und
die thermische Hälfte eines gekoppelten Laufs übereinstimmen.

Verfeinern Sie bei jeder dieser Prüfungen das Netz mindestens einmal und
bestätigen Sie, dass das Potential konvergiert und nicht driftet.

---

## 6. Einschränkungen

### Einschränkungen der Modellierung

- **Elektrostatik, kein Stromfluss.** Die Steifigkeit wird aus der Permittivität
  aufgebaut; die Leitfähigkeit geht nie in das Gleichungssystem ein. Das
  Potential in einem Körper aus zwei Materialien teilt sich daher im Verhältnis
  der Permittivitäten auf, nicht der Widerstände. Ein Leiter aus einem Material
  wird korrekt modelliert, ein Widerstandsnetzwerk nicht - siehe 1.9.
- **Nur statisch.** Es gibt keinen Zeitlöser, keine Kapazität und keine
  Induktivität, daher liegen Ladevorgänge, Wechselfelder, Verschiebungsstrom,
  Wirbelströme und Skineffekt außerhalb des Modells. Das Hinzufügen einer
  Elektrostatik-Aufgabe zu einem Aufgabenablauf entfernt außerdem den Zeitlöser
  aus dem gesamten Ablauf, sodass eine dahinter verkettete Wärmeaufgabe nur
  stationär sein kann - siehe 1.10.
- **Nur lineare, isotrope Dielektrika.** Permittivität und Leitfähigkeit eines
  Elements sind Skalare und während einer Lösung konstant. Anisotrope,
  ferroelektrische und feldabhängige Materialien werden nicht modelliert, und es
  gibt keinen dielektrischen Durchschlag.
- **Nur zwei Randbedingungen.** Ein vorgeschriebenes Potential und eine
  Ladungsdichte bilden das gesamte Vokabular. Es gibt keine vorgeschriebene
  Flächenladung, keinen vorgeschriebenen Normalfluss, keine Symmetriebedingung
  und keine Behandlung offener Ränder oder unendlicher Elemente - ein
  unbegrenztes Feldproblem muss abgeschnitten werden, indem ausreichend
  umgebende Luft vernetzt und an ihrem äußeren Rand ein Potential vorgeschrieben
  wird.
- **Keine Umverteilung freier Ladung.** Ladung wandert nicht an die Oberfläche
  eines Leiters; der Löser kennt einen Leiter über die ihm gegebenen
  Materialeigenschaften hinaus überhaupt nicht. Modellieren Sie einen Leiter,
  indem Sie sein Potential vorschreiben, nicht indem Sie ihm eine große
  Leitfähigkeit geben.
- **Keine Rückwirkung des Stroms.** Die Stromdichte wird aus dem Feld
  nachberechnet, daher hat eine Leitfähigkeit, die sich mit dem Feld oder mit der
  durch die Verlustleistung erzeugten Temperatur ändert, innerhalb einer
  einzelnen Lösung keine Wirkung.
- **Flächen sind über ihre Dicke äquipotential.** Eine Flächenentität führt das
  Feld nur in ihrer eigenen Ebene. Eine Fläche mit Dicke null oder eine Linie mit
  Querschnittsfläche null hat überhaupt keine Steifigkeit und wird sowohl bei
  der Assemblierung als auch bei der Nachbearbeitung übersprungen.
- **Punktelemente leiten nicht.** Ein Punkt trägt eine Ladungsquelle und kann
  ein vorgeschriebenes Potential halten, mehr nicht, und er wird nie
  nachbearbeitet. Ein Punkt, dessen Knoten zu keinem anderen berechenbaren
  Element gehört, hinterlässt eine leere Zeile in der Matrix.
- **Die Temperaturabhängigkeit des Materials wird nicht gelesen.** Tabellen
  werden bei `293.15 K` ausgewertet, sofern nicht eine Wärmeaufgabe weiter vorne
  im Ablauf Elementtemperaturen erzeugt hat; die Umgebungsbedingung
  *Temperature* hat auf diesen Löser keine Wirkung.
- **Ein Modell benötigt ein vorgeschriebenes Potential.** Ohne ein solches hat
  die Matrix einen Nullraum, und die Lösung konvergiert nicht.
