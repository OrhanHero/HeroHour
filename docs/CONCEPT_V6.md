# Konzeptumsetzung V6 – Arbeitsstand

Das Spiel ist nicht fertig. Die M1–M4-Mechanikprüfungen bestätigen implementierte Systeme, nicht die vollständige Umsetzung der Konzeptbilder. Dieser Durchlauf bearbeitet die sichtbare Spielwelt und die UI anhand der tatsächlich geöffneten Referenzen `01_22_00`, `01_22_12`, `01_22_20` und der CityHero-Kartenbilder.

## Verteilte Arbeit

Hier sind maximal vier aktive Agenten einschließlich des koordinierenden Hauptagenten möglich. Drei Fachagenten arbeiteten parallel; abgeschlossene Agenten erhielten weitere Aufgaben. Es wurden keine 15 gleichzeitig arbeitenden Agenten behauptet.

| Arbeitsbereich | Konkrete Lieferung |
|---|---|
| Gelände | Vier geschichtete Canyonwände und zwei unregelmäßige Flussufer, sechs eigene Meshes, 44.352 Dreiecke |
| Innenstadt | Vier neue Gebäude mit echten Fensterlaibungen, Gesimsen, Balkonen und Dachtechnik, 28 ersetzte Kartenobjekte |
| UI | Feinere dunkle Paneele, getrennte Armeezähler, übersichtlichere tatsächliche Schadens-, Reichweiten-, Rüstungs- und Geschwindigkeitswerte |
| Militärgebäude | Detailliertere HQ-/Fabrik-/Raffineriemodelle, gleiche Bounds/Pivots, sechs neue Fraktionsporträts |
| Prüfung | Erweiterte Präsentationsverträge für echte gespeicherte Assets, Ausrichtung, Kollision und Navigation |
| Integration | Serielle Unreal-Imports, neue Boden-/Asphaltmaterialien, Laufzeitaufnahmen und Paketprüfung |

## Grundlegender Platzierungsfehler

Die Sichtprüfung fand gekippte Stadthäuser und unter dem Boden liegende Wohnhäuser. Im hier installierten Unreal ergibt der Python-Aufruf `unreal.Rotator(0,90,0)` tatsächlich `pitch=90, yaw=0, roll=0`. Die bisherigen Skripte wollten damit eine horizontale Drehung erzeugen. Reine lokale Mesh-Bounds und erfolgreiche Importe erkannten diesen Fehler nicht.

Die Korrektur verwendet explizite Schlüsselwörter `pitch=0, yaw=..., roll=0`, repariert bestehende eigene Kartenobjekte und ergänzt Laufzeitprüfungen für 28 aufrechte Stadthäuser und 64 oberhalb des Bodens stehende Wohnhäuser. Rohbefund: `Saved/Logs/ConceptV6-RotationAudit.log`. Die strengen Kollisions-, Ressourcen- und Wegprüfungen bleiben erhalten.

Beim Militärimport wurde zusätzlich der Koordinatenwechsel von Blender zu Unreal ausdrücklich in den Bounds-Nachweis aufgenommen: Unreal spiegelt Y. Der Import prüft weiterhin mit unveränderter Toleranz gegen die korrekt transformierten Bounds; die Modellgeometrie wurde hierfür nicht verändert.

## Reproduktion

Vor den Importen bestehende CityHero-Karte sichern. Neue Geometrie zuerst mit den jeweiligen Blender-Buildern erzeugen; Quellen und FBX-Roundtrip-Berichte liegen unter `Art/Models/ConceptTerrainV6`, `ConceptCityV6` und `MilitaryDetailV6`.

Unreal-Imports ausschließlich seriell:

1. `polish_concept_world_v6.py`
2. `import_concept_city_v6.py`
3. `optimize_world_surfaces_v6.py`
4. `import_military_detail_v6.py`
5. `refresh_military_portraits_v6.py` in Blender; anschließend `import_faction_portraits_v5.py` in Unreal
6. `fix_authored_rotations_v6.py`
7. `build_road_finish_v6.py` in Blender, danach `polish_road_finish_v6.py` in Unreal. Die versionierte Eingabe liegt unter `Art/Models/RoadFinishV6/scene-input.json`.

