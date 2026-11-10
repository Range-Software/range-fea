# Anfangsbedingung

Eine **Anfangsbedingung** ist eine Bedingung, die an jeder Modellentität (Punkt, Linie, Fläche, Volumen) vorgegeben werden kann, und kann als Ausgangspunkt für den Löser betrachtet werden. Bei **stationären Problemen** verbessert eine gut gewählte **Anfangsbedingung** die Konvergenz des Lösers. Bei **transienten** (zeitabhängigen) Problemen legt sie den Zustand zur Startzeit fest.

Anfangsbedingungen sind **problemspezifisch**: Jeder **Problemtyp** definiert seinen eigenen Satz von **Anfangsbedingungen** (z. B. **Temperature** für **Heat transfer**, **Velocity** und **Pressure** für **Incompressible viscous flow** oder **Displacement** für **Stress analysis**).

## Anfangsbedingung zuweisen

1. Wählen Sie eine oder mehrere Entitäten im **Modellbaum** aus.
2. Öffnen Sie die Registerkarte **Anfangsbedingungen** auf der rechten Seite des Hauptfensters.
3. Aktivieren Sie das Kontrollkästchen vor dem Namen der Anfangsbedingung, um sie zuzuweisen. Deaktivieren Sie es, um die Anfangsbedingung von den ausgewählten Entitäten zu entfernen.
4. Wählen Sie die Anfangsbedingung aus, um ihre Eigenschaftswerte in der Liste darunter zu bearbeiten.

![Anfangsbedingungen](image-Initial_conditions.png)

Anfangsbedingungen sind optional. Ist für eine Modellentität keine **Anfangsbedingung** vorgegeben, wird die **Umgebungsbedingung** verwendet. An Entitäten mit einer **expliziten Randbedingung** wird der von der Randbedingung vorgeschriebene Wert verwendet.
