# Wärmeübertragung - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Wärmelöser (`RSolverHeat`) von Range FEA, der den
Problemtyp **Wärmeübertragung** (*Heat transfer*) bedient: die Gleichungen, die
er löst, die Bedeutung jeder Eingabe, die er akzeptiert, die Teile der
grafischen Benutzeroberfläche, die ihn steuern, und zwei ausgearbeitete
Tutorials.

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - stationäre Wärmeübertragung](#3-tutorial---stationäre-wärmeübertragung)
4. [Tutorial - transiente Wärmeübertragung](#4-tutorial---transiente-wärmeübertragung)
5. [Überprüfung eines Modells](#5-überprüfung-eines-modells)
6. [Einschränkungen](#6-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - das Temperaturfeld

Die primäre Unbekannte ist die **Temperatur** `T` in jedem Knoten, in Kelvin.
Jeder Knoten trägt einen einzigen Freiheitsgrad, sodass ein Netz mit `N` Knoten
ein System aus `N` Gleichungen ergibt. Der Wärmestrom wird nach der Lösung aus
dem Temperaturfeld gewonnen.

Die Temperatur ist ein Skalar, wodurch ein Wärmemodell deutlich günstiger ist
als ein Strukturmodell auf demselben Netz: eine Unbekannte pro Knoten statt drei
und eine Matrix mit einem Neuntel der Einträge.

Die Standardtemperatur eines Knotens, den weder eine Bedingung noch ein
vorheriges Ergebnis berührt, ist `293.15 K`.

### 1.2 Grundgleichung

Der Löser implementiert **lineare Wärmeleitung** mit konvektiven Rändern. Die
Energiebilanz eines Körpers mit der Leitfähigkeit `k`, der Dichte `rho` und der
spezifischen Wärmekapazität `c` lautet

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

und bei deaktiviertem Zeitlöser entfällt der transiente Term, sodass die
stationäre Bilanz bleibt

```
div( k * grad(T) ) + q = 0
```

Wärme verlässt eine Fläche durch Konvektion gemäß dem Newtonschen
Abkühlungsgesetz,

```
qs = h * ( T - Tf )
```

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `T` | Temperatur | `K` | wird gelöst |
| `q` | Wärmequellendichte | `W/m^3` | Bedingung Heat, Joulesche Wärme, Strahlung |
| `k` | Wärmeleitfähigkeit | `W/(m*K)` | Materialeigenschaft |
| `c` | Wärmekapazität | `J/(kg*K)` | Materialeigenschaft |
| `rho` | Dichte | `kg/m^3` | Materialeigenschaft |
| `h` | Konvektionskoeffizient | `W/(m^2*K)` | Konvektionsbedingung, siehe 1.5 |
| `Tf` | Fluidtemperatur | `K` | Konvektionsbedingung |
| `Q` | Wärmestrom | `W/m^2` | abgeleitet, siehe 1.9 |

Es gibt keinen Advektionsterm. Ein Wärmeübertragungsproblem transportiert
Energie durch Wärmeleitung innerhalb des Körpers und durch Konvektion über
seinen Rand; es transportiert keine Energie mit einem strömenden Medium.
Verwenden Sie dafür *Heat transfer in fluids*.

**Erforderliche Materialeigenschaften.** Ein Wärmeproblem benötigt **vier**
davon:

| Eigenschaft | Einheiten | Verwendet für |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | die Leitungsmatrix |
| Heat capacity | `J/(kg*K)` | die Kapazitätsmatrix, nur transient |
| Density | `kg/m^3` | die Kapazitätsmatrix, nur transient |
| Emissivity | `-` | nur Wärmeübertragung durch Strahlung |

Eine Entität, deren Material eine der vier fehlt, wird **nicht gelöst**, auch
nicht bei einem rein stationären Leitungslauf, der weder die Dichte noch den
Emissionsgrad betrachtet. Die Einstellungsprüfung warnt vor dem Start des Laufs
davor. Die einzige Ausnahme ist eine Entität mit einer thermischen
Randbedingung: Diese Entität wird unabhängig von ihrem Material in das System
aufgenommen, und ihre Leitfähigkeit und Kapazität sind dann einfach null.

Jede Eigenschaft ist eine Tabelle über der Temperatur. Der für ein Element
verwendete Wert wird aus der Tabelle bei der **Elementtemperatur des vorherigen
Laufs** gelesen, sodass eine temperaturabhängige Leitfähigkeit um eine Iteration
nachläuft. Abschnitt 1.6 erklärt, wie man sie konvergieren lässt.

### 1.3 Finite-Elemente-Diskretisierung

Die schwache Form der Energiebilanz ergibt für jedes Element

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(`conduction` = Wärmeleitung, `convection` = Konvektion, `capacity matrix` =
Kapazitätsmatrix, `sources` = Quellen), wobei `B` aus den Ableitungen der
Formfunktionen aufgebaut ist und `N` die Formfunktionen selbst sind. Jede
Elementmatrix ist für ein Element mit `n` Knoten `n x n` groß - eine Zeile pro
Knoten, nicht drei.

Die Konvektionsterme werden **nur auf Flächenelementen** assembliert und nur
dort, wo eine Konvektionsrandbedingung zugewiesen ist. Die
Konvektionssteifigkeit ist die konsistente Form `INT( N^T * h * N )`, während
die Konvektionslast gleichmäßig auf die Knoten des Elements konzentriert wird.
Auf einem vernünftigen Netz sind beide hinreichend konsistent, und der
Unterschied verschwindet bei Verfeinerung des Netzes.

**Elementmaße.** Wie überall in Range FEA erhalten niederdimensionale Entitäten
ein geometrisches Maß, damit sie reale Körper darstellen können:

| Entität | Maß | Wo es gesetzt wird |
|---|---|---|
| Volumen | 1 | - |
| Fläche | Flächendicke | geometrische Eigenschaften der Entität |
| Linie | Querschnittsfläche der Linie | geometrische Eigenschaften der Entität |
| Punkt | Punktvolumen | geometrische Eigenschaften der Entität |

Eine **Fläche** leitet Wärme in ihrer eigenen Ebene, durch einen Querschnitt
gleich ihrer Dicke, und speichert Energie proportional zu dieser Dicke. Es gibt
keinen Temperaturgradienten durch die Dicke: Ein Flächenelement ist einen Knoten
dick und daher konstruktionsbedingt über seine Dicke isotherm. Eine Fläche mit
**Dicke null** leitet daher nichts und speichert nichts, trägt aber weiterhin
Konvektion - genau das, was für die Randfläche eines vernetzten Volumens
gewünscht ist, wo die Fläche nur existiert, um die Konvektionsbedingung zu
tragen.

Eine **Linie** leitet entlang ihrer eigenen Richtung durch ihre
Querschnittsfläche. Ein **Punkt** hat keine Richtung, entlang derer er leiten
könnte, und trägt daher überhaupt keine Wärmeleitung bei: Er trägt nur eine
Kapazität, proportional zu seinem Punktvolumen, und eine Wärmequelle. Ein Punkt
ist eine konzentrierte thermische Masse, kein Leiter.

Die Kapazitätsmatrix wird nur assembliert, wenn sie benötigt wird, also bei
einer transienten Analyse.

### 1.4 Quellen und Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Temperature | explizit (Dirichlet) | Punkt, Linie, Fläche, Volumen | Temperature `[K]` |
| Heat | natürlich, Quelle | Punkt, Linie, Fläche, Volumen | Heat `[W]`, siehe unten |
| Simple convection | natürlich (Robin) | Fläche | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | natürlich, Korrelation | Fläche | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | natürlich, Korrelation | Fläche | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | natürlich, Quelle | Fläche | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | natürlich, Quelle | Volumen | Heat per unit volume `[W/m^3]` |

**Temperature** ist die einzige explizite Bedingung des Wärmeproblems. Jeder
Knoten der Entität, der sie zugewiesen ist, wird aus dem Gleichungssystem
entfernt und auf dem angegebenen Wert gehalten; sein Beitrag wird auf die rechte
Seite der benachbarten Gleichungen übertragen. Ein stationäres Modell benötigt
mindestens eine davon oder mindestens eine Konvektionsfläche - siehe 1.7.

Wo sich Entitäten überlappen, **gewinnt die niederdimensionale Entität**: Eine
auf einer Fläche vorgeschriebene Temperatur überschreibt in den gemeinsamen
Knoten eine, die auf dem dahinterliegenden Volumen vorgeschrieben ist, eine
Linie überschreibt eine Fläche, und ein Punkt überschreibt alles. Anders als die
Verschiebungsrandbedingungen des Strukturlösers werden Temperaturen nicht
kombiniert - es gibt pro Knoten nur einen zu haltenden Freiheitsgrad, daher
setzt sich einfach die zuletzt gelesene Bedingung durch.

**Heat** ist eine Wärmequelle, und ihr Wert ist die **Gesamtleistung in `W`**,
die in die Entität eingebracht wird, der sie zugewiesen ist, unabhängig von der
Dimension dieser Entität. Der Löser benötigt eine Quellendichte, daher wird die
Gesamtmenge vor der Assemblierung über das Maß der Entität verteilt:

| Entität | Dichte, die der Löser anwendet |
|---|---|
| Volumen | Wert / Volumen der Entität, `W/m^3` |
| Fläche | Wert / Fläche der Entität, `W/m^2` |
| Linie | Wert / Länge der Entität, `W/m` |
| Punkt | Wert / Anzahl der Punktelemente, jeweils `W` |

Die eingebrachte Leistung ist daher genau die eingegebene Zahl - sie ändert sich
nicht mit der Größe der Entität, mit der Netzverfeinerung oder mit dem Maßstab
des Modells. Nur Elemente, die der Löser berechnen kann, erhalten einen Anteil,
daher wird eine Entität ohne Materialeigenschaften übersprungen und ihre
Bedingung stattdessen gemeldet. Eine flächige Wärmequelle wird nicht mit der
Flächendicke skaliert.

**Heat rate (unit area)** und **Heat rate (unit volume)** schreiben denselben
Quellterm als Dichte statt als Gesamtmenge vor: `W/m^2` über die Fläche einer
Flächenentität und `W/m^3` über das Volumen einer Volumenentität. Greifen Sie zu
ihnen, wenn die Quelle natürlicherweise eine Dichte ist - ein Heizelement mit
Leistungsangabe pro Quadratmeter -, und zu *Heat*, wenn sie natürlicherweise
eine Gesamtmenge ist, etwa ein Element bekannter Leistung. Alle summieren sich
auf einer Entität, die mehr als eine trägt, und alle akzeptieren eine Tabelle
über der Zeit, sodass ein Heizelement während eines transienten Laufs ein- und
ausgeschaltet werden kann.

**Konvektions**bedingungen gelten für Flächen und fügen sowohl einen
Steifigkeitsterm als auch eine Last hinzu, sodass sie als Robin-Randbedingung
und nicht als fester Wärmestrom wirken: Steigt die Wandtemperatur, steigt die
Wärme, die die Wand abgibt. Alle drei Varianten enden im selben Paar `(h, Tf)`;
sie unterscheiden sich nur darin, woher `h` stammt:

- **Simple convection** übernimmt `h` und `Tf` direkt aus den beiden
  eingegebenen Werten. Verwenden Sie sie, wann immer der Konvektionskoeffizient
  aus einem Handbuch, einer Messung oder einer früheren Berechnung bekannt ist.
  Sie ist die einzige der drei, die vollständig unter Ihrer Kontrolle steht.
- **Forced convection** hat zwei Modi. An einer Wand zwischen einem vernetzten
  Festkörper und einem vernetzten Fluid koppelt sie, sobald eine Aufgabe *Heat
  transfer in fluids* ein Ergebnis geliefert hat, die beiden Lösungen direkt -
  es ist keine Korrelation beteiligt, da die aufgelöste Strömung die Konvektion
  bereits enthält. Der Löser für Wärme in Fluiden übergibt für jedes Element der
  Wand das Paar `(h, Tf)`, das den Wärmestrom reproduziert, den seine eigene
  Lösung durch die Wand führt; siehe *Konjugierte Wärmeübertragung* in 1.6.
  Überall sonst - auf einer Fläche, die an kein vernetztes Fluid grenzt, oder im
  ersten Durchlauf eines gekoppelten Laufs, bevor die Wärmeaufgabe für Fluide
  gelaufen ist - wird `h` aus einer Korrelation für die äußere Strömung entlang
  einer ebenen Platte berechnet, mit den Fluideigenschaften, dem hydraulischen
  Durchmesser, der *Fluid temperature* und der *Velocity*, die Sie eingeben. Das
  Protokoll gibt an, welcher der beiden Modi verwendet wird.
- **Natural convection** berechnet `h` aus einer Korrelation für horizontale
  Platten, mit den Fluideigenschaften, dem hydraulischen Durchmesser und der
  Temperaturdifferenz zwischen Wand und Fluid.

Werden derselben Fläche mehrere Konvektionsbedingungen zugewiesen, addieren sie
sich nicht: Simple wird zuerst gelesen, Forced überschreibt sie, und Natural
überschreibt beide.

### 1.5 Die Konvektionskorrelationen

Beide berechneten Formen bilden aus den mit der Randbedingung eingegebenen
Fluideigenschaften die üblichen dimensionslosen Kennzahlen, mit dem
hydraulischen Durchmesser `d` als charakteristischer Länge:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

und wandeln die Nusselt-Zahl in einen Konvektionskoeffizienten um als

```
h = Nu * k / d
```

| Form | Korrelation | Nusselt-Zahl |
|---|---|---|
| Erzwungen | äußere Strömung, ebene Platte | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Natürlich | horizontale Platte, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Natürlich | horizontale Platte, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

`k`, `c`, `rho`, `mu` und `beta` in diesen Ausdrücken sind die mit der
Randbedingung eingegebenen **Fluid**eigenschaften, nicht die
Materialeigenschaften des Festkörpers.

Beide Bedingungen werden mit den Eigenschaften **trockener Luft bei 20 Grad
Celsius und 101.325 kPa** erstellt, sodass das Fluid für Sie beschrieben ist und
nur Strömung und Geometrie einzugeben bleiben:

| Komponente | Standard |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, der Wert des idealen Gases `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Ersetzen Sie sie für jedes andere Fluid - es sind die Eigenschaften des Fluids,
nicht das Material der Wand. *Natural convection* ist daher so, wie zugewiesen,
lösungsbereit; *Forced convection* benötigt noch eine **Geschwindigkeit**, die
kein Standardwert erraten kann - es sei denn, die Fläche grenzt an ein
vernetztes Fluid, wo keiner dieser Werte gelesen wird.

Jede dieser Kennzahlen dividiert durch `mu`, `k` oder `d`, und `Re` und `Pr`
fallen auf null, wenn `rho`, `c` oder `v` es tun, was `h = 0` und eine Fläche
ergäbe, die nie abkühlt. Ein Wert von null wird daher als Fehler behandelt und
nicht überdeckt: Der Löser hält an und nennt die Komponente und die Entität, zum
Beispiel

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

Die vom Strömungslöser berechnete Geschwindigkeit wird nie in die Korrelation
eingespeist. Eine Wand mit Haftbedingung hält das Fluid in Ruhe, sodass die
Geschwindigkeit neben ihr null ist, und die Korrelation würde die
Geschwindigkeit der freien Strömung und die mittlere Fluidtemperatur benötigen,
die eine allgemeine dreidimensionale Strömung nicht definiert. Eine an ein
vernetztes Fluid grenzende Wand wird stattdessen mit der Fluidlösung gekoppelt.

Drei Folgen sollte man im Blick behalten:

- Die Korrelation ist **fest**. Der Löser berücksichtigt weder die Ausrichtung
  der Fläche noch, ob die Strömung innen oder außen verläuft oder ob die
  Grenzschicht laminar oder turbulent ist. Alles geht über den hydraulischen
  Durchmesser ein, der die charakteristische Länge der Korrelation ist - er ist
  Ihr einziger Einfluss auf die Geometrie.
- Ein hydraulischer Durchmesser von null würde `h = 0` ergeben, also eine
  isolierte Wand, daher hält der Löser an, statt ihn zu akzeptieren.
- Die natürliche Konvektion hängt von der **Wandtemperatur** ab, die aus der
  Elementtemperatur der **vorherigen** Lösung übernommen wird. Beim ersten Lauf
  einer stationären Analyse ist das die Anfangstemperatur, sodass der
  Koeffizient bei der falschen Temperaturdifferenz ausgewertet wird. Legen Sie
  die Aufgabe in eine Aufgabengruppe mit mehreren Iterationen, um ihn
  konvergieren zu lassen, oder verwenden Sie *Simple convection* mit einem selbst
  berechneten Koeffizienten. Eine verschwindende Temperaturdifferenz ergibt
  `Nu = 1`.

### 1.6 Umgebungs-, Anfangs- und Kopplungsbedingungen

#### Umgebungsbedingungen

| Umgebungsbedingung | Komponenten | Wirkung |
|---|---|---|
| Temperature | Temperature `[K]` | Hintergrundtemperatur der Entität, nur erster Lauf |

#### Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Temperature | Anfangstemperaturfeld, beim ersten Lauf angewendet |

Umgebungsbedingungen, Anfangsbedingungen und Randbedingungen werden in dieser
Reihenfolge gelesen, und jede überschreibt die vorherige, daher **gewinnt eine
Randbedingung gegenüber einer Anfangsbedingung, die gegenüber einer
Umgebungsbedingung gewinnt**. Umgebungs- und Anfangswerte werden **nur beim
ersten Lauf** gelesen; danach schreitet das Feld vom vorherigen Ergebnis aus
fort.

#### Kopplung mit anderen Lösern

Der Wärmelöser verwendet und erzeugt Ergebnisse, die mit dem übrigen
Aufgabenablauf geteilt werden.

| Richtung | Variable | Wirkung |
|---|---|---|
| Strahlung -> Wärme | Heat - radiation part | wird zum Quellterm der **Flächen**elemente addiert |
| Elektrostatik -> Wärme | Joule heat | wird zum Quellterm jedes Elements addiert |
| Spannung -> Wärme | Displacement | das Netz wird für die Wärmelösung verformt und danach wiederhergestellt |
| Wärme -> Spannung | Temperature | treibt den Wärmeausdehnungsterm des Strukturlösers an |
| Wärme -> jeder Löser | Temperature | wählt die Zeile jeder temperaturabhängigen Materialtabelle |
| Wärme in Fluiden -> Wärme | Wand `(h, Tf)` | treibt die an ein vernetztes Fluid grenzenden Wände mit *Forced convection* an |
| Wärme -> Wärme in Fluiden | Temperatur des Festkörpers | hält dieselben Wände in der Fluidlösung auf der Temperatur des Festkörpers |

**Wärmeübertragung durch Strahlung** ist ein eigener Problemtyp mit eigenem
Löser. Er erfordert eine Aufgabe *Heat transfer*, löst ein
Radiositätssystem eines Hohlraums über Flächenpatches mithilfe von
Sichtfaktoren und dem Emissionsgrad des Materials und gibt das Ergebnis als
flächige Wärmequelle zurück. Legen Sie beide Aufgaben in eine Aufgabengruppe und
lassen Sie die Gruppe iterieren: Der Strahlungslöser meldet Konvergenz und
beendet die Gruppe, wenn sich die Wärme seiner Patches nicht mehr ändert.

**Joulesche Wärme** kommt auf dieselbe Weise aus einer Aufgabe
*Electro-statics*, die vor der Wärmeaufgabe steht, sodass Widerstandserwärmung
keine weitere Einrichtung erfordert.

Sofern er nicht mit einem Fluid gekoppelt ist, meldet der Wärmelöser sich
bedingungslos als konvergiert, sodass eine Aufgabengruppe, die nur eine
Wärmeaufgabe enthält, immer nach einer Iteration anhält. Iterieren lohnt sich,
wenn etwas, von dem die Wärmelösung abhängt, selbst nachläuft - Strahlungswärme,
ein Koeffizient der natürlichen Konvektion oder eine stark temperaturabhängige
Leitfähigkeit.

#### Konjugierte Wärmeübertragung

Der Wärmelöser löst **nur Festkörper**. Ein Volumen, dessen Material ein Fluid
ist - ein Gas oder eine Flüssigkeit oder ein Material ohne angegebenen Zustand,
das eine dynamische Viskosität besitzt, was jedes Fluid der Materialdatenbank
abdeckt -, ist nie Teil der Wärmelösung, unabhängig von seinen Eigenschaften oder
Bedingungen. Ebenso wenig eine Punkt-, Linien- oder Flächenentität im Inneren des
Fluids, etwa ein Einlass; eine Bedingung darauf, die nur der Wärmelöser liest,
wird im Protokoll gemeldet. Das Fluidgebiet gehört zum Problemtyp *Heat transfer
in fluids*, der die Wärme mit der Strömung transportiert.

Beide treffen an Wänden mit der Bedingung **Forced convection** aufeinander. Mit
beiden Aufgaben in einer Aufgabengruppe - die Wärmeaufgabe für Fluide zuerst -
geschieht in jedem Durchlauf der Gruppe Folgendes:

1. Der Löser für Wärme in Fluiden hält die Wandknoten auf der Temperatur, die
   der Wärmelöser zuletzt im Festkörper berechnet hat (im allerersten Durchlauf,
   wenn es noch keine solche Temperatur gibt, ist die Wand isoliert), löst das
   Fluid und ermittelt den in jedem Wandknoten in das Fluid eintretenden
   Wärmestrom. Dieser Wärmestrom ist das Residuum der Fluidgleichungen im
   Knoten, dasselbe, das eine einzige Lösung beider Gebiete sähe, sodass eine
   dünne thermische Grenzschicht keine Genauigkeit kostet.
2. Für jedes Wandelement übergibt er das Paar `(h, Tf)`. `h = k * G` ist der
   Leitwert des ersten Fluidelements hinter der Wand - bei einem Tetraeder `k`
   geteilt durch die Höhe des Elements über der Wand -, und `Tf` wird so gesetzt,
   dass `q = h * (Tw - Tf)` der Wärmestrom aus Schritt 1 ist.
3. Der Wärmelöser wendet sie auf die Wand an wie eine Bedingung *Simple
   convection*, löst den Festkörper und gibt seine Wandtemperatur zurück.

Das Paar `(h, Tf)` ist ein numerisches Hilfsmittel, das beide Lösungen in
Übereinstimmung bringt - `h` ist nicht der Wärmeübergangskoeffizient eines
Handbuchs und ändert sich mit dem Netz. Was konvergiert, sind die Temperatur und
der Wärmestrom durch die Wand, also die des gekoppelten Problems. Über die
Geschwindigkeit des Fluids neben der Wand wird nichts benötigt, weshalb eine
Wand mit Haftbedingung - an der die Strömung ruht - Wärme genau so überträgt,
wie sie soll.

Die an das Fluid übergebene Wandtemperatur wird mit einem **Aitken-Faktor**
relaxiert, der aus den letzten beiden Durchläufen berechnet wird. Ohne ihn kann
ein schlecht leitender Festkörper neben einem gut aufgelösten Fluid Hunderte von
Durchläufen benötigen; mit ihm pendelt sich die Gruppe typischerweise in wenigen
ein. Beide Löser melden, solange sie gekoppelt sind, die relative Änderung ihres
Temperaturfelds als Konvergenz; setzen Sie daher den Konvergenzwert der
Aufgabengruppe - `1e-6` ist ein vernünftiger Anfang - und geben Sie ihr genug
Iterationen. Ein Konvergenzwert von null führt alle aus.

### 1.7 Stationäre Analyse

Bei deaktiviertem Zeitlöser assembliert und löst der Löser

```
K * T = f
```

`K` ist symmetrisch positiv definit, sobald mindestens eine Temperatur
vorgeschrieben ist, und wird daher mit dem Verfahren der **konjugierten
Gradienten** mit Jacobi-Vorkonditionierung gelöst.

Ein stationäres Modell **ohne Randbedingung Temperature und ohne
Konvektionsfläche** ist das thermische Gegenstück zu einer ungelagerten
Struktur: Energie kann hineinfließen, hat aber keinen Weg hinaus, die Matrix ist
singulär, und die Lösung konvergiert nicht. Jedes stationäre Modell benötigt
mindestens einen Pfad zu einer bekannten Temperatur.

### 1.8 Transiente Analyse

Bei aktiviertem Zeitlöser schreitet der Löser fort mit

```
M * dT/dt + K * T = f
```

nach einem Theta-Verfahren, wobei `alpha` der Koeffizient der Approximation des
Zeitschrittverfahrens ist:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

Die Einstellung *Approximation* des Zeitlösers wählt `alpha`:

| Approximation | `alpha` | Resultierendes Verfahren | Stabilität |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, ein impliziter Schritt | unbedingt |
| Central difference (accurate) | `0.5` | Crank-Nicolson | unbedingt, kann oszillieren |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, ein expliziter Schritt | bedingt |

**Backward difference** ist der sichere Standard für ein Wärmemodell. Er kostet
eine Matrixlösung pro Schritt, aber der Schritt ist durch die gewünschte
Genauigkeit begrenzt und nicht durch die Größe des kleinsten Elements.
**Central difference** ist genau zweiter Ordnung und die bessere Wahl, sobald
der Zeitschritt klein genug ist, um den Übergang aufzulösen, kann aber bei einem
abrupten Einschalten nachschwingen. **Forward difference** setzt die gesamte
Leitungsmatrix auf den alten Zeitschritt, was jeden Schritt günstig und den Lauf
anfällig macht.

Der Vorwärtszweig ist nur stabil, solange der Zeitschritt ungefähr unter

```
dt < rho * c * h^2 / ( 2 * k )
```

bleibt, wobei `h` die kleinste Elementgröße ist. Da die
Temperaturleitfähigkeit `k/(rho*c)` eines Metalls in der Größenordnung von
`1e-5 m^2/s` liegt, erzwingt bereits ein millimetergroßes Element einen
Zeitschritt von deutlich unter einer Zehntelsekunde. Verwenden Sie für alles
außer einer sehr feinen Zeitauflösung das implizite Verfahren.

Ein instabiler transienter Lauf ist leicht zu erkennen: Das Temperaturfeld
oszilliert von Schritt zu Schritt mit wachsender Amplitude, oft zwischen
physikalisch unmöglichen Werten, statt sich einzupendeln. Abhilfe schafft ein
kleinerer Zeitschritt oder das implizite Verfahren.

### 1.9 Abgeleitete Ergebnisse

Nach der Lösung wird der Element-Wärmestrom aus dem Temperaturfeld als
Mittelwert über die Integrationspunkte von

```
Q = -k * grad(T)
```

gewonnen, im Element ausgewertet und für Linien- und Flächenelemente in globale
Koordinaten gedreht. Auf einer Fläche mit Konvektion wird der konvektive
Wärmestrom

```
Qh = h * ( Tf - T )
```

entlang der Flächennormale addiert, sodass der Wärmestromvektor einer
konvektiven Seite in die Richtung zeigt, in der die Wärme tatsächlich abfließt.

Der Wärmestrom ist eine **Dichte** in `W/m^2`, unabhängig von der Größe des
Elements, in dem er berechnet wurde, sodass er zwischen Elementen und mit einer
Handrechnung verglichen werden kann und bei Verfeinerung des Netzes konvergiert.

Der **Wärmeübergangskoeffizient** wird ebenfalls gespeichert. Er ist der
Koeffizient, den der Löser auf jedem Flächenelement tatsächlich verwendet hat,
und damit die einzige Möglichkeit zu sehen, was die Korrelationen *Forced
convection* und *Natural convection* ergeben haben - und auf einer Fläche mit
natürlicher Konvektion variiert er von Element zu Element, da er von der lokalen
Wandtemperatur abhängt. An einer mit einem Fluid gekoppelten Wand ist er der
Leitwert des ersten Fluidelements, der vom Netz abhängt - siehe *Konjugierte
Wärmeübertragung* in 1.6. Auf jedem Element ohne Konvektionsbedingung ist er
null.

| Ergebnis | Anwendbar auf | Bedeutung |
|---|---|---|
| Temperature `[K]` | Knoten | das gelöste Temperaturfeld |
| Heat flux `[W/m^2]` | Element | Vektor des Leitungswärmestroms, auf konvektiven Flächen plus konvektiver Anteil |
| Heat transfer coefficient `[W/(m^2*K)]` | Element | der verwendete Konvektionskoeffizient, null, wo keine Konvektion vorliegt |

Nach jedem Datensatz gibt das Löserprotokoll die Statistik aller drei und die
Werte an allen Überwachungspunkten aus.

### 1.10 Stationär oder transient?

| | Stationär | Transient |
|---|---|---|
| Löst | `K*T = f` | `M*T' + K*T = f` in der Zeit |
| Beantwortet | „wie heiß wird es am Ende“ | „wie lange dauert es, bis es so weit ist“ |
| Verwendet Dichte und Wärmekapazität | nein, sie müssen aber trotzdem vorhanden sein | ja |
| Benötigt eine vorgeschriebene Temperatur oder eine konvektive Fläche | ja | nein, eine Kapazität genügt |
| Datensätze | einer | einer pro geschriebenem Zeitschritt |

**Verwenden Sie eine stationäre Analyse für** die endgültige
Temperaturverteilung eines dauerhaft betriebenen Bauteils, die Auslegung eines
Kühlkörpers, eine Wärmebrücke in einer Wand oder das Temperaturfeld, das eine
thermische Spannungsanalyse speist. Das ist der Standard und deckt die meisten
Arbeiten ab.

**Verwenden Sie eine transiente Analyse für** Aufheizen und Abkühlen, einen
Lastzyklus, einen Thermoschock, Abschrecken oder alles, bei dem es Ihnen um die
Zeitkonstante des Bauteils geht.

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Umgebungsbedingungen`, `Material`,
`Ergebnisse`), im Menü **Problem** und im Panel **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Fügen Sie eine Aufgabe hinzu und wählen Sie **Heat
transfer** - „Steady-state and transient Heat-transfer“ (stationäre und
transiente Wärmeübertragung).

Wärmeübertragung lässt sich frei mit jeder anderen Physik außer *Modal analysis*
kombinieren. Wissenswerte Ketten sind:

- *Heat transfer*, dann *Stress analysis* - thermische Spannungen;
- *Heat transfer* und *Radiative heat transfer* in einer iterierenden
  Aufgabengruppe - Wärmeleitung plus Strahlung im Hohlraum;
- *Electro-statics*, dann *Heat transfer* - Widerstandserwärmung.

Um ein gekoppeltes Paar zu iterieren, fügen Sie eine Aufgabengruppe hinzu und
setzen Sie ihre Anzahl von Iterationen; die Gruppe hält vorzeitig an, wenn jede
darin enthaltene Aufgabe Konvergenz meldet.

### 2.2 Registerkarte Problem

**Zeitlöser** - wird für die Wärmeübertragung angezeigt, da sie ein
zeitabhängiger Problemtyp ist. Lassen Sie ihn für eine stationäre Analyse
deaktiviert.

| Feld | Bedeutung |
|---|---|
| Aktivieren | schaltet das Zeitschrittverfahren ein und aus |
| Approximation | Theta des Zeitschrittverfahrens: rückwärts, zentral oder vorwärts, siehe 1.8 |
| Startzeit | erster Zeitwert in Sekunden; wird bei einem Neustart des Lösers ignoriert |
| Endzeit | `start time + time-step size * number of time-steps`, schreibgeschützt |
| Zeitschrittweite | `dt` in Sekunden |
| Anzahl der Zeitschritte | wie viele Schritte berechnet werden |
| Ausgabefrequenz | schreibt alle N Schritte eine Ergebnisdatei; `0` schreibt nur den letzten Schritt |

Ein Wärmeproblem hat kein eigenes Einstellungs-Gruppenfeld. Alles andere wird
pro Entität auf den Registerkarten der Bedingungen zugewiesen.

### 2.3 Registerkarte Material

Die Registerkarte `Material` listet das der ausgewählten Entität zugewiesene
Material auf. Wärmeübertragung benötigt **alle vier** dieser Eigenschaften:

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, etwa `50` für Baustahl, `15` für Edelstahl, `200` für Aluminium |
| Heat capacity | `J/(kg*K)` | `c`, etwa `460` für Stahl |
| Density | `kg/m^3` | `rho`, etwa `7850` für Stahl |
| Emissivity | `-` | nur für Wärmeübertragung durch Strahlung verwendet, aber dennoch erforderlich |

Eine Entität, der eine davon fehlt, wird nicht gelöst, und die
Einstellungsprüfung warnt vor dem Start des Laufs. Wenn Sie keine
Wärmeübertragung durch Strahlung rechnen, genügt ein Emissionsgrad von `0`, um
die Anforderung zu erfüllen.

Das Material muss außerdem ein **Festkörper** sein. Ein Volumen, dessen Material
ein Gas oder eine Flüssigkeit ist - oder keinen Zustand gesetzt hat und eine
dynamische Viskosität besitzt -, wird dem Problemtyp *Heat transfer in fluids*
überlassen, auch wenn es alle vier Eigenschaften hat.

Jede Eigenschaft ist eine Tabelle über der Temperatur, daher wird eine
temperaturabhängige Leitfähigkeit berücksichtigt - ausgewertet bei der
Elementtemperatur des vorherigen Laufs, weshalb es sich lohnt, die Aufgabe zu
iterieren, wenn die Abhängigkeit stark ist.

### 2.4 Registerkarte Randbedingungen

Die Registerkarte ist oben in eine Verwaltungsliste der verfügbaren
Randbedingungen und unten in einen Editor für die ausgewählte Bedingung
aufgeteilt. Wählen Sie zuerst eine Entität im Baum `Modell` aus; die Verwaltung
bietet dann die für diesen Entitätstyp und das ausgewählte Problem gültigen
Bedingungen an. Aktivieren Sie eine, um sie zuzuweisen, und bearbeiten Sie dann
ihre Komponenten im unteren Baum.

Es werden die Bedingungen aus Abschnitt 1.4 angeboten. Hinweise zu ihrer
Verwendung:

- **Temperature** ist die eine Bedingung, die einen Wert festhält, statt ihn
  anzutreiben. Weisen Sie sie der Seite, Kante oder dem Punkt zu, dessen
  Temperatur Sie kennen.
- **Heat** wird für jeden Entitätstyp angeboten und nimmt die Gesamtleistung in
  `W` an; der Löser verteilt sie für Sie über die Entität. Sie ist die Bedingung
  der Wahl, wenn Sie die Leistung dessen kennen, was die Wärme erzeugt.
- **Heat rate (unit volume)** auf einem Volumen und **Heat rate (unit area)** auf
  einer Fläche schreiben dieselbe Quelle stattdessen als Dichte vor, für eine
  Quelle, deren Leistung natürlicherweise pro Volumen- oder Flächeneinheit
  angegeben wird.
- Die drei **Konvektions**bedingungen gelten nur für Flächen. Weisen Sie *Simple
  convection* zu, sofern Sie keinen besonderen Grund haben, den Löser den
  Koeffizienten für Sie korrelieren zu lassen; sie ist die vorhersagbare.
  *Forced convection* an einer an ein vernetztes Fluid grenzenden Wand koppelt
  den Festkörper an die Wärmelösung des Fluids und liest keine ihrer
  Komponenten; anderswo korreliert sie den Koeffizienten aus ihnen.
- Jede Komponente ist eine Tabelle über der Zeit, sodass einer vorgeschriebenen
  Temperatur oder einer Wärmequelle für einen transienten Lauf ein Zeitverlauf
  gegeben werden kann. So werden ein Lastzyklus oder ein hochgefahrenes
  Heizelement modelliert.

### 2.5 Anfangs- und Umgebungsbedingungen

Die Registerkarte `Anfangsbedingungen` bietet *Temperature* an, die das
Ausgangsfeld eines transienten Laufs festlegt. Weisen Sie sie dem gesamten
Modell zu und geben Sie ihr die Temperatur, mit der das Bauteil startet.

Die Registerkarte `Umgebungsbedingungen` bietet ebenfalls *Temperature* an, als
Hintergrundwert für eine Entität. Beide werden nur beim ersten Lauf gelesen, und
eine Randbedingung überschreibt beide.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Wärmelöser verwendet den Eintrag **CG**.

Konvergiert eine stationäre Lösung nicht, ist die übliche Ursache ein Modell
ohne vorgeschriebene Temperatur und ohne konvektive Fläche und keine
Lösereinstellung. Prüfen Sie die Randbedingungen, bevor Sie die Anzahl der
Iterationen erhöhen.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable - für ein
Wärmeproblem *Temperature*. In einem transienten Lauf wird der Verlauf über
`Bericht` -> `Überwachungspunkte` angezeigt, was der schnellste Weg ist, eine
Aufheizkurve oder eine Zeitkonstante aus einem Modell abzulesen.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet die berechneten Variablen auf und steuert
die 3D-Ansicht. Die Temperatur ist ein Knotenskalar, der Wärmestrom ein
Elementvektor und der Wärmeübergangskoeffizient ein Elementskalar, der abseits
der konvektiven Flächen null ist.

Die Registerkarte `Datensätze` des Panels `Modell` listet die
Ergebnisdatensätze auf:

- **stationäre Analyse** - ein einziger Datensatz;
- **transiente Analyse** - ein Datensatz pro geschriebenem Zeitschritt. Das
  Animieren der Datensätze spielt das Aufheizen ab.

`Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige Ausgabe des
Lösers, einschließlich der Statistik von Temperatur, Wärmestrom und
Wärmeübergangskoeffizient nach jedem Datensatz.

---

## 3. Tutorial - stationäre Wärmeübertragung

**Ziel.** Die Temperaturverteilung eines Stabs berechnen, der an einem Ende heiß
gehalten und von der umgebenden Luft gekühlt wird, und die Temperatur am kalten
Ende ablesen.

Dieses Tutorial setzt ein vernetztes Volumenmodell voraus - ein langer Quader
eignet sich gut. Verwenden Sie `Datei` -> `Neues Modell`, zeichnen Sie einen
Quader mit `Geometrie` -> `Element erstellen` oder importieren Sie eine
Geometrie und tetraedrisieren Sie sie; das Tutorial *Würfel zeichnen* behandelt
diese Schritte. Markieren Sie die heiße Stirnfläche und die übrigen Seiten als
separate Flächenentitäten, damit ihnen Bedingungen zugewiesen werden können.

### Schritt 1 - Problem auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine Aufgabe hinzu und wählen Sie **Heat transfer**.
3. Bestätigen Sie mit `OK`.

### Schritt 2 - Material zuweisen

1. Wählen Sie die Volumenentität im Baum `Modell` aus.
2. Öffnen Sie die Registerkarte `Material` und weisen Sie ein Material mit allen
   vier Eigenschaften zu, für Baustahl:
   - **Thermal conductivity** = `50` `W/(m*K)`
   - **Heat capacity** = `460` `J/(kg*K)`
   - **Density** = `7850` `kg/m^3`
   - **Emissivity** = `0`

Wärmekapazität und Dichte werden von einer stationären Lösung nicht verwendet,
müssen aber vorhanden sein, damit die Entität überhaupt gelöst wird.

### Schritt 3 - Analyse stationär belassen

Öffnen Sie die Registerkarte `Problem` und lassen Sie im Gruppenfeld
*Zeitlöser* **Aktivieren** deaktiviert. Der Löser assembliert und löst dann
einmal `K*T = f`.

### Schritt 4 - heißes Ende vorschreiben

1. Wählen Sie die Flächenentität am heißen Ende aus.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie
   **Temperature**.
3. Setzen Sie den Wert auf `353.15` K, also 80 Grad Celsius.

Denken Sie daran, dass alle Temperaturen in Range FEA absolut sind, in Kelvin.

### Schritt 5 - den Rest kühlen

1. Wählen Sie die Flächenentität aus, die die übrigen Seiten bedeckt.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie **Simple
   convection**.
3. Setzen Sie **Convection coefficient** = `10` `W/(m^2*K)` und **Fluid
   temperature** = `293.15` K.

Ein Koeffizient von etwa `10` ist typisch für eine Fläche in ruhender Luft;
erzwungene Luftströmung liegt eher bei `50`, Wasser bei Hunderten oder
Tausenden.

Dieser Schritt macht das Modell lösbar. Ohne ihn hat der Stab eine Wärmezufuhr
und keinen Abfluss, und ein stationärer Zustand existiert nicht.

### Schritt 6 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Die Einstellungsprüfung meldet fehlende Materialien oder fehlende
   Randbedingungen vor dem Start des Laufs.
3. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.

### Schritt 7 - Ergebnisse untersuchen

- Registerkarte `Ergebnisse`: Zeigen Sie **Temperature** an. Das Feld sollte vom
  vorgeschriebenen heißen Ende gleichmäßig zur Fluidtemperatur hin abfallen.
- Der kälteste Punkt sollte über der Fluidtemperatur bleiben. Eine Temperatur
  irgendwo unter `Tf` oder über den vorgeschriebenen `353.15` K bedeutet, dass
  etwas Energie in die falsche Richtung zuführt - prüfen Sie das Vorzeichen
  jeder Wärmequelle.
- **Heat flux** zeigt, wohin die Energie fließt, in `W/m^2`. Multiplizieren Sie
  den Wert auf den konvektiven Seiten mit ihrer Fläche und vergleichen Sie ihn
  mit der in den Stab eintretenden Leistung.
- **Heat transfer coefficient** sollte auf den konvektiven Seiten `10` und
  überall sonst `0` anzeigen. Das ist der schnellste Weg zu bestätigen, dass
  eine Konvektionsbedingung die gemeinten Elemente erreicht hat, und der einzige
  Weg zu sehen, was eine korrelierte Bedingung berechnet hat.
- Ein Überwachungspunkt auf *Temperature* am kalten Ende liefert die Antwort als
  Zahl statt als Farbe.

### Schritt 8 - optional, Wärmequelle hinzufügen

Um ein im Stab eingebettetes Heizelement zu modellieren, wählen Sie die
Volumenentität aus, öffnen Sie die Registerkarte `Randbedingungen` und
aktivieren Sie **Heat**. Um insgesamt `P` Watt in den Stab einzubringen, geben
Sie `P` ein - der Löser verteilt es über das Volumen der Entität.

Verwenden Sie stattdessen **Heat rate (unit volume)**, wenn die Leistung der
Quelle pro Volumeneinheit angegeben ist; geben Sie `P / V` `W/m^3` ein, um
dieselbe Gesamtleistung einzubringen - siehe 1.4.

Die stationäre Temperatur muss dann so weit steigen, bis die konvektive Fläche
genau diese Leistung abgeben kann - was die Prüfung der Energiebilanz in
Abschnitt 5 ist.

### Schritt 9 - optional, thermische Spannung

Um das Ergebnis in eine Strukturanalyse zu übernehmen:

1. Fügen Sie im Aufgabenablauf des Problems *nach* der Wärmeaufgabe eine Aufgabe
   **Stress analysis** hinzu und geben Sie der Entität die benötigten
   strukturmechanischen Eigenschaften.
2. Weisen Sie der Strukturentität eine Umgebungsbedingung **Temperature** mit
   der spannungsfreien Bezugstemperatur zu, zum Beispiel `293.15` K.

Der Spannungslöser übernimmt die berechneten Elementtemperaturen und fügt den
Wärmeausdehnungsterm automatisch hinzu.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die Lösung konvergiert nicht | keine Bedingung Temperature und keine konvektive Fläche, daher ist der stationäre Zustand undefiniert |
| *Entity has material assigned which is missing required properties* | eine der vier erforderlichen Eigenschaften fehlt - oft der Emissionsgrad |
| Das gesamte Modell liegt auf einer Temperatur | es tritt keine Wärme ein, oder nichts treibt einen Gradienten vom vorgeschriebenen Wert weg |
| Die Temperaturen sind absurd hoch | eine Dichte in `W/m^3` wurde in eine Bedingung *Heat* eingegeben, die die Gesamtleistung in Watt erwartet, oder der Konvektionskoeffizient ist viel zu klein |
| Eine Wand zwischen Festkörper und Fluid kühlt nicht | die Wärmeaufgabe für Fluide läuft nach der Wärmeaufgabe, oder die Gruppe führt nur eine Iteration aus - setzen Sie die Wärmeaufgabe für Fluide an die erste Stelle und lassen Sie die Gruppe iterieren |
| Das Fluid wird von der Wärmelösung ignoriert | beabsichtigt - ein Fluidmaterial wird nur von *Heat transfer in fluids* gelöst |
| Die Temperaturen liegen überall nahe `293.15` K | die Entität wurde nicht gelöst - prüfen Sie das Protokoll auf eine fehlende Materialeigenschaft |

---

## 4. Tutorial - transiente Wärmeübertragung

**Ziel.** Denselben Stab bei Raumtemperatur starten, das heiße Ende einschalten
und herausfinden, wie lange das Aufheizen dauert.

### Schritt 1 - Problem auswählen

Dieselbe Aufgabe **Heat transfer** wie in Tutorial 3. Ist das Modell noch
eingerichtet, müssen nur die Einstellungen des Zeitlösers und die
Anfangsbedingung hinzugefügt werden.

### Schritt 2 - Material zuweisen

Dieselben vier Eigenschaften. Die **Wärmekapazität** und die **Dichte** spielen
hier eine Rolle: Zusammen mit der Leitfähigkeit bestimmen sie die
Temperaturleitfähigkeit `k/(rho*c)`, die bestimmt, wie schnell die Front
voranschreitet.

### Schritt 3 - Anfangstemperatur setzen

1. Wählen Sie die Volumenentität aus.
2. Öffnen Sie die Registerkarte `Anfangsbedingungen` und aktivieren Sie
   **Temperature**.
3. Setzen Sie sie auf `293.15` K.

Ohne sie startet das Modell trotzdem bei den standardmäßigen `293.15` K, aber
die ausdrückliche Angabe macht das Modell selbstdokumentierend und erlaubt es,
von jedem anderen Zustand aus zu starten.

### Schritt 4 - Zeitlöser konfigurieren

Öffnen Sie die Registerkarte `Problem` und stellen Sie im Gruppenfeld
*Zeitlöser* Folgendes ein:

| Feld | Wert |
|---|---|
| Aktivieren | aktiviert |
| Approximation | `Backward difference (stable)` |
| Startzeit | `0` |
| Zeitschrittweite | `1` |
| Anzahl der Zeitschritte | `300` |
| Ausgabefrequenz | `10` |

*Backward difference* ist das implizite Verfahren - siehe die Tabelle in 1.8 -,
und es ist hier das richtige. Es bleibt bei einem Zeitschritt stabil, der nach
der Physik und nicht nach dem Netz gewählt ist.

Wählen Sie den Zeitschritt anhand der erwarteten Zeitkonstante und zielen Sie
auf etwa fünfzig Schritte über den interessanten Teil der Kurve. Abschnitt 5
gibt eine Formel für eine erste Abschätzung. Das Schreiben jedes zehnten
Schritts hält die Ergebnisdatei klein, ohne die Form der Kurve zu verlieren.

### Schritt 5 - Randbedingungen beibehalten

Die vorgeschriebene **Temperature** am heißen Ende und die **Simple convection**
auf den übrigen Seiten werden unverändert übernommen. Jede Komponente ist eine
Tabelle über der Zeit, sodass dem heißen Ende stattdessen eine Rampe gegeben
werden kann: Öffnen Sie den Komponenteneditor und fügen Sie Zeitpunkte hinzu,
damit die Temperatur über die ersten Sekunden ansteigt, statt sofort
umzuspringen.

### Schritt 6 - Überwachungspunkt hinzufügen

`Problem` -> `Überwachungspunkte definieren`, platzieren Sie einen Punkt am
kalten Ende und setzen Sie seine Variable auf **Temperature**. Die Aufheizkurve
lässt sich als Verlauf weit leichter ablesen als aus einer Folge farbiger
Bilder.

### Schritt 7 - lösen

`Lösung` -> `Löser starten` (`Ctrl+R`). Das Protokoll gibt einen Block pro
Zeitschritt aus:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

gefolgt von den Iterationen des Matrixlösers und der Temperaturstatistik dieses
Schritts.

### Schritt 8 - Ergebnisse untersuchen

- Panel `Modell`, Registerkarte `Datensätze`: ein Datensatz pro geschriebenem
  Schritt. Gehen Sie sie schrittweise durch oder animieren Sie sie, um zu sehen,
  wie die Wärmefront in den Stab wandert.
- Registerkarte `Ergebnisse`: Zeigen Sie **Temperature** an. Fixieren Sie den
  Anzeigebereich über alle Datensätze, damit die Farben in jedem Bild dasselbe
  bedeuten, sonst skaliert sich die Animation selbst neu, und der Fortschritt
  ist unsichtbar.
- `Bericht` -> `Überwachungspunkte` zeichnet die Aufheizkurve am kalten Ende.
  Lesen Sie daraus die Zeitkonstante ab: die Zeit, in der etwa 63 Prozent des
  gesamten Anstiegs erreicht sind.
- Der letzte Datensatz sollte sich dem stationären Ergebnis aus Tutorial 3
  nähern. Tut er das nicht, war der Lauf zu kurz.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die Temperatur oszilliert und wächst von Schritt zu Schritt | das explizite Verfahren mit zu großem Zeitschritt - stellen Sie die Approximation auf *Backward difference (stable)* um oder verringern Sie `dt` |
| Das Ergebnis ändert sich über den ganzen Lauf kaum | der Zeitschritt liegt weit unter der Zeitkonstante, oder der Lauf ist zu kurz |
| Der erste Schritt springt auf das Endergebnis | der Zeitschritt liegt weit über der Zeitkonstante, sodass das Verfahren effektiv stationär ist |
| Es wurde nur ein Datensatz geschrieben | die Ausgabefrequenz ist `0`, wodurch nur der letzte Schritt geschrieben wird |
| Der transiente Lauf erreicht nie den stationären Zustand | das ist oft korrekt - vergleichen Sie mit dem stationären Lauf, bevor Sie den Löser verdächtigen |

---

## 5. Überprüfung eines Modells

Der Löser hat keine eingebaute Verifikationssuite, daher lohnt es sich, ein neues
Modell an etwas zu validieren, das Sie von Hand berechnen können, bevor Sie ihm
vertrauen. Vier günstige Prüfungen, nach Nützlichkeit geordnet:

**Ebene Wand.** Für eine Platte der Dicke `L` und der Leitfähigkeit `k`, die auf
ihren beiden Seiten auf `T1` und `T2` gehalten wird, ist die Temperatur über die
Platte linear, und der Wärmestrom beträgt

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Modellieren Sie die Platte mit Volumenelementen, schreiben Sie beide Seiten vor
und bestätigen Sie, dass die Mittelebene genau auf halbem Weg liegt und der
berechnete **Heat flux** mit `q` übereinstimmt. Dies deckt falsche Einheiten,
eine falsche Leitfähigkeit und ein Netz auf, das nicht so verbunden ist, wie Sie
denken. Verfeinern Sie das Netz einmal und bestätigen Sie, dass beide Zahlen
gleich bleiben.

**Wand mit Konvektion.** Für dieselbe Platte, die auf einer Seite auf `T1`
gehalten und auf der anderen von einem Fluid mit `Tf` und einem Koeffizienten
`h` gekühlt wird, stellt sich die gekühlte Oberfläche ein auf

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Vergleichen Sie mit der berechneten Oberflächentemperatur. Dies validiert die
Konvektionsbedingung, den Teil eines Wärmemodells, bei dem man am leichtesten
Fehler macht.

**Konzentrierte Zeitkonstante.** Für eine transiente Prüfung nehmen Sie einen
Körper mit dem Volumen `V` und der Oberfläche `A` ohne vorgeschriebene
Temperatur, der bei `T0` startet und nur durch Konvektion gekühlt wird. Solange
die Biot-Zahl `h*V/(A*k)` unter etwa `0.1` bleibt, ist der Körper nahezu
isotherm und

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Lesen Sie `tau` aus dem Verlauf des Überwachungspunkts ab und vergleichen Sie.
Dies validiert gleichzeitig die Kapazitätsmatrix, die Konvektionsbedingung und
Ihren Zeitschritt - und es ist der schnellste Weg, einen Zeitschritt für ein
reales Modell zu wählen, da `tau/50` eine gute erste Schätzung ist.

**Energiebilanz.** Für ein stationäres Modell mit bekannter Eingangsleistung `P`
muss die konvektive Fläche bei der Endtemperatur genau `P` abgeben:

```
P = h * A * ( Ts_mean - Tf )
```

Lesen Sie die mittlere Oberflächentemperatur aus der Statistik im
Löserprotokoll ab und prüfen Sie die Bilanz. Dies ist die eine globale Prüfung,
die einen als Gesamtmenge statt als Dichte eingegebenen Wert Heat aufdeckt.

Verfeinern Sie bei jeder dieser Prüfungen das Netz mindestens einmal und
bestätigen Sie, dass das Ergebnis konvergiert und nicht driftet.

---

## 6. Einschränkungen

### Einschränkungen der Modellierung

- **Nur lineare Wärmeleitung.** Leitfähigkeit, Kapazität und Dichte eines
  Elements sind während einer Lösung konstant und werden aus der
  Materialtabelle bei der Temperatur des vorherigen Laufs gelesen. Eine stark
  temperaturabhängige Eigenschaft benötigt eine iterierende Aufgabengruppe, um
  zu konvergieren.
- **Keine Phasenumwandlung.** Es gibt keine latente Wärme und keine
  Enthalpieformulierung, daher werden Schmelzen, Erstarren, Sieden und
  Kondensieren nicht modelliert. Eine Materialtabelle kann das latente Plateau
  nicht abbilden.
- **Keine Advektion.** Energie bewegt sich durch Wärmeleitung innerhalb des
  Körpers und durch Konvektion über seinen Rand. Ein bewegtes Medium führt keine
  Energie mit; verwenden Sie dafür *Heat transfer in fluids*.
- **Konvektion ist ein Randmodell.** Die korrelierten Formen verwenden jeweils
  eine feste Korrelation - äußere ebene Platte für erzwungene Strömung,
  horizontale Platten für natürliche Konvektion - mit dem hydraulischen
  Durchmesser als charakteristischer Länge. Ausrichtung der Fläche,
  Strömungsregime und ob die Strömung innen oder außen verläuft, werden nicht
  erkannt. Wenn der Koeffizient wichtig ist, berechnen Sie ihn selbst und geben
  Sie ihn mit *Simple convection* ein.
- **Der Koeffizient der natürlichen Konvektion läuft nach.** Er wird bei der
  Wandtemperatur der vorherigen Lösung ausgewertet, sodass ein stationärer Lauf
  mit einem Durchlauf effektiv die anfängliche Temperaturdifferenz verwendet.
- **Strahlung nur über den separaten Problemtyp.** Im Wärmelöser selbst gibt es
  keinen Term für den Emissionsgrad der Oberfläche; Strahlung im Hohlraum
  erfordert eine Aufgabe *Radiative heat transfer*, deren Sichtfaktormatrix und
  eine Iteration zwischen beiden.
- **Flächen sind über ihre Dicke isotherm.** Eine Flächenentität leitet nur in
  ihrer eigenen Ebene. Modellieren Sie eine Wand, deren Gradient durch die Dicke
  wichtig ist, mit Volumenelementen.
- **Punktelemente leiten nicht.** Ein Punkt ist eine konzentrierte Kapazität und
  eine Wärmequelle, mehr nicht.
- **Ein stationäres Modell benötigt einen Abfluss.** Ohne vorgeschriebene
  Temperatur und ohne konvektive Fläche ist die Matrix singulär, und die Lösung
  konvergiert nicht.
