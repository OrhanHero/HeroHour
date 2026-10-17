# Nachgewiesene Windows-Builds

## Neuester geprüfter Stand: Zero-Hour-HUD, 17.10.2026, 02:09 Berlin

CreatedUtc `2026-10-17T00:09:58.9298367Z`, Windows Development, `/Game/Maps/CityHero`, `Packaging-20261017-020857-747.log`. **19 Pflichtläufe PASS, 917 Einzelprüfungen, 46 natürliche Match-/Logistikrunden.** [Nachweise](verification/2026-10-17/zero-hour-tempest-ui-package/Transformation-Checks.json), [EXE-/Paketintegrität](verification/2026-10-17/zero-hour-tempest-ui-package/Package-Integrity.json).

Spiel-EXE SHA-256: `f88e26718cccbae83b6860e508073e41f1a1029a79b769c3517dfc27998d43cf`. Beide paketierten UI-Läufe bei 1600×900 bestehen je 76 Prüfungen. Vier zusätzliche Editor-UI-Läufe beider Fraktionen bei 1600×900 und 1280×720 bestehen ebenfalls je 76 Prüfungen; sie sind nicht in den 917 enthalten. [Editor-Belege](verification/2026-10-17/zero-hour-tempest-ui-package/Editor-UI-Checks.json).

Neu: Radar links unten, Befehle mittig, Auswahl rechts, Ressourcen darüber. Geprüfte Paketbilder: `Art/Previews/ZeroHour-Production-1600.png` und `Art/Previews/ZeroHour-Radar-1280.png`; diagnostische Fixtures. Noch Canvas-HUD, keine UMG-Migration. Originalsektor nur im getrennten Prüfprojekt. Keine neue Leistungs-/Langzeitabnahme oder vollständige Konzeptparität. [UI-Abnahme](research/ZeroHour-Tempest-HUD-Implementation.md), [nächste Meilensteine](PROJECT_ROADMAP.md).

Die folgenden Abschnitte dokumentieren ältere Pakete; deren Messungen bleiben historisch.

## Historischer Stand: Tempest-UI-Paket, 16.10.2026, 21:36 Berlin

CreatedUtc `2026-10-16T19:36:08.6233213Z`, Windows Development, `/Game/Maps/CityHero`, `Packaging-20261016-213531-028.log`. **19 Pflichtläufe PASS, 911 Einzelprüfungen, 46 natürliche Match-/Logistikrunden.** Rundenzahl: nur Match und LogisticsStress, keine künstlichen Combat-/FullMatch-Marker. [Nachweis](verification/2026-10-16/tempest-ui-package/Transformation-Checks.json).

Zusätzlich **UI 1280×720 Aegis und Ember je 73 Prüfungen PASS** sowie **FieldVisual Aegis 14 Prüfungen PASS**. Zusatzläufe sind getrennt von den 911 erfasst. [UI-Zusatznachweis](verification/2026-10-16/tempest-ui-package/UI-1280-Checks.json), [FieldVisual-Zusatznachweis](verification/2026-10-16/tempest-ui-package/FieldVisual-Checks.json). Der finale Paketstand enthält den präzisen Suppression-Grenzwertfix und korrigierte Fabrik-Testreferenz; Pflichtsuiten wurden nach vorherigen Fehlversuchen vollständig neu ausgeführt.

Geprüfte Spiel-EXE SHA-256: `066bd29dfed0b516bac1b1338ae84f1f7957071a4d5f4154d0f1e034cd407478`. [EXE/PAK/UTOC/UCAS-Integrität](verification/2026-10-16/tempest-ui-package/Package-Integrity.json). Das Binärpaket liegt lokal unter `Builds/Windows` und wird nicht in Git hochgeladen.

Neu integriert: Tempest-orientiertes rechtes Radar und dreispaltige hohe Sidebar, Bauen/Doktrinen, fünf vertikale Kategorien, zehn Kontrollgruppen, feinere Stahlrahmen, größere Porträts, Modalrouting und geprüfter Pause-Drag-Reset mit erhaltener Auswahl. V10-Aegis-Fabrik und Ressourcenfeld-Präsentation sind enthalten. [REA-/UI-Ableitung](research/Tempest-UI-REA-Implementation.md), [Bild-QA](research/TempestUI-VisualQA.md).

