# TRELLIS HQ V8: CPU-Diagnose und kontrollierter nächster Versuch

Stand: 16. Oktober 2026. Nur eigene erzeugte Geometrie und lokal installierter ComfyUI-Quellcode wurden untersucht. Kein Tempest-Modell wurde extrahiert, kein Spiel/Unreal/Blender und keine GPU-Inferenz gestartet. Der abgelehnte GLB blieb byteidentisch.

## Gesicherter Befund

Der Fehler ist nicht nur eine fehlende Textur oder eine Bildplane. Die bereits dokumentierten fünf tatsächlichen Render zeigen beschädigte Oberflächen und eine verlorene Schüssel. Die CPU-Prüfung `Scripts/research/diagnose_trellis_hq_v8.py` liest die originalen GLB-Accessorbytes, statt Blender-Zahlen als Rohdaten auszugeben. Ergebnis in `docs/verification/2026-10-16/reference-v8/trellis/cpu-diagnosis.json`:

| Messung | Ergebnis |
|---|---:|
| Indexdreiecke im unveränderten GLB | 18.456 |
| UV-Vertices / exakt unterschiedliche Float32-Positionen | 20.682 / 8.600 |
| Positionsverschweißte Zusammenhangskomponenten | 9 |
| Größte Komponente | 17.953 Dreiecke, 97,3 % aller Dreiecke |
| Einzelne isolierte Dreiecke | 0 |
| Randkanten / nichtmannigfaltige Kanten | 6.441 / 1.548 |
| Exakt flächenlose Dreiecke | 0 |
| Zusätzliche positionsgleiche Dreiecke, unabhängig von Umlaufrichtung | 14 |

**Einordnung:** Das sichtbare Fragmentieren bedeutet hier nicht Tausende voneinander getrennte Dreieckobjekte. UV-Nähte erklären einen großen Teil der Vertexvermehrung. Dennoch hat das exportierte positionsverschweißte Netz viele offene beziehungsweise mehrfach belegte Kanten. Exakte Positionsverschweißung erfasst keine nur annähernd gleichen Positionen. Die 14 positionsgleichen Dreiecke erklären **nicht nachweislich** die Differenz von zehn Dreiecken beim Blender-Import; diese bleibt offen.

## Abgleich mit tatsächlichem lokalem Primärquellcode

Die Dateien und ihre SHA-256 stehen im CPU-Bericht. Grundlage ist die bereits gespeicherte installierte Vorlage `Saved/Research/TrellisV8/comfy-installed-trellis-template.json`, nicht eine erfundene ideale Pipeline.

1. **Sampler sind weitgehend vorlagengetreu.** Structure: Seed 56, 12 Schritte, CFG 7,5, Euler/normal, Shift 5, CFGOverride ab 0,667 und Rescale 0,7. Shape: Seed 42, 20 Schritte, CFG 7,5, Euler/normal, CFGOverride ab 0,769 und Rescale 0,5. Texture: Seed 43, 12 Schritte, CFG 1. Diese Werte stimmen mit den jeweiligen Vorlagenstufen überein.
2. **Die Vorlage ist vielseitig und kein identisches Kontrollbeispiel.** Ihr Boolean-Schalter steht standardmäßig auf Pixal3D. Unser expliziter TRELLIS.2-Loader ist beabsichtigt. Außerdem enthält die Vorlage eine 1536-Upsample-Stufe; unser gemessener Pilot bleibt bei 512. Eine fehlende Schüssel darf deshalb nicht ohne Kontrollversuch ausschließlich der Quantisierung oder 512 zugeschrieben werden.
3. **Konkrete ausgelassene Nachbearbeitung:** Die aktive Vorlagenroute führt `GetMeshInfo → RemeshMesh` (UDF, Auflösung 768) → `DecimateMesh` (700.000) → `MeshSmoothNormals` (180 Grad) → `UnwrapMesh`. Unser Pilot führt direkt `VaeDecodeShapeTrellis → DecimateMesh` (60.000) → `UnwrapMesh`. Er lässt somit Remeshing und Normalenerzeugung weg. Beide Unterschiede sind real, ihre jeweilige Wirkung auf diesen Kandidaten ist noch nicht gemessen.
4. **SaveGLB verändert Indextopologie nicht absichtlich.** `comfy_extras/nodes_save_3d.py`, Funktion zur GLB-Serialisierung: Float32-Positionen und UInt32-Indizes werden als Bytes geschrieben. NORMAL wird nur geschrieben, wenn es im Eingabemesh vorhanden ist; die Quelldokumentation nennt ausdrücklich `MeshSmoothNormals`. Fehlende Normalen erklären flache Beleuchtung, aber reparieren keine verlorene Parabolform.
5. **UV-Unwrap kann Topologie beeinflussen.** `nodes_mesh_postprocess.py:UnwrapMesh.execute` ruft `_uv_unwrap` mit Positionsverschweißung auf und übernimmt dessen `vmapping` und Indizes. Die Quelldokumentation beschreibt Vertexkopien an Chartnähten. Es ist daher nicht zulässig, ohne Zwischenexport alle Probleme der Shape-VAE zuzuschreiben. Der Log meldet 2.628 Charts für 18.456 Flächen.
6. **Die Decimation ist nur ein Maximalziel.** `DecimateMesh.execute` ruft QEM nur oberhalb der Zielzahl auf. Ein Export mit 18.456 statt 60.000 Dreiecken beweist weder einen anfänglich kleinen Decoderoutput noch, dass QEM das Ziel exakt treffen musste. Der Log belegt einen ausgeführten QEM-Prozess; Roh-/Zwischenzahlen fehlen.

