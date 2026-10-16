# Aktuelle Übergabe – 16.10.2026

## Aktueller geprüfter Paketstand V8 – 16.10.2026, 14:03 Berlin

V8 ist integriert und als Windows-Paket geprüft: zwei zusammengefasste Basisbodenmeshes mit Betonfugen, Drainagen, Wartungsmarkierungen und Staubrändern; größere HUD-Schrift bei 1280×720, kurze eindeutige Fähigkeitsbeschriftungen und hinterlegte Kamerahilfe. **19 Paketläufe PASS, 775 einzelne Prüfpunkte; 46 natürliche Match-/Logistikrunden.** Weitere zwei echte UI-Läufe bei 1280×720 bestehen mit jeweils 43 Prüfungen. Die Präsentationsprüfung enthält jetzt je 141 Punkte einschließlich tatsächlicher V8-Map-Actors, Materialbindungen und deaktivierter Kollision/Navigation/Schatten. [Paketbelege](verification/2026-10-16/reference-v8/Transformation-Checks.json), [Aufnahmen](verification/2026-10-16/reference-v8/Captures.json).

TRELLIS.2 läuft lokal, aber der erste HQ-Kandidat ist nach echten Mehransichten ausdrücklich **abgelehnt**: fragmentierte Fassaden und beschädigte Satellitenschüssel. Er ersetzt kein Laufzeitmodell. Strukturprüfung PASS ist keine visuelle Freigabe. [Bildprüfung](research/TrellisHQV8-VisualQA.md), [Ursachenanalyse](research/TrellisHQV8-Diagnosis.md). Ein kontrollierter Zwischenexport-Versuch untersucht die Geometriestufen, ohne aus fehlenden Daten eine Fehlerursache zu erfinden.

**Aktive Folgearbeit, noch kein Bestandteil dieses Pakets:** originales Terrain-/Böschungskit V9; Aufteilung des eigenen Sammlers in echte bewegliche Teilmeshes und eine rein visuelle Arbeits-/Entladesequenz. Drei Fachagenten plus Root, getrennte Dateiownership. Keine neue Vollständigkeits-, Konzeptparitäts- oder Dauerleistungsbehauptung. Das Konzept bleibt besonders bei Weltfülle, übrigen Einheiten und Animationen unerfüllt.

Kurzer Wiederanlaufpunkt nach Limit-/Accountwechsel: [CONTINUE_2026-10-16.md](CONTINUE_2026-10-16.md). Gesicherter V7-Code-/Assetstand: Commit `4904c1f` auf `codex/prototype-bootstrap`, nach GitHub gepusht als OrhanHero.

Fortsetzung am Morgen, 16.10.: Weitere echte V7-UI-Läufe bei 1280×720 bestehen für beide Fraktionen ([Nachweis](verification/2026-10-16/reference-v7/UI-1280-Checks.json)). Kurze Startkamera-Messung mit NoBot/1600×900/60-FPS-Grenze: Mittel 60,00 FPS, P95 17,13 ms, 1.500 ausgewertete Frames nach 300 Warmup; kein Gefechts- oder Dauerleistungsnachweis. Der Prozess lieferte Exitcode 777003 bei laut Log normalem Benchmarking-Shutdown; der Bericht ist deshalb eine Frame-Aufzeichnung, keine zusätzliche PASS-Abnahme ([Messdaten](verification/2026-10-16/reference-v7/Performance-V7.json)). V8 arbeitet derzeit an Basisbodendetails und Schriftlesbarkeit; zusätzlich wird TRELLIS.2 lokal geprüft. Diese Änderungen sind noch nicht als neues Paket abgenommen.

## Neuester Stand: Referenzdurchlauf V7, 16.10.2026, 00:34 Berlin

**Das gesamte Spiel ist noch nicht konzeptgerecht fertig.** Vier neue eigenständige Referenzmodelle ersetzen HQ und Kampffahrzeug beider Fraktionen: weißblauer facettierter Aegis-Turm mit schrägen Panzerstützen, sandfarbener Acht-Rad-Panzer, graphitgrüner Ember-Portalbau und eigener dunkler Acht-Rad-Panzer. Vier eigene UV-Atlanten mit Albedo/ORM/Emissionsmaske und vier dazu passende Porträts sind integriert. Die übrigen neun Ember-Porträts und die Laufzeitmarkierungen des älteren Rosters folgen ebenfalls der grünen Konzeptfarbe. Die erste zu einfache Hausform wurde nach direktem Bildvergleich ersetzt. [Umsetzung und Grenzen](REFERENCE_V7.md).

HUD: kompakte Ressourcenzeile, fünf Produktionsspalten, zwei echte obere Doktrinkarten, gemessene Textbreiten und unverzerrte Porträts. UI-Prüfungen bedienen tatsächliche Karten und Geländeziele. Modellprüfungen testen reale Actor-Geometrie, Bodenauflage, Texturbindungen und Schaden/Flash-Rücksetzung. Gleichzeitige Tests erhalten jetzt eindeutige Logdateien; eine zuvor beobachtete Namenskollision ist behoben.

**Frisches Windows-Paket: 16.10.2026, 00:34 Berlin. 19 Paketläufe PASS mit 571 Aegis- und 198 Ember-Prüfpunkten, zusammen 769; dazu 46 natürliche Match-/Logistikpartien.** CreatedUtc `2026-10-15T22:34:24.1236961Z`, Paketierungslog `Packaging-20261016-003345-296.log`. [Prüfungen](verification/2026-10-16/reference-v7/Transformation-Checks.json), [sechs Paketaufnahmen](verification/2026-10-16/reference-v7/Captures.json), [EXE-/Paketprüfsummen](verification/2026-10-16/reference-v7/Package-Integrity.json). Das lokale Paket liegt unter `Builds/Windows`; es wird nicht in Git hochgeladen.

Der REA-Agent hat neue statische Tempest-Belege für UI-Farben, segmentierte Ressourcenmeshes und Sammler-/Materialmetadaten erarbeitet. Originalbytes sind für einen kleinen Funktionskörper verankert; das beweist weder Originalquelltext noch Shipping-Laufzeitverhalten oder einen vollständigen Klon. [Befunde und Anwendungen](research/Tempest-REA-V7.md). Eigene Oberflächen sind umgesetzt; Sammleranimationen und Ressourcenverbindungen bleiben weitere Arbeit.

**Offen:** Die Basisumgebung und Hintergrundstadt sind weiterhin zu leer und regelmäßig. Der übrige Roster besitzt noch geteilte Modelle. Fahrzeug-/Turmanimation, dicht belegte UVs, zusätzliche Normaldetails, individuelle Beschriftungen, Schäden und manuell gesetzte Boden-/Schmutzdetails fehlen. V7 enthält keine neue 20-Minuten-Dauerabnahme. Vorherige Leistungs- und Dauerberichte gehören zu ihren jeweiligen Builds. Drei Fachagenten und Hauptagent arbeiteten parallel, maximal vier aktive Agenten; keine Behauptung von 15 gleichzeitig laufenden Agenten.

## Historischer Konzeptdurchlauf V6, 15.10.2026

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
