# Erster tatsächlicher Tempest-Assettransfer

17. Oktober 2026. Ausgangspunkt ist der von der vorhandenen Setdressing-Karte referenzierte `SM_GDF_Container`. Es wurde kein Ersatzmodell generiert. Der Transfer wurde an einem isolierten Testprojekt tatsächlich ausgeführt; die aktive HeroHour-Karte wurde nicht verändert.

## Behobener Editorstart

Die originale `Tempest.uproject` war inzwischen auf `EngineAssociation: "5.8"` gesetzt. Vorherige Untersuchung und mitgelieferte README verlangen 5.4.4. Die jetzt installierte Engine unter `C:/Program Files/Epic Games/UE_5.4` meldet exakt 5.4.4, CL 35576357. Engine-, Tempest-, GameplayCommands- und NavGrid-Modulmanifeste besitzen dieselbe BuildId `33043543`.

Die Projektdatei wurde unter `Saved/Research/TempestEditor54-Recovery/Tempest.uproject.before-association-fix.json` gesichert und ausschließlich ihre Zuordnung auf 5.4 korrigiert. Keine Spiel-DLL wurde neu gebaut oder gepatcht. Keine Tempest-Quelltextrekonstruktion war zur Behebung dieser Meldung nötig. `Scripts/tools/Open-TempestEditor54.ps1` startet ausdrücklich die passende Engine und prüft Version und BuildId.

Beim abschließenden Kontrolllesen stand dort eine registrierte Engine-GUID statt der Versionszeichenfolge. Deren tatsächlicher Registryeintrag verweist auf `C:/Program Files/Epic Games/UE_5.4`; diese ebenfalls korrekte Zuordnung wurde nicht überschrieben. Der laufende Editorprozess verwendet die 5.4-Installation. Der semantische Vorher-/Nachhervergleich der Projektdatei unterscheidet sich ausschließlich in `EngineAssociation`.

Die Forschungsarbeit erfolgte anschließend an einer **getrennten vollständigen Arbeitskopie** unter `Saved/Tests/TempestMapEditor54` (18.605 kopierte Dateien; Robocopy ohne Kopierfehler). Nur dort wurden Python und der eigene Mapping-Probe aktiviert.

## Tatsächlich benutzte Werkzeugkette

1. CUE4Parse-MCP wurde isoliert gebaut und über echtes stdio getestet; keine globale MCP-Konfiguration geändert. [Prüfung](CUE4Parse-MCP-Local-Build-2026-10-17.md).
2. Der eigene .NET-10-Probe benutzt CUE4Parse und CUE4Parse-Conversion, Commit `605672df7a993a90094bad92d30c388d87ae6c1a`. Der erste Meshversuch lieferte einen echten `MappingException`: unversionierte Properties benötigen eine Typzuordnung.
3. Das eigene normale Editorplugin `TempestMappingsProbe` wurde für UE5.4.4 gebaut und in der Arbeitskopie ausgeführt: **36 Structs und elf Enums** aus echter Engine-Reflection. Keine Prozessinjektion und keine geratenen Property-Offsets. Für den neueren MSVC-Compiler wurde eine lokale `__has_feature(x)=0`-Kompatibilitätsdefinition im Probe-Modul benötigt; die Engineinstallation wurde nicht gepatcht.
4. Mit diesen Zuordnungen exportiert CUE4Parse **sieben Dateien erfolgreich**: Container-GLB, Materialbeschreibung, vier Containertexturen D/N/RMO/C und die referenzierte Grunge-Textur. Tatsächliche Parameter der Original-Materialinstanz wurden zusätzlich gelesen.
5. UE5.4 exportiert das Container-Mesh auch nativ als FBX (71.200 Bytes). Der erste native PNG-Texturversuch löste einen ImageCore-Assert aus. Dieser Weg wird nicht wiederholt; Texturen werden über den erfolgreich geprüften CUE4Parse-Decoder gelesen. Der korrigierte native Mesh-/Metadatenlauf beendet sich mit Exit 0.
6. Blender importiert GLB und nativen FBX getrennt: **1.463 Dreiecke**, ein Materialslot, gleiche Weltabmessungen **7,0123 × 3,0936 × 2,9915 m**. Unterschiedliche Vertex-/UV-Repräsentation ist dokumentiert. [Geometrie und Bildprüfung](Tempest-Container-VisualQA.md).
7. Das echte GLB und die vier Containertexturen wurden in einem neuen unabhängigen **UE5.8.3-Projekt** unter `Saved/Tests/TempestTransferProbe58` importiert. Prozess Exit 0, Marker `TEMPEST_CONTAINER_IMPORT58_RESULT=PASS`. Die importierten Abmessungen stimmen innerhalb 0,1 cm mit dem Original überein; ein Materialslot ist vorhanden. Ein einfaches Material bindet Original-Albedo und Normalmap. Es ist ausdrücklich noch nicht der komplette Tempest-Shader.

## Echte Karte statt erfundener Platzierung

Die vorhandene Setdressing-Karte wurde in UE5.4.4 geladen. Der JSON-Inventarlauf erfasst **8.279 geladene Actors**, drei geladene Levels, Klassen, Transformationen, Komponenten und Mesh-/Materialpfade. Darunter befinden sich **20 tatsächliche Containerinstanzen**. Ihre Originaltransformationen sind separat archiviert und müssen für weitere Testabschnitte nicht erfunden werden.

Der Inventarskriptstatus ist PASS, der gesamte Commandletprozess aber **Exit 1**: Drei Flugzeugtexturen fehlen in der Editorinstallation, außerdem meldet `TempestFieldVolume.cpp` einen behandelten `!IsRunningCommandlet()`-Ensure. Die JSON-Daten sind vorhanden; eine fehlerfreie vollständige Kartenabnahme wird daraus nicht abgeleitet. Der nächste vollständige Kartenlauf sollte außerhalb des Commandletmodus erfolgen und die fehlenden transitive Referenzen gesondert behandeln.

## Ergebnis und verbleibende Arbeit

**Nachgewiesen:** Ein wirklich vorhandenes Tempest-Objekt mit echten Texturen kann nach strukturiertem Auslesen in Unreal 5.8 importiert werden. Das ist ein konkreter erfolgreicher Teiltransfer, keine Behauptung einer fertigen Gesamtkarte.

Noch offen: vollständiger Materialgraph einschließlich RMO-Kanälen, Farbmasken und Wetter-/Schmutzfunktionen; weitere transitive Assets; Landschaft und Beleuchtung; Navigation und HeroHour-Spielanbindung. Reflection/JsonAsAsset ist separat quellengeprüft, aber noch nicht praktisch gebaut oder für den Materialimport benutzt. [Vorbereitung](Tempest-Reflection-Preparation.md).

Geometry, Texturen und Testprojekte bleiben lokal unter `Saved`. Im Repository werden eigener Probe-Code, Messungen, Pfade und Hashes dokumentiert, keine vollständigen Tempest-Spielassets eingebaut. [Transfernachweise](../verification/2026-10-17/container-transfer/Transfer-Checks.json).

Die Gestaltungsvorgabe bleibt unverändert: **Zero-Hour-Anordnung der UI, Tempest-Gestaltung**. Der zuletzt geprüfte Sidebarstand ist noch nicht das gewünschte finale UI-Layout.