## Empfohlener nächster Schritt: ein messbarer Kontrolllauf

Nicht blind ein weiteres Bild oder einen anderen Seed erzeugen. Dieselbe Eingabe, dieselben Modelle, Seeds und Sampler beibehalten und vier Geometriezweige als untexturierte GLBs getrennt speichern:

1. Direkt nach `VaeDecodeShapeTrellis` als **RAW**.
2. Nach unverändertem 60k-Decimator als **DECIMATED**.
3. Nach unverändertem UV-Unwrap als **UNWRAPPED**.
4. Separater Zweig RAW → UDF-Remesh → Decimation → SmoothNormals als **REMESHED**, bevor neues Texturebaking bezahlt wird.

So lässt sich die erste problematische Stufe über Bounds, Flächen, Positionskomponenten und dieselben neutralen Vorder-/Rückansichten bestimmen. Ein Rohmesh mit bereits zerstörter Schüssel macht weitere UV-Arbeit zwecklos. Ein gutes RAW mit beschädigtem DECIMATED verlangt Korrektur der Vereinfachung. Ein gutes DECIMATED mit beschädigtem UNWRAPPED verlangt Prüfung des Weld-Radius und des UV-Indexmappings. Ein besseres REMESHED ist noch kein guter Formersatz: UDF kann Löcher schließen und dünne Details zugleich verschlucken. Erst nach Sichtprüfung texturieren und auf Unreal-Maßstab/Pivot bringen. GPU-Ausführung wird mit dem Hauptagenten koordiniert, hier nicht gestartet.

## Tatsächlich ausgeführter Kontrollversuch

Nach Freigabe des GPU-Zeitfensters wurde **ein** Inferenzauftrag `24467459-47df-44b4-a7d9-293888515952` ausgeführt. Dieselbe Eingabe, dieselben Gewichte und Samplereinstellungen; keine Texture-VAE oder Bakes. Remesh wurde bewusst auf **512 statt Vorlagenwert 768** gesenkt: Der Quellcode schätzt `resolution³×64` Byte, entsprechend 8 GiB statt 27 GiB; auf unserer 16-GiB-Karte wurde 768 nicht erzwungen.

| Tatsächlicher Export | Vertices | Dreiecke | Randkanten | Nichtmannigfaltige Kanten |
|---|---:|---:|---:|---:|
| RAW | 1.342.444 | 3.054.870 | 112.268 | 364.031 |
| DECIMATED, Ziel 60.000 | 18.534 | 17.803 | 6.448 | 1.311 |
| UNWRAPPED | 19.789 | 17.796 | 6.441 | 1.312 |

Die Decimation reduziert tatsächlich um etwa **99,42 %** und unterschreitet ihr 60k-Ziel deutlich. Größte Dreiecksfläche steigt von 0,00000847 im Rohmesh auf 0,03588 nach Decimation, bei nahezu gleichen Gesamtbounds. Damit ist eine starke Veränderung **vor dem UV-Schritt** nun belegt. Das Rohnetz besitzt allerdings selbst bereits sehr viele nichtmannigfaltige Kanten. Ohne neutrale RAW-/DECIMATED-Render wird weder der Decoder künstlerisch freigesprochen noch QEM allein als Ursache erklärt. Die aktuelle Ausgabe ist trotz gleicher Seeds nicht byteidentisch zum ersten Pilot; GPU-Bitdeterminismus wurde nicht erzwungen oder behauptet.

