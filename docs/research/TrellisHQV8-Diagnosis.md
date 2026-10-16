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
