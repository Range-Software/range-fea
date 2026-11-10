# Umgebungsbedingung

Eine **Umgebungsbedingung** ist eine Bedingung, die an jeder Modellentität wie Punkt, Linie, Fläche oder Volumen vorgegeben werden kann. Eine **Umgebungsbedingung** legt **problemspezifische** Werte für die jeweilige Umgebung fest (z. B. **Temperature** – Umgebungstemperatur oder **Gravitational acceleration** – Erdbeschleunigung).

Die verfügbaren Umgebungsbedingungen hängen von den ausgewählten **Problemtypen** ab:

- **Gravitational acceleration** – Komponenten in X-, Y- und Z-Richtung, standardmäßig 0, 0 und -9.80665 `[m/s^2]`.
- **Temperature** – standardmäßig 293.15 `[K]`.

## Umgebungsbedingung bearbeiten

Anders als **Randbedingungen** werden Umgebungsbedingungen nicht angehakt. Sie sind immer vorhanden und haben Standardwerte.

1. Wählen Sie eine oder mehrere Entitäten im **Modellbaum** aus.
2. Öffnen Sie die Registerkarte **Umgebungsbedingungen** auf der rechten Seite des Hauptfensters.
3. Wählen Sie die Umgebungsbedingung aus und bearbeiten Sie ihre Eigenschaftswerte in der Liste darunter.

![Umgebungsbedingungen](image-Environment_conditions.png)

Ist für eine Modellentität keine **Anfangsbedingung** vorgegeben, wird die **Umgebungsbedingung** verwendet.
