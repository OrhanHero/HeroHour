# HeroHour-Dokumentation

Aktueller Stand: **17. Oktober 2026**. Das Konzeptziel ist noch nicht vollständig erreicht. Neueste verbindliche Richtung: Zero-Hour-Anordnung plus Tempest-orientierte Gestaltung.

## Zuerst lesen

- [Aktuelle Übergabe und lokale Arbeitsstände](CURRENT_HANDOVER.md)
- [Verbindlicher Entwicklungsplan M1–M4](PROJECT_ROADMAP.md)
- [Gestaltungsentscheidung und Konzeptgrundlage](DESIGN_DIRECTION_2026-10-17.md)
- [Tatsächlich geprüftes Windows-Paket](VERIFIED_BUILD.md)
- [Starten und bauen](STARTEN.md)

## UI

- [Bereits implementiertes Zero-Hour-Canvas-HUD](research/ZeroHour-Tempest-HUD-Implementation.md)
- [Konkrete UMG-Migrationsanleitung und C++-Vorlagen](UI_REFACTOR_IMPLEMENTATION.md) – noch nicht eingebaut
- [Neueste Paketprüfungen: 19 Suiten / 917 Assertions](verification/2026-10-17/zero-hour-tempest-ui-package/Transformation-Checks.json)
- [Paketintegrität](verification/2026-10-17/zero-hour-tempest-ui-package/Package-Integrity.json)
- [Vier zusätzliche Editor-UI-Läufe](verification/2026-10-17/zero-hour-tempest-ui-package/Editor-UI-Checks.json)
- [Historische Tempest-Rechtssidebar](TEMPEST_UI.md) – durch die neue Layoutentscheidung abgelöst

## Originalkarten, Assets und REA

- [Auswahl, Transfer und Grenzen des Originalsektors](research/Tempest-Sector-Selection.md)
- [Transfernachweise: 16 Mesh-Typen / 38 Actors](verification/2026-10-17/sector-transfer/Transfer-Checks.json)
- [Originalpositionen des ausgewählten Abschnitts](verification/2026-10-17/sector-transfer/Original-Actor-Selection.json)
- [Containertransfer und Engine-Startfehler](research/Tempest-Container-Transfer-2026-10-17.md)
- [Tatsächlicher Container-Materialgraph](research/Tempest-Container-Material-Graph.md)
- [Offline-Kartenanalyse](research/TempestMap-Offline-Analysis-2026-10-17.md)
- [REA-Arbeitsweise](research/REA_WORKFLOW.md)
- [Gameplay-Transformationsplan und historische Systemarbeit](TEMPEST_TRANSFORMATION_PLAN.md)

## Quellen und lokale Dateien

Die Root-README zeigt den aktuellen spielbaren Stand. Zeitgestempelte Prüfberichte beziehen sich ausschließlich auf ihr eigenes Paket. Ältere Arbeitsprotokolle bleiben erhalten; sie überschreiben keine neueren Designentscheidungen.

`Saved/Tests/TempestTransferProbe58` und `Saved/Research/TempestSectorTransfer` sind lokale Prüfbereiche. Sie enthalten Originalexporte und lassen sich nicht allein aus den Git-Metadaten wiederherstellen. Binärpakete unter `Builds/Windows`, installierte Engines und die Original-Installation sind ebenfalls lokal. Im Repository liegen Skripte, Quellcode, Dokumentation, Metadaten und Prüfsummen; ein neuer Account muss die lokalen Ordner erhalten oder die Untersuchungen mit der vorhandenen ModKit-Installation reproduzieren.

Pausierte V11-Kandidaten, fremde DOCX-Dateien und nicht zugeordnete Workflowänderungen sind kein Bestandteil der aktuellen freigegebenen Integration. Sie bei einer Übergabe bewahren und getrennt prüfen.
