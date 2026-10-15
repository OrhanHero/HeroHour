# Faction surfaces V5

15.10.2026. Original HeroHour assets only. This pass creates 20 independent master materials; it does not modify ConceptV2 materials, meshes, maps or source game files.

## Import

Execute `Scripts/art/polish_faction_materials_v5.py` in Unreal Python after the existing ConceptV2 and DistrictV3 imports. The script validates all dependencies before duplication, uses DistrictV3 albedo/roughness/normal maps with asserted colour spaces, recompiles each new material, enables skeletal-mesh usage and forces package saves. It emits `Saved/Research/faction-surfaces-v5.json` and `FACTION_SURFACES_V5_PASS 20` after completion. A rerun updates only its own PresentationV5 destination.

## Runtime mapping

Destination: `/Game/HERO/PresentationV5/Materials/M_HR_{Aegis|Ember}_{surface}`.

Map source material **name**, not slot index. Apply equally to all eleven roles and to infantry's skeletal material overrides. Slot counts and order differ between meshes. The report contains the complete faction/source-path mapping.

| Source name | Aegis destination suffix | Ember destination suffix |
|---|---|---|
| M_CV2_Armor | Aegis_Armor | Ember_Armor |
| M_CV2_Steel | Aegis_Steel | Ember_Steel |
| M_CV2_Dark | Aegis_Dark | Ember_Dark |
| M_CV2_Rubber | Aegis_Rubber | Ember_Rubber |
| M_CV2_Team | Aegis_Team | Ember_Team |
| M_CV2_Glass | Aegis_Glass | Ember_Glass |
| M_CV2_Neon | Aegis_Neon | Ember_Neon |
| M_CV2_Concrete | Aegis_Concrete | Ember_Concrete |
| M_CV2_Dust | Aegis_Dust | Ember_Dust |
| M_CV2_Lamp | Aegis_Lamp | Ember_Lamp |

All names above have prefix `M_HR_` at the destination. Every material retains the vector parameter `Tint` in **linear colour**. Preserve its default on Armor/Steel/etc.; team overrides should target Team/Neon only. Applying a single faction tint to every material would erase the intended contrast.

## Visual decisions

Aegis uses light beige painted armor, cold exposed steel and blue identification panels. Ember uses graphite armor, neutral steel and warm red identification panels. Dark recesses/rubber stay distinct; glass is opaque reflective prototype glass; neon/lamp provide restrained emission. Painted armor remains mostly dielectric, exposed steel is metallic. PBR detail is normalized against measured source means so the original metal texture does not blacken armor.

The graphs retain existing normalized atlas UVs, including animated infantry. This avoids world-projection swimming on moving units. Detail tiling is dimensionless; consistent physical texel density and baked edge wear require future UV work. These materials change surface readability, not silhouettes or building detail.

## Verification status

Python AST syntax validation passed. Unreal execution, fresh-process shader compilation and real packaged screenshots remain root integration responsibilities; this document does not claim those checks have passed before import. The import JSON records graph connections, paths, linear tints and PBR values for each variant.
