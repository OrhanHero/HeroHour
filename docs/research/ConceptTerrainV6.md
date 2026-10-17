# Concept Terrain V6

## Reference and intent

Viewed the actual supplied `HERO_CITYHERO_MAP_1.png`, `Codex-Bild 1. Sept. 2026, 01_22_00.png`, and the current `Art/Previews/PresentationV5-Aegis.png` before implementation. The concepts show a level playable desert valley enclosed by substantial eroded rock relief and narrow uneven river margins. V5 still shows a broad, flat road grid; this change addresses the surrounding geology and shoreline, not the town or road arrangement.

## Original assets

`Scripts/art/build_concept_terrain_v6.py` builds six original Blender/FBX meshes in `Art/Models/ConceptTerrainV6`:

| Asset | Triangles | Placement |
|---|---:|---|
| CanyonSouth | 8,640 | South of playable rectangle |
| CanyonNorth | 8,640 | North of playable rectangle |
| CanyonWest | 8,640 | West of playable rectangle |
| CanyonEast | 8,640 | East of playable rectangle |
| RiverMarginWest | 4,896 | Existing impassable river strip |
| RiverMarginEast | 4,896 | Existing impassable river strip |

Total: **44,352 triangles in six mesh components**. Four continuous canyon escarpments have irregular mesa crests, real geometric sediment shelves, drainage gullies, and approximately 18–30 metre relief. River margins descend from Z=-6cm to approximately -188cm and retain the existing river surface. The canyon gap also lets the river exit through the north and south edges.

No commercial game assets, external generation service, or extracted geometry is used. Deterministic procedural geometry is saved as editable `.blend` and `.fbx`; manifest contains bounds and measured FBX roundtrip dimensions. Every polygon has nonzero area; export checks all vertices against the safe zones.

## Authoritative constraints

Read `Source/HeroHourPrototype/BattlefieldLayout.h`, `build_terrain_slice_v3.py`, `build_environment_v2.py`, `polish_cityhero_environment.py`, and the V5 detail importer before authoring.

* Playable rectangle stays X=±12000cm, Y=±9000cm; all new canyon vertices are at least 50cm outside it.
* River is still X=-3000cm, half-width 700cm. New bank vertices remain inside X=-3685..-2315cm.
* Bridges remain Y=±3200cm. New bank geometry leaves approximately ±1050cm clear around each bridge centre.
* No new collision, navigation, build blocker, traversable bridge, height sampling, or river path changes.
* Six actors use serialized `NoCollision`, `NO_COLLISION`, and `can_ever_affect_navigation=False`.
* Import preserves all existing actor labels and changes no preexisting map actors. Only prior `CTV6 ` actors are removed when rerunning.

## Execution

Build (completed with six asset PASS records and final `CONCEPT_TERRAIN_V6_BUILD_PASS 6`):

```powershell
& 'C:/Program Files/Blender Foundation/Blender 5.2/blender.exe' --background --python Scripts/art/build_concept_terrain_v6.py
```

Root must execute `Scripts/art/polish_concept_world_v6.py` serially through Unreal's Python commandlet. It imports `/Game/HERO/ConceptTerrainV6/Meshes`, assigns the existing `/Game/HERO/DistrictV3/Materials/M_DV3_Sandstone`, places six `HERO_ConceptTerrainV6` tagged actors, saves CityHero, and writes `Saved/Research/ConceptTerrainV6-Import.json`. It does not alter V5 ground/asphalt materials. Require `CONCEPT_TERRAIN_V6_IMPORT_PASS` in the Unreal log; Blender export success alone does not establish map integration.

## Verification still required by integration owner

Fresh Unreal reload must confirm six saved actors have the NoCollision profile and no navigation relevance; capture an actual map overview and a bridge closeup. Run navigation, presentation and relevant packaged gameplay checks after integration. No claim is made here that the final scene has been captured or those runtime checks have passed. Large relief is outside the playable area and will be most visible while panning near the valley perimeter; it deliberately does not put nonblocking hills across a collector route to improve the initial base screenshot.

## Terrain shading review and correction

Inspected actual Unreal capture `Art/Previews/ConceptV6-ArtReview-Overview.png`. Broad canyon walls showed a regular faceted patchwork, which contradicted weathered sandstone references. Rendered the same original north wall twice in Blender with identical camera/light/material: `Art/Models/ConceptTerrainV6/CanyonNorth-NormalsBefore.png` and `CanyonNorth-NormalsAfter.png`. The after image removes broad polygon-to-polygon shading discontinuities while preserving major cliff creases. Both rendered images were actually inspected before retaining the change.

Updated the builder to smooth polygon normals and mark connected edges sharper than 55 degrees as split edges. Exports use FBX EDGE smoothing and tangent space; importer explicitly requests imported normals/tangents. This changes shading only. All six original bounds, dimensions, vertex positions/topology generation and total 44,352 triangles remain unchanged; an independent old/new manifest comparison confirms bounds/dimension/triangle equality for each mesh. New per-mesh geometry hashes and sharp-edge counts are recorded. Build marker again PASS6, FBX dimensions roundtrip PASS6.

This pass does not add collision, change navigation, or move any mesh. Remaining limitation: the crest silhouette is still deliberately low-poly; smoothing improves the broad eroded faces but cannot manufacture missing rocky geometry. Root must inspect a fresh Unreal overview after reimport because a Blender normal preview alone does not establish that the engine's import pipeline honored the normals.
