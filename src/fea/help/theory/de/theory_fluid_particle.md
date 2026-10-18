# Schadstoffausbreitung - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Löser für Schadstoffausbreitung von Range FEA -
`RSolverFluidParticle`, der den Problemtyp **Schadstoffausbreitung**
(*Contaminant dispersion*) bedient: die Gleichung, die er löst, die Bedeutung
jeder Eingabe, die er akzeptiert, die Teile der grafischen Benutzeroberfläche,
die ihn steuern, und ein ausgearbeitetes Tutorial.

Die Schadstoffausbreitung wird auf dem Strömungsfeld einer Aufgabe
**Inkompressible viskose Strömung** (*Incompressible viscous flow*) gelöst. Der
Strömungslöser, seine Randbedingungen, seine nichtlineare Iteration und sein
Zeitschrittverfahren werden im Handbuch [Fluidströmung](theory_fluid.md)
beschrieben, auf dem dieses Handbuch aufbaut.

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - transiente Strömung mit Schadstoffausbreitung](#3-tutorial---transiente-strömung-mit-schadstoffausbreitung)
4. [Überprüfung eines Modells](#4-überprüfung-eines-modells)
5. [Einschränkungen](#5-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - die Partikelkonzentration

Die primäre Unbekannte ist die **Partikelkonzentration** `C` in `kg/m^3`, ein
Freiheitsgrad pro Knoten.

| Problemtyp | Löser | Primäre Unbekannte | Einheiten | Freiheitsgrade pro Knoten |
|---|---|---|---|---|
| Contaminant dispersion | `RSolverFluidParticle` | Partikelkonzentration | `kg/m^3` | 1 |

Der Problemtyp **erfordert** eine Aufgabe *Incompressible viscous flow* im selben
Problem: Er liest das Geschwindigkeitsfeld aus deren Ergebnis und berechnet es
nie selbst.

### 1.2 Grundgleichung

**Schadstoffausbreitung** transportiert eine skalare Konzentration auf dem
Strömungsfeld,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

mit SUPG-Stabilisierung und dem Theta-Zeitschrittverfahren des
Strömungslösers, wobei die Geschwindigkeit aus dem Strömungsergebnis gelesen
wird. (`for s > 0 and a maximum saturation set` bedeutet „für s > 0 und
gesetzte maximale Sättigung“, `otherwise` „sonst“.)

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `C` | Partikelkonzentration | `kg/m^3` | wird gelöst |
| `v` | Geschwindigkeit | `m/s` | das Strömungsergebnis |
| `D` | Diffusionskoeffizient | `m^2/s` | *Einstellungen der Schadstoffausbreitung*, standardmäßig `0` |
| `C_sat` | maximale Sättigung | `kg/m^3` | *Einstellungen der Schadstoffausbreitung*, standardmäßig `0` (unbegrenzt) |
| `s` | Partikelquellrate | `kg/(m^3*s)` | Bedingung Particle rate |

### 1.3 Einstellungen der Schadstoffausbreitung

Die beiden Parameter des Problemtyps werden einmal pro Modell im Gruppenfeld
*Einstellungen der Schadstoffausbreitung* der Registerkarte `Problem`
festgelegt (Abschnitt 2.2). Beide sind standardmäßig null, was reiner Advektion
mit unbegrenzter Quelle entspricht.

#### Diffusionskoeffizient

Der **Diffusionskoeffizient** `D` verteilt den Schadstoff entlang seines
Konzentrationsgradienten. Mit `D = 0` ist der Transport rein advektiv: Der
Schadstoff wird von der Strömung mitgeführt und breitet sich nur durch die
numerische Diffusion der Stabilisierung aus, sodass der Löser die Frage „wohin
trägt ihn die Strömung und wie lange dauert es“ beantwortet, nicht „wie breit
ist die Fahne“. Ein positives `D` macht die Breite der Fahne zu einem
physikalischen Ergebnis.

Es gibt kein Turbulenzmodell (Abschnitt 5), daher ist `D` eine **effektive**
Diffusivität: Die Durchmischung einer realen Strömung wird von der Turbulenz
dominiert, die als `D ~ nu_t / Sc_t` mit der turbulenten Schmidt-Zahl
`Sc_t ~ 0.7` abgeschätzt werden kann.

| Molekulare Diffusion (die physikalische Eigenschaft) | `D [m^2/s]` |
|---|---|
| Gase und Dämpfe in Luft (Wasserdampf `2.5e-5`, CO2 `1.6e-5`, Lösemitteldämpfe etwa `1e-5`) | `1e-6` - `1e-4` |
| gelöste Stoffe in Wasser (O2 `2e-9`, Salze etwa `1.5e-9`) | `1e-10` - `1e-8` |
| Aerosolpartikel in Luft, Brownsche Bewegung (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

In technischen Maßstäben ist die molekulare Diffusion gegenüber dem Transport
durch die Strömung fast immer vernachlässigbar. Der normalerweise eingegebene
Wert ist ein effektiver:

| Effektive (turbulente) Diffusion | `D_eff [m^2/s]` |
|---|---|
| belüfteter Raum, Durchmischung der Raumluft | `1e-3` - `1e-2` |
| Rohr- oder Kanalströmung, etwa `D ~ 0.01 - 0.05 * U * L` (`U` mittlere Geschwindigkeit, `L` Kanalgröße) | `1e-3` - `1e-1` |
| Flüsse, Quervermischung | `1e-2` - `1` |
| Atmosphäre, horizontale Durchmischung | `1` - `100` |

Ob ein Wert eine Rolle spielt, hängt vom Netz ab. Mit mittlerer Geschwindigkeit
`U`, Strömungslänge `L`, Elementgröße `h` und Zeitschritt `dt`:

- **wann `D` überhaupt eine sichtbare Wirkung hat** - der Schadstoff breitet
  sich beim Durchqueren des Gebiets um etwa `sqrt( 2 * D * L / U )` aus. Ist das
  kleiner als `h`, bewirkt `D` nichts, daher spielt `D` erst oberhalb von etwa
  `U * h^2 / ( 2 * L )` eine Rolle. Für einen Kanal mit `U ~ 1 m/s`,
  `L ~ 15 m` und `h ~ 0.1 m` sind das etwa `3e-4 m^2/s`;
- **wann `D` dominiert** - oberhalb von etwa `U * L` (etwa `15 m^2/s` für
  denselben Kanal) verschmiert die Diffusion alles, und die Strömung spielt kaum
  noch eine Rolle;
- **Zeitschrittverfahren** - übersteigt `D * dt / h^2` etwa `1`, wechseln Sie zu
  *Backward difference (stable)*; das Verfahren mit zentraler Differenz dämpft
  die schnellsten diffusiven Moden nicht und oszilliert.

#### Maximale Sättigung

Die **maximale Sättigung** `C_sat` ist die größte Konzentration, die der
Schadstoff im Fluid erreichen kann - eine Sättigungsdampfkonzentration oder
eine Löslichkeit. Ist sie gesetzt, wird eine positive *Particle rate*
abgeschwächt, wenn sich die Konzentration ihr nähert, und hört bei Sättigung
auf, `s * ( 1 - C / C_sat )`; dies modelliert Verdunstung oder Auflösung, und
`s` ist dann die Rate in ein sauberes Fluid, etwa `k_m * C_sat * A / V`. Senken
(`s < 0`) werden nicht begrenzt. Die Grenze wirkt nur auf die Quelle: Eine durch
eine Rand- oder Anfangsbedingung vorgeschriebene Konzentration wird nicht
begrenzt, und das Löserprotokoll warnt, wenn sie `C_sat` übersteigt. Die
Konzentration wird auch nicht abgeschnitten, sodass ein leichtes Überschwingen
neben einer steilen Front weiterhin möglich ist.

Eine Sättigungsgrenze existiert nur dort, wo es eine thermodynamische gibt:
einen Dampfdruck oder eine Löslichkeit. Für einen Dampf in Luft ergibt sich die
Sättigungskonzentration aus dem Sättigungsdampfdruck,
`C_sat = p_sat * M / ( R * T )`:

| Dampf in Luft | Temperatur | `C_sat [kg/m^3]` |
|---|---|---|
| Wasserdampf | 0 degC | `0.0048` |
| Wasserdampf | 20 degC | `0.017` |
| Wasserdampf | 40 degC | `0.051` |
| Wasserdampf | 100 degC | `0.60` |
| Ethanol oder Toluol | 20 degC | etwa `0.11` |
| Methanol | 20 degC | etwa `0.17` |
| Benzol | 20 degC | etwa `0.32` |
| Aceton | 20 degC | etwa `0.59` |

Für einen in Wasser gelösten Stoff ist die Grenze seine Löslichkeit:

| Gelöst in Wasser | `C_sat [kg/m^3]` |
|---|---|
| Sauerstoff im Gleichgewicht mit Luft, 20 degC | `0.009` |
| Kohlendioxid bei 1 atm | `1.7` |
| Gips | `2.4` |
| Natriumchlorid | etwa `360` |

Partikel, Staub und Rauch haben keine Sättigung; lassen Sie den Wert auf `0`
(unbegrenzt). Nur zur Einordnung: Arbeitsplatzgrenzwerte liegen bei etwa
`1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) und untere Staubexplosionsgrenzen bei
etwa `0.02` - `0.06 kg/m^3`.
Die Sättigung hängt stark von der Temperatur ab, während der Löser einen
konstanten Wert verwendet: Nehmen Sie für einen nicht isothermen Fall den Wert
bei der kältesten relevanten Temperatur, bei der die Kondensation einsetzen
würde.

### 1.4 Stabilisierung

Der SUPG-Parameter des Ausbreitungslösers wird durch den Zeitschritt begrenzt
und schließt die Diffusion ein,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

wobei der Term mit `dt` in einer stationären Analyse entfällt. Er reduziert sich
auf `h / ( 2 * |v| )`, wo die Strömung schnell ist, übersteigt nie `dt / 2` und
nähert sich `h^2 / ( 12 * D )` - dem Grenzwert des knotenexakten
eindimensionalen Parameters -, wo die Diffusion dominiert. Die
Zeitschrittgrenze ist in langsamen und rezirkulierenden Bereichen wichtig: Dort
ist die Element-Courant-Zahl `|v| * dt / h` winzig, und ein unbegrenztes `tau`
würde die Stabilisierung in Rauschen verwandeln, das Konzentration vor der
Front erzeugt. Dieser Parameter gehört zum Ausbreitungslöser selbst; der
Strömungslöser verwendet den im Handbuch [Fluidströmung](theory_fluid.md)
beschriebenen.

Der Galerkin-Advektionsterm wird in **advektiver Form mit einer
Divergenzsenke** assembliert, `v . grad(C) + 0.5 * max(div(v), 0) * C`, mit der
stetigen Knotengeschwindigkeit. Eine Geschwindigkeit aus dem Strömungslöser ist
nie exakt divergenzfrei - auf einem groben Netz ist `div(v)` von Dutzenden
`1/s` neben Wänden und Ecken üblich. Wo sich die Strömung ausdehnt
(`div(v) > 0`), würde reine Advektion von selbst Konzentration erzeugen, ein
Schachbrettmuster, das von Schritt zu Schritt wächst, sobald der Zeitschritt
klein ist; die Senke hebt es auf. Wo die Strömung konvergiert, wird kein Term
hinzugefügt: Eine symmetrische Korrektur `0.5 * div(v) * C` würde dort als
Quelle wirken und den Schadstoff neben Wänden anhäufen. Für eine exakt
divergenzfreie Geschwindigkeit ist die Senke null.

Bei vernachlässigbarem Diffusionskoeffizienten ist der Transport fast reine
Advektion, die Galerkin mit SUPG nicht monoton hält: Rechnen Sie mit Über- und
Unterschwingern, am größten an Haftbedingungswänden, wo die Geschwindigkeit null
ist. Ein effektiver Diffusionskoeffizient (Abschnitt 1.3) beseitigt die meisten
davon.

Anders als der Strömungslöser wendet der Ausbreitungslöser die
Theta-Gewichtung konsistent an, daher ist *Central difference (accurate)* hier
ein echtes Crank-Nicolson-Verfahren.

### 1.5 Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponenten |
|---|---|---|---|
| Particle concentration | explizit (Dirichlet) | Punkt, Linie, Fläche | Particle concentration `[kg/m^3]` |
| Particle rate | natürlich, Quelle | Punkt, Linie, Fläche, Volumen | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | natürlich (Neumann) | Fläche | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | natürlich (Robin) | Fläche | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** hält ihre Knoten auf der angegebenen Konzentration.
Einer Einlassfläche mit einer Tabelle über der Zeit zugewiesen, modelliert sie
eine Freisetzung: null vor dem Ereignis, die Freisetzungskonzentration während
des Ereignisses, danach wieder null. Dies ist das nützlichste Muster dieses
Problemtyps, und das Tutorial in Abschnitt 3 baut darauf auf.

**Particle rate** ist eine Quelle und eine **Dichte** in `kg/(m^3*s)`, keine
Gesamtmenge - anders als die Bedingung *Heat* wird sie nicht über das Maß der
Entität verteilt. Sie modelliert eine Quelle, die kontinuierlich innerhalb des
Gebiets freisetzt.

**Particle flux (unit area)** schreibt den Massenstrom `q` vor, der durch eine
Fläche in das Gebiet eintritt, `D * grad(C) . n = q`, mit `n` als äußerer
Normale. Anders als *Particle rate* ist es ein Strom pro Flächeneinheit, daher
ist die Gesamtfreisetzung `q * A`. Positives `q` ist Emission - ein
verdunstender Film, eine ausgasende Wand - und wird durch die maximale
Sättigung auf dieselbe Weise begrenzt wie eine positive Partikelrate,
`q * ( 1 - C / C_sat )`. Negatives `q` ist eine feste Entnahmerate unabhängig
von der lokalen Konzentration und kann diese unter null treiben; eine zur
Konzentration proportionale Ablagerung ist *Particle mass transfer* mit
`C_inf = 0`. An einer Wand tritt der Strom nur durch Diffusion ein; auf einem
Teil des Randes, an dem die Strömung eintritt, wird der Zufluss als sauberes
Fluid behandelt, das den Strom mitführt, sodass `q` zum gesamten eintretenden
Massenstrom wird und `C ~ q / |v . n|` gilt. Einer inneren Fläche zwischen zwei
Volumen zugewiesen, setzt sie `q` pro Flächeneinheit als flächenhafte Quelle
frei.

**Particle mass transfer** tauscht Schadstoff mit einem Umgebungsreservoir aus,
`q = k_m * ( C_inf - C )`, wobei `k_m` der *Mass transfer coefficient* und
`C_inf` die *Ambient particle concentration* ist. Sie ist das Gegenstück zur
einfachen Konvektion bei der Wärmeübertragung. Mit `C_inf = 0` modelliert sie
die Ablagerung auf einer Fläche mit der Ablagerungsgeschwindigkeit `k_m`; mit
`C_inf > 0` einen Austausch mit der Umgebungsluft durch eine Öffnung oder eine
sich auflösende Fläche, die auf `C_inf` gehalten wird. Typische Werte von `k_m`
sind `1e-4` - `1e-2 m/s` für die Partikelablagerung in Räumen und
`1e-3` - `1e-1 m/s` für die Verdunstung in ein strömendes Gas. Ein großes `k_m`
treibt die Fläche in Richtung `C_inf` und nähert sich einer Bedingung *Particle
concentration*.

Jede Komponente ist eine **Tabelle über der Zeit**, sodass eine Freisetzung
während eines transienten Laufs beginnen und enden kann.

**Ränder ohne Bedingung.** Wände und Auslässe ohne Bedingung *Particle
concentration*, *Particle flux* oder *Particle mass transfer* behalten die
natürliche Bedingung **null diffusiver Fluss**: Der Schadstoff diffundiert
nicht durch eine Wand und verlässt einen Auslass nur mit der Strömung.

Wo die Strömung durch einen solchen Rand **eintritt** - typischerweise
Rückströmung an einem Teil eines Auslasses mit *Pressure (implicit)* -, wird
sie als sauberes Fluid behandelt: Der Löser fügt auf diesem Teil der Fläche den
schwachen Zuflussterm `max(-v . n, 0) * C` hinzu, über die Seite verteilt,
sodass er auch die Kantenknoten dämpft, an denen der Auslass auf eine Wand mit
Haftbedingung trifft.
Ohne ihn würde der Rand die Lösung speisen, und die Konzentration am Auslass
würde unbegrenzt wachsen. Ist das wieder eintretende Fluid nicht sauber,
verlängern Sie das Gebiet, sodass der Auslass nur Ausströmung sieht.

Die Strömungsbedingungen - Wände, Einlässe und Auslässe - gehören zur
Strömungsaufgabe und werden im Handbuch [Fluidströmung](theory_fluid.md)
beschrieben.

### 1.6 Anfangsbedingungen

| Anfangsbedingung | Wirkung |
|---|---|
| Particle concentration | Anfangskonzentrationsfeld |

Anfangsbedingungen werden **nur beim ersten Lauf** gelesen; danach schreitet
das Feld vom vorherigen Ergebnis aus fort. Eine Randbedingung überschreibt eine
Anfangsbedingung dort, wo beide aufeinandertreffen.

### 1.7 Stationär oder transient?

Die Schadstoffausbreitung ist vor allem ein **transientes** Problem: ein
Transportproblem in der Zeit, ohne sinnvolle stationäre Antwort für ein
Freisetzungsereignis. Eine stationäre Ausbreitung beantwortet eine andere
Frage - auf welches Konzentrationsfeld sich eine kontinuierliche Quelle
einstellt - und ist eine einzige lineare Lösung auf dem Strömungsfeld (siehe
Abschnitt 2.6 zu ihrem Matrixlöser).

Der zweistufige Arbeitsablauf des Tutorials in Abschnitt 3 ist der
kanonische: die Strömung stationär konvergieren lassen und dann mit aktiviertem
Zeitlöser und aktivierter Ausbreitungsaufgabe neu starten.

### 1.8 Ergebnisse

| Ergebnis | Anwendbar auf | Bedeutung |
|---|---|---|
| Particle concentration `[kg/m^3]` | Knoten | das gelöste Konzentrationsfeld |
| Relative saturation `[-]` | Knoten | `C / C_sat`, nur wenn eine maximale Sättigung gesetzt ist |

Die relative Sättigung wird standardmäßig von `0` bis `1` angezeigt und
entfernt, wenn die maximale Sättigung wieder auf null gesetzt wird.

Für diesen Problemtyp ist nur die Materialeigenschaft **Dichte** erforderlich.

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Randbedingungen`, `Anfangsbedingungen`, `Material`, `Ergebnisse`), im Menü
**Problem** und im Panel **Modell**.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Legen Sie eine Aufgabe **Contaminant dispersion**
in dieselbe Aufgabengruppe wie eine Aufgabe **Incompressible viscous flow** -
Ausbreitung auf der Strömung, die klassische transiente Kombination. Der
Problemtyp erfordert, dass die Strömungsaufgabe vorhanden ist.

Die Werte der Aufgabengruppe - **Anzahl Iterationen** und **Konvergenz** -
steuern die nichtlineare Iteration der Strömung und werden im Handbuch
[Fluidströmung](theory_fluid.md) beschrieben. In einem transienten Lauf sind es
die Iterationen *pro Zeitschritt*.

### 2.2 Registerkarte Problem

**Zeitlöser** - lassen Sie ihn für eine stationäre Ausbreitung deaktiviert und
aktivieren Sie ihn für ein Freisetzungsereignis.

| Feld | Bedeutung |
|---|---|
| Aktivieren | schaltet das Zeitschrittverfahren ein und aus |
| Approximation | Theta des Zeitschrittverfahrens: rückwärts, zentral oder vorwärts |
| Startzeit | erster Zeitwert in Sekunden |
| Endzeit | `start time + time-step size * number of time-steps`, schreibgeschützt |
| Zeitschrittweite | `dt` in Sekunden |
| Anzahl der Zeitschritte | wie viele Schritte berechnet werden |
| Ausgabefrequenz | schreibt alle N Schritte eine Ergebnisdatei; `0` schreibt nur den letzten Schritt |

**Einstellungen der Schadstoffausbreitung** - wird angezeigt, sobald eine
Aufgabe *Contaminant dispersion* im Aufgabenablauf enthalten ist. Es enthält
die beiden Parameter aus Abschnitt 1.3:

| Feld | Bedeutung |
|---|---|
| Maximale Sättigung `[kg/m^3]` | größtmögliche Konzentration; eine positive Partikelrate endet dort, und *Relative saturation* wird berechnet. `0` bedeutet unbegrenzt |
| Diffusionskoeffizient `[m^2/s]` | effektive Diffusivität des Schadstoffs im Fluid. `0` bedeutet reine Advektion |

Typische Werte für beide sind in Abschnitt 1.3 aufgeführt.

### 2.3 Registerkarte Material

| Eigenschaft | Einheiten | Kommentar |
|---|---|---|
| Density | `kg/m^3` | `rho`, etwa `1000` für Wasser, `1.2` für Luft |

Die Strömungsaufgabe im selben Problem benötigt zusätzlich die dynamische
Viskosität. Die mitgelieferte Materialdatenbank enthält **Water** und **Air**,
womit dieser Schritt meist bereits erledigt ist.

### 2.4 Registerkarte Randbedingungen

Die Bedingungen aus Abschnitt 1.5 werden angeboten, sobald eine Aufgabe
*Contaminant dispersion* im Aufgabenablauf enthalten ist. Wählen Sie zuerst
eine Entität im Baum `Modell` aus; die Verwaltung bietet dann die für diesen
Entitätstyp gültigen Bedingungen an. Aktivieren Sie eine, um sie zuzuweisen,
und bearbeiten Sie dann ihre Komponenten im unteren Baum.

Jede Komponente ist eine Tabelle über der Zeit. Klicken Sie auf
**Zeitabhängige Werte bearbeiten**, um den Komponenteneditor zu öffnen, in dem
Werte über der Zeit eingegeben werden; zwischen zwei angegebenen Zeitpunkten
wird der Wert linear interpoliert, und nach dem letzten Zeitpunkt bleibt der
letzte Wert erhalten. So wird ein Freisetzungsereignis modelliert.

### 2.5 Anfangsbedingungen

Die Registerkarte `Anfangsbedingungen` bietet *Particle concentration* an. Sie
legt das Ausgangsfeld des ersten Laufs fest.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Der Ausbreitungslöser verwendet den Eintrag **GMRES**, da seine Matrix
nicht symmetrisch ist.

Eine **stationäre Schadstoffausbreitung mit Diffusion** erfordert hier
Aufmerksamkeit. Sie ist eine einzige lineare Lösung, daher gibt es keine
Iterationen der Aufgabengruppe als Rückfallebene, und ein diffusionsdominiertes
System konvergiert unter der Standardgrenze von GMRES mit 10 inneren mal 10
äußeren Iterationen nur langsam. Der Matrixlöser warnt nicht, wenn er seine
Grenze erreicht: Lesen Sie seine Iterationstabelle im Löserprotokoll, und wenn
er bis zur letzten äußeren Iteration läuft, während das Residuum noch deutlich
über dem Konvergenzwert des Lösers liegt, erhöhen Sie die Anzahl der äußeren
Iterationen. Eine nicht konvergierte Lösung liefert ohne weiteres Anzeichen ein
falsches Ergebnis.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable - *Particle
concentration*, *Particle rate* oder *Relative saturation*. In einem
transienten Lauf wird der Verlauf über `Bericht` -> `Überwachungspunkte`
angezeigt.

Ein Überwachungspunkt am interessierenden Ort liefert die Antwort in der Form,
in der man sie meist haben möchte: die Konzentration über der Zeit an einem
bestimmten Ort, abgelesen aus einem Diagramm statt aus einer Folge von Bildern.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet die berechneten Variablen auf und steuert
die 3D-Ansicht. Partikelkonzentration und relative Sättigung sind
Knotenskalare.

Die Registerkarte `Datensätze` des Panels `Modell` listet die
Ergebnisdatensätze auf - einen pro geschriebenem Zeitschritt eines transienten
Laufs. Das Animieren der Datensätze spielt die Ausbreitung ab, und die
Schaltfläche **Datensatz** unter dem Baum schreibt die Animation in eine
Videodatei.

---

## 3. Tutorial - transiente Strömung mit Schadstoffausbreitung

**Ziel.** Einen Schadstoff in eine konvergierte Strömung freisetzen und die
Fahne in der Zeit durch einen Kanal verfolgen.

Dies ist der zweistufige Arbeitsablauf aus Abschnitt 1.7, und er entspricht
dem mitgelieferten Tutorial *Schadstoffausbreitung in Fluiden*.

### Schritt 1 - von einer konvergierten Strömung ausgehen

Führen Sie zuerst das stationäre Kanal-Tutorial aus dem Handbuch
[Fluidströmung](theory_fluid.md) bis zur Konvergenz durch. Ein aus der Ruhe
gestarteter Ausbreitungslauf verschwendet seine ersten Zeitschritte mit der
Entwicklung des Strömungsfelds, statt etwas zu transportieren.

### Schritt 2 - Ausbreitungsaufgabe hinzufügen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Behalten Sie die Aufgabengruppe und die Aufgabe **Incompressible viscous
   flow** bei.
3. Fügen Sie innerhalb derselben Gruppe eine Aufgabe **Contaminant dispersion**
   hinzu.
4. Verringern Sie die **Anzahl Iterationen** der Gruppe auf etwa `20` - dies
   sind nun die nichtlinearen Iterationen *pro Zeitschritt*, nicht die
   Iterationen einer stationären Lösung. Lassen Sie den Wert **Konvergenz**
   unverändert; die meisten Schritte pendeln sich vor dem zwanzigsten Durchlauf
   ein und enden früher, und diejenigen, die das nicht tun, sind genau die, die
   die vollen zwanzig verdienen.

*Contaminant dispersion* erfordert, dass die Strömungsaufgabe vorhanden ist; es
liest das Geschwindigkeitsfeld und berechnet es nie.

### Schritt 3 - Zeitlöser konfigurieren

Öffnen Sie die Registerkarte `Problem` und stellen Sie im Gruppenfeld
*Zeitlöser* Folgendes ein:

| Feld | Wert |
|---|---|
| Aktivieren | aktiviert |
| Approximation | `Backward difference (stable)` |
| Startzeit | `0` |
| Zeitschrittweite | gemäß der Courant-Empfehlung unten |
| Anzahl der Zeitschritte | genug, um die Fahne durch das Gebiet zu tragen |
| Ausgabefrequenz | `1` für eine flüssige Animation, mehr für eine kleinere Datei |

Wählen Sie den Zeitschritt anhand des Netzes und der Strömungsgeschwindigkeit
so, dass das Fluid pro Schritt etwa ein Element durchquert:

```
dt ~ h / |v|
```

Wählen Sie dann die Anzahl der Schritte danach, wie lange die Fahne zum
Durchqueren des Gebiets braucht, `L / |v|`, geteilt durch diesen Schritt.

Lassen Sie im Gruppenfeld *Einstellungen der Schadstoffausbreitung* derselben
Registerkarte beide Werte für reine Advektion auf `0`, oder geben Sie einen
effektiven **Diffusionskoeffizienten** aus der Tabelle in Abschnitt 1.3 ein,
wenn die Breite der Fahne von Bedeutung ist. Eine **Maximale Sättigung** ist
nur für einen Dampf oder einen gelösten Stoff sinnvoll; sie darf nicht unter
der Freisetzungskonzentration aus Schritt 4 liegen.

### Schritt 4 - Schadstoff freisetzen

1. Wählen Sie die Einlass-Flächenentität aus.
2. Öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie **Particle
   concentration**.
3. Statt einen einzelnen Wert einzugeben, klicken Sie auf **Zeitabhängige Werte
   bearbeiten**.
4. Geben Sie im Komponenteneditor ein Freisetzungsprofil ein - zum Beispiel `0`
   zum Zeitpunkt `0`, die Freisetzungskonzentration zum Zeitpunkt, an dem die
   Freisetzung beginnt, und wieder `0`, wenn sie endet.

Zwischen zwei angegebenen Zeitpunkten wird der Wert linear interpoliert, und
nach dem letzten Zeitpunkt bleibt der letzte Wert erhalten; eine Freisetzung,
die abrupt beginnen soll, benötigt daher zwei dicht beieinander liegende
Einträge. Diese zeitabhängige Bedingung macht aus einem stationären Einlass ein
Freisetzungsereignis, und sie ist das Herzstück eines Ausbreitungsmodells.

### Schritt 5 - Strömungsbedingungen beibehalten

Die Bedingung **Wall**, die Zuflussbedingung und der Auslass mit **Pressure
(implicit)** werden unverändert übernommen. Die Strömung wird weiterhin in
jedem Zeitschritt gelöst - die Ausbreitung fährt auf ihr mit.

### Schritt 6 - Überwachungspunkt hinzufügen

`Problem` -> `Überwachungspunkte definieren`, platzieren Sie einen Punkt
stromabwärts und setzen Sie seine Variable auf **Particle concentration**. Die
Ankunftszeit und die Spitzenkonzentration an einem bestimmten Ort lassen sich
aus einem Verlauf weit leichter ablesen als aus einer Folge farbiger Bilder.

### Schritt 7 - mit Neustart lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Aktivieren Sie **Löser neu starten / fortsetzen**.

Der Neustart macht das konvergierte stationäre Feld zum Ausgangspunkt des
Zeitschrittverfahrens. Ohne ihn startet der Lauf aus der Ruhe, und die ersten
Schritte sind verschwendet.

### Schritt 8 - Ergebnisse untersuchen

- Panel `Modell`, Registerkarte `Datensätze`: ein Datensatz pro geschriebenem
  Schritt. Gehen Sie sie schrittweise durch oder animieren Sie sie, um die
  Fahne wandern zu sehen.
- Registerkarte `Ergebnisse`: Zeigen Sie **Particle concentration** an. Fixieren
  Sie den Anzeigebereich über alle Datensätze, damit die Farben in jedem Bild
  dasselbe bedeuten - sonst skaliert sich die Animation selbst neu, und die
  Fahne scheint nicht abzuklingen.
- `Bericht` -> `Überwachungspunkte` zeichnet den Konzentrationsverlauf am
  Messpunkt. Lesen Sie daraus die Ankunftszeit ab und vergleichen Sie sie mit
  `L / |v|`, der Laufzeit, die sich aus der mittleren Strömung ergibt - das ist
  die günstigste Validierung des gesamten Modells.
- Mit einem Diffusionskoeffizienten von null breitet sich die Fahne nur durch
  numerische Diffusion aus, daher ist ihre **Breite** eine Eigenschaft des
  Netzes und des Zeitschritts, nicht der Physik. Mit einem effektiven
  Diffusionskoeffizienten ist die Breite ein Ergebnis - prüfen Sie sie an einem
  feineren Netz, bevor Sie ihr vertrauen.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Die Konzentration verlässt den Einlass nie | das Strömungsfeld ist null - die Strömungsaufgabe fehlt in der Gruppe, oder der Lauf wurde nicht von einer konvergierten Strömung neu gestartet |
| Die Fahne verschmiert fast sofort | die Courant-Zahl liegt weit über 1, oder das Netz ist entlang des Weges zu grob |
| Die Konzentration wird negativ oder schwingt über | der Zeitschritt ist für das Netz zu groß, sodass die stabilisierte Advektion oszilliert. Leichte Unterschwinger neben einer steilen Front sind normal; die Lösung wird nicht bei null abgeschnitten, weil Abschneiden Schadstoff hinzufügen würde |
| Es wurde nur ein Datensatz geschrieben | die Ausgabefrequenz ist `0`, wodurch nur der letzte Schritt geschrieben wird |
| Das Residuum jedes Schritts bleibt hoch | zu wenige Iterationen pro Zeitschritt - die Strömung innerhalb jedes Schritts ist nicht konvergiert |
| Die Fahne breitet sich nie seitlich aus | der Diffusionskoeffizient ist null, daher kommt die seitliche Ausbreitung nur aus dem Strömungsfeld - geben Sie einen effektiven Diffusionskoeffizienten ein, siehe Abschnitt 1.3 |
| Die Fahne breitet sich sofort über das ganze Gebiet aus | der Diffusionskoeffizient ist viel zu groß - vergleichen Sie ihn mit `U * L` |
| Das Protokoll warnt, dass die vorgeschriebene Konzentration die maximale Sättigung übersteigt | eine Rand- oder Anfangsbedingung liegt über der maximalen Sättigung; die Grenze wirkt nur auf die Partikelrate, prüfen Sie daher beide Werte |
| Eine stationäre Lösung mit Diffusion sieht falsch aus | der Matrixlöser hat an seiner Iterationsgrenze angehalten - erhöhen Sie die äußeren Iterationen von GMRES, siehe Abschnitt 2.6 |
| Die Konzentration schwingt über den Einlasswert oder wird neben Wänden negativ, wächst aber nicht weiter | nahezu reine Advektion - der Diffusionskoeffizient ist nahe null. Geben Sie einen effektiven Diffusionskoeffizienten ein; prüfen Sie bei einem groben Netz auch, ob das Strömungsfeld konvergiert ist |
| Die Konzentration übersteigt den Einlasswert und wächst zu einem verrauschten Muster | die Strömung ist nicht konvergiert - `Residual ratio` bleibt in jedem Schritt nahe `1`. Ihre Geschwindigkeit ist weit von Divergenzfreiheit entfernt, und keinem Transport darauf kann vertraut werden; lassen Sie zuerst die Strömung konvergieren (Netz, Reynolds-Zahl, Iterationen pro Schritt) |

---

## 4. Überprüfung eines Modells

**Laufzeit.** Die Zeit, die ein Marker benötigt, um eine Strecke `L` mit der
mittleren Geschwindigkeit `V` zurückzulegen, ist `L/V`. Lesen Sie die
Ankunftszeit aus dem Verlauf eines Überwachungspunkts ab und vergleichen Sie.
Dies bestätigt, dass die Ausbreitungsaufgabe auf dem Strömungsfeld mitfährt,
von dem Sie ausgehen, und deckt einen Lauf auf, der nicht von einer
konvergierten Strömung neu gestartet wurde.

Das Strömungsfeld selbst prüft man am besten zuerst, mit der Massenbilanz und
den analytischen Strömungen, die im Handbuch [Fluidströmung](theory_fluid.md)
aufgeführt sind. Ein nicht konvergiertes Strömungsmodell besteht aus demselben
Grund keine Prüfung, und keinem Transport darauf kann vertraut werden.

---

## 5. Einschränkungen

- **Kein Turbulenzmodell.** Die Strömung ist laminar, siehe Handbuch
  [Fluidströmung](theory_fluid.md), daher muss die Durchmischung einer
  turbulenten Strömung als effektiver Diffusionskoeffizient eingegeben werden.
- **Konstante, isotrope Diffusion.** Der Diffusionskoeffizient ist ein einziger
  Wert für das gesamte Modell; er kann weder räumlich noch zeitlich variieren,
  der Strömungsrichtung folgen oder aus einem Turbulenzmodell stammen. Er muss
  als effektive Diffusivität abgeschätzt werden - siehe Abschnitt 1.3.
- **Konstante maximale Sättigung.** Die Sättigung folgt nicht der Temperatur
  eines Ergebnisses *Heat transfer in fluids* und begrenzt nur eine positive
  Partikelrate oder einen positiven Partikelstrom. Durch Bedingungen
  vorgeschriebene Konzentrationen werden nicht begrenzt, und das Feld wird
  nicht abgeschnitten.
- **Die Kopplung an die Strömung ist einseitig.** Die Strömung treibt den
  Transport an; der Transport verändert die Strömung nie.
- **Nicht monoton bei nahezu reiner Advektion.** Galerkin mit SUPG erzeugt Über-
  und Unterschwinger, wenn der Diffusionskoeffizient nahe null ist, am größten
  an Wänden mit Haftbedingung - siehe Abschnitt 1.4.
