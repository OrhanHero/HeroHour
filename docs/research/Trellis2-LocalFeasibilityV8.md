# TRELLIS.2: lokale Machbarkeit und gemessener HQ-Pilot

Stand: 16. Oktober 2026. **Gemessenes Go für den getesteten lokalen 512er-TRELLIS.2-Pfad mit 1024er-PBR-Texturen.** Genau ein freigegebener HQ-Lauf über das vorhandene native ComfyUI hat erfolgreich eine GLB-Datei erzeugt, ohne OOM, fehlende Kernel oder Wiederholung. Die vier benötigten Modelldateien sind vollständig anhand Größe/SHA-256 geprüft. Das Ergebnis ist ein gesonderter Kandidat; bestehende HeroHour-Modelle wurden nicht ersetzt und kein Unreal-Import vorgenommen.

Die native ComfyUI-Implementierung mit quantisiertem Modell und Speichermanagement lief auf der vorhandenen 16-GB-Karte einschließlich Shape-/Texture-Decoding, UV-Unwrap und Texturbake. Das ist ein Nachweis für diesen einen Workflow und diese Eingabe. Die allgemeine Eignung aller Assets, höhere Geometrieauflösungen und der direkte Microsoft-BF16-Pfad sind damit weiterhin nicht nachgewiesen.

## Gemessener Rechner und vorhandene Software

| Punkt | Beobachtet |
| --- | --- |
| GPU | NVIDIA GeForce RTX 4070 Ti SUPER, 16.376 MiB VRAM, Compute Capability8.9, Treiber617.42. Zum Hardware-Check waren15.370 MiB frei; das ist kein zugesichertes Inferenzbudget. |
| RAM | Windows meldet68.448.337.920 Byte nutzbaren physischen Speicher, rund63,75 GiB; 20 logische Prozessoren. |
| Freier Platz vor Download | E:292.733.480.960 Byte; C:247.608.483.840 Byte. Vor dem Download wurde zusätzlich die Schranke mindestens20 GB freier Platz geprüft. |
| Windows | Windows11 Pro Insider Preview, Build10.0.29683. Globales Python3.14 ist nicht der verwendete Modellruntime. |
| WSL | Ubuntu26.04, WSL2-Kernel6.18.33.2; GPU sichtbar. Rund31,2 GiB RAM und8 GiB Swap. Kein CUDA-Toolkit/nvcc, Conda, Torch oder TRELLIS in der geprüften Linux-Umgebung vorhanden. |
| Vorhandener nativer Runtime | `E:/Comfy-Desktop/ComfyUI-Installs/Hero/ComfyUI`, Version0.37.2; dessen `.venv/Scripts/python.exe` verwendet Python3.13.12 und Torch2.12.1+cu130. CUDA-Verfügbarkeit und korrekter GPU-Name wurden abgefragt. |
| Native Nodes | `comfy_extras.nodes_trellis2` und `nodes_mesh_postprocess` wurden tatsächlich importiert: Exit0, Marker `TRELLIS_NATIVE_IMPORT_PASS`. Damit sind nicht nur Dateien auf der Platte gefunden. |

Der zunächst untersuchte `standalone-env/python.exe` ist lediglich der Basisinterpreter und enthält selbst kein Torch. Der korrekte ComfyUI-Interpreter ist die darunter eingesetzte `.venv`. Vorhandene DINOv2- oder Bildgenerierungsmodelle ersetzen die benötigten TRELLIS.2/DINOv3-Gewichte nicht.

## Offizieller Microsoft-Pfad und native Alternative

