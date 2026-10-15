# Aktuelle Übergabe – 15.10.2026

## Neuester Stand: Konzeptdurchlauf V6, 15.10.2026

**Das Spiel ist weiterhin in Entwicklung und nicht vollständig konzeptgerecht fertiggestellt.** Drei Fachagenten arbeiteten in mehreren parallelen Arbeitswellen; der Hauptagent integrierte und prüfte. Technisches Maximum: vier aktive Agenten einschließlich Hauptagent.

Neu: sechs Canyon-/Ufermeshes mit verbesserten Normalen; vier detaillierte Stadtgebäude an 28 Positionen; drei überarbeitete Militärgebäude mit sechs aktualisierten Porträts; drei zusammengefasste Straßenfinish-Meshes; ruhigere UI und lesbare Kampfwerte. **999 gekippte eigene Kartenobjekte repariert**, darunter alle 64 Wohnhäuser. Die fünf betroffenen alten Python-Importskripte verwenden jetzt ausdrücklich benannte Rotationsparameter. [Umsetzung, Verfahren und Grenzen](CONCEPT_V6.md).

Aktuelles Windows-Paket: **19:35 Berlin**, `Packaging-20261015-193450-794.log`, CreatedUtc `2026-10-15T17:35:27.7561703Z`. **19 Paketprüfungen PASS: 512 Aegis- und 139 Ember-Prüfpunkte, zusammen 651; dazu 46 natürliche Match-/Logistikpartien.** Präsentation umfasst jetzt 100 tatsächliche Asset-/Szenenprüfungen je Lauf. [Aktuelle Nachweise](verification/2026-10-15/concept-v6/Transformation-Checks.json), [Paketaufnahmen](verification/2026-10-15/concept-v6/Captures.json).

Kurze Einzelinstanz-Messung bei 1600×900, Startkamera, ohne Gegnerbot, 60-FPS-Grenze: **55,39 FPS Mittel / P95 29,25 ms**, 1.500 ausgewertete Frames nach 300 Warmup-Frames. Die V5-Vergleichsaufnahme lag bei rund 60 FPS. Das ist keine Leistungsverbesserung und keine neue 20-Minuten-Abnahme. Weitere Profilierung bleibt offen. Die früheren Langzeitberichte gehören zu ihren damaligen Builds.

Unentdeckte Gebäude werden weiterhin normal vom Nebel abgedunkelt. Ein vorgeschlagenes zusätzliches Ausblenden wurde nicht eingebaut: Es würde die vorhandenen Silhouetten/Schatten verändern, und der Nutzen ist noch nicht nachgewiesen. ArtReview-Aufnahmen sind ausdrücklich getrennt vom normalen Spielmodus.

Der Nutzer autorisiert den vollständigen Tempest-orientierten **Prototypumfang M1–M4**, selbstständige Arbeit und notwendige Agenten. Keine erneute Freigabe für bereits beauftragte Arbeit verlangen. Dokumentieren und ausschließlich als **OrhanHero** committen.

## Historischer Presentation-V5-Build 15.10.2026, 18:09 Berlin

20 eigene Fraktionsmaterialien, 22 transparente Blender-Porträts (512×512), sechs Detailmeshes und 280 gespeicherte kollisionsfreie Dekorationen sind integriert. [Assetverfahren und Grenzen](PRESENTATION_V5.md).

**19 Paketprüfungen PASS: 488 Aegis- und 115 Ember-Prüfpunkte, zusammen 603, dazu 46 natürliche Match-/Logistikpartien.** [Aktuelle Abnahme](verification/2026-10-15/presentation-v5/Transformation-Checks.json), [neue Paketaufnahmen](verification/2026-10-15/presentation-v5/Captures.json). **Beide neuen 20-Minuten-Grafikläufe bestanden:** Aegis 1.200,035 Sekunden / 7 vollständige Gefechte, Ember 1.200,016 Sekunden / 8. [Aktuelle Abschlussberichte](verification/2026-10-15/presentation-v5/Soak-Acceptance.json). Mittlere Bildraten 35,859 und 35,776 FPS unter gemeinsamer GPU-Last; neben den beiden Offscreen-Läufen liefen anfangs Paket-/Aufnahmeprüfungen und später ein zusätzliches natives Vollbildspiel. P95 86,774 / 85,806 ms; schlechteste Frames 648,469 / 761,464 ms. Stabilitätsprüfung bestanden, **keine 60-FPS-Leistungsabnahme**. Ein isolierter Profilinglauf bleibt erforderlich.

Paket: `Packaging-20261015-180917-692.log`, CreatedUtc `2026-10-15T16:09:49.5080994Z`. EXE-SHA-256: `54292950892f8e02c55fc615356a9286e42eee52adbdfd3bffaa404c6f08ffc7`.

Korrekturen: Dock-Wartewinkel aus räumlicher Ankunft statt UObject-ID; dauerhaft gespeichertes `NoCollision`-Profil; im Editor ohne Rendering Prüfung gültiger importierter Porträtquellen statt nicht erzeugter Plattformdaten. Paketprüfungen verlangen weiterhin 512×512-Laufzeittexturen. Beide Seiten starten jetzt mit gleichen Rollen und 800 Credits, ohne kostenlose Gegnerfabrik (14 Startobjekte). Der unveränderte natürliche Matchtest besteht für beide Fraktionen; versuchte zusätzliche defensive Teststrategien wurden entfernt. Das ist keine statistische Balanceabnahme. Ember-Overdrive-Zielwahl bleibt ein offener Botpunkt.

## Historischer vorheriger Abnahmestand

