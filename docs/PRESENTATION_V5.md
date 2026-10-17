# Presentation V5 – visueller Konzeptpass

Stand: 15.10.2026, V5-Paketprüfungen und beide Stabilitäts-Dauerläufe abgeschlossen. Dieser Pass verbessert die vorhandene CityHero-Szene mit eigenen Assets. Er ist kein Nachweis für die vollständige visuelle Qualität der Konzeptbilder.

## Eigene Assets und Herkunft

Die sechs neuen Dekorationsmeshes entstehen deterministisch durch `Scripts/art/build_ground_detail_v5.py`, Seed `150151`. Quellen und Exporte liegen unter `Art/Models/PresentationV5` als `.blend`, `.fbx`, einzelne `.json` und `manifest.json`. Es werden keine Modelle aus Tempest Rising oder anderen Spielen kopiert.

| Mesh | Dreiecke | Abmessungen und Verwendung |
|---|---:|---|
| `SM_PV5_RockCluster` | 400 | Verwittertes Sandsteincluster; skalierte Randgeologie und kleine Bodensteine |
| `SM_PV5_DryScrub` | 900 | Geschlossene trockene Zweige und Grasgeometrie ohne Alphakarten |
| `SM_PV5_CurbStrip` | 864 | Vier getrennte niedrige Betonrandsegmente |
| `SM_PV5_LowBarrier` | 864 | Niedrige Betonbarriere mit abgesetzten Details |
| `SM_PV5_SupplyCrates` | 1.296 | Drei Versorgungskisten mit Bändern und Kennzeichnungen |
| `SM_PV5_PipeBundle` | 756 | Drei geschlossene Rohre mit dunklen Stirneinsätzen und Haltebändern |

Alle sechs Builder-Reports melden geschlossene Manifold-Geometrie, nicht degenerierte Flächen, Bodennullpunkt, Metermaßstab und bestandene FBX-Roundtrip-Abmessungen. UVs verwenden dominante Flächenprojektion mit einer UV-Einheit pro Meter. Dies sind Geometrieprüfungen, keine spielbaren V5-Pakettests.

`polish_faction_materials_v5.py` erzeugt 20 neue Materialien unter `/Game/HERO/PresentationV5/Materials`: zehn Oberflächentypen je Aegis/Ember. Es verwendet vorhandene eigene DistrictV3-PBR-Texturen und liest ConceptV2-Vorlagen, ohne diese Quellmaterialien zu verändern. Aegis erhält sandfarbene Panzerung und blaue Markierungen; Ember dunkle Panzerung und rote Markierungen. Die Laufzeit ordnet Varianten nach Materialnamen zu, einschließlich der Skelettmaterialien. Das sind Oberflächenvarianten gemeinsamer Modelle, keine getrennten Fraktionsmodelle.

`render_faction_portraits_v5.py` erzeugt 22 tatsächliche Blender-Renderings der eigenen Modelle unter `Art/UI/PortraitsV5`, 512 × 512 Pixel mit Transparenz. `metadata.json` enthält Quellen und SHA-256-Werte sowie Angaben zur unveränderten Quellgeometrie. Der Importer weist die Porträts ausschließlich den Icon-Feldern vorhandener DataV4-Definitionen zu; Werte und Balance werden dadurch nicht geändert.

## Reproduzierbare Reihenfolge

Voraussetzung ist ein funktionsfähiger bisheriger Projektstand mit ConceptV2, DistrictV3, FrontlinesV4, InfantryRigV4 und DataV4. Nicht bestehende Import-Pipelines blind neu ausführen.

1. Im Blender-Batch `build_ground_detail_v5.py` ausführen. Die sechs Mesh-Metadaten und das Manifest prüfen.
2. Im Unreal-Python-Kontext `polish_faction_materials_v5.py` ausführen. Quellen müssen bereits importiert sein.
3. `import_ground_detail_v5.py` im Unreal-Python-Kontext ausführen. Ziel: `/Game/HERO/PresentationV5/Meshes`. Der Importer weist bestehende ConceptV2-Materialslots zu und fordert keine automatisch erzeugten Kollisionskörper an. Er platziert keine Kartenobjekte.
4. Die Porträts mit `render_faction_portraits_v5.py` im Blender-Batch rendern; anschließend `import_faction_portraits_v5.py` im Unreal-Python-Kontext ausführen. Ziel: `/Game/HERO/PresentationV5/Portraits` und DataV4-Icon-Verweise.
5. `polish_world_presentation_v5.py` im Unreal-Python-Kontext ausführen. Es lädt `/Game/Maps/CityHero`, ersetzt ausschließlich eigene `PV5 `-Dekorationen, weist neue Boden-/Asphaltmaterialien zu und erhält die vorherigen Nicht-PV5-Akteure. Dieser Schritt verändert die gespeicherte Szene.
6. Szene in einem frischen Editorprozess erneut laden und Kollisionszustand, Materialkompilierung, Porträts sowie tatsächliche Darstellung prüfen. Erst danach neu bauen, paketieren und V5-Abnahme durchführen.

Die Szene verwendet Seed `150026`. `Saved/Research/PresentationV5-Scene.json` protokolliert 280 Dekorationsplatzierungen und den Erhalt vorheriger Akteure. `PresentationV5-Import.json` protokolliert sechs importierte Meshes mit Zentimeterabmessungen; `faction-surfaces-v5.json` dokumentiert die Materialzuordnung. Dateien unter `Saved` sind lokale Arbeitsnachweise und ersetzen keine versionierte finale Abnahme.

## Platzierung und Navigationsvertrag

