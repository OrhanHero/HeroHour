# ConceptV2 – Materialüberarbeitung V3

Stand: 14. Oktober 2026. Die Überarbeitung betrifft die vorhandenen eigenen HeroHour-Materialien unter `/Game/HERO/ConceptV2/Materials/M_CV2_*`. Das Skript ist `Scripts/art/polish_concept_materials_v3.py`.

Der vorherige Importer mischt die Grundfarbe und Rauheit mit einem hochfrequenten Noise-Knoten. Das lässt die Gebäude in der nativen Spielansicht körnig und steinartig erscheinen. V3 ersetzt diese Knoten durch zurückhaltende PBR-Texturvariation und hellere, matte Lackoberflächen. Es verwendet ausschließlich die vorhandenen eigenen DistrictV3-Texturen.

| Oberfläche | Tint, linear RGB | Metallic | Rauheit | Eigene Texturbasis |
| --- | --- | ---: | ---: | --- |
| Armor | 0.16 / 0.20 / 0.21 | 0.12 | 0.70 | Metal |
| Steel | 0.27 / 0.30 / 0.32 | 0.80 | 0.44 | Metal |
| Dark | 0.06 / 0.075 / 0.08 | 0.05 | 0.76 | Metal |
| Concrete | 0.30 / 0.31 / 0.29 | 0.00 | 0.84 | Concrete |
| Dust | 0.30 / 0.285 / 0.23 | 0.00 | 0.88 | CompactedDirt |
| Rubber | 0.025 / 0.03 / 0.032 | 0.00 | 0.88 | Plaster |
| Team | 0.035 / 0.24 / 0.15 | 0.05 | 0.70 | Metal |

`Glass`, `Neon` und `Lamp` behalten ihre bisherigen Farb-, Metallic-, Rauheits- und Emissionswerte. Der Vector-Parameter `Tint` bleibt für alle Materialien erhalten. Die C++-Zuordnung der Slots `Team` und `Neon` und ihre dynamischen Teamfarben funktionieren damit weiterhin. Der Emissionsfaktor bleibt 2 für Neon und 1 für Lamp.

Die Albedo wird nach ihrer sRGB-Decodierung relativ zum dokumentierten Mittelwert normalisiert und nur schwach mit `Tint` multipliziert. Dadurch verdunkelt die dunkle Metal-Quelltextur den gewünschten Lack nicht erneut. Die Rauheitsvariation ist auf wenige Prozentpunkte reduziert; Normalmaps werden zu 4–12 Prozent mit der flachen Tangentennormale gemischt und normalisiert. Alle drei Texturrollen verwenden explizite passende Sampler: Color, Masks und Normal.

Die ConceptV2-Exporte verwenden laut `concept_asset_common.py` normalisierte Smart-Project-UVs. Sie besitzen keine metrische Cube-Projektion. Das Skript nutzt deshalb die vorhandenen UVs mit einem dimensionslosen Detailfaktor von 1,5 oder 2. Eine feste Zentimetergröße pro Texel wird damit nicht behauptet. Gleichmäßige UV-Dichte, gebackene Kantenabnutzung, Schmutzmasken und neue Geometrie gehören in einen späteren Assetdurchgang.

Ausführung und Nachweis:

1. Zuerst müssen `import_concept_assets.py` und `import_district_v3.py` gelaufen sein. Alle Material- und Texturabhängigkeiten einschließlich Farb- und Kompressionseinstellungen werden vor der ersten Graphänderung geprüft.
2. `polish_concept_materials_v3.py` über den Unreal-Python-Commandlet ausführen. Es verändert ausschließlich die bestehenden ConceptV2-Materialgraphen und speichert einen Bericht unter `Saved/Research/concept-materials-v3.json`.
3. Erfolg erwartet die Logmarke `CONCEPT_MATERIALS_V3_PASS 10`. Jede Verbindung und jeder Speichervorgang wird geprüft; falsche Eingangsbezeichnungen dürfen den Vorgang nicht stillschweigend passieren. Die verwendeten Eingänge sind unter anderem `UVs`, `VectorInput` sowie `A`, `B` und `Alpha`.
4. Danach Shader-Fehler im Commandlet- und Packaging-Log prüfen und die tatsächliche Spielansicht rendern. Der Skriptabschluss bestätigt Graphaufbau und Speicherung; die native visuelle Abnahme ist ein eigener Nachweis.

Zum Zeitpunkt der Erstellung wurden die Python-Syntax und Graphaufbau-Logik statisch geprüft. Unreal-Ausführung, Shaderkompilierung und Sichtprüfung übernimmt der integrierende Hauptprozess. Diese Überarbeitung ist eine Materialverbesserung der vorhandenen Geometrie und keine fertige Tempest-Rising-Grafikqualität.
