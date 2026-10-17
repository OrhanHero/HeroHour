# Reference Ember HQ V7

This original building follows the graphite HQ in the left half of the supplied `Codex-Bild 1. Sept. 2026, 01_22_20.png`. Its silhouette is independently authored: a sloping central command hall, a deep forward portal, asymmetric low service wings, a low eight-sided command crown and two slender antenna blades. It does not reuse the Aegis building or its roof dish, and it does not use the previous circular fortress core.

Graphite armor and green equipment accents follow the concept. The mesh import stage configures `M_RV7_Ember_TeamGreen` (Tint .025/.32/.13), `M_RV7_Ember_NeonGreen` (Tint .025/.70/.28) and `M_RV7_Ember_GlassGreen` (Tint .012/.055/.032) under `/Game/HERO/ReferenceV7/Materials`. These intermediate materials deliberately bypass runtime `M_CV2` surface replacement. The final surface stage replaces them with the model-local `M_RV7_Baked_Ember_HQ` material, whose Albedo/ORM/EmissionMask atlas preserves that graphite/green design with neutral white Tint.

The nine legacy Ember roles also receive green Team/Neon markings at runtime, giving the equipment a consistent faction palette alongside the reference HQ and APC. Their geometry remains unchanged. Enemy health bars, selection indicators and gameplay identification keep their gameplay colors. The thirteen refreshed portraits cover both the four reference models and those nine legacy Ember roles; the other nine Aegis PNGs and metadata records remain unchanged.

## Reproduce and integrate

1. Run Blender 5.2 in background with `Scripts/art/build_reference_ember_hq_v7.py`.
2. Inspect the actual exported-mesh render `Art/Models/ReferenceEmberHQV7/SM_RV7_EmberHQ.png`.
3. Root runs `Scripts/art/import_reference_ember_hq_v7.py` through Unreal Python. It imports only `/Game/HERO/ReferenceV7/Meshes/SM_RV7_EmberHQ`, idempotently configures three green material instances, and changes only `/Game/HERO/DataV4/D_Ember_HQ.static_mesh` on unit definitions. Statistics and icon identity are checked unchanged.
4. After matching source geometry is final, run `Scripts/art/bake_reference_surfaces_v7.py -- --only Ember_HQ`, then `Scripts/art/import_reference_surfaces_v7.py` through Unreal Python after the mesh imports. HQ atlas resolution is 2048². The surface importer checks source/FBX/texture hashes and binds the final baked material to all HQ slots. Rebuilding the geometry invalidates its atlas; rerunning the mesh importer requires reapplying the final surface stage.
5. Run `Scripts/art/render_reference_portraits_v7.py`, then `Scripts/art/import_reference_portraits_v7.py`. They refresh thirteen existing 512×512 RGBA portraits: four reference-model portraits from their matching baked atlases and nine legacy Ember portraits using the green faction PBR palette. Source blends are read only; all nine other Aegis PNG hashes and metadata records must remain unchanged. Expected markers are `REFERENCE_PORTRAITS_V7_COMPLETE 13; remaining 9 Aegis images and records unchanged` and `REFERENCE_PORTRAITS_V7_IMPORT_PASS 13`.
6. Root updates architecture expectations and inspects an actual Ember runtime capture after packaging. Studio imagery alone does not establish runtime concept fidelity or package acceptance.

## Geometry and evidence

The export has 27,472 triangles and a 7.50 x 7.60m source footprint (11.25 x 11.40m at runtime scale 1.5). The source front is Blender +Y, which imports as Unreal -Y toward the standard starting camera. Geometry is authored in that orientation rather than relying on a special actor rotation. Source minimum Z is -110cm; runtime actor Z110cm plus mesh relative Z55cm and scale 1.5 produce ground Z0. The source footprint is constrained to at most 8m per axis so runtime bounds fit the existing 12m HQ footprint. Exact bounds, FBX SHA256 and provenance are in the adjacent JSON manifest.

The final 2026-10-16 revision replaces the large empty hall walls with fitted mechanical armor fields, captive fasteners and inset green service lines. The low command crown and its windows follow the actual faceted wall outline; a fitted graphite insert breaks up the large portal lintel. The actual final green render was inspected against the supplied reference. The broad architecture and green identity are closer, while clean surface wear and simplified individual panel/pipe placement still differ from the concept.

The script imports the exported FBX back into Blender and compares actual world bounds against the source to a 1mm tolerance. The Unreal importer independently checks expected mirrored-Y bounds, ground compensation and original icon/statistics. Blender completion marker is `REFERENCE_EMBER_HQ_V7_PASS`; Unreal completion marker is `REFERENCE_EMBER_HQ_V7_IMPORT_PASS`. The Blender export is limited to 35,000 triangles.

## Limits

This is a new reference-shaped prototype asset with original UV-baked surface atlases, not finished production art. The geometry-derived and procedural variation is not a complete normal-map detail pass and does not guarantee convincing wear at RTS distance. Geometry is static and joined; antenna animation, destruction stages and a hand-authored LOD chain remain absent. The portal is a real geometric recess with a shutter at the rear, not a traversable interior. Individual panel and pipe placement remains a simplification of the supplied concept. No Tempest Rising or other commercial game models were extracted or copied. The original concepts and source blends remain unchanged by the bake and portrait stages.