Der Positionsreview gegen `BattlefieldLayout.h`, Produktionsstartpositionen und Gebäuderaster ergab: HQ-Requisiten bleiben innerhalb des 6×6-Zellen-Rechtecks, Raffinerie-Requisiten innerhalb des 4×4-Zellen-Rechtecks. Innenstadt-Requisiten liegen innerhalb bestehender Stadtblock-Sperrflächen. Randfelsen bleiben außerhalb der spielbaren X-Grenzen. Der nächste Busch liegt rund 568 cm von einem Ressourcen-Pickup entfernt; kein Pickup wird durch diese Platzierungen direkt überdeckt.

Die aktuelle eigene A*-Navigation sammelt nur lebende Gebäude vom Typ `APrototypeUnit` als dynamische Hindernisse. Dekorative `StaticMeshActor` verändern diese Registry nicht. Für Dekoration gilt trotzdem der Vertrag: keine Kollision, keine Navigationseinwirkung. Eine bloße Beschriftung im Manifest ist dafür kein Laufzeitbeweis.

**Persistenzprüfung:** Nur `set_collision_enabled` überlebte den ersten Reload nicht. Das Platzierungsskript setzt jetzt zusätzlich das gespeicherte Profil `NoCollision`. Ein frischer Prozess bestätigt in `PresentationV5-Audit-Fixed.log` und `PresentationV5-SceneAudit.json` alle **280 Komponenten mit NO_COLLISION**; beide Editor-Präsentationsläufe bestehen die Detailprüfung. Das Skript deaktiviert auch Navigationseinwirkung.

Das historische Windows-Paket vom **15.10.2026, 13:45 Berlin**, `Packaging-20261015-134515-393.log`, CreatedUtc `2026-10-15T11:45:50.6583192Z`, gehört zur vorausgehenden Gameplay-Abnahme. Die dort dokumentierten 19 Paket-Testläufe und zwei 20-Minuten-Dauerläufe sind **keine V5-Abnahme**. Maßgeblich bleiben deren Build-ID und EXE-Hash in `docs/verification/2026-10-15`.

V5 ist inzwischen paketiert: Szenen-Reload, 19 Paketprüfungen einschließlich UI und Präsentation beider Fraktionen sowie echte Paketaufnahmen sind bestanden. Die aktuellen 20-Minuten-Grafikläufe sind abgeschlossen; beide bestehen die Stabilitätsprüfung, jedoch keine 60-FPS-Leistungsabnahme. Nachweise stehen im folgenden Abschnitt und separat im V5-Verzeichnis.

Die Karte, Brücken, Ressourcenlogik, Gebäuderaster und Kampfsystematik bleiben inhaltlich unverändert. Oberflächen und kleine Dekoration ersetzen weder ein vollständiges Landschafts- und Gebäuderedesign noch eigenständige Fraktionssilhouetten, Fahrzeuganimationen, hochwertige VFX oder professionelle Art-Direction-Abnahme. Das Ziel dieses Passes ist ein sichtbarer, überprüfbarer Schritt zu den Konzepten.



## Presentation V5 – aktueller Build 15.10.2026, 18:09 Berlin

20 eigene Fraktionsmaterialien, 22 transparente Blender-Porträts (512×512), sechs Detailmeshes und 280 gespeicherte kollisionsfreie Dekorationen sind integriert. [Assetverfahren und Grenzen](PRESENTATION_V5.md).

**19 Paketprüfungen PASS: 488 Aegis- und 115 Ember-Prüfpunkte, zusammen 603, dazu 46 natürliche Match-/Logistikpartien.** [Aktuelle Abnahme](verification/2026-10-15/presentation-v5/Transformation-Checks.json), [neue Paketaufnahmen](verification/2026-10-15/presentation-v5/Captures.json). **Beide neuen 20-Minuten-Grafikläufe bestanden:** Aegis 1.200,035 Sekunden / 7 vollständige Gefechte, Ember 1.200,016 Sekunden / 8. [Aktuelle Abschlussberichte](verification/2026-10-15/presentation-v5/Soak-Acceptance.json). Mittlere Bildraten 35,859 und 35,776 FPS unter gemeinsamer GPU-Last; neben den beiden Offscreen-Läufen liefen anfangs Paket-/Aufnahmeprüfungen und später ein zusätzliches natives Vollbildspiel. P95 86,774 / 85,806 ms; schlechteste Frames 648,469 / 761,464 ms. Stabilitätsprüfung bestanden, **keine 60-FPS-Leistungsabnahme**. Ein isolierter Profilinglauf bleibt erforderlich.

Paket: `Packaging-20261015-180917-692.log`, CreatedUtc `2026-10-15T16:09:49.5080994Z`. EXE-SHA-256: `54292950892f8e02c55fc615356a9286e42eee52adbdfd3bffaa404c6f08ffc7`.

Korrekturen: Dock-Wartewinkel aus räumlicher Ankunft statt UObject-ID; dauerhaft gespeichertes `NoCollision`-Profil; im Editor ohne Rendering Prüfung gültiger importierter Porträtquellen statt nicht erzeugter Plattformdaten. Paketprüfungen verlangen weiterhin 512×512-Laufzeittexturen. Beide Seiten starten jetzt mit gleichen Rollen und 800 Credits, ohne kostenlose Gegnerfabrik (14 Startobjekte). Der unveränderte natürliche Matchtest besteht für beide Fraktionen; versuchte zusätzliche defensive Teststrategien wurden entfernt. Das ist keine statistische Balanceabnahme. Ember-Overdrive-Zielwahl bleibt ein offener Botpunkt.