Die regenerierten DataV4-Definitionen dürfen die neuen Mesh-/Porträtverweise nicht wieder mit älteren Pfaden überschreiben. Nach einer vollständigen DataV4-Neuerzeugung die V6-Militär- und Porträtimports wiederholen.

`Capture-HeroHour.ps1 -ArtReview -View District` beziehungsweise `Overview` erzeugt ausdrücklich eine Gestaltungskontrolle ohne HUD und sichtbaren Nebel. Dieser Modus ist auf diese zwei Kameras beschränkt und verändert weder die Simulationssicht noch die Zielauswahl. Normale Battle-Aufnahmen bleiben echte Spielaufnahmen mit Fog of War. Gestaltungsaufnahmen sind keine Nachweise einer vollständigen manuell gespielten Partie.

## Noch offen

Lange regelmäßige Asphaltflächen, kahle Basisgrundstücke, eigenständige Fraktionssilhouetten, differenzierte Industrie-/Wohngebiete, Fahrzeuganimationen, weitergehende Effekte, Ton und längere manuelle Partien bleiben Qualitätsarbeit. Das Spielfeld ist aktuell ein komprimierter 240×180-Meter-Ausschnitt. Eine Vergrößerung oder Änderung von Straßen und Blockern erfordert eine gemeinsame Überarbeitung von Navigation, Gebäuderaster, Ökonomie und Kameramaßstab.

Die neuen Bodenmaterialien ersetzen teure prozedurale 3D-Noise-Berechnung durch vier Texturabfragen auf zwei Maßstäben. Eine gemessene Leistungsverbesserung wird erst nach einem kontrollierten Lauf behauptet. Frühere V5-Dauerläufe gelten weiterhin ausschließlich für ihren damaligen Build.

## Neuester Stand: Konzeptdurchlauf V6, 15.10.2026

**Das Spiel ist weiterhin in Entwicklung und nicht vollständig konzeptgerecht fertiggestellt.** Drei Fachagenten arbeiteten in mehreren parallelen Arbeitswellen; der Hauptagent integrierte und prüfte. Technisches Maximum: vier aktive Agenten einschließlich Hauptagent.

Neu: sechs Canyon-/Ufermeshes mit verbesserten Normalen; vier detaillierte Stadtgebäude an 28 Positionen; drei überarbeitete Militärgebäude mit sechs aktualisierten Porträts; drei zusammengefasste Straßenfinish-Meshes; ruhigere UI und lesbare Kampfwerte. **999 gekippte eigene Kartenobjekte repariert**, darunter alle 64 Wohnhäuser. Die fünf betroffenen alten Python-Importskripte verwenden jetzt ausdrücklich benannte Rotationsparameter. [Umsetzung, Verfahren und Grenzen](CONCEPT_V6.md).

Aktuelles Windows-Paket: **19:35 Berlin**, `Packaging-20261015-193450-794.log`, CreatedUtc `2026-10-15T17:35:27.7561703Z`. **19 Paketprüfungen PASS: 512 Aegis- und 139 Ember-Prüfpunkte, zusammen 651; dazu 46 natürliche Match-/Logistikpartien.** Präsentation umfasst jetzt 100 tatsächliche Asset-/Szenenprüfungen je Lauf. [Aktuelle Nachweise](verification/2026-10-15/concept-v6/Transformation-Checks.json), [Paketaufnahmen](verification/2026-10-15/concept-v6/Captures.json).

Kurze Einzelinstanz-Messung bei 1600×900, Startkamera, ohne Gegnerbot, 60-FPS-Grenze: **55,39 FPS Mittel / P95 29,25 ms**, 1.500 ausgewertete Frames nach 300 Warmup-Frames. Die V5-Vergleichsaufnahme lag bei rund 60 FPS. Das ist keine Leistungsverbesserung und keine neue 20-Minuten-Abnahme. Weitere Profilierung bleibt offen. Die früheren Langzeitberichte gehören zu ihren damaligen Builds.

Unentdeckte Gebäude werden weiterhin normal vom Nebel abgedunkelt. Ein vorgeschlagenes zusätzliches Ausblenden wurde nicht eingebaut: Es würde die vorhandenen Silhouetten/Schatten verändern, und der Nutzen ist noch nicht nachgewiesen. ArtReview-Aufnahmen sind ausdrücklich getrennt vom normalen Spielmodus.
