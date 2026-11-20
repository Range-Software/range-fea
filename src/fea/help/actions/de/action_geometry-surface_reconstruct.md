# Fläche rekonstruieren

Diese Aktion trianguliert die ausgewählten Flächen neu. Sie führt koplanare Elemente zusammen, entfernt Knoten, die die Form nicht bestimmen (Knoten innerhalb ebener Bereiche und auf geraden Kanten), und entfernt sehr kleine und schmale Elemente.

Bestehende Knoten werden nie verschoben, neue Knoten werden nur durch die Option Qualitätsnetz erzeugt. Knoten und Kanten, die mit anderen Entitäten geteilt werden (Punkte, Linien, nicht ausgewählte Flächen), bleiben erhalten, geschlossene Flächen bleiben geschlossen und es entstehen keine neuen Selbstdurchdringungen. Es werden nur lineare Dreiecke verarbeitet und das Volumennetz muss zuerst entfernt werden.

* **Winkeltoleranz** - größter Winkel zwischen den Normalen koplanarer Elemente und zwischen den Richtungen kollinearer Kanten.
* **Abstandstoleranz** - größter Abstand eines Knotens von einer Ebene (Linie), damit er als koplanar (kollinear) gilt.
* **Minimale Kantenlänge** - kürzere Kanten werden zusammengezogen. Details, die kleiner als dieser Wert sind, können entfernt werden.
* **Splitterwinkel** - Elemente mit dem kleinsten Winkel unter diesem Wert gelten als Splitter.
* **Delaunay-Triangulierung** - abschließend wird die Fläche Delaunay-trianguliert: Kanten, deren gegenüberliegende Winkel zusammen mehr als 180 Grad ergeben, werden geklappt, sofern die Form innerhalb der Abstandstoleranz erhalten bleibt.
* **Qualitätsnetz (Knoten einfügen)** - fügt neue Knoten ein (Delaunay-Verfeinerung), sodass der kleinste Winkel der Elemente mindestens den **Qualitätswinkel** erreicht und Kanten nicht länger als die **Maximale Kantenlänge** sind (null bedeutet keine Begrenzung). Neue Knoten werden auf der bestehenden Fläche und auf geraden Kanten platziert, sodass die Form erhalten bleibt. Kanten, die mit nicht ausgewählten Entitäten geteilt werden, werden nicht geteilt. Gekrümmte Flächen aus einzeln geneigten Facetten lassen sich nur teilweise verbessern.

Die Standardtoleranzen beziehen sich auf die Größe der ausgewählten Flächen. Größere Toleranzen führen zu stärker vereinfachten Flächen.