[Sieben tatsächliche Paketaufnahmen](verification/2026-10-16/tempest-ui-package/Captures.json): Radar, LowPower, Mehrfachauswahl, Produktion, Pause, Sieg und Tooltip. Alle sieben sind deterministische diagnostische Fixtures, keine natürlich erspielten Zustände. Standbilder ersetzen keine Interaktions-/Timingprüfung; FieldVisual allein belegt keine komplette natürliche Regrowth-Präsentation. **Keine vollständige Konzept-/Tempest-Parität, neue Dauerabnahme oder Leistungsfreigabe.** V11-Wohnhaus bleibt pausierter Kandidat, keine 64 Map-Häuser ersetzt. Frühere Abnahmen unten bleiben historisch ihrem jeweiligen Paket zugeordnet.

Nachträge zum V7-Paket: [echte 1280×720-UI-Prüfungen](verification/2026-10-16/reference-v7/UI-1280-Checks.json), [kurze Frame-Aufzeichnung](verification/2026-10-16/reference-v7/Performance-V7.json). Die 1.500 Frames nach Warmup ergeben 60,00 FPS Mittel und P95 17,13 ms bei Startkamera/NoBot/1600×900/60-FPS-Grenze. Kein Kampflast- oder Langzeitnachweis; Profilprozess-Exitcode 777003 trotz normalem Shutdown im Log ist offengelegt. Die früheren Leistungsdaten bleiben historische Messungen ihrer jeweiligen Pakete.

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

## Historischer Presentation-V5-Build 15.10.2026, 18:09 Berlin

20 eigene Fraktionsmaterialien, 22 transparente Blender-Porträts (512×512), sechs Detailmeshes und 280 gespeicherte kollisionsfreie Dekorationen sind integriert. [Assetverfahren und Grenzen](PRESENTATION_V5.md).

**19 Paketprüfungen PASS: 488 Aegis- und 115 Ember-Prüfpunkte, zusammen 603, dazu 46 natürliche Match-/Logistikpartien.** [Aktuelle Abnahme](verification/2026-10-15/presentation-v5/Transformation-Checks.json), [neue Paketaufnahmen](verification/2026-10-15/presentation-v5/Captures.json). **Beide neuen 20-Minuten-Grafikläufe bestanden:** Aegis 1.200,035 Sekunden / 7 vollständige Gefechte, Ember 1.200,016 Sekunden / 8. [Aktuelle Abschlussberichte](verification/2026-10-15/presentation-v5/Soak-Acceptance.json). Mittlere Bildraten 35,859 und 35,776 FPS unter gemeinsamer GPU-Last; neben den beiden Offscreen-Läufen liefen anfangs Paket-/Aufnahmeprüfungen und später ein zusätzliches natives Vollbildspiel. P95 86,774 / 85,806 ms; schlechteste Frames 648,469 / 761,464 ms. Stabilitätsprüfung bestanden, **keine 60-FPS-Leistungsabnahme**. Ein isolierter Profilinglauf bleibt erforderlich.

Paket: `Packaging-20261015-180917-692.log`, CreatedUtc `2026-10-15T16:09:49.5080994Z`. EXE-SHA-256: `54292950892f8e02c55fc615356a9286e42eee52adbdfd3bffaa404c6f08ffc7`.

Korrekturen: Dock-Wartewinkel aus räumlicher Ankunft statt UObject-ID; dauerhaft gespeichertes `NoCollision`-Profil; im Editor ohne Rendering Prüfung gültiger importierter Porträtquellen statt nicht erzeugter Plattformdaten. Paketprüfungen verlangen weiterhin 512×512-Laufzeittexturen. Beide Seiten starten jetzt mit gleichen Rollen und 800 Credits, ohne kostenlose Gegnerfabrik (14 Startobjekte). Der unveränderte natürliche Matchtest besteht für beide Fraktionen; versuchte zusätzliche defensive Teststrategien wurden entfernt. Das ist keine statistische Balanceabnahme. Ember-Overdrive-Zielwahl bleibt ein offener Botpunkt.