**Teilweise gescheitert, nicht als PASS umdeuten:** Die drei obigen GLBs wurden gespeichert; der vierte REMESHED-Zweig scheiterte vor Remesh-Berechnung mit `RemeshMesh.execute() missing 1 required positional argument: 'sign_mode'`. Unsere GET-Schemavalidierung erkannte den verschachtelten DynamicCombo als gültige Wahl, prüfte aber dessen Laufzeitnormalisierung nicht vollständig. Der tatsächlich installierte `_io.py:DynamicCombo._expand_schema_for_dynamic` vergleicht den Live-Input direkt mit dem Option-Key: Die korrekte API-Form ist `sign_mode: "udf"` mit flachen `sign_mode.qef` usw.; ein verschachteltes `sign_mode`-Dictionary wird hier nicht normalisiert. Das war ein Workflowfehler, **kein OOM**. Kein automatischer zweiter GPU-Auftrag wurde gestartet.

Gesamtlauf einschließlich Serverstart: 100,56 Sekunden; eigentliche Promptausführung laut Server: 18,81 Sekunden. Beobachteter ganzer GPU-Kartenspeicher maximal 9.256 MiB, Stichprobe alle drei Sekunden. Eigener Server PID 9056 wurde danach beendet. Explizite eigene SQLite-URL; ursprüngliche Nutzer-Datenbank vorher/nachher byteidentisch, Nutzerport 8188 unberührt. Ein vorheriger CLI-Preflight ohne Auftrag stoppte wegen fehlendem eigenen User-Verzeichnis; nach Anlegen dieses Verzeichnisses startete der Server. Beides bleibt in `Saved/Research/TrellisV8/diagnostic/` dokumentiert.

Unveränderte eigene Exporte liegen unter `Art/Models/TrellisV8-Diagnostic/`, getrennt vom abgelehnten ersten Kandidaten. Kompakte Belege: `controlled-workflow-executed.json`, `controlled-run-status.json`, `controlled-schema-validation.json`, `controlled-geometry-comparison.json`. Reproduktion: `prepare_trellis_diagnostic_v8.py`, `run_trellis_diagnostic_v8.py`, `compare_trellis_diagnostic_v8.py`; der gespeicherte ausgeführte Workflow dokumentiert ausdrücklich den fehlerhaften vierten Zweig. Vor jeder weiteren Ausführung muss der DynamicCombo korrigiert und über die tatsächliche Normalisierung getestet werden. Nächster Schritt bleibt die neutrale Sichtprüfung der jetzt verfügbaren RAW-/DECIMATED-Geometrie, bevor erneut Inferenz oder Texturbaking gestartet wird.

## Neutrale Sichtprüfung und gezielte Geometriereparatur

Alle neun tatsächlich erzeugten RAW-/DECIMATED-/UNWRAPPED-Ansichten wurden anschließend selbst geöffnet (`Preview-{RAW,DECIMATED,UNWRAPPED}-{Front,Back,Side}.png`). Gleiches neutrales Material und dieselben aus RAW abgeleiteten Kameras: **RAW besitzt eine vollständige runde Schüssel, lesbare Panzerflächen, Tor und Serviceflügel. DECIMATED verliert diese Form und erscheint zerknüllt; UNWRAPPED zeigt denselben Defekt.** Damit ist die sichtbare Verschlechterung durch unsere direkte Vereinfachungsroute jetzt stärker belegt als durch reine Zahlen. RAW bleibt hinsichtlich seiner nichtmannigfaltigen Topologie technisch problematisch, ist aber visuell deutlich hochwertiger als der zuvor irrtümlich als generative Grenze behandelte Export.

