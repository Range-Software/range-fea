# Magnetostatik - Theoriehandbuch und Benutzerhandbuch

Dieses Dokument beschreibt den Magnetostatik-Löser (`RSolverMagnetostatics`) von
Range FEA, der den Problemtyp **Magnetostatik** (*Magneto-statics*) bedient: das
Gesetz, das er auswertet, die Eingaben, die er akzeptiert, die Teile der
grafischen Benutzeroberfläche, die ihn steuern, ein ausgearbeitetes Tutorial und
was sein Ergebnis wert ist.

> **Status.** Der Löser wertet das Magnetfeld des von der Elektrostatik-Aufgabe
> berechneten Stroms direkt mit dem **Biot-Savart-Gesetz** aus. Es gibt kein
> Gleichungssystem und keine Randbedingung: Das Feld wird allein durch den Strom
> bestimmt, und die Bedingung, dass es fern vom Leiter verschwindet, ist im
> Gesetz enthalten. Für die gegebene Stromdichte ist das Ergebnis bis auf einen
> kontrollierten Quadraturfehler deutlich unter `1e-4` des Spitzenfelds exakt.
> Der Löser arbeitet im **freien Raum** - magnetische Materialien können nicht
> abgebildet werden - und er berechnet nur das Feld des modellierten Stroms,
> daher sollte der Strompfad innerhalb des Modells geschlossen sein. Abschnitt 5
> listet die Einschränkungen auf.

**Inhalt**