## Historischer Build 13:45

Stand: **15.10.2026, Paket 13:45 Berlin**. Unreal Engine 5.8.3, Win64 Development, Karte /Game/Maps/CityHero. CreatedUtc: 2026-10-15T11:45:50.6583192Z. Packaging-20261015-134515-393.log: erfolgreicher Build; HERO-Assets ausdrücklich gekocht. Das Paket liegt lokal unter Builds/Windows und wird nicht als Binärbuild in Git gespeichert.

**19 Paketläufe PASS: 14 Aegis-Läufe mit 446 Assertion-Prüfpunkten und 5 Ember-Läufe mit 73 Assertion-Prüfpunkten. Zusammen 519 Assertions und 46 natürliche Partien:** je Fraktion 3 Match-Partien und 20 LogisticsStress-Partien mit 60-Hz-Simulationsschritten. Match-/Stress-Runden werden nicht zusätzlich als Assertions gezählt. Maschinenlesbarer Nachweis: [Transformation-Checks.json](verification/2026-10-15/Transformation-Checks.json).

SHA-256 der geprüften Spiel-EXE:

236b68194a924009475c9c45ced1cb73534ffa22167bd16b026bbfef40d2a6f2

## Tatsächliche Paketprüfungen

| Suite | Fraktion | Ergebnis | Log unter Saved/Logs |
|---|---|---|---|
| Combat | Aegis | 17 Prüfpunkte PASS | CombatTest-20261015-134615-941.log |
| Economy | Aegis | 102 Prüfpunkte PASS | EconomyTest-20261015-134616-073.log |
| Navigation | Aegis | 20 Prüfpunkte PASS | NavigationTest-20261015-134616-227.log |
| Match | Aegis | 3 natürliche Partien PASS | MatchTest-20261015-134616-388.log |
| UI | Aegis | 22 Prüfpunkte PASS | UITest-20261015-134616-569.log |
| Architecture | Aegis | 63 Prüfpunkte PASS | ArchitectureTest-20261015-134616-742.log |
| Resource | Aegis | 18 Prüfpunkte PASS | ResourceTest-20261015-134616-921.log |
| Docking | Aegis | 28 Prüfpunkte PASS | DockingTest-20261015-134617-130.log |
| Tactical | Aegis | 46 Prüfpunkte PASS | TacticalTest-20261015-134617-315.log |
| Visibility | Aegis | 25 Prüfpunkte PASS | VisibilityTest-20261015-134617-519.log |
| Systems | Aegis | 54 Prüfpunkte PASS | SystemsTest-20261015-134617-733.log |
| FullMatch | Aegis | 17 Prüfpunkte PASS | FullMatchTest-20261015-134617-955.log |
| Presentation | Aegis | 34 Prüfpunkte PASS | PresentationTest-20261015-134618-177.log |
| LogisticsStress | Aegis | 20 natürliche Partien PASS | LogisticsStressTest-20261015-134618-395.log |
| FullMatch | Ember | 17 Prüfpunkte PASS | FullMatchTest-20261015-134618-601.log |
| Match | Ember | 3 natürliche Partien PASS | MatchTest-20261015-134618-843.log |
| UI | Ember | 22 Prüfpunkte PASS | UITest-20261015-134619-154.log |
| Presentation | Ember | 34 Prüfpunkte PASS | PresentationTest-20261015-134619-419.log |
| LogisticsStress | Ember | 20 natürliche Partien PASS | LogisticsStressTest-20261015-134619-685.log |

Die Testwrapper verlangen PASS und prüfen Prozessabschluss sowie fehlende HERO-Assets und Materialfehler. UI benötigt ein echtes Grafik-Backend; viele numerische und manuell fortgeschriebene Gameplay-Tests verwenden NullRHI. Diese Paketläufe sind von den separaten Echtzeit-Dauerläufen zu unterscheiden.

