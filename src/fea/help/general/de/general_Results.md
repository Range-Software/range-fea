# Ergebnisse

**Ergebnisse** werden vom **Löser** erzeugt und bestehen aus **Ergebnisvariablen**.

## Ergebnisvariable

Eine **Ergebnisvariable** ist eine Menge von Werten, die vom **Löser** berechnet wurden. Eine **Ergebnisvariable** wird durch ihren **Typ** (z. B. **Temperatur** oder **Verschiebung**) und einen Vektor von Werten bestimmt. Je nach **Typ** kann eine **Ergebnisvariable** einen Vektor skalarer Werte oder vektorieller Werte enthalten.


Die Ergebnisvariable **Temperatur** enthält beispielsweise einen Vektor von Temperaturen für jeden Knoten, während **Verschiebung** einen Vektor von Verschiebungsvektoren für jeden Knoten enthält.


Eine **Ergebnisvariable** kann entweder auf Knotenebene oder auf Elementebene berechnet werden. Beispielsweise wird **Verschiebung** auf Knotenebene berechnet, **Spannung** dagegen auf Elementebene. Jede auf Elementebene berechnete Variable kann in die Knotenebene umgewandelt werden. Die Umwandlung einer Variable von der Element- auf die Knotenebene ist ein einmaliger Vorgang und kann nicht rückgängig gemacht werden; dazu müssen die Ergebnisse vom **Löser** neu berechnet werden.
