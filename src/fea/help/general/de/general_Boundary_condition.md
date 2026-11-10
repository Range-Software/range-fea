# Randbedingung

Eine **Randbedingung** ist eine am Rand vorgegebene Bedingung, wobei der Rand eine beliebige Modellentität sein kann (Punkt, Linie, Fläche, Volumen).

Randbedingungen sind:
- **Problemspezifisch** – Jeder **Problemtyp** definiert seinen eigenen Satz von **Randbedingungen**.
- **Entitätsspezifisch** – Nicht jede Randbedingung kann auf jeden Entitätstyp angewendet werden (z. B. kann **Pressure** nur auf eine **Fläche** angewendet werden).
- **Explizit** – Am Rand wird die exakte Lösung vorgeschrieben (z. B. **Temperature** für **Heat transfer**). In der Liste **Randbedingungen** fett dargestellt.
- **Implizit** – Die Lösung am Rand ergibt sich aus ihnen (z. B. **Heat** für **Heat transfer**).

## Randbedingung zuweisen

1. Wählen Sie eine oder mehrere Entitäten im **Modellbaum** aus.
2. Öffnen Sie die Registerkarte **Randbedingungen** auf der rechten Seite des Hauptfensters. Die Liste enthält nur Randbedingungen, die für die ausgewählten **Problemtypen** und die ausgewählten Entitäten gültig sind.
3. Aktivieren Sie das Kontrollkästchen vor dem Namen der Randbedingung, um sie zuzuweisen. Deaktivieren Sie es, um die Randbedingung von den ausgewählten Entitäten zu entfernen.
4. Wählen Sie die Randbedingung aus, um ihre Eigenschaftswerte in der Liste darunter zu bearbeiten.

![Randbedingungen](image-Boundary_conditions.png)

Manche Eigenschaften haben ein eigenes Kontrollkästchen. So kann zum Beispiel jede Komponente der Randbedingung **Displacement** deaktiviert werden, um die Verschiebung in dieser Richtung frei zu lassen.

Randbedingungen, die in einer durch die Geometrie gegebenen Richtung wirken (**Normal displacement**, **Roller displacement**), leiten ihre lokale Richtung aus der Entität ab. Aktivieren Sie **Eingegebene lokale Richtung verwenden**, um die Richtung von Hand einzugeben.

Um zugewiesene Randbedingungen im 3D-Bereich anzuzeigen, aktivieren Sie **Randbedingungen** in den **Anzeigeeigenschaften**.

## Zeitabhängige Werte

Der Wert jeder Eigenschaft kann zeitabhängig sein. Klappen Sie die Eigenschaft auf und klicken Sie auf **Zeitabhängige Werte bearbeiten**, um eine Tabelle von Werten für gegebene Zeiten einzugeben. Zeitabhängige Werte werden verwendet, wenn das Problem **transient** (zeitabhängig) ist, also wenn in der Registerkarte **Problem** der **Zeitlöser** aktiviert ist.