## Enthaltener und geprüfter Funktionsumfang

- Zwei gespeicherte Fraktionsdefinitionen und elf Rollen: HQ, Raffinerie, Fabrik, Kraftwerk, Kaserne, Radar, Abwehr, Sammler, Striker, Infanterie und Artillerie. Aegis/Ember unterscheiden sich durch Tempo, Panzerung, Waffen und Fähigkeiten.
- Drei räumliche Ressourcenfelder mit insgesamt 48 Zellen; Nachwachsen ist Standard. Exklusive Zellreservierungen, echte Sammlerfahrt, getrennte Raffinerie-FIFO mit einem Dockslot und zeitliches Entladen. Eigene Rate 40/s, keine gemessene Originalrate. Cargo/Wallet sind double; gleiche darstellbare Buchungsschritte und Operations-IDs verhindern Doppelbuchung beziehungsweise Verlust kleiner Restmengen.
- Aktive Entladepause und manuelle Raffinerieabschaltung: Tests sichern Cargo, Geld, Operation und Dockzustand beziehungsweise Freigabe und korrekte Wiederaufnahme. Mehrsammler-, blockierte/zerstörte Raffinerie- und Bilanzfälle bleiben Teil der Wirtschafts-/Docking-Suiten.
- Echtes Strombudget mit voller, niedriger und fehlender Versorgung; manuelles Schalten, Radar-/Abwehrausfall und unabhängiger Kraftwerksbau zur Wiederherstellung. Bezahlter Queue-Fortschritt bleibt erhalten.
- Sidebar mit fünf Kategorien, Produzenten-FIFOs, Storno, Verkauf und Rally. Rasterplatzierung mit Rotation, verbundenem Bauradius, sofortiger Baustellenbelegung und rechteckiger Navigation. Tests decken mobile Einheiten und Ressourcen an Footprint-Ecken ab.
- Getrennte Sicht/Erkundung je Team, Welt-Fog-Material, gefilterte Gegneranzeige/Ziele und radarabhängige Minimap. Rauch beeinflusst gegnerische Sicht.
- Drei Schadensarten und Rüstungsklassen, Veteranenstufen, Infanteriedeckung/-unterdrückung, Granate, Rauch, Artillerie-Deployment und je zwei Commander-Doktrinen. Pause sperrt die autoritativen Aktionen ebenso wie die HUD-Bedienung.
- Öffentliche Matchbefehle für Spieler und Bot; volle Wirtschafts-/Bau-/Kampfabläufe und Neustart. Natürliche Matchläufe verwenden keine geschenkten Kampfwerte oder unmittelbaren Test-Kills; gezielte isolierte Einzelfixtures sind davon getrennt.

ConceptV2, DistrictV3 und FrontlinesV4 enthalten eigene Modelle, Materialien, Gelände, Gebäude, UI und Porträts. Eigene prozedurale Audios, Waffen-/Raucheffekte und Infanterie-Skeletal-Mesh mit drei Clips sind im Paket. Die Präsentationsprüfung kontrolliert gespeicherte Paketressourcen und Materialzuordnungen. Ein erfolgreiches Importprotokoll allein wäre dafür nicht ausreichend.

## Dauerprüfung und verbleibende Grenzen

**Beide neuen 20-Minuten-RHI-Dauerläufe des 13:45-Pakets bestanden.** Sie liefen gleichzeitig von etwa 13:46:45 bis 14:06:45 Berlin bei 1600×900 und 60-FPS-Grenze. In den ersten Minuten liefen zusätzlich die Paketprüfungen. Dies ist keine isolierte Maximalleistungsmessung; einzelne lange Frames bleiben in den Rohdaten sichtbar. Kein erneuter Logistik-Stillstand, keine ungültigen Wallet-/Cargo-Werte und keine fehlenden natürlichen Spielausgänge.

