# Problem

**Problem** ist ein allgemeiner Begriff für ein oder mehrere physikalische Phänomene, die auf eine Geometrie angewendet werden. Es wird durch den **Problemtyp**, die **problemspezifischen Einstellungen**, die **Randbedingungen**, die **Anfangsbedingungen**, die **Umgebungsbedingungen** und die **Materialeinstellungen** definiert.

Ein Problem wird für jedes Modell separat definiert. Seine Teile werden in den Registerkarten auf der rechten Seite des Hauptfensters eingestellt: **Problem**, **Randbedingungen**, **Anfangsbedingungen**, **Umgebungsbedingungen** und **Material**.

## Problem auswählen

**Menü:** _Problem -> Problem auswählen_

Im Dialog **Problem auswählen** wird festgelegt, was gelöst werden soll (siehe [Problemtyp](general_Problem_type.md)). Er wird auch automatisch angezeigt, wenn ein Modell ohne Problem geladen wird. Solange kein Problem ausgewählt ist, enthält die Registerkarte **Problem** nur die Schaltfläche **Problem auswählen**.

## Problemspezifische Einstellungen

Sobald ein Problem ausgewählt ist, enthält die Registerkarte **Problem** die Einstellungen, die für die ausgewählten **Problemtypen** spezifisch sind.

![Registerkarte Problem](image-Problem_tab.png)

- **Zeitlöser** – aktivieren, um das Problem als **transient** (zeitabhängig) zu lösen. Dann können das Approximationsverfahren, die Startzeit, die Zeitschrittweite, die Anzahl der Zeitschritte und die Ausgabefrequenz eingestellt werden; die Endzeit wird daraus berechnet (_Endzeit = Startzeit + Zeitschrittweite * Anzahl der Zeitschritte_). Ist die Option nicht aktiviert, wird das Problem als **stationär** gelöst. Der Zeitlöser ist nur für Problemtypen verfügbar, die ihn unterstützen.
- Weitere Abschnitte hängen von den ausgewählten **Problemtypen** ab (z. B. **Einstellungen der transienten Spannungsanalyse**, Einstellungen für Strahlung, Akustik oder Modalanalyse).

Weitere problembezogene Aktionen finden sich im Menü **Problem**: _Matrixlöser der Probleme einrichten_, _Überwachungspunkte definieren_ und _Einstellungen zurücksetzen_.
