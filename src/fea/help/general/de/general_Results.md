# Ergebnisse

**Ergebnisse** werden vom **Löser** erzeugt und bestehen aus **Ergebnisvariablen**.

## Ergebnisvariable

Eine **Ergebnisvariable** ist ein Satz von Werten, die vom **Löser** berechnet wurden. Eine **Ergebnisvariable** ist durch ihren **Typ** (z. B. **Temperature** oder **Displacement**) und einen Vektor von Werten definiert. Je nach **Typ** kann eine **Ergebnisvariable** einen Vektor skalarer oder vektorieller Werte enthalten.

Die Ergebnisvariable **Temperature** enthält zum Beispiel einen Vektor der Temperaturen für jeden Knoten, während **Displacement** einen Vektor der Verschiebungsvektoren für jeden Knoten enthält.

Alle Ergebnisvariablen des ausgewählten Modells sind im Dropdown-Menü oben in der Registerkarte **Ergebnisse** zusammen mit ihrer Art (**Scalar** oder **Vector**) aufgeführt.

![Ergebnisvariablen](image-Results_variables.png)

Eine **Ergebnisvariable** kann entweder auf Knoten- oder auf Elementebene berechnet werden. **Displacement** wird zum Beispiel auf Knotenebene berechnet, **Von Mises stress** dagegen auf Elementebene. Wo die ausgewählte Variable gespeichert ist (**Auf Knoten** oder **Auf Elementen**), wird unter dem Dropdown-Menü angezeigt.

Variablen auf Elementebene werden mit einem Wert pro Element dargestellt. Für eine stetige Darstellung:

- Spannungsvariablen bieten das Kontrollkästchen **Geglättete Anzeige**. Es mittelt die Elementergebnisse an den Knoten nur für die Anzeige, die Ergebnisse selbst bleiben unverändert. Die Mittelung verringert Spitzenwerte.
- Andere Variablen auf Elementebene bieten die Schaltfläche **Auf Knoten anzeigen**, die die Variable von Elementen auf Knoten umstellt.

## Ergebnisse anzeigen

Ergebnisse werden auf den Entitäten angezeigt, die im **Modellbaum** ausgewählt sind. Ergebnisse eines auf **Volumen** gelösten Problems sind am besten zu sehen, wenn die Flächenentitäten ausgeblendet sind.

![Ergebnisse](image-Results.png)

- **Auf ausgewählten Entitäten anzeigen** – aktivieren Sie **Farbkarte**, um die ausgewählten Entitäten nach dem Variablenwert einzufärben. Vektorvariablen können auch als **Verformte Form** angezeigt werden.
- **Farbbereich** – die Werte **Max** und **Min** der Farbkarte. Sie können eingegeben oder in der **Feineinstellung** geändert werden. **Auf Daten zurücksetzen** setzt den Bereich auf den aus den Ergebnissen berechneten Bereich zurück.
- **Farben** – Farbschema der Farbkarte.
- **Legende anzeigen** – zeigt den Wertebereich im 3D-Bereich an.
- **Maßstab für Verformung und Pfeile** – multipliziert die Verformung verformter Formen und die Länge der Pfeile von Vektorfeldern (nur Vektorvariablen).
- **Diagramm…** – zeigt die Verteilung der Werte und ermöglicht das Einstellen des Farbbereichs im Diagramm.
- **Entfernen** – entfernt die Variable aus dem Modell.

Ergebnisse können auch auf dafür erstellten Entitäten angezeigt werden: **Schnitte**, **ISOs**, **Stromlinien**, **Skalarfelder** und **Vektorfelder**.
