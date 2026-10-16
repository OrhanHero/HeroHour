# TRELLIS HQ V8 — visuelle Prüfung des ersten echten GLB

## Ergebnis

**Technisch ein echtes, texturiertes 3D-Artefakt; visuell als Spielasset abgelehnt.** Der Kandidat erhält noch keine HQ-Ersatzfreigabe. Vorder- und Rückseite sind stark zerklüftet, die Satellitenschüssel ist größtenteils verloren und breite Fassaden bestehen aus unregelmäßig gefalteten Dreiecken. Dieselben Probleme sind im neutralen Wireframe ohne Originaltextur sichtbar. Ein erfolgreicher Inferenz-/Dateiexport ist damit kein Nachweis ausreichender Modellqualität.

Geprüft wurde ausschließlich `Art/Models/TrellisV8-Candidate/AegisHQ-Trellis512_00001_.glb`, 2.722.472 Byte, SHA-256 `fb53b1c581598eb06507375ffdaa0424b00b0844597fb5ff2cb11237fcd44f28`. Die Datei blieb vor/nach der Inspektion unverändert. Die erzeugte Ausgabe wurde weder repariert, vereinfacht, geglättet noch nach Unreal übernommen.

## Tatsächlich betrachtete Referenzen und Bilder

- Originales Nutzerkonzept: `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png`, insbesondere der helle Kommandobau rechts.
- Generierte Inferenz-Eingabe: `Saved/Research/TrellisV8/input/HeroHour-HQ-ReferenceV8.png`, SHA-256 `c28f14775d447077c6a32b20bddc9a344cab4353b719966c190750f750ed020e`. Tatsächlich geöffnet. Sie ist eine 1536×1024-RGBA-Datei mit vorhandenem Alphahintergrund, kein vollständig opakes Hintergrundfoto. Die Eingabe interpretiert den ursprünglichen Bau bereits mit mehr Anbauten, stärkerer Verschmutzung und zusätzlicher Antennendetailierung.
- [PBR A: Torseite, 3/4](../../Art/Models/TrellisV8-Candidate/Preview-PBR-A.png), [PBR B: gegenüberliegende Rückseite](../../Art/Models/TrellisV8-Candidate/Preview-PBR-B.png), [PBR C: weitere Ecke](../../Art/Models/TrellisV8-Candidate/Preview-PBR-C.png).
- [Wireframe A](../../Art/Models/TrellisV8-Candidate/Preview-Wire-A.png) und [gegenüberliegender Wireframe B](../../Art/Models/TrellisV8-Candidate/Preview-Wire-B.png).

Alle fünf tatsächlichen Render sowie beide extrahierten Texturatlanten wurden mit `view_image` geöffnet. Die drei PBR-Ansichten nutzen die unveränderten importierten glTF-PBR-Materialknoten und eingebetteten Texturen. Nur die zwei ausdrücklich bezeichneten Wireframes verwenden einen temporären Material-Override; es werden keine Bildtafeln als Modellvorschau eingesetzt.

## Geometrie- und Materialnachweise

Blender 5.2 importiert ein räumliches Mesh mit 20.682 UV-Vertices. Seine drei Ausdehnungen betragen nach glTF-Y-up-Konvertierung ungefähr **1,0003 × 0,7894 × 0,5577** normalisierte Blender-Einheiten. Die voneinander abweichenden Vorder-/Rückansichten und Wireframes bestätigen räumliche Tiefe und echte Dreiecksflächen, keine bloße Bildplane. Dies ist noch kein sinnvoller Unreal-Maßstab oder Bodenpivot.

Der Roh-GLB enthält laut separater Strukturprüfung 18.456 Indexdreiecke; Blender zeigt nach dem Import 18.446 Dreiecke. **Die Differenz von zehn ist derzeit nicht abschließend erklärt.** Der Prüfagent fand in den Float32-Rohpositionen keine exakt flächenlosen Dreiecke und keine unter seiner geprüften Flächenschwelle; die Differenz darf daher nicht einfach als nachgewiesene Degenerates-Bereinigung bezeichnet werden. Innerhalb der Inspektion sind Vertex-/Polygon-/UV-Zahlen vor und nach sämtlichen Rendern identisch. Es wurde keine eigene Mesh-Operation angewendet.