| Fraktion | Echte Sekunden | Gefechte | FPS-Mittel | P95 / schlechtester Frame | Speicher Ende / Peak |
|---|---:|---:|---:|---|---|
| Aegis | 1200.009 | 7 | 59.647 | 16.735 / 285.805 ms | 1835.0 / 1986.2 MiB |
| Ember | 1200.005 | 8 | 59.620 | 16.752 / 156.053 ms | 1826.9 / 1997.8 MiB |

[Zusammenfassung mit EXE-/Berichthashes](verification/2026-10-15/Soak-Acceptance.json), [Aegis-Rohdaten](verification/2026-10-15/Soak-Aegis.json), [Ember-Rohdaten](verification/2026-10-15/Soak-Ember.json). Lokale Logs: Soak-Aegis-20261015-134643-285.log und Soak-Ember-20261015-134643-406.log. Die früheren 09:27-Berichte bleiben ausdrücklich historische Regressionsevidenz.

Die Codebasis bleibt ein GameMode-basierter Prototyp mit festem Rollen-/Asset-Registry-Vertrag. Die im ursprünglichen Architekturplan vorgeschlagenen UHR-Subsystemklassen sind nicht als fertige Module eingebaut. Beide Fraktionen teilen viele Meshes; eigene visuelle Fraktionssilhouetten, feinere Oberflächen, umfangreichere Animationen, Balance und bediente Langzeitpartien bleiben Entwicklungsarbeit. Technische Clipprüfung beseitigt nicht automatisch alle Bind-Pose-Warnungen oder ersetzt visuelle Animationsabnahme.

RenderOffscreen-Captures sind reale Spielgrafik, aber kein Nachweis manueller Bedienung oder grafischer Gleichwertigkeit mit Tempest Rising. REA-Originalbyteanker belegen analysierte Originalbereiche, keine bytegleiche Neukompilierung und keine vollständige Original-Verhaltensparität. Originalassets werden nicht in HeroHour übernommen. Kampagne, Multiplayer, Luft-/Marine-Roster und große Techbäume gehören nicht zum aktuellen Umfang.

Reproduzieren: Build-HeroHour.ps1 -Package, dann Test-HeroHour.ps1 -Mode Packaged -Suite mit den Tabellen-Suiten; zusätzliche Ember-Läufe mit -Faction Ember. Start: HeroHour-Starten.cmd beziehungsweise HeroHour-Ember-Starten.cmd. Weitere Anweisungen: [STARTEN.md](STARTEN.md), [Übergabe](CURRENT_HANDOVER.md), [Transformationsplan](TEMPEST_TRANSFORMATION_PLAN.md), [Docking-Abnahme](research/Refinery-Docking-Acceptance.md).

## Historischer Dauerlauffehler und aktuelle Korrektur

Das frühere 09:27-Paket bestand kurze Suiten, scheiterte aber im Aegis-Dauerlauf nach 7 abgeschlossenen Runden; Ember bestand separat mit 8 Runden. Diese Ergebnisse sind historische Nachweise unter [Aegis 09:27 FAIL](verification/2026-10-15/Soak-Aegis-0927-regression.json) und [Ember 09:27 PASS](verification/2026-10-15/Soak-Ember-0927.json), keine Dauerlaufabnahme des aktuellen Pakets.

Eine 10-Hz-Reproduktion bestand 20 Runden, während 60-Hz-Schritte bereits in Runde 1 den Sammler-Stillstand reproduzierten. Die Diagnose zeigte 43 erreichbare positive Zellen und eine erreichbare betriebsfähige Raffinerie; die Enklavenhypothese erklärt diesen Fall nicht. Analytische Segmentabstände gegen Rechtecke/Kreise und das Prüfen der vollständigen Verbindung beim Überspringen eines nahen Wegpunkts beheben den identifizierten Geometriefehler. Navigation prüft jetzt 20 Gates einschließlich drei Eck-/Wegpunktregressionen und zwei Brückenuferfällen. Beide Fraktionen bestanden danach die je 20 natürlichen 60-Hz-Stresspartien des aktuellen Pakets; die unabhängigen Echtzeit-Dauerläufe bestanden ebenfalls, siehe Dauerprüfung oben.
