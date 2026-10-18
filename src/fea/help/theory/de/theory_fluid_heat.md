# Wärmeübertragung in Fluiden - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Löser für Wärmeübertragung in Fluiden von Range
FEA - `RSolverFluidHeat`, der den Problemtyp **Wärmeübertragung in Fluiden**
(*Heat transfer in fluids*) bedient: die Gleichung, die er löst, die Bedeutung
jeder Eingabe, die er akzeptiert, seine Kopplung an den Wärmelöser eines
Festkörpers und die Teile der grafischen Benutzeroberfläche, die ihn steuern.

Die Wärmeübertragung in Fluiden wird auf dem Strömungsfeld einer Aufgabe
**Inkompressible viskose Strömung** (*Incompressible viscous flow*) gelöst. Der
Strömungslöser, seine Randbedingungen, seine nichtlineare Iteration und sein
Zeitschrittverfahren werden im Handbuch [Fluidströmung](theory_fluid.md)
beschrieben, der Wärmelöser für Festkörper im Handbuch
[Wärmeübertragung](theory_heat.md).

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Einschränkungen](#3-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - die Temperatur

Die primäre Unbekannte ist die **Temperatur** `T` in `K`, ein Freiheitsgrad pro
Knoten.

| Problemtyp | Löser | Primäre Unbekannte | Einheiten | Freiheitsgrade pro Knoten |
|---|---|---|---|---|
| Heat transfer in fluids | `RSolverFluidHeat` | Temperatur | `K` | 1 |

Der Problemtyp **erfordert** eine Aufgabe *Incompressible viscous flow* im selben
Problem: Er liest das Geschwindigkeitsfeld aus deren Ergebnis und berechnet es
nie selbst.

### 1.2 Grundgleichung

**Wärmeübertragung in Fluiden** löst die Advektion-Diffusion der Temperatur auf
dem Strömungsfeld,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

mit derselben SUPG-Stabilisierung des konvektiven Terms wie der Strömungslöser
und demselben Theta-Zeitschrittverfahren. Die Geschwindigkeit `v` wird **aus dem
Strömungsergebnis übernommen** und hier nicht gelöst - deshalb erfordert der
Problemtyp eine Aufgabe *Incompressible viscous flow*.

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `T` | Temperatur | `K` | wird gelöst |
| `v` | Geschwindigkeit | `m/s` | das Strömungsergebnis |
| `k` | Wärmeleitfähigkeit | `W/(m*K)` | Materialeigenschaft |
| `c` | Wärmekapazität | `J/(kg*K)` | Materialeigenschaft |
| `rho` | Dichte | `kg/m^3` | Materialeigenschaft |
| `q` | Wärmequellendichte | `W/m^3` | Bedingung Heat, Joulesche Wärme, Strahlung |

Der Unterschied zum einfachen Problemtyp **Wärmeübertragung** ist genau der Term
`v . grad(T)`: Energie wird vom bewegten Fluid mitgeführt und nicht nur
geleitet. Das bedeutet auch, dass dieser Löser keine
Konvektionsrandbedingungen hat - es gibt nichts zu korrelieren, da die von
einer Wand in das Fluid abgegebene Wärme durch das Netz aufgelöst und nicht
modelliert wird.

### 1.3 Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Temperature | explizit (Dirichlet) | Punkt, Linie, Fläche, Volumen | Temperature `[K]` |
| Heat | natürlich, Quelle | Punkt, Linie, Fläche, Volumen | Heat `[W]` |

**Temperature** hält die Knoten ihrer Entität auf dem angegebenen Wert. Weisen
Sie sie der Einlassfläche zu, um die Temperatur des in das Gebiet eintretenden
Fluids festzulegen, und einer Wand, um einen beheizten oder gekühlten Rand zu
modellieren.

**Heat** ist eine Quelle und nimmt die **Gesamtleistung in `W`** an, die in die
Entität eingebracht wird; sie wird vor der Assemblierung über das Maß der
Entität verteilt - dieselbe Konvention wie beim Wärmelöser. *Heat rate (unit
area)* und *Heat rate (unit volume)* werden von der Oberfläche für diesen
Problemtyp angeboten, vom Löser für Wärme in Fluiden jedoch **nicht gelesen**;
verwenden Sie hier für eine Quelle *Heat*.

Jede Komponente ist eine **Tabelle über der Zeit**.

Die Strömungsbedingungen - Wände, Einlässe und Auslässe - gehören zur
Strömungsaufgabe und werden im Handbuch [Fluidströmung](theory_fluid.md)
beschrieben.

### 1.4 Konjugierte Wärmeübertragung

Eine Wand zwischen einem vernetzten Fluid und einem vernetzten Festkörper, die
die Bedingung **Forced convection** des einfachen Lösers *Heat transfer* trägt,
koppelt beide. Sobald der Wärmelöser gelaufen ist, hält der Löser für Wärme in
Fluiden die Wandknoten auf der Temperatur des Festkörpers - bis dahin ist die
Wand isoliert - und gibt für jedes Wandelement das Paar `(h, Tf)` zurück, das
den Wärmestrom reproduziert, den seine Lösung durch die Wand führt. Der
Wärmelöser wendet dieses Paar als Konvektionsbedingung an. Der Wärmestrom ist
das Residuum der Fluidgleichungen an den Wandknoten, konsistent mit der
Diskretisierung, und bleibt daher auch in einer dünnen thermischen Grenzschicht
genau. Die Wandtemperatur wird von Durchlauf zu Durchlauf mit einem
Aitken-Faktor relaxiert, wodurch sich der Austausch in wenigen Durchläufen
einpendelt. Das Handbuch [Wärmeübertragung](theory_heat.md) beschreibt das
Verfahren unter *Konjugierte Wärmeübertragung*. Strahlungswärme und Joulesche
Wärme, die von anderen Lösern kommen, werden auf dieselbe Weise wie beim
Wärmelöser zum Quellterm addiert.

### 1.5 Konvergenz

Ungekoppelt meldet der Löser für Wärme in Fluiden sich bedingungslos als
konvergiert - er ist linear, sobald das Strömungsfeld festliegt, daher ist eine
Lösung bereits das Endergebnis. Eine Gruppe, die sowohl die Strömungs- als auch
die Wärmeaufgabe für Fluide enthält, endet daher, wenn die Strömungsaufgabe
konvergiert. Mit gekoppelten Wänden meldet er stattdessen die relative Änderung
seines Temperaturfelds, sodass die Gruppe iteriert, bis sich die Wandtemperatur
eingependelt hat.

### 1.6 Rückkopplung auf die Strömung - Auftrieb

Die von diesem Löser berechnete Temperatur wird an die Strömung zurückgegeben.
Bei jedem Durchlauf wertet der Strömungslöser Dichte und dynamische Viskosität
jedes Elements bei der neuesten Temperatur aus, sodass bei einem Material mit
temperaturabhängiger Dichte die Volumenkraft `rho(T)*g` eine **natürliche
Konvektion** antreibt - warmes Fluid steigt, kaltes Fluid sinkt. Die
mitgelieferten Materialien *Water* und *Air* haben eine temperaturabhängige
Dichte.

In einem stationären Lauf sieht der erste Strömungsdurchlauf nur die
Anfangstemperatur, daher tritt der Auftrieb ab der zweiten Iteration der Gruppe
auf; geben Sie der Gruppe mehrere Iterationen und einen Konvergenzwert. In
einem transienten Lauf mit einer Iteration pro Zeitschritt sieht die Strömung
die Temperatur des vorherigen Schritts. Das Handbuch
[Fluidströmung](theory_fluid.md) beschreibt das Verfahren in Abschnitt 1.9.

### 1.7 Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Temperature | Anfangstemperaturfeld |

Anfangsbedingungen werden **nur beim ersten Lauf** gelesen; danach schreitet
das Feld vom vorherigen Ergebnis aus fort. Eine Randbedingung überschreibt eine
Anfangsbedingung dort, wo beide aufeinandertreffen.

### 1.8 Ergebnisse

| Ergebnis | Anwendbar auf | Bedeutung |
|---|---|---|
| Temperature `[K]` | Knoten | das gelöste Temperaturfeld |
| Heat flux `[W/m^2]` | Element | konduktiver Wärmestrom `-k*grad(T)` |

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Material`, `Ergebnisse`), im Menü
**Problem** und im Panel **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Wissenswerte Kombinationen:

- *Incompressible viscous flow* und *Heat transfer in fluids* in einer Gruppe -
  eine beheizte oder gekühlte Strömung. Geben Sie der Gruppe mehrere
  Iterationen, wenn die Strömung durch Auftrieb angetrieben oder merklich
  verändert wird, damit sich Strömung und Temperatur gemeinsam einpendeln -
  siehe 1.6;
- *Heat transfer in fluids* und eine separate Aufgabe *Heat transfer* auf einem
  vernetzten Festkörper, die Wärmeaufgabe für Fluide zuerst, in einer Gruppe
  mit mehreren Iterationen - konjugierte Wärmeübertragung durch die Wände mit
  *Forced convection*.

Der Problemtyp erfordert, dass eine Strömungsaufgabe vorhanden ist. Die Werte
der Aufgabengruppe - **Anzahl Iterationen** und **Konvergenz** - werden im
Handbuch [Fluidströmung](theory_fluid.md) beschrieben.

### 2.2 Registerkarte Problem

**Zeitlöser** - wird angezeigt, da der Problemtyp zeitabhängig ist. Lassen Sie
ihn für einen stationären Lauf deaktiviert. Seine Felder werden im Handbuch
[Fluidströmung](theory_fluid.md) beschrieben.

Der Problemtyp hat kein eigenes Einstellungs-Gruppenfeld. Alles andere wird pro
Entität auf den Registerkarten der Bedingungen zugewiesen.

### 2.3 Registerkarte Material

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Density | `kg/m^3` | `rho`, etwa `1000` für Wasser, `1.2` für Luft |
| Dynamic viscosity | `kg/(m*s)` | `mu`, etwa `1.0e-3` für Wasser, `1.8e-5` für Luft |
| Thermal conductivity | `W/(m*K)` | `k`, etwa `0.6` für Wasser, `0.025` für Luft |
| Heat capacity | `J/(kg*K)` | `c`, etwa `4180` für Wasser, `1005` für Luft |

Eine Entität, der eine erforderliche Eigenschaft fehlt, wird nicht gelöst, und
die Einstellungsprüfung warnt vor dem Start des Laufs. Die mitgelieferte
Materialdatenbank enthält **Water** und **Air**, womit dieser Schritt meist
bereits erledigt ist. Jede Eigenschaft ist eine Tabelle über der Temperatur und
wird bei der Elementtemperatur des vorherigen Laufs ausgewertet.

### 2.4 Registerkarte Randbedingungen

**Temperature** und **Heat** erscheinen, sobald eine Aufgabe *Heat transfer in
fluids* im Aufgabenablauf enthalten ist - siehe Abschnitt 1.3. Jede Komponente
ist eine Tabelle über der Zeit; klicken Sie auf **Zeitabhängige Werte
bearbeiten**, um Werte über der Zeit einzugeben.

### 2.5 Anfangsbedingungen

Die Registerkarte `Anfangsbedingungen` bietet *Temperature* an. Sie legt das
Ausgangsfeld des ersten Laufs fest.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Löser für Wärme in Fluiden verwendet den Eintrag **GMRES**, da seine
Matrix nicht symmetrisch ist.

### 2.7 Überwachungspunkte und Ergebnisse

`Problem` -> `Überwachungspunkte definieren` kann *Temperature* aufzeichnen. Die
Registerkarte `Ergebnisse` zeigt die Temperatur als Knotenskalar und den
Wärmestrom als Elementvektor.

---

## 3. Einschränkungen

- **Auftrieb ohne Ausdehnung des Fluids.** Die Temperatur verändert die
  Strömung über `rho(T)` und `mu(T)` (siehe 1.6), aber die Wärmeausdehnung des
  Fluids geht nie in die Massenbilanz `div(v) = 0` ein. Das ist für die kleinen
  Dichteunterschiede der natürlichen Konvektion genau, nicht jedoch für ein um
  Hunderte Kelvin erhitztes Gas. Die Auftriebsströmung ist laminar, wie jede
  Strömung in Range FEA.
- **Die Kopplung ist gestaffelt, nicht monolithisch.** Strömung und Temperatur
  werden nacheinander gelöst und tauschen zwischen den Durchläufen Daten aus.
  Eine stationäre Auftriebsströmung benötigt eine Gruppe mit mehreren
  Iterationen; bei einer Iteration pro Zeitschritt hinkt eine transiente
  Strömung der Temperatur um einen Schritt hinterher.
- **Bedingungen `Heat rate` werden ignoriert.** *Heat rate (unit area)* und
  *Heat rate (unit volume)* werden von der Oberfläche für *Heat transfer in
  fluids* angeboten, von `RSolverFluidHeat` jedoch nicht gelesen; nur die
  Bedingung *Heat* erreicht ihn. Der einfache Problemtyp *Heat transfer* liest
  alle drei.
- **Keine Konvektionsrandbedingungen.** Die von einer Wand in ein vernetztes
  Fluid abgegebene Wärme wird aufgelöst statt korreliert, daher gehören die
  Bedingungen *Simple*, *Forced* und *Natural convection* zum einfachen
  Wärmelöser. Konjugierte Wärmeübertragung wird als Aufgabe *Heat transfer* auf
  dem Festkörper eingerichtet, dessen Wände mit *Forced convection* den
  berechneten Zustand des Fluids lesen.
- **Kein Turbulenzmodell.** Die Strömung ist laminar, siehe Handbuch
  [Fluidströmung](theory_fluid.md), daher wird turbulente Wärmeübertragung
  nicht abgebildet.
