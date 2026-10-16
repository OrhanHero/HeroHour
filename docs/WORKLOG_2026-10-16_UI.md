# UI-Priorität am 16. Oktober 2026

**Finale Paketabnahme bestanden:** 19 erforderliche Suiten, 911 Einzelprüfungen, 46 natürliche Match-/Logistikrunden. Vier Fraktions-/Auflösungskombinationen bestehen jeweils 73 UI-Prüfungen; die beiden zusätzlichen 1280-Läufe und 14 FieldVisual-Prüfungen sind separat gezählt. Sieben diagnostische Paketbilder wurden visuell geprüft. [Nachweise](verification/2026-10-16/tempest-ui-package/Transformation-Checks.json), [Aufnahmen](verification/2026-10-16/tempest-ui-package/Captures.json).

Der jüngste Auftrag lautet, zuerst das vollständige UI-Design umzusetzen. Die tatsächliche gespeicherte Tempest-Aufnahme ist für die Anordnung maßgeblich; die frühere Konzeptanordnung mit Radar unten links und fünf Produktionsspalten wird damit ersetzt. REA steht für Reverse Engineer Anything. Originale Tempest-Texturen, Porträts und Widgets wurden nicht importiert.

## Umsetzung

- Radar oben rechts, Ressourcenzeile daneben, zentraler Timer, hohe Sidebar mit drei Produktionsspalten, Bauen/Doktrinen und vertikalen Kategorien. Die nicht implementierte Luftwaffe ist sichtbar deaktiviert.
- Eigene geschichtete Stahlrahmen und Zustandsfarben; unverzerrte, anhand des tatsächlichen Alpha-Inhalts beschnittene Porträts. Preise, Kategorien, Ereignisse und Tooltips wurden anhand echter 1280- und 1600-Aufnahmen nachkorrigiert.
- Zehn bedienbare Kontrollgruppen, echte Mehrfachauswahl-Porträts und zuordenbare Produzentenqueues. Vorhandene Kosten, Erstattungen, Cooldowns, Radarabschaltung und Fähigkeitsbefehle bleiben an die Spielsysteme gebunden.
- Mausbedienbare Pause- und Ergebnisfenster. Modale Zustände konsumieren Klicks; eine unterbrochene Welt-Auswahlziehung wird beendet, ohne die aktuelle Auswahl zu löschen.
- Sieben ausdrücklich diagnostische Aufnahmezustände: Radar, LowPower, Multi, Production, Tooltip, Pause und Victory. Pauseaufnahmen verwenden den Echtzeit-Ticker für Aufnahme/Prozessende, damit die pausierte Welt den Helfer nicht blockiert.

## Rollen und Review

Hauptagent: Integration, Build, Paketierung, Tests und Quellkontrolle. REA-Agent: belegte Referenzanordnung, Porträtgrenzen, Bildprüfung und Nachweisrekorder. UI-Agent: Interaktionen, Kontrollgruppen, Tests und Dokumentation. Review-Agent: Rahmenhilfe, Quelltextprüfung und Übergabe. Es waren höchstens vier Agenten gleichzeitig aktiv. Wohnhaus-/V11-Arbeit wurde zugunsten der UI pausiert.

## Während der Abnahme behobene Fehler

1. Compilerfehler durch private Include-Pfade und unerreichbaren früheren Rahmen-Code wurden behoben.
2. Die erste Paket-Architekturprüfung erwartete noch das alte Fabrikmesh. Die exakte Referenz wurde auf die wirklich integrierte V10-Fabrik aktualisiert; keine Klassen-, Kosten- oder Assetprüfung wurde entfernt.
3. Ein Tactical-Test deckte eine instabile Float-Berechnung an der Unterdrückungsschwelle auf. Geschützte Double-Arithmetik behebt den Laufzeitfehler; ursprüngliche exakte Assertions bleiben bestehen, vier Grenztests ergänzen sie. [Präzisionsnachweis](research/Suppression-Threshold-Precision.md).
4. Der abschließende UI-Review fand den unterbrochenen Auswahl-Drag während einer Pause. Laufzeitkorrektur und ein zusätzlicher realer Modaltest wurden vor der finalen Paketabnahme integriert.

Nach relevanten Änderungen wurde neu paketiert. Frühere erfolgreiche Zwischenläufe sind keine Nachweise des letzten Pakets. Der finale Rekorder akzeptiert nur frische Logs desselben Pakets und sieben zugeordnete Paketbilder. [Aktueller UI-Stand](TEMPEST_UI.md), [Bildprüfung](research/TempestUI-VisualQA.md), [Übergabe](CURRENT_HANDOVER.md).

## Grenzen

Die Oberfläche adaptiert die beobachtete Referenz für HeroHours vorhandenen Roster; pixelgenaue Tempest-Parität und eine vollständige Spielumsetzung sind nicht erreicht. Große Auswahlen zeigen neun Mitgliedsporträts, die aggregierte Queue fünf Aufträge; ausgewählte Produzenten zeigen ihre vollständige maximal fünfteilige Queue. Alle ausgewählten Einheiten erhalten weiterhin Gruppenbefehle. Wohngebiete, weitere Modelle und Weltqualität bleiben Folgearbeit. V10-Fabrik und Ressourcenfeld-Darstellung sind mitintegrierte Laufzeitabhängigkeiten; die vorbereiteten Live-Timer-Feldtests sind noch nicht an einen Testaufruf angeschlossen.
