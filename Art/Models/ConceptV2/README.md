# HERO – Konzeptset V2

Fünf eigene Static-Mesh-Assets, erstellt aus der grünen HERO-Basisreferenz `Art/Concepts/CityHero/Codex-Bild 1. Sept. 2026, 01_22_20.png`. Die Formen sind eigenständig modelliert; es wurden keine Tempest-Rising- oder C&C-Spielassets extrahiert oder übernommen.

| Asset | Funktion im Prototyp | Dreiecke | Abmessungen in Metern |
| --- | --- | ---: | --- |
| SM_HERO_CommandHub | Hauptquartier | 36.692 | 6,89 × 6,89 × 5,95 |
| SM_HERO_VehiclePlant | Fahrzeugfabrik | 27.080 | 4,40 × 4,05 × 3,33 |
| SM_HERO_Refinery | Depot / Ressourcenabgabe | 34.512 | 3,57 × 3,82 × 3,90 |
| SM_HERO_Striker | Kampffahrzeug mit acht Rädern | 28.716 | 3,51 × 1,88 × 2,26 |
| SM_HERO_Harvester | Sammler mit sechs Rädern, Frontfräse, Tank und Entladekupplung | 36.884 | 3,885 × 2,018 × 2,036 |

Zu jedem Modell gehören `.blend`, `.fbx` und `.json` mit Meshdaten. Materialien, UVs, Chamfers, Lüfter, Rohrleitungen, Panzerplatten und Leuchtstreifen sind enthalten. Unreal verwendet zehn gemeinsam genutzte Materialien unter `/Game/HERO/ConceptV2/Materials`; Oberflächenfarbe und Rauheit variieren prozedural. Team-/Neon-Slots lassen sich zur Laufzeit umfärben. Noch nicht enthalten sind finale gebackene Textursets, LOD-Ketten oder bewegliche Räder/Türme/Tore.

Die FBX-Dateien enthalten explizite Glättungsgruppen. Der Import prüft die tatsächliche Umrechnung von Metern in Zentimeter sowie die Materialzuordnung. Importbericht: `Saved/Research/concept-assets-import.json`. Laufzeit-Z-Kompensation: HQ −1,10 m, andere Modelle −0,35 m. Gebäude zeigen nach −Y, das Fahrzeug nach +X.

Erstellen: Blender 5.2 im Hintergrund mit den `Scripts/art/build_concept_*.py`-Skripten aufrufen. Gemeinsame Hilfen: `Scripts/art/concept_asset_common.py`. `Scripts/art/import_concept_assets.py` importiert das Set in Unreal. `Scripts/art/render_concept_asset_set.py` erstellt eine gemeinsame tatsächliche Blender-Szene und die Render-Vorschau `Art/Previews/ConceptV2/AssetSet.png`.

Die Vorschau ist ein Render der gelieferten Modelle. Sie ist kein Screenshot aus Unreal und kein Ersatz für finale Konzeptqualität. Die fünf Modelle sind in die entsprechenden Gameplayrollen eingebunden; ältere Assets bleiben als Quellen und Fallback erhalten.