Genau ein gezielter Reparaturauftrag wurde ausgeführt: Das gespeicherte RAW wurde read-only als Tensoren geladen und direkt durch **dieselben installierten nativen Comfy-Funktionen** `remesh_narrow_band_dc`, `qem_decimate_simplify`, `_compute_vertex_normals` und `save_glb` geführt. Kein erneutes Diffusionssampling, kein Server, keine neue Datenbank. Das umgeht die zuvor fehlerhafte API-Parameterverpackung, ohne die Verarbeitung durch eine unbekannte Ersatzimplementierung zu wechseln. Skript `Scripts/research/remesh_trellis_raw_v8.py`, harte Laufzeitgrenze 600 Sekunden, tatsächlich 21,34 Sekunden.

Ergebnis: **UDF512 → 3.203.446 Flächen → QEM-Ziel700k → 699.820 Dreiecke / 349.991 Vertices mit NORMAL-Attributen**. Peak Torch-Allocation 3.469.046.784 Byte, Reserved 5.681.184.768 Byte. Original-RAW-SHA unverändert. `REMESHED512-RAW.glb` und `REMESHED_00001_.glb` bleiben getrennte eigene Dateien. Nachfolgend wurden Front, Rückseite und Seite des endgültigen 699k-Modells tatsächlich gerendert und selbst betrachtet: Schüssel, Tor, Turm und Fassaden bleiben stabil; 20 Glättungsdurchgänge machen feine Kanten sichtbar weicher. **Geometrie ist als wesentlich besserer Kandidat bestätigt, kein fertiges texturiertes Spielasset.** Der frühere fehlgeschlagene Schema-/Promptlauf bleibt vollständig dokumentiert.

Reparaturresultat: `remesh-native-result.json`; zwölf Render mit SHA und Quellschutz: `neutral-render-evidence.json`; erweiterte Rohstatistiken: `controlled-geometry-comparison.json`. GPU-Prozesse beendet und freigemeldet. Ein anschließender Texturworkflow ist nur vorbereitet in `Saved/Research/TrellisV8/repaired-texture/`, erzeugt durch `prepare_trellis_repaired_texture_v8.py`: gleiche ursprüngliche Bild-/Sampler-/Voxelroute, neue UVs auf repariertem Mesh, frisches Voxel-PBR-Baking in 2048² und Normalenberechnung nach UV. **Keine Textur oder UV vom zerknüllten Kandidaten übertragen.** Er ist noch nicht ausgeführt und muss gegen die tatsächliche V3-DynamicCombo-Normalisierung validiert werden.

## Erfolgreicher frischer PBR-Texturauftrag

Nach erneut freigegebenem GPU-Zeitfenster wurde der vorbereitete korrigierte Workflow **tatsächlich** gegen Live-Schemata geprüft und als ein Auftrag `43a7d03e-7e80-4779-8af3-20c5437c8931` ausgeführt. Die DynamicCombo-Optionen wurden diesmal flach entsprechend dem Primärquellcode verpackt und vor dem Queueing anhand der gewählten Option erweitert geprüft. Ergebnis **success**, keine automatische Wiederholung: Prompt 46,55 Sekunden, einschließlich Start/Monitoring 73,49 Sekunden, Kartenspeicher-Stichprobenpeak 9.489 MiB. Eigener Server PID10960 beendet; eigene explizite Datenbank-URL; Nutzer-Datenbank byteidentisch.

Ergebnis `Art/Models/TrellisV8-Repaired/AegisHQ-Remeshed512-PBR2048_00001_.glb`, SHA-256 `802747bb8124a02ad0c0f899f4a2dc5b9578f23bff12ea534c28d3b5cd201f03`, 31.218.732 Byte: **699.820 Roh-Indexdreiecke, 495.217 UV-Vertices, vollständige NORMAL-Attribute und zwei eingebettete 2048²-PBR-Texturen**. Blender importiert 699.814 Dreiecke; die Differenz von sechs ist nicht abschließend erklärt. BaseColor ist sRGB; MetallicRoughness ist Non-Color mit G=Roughness/B=Metallic/R ungenutzt. Kein AO-, Emissions- oder Normalmap-Kanal wird erfunden.

