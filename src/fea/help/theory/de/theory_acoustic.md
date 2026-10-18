# Akustische Analyse - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Akustiklöser (`RSolverAcoustic`) von Range FEA:
die Gleichungen, die er löst, die Bedeutung jeder Eingabe, die er akzeptiert,
die Teile der grafischen Benutzeroberfläche, die ihn steuern, und zwei
ausgearbeitete Tutorials.

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - transiente akustische Analyse](#3-tutorial---transiente-akustische-analyse)
4. [Tutorial - harmonische akustische Analyse](#4-tutorial---harmonische-akustische-analyse)
5. [Verifikation](#5-verifikation)
6. [Einschränkungen](#6-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Primäre Unbekannte - das Geschwindigkeitspotential

Der Akustiklöser löst nicht direkt nach dem Druck. Seine primäre Unbekannte ist
das **Geschwindigkeitspotential** `phi`, ein Skalarfeld mit der Einheit
`m^2/s`. Beide interessierenden physikalischen Größen werden daraus gewonnen:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`particle velocity` = Schallschnelle, `acoustic pressure` = Schalldruck.) Die
Verwendung eines Potentials anstelle des Drucks hat zwei praktische Vorteile.
Das Problem bleibt skalar, obwohl die Schallschnelle ein Vektor ist, und eine
vorgeschriebene Wandgeschwindigkeit - die Art, wie die meisten Schallquellen
beschrieben werden - geht als natürliche Randbedingung statt als Zwangsbedingung
ein.

Die obige Vorzeichenkonvention (`u = -grad(phi)`, `p = +rho * dphi/dt`) wird im
gesamten Löser und in den gespeicherten Ergebnissen einheitlich verwendet.

### 1.2 Grundgleichung

Schall kleiner Amplitude in einem ruhenden, homogenen Fluid gehorcht der
gedämpften Wellengleichung für das Geschwindigkeitspotential:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `phi` | Geschwindigkeitspotential | `m^2/s` | wird gelöst |
| `c` | Schallgeschwindigkeit | `m/s` | Materialeigenschaft |
| `rho` | Dichte des Fluids | `kg/m^3` | Materialeigenschaft |
| `beta` | Volumendämpfungsfaktor | `1/s` | Materialeigenschaft, Standard `0` |

Die Gleichung ist linear. Folglich ist eine einzige Lösung pro Zeitschritt oder
pro Frequenz das Endergebnis - es wird keine äußere nichtlineare Iteration
durchgeführt, und `hasConverged()` meldet immer Erfolg, sobald das lineare
System gelöst wurde.

**Schallgeschwindigkeit.** `c` kann direkt über die Materialeigenschaft *Speed
of sound* angegeben werden. Fehlt sie oder ist sie null, wird sie aus der
Kompressionssteifigkeit und der Dichte abgeleitet:

```
c = sqrt(E / rho)
```

wobei `E` die Materialeigenschaft *Modulus of elasticity* ist. Für ein Gas ist
`E` der adiabatische Kompressionsmodul (`gamma * p0`, etwa `1.42e5 Pa` für Luft
bei Atmosphärendruck), was zusammen mit `rho = 1.2 kg/m^3` die bekannte
`c = 344 m/s` ergibt. Die direkte Angabe von `c` ist meist weniger
fehleranfällig.

Elemente ohne eigenes Material - beispielsweise eine Randfläche, die ein
vernetztes Volumen begrenzt - übernehmen die mittleren Werte `c` und `rho` des
berechenbaren Gebiets, damit Impedanz- und Abstrahlungsterme auf ihnen korrekt
ausgewertet werden.

### 1.3 Finite-Elemente-Diskretisierung

Multiplikation mit einer Gewichtsfunktion `w`, Integration über das Gebiet,
Anwendung des Divergenzsatzes und Multiplikation mit `c^2` ergibt die schwache
Form

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

die nach Einsetzen der Element-Formfunktionen `N` zum System zweiter Ordnung
wird

```
M * phi'' + C * phi' + K * phi = f
```

mit

| Matrix | Definition | Kommentar |
|---|---|---|
| `M` | `INT( N_m * N_n )` | Massenmatrix |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | Steifigkeitsmatrix |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | Volumendämpfung plus Randimpedanz |
| `f` | `c^2 * SURF( v_n * N_m )` | Quelle der vorgeschriebenen Normalgeschwindigkeit |

`M` und `K` sind positiv semidefinit; `C` ist positiv semidefinit, solange das
Impedanzverhältnis `zeta` positiv ist.

**Elementmaße.** Gebietsterme (`M`, `K`, der Volumenanteil von `C`) werden mit
dem geometrischen Maß der Entität skaliert, zu der sie gehören:

| Entität | Maß | Wo es gesetzt wird |
|---|---|---|
| Volumen | 1 | - |
| Fläche | Flächendicke | geometrische Eigenschaften der Entität |
| Linie | Querschnittsfläche der Linie | geometrische Eigenschaften der Entität |
| Punkt | Punktvolumen | geometrische Eigenschaften der Entität |

Randterme (der Impedanzanteil von `C` und die Quelle `f`) wirken über das
einfache Elementmaß - die Fläche eines Flächenelements, die Länge eines
Linienelements, die Einheitsfläche bei einem Punktelement. Eine Fläche, die ein
vernetztes Volumen begrenzt, trägt daher nur ihre Randterme bei, da ihre Dicke
standardmäßig null ist.

Punktelemente haben keine Integrationspunkte. Ihre Beiträge werden direkt auf
ihren einzigen Knoten konzentriert, wodurch Punkt-Randbedingungen an den Enden
eindimensionaler Kanäle funktionieren.

### 1.4 Randbedingungen

| Randbedingung | Typ | Anwendbar auf | Komponente |
|---|---|---|---|
| Velocity potential | explizit (Dirichlet) | Punkt, Linie, Fläche | Potential `[m^2/s]` |
| Velocity | natürlich (Quelle) | Punkt, Linie, Fläche | Velocity `[m/s]` |
| Absorbing boundary | natürlich (Dämpfung) | Punkt, Linie, Fläche | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | natürlich (Dämpfung) | Punkt, Linie, Fläche | Acoustic impedance `[Pa*s/m]` |

**Velocity.** Die klassische akustische Quelle: eine Fläche, die mit einer
vorgeschriebenen Normalgeschwindigkeit schwingt, etwa eine
Lautsprechermembran oder ein Kolben. Ein positiver Wert treibt das Fluid an,
d. h. er zeigt *in* das akustische Gebiet. Er geht in die rechte Seite als
`f = c^2 * SURF(v_n * N)` ein.

Ein Knoten ganz ohne Randbedingung ist eine ideale **schallharte Wand** - die
natürliche Bedingung der schwachen Form ist eine Normalgeschwindigkeit von null.
Für einen schallhart begrenzten Hohlraum muss nichts zugewiesen werden.

**Acoustic impedance.** Schreibt das Verhältnis von Druck zur nach außen
gerichteten Normal-Schallschnelle am Rand vor, `p = Z * u_n`. Intern wird es
durch das dimensionslose **spezifische Impedanzverhältnis** ausgedrückt

```
zeta = Z / (rho * c)
```

und trägt die Randdämpfungsmatrix `(c/zeta) * SURF(N_m * N_n)` bei.
`Z = rho*c` (`zeta = 1`) ist die Kennimpedanz des Fluids und ergibt einen
vollkommen reflexionsfreien Abschluss. Für Luft bei Raumbedingungen liegt
`rho*c` nahe `415 Pa*s/m`, dem Wert, den die Randbedingung standardmäßig
anbietet.

**Absorbing boundary.** Derselbe Term, jedoch parametriert durch den
**Absorptionsgrad** `alpha` bei senkrechtem Einfall in `[0, 1]`, den
Materialdatenblätter angeben. Er wird über den Reflexionsfaktor der ebenen Welle
`R = (zeta - 1) / (zeta + 1)` und `alpha = 1 - |R|^2` in ein
Impedanzverhältnis umgerechnet:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Verhalten |
|---|---|---|
| `1.0` | `1` | voll absorbierend, reflexionsfreier Abschluss, keine Reflexion |
| `0.5` | `5.83` | teilweise absorbierend |
| `0.1` | `38.0` | schwach absorbierend |
| `0.0` | unendlich | schallharte Wand, überhaupt keine Dämpfung |

Verwenden Sie `alpha = 1` an jedem künstlichen Schnitt durch ein unbegrenztes
Gebiet, um freie Abstrahlung nachzubilden, und einen gemessenen Wert `alpha` an
einer verkleideten Wand. Werden derselben Entität sowohl eine Bedingung
*Acoustic impedance* als auch *Absorbing boundary* zugewiesen, hat die Impedanz
Vorrang.

### 1.5 Anfangsbedingungen

| Anfangsbedingung | Setzt | Kommentar |
|---|---|---|
| Velocity potential | `phi` bei `t = 0` | allein selten ungleich null |
| Velocity potential / s | `dphi/dt` bei `t = 0` | damit wird der Anfangs**druck** gesetzt, `p = rho * dphi/dt` |

Beide werden nur im ersten Zeitschritt angewendet. Um einen transienten Lauf aus
einem unter Druck stehenden Bereich zu starten, weisen Sie dieser Entität
*Velocity potential / s* `= p0 / rho` zu. Anfangsbedingungen haben auf eine
harmonische Analyse keine Wirkung.

### 1.6 Transiente Analyse

Das System zweiter Ordnung wird mit dem **Newmark**-Verfahren mit
`gamma = 1/2` und

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

integriert (`time-march approximation coefficient` ist der Koeffizient der
Approximation des Zeitschrittverfahrens). Die Begrenzung garantiert
`beta_N >= 1/4`, also mindestens die unbedingt stabile Variante der mittleren
Beschleunigung, und verhindert eine Division durch null bei der
Vorwärts-Approximation, deren Koeffizient `0` ist.

Jeder Schritt löst

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

mit den üblichen Koeffizienten

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

gefolgt von der Zustandsaktualisierung

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

Die Systemmatrix ist symmetrisch positiv definit und wird daher mit dem
Verfahren der **konjugierten Gradienten** und Jacobi-Vorkonditionierung gelöst.

`phi'` und `phi''` werden als *Potential / s* und *Potential / s^2* in die
Ergebnisdatei geschrieben. Sie sind der Zustand des Zeitintegrators und müssen
daher für einen Neustart oder für die korrekte Fortsetzung mit dem nächsten
Zeitschritt vorhanden sein.

**Eine transiente Analyse erfordert einen aktivierten Zeitlöser.** Ohne ihn
verschwinden die Massen- und Dämpfungsterme, und das Problem entartet zu einem
bedeutungslosen Laplace-Problem; der Löser meldet dies als Fehler, statt
Unsinn zu erzeugen.

**Wahl des Zeitschritts.** Das Verfahren ist unbedingt stabil, daher wird `dt`
durch die Genauigkeit bestimmt, nicht durch die Stabilität. Eine nützliche
Regel ist, die höchste interessierende Frequenz `f_max` mit mindestens 20
Schritten pro Periode aufzulösen:

```
dt <= 1 / (20 * f_max)
```

**Wahl des Netzes.** Lösen Sie die kürzeste Wellenlänge
`lambda_min = c / f_max` mit mindestens 10 linearen Elementen auf:

```
h <= c / (10 * f_max)
```

### 1.7 Harmonische Analyse

Für eine stationäre Antwort bei einer einzelnen Kreisfrequenz
`omega = 2*pi*f` schreibt man `phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Dann
gilt `dphi/dt -> i*omega*PHI` und `d2phi/dt2 -> -omega^2*PHI`, und das System
wird zum komplexen **Helmholtz**-Problem

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

Die Matrixlöser von Range FEA sind reell, daher wird das komplexe System als
äquivalentes reelles Blocksystem doppelter Größe gelöst. Mit `S = K - omega^2*M`
und `D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Diese Blockmatrix ist nicht symmetrisch und oberhalb des statischen Grenzfalls
indefinit, daher wird sie mit **GMRES** und Jacobi-Vorkonditionierung gelöst.
GMRES mit Neustart stagniert bei indefiniten Systemen, wann immer der
Krylov-Raum viel kleiner als das System ist; daher erweitert der Löser den
GMRES-Neustart automatisch auf die Systemgröße, begrenzt durch ein festes Budget
von 256 MB für die beiden Krylov-Basen. Der eingestellte Neustart wird nur
erhöht, nie verringert, und die Änderung wird im Löserprotokoll gemeldet:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

Eine harmonische Analyse durchläuft eine Liste von Frequenzen

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(`first frequency` = erste Frequenz, `frequency step` = Frequenzschritt,
`number of frequencies` = Anzahl der Frequenzen.) Jede Frequenz ist eine
unabhängige Lösung und erzeugt einen eigenen Ergebnisdatensatz, so wie eine
transiente Analyse einen Datensatz pro Zeitschritt erzeugt. Der Zeitlöser spielt
keine Rolle; ist er zufällig aktiviert, wird er mit einer Warnung übersprungen,
und seine Einstellungen bleiben unverändert, sodass sie beim Zurückwechseln zu
einer transienten Analyse nicht verloren gehen.

### 1.8 Abgeleitete Ergebnisse

| Ergebnis | Anwendbar auf | Transient | Harmonisch |
|---|---|---|---|
| Potential `[m^2/s]` | Knoten | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | Knoten | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | Knoten | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | Knoten | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | Knoten | `rho * dphi/dt`, momentan | `omega * rho * abs(PHI)`, Amplitude |
| Acoustic phase `[Deg]` | Knoten | - | `arg(P)` |
| Sound pressure level `[dB]` | Knoten | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | Element | `-grad(phi)` | Amplitude von `-grad(PHI)`, pro Komponente |
| Acoustic intensity `[W/m^2]` | Element | `p * u`, momentan | `0.5 * Re{P * conj(U)}`, zeitlich gemittelt |

Der **Schalldruckpegel** wird gegenüber dem *Bezugsdruck* der akustischen
Einstellungen berechnet, standardmäßig `2e-5 Pa` - die Hörschwelle in Luft. In
einer harmonischen Analyse wird der Effektivwert des harmonischen Signals
verwendet, also die Amplitude geteilt durch `sqrt(2)`; in einer transienten
Analyse wird der momentane Betrag verwendet, sodass der angegebene Pegel
innerhalb einer Periode schwankt.

Die **Schallintensität** in einer harmonischen Analyse ist die zeitlich
gemittelte Wirkintensität, also der Netto-Schallleistungsfluss. Sie ist in einer
reinen stehenden Welle identisch null und gleich `0.5 * |p| * |u|` in einer
ebenen fortschreitenden Welle.

### 1.9 Transient oder harmonisch?

| | Transient | Harmonisch |
|---|---|---|
| Löst | `M*phi'' + C*phi' + K*phi = f` in der Zeit | `(K - w^2*M + i*w*C)*PHI = F` pro Frequenz |
| Beantwortet | „was passiert, wenn ...“ | „wie ist die stationäre Antwort bei ...“ |
| Anregung | beliebiger Zeitverlauf, einschließlich eines einzelnen Impulses | jeweils eine Sinusschwingung |
| Aufwand | eine SPD-Lösung pro Zeitschritt | eine indefinite Lösung doppelter Größe pro Frequenz |
| Datensätze | einer pro Zeitschritt | einer pro Frequenz |
| Phaseninformation | implizit im Zeitverlauf | explizit, als Ergebnisfeld |

**Verwenden Sie eine transiente Analyse für**

- Impulsantwort-, Echo- und Nachhalluntersuchungen;
- transiente Ereignisse - einen Knall, das Schließen eines Ventils, das
  Zuschlagen einer Tür;
- breitbandige Anregung, bei der ein einziger Lauf mit anschließender
  Fourier-Transformation eines Überwachungspunkts das gesamte Spektrum auf
  einmal abdeckt;
- jeden Fall, in dem der Zeitverlauf selbst die Antwort ist.

**Verwenden Sie eine harmonische Analyse für**

- Frequenzgänge und Kurven des Durchgangsdämpfungsmaßes;
- Hohlraum- und Kanalresonanzen sowie die Druckverteilung bei einer Resonanz;
- die Wirkung von Schalldämpfern und Absorbern über ein Frequenzband;
- Karten des Schalldruckpegels bei einem einzelnen Ton;
- jeden Fall, in dem nur der stationäre Zustand zählt. Ein harmonischer Lauf
  ist meist weit günstiger, als einen transienten Lauf so lange zu rechnen, bis
  die Einschwingvorgänge abgeklungen sind.

Ein praktischer Kompromiss besteht darin, Resonanzen mit einem groben
harmonischen Durchlauf zu lokalisieren und dann nur das interessante Band zu
verfeinern.

---

## 2. Grafische Benutzeroberfläche

Alles, was mit Akustik zusammenhängt, befindet sich im Panel **Löser**
(Registerkarten `Problem`, `Randbedingungen`, `Anfangsbedingungen`, `Material`,
`Ergebnisse`), im Menü **Problem** und im Panel **Modell**.

### 2.1 Auswahl des Akustikproblems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Fügen Sie eine Aufgabe hinzu und wählen Sie
**Acoustics** - „Customized wave equation for solving acoustic problems“
(angepasste Wellengleichung zur Lösung akustischer Probleme).

Akustik schließt sich gegenseitig mit **Wave - general** und mit **Modal
analysis** aus. Sie kann mit anderer Physik kombiniert werden; in diesem Fall
bestimmt der Aufgabenablauf die Ausführungsreihenfolge.

### 2.2 Registerkarte Problem

Sobald Akustik ausgewählt ist, zeigt die Registerkarte `Problem` des Panels
Löser zwei Gruppenfelder.

**Zeitlöser** - wird angezeigt, da Akustik ein zeitabhängiger Problemtyp ist.

| Feld | Bedeutung |
|---|---|
| Aktivieren | schaltet das transiente Zeitschrittverfahren ein und aus |
| Approximation | Approximation des Zeitschrittverfahrens; wird wie in 1.6 beschrieben auf das Newmark-`beta_N` abgebildet |
| Startzeit | erster Zeitwert in Sekunden; wird bei einem Neustart des Lösers ignoriert |
| Endzeit | `start time + time-step size * number of time-steps`, schreibgeschützt |
| Zeitschrittweite | `dt` in Sekunden |
| Anzahl der Zeitschritte | wie viele Schritte berechnet werden |
| Ausgabefrequenz | schreibt alle N Schritte eine Ergebnisdatei; `0` schreibt nur den letzten Schritt |

**Einstellungen der akustischen Analyse** - das für die Akustik hinzugefügte
Gruppenfeld.

| Feld | Bedeutung |
|---|---|
| Analysetyp | *Transient* oder *Harmonic* |
| Erste Frequenz `[Hz]` | erste Frequenz des harmonischen Durchlaufs |
| Frequenzschritt `[Hz]` | Zuwachs zwischen zwei aufeinanderfolgenden Frequenzen |
| Anzahl der Frequenzen | wie viele Frequenzen gelöst werden; jede erzeugt einen eigenen Datensatz |
| Durchlauf | schreibgeschützte Beschriftung, die den resultierenden Frequenzbereich anzeigt |
| Bezugsdruck `[Pa]` | Bezugswert für den Schalldruckpegel, standardmäßig `2e-5 Pa` |

Die drei Frequenzfelder sind ausgegraut, solange *Transient* ausgewählt ist.
Der Bezugsdruck gilt für beide Analysetypen.

### 2.3 Registerkarte Material

Die Registerkarte `Material` listet das der ausgewählten Entität zugewiesene
Material und seine Materialeigenschaften auf. Für die Akustik relevant sind:

| Eigenschaft | Einheiten | Erforderlich | Kommentar |
|---|---|---|---|
| Density | `kg/m^3` | ja | `rho` |
| Speed of sound | `m/s` | eine der beiden | `c`, wird direkt verwendet, wenn ungleich null |
| Modulus of elasticity | `Pa` | eine der beiden | Ersatz, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | nein | `beta`, Volumendämpfung, standardmäßig `0` |

Eine Entität wird vom Akustiklöser gelöst, wenn sie eine Dichte **und** entweder
eine Schallgeschwindigkeit oder einen Elastizitätsmodul hat. Da beide
Alternativen sind, wird keine von der Einstellungsprüfung als fehlend gemeldet;
ein Modell, das keine von beiden hat, wird vom Löser mit einer ausdrücklichen
Meldung abgewiesen.

Jede Eigenschaft ist eine Tabelle über der Temperatur, sodass eine
temperaturabhängige Schallgeschwindigkeit eingegeben werden kann, die
interpoliert wird, wenn parallel ein Wärmeübertragungsproblem läuft.

### 2.4 Registerkarte Randbedingungen

Die Registerkarte ist oben in eine Verwaltungsliste der verfügbaren
Randbedingungen und unten in einen Editor für die ausgewählte Bedingung
aufgeteilt. Wählen Sie zuerst eine Entität im Baum `Modell` aus; die Verwaltung
bietet dann die für diesen Entitätstyp und das ausgewählte Problem gültigen
Randbedingungen an. Aktivieren Sie eine, um sie zuzuweisen, und bearbeiten Sie
dann ihre Komponenten im unteren Baum.

Für die Akustik bietet die Verwaltung an:

- **Velocity potential** - explizit, Punkte, Linien und Flächen. Komponente:
  *Potential* `[m^2/s]`.
- **Velocity** - Punkte, Linien und Flächen. Komponente: *Velocity* `[m/s]`.
- **Absorbing boundary** - Punkte, Linien und Flächen. Komponente: *Acoustic
  absorption coefficient*, dimensionslos, Standard `1.0`.
- **Acoustic impedance** - Punkte, Linien und Flächen. Komponente: *Acoustic
  impedance* `[Pa*s/m]`, Standard `415`.

Jede Komponente ist eine Tabelle über der Zeit, sodass einer Quelle ein
Zeitverlauf gegeben werden kann: Fügen Sie Zeilen hinzu, um eine
Geschwindigkeit hochzufahren oder einen kurzen Impuls auszulösen. In einer
harmonischen Analyse wird der Wert zur aktuellen Lösungszeit verwendet, was bei
einer Tabelle mit einem einzigen Wert einfach dieser Wert ist.

Lassen Sie eine Wand ohne Randbedingung, um sie akustisch schallhart zu machen.

### 2.5 Registerkarte Anfangsbedingungen

Gleicher Aufbau wie die Registerkarte der Randbedingungen. Für die Akustik
bietet sie **Velocity potential** und **Velocity potential / s** an. Letztere
setzt das Anfangsdruckfeld.

### 2.6 Einrichtung des Matrixlösers

`Problem` -> `Matrixlöser der Probleme einrichten` konfiguriert die iterativen
Löser. Eine transiente akustische Analyse verwendet den Eintrag **CG**, eine
harmonische Analyse den Eintrag **GMRES**.

Der Standard-Neustart von GMRES mit `10` ist für das Blocksystem im
Frequenzbereich viel zu klein. Der Löser erhöht ihn automatisch, die Anzahl der
*äußeren* Iterationen und der Konvergenzwert werden jedoch wie eingestellt
übernommen; erhöhen Sie sie daher hier, wenn ein harmonischer Lauf nicht
konvergiert.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable. Ein
Überwachungspunkt auf *Acoustic pressure* ist das natürliche Mikrofon: In einem
transienten Lauf liefert er den zeitlichen Druckverlauf an diesem Ort, der über
`Bericht` -> `Überwachungspunkte` untersucht werden kann.

### 2.8 Ergebnisse

Nach dem Lauf listet die Registerkarte `Ergebnisse` die berechneten Variablen
auf und steuert, was die 3D-Ansicht anzeigt. Die akustischen Variablen sind
die aus Abschnitt 1.8.

Knotenvariablen (Druck, Schalldruckpegel, Phase, Potential) werden als glatte
Felder gezeichnet; Elementvariablen (Schallschnelle, Intensität) sind Vektoren
und lassen sich am besten über ein Vektorfeld oder einen Schnitt darstellen.

Die Registerkarte `Datensätze` des Panels `Modell` listet die einzelnen
Ergebnisdatensätze auf - einen pro Zeitschritt bei einem transienten Lauf, einen
pro Frequenz bei einem harmonischen Durchlauf - und ermöglicht es, sie
schrittweise durchzugehen oder zu animieren.

`Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige Ausgabe des
Lösers, einschließlich des Statistikblocks, der nach jedem Datensatz für jede
akustische Variable ausgegeben wird.

---

## 3. Tutorial - transiente akustische Analyse

**Ziel.** Die Impulsantwort eines geschlossenen Lufthohlraums berechnen und den
Druckverlauf an einer Mikrofonposition ablesen.

Dieses Tutorial setzt ein vernetztes Volumenmodell voraus. Verwenden Sie
`Datei` -> `Neues Modell`, zeichnen Sie einen Quader mit `Geometrie` ->
`Element erstellen` oder importieren Sie eine Geometrie und tetraedrisieren Sie
sie - das Tutorial *Würfel zeichnen* behandelt diese Schritte.

### Schritt 1 - Problem auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Fügen Sie eine Aufgabe hinzu und wählen Sie **Acoustics**.
3. Bestätigen Sie mit `OK`.

Die Registerkarte `Problem` des Panels Löser zeigt nun die Gruppenfelder
*Zeitlöser* und *Einstellungen der akustischen Analyse*.

### Schritt 2 - Material zuweisen

1. Wählen Sie die Volumenentität im Baum `Modell` aus.
2. Öffnen Sie die Registerkarte `Material`.
3. Weisen Sie ein Material zu oder erstellen Sie eines und geben Sie ihm:
   - **Density** = `1.2` `kg/m^3`
   - **Speed of sound** = `344` `m/s`
4. Lassen Sie **Acoustic damping factor** für einen ungedämpften Hohlraum auf
   `0`, oder setzen Sie ihn auf einen kleinen Wert wie `1.0` `1/s`, damit die
   Antwort abklingt.

### Schritt 3 - Analysetyp und Zeitschritt wählen

1. Öffnen Sie die Registerkarte `Problem`.
2. Setzen Sie in *Einstellungen der akustischen Analyse* den **Analysetyp** =
   `Transient`.
3. In *Zeitlöser*:
   - **Aktivieren**: ankreuzen;
   - **Startzeit** = `0`;
   - **Zeitschrittweite** = `2e-5` s. Nach `dt <= 1/(20*f_max)` werden damit
     Frequenzen bis etwa `2.5 kHz` aufgelöst;
   - **Anzahl der Zeitschritte** = `500`, was `10 ms` simulierte Zeit ergibt;
   - **Ausgabefrequenz** = `5`, was 100 Ergebnisdatensätze schreibt.

Prüfen Sie das Netz anhand von `h <= c/(10*f_max)`: Für `f_max = 2.5 kHz` und
`c = 344 m/s` sollte die Elementgröße etwa `14 mm` nicht überschreiten.

### Schritt 4 - Hohlraum anregen

Zwei Möglichkeiten, wählen Sie eine.

**A - eine bewegte Wand.** Wählen Sie die Fläche aus, die die Quelle darstellt,
öffnen Sie die Registerkarte `Randbedingungen` und aktivieren Sie **Velocity**.
Geben Sie in der Komponententabelle einen Zeitverlauf ein, zum Beispiel einen
kurzen Impuls:

| Zeit `[s]` | Velocity `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - ein anfänglich unter Druck stehender Bereich.** Wählen Sie die Entität
aus, die unter Druck starten soll, öffnen Sie die Registerkarte
`Anfangsbedingungen` und aktivieren Sie **Velocity potential / s**. Geben Sie
`p0 / rho` ein; für `p0 = 1 Pa` und `rho = 1.2 kg/m^3` sind das `0.833`.

Lassen Sie alle anderen Flächen ohne Randbedingung, damit der Hohlraum
schallhart begrenzt ist, oder aktivieren Sie **Absorbing boundary** mit einem
zur Wandverkleidung passenden Absorptionsgrad, damit Energie entweichen kann.

### Schritt 5 - Mikrofon hinzufügen

1. `Problem` -> `Überwachungspunkte definieren`.
2. Fügen Sie am interessierenden Ort einen Punkt hinzu.
3. Wählen Sie **Acoustic pressure** als seine Variable.

### Schritt 6 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Die Einstellungsprüfung meldet fehlende Materialien oder Randbedingungen vor
   dem Start des Laufs.
3. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.
   Jeder Zeitschritt gibt die CG-Konvergenz und die Statistik jeder akustischen
   Variable aus.

### Schritt 7 - Ergebnisse untersuchen

- Panel `Modell`, Registerkarte `Datensätze`: Gehen Sie die Zeitdatensätze
  schrittweise durch oder animieren Sie sie.
- Registerkarte `Ergebnisse`: Zeigen Sie **Acoustic pressure** oder **Sound
  pressure level** auf dem Modell und **Acoustic particle velocity** als
  Vektorfeld an.
- `Bericht` -> `Überwachungspunkte`: der Druckverlauf am Mikrofon. Seine
  Abklingrate ergibt die Nachhallzeit; seine Fourier-Transformation ergibt die
  Resonanzen des Hohlraums.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* | *Aktivieren* ist in *Zeitlöser* deaktiviert, oder der Analysetyp sollte *Harmonic* sein |
| *"Acoustic solver requires a positive time step size"* | die Zeitschrittweite ist null |
| *"No element with a valid speed of sound was found"* | keine Entität hat eine Dichte zusammen mit einer Schallgeschwindigkeit oder einem Elastizitätsmodul |
| Die Antwort klingt nie ab | nirgends Dämpfung - erwartet für einen vollkommen schallharten, verlustfreien Hohlraum; fügen Sie eine absorbierende Randbedingung oder einen akustischen Dämpfungsfaktor hinzu |
| Wellen vor der Wellenfront | das Netz oder der Zeitschritt ist für den Frequenzinhalt zu grob; verfeinern Sie beides |

---

## 4. Tutorial - harmonische akustische Analyse

**Ziel.** Den Frequenzgang eines reflexionsfrei abgeschlossenen Kanals und für
jede Frequenz eine Karte des Schalldruckpegels berechnen.

### Schritt 1 - Problem auswählen

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`), fügen Sie eine Aufgabe
hinzu und wählen Sie **Acoustics**.

### Schritt 2 - Material zuweisen

Wie im transienten Tutorial: **Density** `1.2 kg/m^3` und **Speed of sound**
`344 m/s` auf der Fluidentität.

Legen Sie das Netz für die **höchste** Frequenz des Durchlaufs aus:
`h <= c / (10 * f_max)`.

### Schritt 3 - Durchlauf konfigurieren

Öffnen Sie die Registerkarte `Problem` und stellen Sie in *Einstellungen der
akustischen Analyse* Folgendes ein:

| Feld | Wert |
|---|---|
| Analysetyp | `Harmonic` |
| Erste Frequenz | `50` Hz |
| Frequenzschritt | `50` Hz |
| Anzahl der Frequenzen | `20` |
| Bezugsdruck | `2e-5` Pa |

Die Beschriftung *Durchlauf* bestätigt `50 - 1000 [Hz]`, also 20 Lösungen und
20 Ergebnisdatensätze.

Das Gruppenfeld *Zeitlöser* kann aktiviert bleiben - eine harmonische Analyse
ignoriert es und schreibt eine Warnung ins Protokoll. Seine Einstellungen
bleiben erhalten.

### Schritt 4 - Kanal anregen

Wählen Sie die Einlassfläche aus, öffnen Sie die Registerkarte
`Randbedingungen` und aktivieren Sie **Velocity**. Geben Sie eine einzige Zeile
mit der Kolbengeschwindigkeit ein, zum Beispiel `0.01` `m/s`. Die Amplitude
wird als Amplitude der harmonischen Anregung bei jeder Frequenz des Durchlaufs
interpretiert.

### Schritt 5 - Kanal abschließen

Wählen Sie die Auslassfläche aus und aktivieren Sie entweder:

- **Absorbing boundary** mit **Acoustic absorption coefficient** = `1.0` für
  einen vollkommen reflexionsfreien Abschluss; oder
- **Acoustic impedance** mit **Acoustic impedance** = `rho * c` = `413`
  `Pa*s/m`, was dasselbe anders ausgedrückt ist.

Verwenden Sie einen kleineren Absorptionsgrad, um einen teilweise
reflektierenden Abschluss zu modellieren, und lassen Sie den Auslass ohne
Bedingung, um ein geschlossenes, schallhartes Ende zu modellieren.

Die Kanalwände benötigen keine Randbedingung - sie sind standardmäßig
schallhart.

### Schritt 6 - Matrixlöser prüfen

`Problem` -> `Matrixlöser der Probleme einrichten`, Eintrag **GMRES**. Der
Neustart wird automatisch erhöht; zeigt das Protokoll jedoch ein stagnierendes
Residuum, erhöhen Sie die Anzahl der äußeren Iterationen und lockern oder
verschärfen Sie den Konvergenzwert nach Bedarf.

### Schritt 7 - lösen

`Lösung` -> `Löser starten` (`Ctrl+R`). Das Protokoll zeigt einen Block pro
Frequenz:

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

### Schritt 8 - Ergebnisse untersuchen

- Panel `Modell`, Registerkarte `Datensätze`: ein Datensatz pro Frequenz. Das
  schrittweise Durchgehen entspricht dem Frequenzdurchlauf.
- Registerkarte `Ergebnisse`: Zeigen Sie **Sound pressure level** für eine
  Dezibelkarte, **Acoustic pressure** für die Amplitude und **Acoustic phase**
  zur Darstellung der Wellenfronten an.
- **Acoustic intensity** zeigt, wohin die Schallleistung fließt. In einem gut
  abgeschlossenen Kanal ist sie gleichmäßig und zum Auslass gerichtet; in einem
  resonanten Hohlraum ist sie nahezu null.
- Ein Überwachungspunkt auf **Acoustic pressure** zeichnet die Amplitude bei
  jeder Frequenz auf, also den Frequenzgang an diesem Ort.

### Interpretation des Ergebnisses

Für einen geraden Kanal, der mit `v` angeregt und reflexionsfrei abgeschlossen
ist, lautet die analytische Lösung für ebene Wellen

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`constant along the duct` = konstant entlang des Kanals, `with` = mit.) Für
`rho = 1.2`, `c = 344`, `v = 0.01` ergibt das `|p| = 4.13 Pa`, einen
Schalldruckpegel von `103.3 dB` und eine Intensität von `0.0206 W/m^2`.
Abweichungen von einem konstanten `|p|` entlang des Kanals bedeuten, dass der
Abschluss reflektiert - entweder beabsichtigt oder weil der Absorptionsgrad
nicht `1` ist.

Mit einem schallharten Abschluss wird derselbe Kanal resonant. Die
Druckamplitude erreicht Spitzen bei den Viertelwellenresonanzen
`f = (2n+1) * c / (4L)`, und die Phase ist konstant bei `+/-90` Grad, mit einer
Nettointensität von null.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* | die erste Frequenz ist null und der Frequenzschritt ist null |
| Das Residuum stagniert, die Ergebnisse wirken verrauscht | GMRES ist nicht konvergiert; erhöhen Sie die äußeren Iterationen in der Einrichtung des Matrixlösers oder vergröbern Sie das Netz |
| Die Amplitude explodiert bei einer Frequenz | der Durchlauf hat eine ungedämpfte Resonanz getroffen; fügen Sie eine absorbierende Randbedingung oder einen akustischen Dämpfungsfaktor hinzu |
| Der Druck schwankt entlang eines vermeintlich reflexionsfreien Kanals stark | der Abschluss reflektiert; prüfen Sie den Absorptionsgrad oder den Impedanzwert |

---

## 5. Verifikation

Der Unit-Test `tst_solver_acoustic` (in `range-solver-lib/tests`, gebaut mit
`-DRANGE_BUILD_TESTS=ON`) prüft den Löser gegen geschlossene Lösungen an einem
eindimensionalen Kanal von `1 m`, diskretisiert mit 200 Stabelementen, mit den
Standardeinstellungen des Matrixlösers.

**Harmonisch, reflexionsfreier Abschluss** - `rho = 1.2 kg/m^3`,
`c = 340 m/s`, `v = 0.01 m/s`, `f = 500 Hz`:

| Größe | Analytisch | Berechnet |
|---|---|---|
| Druckamplitude | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` in jedem Knoten |
| Potentialamplitude | `v/k = 1.0823e-3 m^2/s` | innerhalb von 1 % |
| Schallschnelle | `0.01 m/s` | `0.009999 m/s` |
| Schalldruckpegel | `103.18 dB` | innerhalb von `0.1 dB` |
| Intensität | `0.0204 W/m^2` | innerhalb von 2 % |
| Phasengradient | `k * dx` | innerhalb von 5 Grad |

**Harmonisch, schallharter Abschluss** - die stehende Welle ergibt eine Phase
von genau `+/-90` Grad und eine Nettointensität von null, und die
Spitzenamplitude übersteigt die Amplitude der fortschreitenden Welle.

**Transient** - ein angeregter, reflexionsfrei abgeschlossener Kanal bleibt
beschränkt, der Newmark-Zustand wird veröffentlicht, sodass die Integration über
die Schritte hinweg korrekt fortgesetzt wird, und das angeregte Feld ist
überall ungleich null und endlich.

---

## 6. Einschränkungen

- **Keine Struktur-Akustik-Kopplung.** Akustik kann nicht mit der Modalanalyse
  kombiniert werden, und es gibt keine Fluid-Struktur-Schnittstelle mit
  `RSolverStress`. Eine schwingende Struktur muss durch eine vorgeschriebene
  Randbedingung *Velocity* dargestellt werden.
- **Nur lineare Akustik.** Kleine Amplituden, ruhendes Medium. Keine
  Grundströmung, keine konvektiven Effekte, keine nichtlineare Ausbreitung.
- **Nur reelle Impedanz.** Die absorbierende und die Impedanz-Randbedingung
  nehmen einen reellen Wert an. Eine komplexe, frequenzabhängige
  Oberflächenimpedanz wird nicht unterstützt.
- **Frequenzunabhängige Materialdaten.** Schallgeschwindigkeit und
  Dämpfungsfaktor sind über der Temperatur tabelliert, nicht über der Frequenz,
  und daher über einen Durchlauf konstant.
- **Randterme verwenden das einfache Elementmaß.** Bei einem ein- oder
  zweidimensionalen Modell, dessen Querschnittsfläche oder Dicke nicht `1` ist,
  werden Quell- und Impedanzterme nicht damit skaliert, die Gebietsterme
  hingegen schon. Verwenden Sie eine Einheits-Querschnittsfläche oder ein
  vollständiges dreidimensionales Modell, um die Abweichung zu vermeiden.
- **Harmonische Anregung ist reell.** Eine vorgeschriebene Geschwindigkeit geht
  als Realteil der komplexen Last ein; Quellen mit einer relativen
  Phasenverschiebung können nicht definiert werden.
