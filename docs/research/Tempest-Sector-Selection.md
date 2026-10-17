# Tatsächlicher Tempest-Sektor: GDF-Containerhof

## Ausgeführter Transfer

Der vollständige Abschnitt aus `FullSector.json` wurde inzwischen tatsächlich exportiert und in UE5.8.3 importiert: **16 Mesh-Typen, 38 Actors, 19 importierte Baseline-Materialien, 16 importierte Albedo-/Normaltexturen**. Der zweite UE5.8-Importprozess endete mit **Exit 0**. Ziel ist ausschließlich `Saved/Tests/TempestTransferProbe58`, Map `/Game/TransferProbe/Maps/OriginalSector02`; die HeroHour-Spielkarte wurde dadurch nicht verändert.

Die genaue UE5.4-Komponentenprüfung bestätigt alle 38 Mesh-Components als Actorwurzel. Eine zusätzliche lokale Transformation fehlt damit nicht. Alle 64 Landscape-Komponenten wurden anhand ihrer tatsächlichen Weltbounds geprüft; `LandscapeComponent_38` und `_46` berühren den Abschnitt. Dies ist noch kein Landscape-Export. Der native UE5.4-Commandlet meldet weiterhin das bekannte TempestFieldVolume-Ensure und fehlende andere Beispielkartenassets; nur die gezielten Metadatenprüfungen sind PASS, keine fehlerfreie Vollkartenabnahme.

Die Materialauflösung wurde nach dem ersten abgebrochenen Import erweitert: tatsächliche Component-Overrides aus dem Setdressing-Plugin und Mesh-Standardmaterialien werden gemeinsam gelesen. Gleichnamige Varianten werden durch vollständige Assetpfade unterschieden. CUE4Parse lädt dafür die Virtual-Mounts aus den vorhandenen Pluginbeschreibungen. **13 Materialquellen und 31 Texturen** sind über echte Parentketten und Masterdefaults aufgelöst. Die UE-Materialien verwenden Originalalbedo und Normalen als Baseline; Farbmasken, Grunge, Wetter und vollständige Shaderparität bleiben offen.

Blender hat denselben Abschnitt unabhängig mit **19.788 instanzierten Dreiecken** gerendert. Root hat `Saved/Research/TempestSectorTransfer/VisualQA/OriginalSector.png` persönlich geprüft: Containerstapel, Abschlusswand, Technikprops und echte Bodenplatten sind sichtbar. Das Bild ist eine neutrale Blender-Prüfansicht, keine UE-Spielaufnahme. Keine zusätzliche Bodenebene wurde erfunden.

Reproduzierbare Skripte: `resolve_tempest_material_bindings54.py`, `verify_tempest_sector_components54.py`, `import_tempest_sector_probe58.py`, `render_tempest_original_sector_blender.py` und `record_tempest_sector_transfer.py` unter `Scripts/research/`. Begrenzte Nachweise einschließlich Originalpositionen: `docs/verification/2026-10-17/sector-transfer/`. Original-Geometrie und Texturpixel bleiben lokal unter `Saved`, außerhalb von Git.

Die folgenden Absätze beschreiben den ursprünglichen Auswahlstand vor diesen ausgeführten Prüfungen.

Quelle ist die erfolgreiche UE-5.4.4-Inventarisierung mit 8.279 Actors. Diese Auswahl wurde ausschließlich offline gelesen; keine Originaldatei wurde verändert und kein Engine-Prozess gestartet.

## Auswahl

Actor-Ursprünge innerhalb X 13900..15500 und Y 5850..7700 cm ergeben 38 StaticMeshActors und 16 unterschiedliche Meshes. Das ist ein real benachbarter Industriehof mit vier teils gestapelten Containern, zwei originalen Bodenplatten, Abschlussprofilen, Wand und mechanischen Anbauten. Kein neues Boden-Plane oder erfundene Komposition.

Der angeforderte begrenzte Probeumfang enthält 21 Actors und acht Mesh-Typen; dabei werden die sieben Wandprofil-Typen und ein benachbartes Kraftwerks-Prop explizit weggelassen. Für die vollständigere Übertragung enthält dasselbe Manifest zusätzlich alle 38 Actors und 16 Typen. Diese Variante ist vorzuziehen, wenn der Exportbatch 16 Meshes erlaubt.

`Saved/Research/TempestSectorTransfer/Selection.json` enthält Originalpositionen, Rotationen, Skalierungen und abgeleitete relative Positionen. Anchor ist der erste Container bei (14306.897261, 6533.27865, -158.965657); relative Koordinaten sind reine gemeinsame Verschiebung. Ebenfalls enthalten: alle 20 Containerpositionen der Karte und die Referenzen des globalen Landscape-Actors.

## Grenzen und nächste Prüfung

Die Inventarisierung liefert Actor-, aber keine Component-Transforms oder Geometriebounds. Für exakte Platzierung muss noch geprüft werden, ob die StaticMeshComponents relativ zur jeweiligen Actorwurzel identisch transformiert sind. Die rechteckige Auswahl verwendet Actorursprünge, keine Mesh-Ausdehnungen.

Das globale Landscape liegt bei (-24800,-24800,0), skaliert mit (100,100,100). Seine 64 LandscapeComponents müssen anhand echter Componentbounds auf den Sektor eingeschränkt werden. Das Inventar enthält diese Bounds noch nicht. Die Original-Bodenplatten sind bereits selektiert; das Landscape darf nicht durch eine erfundene Ebene als angeblich original ersetzt werden.

Alle 16 referenzierten Meshdateien existieren im ModKit. Die bounded Offlineanalyse bestätigt bei allen `cooked=true`, `FilterEditorOnly=true` und unversionierte Legacy-8-Pakete. Der bestehende Tabellenparser unterstützt diese unversionierten Importtabellen nicht; deshalb wird keine Material-Importauflösung behauptet. Materialpfade stammen stattdessen aus den tatsächlich geladenen MeshComponents im UE-Inventar. Hashes und bestätigte Präfixfelder sind in `PackageInspection.json` abgelegt.

## Tatsächliche Häufigkeiten

- SM_Research_WallPiece_x4: 1
- ResearchTasks_wallTrim_01: 2
- ResearchTasks_wallTrim_02: 4
- ResearchTasks_wallTrim_03: 2
- ResearchTasks_wallTrim_04: 2
- ResearchTasks_wallTrim_05: 2
- ResearchTasks_wallTrim_07: 2
- ResearchTasks_wallTrim_08: 2
- SM_Research_floorTiles: 2
- SM_GDF_Container: 4
- SM_AirtowerDetail_01: 9
- SM_FactoryDetail_02: 1
- SM_Research_floorTileBorderMesh_02: 2
- SM_Research_floorTileBorderMesh_01: 1
- SM_Research_floorTileBorderMesh_03: 1
- SM_GDF_Powerplant_Prop_01: 1

Dieser Stand beweist eine nachvollziehbare Sektorauswahl und existierende Quelldateien, noch keinen erfolgreichen Sektorimport oder vollständig rekonstruierten Shader.

Root verwendet nun die vollständige Variante: `Saved/Research/TempestSectorTransfer/FullSector.json`, mit standardisierten Feldern `mesh_paths` (16) und `actors` (38).
