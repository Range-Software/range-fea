# Multiphysik

Dieses Tutorial zeigt, wie mehrere gekoppelte Probleme auf einem Modell gelöst werden. Es wird erneut das Modell **Beam** aus dem Tutorial **Spannungs- und Verformungsanalyse** verwendet. Diesmal fließt ein elektrischer Strom durch den Balken, der Strom erwärmt den Balken (Joulesche Wärme), und die Wärme lässt den Balken sich ausdehnen und verformen.

Es werden drei Problemtypen benötigt, die in dieser Reihenfolge gelöst werden:

1. **Electro-statics** (Elektrostatik) – berechnet das elektrische Potential, das elektrische Feld, die Stromdichte und die vom Strom erzeugte Joulesche Wärme.
2. **Heat transfer** (Wärmeübertragung) – berechnet die durch die Joulesche Wärme verursachte Temperaturverteilung.
3. **Stress analysis** (Spannungsanalyse) – berechnet die Wärmeausdehnung (Verschiebungen) und die daraus resultierende Spannung.

Jedes Problem verwendet die Ergebnisse des vorherigen, daher ist die Reihenfolge der Probleme wichtig.

## 1. Modell laden

**Menü:** _Datei -> Modell öffnen_

Wählen Sie im Dialog **Modell öffnen** die Datei **Beam.tmsh** und klicken Sie auf **Öffnen**.

Da dem Modell noch kein Problemtyp zugewiesen ist, öffnet sich nach dem Laden automatisch der Dialog **Aufgabenablauf des Problems**.

## 2. Aufgabenablauf des Problems

1. Klicken Sie im Dialog **Aufgabenablauf des Problems** auf **Problemtyp hinzufügen**.
2. Aktivieren Sie in der **Problemtyp-Auswahl** die Einträge **Electro-statics**, **Heat transfer** und **Stress analysis** und klicken Sie auf **OK**.

![Problemtyp-Auswahl – Multiphysik](image-Problem_type_selector-Multi.png)

Der **Aufgabenablauf des Problems** sollte nun die drei Probleme in einer Iterationsgruppe in der unten gezeigten Reihenfolge auflisten. Ist die Reihenfolge anders oder gibt es zusätzliche Einträge, wählen Sie einen Eintrag aus und korrigieren Sie dies mit den Schaltflächen **Nach oben**, **Nach unten** und **Entfernen**. Bestätigen Sie mit **OK**.

![Aufgabenablauf des Problems – Multiphysik](image-Problem_task_flow-Multi.png)

Der Dialog kann jederzeit über die Schaltfläche **Aufgabenablauf der Probleme** in der Werkzeugleiste erneut geöffnet werden.

## 3. 3D-Netz erzeugen

Alle drei Probleme werden auf einem Volumennetz gelöst.

**Menü:** _Geometrie -> Volumen -> Tetraedernetz erzeugen_

Die vorausgefüllten Werte im Dialog **3D-Netz erzeugen** reichen aus. Klicken Sie auf **OK**. Nach Abschluss der Vernetzung erscheint im Baum **Modell** unter **Volumen** eine neue Entität **Volume 1**.

## 4. Material zuweisen

Allen Modellentitäten muss ein Material zugewiesen sein. Das Material muss die Eigenschaften bereitstellen, die alle drei Probleme benötigen; **Iron** (Eisen) erfüllt dies.

1. Klicken Sie im Baum **Modell** auf **Surface** und dann mit gedrückter _Umschalt_-Taste auf **Volume 1**, um alle Entitäten auszuwählen.
2. Öffnen Sie die Registerkarte **Material** auf der rechten Seite des Hauptfensters.
3. Aktivieren Sie **Iron**.

## 5. Randbedingungen zuweisen

Da mehrere Problemtypen ausgewählt sind, bietet die Registerkarte **Randbedingungen** nun die Bedingungen aller dieser Probleme an, und einer Entität können mehrere Bedingungen zugewiesen werden – eine für jedes Problem. Weisen Sie den **Flächen**-Entitäten die folgenden Bedingungen zu:

1. **Surface** (die äußere Oberfläche des Balkens)
    - _Simple convection_ (einfache Konvektion)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293.15 `[K]`
2. **Load** (die Fläche in der Mitte des Balkens)
    - _Electric potential_ (elektrisches Potential)
        - Electric potential = 0.4 `[V]`
    - _Simple convection_ (Werte wie bei **Surface**)
3. **Left** und **Right** (die Flächen an beiden Enden des Balkens)
    - _Displacement_ (Verschiebung)
        - Displacement in X, Y and Z direction = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0.4 `[V]`
    - _Simple convection_ (Werte wie bei **Surface**)

Der Strom fließt also von der Mitte des Balkens zu beiden Enden, und alle Flächen geben durch Konvektion Wärme an die umgebende Luft ab.