1. [Theoretische Grundlagen](#1-theoretische-grundlagen)
2. [Grafische Benutzeroberfläche](#2-grafische-benutzeroberfläche)
3. [Tutorial - das Feld eines stromdurchflossenen Leiters](#3-tutorial---das-feld-eines-stromdurchflossenen-leiters)
4. [Überprüfung eines Modells](#4-überprüfung-eines-modells)
5. [Einschränkungen](#5-einschränkungen)

---

## 1. Theoretische Grundlagen

### 1.1 Das Ergebnis - das Magnetfeld

Das Ergebnis ist die **magnetische Flussdichte** `B` in jedem Knoten des Modells,
in Tesla, als Vektor mit drei Komponenten.

Sie wird **ausgewertet, nicht gelöst**. Sobald die Stromdichte bekannt ist,
ergibt sich das Magnetfeld im freien Raum daraus durch ein Integral, daher
assembliert der Löser keine Matrix und führt keine Iteration durch. Jeder Knoten
des Modells erhält einen Wert, unabhängig davon, ob er im Leiter, auf seiner
Oberfläche oder irgendwo sonst im Netz liegt.

### 1.2 Grundgesetz

Im freien Raum erfüllt das statische Magnetfeld das Ampèresche Gesetz und hat
keine Quellen:

```
curl(B) = u0 * J
div(B)  = 0
```

zusammen mit der Bedingung, dass `B` fern von allen Strömen verschwindet. Das
eindeutige Feld, das alle drei erfüllt, ist durch das **Biot-Savart-Gesetz**
gegeben

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Symbol | Bedeutung | Einheiten | Quelle |
|---|---|---|---|
| `B` | magnetische Flussdichte | `T` | berechnet |
| `J` | Stromdichte | `A/m^2` | aus der Elektrostatik-Aufgabe, siehe 1.4 |
| `u0` | magnetische Feldkonstante | `H/m` | eingebaut, `1.25663706212e-6` |
| `x` | Feldpunkt - ein Knoten des Modells | `m` | |
| `y` | Quellpunkt - innerhalb eines leitenden Elements | `m` | |

Drei Eigenschaften folgen aus dem Gesetz selbst und nicht aus etwas, das der
Löser tut:

- **`div(B) = 0` gilt exakt**, überall, unabhängig vom Netz.
- **Das Fernfeld stimmt ohne Rand.** `B` fällt mit der Entfernung so ab, wie es
  das Gesetz vorschreibt; es gibt keine äußere Fläche, keinen zu vernetzenden
  Luftbereich und keine dort zu setzende Bedingung.
- **`curl(B) = u0 * J` gilt überall dort, wo der Strom geschlossen ist**, also
  wo `div(J) = 0` gilt und der Strompfad Schleifen bildet. Abschnitt 1.4 erklärt,
  was das für ein elektrostatisches Modell bedeutet, dessen Strom in den Körper
  ein- und wieder austritt.

Die Permeabilität des Materials geht nirgendwo ein. Es gibt keine
Materialeigenschaft relative Permeabilität, keine Magnetisierung und keine
Unterscheidung zwischen `B` und `H` - der Löser arbeitet im freien Raum,
unabhängig davon, woraus das Modell besteht. Abschnitt 1.9 erklärt, warum dies
im Rahmen desselben Ansatzes nicht aufgehoben werden kann.

**Es ist keine Materialeigenschaft erforderlich.** Der Problemtyp Magnetostatik
deklariert überhaupt keine, und die Einstellungsprüfung hat für ihn nichts zu
prüfen. Die Elektrostatik-Aufgabe, von der der Problemtyp abhängt, hat eigene
drei Anforderungen - siehe Handbuch Elektrostatik.

### 1.3 Diskretisierung

Der Elektrostatik-Löser liefert eine Stromdichte, die **in jedem Element konstant**
ist. Das Integral aus 1.2 zerfällt daher in ein Integral pro Element, jeweils mit
einem konstanten `J_e`, das herausgezogen werden kann:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` hängt nur von der Geometrie des Elements und vom Feldpunkt ab. Jeder vom
Löser akzeptierte Elementtyp hat ebene Seiten, und für diese hat `G_e` eine
geschlossene Form, daher integriert der Löser jedes Element **exakt** statt
näherungsweise:

- **Tetraeder.** Der Integrand ist der Gradient von `1/|x - y|` bezüglich `y`,
  daher wird das Volumenintegral zu einer Summe über die vier Seiten,
  `G = SUM_f n_f * INT_f 1/R dA`. Das Integral von `1/R` über ein ebenes Dreieck
  ist in geschlossener Form bekannt: ein Logarithmus entlang jeder Kante und der
  Raumwinkel, unter dem das Dreieck von `x` aus erscheint,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Dreieck** einer Flächenentität mit einer Dicke `t`. Der Strom ist ein
  Flächenstrom `K = J*t` in `A/m`. Der tangentiale Anteil von `G` ist wieder eine
  Summe von Kantenlogarithmen, `SUM_e m_e * I_e`, und der normale Anteil ist der
  vorzeichenbehaftete Raumwinkel.
- **Viereck.** Wird in zwei Dreiecke zerlegt.
- **Liniensegment** einer Linienentität mit einer Querschnittsfläche `A`. Der
  Strom ist `I = A * (J . t)` entlang des Segments, und das Feld eines geraden
  endlichen Drahts ist klassisch:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  wobei `a` und `b` die Vektoren vom Feldpunkt zu den beiden Enden sind.

Dabei ist `I_e` das Integral von `1/R` entlang der Kante `e`, `P_e` der Abstand
des Feldpunkts von dieser Kante in der Ebene, `m_e` die nach außen gerichtete
Normale der Kante in der Ebene, `d` die Höhe des Feldpunkts über der Ebene der
Seite und `omega` der Raumwinkel der Seite vom Feldpunkt aus gesehen.

**Entfernte Elemente** benötigen die geschlossene Form nicht. Der Integrand ist
dort glatt, und eine Quadraturregel ist zu einem Bruchteil der Kosten so gut wie
exakt:

| Abstand des Feldpunkts vom Elementmittelpunkt | Regel |
|---|---|
| unter 2 x der längsten Elementkante | geschlossene Form |
| 2 bis 6 x | Regel zweiten Grades - 3 Punkte auf einem Dreieck, 4 auf einem Tetraeder |
| über 6 x | Mittelpunktregel - das Element als Punktstrom |

Liniensegmente verwenden immer die geschlossene Form, die so günstig ist wie ein
Quadraturpunkt. Der Gesamtfehler der Regeln wird von den Unit-Tests gegen die
über alle Elemente eines ganzen Leiters summierte geschlossene Form gemessen und
bleibt in jedem Knoten unter `1e-4` des Spitzenfelds - im Leiter, auf seiner
Oberfläche und darüber hinaus.

**Das Feld am Leiter selbst.**

- Für einen **Volumenstrom** ist das Feld überall stetig und endlich,
  einschließlich der Knoten, Kanten und Seiten der leitenden Elemente. Ein Knoten
  auf der Oberfläche eines massiven Leiters erhält das wahre Oberflächenfeld.
- Eine **Linien**- oder **Flächen**-Entität steht für einen Leiter mit
  Querschnitt null, und das Feld eines solchen Leiters ist singulär: Es wächst zu
  einem Draht hin mit `1/r` und zur Kante einer Schicht hin logarithmisch, und es
  springt über eine Schicht hinweg um `u0 * K x n`. In einem Knoten auf einem
  solchen Leiter liefert der Löser einen endlichen Wert: Ein Segment trägt zu
  einem Punkt auf seiner eigenen Geraden nichts bei, eine Schicht trägt den
  Mittelwert der Felder auf ihren beiden Seiten bei, und der Kantenlogarithmus
  einer Schicht wird in einem Knoten auf dieser Kante weggelassen. Diese Werte
  sind regularisiert und nicht physikalisch. Lesen Sie das Feld eines dünnen
  Leiters einige Querschnittsgrößen von ihm entfernt ab, wo das dünne Modell
  gültig ist.

### 1.4 Die Stromdichte als Quelle

Das Magnetfeld wird nur durch eine Eingabe angetrieben: die **Stromdichte**, die
eine Aufgabe *Electro-statics* weiter vorne im Aufgabenablauf des Problems
hinterlässt. Es gibt nichts einzugeben und nichts zuzuweisen.

Der Löser liest die von der Elektrostatik-Aufgabe gespeicherte
**Element**-Stromdichte und verwendet sie unverändert - es gibt keine Mittelung
auf Knoten. Die stromführenden Elemente sind:

| Entität | Bedingung | Quelle |
|---|---|---|
| Volumen | Tetraeder | `J` |
| Fläche | Dicke `> 0` | Flächenstrom `J * thickness` |
| Linie | Querschnittsfläche `> 0` | Strom `J * cross area` |

Elemente mit Stromdichte null werden übersprungen, also jedes Element, das die
Elektrostatik-Aufgabe nicht berechnet hat. Ebenso Elemente, deren Stromdichte
unter `1e-10` der größten im Modell liegt: Ein schlecht leitender Bereich - Luft
mit einer Leitfähigkeit von `5e-15 S/m` neben Kupfer - würde sonst eine Quelle
pro Element und nichts Messbares zum Feld beitragen. Führt kein Element Strom -
die Elektrostatik-Aufgabe ist nicht gelaufen, die Potentiale sind gleich oder
kein Material hat eine elektrische Leitfähigkeit -, gibt das Protokoll eine
Warnung aus, und das Feld ist überall null.

**Der Strom muss geschlossen sein.** Das Biot-Savart-Gesetz liefert das Feld des
ihm übergebenen Stroms und nichts anderes. Ein physikalischer Strom fließt in
geschlossenen Schleifen, ein elektrostatisches Modell enthält aber meist nur
einen Teil der Schleife: einen Stab zwischen zwei Elektroden, bei dem der Strom
an einem Ende ein- und am anderen austritt. Die Zuleitungen und der Rest des
Stromkreises sind nicht im Modell, und ihr Feld ebenso wenig. Was der Löser dann
berechnet, ist das Feld des modellierten Leiters, als ob der Strom an einem Ende
entstünde und am anderen verschwände:

- **in der Nähe des Leiters** ist dies meist eine ausgezeichnete Näherung, da
  das Feld dort vom nahen Strom dominiert wird;
- **weit davon entfernt**, in Abständen vergleichbar mit dem Leiter selbst, ist
  der fehlende Teil der Schleife ebenso wichtig wie der modellierte, und das
  Ergebnis ist nicht das Feld eines realen Stromkreises.

Wenn das Fernfeld wichtig ist, nehmen Sie den Rückleiter in das
elektrostatische Modell auf, damit der Strom eine Schleife bildet. Ein dünner
Rückleiter kann durch eine Linienentität mit Querschnittsfläche abgebildet
werden.

### 1.5 Rand-, Anfangs- und Umgebungsbedingungen

Es gibt **keine**, und es werden keine benötigt.

| Art der Bedingung | Angeboten für Magnetostatik |
|---|---|
| Randbedingungen | keine |
| Anfangsbedingungen | keine |
| Umgebungsbedingungen | keine |
| Materialeigenschaften | keine |

Das Feld ist vollständig durch den Strom und durch die Forderung festgelegt, dass
es in großer Entfernung verschwindet, was das Biot-Savart-Gesetz mit sich
bringt. Es gibt keinen äußeren Rand zu schließen und nichts darauf
vorzuschreiben.

Wenn sich eine Elektrostatik- und eine Magnetostatik-Aufgabe einen
Aufgabenablauf teilen - was immer der Fall ist, siehe 1.6 -, bieten die
Registerkarten der Bedingungen trotzdem die **elektrostatischen** Bedingungen
an, weil die Registerkarten aus den Problemtypen des gesamten Ablaufs befüllt
werden. *Electric potential* und *Charge density* gehören zur
Elektrostatik-Aufgabe und erreichen das Magnetfeld über die Stromdichte, die sie
erzeugen.

### 1.6 Kopplung mit anderen Lösern

| Richtung | Variable | Wirkung |
|---|---|---|
| Elektrostatik -> Magnetostatik | Current density | die einzige Quelle des Magnetfelds |

**Magnetostatik erfordert Elektrostatik.** Die Abhängigkeit ist im Problemtyp
deklariert, und die Benutzeroberfläche erzwingt sie: Das Aktivieren von
*Magneto-statics* in der Problemauswahl aktiviert automatisch
*Electro-statics*, und das Deaktivieren von *Electro-statics* deaktiviert
*Magneto-statics* mit. Der Standard-Aufgabenablauf setzt die
Elektrostatik-Aufgabe an die erste Stelle, also in die Reihenfolge, die die
Kopplung benötigt.

Nichts verwendet das Magnetfeld weiter. Es ist das Ende seiner Kette - kein
Löser liest es, und es gibt keine Lorentzkraft, keinen induzierten Strom und
keinen magnetischen Beitrag zu einer anderen Physik.

Da der Löser sich bedingungslos als konvergiert meldet, hält eine Aufgabengruppe
mit einer Magnetostatik-Aufgabe immer nach einer Iteration an.

### 1.7 Auswertung des Felds

Jedes stromführende Element trägt zu jedem Knoten bei, daher wächst der Aufwand
mit der **Anzahl der Knoten mal der Anzahl der stromführenden Elemente**. Die
Auswertung läuft parallel über die Knoten, die in Blöcken verarbeitet werden,
sodass jedes Element einmal pro Block statt einmal pro Knoten aus dem Speicher
geholt wird.

Als Anhaltspunkt: Ein Stab aus `120 000` Tetraedern, ausgewertet in seinen
`24 000` Knoten, benötigt auf 14 Threads etwa fünf Sekunden. Eine Verdopplung
des Netzes in jeder Richtung verachtfacht jeweils die Anzahl der Knoten und der
Elemente und die Zeit etwa um das Vierundsechzigfache; ein Modell mit einigen
hunderttausend Elementen benötigt Minuten statt Sekunden.

Nur stromführende Elemente zählen. Ein großes Netz um einen kleinen Leiter - ein
Luftbereich, siehe 2.8 - fügt Knoten, aber keine Quellen hinzu.

Die Einrichtung des Matrixlösers wird nicht verwendet.

### 1.8 Ergebnisse

| Ergebnis | Anwendbar auf | Bedeutung |
|---|---|---|
| Magnetic field `[T]` | Knoten | der Feldvektor |

Das ist die gesamte Ausgabe. `process()` tut nichts, daher gibt es keine
abgeleitete Größe - keinen separat gespeicherten Feldbetrag, keinen Fluss, keine
Energiedichte. Die 3D-Ansicht kann Betrag und Komponenten der Vektorvariable auf
die übliche Weise anzeigen, und `Magnetic field in X direction` sowie die
Entsprechungen für `Y` und `Z` stehen für die Komponentenanzeige zur Verfügung.

Das Magnetfeld wird als **Knoten**variable gespeichert, anders als die
elektrostatischen Feldergebnisse, die Elementvariablen sind. **Jeder Knoten**
des Modells hat einen Wert, auch Knoten von Entitäten, die keinen Strom führen,
sodass ein Netz um den Leiter das Feld im Raum um ihn herum zeigt.

Nach dem Schreiben des Datensatzes gibt das Löserprotokoll die Statistik des
Magnetfelds und die Werte an allen Überwachungspunkten aus.

### 1.9 Was der Löser ist und was nicht

**Er ist ein Integral, keine Feldgleichung.** Die Auswertung von Biot-Savart
benötigt weder einen Rand noch ein Netz außerhalb des Leiters und ist für den
gegebenen Strom exakt. Der Preis ist, dass sie nur in einem **homogenen** Medium
funktioniert. Mit magnetischen Materialien hängt das Feld eines Stroms davon ab,
wo sich das Eisen befindet, und kein Integral allein über den Strom kann das
ausdrücken; dieses Problem wird mit einer Potentialformulierung gelöst -
üblicherweise einem magnetischen Vektorpotential auf Kantenelementen -, was ein
anderer Löser ist und keine Erweiterung dieses Lösers.

**Er wird von einem berechneten Leitungsstrom angetrieben.** Es gibt keine
Möglichkeit, einen Strom direkt anzugeben, keinen Permanentmagneten und kein
eingeprägtes Feld; der Strom muss durch eine elektrostatische Lösung auf einem
leitenden Körper erzeugt werden, und seine Genauigkeit bestimmt die Genauigkeit
des magnetischen Ergebnisses. Das Handbuch Elektrostatik beschreibt, was diese
Stromdichte ist und was nicht. Eine pro Element konstante Stromdichte ist eine
Näherung erster Ordnung der realen und konvergiert bei Verfeinerung des Netzes.

**Er berechnet das Feld des modellierten Stroms.** Siehe 1.4 - der Strom sollte
innerhalb des Modells geschlossen sein, wenn das Feld fern vom Leiter wichtig
ist.

### 1.10 Statische Analyse und der Zeitlöser

Magnetostatik ist als Problemtyp **ohne** Zeitlöser deklariert, genau wie die
Elektrostatik. Das Gruppenfeld *Zeitlöser* erscheint nicht auf der
Registerkarte `Problem`, der Lauf erzeugt einen einzigen Datensatz, und es gibt
nichts zu konfigurieren.

Da ein Magnetostatik-Ablauf immer auch eine Elektrostatik-Aufgabe enthält und
keine der beiden einen Zeitlöser unterstützt, entfernt das Hinzufügen einer der
beiden zu einem Ablauf den Zeitlöser aus dem gesamten Ablauf. Eine dahinter
verkettete Wärmeaufgabe kann nur stationär sein. Das Handbuch Elektrostatik
behandelt dies in seinem eigenen Abschnitt 1.10.

---

## 2. Grafische Benutzeroberfläche

Alles Relevante befindet sich im Panel **Löser** (Registerkarten `Problem`,
`Material`, `Ergebnisse`), im Menü **Problem** und im Panel **Modell**. Die
Registerkarten der Bedingungen enthalten für diesen Problemtyp nichts.

### 2.1 Auswahl des Problems

`Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`) öffnet den Dialog für den
Aufgabenablauf des Problems. Fügen Sie eine Aufgabe hinzu und wählen Sie
**Magneto-statics** - „Calculate magnetic field“ (Magnetfeld berechnen).

**Electro-statics wird für Sie aktiviert.** Die Auswahl erzwingt die
Abhängigkeit aus 1.6 in beide Richtungen: Das Aktivieren von *Magneto-statics*
aktiviert *Electro-statics*, und das Deaktivieren von *Electro-statics*
deaktiviert *Magneto-statics*. Der Standard-Aufgabenablauf setzt die
Elektrostatik-Aufgabe an die erste Stelle.

Magnetostatik lässt sich mit jeder anderen Physik außer *Modal analysis*
kombinieren. In der Praxis ist die einzige sinnvolle Kette diejenige, auf der
sie aufbaut:

- *Electro-statics*, dann *Magneto-statics* - das Magnetfeld eines berechneten
  Leitungsstroms.

### 2.2 Registerkarte Problem

Diese Registerkarte ist für einen Ablauf aus Elektrostatik und Magnetostatik
**leer**. Keiner der beiden Problemtypen hat einen Zeitlöser, und keiner hat ein
eigenes Einstellungs-Gruppenfeld.

### 2.3 Registerkarte Material

Magnetostatik benötigt vom Material **nichts**. Die Elektrostatik-Aufgabe, von
der sie abhängt, benötigt drei Eigenschaften - relative Permittivität,
elektrische Leitfähigkeit und Dichte -, und eine Entität, deren Material eine
davon fehlt, wird von *dieser* Aufgabe übersprungen, sodass sie keine
Stromdichte und damit keinen Beitrag zum Magnetfeld hat.

Das Material spielt also dennoch eine Rolle, und zwar ausschließlich über die
elektrostatische Hälfte der Kette. Weisen Sie ein Material mit allen drei
Eigenschaften zu, etwa das mitgelieferte *Copper*, und lesen Sie im Handbuch
Elektrostatik nach, wofür jede verwendet wird.

Es gibt keine Eigenschaft relative Permeabilität zuzuweisen. Der Löser arbeitet
im freien Raum.

### 2.4 Registerkarte Randbedingungen

Nichts auf dieser Registerkarte gehört zur Magnetostatik-Aufgabe. Die
angebotenen Bedingungen - *Electric potential* und *Charge density* - sind die
elektrostatischen, und sie treiben die Stromdichte an, die ihrerseits das
Magnetfeld erzeugt. Weisen Sie sie wie im Handbuch Elektrostatik beschrieben zu.

Es gibt keine magnetostatische Randbedingung, und es wird keine benötigt - siehe
1.5.

### 2.5 Anfangs- und Umgebungsbedingungen

Keine der beiden Registerkarten bietet etwas für die Magnetostatik an. Die
angezeigte Anfangsbedingung *Electric potential* und die Umgebungsbedingung
*Temperature* gehören zur Elektrostatik-Aufgabe.

### 2.6 Einrichtung des Matrixlösers

Der Magnetostatik-Löser löst kein Gleichungssystem und verwendet die Einrichtung
des Matrixlösers nicht. Der Eintrag **CG** konfiguriert nur den
Elektrostatik-Löser.

### 2.7 Überwachungspunkte

`Problem` -> `Überwachungspunkte definieren` platziert Messpunkte an
angegebenen Koordinaten und wählt die aufzuzeichnende Variable. *Magnetic field*
ist zusammen mit den elektrostatischen Variablen in der Liste verfügbar und
liest das Knotenfeld am Messpunkt.

### 2.8 Ergebnisse und Datensätze

Die Registerkarte `Ergebnisse` listet **Magnetic field** als Knotenvektor
neben den elektrostatischen Ergebnissen der ersten Aufgabe auf. Die
Registerkarte `Datensätze` des Panels `Modell` enthält genau einen Datensatz:
Beide Problemtypen sind statisch.

`Bericht` -> `Protokolldatei des Lösers` zeigt die vollständige Ausgabe: zuerst
die Statistik der Elektrostatik, dann die Anzahl der stromführenden Elemente und
der Knoten, für die das Biot-Savart-Integral ausgewertet wurde, und schließlich
die Statistik des Magnetfelds.

**Das Feld um einen Leiter sehen.** Das Feld wird in jedem Knoten berechnet,
daher zeigt es alles, was um den Leiter vernetzt ist. Vernetzen Sie den
umgebenden Raum als separate Volumenentität - einen Luftbereich - und geben Sie
ihr ein Material ohne elektrische Leitfähigkeit: Die Elektrostatik-Aufgabe
überspringt sie, sodass sie keinen Strom führt, ihre Knoten aber das Magnetfeld
erhalten. Der Bereich kostet Knoten, aber keine Quellen (1.7), und benötigt
keine bestimmte Größe und keinen Rand, da auf ihm nichts gelöst wird.

Das mitgelieferte Material *Air* hat eine elektrische Leitfähigkeit, wenn auch
eine winzige, sodass die Elektrostatik-Aufgabe den Bereich in ihre eigene Lösung
einbeziehen würde. Verwenden Sie eine Kopie davon, bei der die Eigenschaft
elektrische Leitfähigkeit entfernt ist. Die Einstellungsprüfung warnt dann,
dass dem Material der Entität erforderliche Eigenschaften fehlen; für den
Luftbereich ist das zu erwarten, und der Lauf wird fortgesetzt.

---

## 3. Tutorial - das Feld eines stromdurchflossenen Leiters

**Ziel.** Die Kette aus Elektrostatik und Magnetostatik von Anfang bis Ende an
einem stromdurchflossenen Kupferstab durchlaufen, das Magnetfeld mit einer
Handrechnung vergleichen und sehen, wo sich die Näherungen aus 1.4 zeigen.

Dieses Tutorial setzt den vernetzten Stab aus dem Handbuch Elektrostatik voraus -
einen Quader von `0.1 m` Länge mit quadratischem Querschnitt von `0.01 m`,
tetraedrisiert, mit den beiden Stirnflächen als separate Flächenentitäten
markiert. Ist dieses Modell noch eingerichtet, ändert sich nur der
Aufgabenablauf.

### Schritt 1 - Probleme auswählen

1. `Problem` -> `Aufgabenablauf der Probleme` (`Ctrl+P`).
2. Aktivieren Sie **Magneto-statics**. Beachten Sie, wie sich
   **Electro-statics** von selbst aktiviert.
3. Bestätigen Sie mit `OK`.

Der Aufgabenablauf enthält nun zwei Aufgaben, die Elektrostatik zuerst. Die
Registerkarte `Problem` bleibt leer.

### Schritt 2 - Material zuweisen

Wählen Sie die Volumenentität aus und weisen Sie auf der Registerkarte
`Material` **Copper** zu, oder ein Material mit den drei Eigenschaften, die die
Elektrostatik-Aufgabe benötigt:

- **Relative permittivity** = `2.8`
- **Electrical conductivity** = `5.882e7` `S/m`
- **Density** = `8960` `kg/m^3`

Nichts davon wird von der Magnetostatik-Aufgabe gelesen. Die
Elektrostatik-Aufgabe benötigt sie, und ohne Stromdichte gibt es kein
Magnetfeld.

### Schritt 3 - Strom antreiben

1. Wählen Sie die Flächenentität an einem Ende aus, öffnen Sie die
   Registerkarte `Randbedingungen` und aktivieren Sie **Electric potential**.
   Setzen Sie es auf `0` V.
2. Wählen Sie die Flächenentität am anderen Ende aus, aktivieren Sie **Electric
   potential** und setzen Sie es auf `0.01` V.

Der Strom durch den Stab beträgt

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Ein Stab ohne Potentialdifferenz führt keinen Strom und ergibt ein Magnetfeld
von genau null - das ist das Erste, was zu prüfen ist, wenn das Ergebnis leer
ausfällt.

### Schritt 4 - lösen

1. `Lösung` -> `Löser starten` (`Ctrl+R`).
2. Verfolgen Sie den Fortschritt in `Bericht` -> `Protokolldatei des Lösers`.

Das Protokoll zeigt die Elektrostatik-Aufgabe mit ihren Variablen und dann die
Magnetostatik-Aufgabe: die Anzahl der stromführenden Elemente, die Anzahl der
Knoten und die Statistik des Magnetfelds. Beide laufen einmal.

### Schritt 5 - Ergebnis untersuchen

- Registerkarte `Ergebnisse`: Zeigen Sie **Magnetic field** an. Es ist ein
  Knotenvektor, daher bietet die 3D-Ansicht seinen Betrag und seine drei
  Komponenten an.
- Das Feld **umkreist den Stab**, senkrecht zum Strom, im Sinne der
  Rechte-Hand-Regel.
- Es ist **null auf der Achse** des Stabs und wächst zur Oberfläche hin, wo es
  am größten ist - in der Mitte jeder Längsseite statt an den Ecken.
- Es ist **in der Nähe der Enden** des Stabs **schwächer** als in der Mitte. Das
  ist der offene Strompfad aus 1.4: In der Nähe eines Endes fehlt die Hälfte
  dessen, was ein längerer Leiter wäre.

### Schritt 6 - Betrag prüfen

Für einen runden Leiter mit Radius `R` ist das Oberflächenfeld
`u0*I/(2*pi*R)`. Ein quadratischer Stab mit Seitenlänge `a` ist nicht rund,
aber die Mitte einer Seite im Abstand `a/2` von der Achse kommt dem nahe: mit
`a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

Das berechnete Feld in der Mitte einer Längsseite, auf halber Länge des Stabs,
beträgt etwa `0.020 T`, rund 15 % weniger - der quadratische Querschnitt legt
einen Großteil des Stroms weiter vom Punkt entfernt als ein runder. An einer
Ecke, `a/sqrt(2)` von der Achse entfernt, ist es nochmals niedriger, etwa
`0.019 T`.

Eine schärfere Prüfung benötigt einen Feldpunkt abseits des Stabs, wo der
Querschnitt keine Rolle mehr spielt. Fügen Sie wie in 2.8 beschrieben einen
Luftbereich hinzu und lesen Sie das Feld bei `r = 0.02 m` von der Achse auf
halber Länge ab. Für einen geraden endlichen Leiter der Länge `L` gilt

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

und der berechnete Wert sollte auf etwa `0.1 %` übereinstimmen, wobei die
Abweichung vom quadratischen Querschnitt des Stabs herrührt. Dieser Wert ist das
Feld des Stabs allein, wie 1.4 erklärt - ein realer Stromkreis würde das Feld
seiner Zuleitungen hinzufügen.

### Fehlerbehebung

| Symptom | Ursache |
|---|---|
| Das Magnetfeld ist überall null, und das Protokoll warnt, dass kein Element Strom führt | keine Stromdichte - prüfen Sie, ob die Elektrostatik-Aufgabe zuerst gelaufen ist, ob sich die Potentiale unterscheiden und ob das Material eine elektrische Leitfähigkeit hat |
| **Magneto-statics** lässt sich nicht allein deaktivieren | erwartet - es erfordert *Electro-statics*, und deren Deaktivierung deaktiviert beide |
| Das Gruppenfeld *Zeitlöser* fehlt | erwartet - keiner der Problemtypen hat einen Zeitlöser, siehe 1.10 |
| Das Feld ist an den Enden eines Leiters schwächer | erwartet - der Strompfad ist offen, siehe 1.4 |
| Das Feld fern vom Leiter stimmt nicht mit dem Stromkreis überein | die Zuleitungen und der Rest des Stromkreises sind nicht im Modell, siehe 1.4 |
| Das Feld an einem Linien- oder dünnen Flächenleiter ist unregelmäßig | erwartet - das Feld eines Leiters mit Querschnitt null ist dort singulär, siehe 1.3 |
| Der Lauf dauert lange | der Aufwand ist Knoten mal stromführende Elemente, siehe 1.7 |

---

## 4. Überprüfung eines Modells

**Gerader Leiter.** Für einen langen Leiter mit Radius `R`, der einen
Gesamtstrom `I` gleichmäßig führt, ist das Feld umlaufend und

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(`outside` = außen, `inside` = innen) mit `u0 = 1.2566e-6`. Das Feld ist an der
Oberfläche des Leiters am größten, `B = u0*I/(2*pi*R)`, und fällt außerhalb mit
`1/r` ab. Für einen Strom von `1 A` in einem Leiter mit `5 mm` Radius beträgt
diese Spitze `4e-5 T`, etwa die Größe des Erdmagnetfelds.

**Endlicher Leiter.** Für einen geraden Leiter der Länge `L`, im Abstand `r` von
seiner Achse und auf halber Länge,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

was mit wachsendem `L` gegen die Formel für den langen Leiter strebt. Abseits
des Leiters spielt die Form des Querschnitts keine Rolle mehr, daher kann jeder
Querschnitt damit geprüft werden.

**Draht.** Eine Linienentität mit Querschnittsfläche führt ihren Strom entlang
der Segmente, und der Löser verwendet für jedes davon die geschlossene Form
eines geraden Drahts. Eine gerade Linienentität reproduziert die Formel für den
endlichen Leiter daher in jedem Abstand exakt, bis auf Rundung.

**Spule.** Für eine lange Spule mit `n` Windungen pro Meter, die `I` führt, ist
das Feld im Inneren homogen und entlang der Achse gerichtet,

```
B = u0 * n * I
```

und außen nahezu null. Eine Wicklung kann als Linienentität modelliert werden,
die den Windungen folgt, mit der Querschnittsfläche des Drahts, und die
Elektrostatik-Aufgabe treibt den Strom über Potentiale an ihren beiden Enden
hindurch.

**Divergenz.** Für jedes Magnetfeld gilt überall `div(B) = 0`. Das
Biot-Savart-Gesetz erfüllt dies exakt, daher testet es diesen Löser nicht; es
würde nur die Interpolation zwischen den Knoten testen.

**Konvergenz mit dem Netz.** Die Integration selbst ist exakt (1.3); vom Netz
abhängig ist die Stromdichte, die die Elektrostatik-Aufgabe pro Element konstant
berechnet. Das Verfeinern des Netzes im Leiter und der Vergleich des Felds am
selben Punkt ist die Prüfung dieses Teils.

---

## 5. Einschränkungen

### Einschränkungen der Modellierung

- **Nur freier Raum.** In die Formulierung geht keine Permeabilität ein. Es gibt
  keine Materialeigenschaft relative Permeabilität, keine Unterscheidung
  zwischen `B` und `H`, keine Magnetisierung und keine Sättigung, daher können
  Eisen, Ferrite und Abschirmungen überhaupt nicht abgebildet werden. Siehe 1.9,
  warum dies einen anderen Löser erfordert.
- **Nur das Feld des modellierten Stroms.** Das Feld der Zuleitungen und des
  übrigen Stromkreises ist nicht enthalten. Ist der Strompfad im Modell offen,
  ist das Feld fern vom Leiter nicht das eines realen Stromkreises - siehe 1.4.
- **Keine Permanentmagnete und kein eingeprägtes Feld.** Die einzige Quelle ist
  ein von einer Elektrostatik-Aufgabe berechneter Leitungsstrom. Ein Magnet, ein
  Hintergrundfeld und eine externe Anregung haben keine Darstellung.
- **Ein Strom kann nicht direkt angegeben werden.** Ein Strom entsteht durch eine
  elektrostatische Lösung auf einem leitenden Körper. Eine Spule kann als
  Linienentität modelliert werden, die der Wicklung folgt, ihr Strom wird aber
  dennoch über Potentiale eingestellt.
- **Dünne Leiter sind singulär.** Das Feld in einem Knoten auf einer
  Linienentität oder an der Kante einer Flächenentität ist ein regularisierter
  Wert, kein physikalischer - siehe 1.3.
- **Elementtypen.** Tetraeder, Dreiecke, Vierecke und zweiknotige
  Liniensegmente führen Strom; andere Elementtypen tragen nichts bei.
- **Aufwand.** Der Aufwand wächst mit der Anzahl der Knoten mal der Anzahl der
  stromführenden Elemente - siehe 1.7.
- **Keine abgeleiteten Ergebnisse.** `process()` berechnet nichts, daher ist der
  Magnetfeldvektor die gesamte Ausgabe. Es gibt keinen Fluss, keine Energie,
  keine Kraft und keinen als eigene Variable gespeicherten Feldbetrag.
- **Nichts verwendet das Ergebnis.** Kein anderer Löser liest das Magnetfeld. Es
  gibt keine Lorentzkraft auf den Leiter, keinen induzierten Strom und keine
  Rückkopplung in die elektrostatische, thermische oder strukturmechanische
  Physik.
- **Nur statisch.** Es gibt keinen Zeitlöser, und das Hinzufügen einer
  Magnetostatik-Aufgabe - die eine Elektrostatik-Aufgabe mit sich bringt -
  entfernt den Zeitlöser aus dem gesamten Aufgabenablauf, sodass jede dahinter
  verkettete andere Physik ebenfalls stationär ist.
