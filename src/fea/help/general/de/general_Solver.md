# Löser

Der **Löser** ist ein Werkzeug, das die Einstellungen des **Problems** verarbeitet und auf einem gegebenen **Modell** die **Ergebnisse** berechnet.

Abhängig von der Größe des **Modells**, dem **Problemtyp**, den Konvergenzeinstellungen und den transienten Einstellungen kann die Lösung des **Problems** von Sekunden bis zu Stunden und länger dauern.

## Löser starten

**Menü:** _Lösung -> Löser starten_

Das Modell muss gespeichert sein, bevor der Löser starten kann. Wurde es noch nicht gespeichert, wird zuerst der Dialog **Modell speichern** angezeigt. Danach wird der Dialog **Löser starten** angezeigt.

![Löser starten](image-Start_solver.png)

Vor dem Start des Lösers werden die Problemeinstellungen geprüft. Gefundene **Fehler** (z. B. eine Entität ohne Material) und **Warnungen** werden im Dialog **Löser starten** aufgelistet. Enthält das Modell bereits Ergebnisse, kann **Löser neu starten / fortsetzen** aktiviert werden, um von diesen Ergebnissen aus fortzufahren, statt von vorn zu beginnen.

Der Löser läuft als eigener Prozess im Hintergrund, sodass währenddessen weitergearbeitet werden kann. Seine gesamte Ausgabe wird in der Registerkarte **Prozessausgabe** unter dem 3D-Bereich angezeigt. Sobald der Löser fertig ist, werden die berechneten **Ergebnisse** automatisch in das Modell geladen.

## Löser anhalten

- _Lösung -> Löser anhalten_ – fordert alle laufenden Löser zum Anhalten auf.
- _Lösung -> Löser abbrechen_ – beendet den Löserprozess sofort.

## Löser überwachen

- _Bericht -> Konvergenz des Lösers_ – zeigt ein Diagramm der Konvergenz des Lösers.
- _Bericht -> Überwachungspunkte_ – zeigt die Werte an den Überwachungspunkten, die mit _Problem -> Überwachungspunkte definieren_ definiert wurden.
- _Bericht -> Protokolldatei des Lösers_ – zeigt die Protokolldatei des Lösers.
