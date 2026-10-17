# Military Detail V6

Viewed the actual concept `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png`. It shows layered military armor, segmented roofs, mechanical towers, roof communications and exterior cooling equipment. Added original geometry to HeroHour's own ConceptV2 buildings; no original commercial game assets were read or extracted.

## Results

`Scripts/art/build_military_detail_v6.py` opens the three existing own `.blend` files, supplements their geometry, and exports new independent files under `Art/Models/MilitaryDetailV6`. Existing source files are never saved; SHA256 before/after must match.

* **CommandHub / HQ:** 36,692 to 51,016 triangles. 129 additional detail objects before final merge: independent armored side annexes, roof panels and heat exchangers, entry baffles, conduit protection, and offset concave communications dish with receiver/struts.
* **VehiclePlant / Factory:** 27,080 to 37,956 triangles. Six segmented roof cassettes with individual radiator blades, flank inspection panels and conduit, portal header segments, gantry control console and maintenance stairs.
* **Refinery / Depot:** 34,512 to 45,188 triangles. 77 additional detail objects: segmented processing armor, protected loading canopy, roof heat exchangers, auxiliary fluid buffers, visible sight glasses and rear control cabinets. Receiving mouth remains clear.

Exact triangle totals, original/updated bounds, source hashes, material semantics and FBX roundtrip results are in `Art/Models/MilitaryDetailV6/manifest.json`. New geometry is checked vertex by vertex against the original bounding box. Original vertices remain untouched, ensuring all six bounding faces, source pivot/runtime Z compensation, footprint and dimensions stay identical. Each output is a single static mesh; the additions do not create extra runtime components.

All three output `.blend` files remain editable. Three 768px `*-Preview.png` renders were generated and visually inspected. They are neutral original-source palette previews; actual Aegis/Ember colors are assigned by the existing runtime material system.

## Build and import

```powershell
& 'C:/Program Files/Blender Foundation/Blender 5.2/blender.exe' --background --python Scripts/art/build_military_detail_v6.py
```

Completed build marker: `MILITARY_DETAIL_V6_BUILD_PASS 3`; all three asset records report PASS, unchanged original SHA256 and successful FBX dimension roundtrip.

Root runs `Scripts/art/import_military_detail_v6.py` serially in Unreal. It imports only `/Game/HERO/MilitaryDetailV6/Meshes`, assigns existing `M_CV2_*` material parents by original semantic slot name, checks saved mesh bounds in centimetres, and changes **only `static_mesh`** on the six DataV4 definitions for Aegis/Ember HQ, Factory and Depot. It writes `Saved/Research/MilitaryDetailV6-Import.json` and logs `MILITARY_DETAIL_V6_IMPORT_PASS`.

Statistics, faction definitions, footprints, icons, original ConceptV2 source packages, map, and C++ are untouched by this agent. Re-running the DataV4 base definition generator would restore old mesh references, so run this detail importer after that generator if it is needed.

## Integration validation

Runtime presentation/architecture tests that assert old mesh paths need to select these new paths for the three roles while continuing to validate all other roles and CV2 semantic slots. Six V5 building portrait renders still depict the old versions until the separate portrait renderer is pointed at the new sources. Root must confirm new faction materials, packaged mesh loading and actual scene appearance after import; source export/preview success is not a claim of packaged validation.


## Import coordinate correction

The first Unreal integration attempt caught a genuine bounds-check defect: FBX uses Blender's right-handed frame while the Unreal import negates Y for this X-forward/Z-up export. HQ's symmetric AABB hid the issue; Factory's Blender minY=-205.1416cm/maxY=200cm correctly imports as minY=-200cm/maxY=205.1416cm. Refinery is asymmetric as well. The builder now records explicit `unreal_bounds_m` alongside unchanged Blender/original bounds; importer checks that transformed AABB with the original strict <1cm tolerance. No vertices, FBX files, source pivots or portrait sources were changed for this fix. The failure was a frame-conversion error in validation, not missing exported geometry.
