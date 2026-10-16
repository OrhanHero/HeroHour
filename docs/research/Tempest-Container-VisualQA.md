# Echter Container: Geometrie- und Bildpruefung

17. Oktober 2026 (Berlin). Blender 5.2.0, CPU-Render, 24 Samples, 1024 x 768.
`Scripts/research/verify_tempest_container_blender.py` importierte den tatsaechlichen
CUE4Parse-GLB-Export und den nativen UE5.4-FBX-Export getrennt. Kein Unreal-Job
gestartet und keine Quelldatei veraendert. Laufmarker
`TEMPEST_CONTAINER_BLENDER_PASS`, Prozess Exit 0.

## Messwerte

| Merkmal | CUE4Parse GLB | Nativer UE5.4 FBX |
|---|---:|---:|
| Dreiecke | 1463 | 1463 |
| Meshvertices im Blenderimport | 1579 | 930 |
| Materialslots | 1 | 1 |
| UV-Layer | 8 | 1 (`LightMapUV`) |
| Objektmassstab | 1 | 0.01 |
| Weltabmessungen in Metern | 7.0123019 x 3.0935581 x 2.9915159 | identisch |

Beide Imports belegen exakt gleiche Weltbounds und Dreieckszahl. FBX konvertiert
Zentimeter mit Objektmassstab 0.01; GLB liefert bereits Meter. Unterschiedliche
Vertexzahlen und UV-Layer zeigen unterschiedliche Export-/Importrepraesentation.
Es wird keine bitgleiche Geometrie oder vollstaendige UV-Paritaet behauptet.
Die Enginebounds in Zentimetern sind entsprechend mit Faktor 100 zu vergleichen.

## Tatsaechliches Bild

Persoenlich mit `view_image` geprueft:
`Saved/Research/TempestAssetTransfer/VisualQA/20261016T234157Z-78e2d938/Container-OriginalAlbedo-Baseline.png`.
Das Bild zeigt einen vollstaendigen grauen GDF-Container mit gerippten Seiten,
zwei oberen Kreuzmarkierungen, mittigem Dachluefter, Griffen, gelb-schwarzen
Warnstreifen und GDF-Zeichen. Keine sichtbar fehlende Geometrie, keine offensichtlich
verzogene Albedozuordnung in der geprueften Dreiviertelansicht.

Dies ist eine **Original-Albedo-Baseline**, keine Tempest-Shaderrekonstruktion:
nur die echte exportierte `_D`-Textur ist zugewiesen, Roughness konstant 0.65,
Metallic 0. Keine Normalmap, RMO-Kanalzuordnung, Teamfarbmaske, Snow/Grunge-Logik
oder originalgetreue Tempest-Beleuchtung wird behauptet. Das Preview rendert den
GLB, nicht den FBX. Geometrie und Texturen bleiben unter Saved und werden nicht
als HeroHour-Originalassets oder neue KI-Modelle bezeichnet.

Vollstaendiger Vergleich inklusive Eingabepfaden und UV-/Slotnamen:
`Saved/Research/TempestAssetTransfer/VisualQA/20261016T234157Z-78e2d938/Container-Blender-Verification.json`.