- Checkout `C:\Users\Hero\HeroHour`, Branch `codex/prototype-bootstrap`, Draft [PR #8](https://github.com/OrhanHero/HeroHour/pull/8). `E:\HeroHour` ist der alte Projekt-/Konzeptordner.
- Aktiv sind `HeroHourPrototype`, Unreal **5.8.3** unter `F:\UE\UE_5.8` und `/Game/Maps/CityHero`. Das alte F#-/Lockstep-Konzept ist kein aktiver Build. [Historische Vision](archive/ORIGINAL_PROJECT_VISION.md).
- Letztes Windows-Paket: **15.10., 13:45 Berlin**, `Packaging-20261015-134515-393.log`, CreatedUtc `2026-10-15T11:45:50.6583192Z`.
- **19 Paket-Testläufe PASS:** alle 14 Suiten als Aegis, zusätzlich FullMatch/Match/UI/Presentation/LogisticsStress als Ember. **446 + 73 Assertion-Prüfungen**, außerdem 46 natürliche Partien; davon 40 Logistik-Stressrunden mit 60-Hz-Simulationsschritten. Exakte Logs und EXE-Hash: [Transformation-Checks.json](verification/2026-10-15/Transformation-Checks.json).
- **Beide 20-Minuten-RHI-Dauerläufe bestanden:** Aegis 1.200,009 Sekunden / 7 vollständige Gefechte, Ember 1.200,005 Sekunden / 8. [Abschlussberichte](verification/2026-10-15/Soak-Acceptance.json).

Code-/Asset-Commit **be57d0b** ist als OrhanHero auf GitHub. Abschließende Berichte sind unter docs/verification/2026-10-15 versioniert. Eine kurze native Bedienprüfung bestätigte Pause/Fortsetzen, HQ-Auswahl, Sidebar-Radarbau, aktive Minimap, natürliche Niederlage und Neustart. Weitergehende manuelle Balance-/Micro-Partien bleiben Qualitätsarbeit.

## Was tatsächlich integriert ist

Zwei Fraktionen, vier mobile Rollen und sieben Gebäude je Fraktion. 25 neue DataV4-Assets plus aktives DataV3-Feldasset. 48 räumliche Feldzellen; Regrowth standardmäßig, exklusive Zell- und Dockreservierung, 40/s Entladung mit Cargo-/Wallet-Erhalt. Aktive Pause und manuell abgeschaltete Raffinerie sind jetzt ausdrücklich geprüft.

Fünf funktionale Sidebar-Kategorien, bezahlte FIFO je Produzent, getrennte Bau-/Abwehr-/Kraftwerks-Baulanes, Rechteckraster/Rotation, verbundener Bauradius, Verkauf und manuelles Abschalten. Strombedarf bleibt bei Low Power bestehen; Rate 1/0,5/0. Kraftwerkswiederaufbau funktioniert auch bei vollständig ausgefallenem Netz. Radar und Abwehr benötigen volle Versorgung.

Getrennte Sicht/Erkundung, Weltnebel, Radar-Minimap, sichtgefilterte Gegneranzeigen und Angriffe. Drei Schadensarten, Rüstungsmatrix, drei Veteranenstufen, Deckung/Unterdrückung, Granate/Rauch/Deployment und vier fraktionsgebundene Doktrinen. Bot baut und kämpft über dieselben öffentlichen Befehle.

Neue eigene Modelle für Kraftwerk, Kaserne, Radar, Abwehr, Infanterie und Belagerungsfahrzeug ergänzen HQ/Fabrik/Raffinerie/Striker/Sammler. Echtes Infanterie-Rig mit Idle/Walk/Fire; Knochenbewegung und Materialzuordnung aller elf Laufzeitrollen geprüft. Eigenes Audio und Schuss-/Rauch-/Treffereffekte. [Assetverfahren](FRONTLINES_V4.md), [Abnahme und behobene Fehler](research/Transformation-Acceptance-2026-10-15.md).

## Start und Entwicklung

Aegis `HeroHour-Starten.cmd`, Ember `HeroHour-Ember-Starten.cmd`. [Start/Build](STARTEN.md), [Steuerung](PROTOTYP_START.md), [Plan](TEMPEST_TRANSFORMATION_PLAN.md), [heutiges Arbeitsprotokoll](WORKLOG_2026-10-15.md). Nach Clone zuerst `git lfs pull`; Paket und rohe Logs unter Builds/Saved sind lokal und ignored.

Git-Identität: OrhanHero `<189524118+OrhanHero@users.noreply.github.com>`; vor Commit Autor und Committer mit `git var` prüfen. Unbearbeitete `.github/workflows/` und fünf Word-Dokumente bleiben untracked. AndroidFileServer kann beim Editorstart einen lokalen Token an `DefaultEngine.ini` anhängen: vor Git den gesamten AndroidFileServer-Abschnitt entfernen, niemals ausgeben/committen.

## Grenzen und nächste Qualitätsarbeit

Der Prototyp enthält den vereinbarten kleinen Offline-Roster, keine vollständige kommerzielle Tempest-Rekonstruktion. Assetdetails, getrennte Fraktionsmodelle, Fahrzeuganimationen, differenzierteres Audio, Feinschliff der Balance und längere manuelle Partien folgen. Multiplayer, Kampagne und Luft-/Marine-Roster sind außerhalb dieses Umfangs. Datenassets treiben die implementierten Werte; die elf Rollen, Produzenten und drei Fähigkeitstypen bleiben eine feste C++-Registry.

REA heißt **Reverse Engineer Anything**, kein RTS-Framework. Belegt sind gezielte Original-Bytebereiche, keine neu kompilierte Vollrekonstruktion oder vollständige Laufzeitparität. Originalspiel und Konzeptdateien bleiben unverändert. [REA-Belege](research/REA_WORKFLOW.md).