Der GLB exportiert keine NORMAL-Attribute. Blender berechnet aus der tatsächlichen Geometrie seine Flächennormalen; die Inspektion fügt keine Normalmap oder topologische Reparatur hinzu. Ein nativer PBR-Shader bindet BaseColor als sRGB und MetallicRoughness als Non-Color, jeweils **1024×1024**. Die importierten Knoten wurden protokolliert. Es gibt keine separate Emissionsmaske, Normalmap oder AO-Textur. Die blauen Fensterbereiche sind somit überwiegend Farbtextur, keine nachgewiesenen funktionalen Leuchtflächen.

## Konkrete Abweichungen

| Merkmal | Beobachtung am tatsächlichen GLB |
|---|---|
| Grundvolumen/Anbauten | Breiter Sockel, zentraler Aufbau und seitlicher Flügel sind grob erkennbar; ihre Außenflächen sind jedoch gefaltet, löchrig wirkend und teilweise stark zugespitzt. |
| Satellitenschüssel | Die klare runde Parabolform aus Konzept/Eingabe fehlt weitgehend. Übrig bleiben wenige dünne, unregelmäßige Flächen. |
| Antennen | Schmale Spitzen sind vorhanden, aber keine verlässlich geraden, stabilen Stäbe mit den Referenzdetails. |
| Tor | Ein dunkler torähnlicher Bereich ist erkennbar. Die offenen Bereiche bilden keinen sauber aufgebauten Torrahmen mit kontrolliertem Innenvolumen. Begehbarkeit oder ein korrekt hohles Garagentor sind nicht nachgewiesen. |
| Dach/Kommandoring | Nur grobe ringähnliche Massen; Panelkanten und umlaufende Fenster sind stark beschädigt beziehungsweise unregelmäßig. |
| Rückseite | Keine versteckte hochwertige Lösung: auch die gegenüberliegende Seite ist stark fragmentiert. Sie wurde ausdrücklich gerendert, nicht aus der Einzelansicht angenommen. |
| UV/Textur | UV0 und echte Texturen sind vorhanden. Der Atlas ist sehr kleinteilig und dunkel; im Render entstehen graue Flecken und blaue Fragmente statt klarer heller Panzerflächen. Eine Ursache ausschließlich in der UV-Zuordnung ist nicht belegt. |

Die Qualität liegt sowohl hinter der generierten Eingabe als auch hinter dem stabilen vorhandenen V7-Prototypmesh. Für einen nächsten Versuch sollten Geometrie vor/nach Extraktion beziehungsweise Export, Sampling-/Auflösungsroute und die Import-Zähldifferenz getrennt untersucht werden. Einfach nur stärkere Beleuchtung, geglättete Normalen oder mehr Texturkontrast würde die sichtbaren Geometrieprobleme nicht lösen. Die konkrete Fehlerursache wird hier nicht ohne Beleg auf TRELLIS-Modell, Quantisierung oder Export geschoben.

## Reproduktion und Übergabe

Skript: `Scripts/art/inspect_trellis_hq_v8.py`. Aufruf mit Blender 5.2 `--background --python Scripts/art/inspect_trellis_hq_v8.py`. Es prüft den Quellen-SHA, importiert den GLB, rendert aus neutralen Kameras und prüft anschließend erneut SHA und unveränderte importierte Geometriedaten. Es speichert keine `.blend`, keine neuen Meshes und keine Unreal-Assets.

Ausgeführter Prozess: **Blender PID 19304**, abgeschlossen; OPTIX, fünf Bilder in 18,24 Sekunden gemessener Skriptlaufzeit. Bilddateien entstanden am **16. Oktober 2026, 07:37:00–07:37:14 UTC / 09:37 Uhr Berlin**. stdout: `Saved/Logs/TrellisHQV8-VisualInspection-Blender.log`, Abschlussmarker `TRELLIS_HQ_V8_INSPECTION_PASS`; stderr: `Saved/Logs/TrellisHQV8-VisualInspection-Blender.stderr.log`. Dort stehen lediglich Laufzeit-/Deprecation-Hinweise, kein Render-Traceback. Der PASS-Marker bezeichnet die korrekte Durchführung der Inspektion und den Quellschutz — **keine visuelle Assetfreigabe**. Die GPU wurde danach für die nächste Integrationsaufnahme freigemeldet.
