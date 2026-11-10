# Modell

Ein **Modell** ist eine virtuelle geometrische Darstellung eines dreidimensionalen Objekts. Ein Modell besteht aus geometrischen **Entitäten**.

![Modell](image-Model.png)

## Entitäten

Eine **Entität** ist eine Sammlung von Elementen (geometrischen Simplexen) gleichen Typs. Es gibt folgende Typen:

- **Punkt** – Sammlung von 0D-Elementen (Knoten).
- **Linie** – Sammlung von 1D-Elementen (Strecke).
- **Fläche** – Sammlung von 2D-Elementen (Dreieck, Viereck).
- **Volumen** – Sammlung von 3D-Elementen (Tetraeder).

Unabhängig davon, ob die Elemente 0D, 1D, 2D oder 3D sind, werden alle im 3D-Raum dargestellt. Ein Punkt (Knoten) hat zum Beispiel keine Abmessungen, er hat nur eine Position, während eine Linie eine Dimension (Länge) hat.

Neben diesen geometrischen Entitäten kann ein Modell Entitäten enthalten, die aus berechneten **Ergebnissen** erzeugt wurden: **Schnitte**, **ISOs**, **Stromlinien**, **Skalarfelder** und **Vektorfelder**.

## Modellbaum

Alle Entitäten eines Modells sind im **Modellbaum** (Registerkarte **Modell** auf der linken Seite des Hauptfensters) nach Typ gruppiert aufgeführt. Die Spalte **#** zeigt die Anzahl der Entitäten in einer Gruppe bzw. die Anzahl der Elemente in einer Entität.

![Modellbaum](image-Model_tree.png)

- Das Kontrollkästchen vor einem Namen blendet das Modell, die Gruppe oder die Entität im 3D-Bereich ein oder aus.
- Ein Klick auf eine Zeile wählt sie aus. Um mehrere Entitäten auszuwählen, halten Sie beim Auswählen die Taste _Strg_ (_⌘ Cmd_ unter macOS) gedrückt, oder die _Umschalttaste_, um einen Bereich auszuwählen. Die Auswahl eines Modells oder einer Gruppe wählt alle darin enthaltenen Entitäten aus.
- **Randbedingungen**, **Anfangsbedingungen**, **Umgebungsbedingungen**, **Material** und **Ergebnisse** werden immer den Entitäten zugewiesen, die im **Modellbaum** ausgewählt sind.

Die **Anzeigeeigenschaften** unter dem Baum bestimmen, wie die ausgewählten Entitäten gezeichnet werden (z. B. **Als Drahtgitter**, **Elementkanten**, **Elementknoten**, **Randbedingungen**), sowie deren Farbe.

## Geometrische Eigenschaften

Jeder **Entität** außer dem **Volumen** kann eine **geometrische** Eigenschaft zugewiesen werden, die ihre 3D-Darstellung vervollständigt. Sie wird für die ausgewählten Entitäten in der Liste **Geometrische Eigenschaften** unter dem **Modellbaum** angezeigt:

- **Punktvolumen** – Volumen eines **Punkts** in Kubikmetern.
- **Querschnittsfläche der Linie** – Querschnittsfläche einer **Linie** in Quadratmetern.
- **Flächendicke** – Dicke einer **Fläche** in Metern.