Die drei tatsächlichen PBR-Bilder sowie zwei Wireframes wurden erzeugt; alle drei PBR-Ansichten selbst geöffnet. Sie zeigen nun abgenutzte helle Panzerung, blaue Fenster, vollständige Schüssel und stabile räumliche Gebäudeteile auch rückseitig. Das ist ein **wesentlich besserer originaler HQ-Kandidat**, kein bereits abgenommenes Unreal-Spielasset. Maßstab, Bodenpivot, Kollision, Fraktions- und Schadensmaterial sowie echte Spielansicht bleiben Integrationsaufgaben. Inspektionsskript `render_trellis_repaired_v8.py` prüft Quell-SHA und unveränderte importierte Geometrie vor/nach fünf Rendern; PID15800, 27,43 Sekunden. Separate Rohprüfung `verify_trellis_repaired_v8.py` extrahiert nur unveränderte eingebettete Texturbytes. Kompakte Belege: `repaired-{run-status,schema-validation,workflow-executed,pbr-verification,pbr-render-evidence}.json`.

Parallel wurden zwei kleinere Kandidaten **ausschließlich auf CPU** aus dem vorherigen stabilen remeshten Netz erzeugt: Blender-Collapse-Decimation 149.996 und 99.994 Dreiecke, Quellen unverändert, 11,87 Sekunden. Das sind eigenständige untexturierte LOD-Versuche; sie haben noch keine visuelle Abnahme und dürfen nicht automatisch als gleichwertig gelten. Ihre alte UV-freie Geometrie ist auch nicht einfach mit der neuen PBR-Textur zu verbinden. Für ein kleineres finales Mesh ist eigener frischer Bake oder gezielter Attributtransfer mit Sichtprüfung erforderlich.

## Tempest-REA → eigene sichtbare Sammlerfunktion

**Beobachtet in bereits gespeicherter REA-/Dateievidenz:** `Tempest-REA-V7.md` belegt Skeleton/SkeletalMesh- und Clipreferenzen für MoveStart/Move/MoveStop, Harvest_Start/Loop/Stop und Unload_Loop/Stop. Die Metadaten unterscheiden Treads- und Körpermaterial. `Tempest-REA-HarvesterV8.md` und `harvester-v8/` belegen getrennte schreibbare Ernte-/Ablagefelder und Kandidaten für Kapazität/Frequenz. Das ist keine Beobachtung abgespielter Animation, kein genauer Docktransform und kein Nachweis einer Clipdauer. Die damalige native Sitzung ist geschlossen und wird für diese Zuordnung nicht erneut geöffnet.

**Konkrete eigene Umsetzung:** Die vorhandenen `EPrototypeCollectorState`-Zustände in `PrototypeGameMode.h` und ihre Übergänge in `PrototypeEconomy.cpp` treiben ausschließlich eine Präsentationskomponente an:

| Autoritative Situation | Eigene visuelle Reaktion |
|---|---|
| Bewegung in SeekingField / MovingToRefinery | Räder beziehungsweise Laufband mit tatsächlich zurückgelegter Strecke antreiben; nach Stillstand auslaufen lassen. |
| Eintritt Harvesting | Eigenen Sammelarm absenken, Werkzeug ausfahren; visuelle Startdauer frei gestalten. |
| Weiter Harvesting | Werkzeug zyklisch bewegen, geringer Staub am eigenen Werkzeug, Ladungsanzeige aus tatsächlichem Cargo/Capacity. |
| Verlassen Harvesting | Werkzeug einziehen; Unterbrechung und Felderschöpfung abbrechbar behandeln. |
| WaitingForDock | Motor im Leerlauf, Werkzeug geschlossen, keine Abladeeffekte. |
| Eintritt/Verbleib Unloading | Eigenen Schlauch/Entladeklappe ausfahren; Effekt nur bei tatsächlicher Cargo-Abnahme. |
| Verlassen Unloading | Schlauch/Klappe zurück; bei zerstörter Raffinerie oder neuem Befehl sauber zurücksetzen. |

**Gestaltungsentscheidung, keine Tempest-Rekonstruktion:** Beginnend mit einer eigenen hierarchischen StaticMesh-Präsentation ist das sichtbar und ohne fremdes Rig umsetzbar. Zwei Komponenten für Werkzeug und Klappe reichen für einen ersten vollständigen Zyklus. Kein Animationsnotify darf Ressourcen abbuchen, Dockslots freigeben oder Simulationszeiten ändern. Eine Aufnahme Feld → Dockwarteschlange → Entladen → Feld und identische Cargo-/Credits-Endwerte bei verschiedenen Bildraten sind der relevante Abnahmenachweis. Dies verbessert genau das derzeit statisch wirkende Sammelfahrzeug, ohne noch eine unbelegte Vollrekonstruktion zu behaupten.