Für jede Entität: Wählen Sie sie im Baum **Modell** aus (halten Sie _Strg_ / _⌘ Cmd_ gedrückt, um **Left** und **Right** gemeinsam auszuwählen), aktivieren Sie die Bedingung auf der Registerkarte **Randbedingungen**, wählen Sie die Bedingung aus, damit ihre Eigenschaften unter der Liste angezeigt werden, doppelklicken Sie dann auf den Wert, geben Sie den neuen ein und drücken Sie _Enter_. Die Standardwerte von **Simple convection** (100 `[W/(m^2*K)]` und 293.15 `[K]`) und von **Displacement** (0 `[m]`) stimmen bereits, daher muss nur **Electric potential** geändert werden.

## 6. Umgebungsbedingungen prüfen

Umgebungsbedingungen gelten für das gesamte Modell und werden nicht wie Randbedingungen aktiviert. Öffnen Sie die Registerkarte **Umgebungsbedingungen** und wählen Sie jede Bedingung aus, um ihre Werte zu überprüfen:

- _Gravitational acceleration_ (Erdbeschleunigung)
    - 0, 0 und -9.80665 `[m/s^2]` in X-, Y- und Z-Richtung
- _Temperature_ (Temperatur)
    - Temperature = 293.15 `[K]`

## 7. Problem lösen

**Menü:** _Lösung -> Löser starten_

Das Modell muss gespeichert sein, bevor der Löser gestartet werden kann. Wurde es noch nicht gespeichert, wird zuerst der Dialog **Modell speichern** angezeigt; bestätigen Sie mit **Speichern** (falls bereits eine Datei mit demselben Namen existiert, bestätigen Sie das Ersetzen). Danach wird der Dialog **Löser für Modell starten** angezeigt.

Der Dialog listet einige Warnungen auf, etwa **Heat transfer: No explicit boundary condition assigned.** Diese sind bei dieser Einstellung zu erwarten und können ignoriert werden. Klicken Sie auf **OK**, um den Löser zu starten.

Der Löser läuft im Hintergrund, und seine Ausgabe wird auf der Registerkarte **Prozessausgabe** angezeigt. Die Probleme werden nacheinander in der Reihenfolge des Aufgabenablaufs gelöst; der Lauf ist abgeschlossen, wenn die Ausgabe mit **SOLVER - END** endet.

## 8. Vektorfeld erstellen

Ein **Vektorfeld** ist eine Anzeigeentität, die eine Vektorvariable als Pfeile darstellt. Hier wird es verwendet, um das elektrische Feld im Inneren des Balkens zu zeigen.

**Menü:** _Geometrie -> Vektorfeld -> Vektorfeld erstellen_

Wählen Sie im Dialog **Vektorfeld-Editor** in der Liste **Modell** den Eintrag **Volume 1** und in der Liste **Variablen** den Eintrag **Electric field** (elektrisches Feld) aus und lassen Sie **3D-Vektorfeld** aktiviert.

![Vektorfeld erstellen](image-Create_vector_field.png)

Klicken Sie auf **OK**, um das Vektorfeld zu erstellen. Es wird im Baum **Modell** unter **Vektorfelder** hinzugefügt, und die berechneten Ergebnisse können darauf wie auf jede andere Modellentität angewendet werden.

## 9. Ergebnisse anwenden

Die Ergebnisse werden auf der Registerkarte **Ergebnisse** auf die im Baum **Modell** ausgewählte Entität angewendet, genauso wie im Tutorial **Spannungs- und Verformungsanalyse**.

### 9.1. Temperatur und Verformung des Volumens

1. Aktivieren Sie im Baum **Modell** die Gruppe **Volumen** (die Flächen bleiben deaktiviert) und wählen Sie dann **Volume 1** aus.
2. Wählen Sie auf der Registerkarte **Ergebnisse** in der Dropdown-Liste **Temperature - Scalar** und aktivieren Sie **Scalar** und **Wertebereich anzeigen**. Stimmt der angezeigte Bereich nicht mit den Ergebnissen überein, klicken Sie auf **Standard-Min/Max-Werte setzen**.
3. Wählen Sie in der Dropdown-Liste **Displacement - Vector [3]**, aktivieren Sie **Displacement** und setzen Sie **Maßstab** auf **20**, damit die Verformung sichtbar ist (die berechneten Verschiebungen betragen nur etwa 1.3 mm).

Die Temperatur reicht von etwa 505 K bis 620 K; der Balken ist in der Mitte seiner beiden äußeren Abschnitte am heißesten und biegt sich nach oben, da die festen Enden ihn nicht frei ausdehnen lassen.

![Angewendete Temperatur auf dem Volumen](image-Applied_temperature_on_volume.png)

### 9.2. Temperatur auf dem Vektorfeld

1. Deaktivieren Sie im Baum **Modell** die Gruppe **Volumen**, aktivieren Sie **Vektorfelder** und wählen Sie **Electric field** aus.
2. Wählen Sie auf der Registerkarte **Ergebnisse** den Eintrag **Temperature - Scalar** und aktivieren Sie **Scalar** und **Wertebereich anzeigen**. Die Pfeile werden nun nach der Temperatur an ihrer Position eingefärbt.
3. Aktivieren Sie in den **Anzeigeeigenschaften** unter dem Baum die Option **Pfeilspitzen zeichnen**, um die Richtung des elektrischen Felds anzuzeigen.

![Angewendete Temperatur auf dem Vektorfeld](image-Applied_temperature_on_vector_field.png)
