# Aegis-Fabrik V9: tatsächlicher 3D-Kandidat

## Durchgeführter Lauf

Ein einziger freigegebener TRELLIS.2-Lauf verwendete die neue eigene Fabrikvorlage und feste Seeds 56/42/43. Der Ablauf übernimmt die reparierte HQ-Pipeline: Shape-Decoding, Remesh 512, Ziel 700.000 Faces, neue UVs in 2048, frisches Backen aus denselben Textur-VAE-Voxelfarben und Normalenberechnung **nach** der endgültigen UV-Topologie. Die defekte direkte frühe Decimation wird nicht verwendet.

Vor der Submission wurden die tatsächlich laufenden Node-Schemas abgefragt und ohne Fehler validiert. Die native ThresholdMask mit 0,5 entfernt den weichen Alpha-Saum. Die tatsächlich vom Graph gespeicherte `Conditioning512-Executed.png` wurde geöffnet: vollständige Fabrik, schwarzer Hintergrund, kein grauer Halo oder zusätzliches Gelände.

Der Server meldet **50,38 Sekunden** für den Prompt. Gemessener Peak des gesamten GPU-Speichers: **8.866 MiB**. Der Lauf war auf 600 Sekunden begrenzt, ohne Wiederholung oder weitere Modelldownloads. Er nutzte einen eigenen Server an 127.0.0.1:8189 und eine eigene SQLite-Datenbank. Der Server wurde anschließend beendet; die vorhandene Comfy-User-Datenbank blieb SHA-identisch. Port 8188 wurde nicht verwendet.

## Ergebnis und tatsächlich betrachtete Ansichten

Unter `Art/Models/TrellisFactoryV9-Candidate/` liegen das unveränderte GLB, seine originalen eingebetteten PBR-Bildbytes, der ausgeführte Workflow, die Konditionierung und fünf tatsächliche Blender-Aufnahmen. GLB-SHA: `d26273577175ed8a556de00d73fe7082dba07d695329f272e0bb6ad2e01151a9`.

- GLB: **699.468 Dreiecke**, 485.694 UV-Vertices, passende NORMAL-/TEXCOORD-Attribute. Blender importiert 699.446 Dreiecke; 22 degenerierte Dreiecke werden vom Importer ausgelassen. Diese Differenz wird ausdrücklich dokumentiert.
- BaseColor und MetallicRoughness: jeweils echte **2048×2048** PNGs. Im tatsächlichen Blender-Material ist BaseColor sRGB, MetallicRoughness Non-Color. Es existiert weder eine Normalmap noch eine separate Emissionsmap.
- Echte 3D-Ausdehnung: ungefähr 1,004×0,789×0,327 normalisierte Blender-Einheiten. Das ist kein bereits auf Unreal-Metermaß eingerichtetes Gebäude.
- `Preview-PBR-A/B/C.png` und `Preview-Wire-A/B.png` wurden alle geöffnet. Das Modell ist rundum volumetrisch, besitzt sichtbare seitliche und rückwärtige Baugruppen und keine flache Bildtafel. Die Quell-GLB-Datei und importierte Geometrie wurden durch die Inspektion nicht geändert.

## Qualitätsurteil

Die breite Torhalle, der niedrigere Serviceflügel, die schrägen Panzerstützen und die weiß-blaue Farbgebung liegen deutlich näher an der gelieferten Aegis-Bausprache als die alte gleichförmig dunkle Fabrik. Das klare Funktionsbild bleibt auch auf kleinerer Darstellung erkennbar. Das Modell besitzt keine HQ-Schüssel und keinen Kommandoturm.

Die unbekannte Rückseite wurde mit einem zweiten großen Tor und ähnlichen Frontmerkmalen ergänzt. Das kann als Durchfahrhalle funktionieren, ist aber eine generative Interpretation und keine belegte Konzeptansicht. Die Dachlüfter wirken teilweise weich und verklumpt; hier sind manuelle, sauber konstruierte Ersatzteile sinnvoll. Feine Oberflächenmerkmale sind gegenüber dem 2D-Input verwaschen. Ein einzelner Texturatlas erzeugt noch keine sauber getrennten animierbaren Tore oder Lüfter.

**Als Highpoly-Quelle geeignet, noch kein freigegebenes Spielasset.** Vor dem Einsatz fehlen mindestens bereinigte Lüfter, zweckmäßige Spiel-LODs, Maßstab und Bodenkontakt, eine eindeutige Vorwärtsachse, kontrollierte Material-/Schadensbindungen sowie tatsächliche Unreal- und Produktionsprüfung. Diese Aufgabe führte keinen Unreal-Import und keinen Ersatz der bisherigen Fabrik durch.

Kompakte maschinenlesbare Belege: `docs/verification/2026-10-16/factory-v9/`. Sämtliche Ausgabedateien und Hashes stehen im `artifact-manifest.json` des Kandidaten. Vollständige lokale Server-/Renderlogs bleiben unter `Saved/Research/TrellisFactoryV9/`.
