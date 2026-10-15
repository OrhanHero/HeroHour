# HeroHour – Unreal RTS-Prototyp

Ein spielbarer Offline-Prototyp mit C&C-Bedienung und Tempest-orientierter Wirtschaft, Basisbau und Gefechtsmechanik. Unreal Engine **5.8.3**, aktives C++-Modul **HeroHourPrototype**, Karte **CityHero**.

**Account-Übergabe:** [CURRENT_HANDOVER](docs/CURRENT_HANDOVER.md). [Geprüftes Windows-Paket](docs/VERIFIED_BUILD.md), [Transformationsplan M1–M4](docs/TEMPEST_TRANSFORMATION_PLAN.md), [Arbeitsprotokoll](docs/WORKLOG_2026-10-15.md).

## Spielen

- Aegis: `HeroHour-Starten.cmd`.
- Ember: `HeroHour-Ember-Starten.cmd`.
- Das lokale Windows-Paket liegt unter `Builds/Windows`; nach einem Clone zuerst Git LFS laden und das Paket bauen. [Start und Build](docs/STARTEN.md), [Steuerung](docs/PROTOTYP_START.md).

## Aktiver Umfang

- Zwei asymmetrische Fraktionen: robuste Aegis mit Rauch und Reparatur/Präzisionsschlag; schnelle Ember mit Tempest-Schaden, Scorch und Overdrive.
- Sammler, Striker, Infanterie und Belagerungsfahrzeug; HQ, Kraftwerk, Raffinerie, Fahrzeugfabrik, Kaserne, Radar und Abwehr.
- Drei Ressourcenfelder mit zusammen **48 räumlichen Zellen**, Nachwachsen, exklusiven Zell- und Dockreservierungen, tatsächlicher Anfahrt und zeitlicher Entladung.
- Fünf Sidebar-Kategorien, bezahlte Produzentenwarteschlangen, Bauplatzraster mit Rotation und verbundenem Bauradius, Strommangel, manuelle Abschaltung und Verkauf.
- Getrennte Sicht und Erkundung, Weltnebel und Radar-Minimap; Gegneranzeigen, Zielsuche und Befehle berücksichtigen die Sicht.
- Drei Schadensarten, Rüstungsmatrix, Deckung, Unterdrückung, Veteranenstufen, Granaten, Rauch und Deployment; je zwei Commander-Fähigkeiten.
- Eigene Blender-Modelle, Infanterie-Rig mit Idle/Walk/Fire, Porträts, Materialien und synthetisierte Effekte/Audiodateien. [Frontlines-V4-Quellen](docs/FRONTLINES_V4.md).

## Abnahme und Grenzen

## Presentation V5 – aktueller Build 15.10.2026, 18:09 Berlin

20 eigene Fraktionsmaterialien, 22 transparente Blender-Porträts (512×512), sechs Detailmeshes und 280 gespeicherte kollisionsfreie Dekorationen sind integriert. [Assetverfahren und Grenzen](docs/PRESENTATION_V5.md).

**19 Paketprüfungen PASS: 488 Aegis- und 115 Ember-Prüfpunkte, zusammen 603, dazu 46 natürliche Match-/Logistikpartien.** [Aktuelle Abnahme](docs/verification/2026-10-15/presentation-v5/Transformation-Checks.json), [neue Paketaufnahmen](docs/verification/2026-10-15/presentation-v5/Captures.json). Beide neuen 20-Minuten-Grafikläufe laufen noch; historische Langzeitergebnisse gelten für ihren damaligen Build.

Paket: `Packaging-20261015-180917-692.log`, CreatedUtc `2026-10-15T16:09:49.5080994Z`. EXE-SHA-256: `54292950892f8e02c55fc615356a9286e42eee52adbdfd3bffaa404c6f08ffc7`.

Korrekturen: Dock-Wartewinkel aus räumlicher Ankunft statt UObject-ID; dauerhaft gespeichertes `NoCollision`-Profil; im Editor ohne Rendering Prüfung gültiger importierter Porträtquellen statt nicht erzeugter Plattformdaten. Paketprüfungen verlangen weiterhin 512×512-Laufzeittexturen. Beide Seiten starten jetzt mit gleichen Rollen und 800 Credits, ohne kostenlose Gegnerfabrik (14 Startobjekte). Der unveränderte natürliche Matchtest besteht für beide Fraktionen; versuchte zusätzliche defensive Teststrategien wurden entfernt. Das ist keine statistische Balanceabnahme. Ember-Overdrive-Zielwahl bleibt ein offener Botpunkt.

Historischer Build 13:45: **19 Paketläufe bestanden:** 14 Aegis-Suiten und fünf zusätzliche Ember-Läufe, insgesamt 519 Assertion-Prüfungen und 46 natürliche Simulationspartien einschließlich 40 Logistik-Stressrunden bei 60-Hz-Schritten. Die [maschinenlesbare Abnahme](docs/verification/2026-10-15/Transformation-Checks.json) verzeichnet ausführbare Datei, Zeit und einzelne Logs. **Beide 20-Minuten-Grafik-Dauerläufe bestanden**, mit sieben beziehungsweise acht vollständigen Gefechten. [Abschlussberichte](docs/verification/2026-10-15/Soak-Acceptance.json).

Der Umfang ist ein eigener Prototyp. Assetdetails und Balance bleiben ausbaufähig; beide Fraktionen teilen einen Teil ihrer Modelle. Multiplayer, Kampagne, Luft-/Marine-Roster und die ursprüngliche F#-/Lockstep-Vision sind nicht implementiert. Die [ursprüngliche Projektvision](docs/archive/ORIGINAL_PROJECT_VISION.md) bleibt als Archiv erhalten.

REA bedeutet **Reverse Engineer Anything** und liefert nachvollziehbare Referenzbelege, keine RTS-Laufzeitbibliothek. Original-Byteanker sind von HeroHour-Verhaltenstests getrennt; vollständige Tempest-Rising-Parität ist nicht nachgewiesen. [REA-Workflow und Quellen](docs/research/REA_WORKFLOW.md).

Aktuelle tatsächliche Paketaufnahme (1600×900):

![HeroHour – Aegis Presentation V5](Art/Previews/PresentationV5-Aegis.png)

[Bildnachweise und Grenzen](docs/verification/2026-10-15/presentation-v5/Captures.json).
