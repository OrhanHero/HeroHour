# Reference APC V7 — original Aegis 8×8

## Evidence and decisions

Viewed the actual right-hand tan armoured vehicle in `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png`, the battlefield/UI illustration `01_22_00.png`, and `Art/Previews/ConceptV6-Combat.png` before modelling.

The old Striker reads as a short cabbed utility vehicle because of its high windscreen, thin side rail and conical turret. This asset replaces the entire mesh with a continuous low armoured hull, broad sloped glacis, narrow driver prisms, eight large independently visible tires, actual boolean recessed wheel cavities, angular turret cheeks and bustle, autocannon, rear troop ramp and restrained blue identifiers. The tan/steel/blue studio palette follows the right-hand concept. Material slots keep existing `Armor`, `Steel`, `Dark`, `Rubber`, `Glass`, `Team`, `Lamp` semantics; Unreal binds existing M_CV2 parents and runtime applies the Aegis materials.

## Files and reproduction

- `Scripts/art/build_reference_apc_v7.py`: original procedural Blender source, no extracted or downloaded models.
- `Art/Models/ReferenceAPCV7/SM_HR_Aegis_APC8.blend`, `.fbx`, `.json`, `.png`: editable source, game mesh, validation metadata and actual Cycles render.
- `Scripts/art/import_reference_apc_v7.py`: Unreal import to `/Game/HERO/ReferenceV7/Meshes/SM_HR_Aegis_APC8`; changes only `D_Aegis_Combat.static_mesh`, checks unchanged stats/icon.

Run Blender 5.2 in background with `--python Scripts/art/build_reference_apc_v7.py`; then run the importer through Unreal Python. No map or gameplay source is changed by either script. Portrait remains separately managed, so integration must render a matching new icon before visual acceptance.

## Validation and bounds

18,914 triangles; 370.900 × 187.925 × 203.142 cm, forward +X, wheel axle Y. Width stays within the old 188 cm visual envelope; no selection, logical radius, navigation or gameplay values changed. Tire tread minimum Z is -35 cm to match the runtime actor origin +35 cm. FBX is independently reimported and all axis extrema compared within 2 mm; symmetric Y bounds explicitly checked against export reflection. JSON records final bounds and PASS. Blender preview is an actual render of this exported geometry, not generated concept art.

## Remaining limits

This establishes the primary silhouette rather than final production art. Panels are deliberately readable at RTS scale; it still needs authored weathering/normal maps, animated wheels/turret, LODs and faction-specific Ember geometry. Wheels are disconnected closed geometry inside one static mesh. The old combat actor uses its own aim/effect logic, so the static cannon is not yet a separately rotating turret. Match the final Unreal capture against the concept before treating this asset as accepted.