Microsoft beschreibt einen auf Linux geprüften Ablauf mit mindestens24 GB NVIDIA-Speicher, CUDA-Toolkit, empfohlen12.4, und einem Setup mit Torch2.6/CUDA12.4/Python3.10. Die veröffentlichten Zeiten beziehen sich auf H100 und sind keine Zeitprognose für diesen Rechner. Die direkten Linux-Voraussetzungen sind hier noch nicht eingerichtet. [Microsoft-README](https://github.com/microsoft/TRELLIS.2#installation), [Setupskript](https://github.com/microsoft/TRELLIS.2/blob/main/setup.sh).

Der offizielle Pipelinecode besitzt zugleich standardmäßig `low_vram=True` und verschiebt einzelne Modelle zwischen CPU und GPU. Das ist eine vorhandene technische Entlastung, jedoch keine dort garantierte16-GB-Abnahme. Die Standardkonfiguration lädt zusätzlich DINOv3 und RMBG sowie mehrere Auflösungsstufen. Diese zusätzlichen Modellzugriffe und der komplette CUDA-Buildstack sind für den hier gewählten nativen Weg nicht nötig. [Microsoft-Pipelinecode](https://github.com/microsoft/TRELLIS.2/blob/main/trellis2/pipelines/trellis2_image_to_3d.py), [Pipelinekonfiguration](https://huggingface.co/microsoft/TRELLIS.2-4B/blob/main/pipeline.json).

Comfy-Org veröffentlicht einen ausdrücklich als Microsoft-TRELLIS.2-Repack ausgewiesenen Single-File-Satz, darunter INT8- und BF16-Varianten. Die bereits installierten nativen Nodes stellen Structure-, Shape- und Texture-Stufen sowie UV-Unwrap, PBR-Bake und GLB-Export bereit. Dieser Repack ist eine konvertierte/quantisierte Ausführung und kein byteidentischer Microsoft-BF16-Lauf. [Comfy-Org-Modellkarte](https://huggingface.co/Comfy-Org/TRELLIS.2), [native Nodes](https://github.com/Comfy-Org/ComfyUI/blob/master/comfy_extras/nodes_trellis2.py).

## Gezielte Modelldateien: geladen und geprüft

Alle Dateien stammen aus `Comfy-Org/TRELLIS.2` und liegen unter `E:/Comfy-Desktop/ComfyUI-Shared/models/`. Die Größen und SHA-Werte wurden zuerst aus den öffentlichen Dateimetadaten gelesen; anschließend wurden die tatsächlich geladenen Dateien vollständig gehasht. [Dateiliste des Herausgebers](https://huggingface.co/Comfy-Org/TRELLIS.2/tree/main).

| Relativer Pfad | Exakte Bytezahl | SHA-256 |
| --- | ---: | --- |
| `diffusion_models/trellis_2_int8_convrot.safetensors` | 5.253.048.192 | `d01952ad137213f6a868f86b6b877026276f84af5eec23069217475a0bad3a31` |
| `vae/trellis_2_shape_vae_bf16.safetensors` | 1.095.844.024 | `de0cb4949a76c59ee5c091a995a69bcc8c51d5aeda939f0c641a50d2a72341f4` |
| `vae/trellis_2_texture_vae_bf16.safetensors` | 948.461.364 | `714e5ebf094a610e12a8e3b5175c18a62f37f6ea4218acb6073644456b73ab0e` |
| `clip_vision/dino_v3_vit_l.safetensors` | 1.212.559.776 | `5cb785e458de7c460579082418af81f5c62380c181599344bdc60898c63468ee` |
| **Summe** | **8.509.913.356** | ungefähr7,93 GiB |

Der Downloader verwendet `.part` und HTTP-Range zur Wiederaufnahme, prüft vor dem finalen Umbenennen Größe und Hash und ersetzt keine vorhandene abweichende Finaldatei. Es wurde kein Cloudkonto verwendet und kein Schlüssel gesucht. Abschlussmarker am16. Oktober,09:14:17 Berliner Zeit: `TRELLIS_MINIMAL_DOWNLOAD_PASS 4 files 8509913356 bytes; no inference`. Das anfängliche Planmanifest behält `downloaded:false`; der tatsächliche Abschluss steht separat in `download-progress.json`.

Für Scratchdateien, Texturbake, Output und mögliche Teilergebnisse ist zusätzlicher Platz nötig.20 GB waren die gewählte Vorabreserve, kein veröffentlichter universeller Mindestwert. Die große Anzeige freier WSL-VHD-Kapazität ist kein zusätzlich verfügbarer physischer Speicher auf C:. Der unten angegebene VRAM-Wert ist ein alle fünf Sekunden gemessener Gesamtkartenwert; eine Host-RAM-Spitze wurde bei diesem ersten Lauf nicht aufgezeichnet.

## Ein HQ mit echten PBR-Texturen: ausgeführter Ablauf

Die vorhandene offizielle Comfy-Vorlage enthält sowohl Pixal3D als auch TRELLIS.2 und startet standardmäßig im Pixal3D-Zweig mit hoher Cascade-Auflösung. Sie wurde nicht ungeprüft ausgeführt. Der eigene kleinere API-Graph verwendet ausschließlich die TRELLIS.2-Stufen, eine Probe und512er-Geometrie; MoGe, NAF-Zusatzmodell, Hintergrundentferner und Upsample-Cascade fehlen bewusst in diesem Testgraph. [Vorlage des Herausgebers](https://github.com/Comfy-Org/workflow_templates/blob/main/templates/3d_pixal3d_trellis2_image_to_model.json).

1. Root hat aus der Nutzerreferenz ein isoliertes HQ-Bild vorbereitet: `Art/Concepts/GeneratedV8/AegisHQ-TrellisInput-v1.png`,1536×1024RGBA, Alpha0…254. SHA-256: `c28f14775d447077c6a32b20bddc9a344cab4353b719966c190750f750ed020e`. Das Bild ist eine generierte Arbeitsreferenz, kein Spielnachweis. Es wurde unverändert in den eigenen Comfy-Eingabeordner kopiert.
2. `LoadImage` liefert eine inverse Alphamaske. `InvertMask` stellt die positive Objektmaske her; `ImageCropToMask` schneidet/paddet quadratisch auf512×512, mit schwarzem Hintergrund und `pad_factor=1`. Der transparente Hintergrund wird dadurch nicht als auszufüllende Gebäudeform konditioniert.
3. DINOv3 → Structure-Sampling → Structure-VAE mit32er-Koordinatengitter → Shape-Stufe mit512er-Decoding. Danach eine separate Texture-Sampling-/Texture-VAE-Stufe. Feste Seeds56/42/43 erlauben eine nachvollziehbare Probe.
4. Das Ergebnis wird zunächst auf maximal60.000 Zielflächen vereinfacht, auf1024er-UVs ausgelegt und aus dem Farbvoxel mit **BaseColor, Metallic und Roughness** in tatsächlich1024×1024 große Bilder gebacken. Derselbe UV-Mesh erhält diese Texturen und wird als GLB gespeichert. Dies ist keine reine Vertexfarbenausgabe. Normal-/AO-Bake sind im ersten Test nicht zwingend vorgeschaltet.

Der Graph enthält27 Nodes/22 native Typen. Vor der Ausführung bestätigte der eigene Server über `GET /object_info/...` alle benötigten Typen, Eingaben, Modellnamen und Verbindungen; beide Queues waren leer. Marker: `TRELLIS_NATIVE_SCHEMA_PASS`. Erst danach wurde mit ausdrücklichem GPU-Go genau ein `POST /prompt` gesendet. Die vollständige History meldet `success`, mit leerer Liste gecachter Nodes; es handelt sich um einen tatsächlich ausgeführten Lauf.

## Tatsächliches Ergebnis und Messgrenzen

| Punkt | Beobachtet |
| --- | --- |
| Prompt-ID | `03754b72-3374-40bf-a4ed-e2112d023fa5` |
| Lauf | 16. Oktober 2026, ungefähr09:27:41–09:28:07 Berliner Zeit; Server-Ausführungszeit26,297 s, Monitorabschluss nach30,67 s |
| Wiederholungen | Eine Einreichung, kein Retry und keine weitere Inferenz |
| GPU | Beobachteter Gesamtkarten-Peak10.671 MiB bei Fünf-Sekunden-Abtastung; kein exakter CUDA-Allokationspeak und keine allgemeine16-GB-Garantie |
| GLB | `Art/Models/TrellisV8-Candidate/AegisHQ-Trellis512_00001_.glb`,2.722.472 Byte; SHA-256 `fb53b1c581598eb06507375ffdaa0424b00b0844597fb5ff2cb11237fcd44f28` |
| Geometrie | Ein Mesh/Primitive,18.456 Dreiecke,20.682 UV-Vertices; endliche Positionen/UVs und gültige Indexbereiche geprüft |
| Texturen | Zwei tatsächlich1024×1024 große eingebettete PNGs: BaseColor und glTF-Metallic/Roughness. Beide sind im PBR-Material über UV0 gebunden. PNG-Bytes wurden unverändert separat ausgegeben. |
| Exportgrenzen | Kein `NORMAL`-Attribut; Normalen müssen für eine Nutzung berechnet werden. Keine eigene Normal-/AO-/Emissive-Map, keine Animation oder Gameplay-Sockets. |
| Koordinaten | Normalisierte glTF-Koordinaten, Y nach oben; Y-Minimum ungefähr−0,29682 statt Bodenpivot0. Weltmaßstab und Bodenauflage sind noch nicht hergestellt. |
| GPU-Freigabe | Nach erfolgreichem Abschluss beide eigenen Queues leer; Modelle über den geprüften `/free`-Endpunkt entladen. Danach1.041 MiB Gesamtkartenbelegung gemessen. Der vorhandene Server8188 blieb unberührt. |

`Saved/Research/TrellisV8/candidate-verification.json` verankert Eingabe-, Workflow-, Modell-, GLB- und Texturhashes sowie Materialbindungen, Bounds, Abmessungen und Laufzeit. Derselbe Prüfbericht und der verwendete API-Workflow liegen neben dem Kandidaten. `prompt-history.json` bleibt als vollständiger Rohbeleg unter Saved. Dies ist keine visuelle Abnahme und kein Beleg für Gleichwertigkeit mit der Konzeptzeichnung; eine Mehransichtenprüfung erfolgt separat.

## Server und Wiederaufnahme

Der eigene Server läuft auf `http://127.0.0.1:8189`, PID15460 zum Start, mit tasklokalen Input-/Output-/User-Verzeichnissen und deaktivierten Custom-Nodes. Ein vorhandener Benutzerserver auf8188 wurde nicht angesprochen oder beendet. `--lowvram` und2 GB Reserve sind gesetzt; bei aktiviertem DynamicVRAM steuert allerdings dessen Speichermanagement das Offloading. Das Flag allein ist kein Nachweis einer bestimmten VRAM-Spitze.

Unter `Saved/Research/TrellisV8/` liegen:

- `windows-host.json`, Quellen-/Modellmetadaten und `official-source-manifest.json`;
- `native-minimal-model-manifest.json`, `download_minimal_models.py`, `download-progress.json`, `download-process.json`, `model-download.log`;
- `comfy-server-process.json`, `comfy-model-paths.yaml`, `comfy8189.log` und `.stderr.log`;
- `prepare_hq_workflow.py`, `hq-trellis512-api.json`, `workflow-preparation.json`;
- `validate_native_workflow.py`, `native-workflow-schemas.json`, `native-workflow-schema-validation.json`.
- `run_one_hq_job.py`, `prompt-request.json`, `prompt-response.json`, `prompt-history.json`, `job-status.json`, `job-monitor-process.json`, `job-monitor.log`;
- `verify_export_candidate.py`, `candidate-verification.json`, `candidate-gltf.json` und `gpu-release.json`.

Vor einem späteren Lauf PID/Port und GPU-Belegung prüfen, statt einen zweiten Server zu starten. `run_one_hq_job.py --submit` verweigert eine erneute Einreichung, sobald bereits `prompt-response.json` existiert; ohne `--submit` überwacht es die gespeicherte ID. Dieser bereits erfolgreiche Prompt darf bei einer Wiederaufnahme nicht als neue Arbeit doppelt eingereiht werden. Weitere Proben benötigen einen eigenen Belegordner und GPU-Koordination mit Root. Bei Speicherfehlern keine automatische Wiederholung mit höheren Auflösungen.

Beim Start migrierte ComfyUI unerwartet eine alte `user/comfyui.db` trotz des eigenen User-Verzeichnisses. Der ursprüngliche Pfad wurde sofort aus dem erhaltenen `.bak` byteidentisch wiederhergestellt; SHA-Prüfung und Ablauf stehen in `legacy-db-restoration.json`. Der eigene Server verwendet die tasklokale Kopie. Diese Nebenwirkung darf bei späterem Neustart nicht unbemerkt wiederholt werden.

## Visuelle und technische Abnahme nach dem Pilot

Ein erfolgreicher Generatorlauf liefert einen Assetkandidaten. Bei einem einzelnen Bild werden verdeckte Seiten erfunden; besonders Gebäuderückseite, Rohre und Eingänge müssen visuell geprüft werden. Danach Maßstab, Orientierung, Bodenauflage, Materialkanäle, UV-Seams und vereinfachte Kollision/Navigation in Blender/Unreal an die bestehenden HeroHour-Verträge anpassen. Emissive-Fenster und bewegliche Komponenten entstehen nicht automatisch als spielbare Funktion. Erst eine tatsächliche Unreal-Aufnahme neben derselben Nutzerreferenz entscheidet, ob das Ergebnis die bisher zu einfachen Formen sinnvoll übertrifft.
