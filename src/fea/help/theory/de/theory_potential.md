# Strömung durch poröse Medien - Theoriehandbuch und Benutzerleitfaden

Dieses Dokument beschreibt den Löser für die Strömung durch poröse Medien
(`RSolverPotential`) von Range FEA, der den Problemtyp **Flow through porous
media** bedient: die Gleichungen, die er löst, die Bedeutung jeder Eingabe, die er
akzeptiert, die Teile der grafischen Benutzeroberfläche, die ihn steuern, und zwei
ausgearbeitete Tutorials.

**Inhalt**

1. [Theoretischer Hintergrund](#1-theoretischer-hintergrund)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - druckgetriebene Strömung durch einen Sandfilter](#3-tutorial---druckgetriebene-strömung-durch-einen-sandfilter)
4. [Tutorial - aufwärts gerichtete Sickerströmung in einer senkrechten Säule](#4-tutorial---aufwärts-gerichtete-sickerströmung-in-einer-senkrechten-säule)
5. [Überprüfung eines Modells](#5-überprüfung-eines-modells)
6. [Einschränkungen](#6-einschränkungen)

---

## 1. Theoretischer Hintergrund

### 1.1 Primäre Unbekannte - das Druckfeld

Die primäre Unbekannte ist der **Druck** `p` des Porenfluids in jedem Knoten, in
Pascal. Jeder Knoten trägt einen einzigen Freiheitsgrad, ein Netz mit `N` Knoten
ergibt also ein System aus `N` Gleichungen - so günstig wie ein Wärme- oder
Elektrostatikmodell desselben Netzes. Die Geschwindigkeit des Fluids wird nach der
Lösung aus dem Druckfeld zurückgewonnen.

Ein von dieser Gleichung bestimmtes Druckfeld ist nur **bis auf eine additive
Konstante** festgelegt: Wird überall derselbe Wert zum Druck addiert, ändert sich
keine Geschwindigkeit. Jedes Modell braucht daher mindestens eine Randbedingung
*Pressure (explicit)*, siehe 1.7. Ob der Druck als Absolut- oder Überdruck gelesen
wird, bleibt dem Benutzer überlassen - der Löser arbeitet nur mit seinem Gradienten.

### 1.2 Grundgleichungen

Der Löser implementiert das **Darcy-Gesetz** für die langsame Strömung eines
einzelnen inkompressiblen Fluids, das die Poren eines starren porösen Körpers
füllt:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

zusammen mit der Massenerhaltung eines inkompressiblen Fluids

```
div( v ) = 0
```

was zusammen eine Potentialgleichung (vom Laplace-Typ) für den Druck ergibt:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Symbol | Bedeutung | Einheit | Quelle |
|---|---|---|---|
| `p` | Porenfluiddruck | `Pa` | gelöst |
| `v` | Darcy-Geschwindigkeit (Filtergeschwindigkeit) | `m/s` | zurückgewonnen, siehe 1.8 |
| `k` | intrinsische Permeabilität des porösen Körpers | `m^2` | Materialeigenschaft *Permeability to fluids* |
| `mu` | dynamische Viskosität des Porenfluids | `kg/(m*s)` | Materialeigenschaft *Dynamic viscosity* |
| `rho` | Dichte des Porenfluids | `kg/m^3` | Materialeigenschaft *Density* |
| `g` | Erdbeschleunigung | `m/s^2` | Umgebungsbedingung, siehe 1.6 |

Das Verhältnis `k/mu` spielt die Rolle, die in einem Wärmemodell die
Leitfähigkeit spielt. Ohne Schwerkraft ist die Gleichung die Laplace-Gleichung für
den Druck, und die Strömung folgt direkt dem Druckgefälle. Mit Schwerkraft sorgt
der Term `rho*g` dafür, dass ein ruhendes Fluid einen hydrostatischen
Druckgradienten trägt: `grad(p) = rho*g` ergibt `v = 0`.

**Die Geschwindigkeit ist die Darcy-Geschwindigkeit.** Sie ist der Volumenstrom je
Einheit des **gesamten** Querschnitts, Feststoff und Poren zusammen - nicht die
Geschwindigkeit des Fluids in den Poren. Die mittlere Porengeschwindigkeit ist um
den Kehrwert der Porosität `n` größer:

```
v_pore = v / n
```

Die Porosität geht in eine stationäre Strömung nicht ein, der Löser braucht und
akzeptiert sie daher nicht. Verwenden Sie sie von Hand, um Laufzeiten
abzuschätzen.

**Permeabilität und Durchlässigkeitsbeiwert.** Bodenmechanik und Hydrogeologie
geben statt der intrinsischen Permeabilität oft den *Durchlässigkeitsbeiwert*
(kf-Wert) `K` in `m/s` an - den Beiwert des für die hydraulische Höhe
geschriebenen Darcy-Gesetzes. Beide hängen über die Eigenschaften des Fluids
zusammen:

```
k = K * mu / ( rho * |g| )
```

Für Wasser bei 20 °C entspricht `K = 1e-4 m/s` dem Wert `k = 1.02e-11 m^2`. Die
Erdöltechnik gibt die Permeabilität in Darcy an: `1 D = 9.869e-13 m^2`.

**Erforderliche Materialeigenschaften.** Ein Problem der Strömung durch poröse
Medien erfordert **drei** davon:

| Eigenschaft | Einheit | Verwendet für |
|---|---|---|
| Permeability to fluids | `m^2` | die Steifigkeitsmatrix - Permeabilität des Feststoffs |
| Dynamic viscosity | `kg/(m*s)` | die Steifigkeitsmatrix - Viskosität des **Fluids** |
| Density | `kg/m^3` | den Schwerkraftterm - Dichte des **Fluids** |

**Dichte und Viskosität sind die des Porenfluids, nicht des porösen Körpers.** Ein
Material für diesen Problemtyp beschreibt daher das *gesättigte* Medium: die
Permeabilität des festen Gerüsts und Dichte und Viskosität des Fluids, das es
füllt. Die mitgelieferten Materialien mit dem Namen *... (water saturated)* sind
so aufgebaut (siehe 2.3). Ein Material mit der Rohdichte eines trockenen Bodens
verfälscht den hydrostatischen Druck um das Verhältnis der beiden Dichten.

Eine Entität, deren Material eine der drei Eigenschaften fehlt, wird **nicht
gelöst**. Die Setup-Prüfung warnt vor dem Start davor. Die einzige Ausnahme ist
eine Entität mit einer Randbedingung dieses Problemtyps: Sie wird unabhängig von
ihrem Material in das System aufgenommen, und ihre Permeabilität ist dann einfach
null. So erhält auch eine Randfläche ohne Material einen Zufluss.

Jede Eigenschaft ist eine Tabelle über der Temperatur. Eine Aufgabe der Strömung
durch poröse Medien hat für sich allein kein Temperaturfeld, also wird jede Tabelle
bei `293.15 K` ausgewertet. Die Tabellen werden nur berücksichtigt, wenn eine
*Heat transfer*-Aufgabe früher im selben Aufgabenablauf Elementtemperaturen
berechnet hat - so fließt ein wärmeres Fluid mit geringerer Viskosität schneller.

### 1.3 Finite-Elemente-Diskretisierung

Die schwache Form der Druckgleichung ergibt für jedes Element

```
K = INT( B^T * ( k / mu ) * B )                  Steifigkeit
f = INT( B^T * ( k / mu ) * rho * g )            Schwerkraftlast
  + INT_S( N^T * v_in )                          Zufluss über den Rand
```

wobei `B` aus den Ableitungen der Formfunktionen gebildet wird, `N` die
Formfunktionen sind und `v_in` die auf einer Randfläche vorgegebene
Zuflussgeschwindigkeit ist. Jede Elementmatrix ist `n x n` für ein Element mit `n`
Knoten. Es gibt keine Massen- oder Speichermatrix: Die Strömung ist stationär,
siehe 1.10.

**Elementmaße.** Wie überall in Range FEA erhalten Entitäten niedrigerer Dimension
ein geometrisches Maß, damit sie reale Körper darstellen können:

| Entität | Maß | Wo es gesetzt wird |
|---|---|---|
| Volumen | 1 | - |
| Fläche | Flächendicke | geometrische Eigenschaften der Entität |
| Linie | Linienquerschnitt | geometrische Eigenschaften der Entität |
| Punkt | - | Punkte leiten keine Strömung |

Eine **Fläche** mit Dicke leitet die Strömung in ihrer eigenen Ebene - so wird ein
zweidimensionales Modell (ein Schnitt durch einen Damm oder einen Grundwasserleiter)
aufgebaut. Ihre Schwerkraftlast verwendet die in der Flächenebene liegende
Komponente von `g`. Eine Fläche mit **Dicke null** - die übliche Randfläche eines
Volumenmodells - hat keine Steifigkeit und nimmt nur einen Zufluss auf.

Eine **Linie** mit Querschnitt leitet die Strömung entlang ihrer eigenen Richtung,
für ein eindimensionales Modell einer Säule oder eines gefüllten Rohrs. Ein
**Punkt** trägt nichts bei. Ein Punktelement, dessen Knoten zu keinem anderen
berechenbaren Element gehört, hinterlässt eine leere Zeile in der Matrix - weisen
Sie Punktentitäten kein poröses Material zu.

### 1.4 Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Pressure (explicit) | explizit (Dirichlet) | Fläche | Pressure `[Pa]` |
| Velocity (inflow) | natürlich (vorgegebener Fluss) | Fläche | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | natürlich (vorgegebener Fluss) | Fläche | Volumetric flow rate `[m^3/s]` |

Eine Randfläche ohne eine dieser Bedingungen ist **undurchlässig**: Kein Fluid
tritt durch sie hindurch. Das ist die natürliche Bedingung der Gleichung, Wände,
die Sohle eines Grundwasserleiters und Symmetrieebenen brauchen also keine
Bedingung.

**Pressure (explicit)** ist die einzige explizite Bedingung. Jeder Knoten der
Fläche wird aus dem Gleichungssystem entfernt und auf dem angegebenen Druck
gehalten. Hier verlässt das Fluid das Modell (Auslass, Drän, freies Wasser eines
Beckens) oder tritt unter bekanntem Druck ein. Jedes Modell braucht mindestens
eine. Eine neu zugewiesene Bedingung beginnt bei `1000 Pa`.

**Velocity (inflow)** gibt die normale Darcy-Geschwindigkeit durch die Fläche vor.
Ein **positiver** Wert ist ein Zufluss in den porösen Körper, ein **negativer** ein
Abfluss (Entnahme). Der Wert ist ein Fluss je Flächeneinheit, der Gesamtstrom
wächst also mit der Fläche.

**Volumetric flow rate (inflow)** gibt den **gesamten** Volumenstrom durch die
Fläche in `m^3/s` vor. Der Löser teilt ihn durch die Fläche der ganzen Entität und
wendet das Ergebnis als gleichmäßige Zuflussgeschwindigkeit an, sodass der
Volumenstrom bei Neuvernetzung oder Vergrößerung der Fläche erhalten bleibt.
Positiv ist wieder ein Zufluss.

Keine der Zuflussbedingungen legt den Druck fest: Der Druck am Einlass ist Teil des
Ergebnisses. Wo eine Zuflussfläche und eine Fläche mit *Pressure (explicit)*
Knoten teilen, gewinnt an diesen Knoten der Druck.

Eine Zuflussfläche braucht **weder Material noch Dicke**: Die Bedingung allein
macht sie zum Teil des Systems. Hat eine Fläche ein poröses Material und eine
Dicke, leitet sie zusätzlich Strömung in ihrer Ebene.

Die Bedingung *Pressure (implicit)* des Lösers für inkompressible Strömung wird
für diesen Problemtyp nicht angeboten.

Alle Komponenten akzeptieren eine Tabelle über der Zeit, die bei eingeschaltetem
Zeitlöser wirksam wird, siehe 1.10.

### 1.5 Schwerkraft

Die Schwerkraft geht über die Umgebungsbedingung *Gravitational acceleration* ein.
**Jede Entität trägt sie standardmäßig mit `g = (0, 0, -9.80665) m/s^2`**, die
Schwerkraft zeigt also entlang der negativen Z-Achse, solange sie nicht geändert
wird. Zwei Folgen sind wissenswert:

- In einem Modell, dessen Z-Achse nach oben zeigt, erhält ein ruhendes Fluid
  automatisch einen hydrostatischen Druck, der um `rho*|g|` je Meter Tiefe
  zunimmt. Das ist meist erwünscht.
- Bei einem Modell, in dem eine andere Achse nach oben zeigt, muss die Bedingung
  an jeder porösen Entität angepasst werden, sonst „fällt" das Fluid seitwärts.
  Setzen Sie alle drei Komponenten auf null, um die Schwerkraft abzuschalten -
  etwa für ein waagerechtes Modell, in dem nur die Druckdifferenz interessiert.

Schwerkraft zusammen mit einer nicht waagerechten Fläche *Pressure (explicit)*
erfordert Sorgfalt: Ein gleichmäßiger Druck auf einer senkrechten Fläche ist
**nicht** hydrostatisch, das Fluid zirkuliert also in ihrer Nähe. Geben Sie den
Druck entweder höhenabhängig vor (teilen Sie die Fläche in mehrere Entitäten) oder
schalten Sie die Schwerkraft ab, wenn sie nicht Teil der Fragestellung ist.

### 1.6 Umgebungs-, Anfangs- und Kopplungsbedingungen

#### Umgebungsbedingungen

| Umgebungsbedingung | Komponenten | Wirkung |
|---|---|---|
| Gravitational acceleration | g in X, Y, Z `[m/s^2]` | Schwerkraftterm des Darcy-Gesetzes, siehe 1.5 |
| Temperature | Temperature `[K]` | angeboten, aber nicht gelesen - siehe unten |

Die Umgebungsbedingung *Temperature* wird angeboten, weil Materialeigenschaften
Tabellen über der Temperatur sind. Der Löser liest sie **nicht**: Er übernimmt
Elementtemperaturen nur aus den gemeinsamen Daten einer *Heat transfer*-Aufgabe
und wertet sonst jede Tabelle bei `293.15 K` aus.

#### Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Pressure | angeboten, nicht gelesen |
| Velocity | angeboten, nicht gelesen |

Die Strömung ist stationär, das Ergebnis hängt also nicht von einem Startfeld ab,
und der Löser liest keine der beiden Anfangsbedingungen. Beide werden angeboten,
weil der Problemtyp sie mit dem Problem der inkompressiblen Strömung teilt.

#### Kopplung mit anderen Lösern

| Richtung | Variable | Wirkung |
|---|---|---|
| Wärme -> Strömung durch poröse Medien | Temperature | wählt die Zeile jeder temperaturabhängigen Materialtabelle |

Druck und Geschwindigkeit werden von keinem anderen Löser verwendet. Die Strömung
durch poröse Medien lässt sich mit jeder anderen Physik kombinieren außer
*Incompressible viscous flow* und *Modal analysis*.

Der Löser meldet sich bedingungslos als konvergiert, eine Aufgabengruppe, die nur
eine Aufgabe der Strömung durch poröse Medien enthält, endet also immer nach einer
Iteration.

### 1.7 Lösen des Systems

Der Löser assembliert und löst

```
K * p = f
```

`K` ist symmetrisch positiv definit, sobald mindestens ein Druck vorgegeben ist,
und wird daher mit dem Verfahren der **konjugierten Gradienten** mit
Jacobi-Vorkonditionierung gelöst. Die Assemblierung ist über die Elemente mit
Puffern je Thread parallelisiert, die am Ende einmal zusammengeführt werden; das
Ergebnis hängt also nicht von der Anzahl der Threads ab.

Die Permeabilitäten realer Materialien reichen von etwa `1e-9 m^2` (Kies) bis
`1e-20 m^2` (ungestörter Ton), was die Matrixeinträge nahe an die Rundungsgrenzen
des iterativen Lösers brächte. Vor dem Lösen wird daher jede Gleichung durch das
größte `k/mu` des Modells geteilt. Das Teilen aller Gleichungen durch dieselbe Zahl
ändert die Lösung nicht; es hält nur die Iteration gut konditioniert.

Ein Modell **ohne Randbedingung Pressure (explicit)** hat keine eindeutige Lösung.
Der Löser erkennt dies und bricht mit dem Fehler *Flow through porous media
requires at least one 'Pressure (explicit)' boundary condition* ab.

Weitere Wege zu einem singulären oder leeren System, alle ohne Meldung:

- eine Entität, deren Material eine der drei erforderlichen Eigenschaften fehlt,
  wird nicht gelöst, und ihre Knoten fallen aus dem System;
- eine Entität mit Permeabilität oder Viskosität null hat keine Steifigkeit, ihre
  Knoten bleiben also im System, sind aber mit nichts verbunden;
- eine Fläche mit Dicke null oder eine Linie mit Querschnitt null leitet keine
  Strömung.

### 1.8 Abgeleitete Ergebnisse

Nach der Lösung wird die Elementgeschwindigkeit aus dem Knotendruckfeld
zurückgewonnen. Der Druckgradient wird über die Integrationspunkte des Elements
gemittelt und mit dem Schwerkraftterm kombiniert; für Linien- und Flächenelemente
wird er in globale Koordinaten gedreht:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Ergebnis | Angewendet auf | Bedeutung |
|---|---|---|
| Pressure `[Pa]` | Knoten | das gelöste Porendruckfeld |
| Velocity `[m/s]` | Element | Vektor der Darcy-Geschwindigkeit |

Die Geschwindigkeit ist eine **Dichte** im selben Sinne wie die Feldergebnisse der
anderen Löser: Sie hängt nicht von der Elementgröße ab, ist in einem linearen
Element konstant und konvergiert bei Netzverfeinerung. Randflächen mit Dicke null
zeigen die Geschwindigkeit null - zeigen Sie das Ergebnis auf dem Volumen an.

Ein Gesamtvolumenstrom wird nicht als Ergebnis gespeichert. Berechnen Sie ihn von
Hand aus der Geschwindigkeit und der Fläche eines Querschnitts oder aus den
Zuflussbedingungen, siehe 5.

Nach dem Schreiben des Datensatzes gibt das Protokoll des Lösers die Statistik
beider Variablen und die Werte an allen Überwachungspunkten aus.

### 1.9 Gültigkeit des Darcy-Gesetzes

Das Darcy-Gesetz gilt für langsame, laminare Strömung, in der die viskosen Kräfte
die Trägheit des Fluids überwiegen. Ein übliches Kriterium verwendet die mit dem
mittleren Korndurchmesser `d` gebildete Reynolds-Zahl:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

Grundwasserströmung in Böden, Sickerströmung durch Dämme und Deiche, Strömung durch
Filter, Schüttungen und die meisten porösen technischen Werkstoffe erfüllen es mit
großem Abstand. Strömung durch groben Kies, Steinschüttungen oder sehr offene
Schäume bei hohen Geschwindigkeiten nicht; das Darcy-Gesetz unterschätzt dann den
Druckverlust (Forchheimer-Bereich), und dieser Löser liefert eine zu große
Geschwindigkeit.

### 1.10 Stationäre Strömung und der Zeitlöser

Der Problemtyp erlaubt den Zeitlöser, die Strömung selbst ist aber **stationär**:
Fluid und poröser Körper sind inkompressibel, und es gibt keinen Speicherterm. Bei
eingeschaltetem Zeitlöser löst jeder Zeitschritt einen eigenen stationären Zustand
mit den Bedingungswerten zu diesem Zeitpunkt. Das ist das richtige Modell, wenn
sich die Randbedingungen langsam ändern im Vergleich zur Zeit, die der Druck zum
Einstellen braucht - ein Beckenwasserstand über einem Sandfilter, der sich über
Stunden ändert - und es erlaubt einer Aufgabe der Strömung durch poröse Medien,
einen Aufgabenablauf mit einer instationären Wärmeaufgabe zu teilen.

Konsolidierung, Entwässerung eines ungesättigten Bodens, kompressible
Gasströmung und jede andere Erscheinung, bei der sich die in den Poren
gespeicherte Fluidmenge mit der Zeit ändert, liegen außerhalb des Modells, siehe
Abschnitt 6.

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Dock **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Umgebungsbedingungen`, `Material`,
`Ergebnisse`), im Menü **Problem** und im Dock **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog des
Aufgabenablaufs. Fügen Sie eine Aufgabe hinzu und wählen Sie **Flow through porous
media**.

Der Problemtyp lässt sich nicht mit *Incompressible viscous flow* oder *Modal
analysis* kombinieren. Eine davor platzierte Aufgabe *Heat transfer* lässt die
Materialtabellen der berechneten Temperatur folgen.

### 2.2 Registerkarte Problem

Die Registerkarte zeigt die Gruppe *Zeitlöser*, weil der Problemtyp den Zeitlöser
unterstützt - was eine instationäre Rechnung hier bedeutet, erklärt 1.10. Die
Strömung durch poröse Medien hat keine eigene Einstellungsgruppe. Alles wird je
Entität in den Registerkarten der Bedingungen zugewiesen.

### 2.3 Registerkarte Material

Die Registerkarte `Material` zeigt das der ausgewählten Entität zugewiesene
Material. Die Strömung durch poröse Medien erfordert **alle drei** dieser
Eigenschaften:

| Eigenschaft | Einheit | Bemerkung |
|---|---|---|
| Permeability to fluids | `m^2` | intrinsische Permeabilität des Feststoffs, siehe Tabelle unten |
| Dynamic viscosity | `kg/(m*s)` | des Porenfluids, `1.003e-3` für Wasser bei 20 °C |
| Density | `kg/m^3` | des Porenfluids, `998.2` für Wasser bei 20 °C |

Die mitgelieferte Bibliothek enthält gebrauchsfertige wassergesättigte poröse
Materialien. Jedes trägt die temperaturabhängige Dichte und Viskosität von Wasser
und eine typische Permeabilität:

| Material | Permeabilität `[m^2]` | Durchlässigkeitsbeiwert bei 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

Die Permeabilität eines realen Bodens kann um eine Größenordnung oder mehr von
diesen Werten abweichen - sie sind Ausgangswerte, keine Daten eines bestimmten
Standorts. Um ein anderes Fluid zu modellieren, kopieren Sie eines davon und
ersetzen Dichte und Viskosität. Das einfache Material *Sand* ist ein trockenes
Schüttgut ohne Viskosität und wird daher für diesen Problemtyp als ungültig
markiert.

### 2.4 Registerkarte Randbedingungen

Die Registerkarte ist oben in eine Liste der verfügbaren Randbedingungen und unten
in einen Editor der ausgewählten Bedingung geteilt. Wählen Sie zuerst eine
Flächenentität im Baum `Modell`; die Liste bietet dann die Bedingungen aus 1.4
an. Haken Sie eine an, um sie zuzuweisen, und bearbeiten Sie ihre Komponenten im
unteren Baum.

- **Pressure (explicit)** hält den Druck am Auslass. Ein Modell ohne sie wird
  nicht gelöst.
- **Velocity (inflow)** und **Volumetric flow rate (inflow)** treiben einen
  bekannten Strom in das Modell. Positive Werte fließen hinein, negative hinaus.
- Flächen ohne Bedingung sind undurchlässig.

### 2.5 Anfangs- und Umgebungsbedingungen

Die Registerkarte `Umgebungsbedingungen` bietet *Gravitational acceleration*, die
jede Entität bereits mit `(0, 0, -9.80665) m/s^2` trägt (siehe 1.5), und
*Temperature*, die dieser Löser ignoriert.

Die Registerkarte `Anfangsbedingungen` bietet *Pressure* und *Velocity*, die dieser
Löser nicht liest.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Löser für die Strömung durch poröse Medien verwendet den Eintrag **CG**.
Konvergiert die Lösung nicht, suchen Sie zuerst nach einer Entität mit
Permeabilität null oder einem isolierten Punktelement statt an den
Lösereinstellungen.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Sonden an gegebenen
Koordinaten und wählt die aufzuzeichnende Variable - *Pressure* oder *Velocity*.
`Bericht` -> `Überwachungspunkte` zeigt die Werte.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet die berechneten Variablen und steuert die
3D-Ansicht. Der Druck ist ein Knotenskalar, die Geschwindigkeit ein
Elementvektor. Zeigen Sie die Geschwindigkeit als Vektorfeld auf dem Volumen an,
um die Strömungswege zu sehen.

Die Registerkarte `Datensätze` des Docks `Modell` listet die Ergebnisdatensätze:
einen bei stationärer Rechnung, einen je Ausgabezeitschritt bei eingeschaltetem
Zeitlöser.

`Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige Ausgabe des Lösers
einschließlich der Statistik beider Variablen.

---

## 3. Tutorial - druckgetriebene Strömung durch einen Sandfilter

**Ziel.** Die durch eine bekannte Druckdifferenz getriebene Wasserströmung durch
einen waagerechten Sandfilter berechnen und den Volumenstrom mit dem Darcy-Gesetz
prüfen.

Dieses Tutorial setzt ein vernetztes Volumenmodell eines Quaders voraus - verwenden
Sie `Datei` -> `Neues Modell`, zeichnen Sie einen Quader mit `Geometrie` ->
`Element erstellen` oder importieren Sie eine Geometrie und vernetzen Sie sie mit
Tetraedern; das Tutorial *Draw cube* behandelt dies. Markieren Sie die beiden
Endflächen als getrennte Flächenentitäten, damit ihnen Bedingungen zugewiesen
werden können.

Der Filter sei `1 m` lang entlang der X-Achse mit einem Querschnitt von `0.1 m`
mal `0.1 m`.

### Schritt 1 - Problem auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine Aufgabe hinzu und wählen Sie **Flow through porous media**.
3. Bestätigen Sie mit `OK`.

Lassen Sie den *Zeitlöser* ausgeschaltet - das Ergebnis ist ein einziger
stationärer Zustand.

### Schritt 2 - Material zuweisen

1. Wählen Sie die Volumenentität im Baum `Modell`.
2. Öffnen Sie die Registerkarte `Material` und weisen Sie **Coarse Sand (water
   saturated)** zu.

Das Material trägt die Permeabilität `1e-10 m^2` des Sandes sowie Dichte und
Viskosität von Wasser.

### Schritt 3 - Schwerkraft abschalten

Der Filter liegt waagerecht, und nur die Druckdifferenz interessiert, also
entfernen Sie die Standardschwerkraft:

1. Öffnen Sie bei ausgewählter Volumenentität die Registerkarte
   `Umgebungsbedingungen`.
2. Wählen Sie **Gravitational acceleration** und setzen Sie alle drei Komponenten
   auf `0`.

Mit eingeschalteter Schwerkraft trüge das Wasser zusätzlich einen hydrostatischen
Druck über die Höhe von `0.1 m` des Filters, und die gleichmäßigen Drücke auf den
senkrechten Endflächen ließen es in deren Nähe zirkulieren - siehe 1.5.

### Schritt 4 - beide Enden vorgeben

1. Wählen Sie die Flächenentität bei `x = 0`.
2. Öffnen Sie die Registerkarte `Randbedingungen`, haken Sie **Pressure
   (explicit)** an und setzen Sie `10000` Pa - etwa ein Meter Wassersäule.
3. Wählen Sie die Flächenentität bei `x = 1 m`, haken Sie **Pressure (explicit)**
   an und setzen Sie `0` Pa.

Die vier Seitenflächen erhalten keine Bedingung: Sie sind undurchlässige Wände.

### Schritt 5 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.

Die Rechnung ist eine einzige Lösung mit konjugierten Gradienten.

### Schritt 6 - Ergebnisse ansehen

- Registerkarte `Ergebnisse`: Zeigen Sie **Pressure** an. Der Druck fällt linear
  von `10000` Pa auf `0` Pa entlang des Filters, mit ebenen Isobaren senkrecht zu
  seiner Achse.
- Zeigen Sie **Velocity** an. Sie zeigt in +X-Richtung und ist in jedem Element
  gleich.
- Ein Überwachungspunkt auf *Pressure* in der Filtermitte zeigt `5000` Pa.

### Schritt 7 - Nachrechnen von Hand

Mit `k = 1e-10 m^2` und `mu = 1.003e-3 kg/(m*s)` für Wasser bei 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

Das Ergebnis **Velocity** muss `9.97e-4 m/s` zeigen. Die Reynolds-Zahl mit einer
Korngröße von `1 mm` beträgt `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, an der Grenze
des Darcy-Bereichs (1.9). Bei einer Porosität von `0.35` bewegt sich das Wasser
selbst mit `9.97e-4 / 0.35 = 2.8e-3 m/s` und braucht etwa sechs Minuten durch den
Filter.

### Fehlersuche

| Symptom | Ursache |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | nirgends ist ein Auslassdruck vorgegeben |
| *Entity has material assigned which is missing required properties* | dem Material fehlt eine Viskosität - oft ein trockenes Bodenmaterial wie *Sand* |
| Der Druck ändert sich über die Höhe und die Geschwindigkeit hat eine Z-Komponente | die Standardschwerkraft ist noch eingeschaltet, siehe Schritt 3 |
| Die Geschwindigkeit ist überall null | beide Enden haben denselben Druck oder die Permeabilität ist null |

---

## 4. Tutorial - aufwärts gerichtete Sickerströmung in einer senkrechten Säule

**Ziel.** Wasser mit bekanntem Volumenstrom aufwärts durch eine senkrechte
Sandsäule drücken und sehen, wie sich der Einlassdruck in einen hydrostatischen
und einen Reibungsanteil aufteilt.

Verwenden Sie einen Quader von `0.1 m` mal `0.1 m` Grundfläche und `1 m` Höhe
entlang der **Z**-Achse, mit der unteren Fläche (`z = 0`) und der oberen Fläche
(`z = 1 m`) als getrennten Flächenentitäten.

### Schritt 1 - Problem und Material

1. Wählen Sie **Flow through porous media** wie in Tutorial 3.
2. Weisen Sie dem Volumen **Coarse Sand (water saturated)** zu.

### Schritt 2 - Schwerkraft beibehalten

Die Säule steht entlang Z, die Standardschwerkraft `(0, 0, -9.80665) m/s^2` zeigt
also in die richtige Richtung. Lassen Sie die Bedingung *Gravitational
acceleration* unverändert.

### Schritt 3 - Randbedingungen

1. Wählen Sie die obere Fläche, haken Sie **Pressure (explicit)** an und setzen
   Sie `0` Pa: das freie Wasser über der Säule.
2. Wählen Sie die untere Fläche, haken Sie **Volumetric flow rate (inflow)** an
   und setzen Sie `1e-6` m^3/s. Über die `0.01 m^2` der Fläche entspricht das
   einer Zuflussgeschwindigkeit von `1e-4 m/s`.

Die obere Fläche ist waagerecht, ein gleichmäßiger Druck auf ihr ist also mit der
Schwerkraft verträglich.

### Schritt 4 - lösen und ansehen

`Lösung` -> `Löser starten` (`Ctrl+R`), dann **Pressure** und **Velocity**
anzeigen.

- Die Geschwindigkeit beträgt in jedem Element `1e-4 m/s` nach oben (+Z).
- Der Druck steigt linear von `0` Pa oben bis zu seinem Maximum unten.

### Schritt 5 - den Einlassdruck aufteilen

Der Druck am Boden muss die Wassersäule anheben und sie durch den Sand drücken:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Ein Überwachungspunkt an der unteren Fläche bestätigt die Summe. Setzen Sie den
Volumenstrom auf null und lösen Sie erneut: Die Geschwindigkeit verschwindet, und
der Bodendruck fällt auf rein hydrostatische `9789 Pa` - der Löser reproduziert ein
ruhendes Fluid exakt.

Erhöhen Sie den Volumenstrom, bis der Reibungsanteil das Gewicht des Sandes unter
Auftrieb erreicht, etwa `(rho_s - rho) * g * (1 - n) * L`: Das ist der Beginn der
Fluidisierung (Treibsand), die das starre Gerüst dieses Modells nicht abbilden
kann - der Löser berechnet einfach einen immer größeren Druck.

### Fehlersuche

| Symptom | Ursache |
|---|---|
| Der Bodendruck ist nur der Reibungsanteil | die Schwerkraft ist auf dem Volumen abgeschaltet |
| Der Bodendruck ist nur der hydrostatische Anteil | der Zufluss liegt auf der falschen Fläche oder sein Wert ist null |
| Die Geschwindigkeit zeigt seitwärts | die Säule wurde entlang einer anderen Achse als Z aufgebaut; passen Sie die Schwerkraftbedingung an |

---

## 5. Überprüfung eines Modells

**Gleichmäßige Säule.** Für einen Körper der Länge `L`, der auf zwei
gegenüberliegenden Flächen auf `p1` und `p2` gehalten wird, aus einem Material und
mit abgeschalteter Schwerkraft, ist der Druck entlang des Körpers linear und

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

Das Geschwindigkeitsergebnis muss bis auf die Lösertoleranz übereinstimmen, und der
Druck in der Mittelebene muss der Mittelwert der beiden vorgegebenen Werte sein.

**Hydrostatik.** Ein Körper mit Schwerkraft, mit einer Bedingung *Pressure
(explicit)* nur auf seiner waagerechten Oberseite und sonst undurchlässigen
Wänden, muss die Geschwindigkeit null und einen um `rho*|g|` je Meter Tiefe
wachsenden Druck zeigen. Das prüft Dichte und Schwerkraftbedingung zugleich.

**Strombilanz.** In einem stationären Modell muss alles eintretende Fluid auch
austreten. Bei einem über *Volumetric flow rate (inflow)* vorgegebenen Zufluss
muss die Geschwindigkeit in einem Querschnitt mal dessen Fläche den vorgegebenen
Strom ergeben. Lesen Sie die Geschwindigkeit in der Säule mit einem
Überwachungspunkt ab.

**Schichten in Reihe.** Für zwei Schichten der Länge `L1`, `L2` und Permeabilität
`k1`, `k2` in Reihe zwischen zwei vorgegebenen Drücken verhält sich die Strömung
wie zwei Widerstände in Reihe:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          Druck an der Grenzfläche
```

Anders als der Elektrostatiklöser teilt dieser den Druck nach den
**Permeabilitäten** auf, wie es ein Widerstandsnetzwerk täte.

**Radiale Strömung zu einem Brunnen.** Für einen gespannten Grundwasserleiter der
Mächtigkeit `H` zwischen einem Brunnen mit Radius `rw` auf `pw` und einem äußeren
Radius `R` auf `pR` ist der Druck logarithmisch, und die Förderrate des Brunnens
beträgt

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Das prüft den Löser an einem ungleichmäßigen Feld und zeigt, ob das Netz am
Brunnen fein genug ist.

Verfeinern Sie das Netz mindestens einmal und vergewissern Sie sich, dass der Druck
konvergiert und nicht driftet.

---

## 6. Einschränkungen

- **Stationäre, inkompressible, einphasige Strömung.** Es gibt keinen
  Speicherterm, Konsolidierung, kompressible Gasströmung und die instationäre
  Antwort eines gespannten Grundwasserleiters werden also nicht modelliert. Eine
  instationäre Rechnung ist eine Folge stationärer Zustände, siehe 1.10.
- **Nur vollständig gesättigte Medien.** Es gibt keine freie Oberfläche
  (Sickerlinie) und keine ungesättigte Zone. Die Sickerströmung durch einen Damm,
  dessen Grundwasserspiegel erst gefunden werden muss, liegt außerhalb des Modells,
  sofern der gesättigte Bereich nicht explizit vernetzt wird.
- **Starrer poröser Körper.** Das Gerüst verformt sich nicht, es gibt also keine
  Kopplung zur Spannungsanalyse, keine effektive Spannung, keine Konsolidierung
  und keine Fluidisierung.
- **Isotrope Permeabilität.** Die Permeabilität ist ein Skalar. Geschichtete Böden
  mit unterschiedlicher horizontaler und vertikaler Permeabilität müssen als
  getrennte Schichten modelliert werden.
- **Nur Darcy-Bereich.** Trägheitsbehaftete (Forchheimer-) und turbulente
  Strömung in sehr groben Medien wird nicht modelliert, siehe 1.9.
- **Randbedingungen nur auf Flächen.** Drücke und Zuflüsse werden auf
  Flächenentitäten vorgegeben; es gibt keine Sickerfläche, keinen Brunnen oder
  Linienquelle und keinen vorgegebenen Druck auf einer Linie oder einem Punkt.
- **Kein Ergebnis für den Gesamtvolumenstrom.** Volumenströme durch einen
  Querschnitt müssen von Hand aus der Geschwindigkeit berechnet werden.
- **Anfangsbedingungen werden nicht gelesen**, und die Umgebungsbedingung
  *Temperature* hat keine Wirkung; Materialtabellen folgen der Temperatur nur über
  eine Wärmeaufgabe früher im Aufgabenablauf.
- **Ein Modell braucht einen vorgegebenen Druck.** Ohne ihn bricht der Löser mit
  einem Fehler ab.
