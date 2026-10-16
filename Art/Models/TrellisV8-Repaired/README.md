# Eigener Aegis-HQ-Kandidat: reparierte TRELLIS.2-Pipeline

Der GLB ist ein tatsächliches texturiertes 3D-Modell, kein Bild-Proxy. Originale eigene Konzeptableitung: `Art/Concepts/GeneratedV8/AegisHQ-TrellisInput-v1.png`; gleiche Eingabe und Seeds wie erster Versuch, Remesh512 vor Vereinfachung, neue UVs und frisches ursprüngliches Voxel-PBR-Baking. Keine extrahierten Tempest-Modelle oder -Texturen.

- GLB: 699.820 Indexdreiecke, 495.217 UV-Vertices mit NORMAL-Attributen. Blender importiert 699.814 Dreiecke; sechs entfallen im Import, genaue Ursache nicht nachgewiesen.
- `BaseColor.png`: unveränderte eingebettete 2048²-sRGB-Textur.
- `MetallicRoughness.png`: unveränderte eingebettete 2048²-Textur, Non-Color; **G = Roughness, B = Metallic, R ungenutzt**. Nicht fälschlich R als AO verwenden.
- Kein eigener Normalmap-, Emissions- oder AO-Kanal. Blaue Fenster sind zunächst BaseColor.
- Drei PBR- und zwei Wireframe-Ansichten tatsächlich gerendert und geprüft. Lesbare Schüssel/Fassaden/Serviceflügel auch auf Rückseite; feine Kanten sind durch Remesh-Glättung weicher als RAW.
- Noch normalisierter Maßstab/Pivot; kein Unreal-Import, keine Kollision, Navigation, LOD-Abnahme, Fraktions-/Schadensmaterialintegration oder Paketabnahme behauptet.

`verification.json` enthält Quell-/Textur-SHAs. `generation-workflow-api.json` ist der tatsächlich erfolgreich ausgeführte Workflow. Vollständige begrenzte Diagnose: `docs/research/TrellisHQV8-Diagnosis.md` und kompakte Belege unter `docs/verification/2026-10-16/reference-v8/trellis/`.
