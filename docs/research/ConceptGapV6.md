# Concept gap V6: environment evidence and next work

Read-only review of authored scripts, shared layout constants, existing V5 capture and V6 FBX previews. **This is a gap analysis, not a claim that the concept has been reproduced.** No map, source code, material or asset was changed for this review. The current coordinating Unreal import may still be running; references to V6 describe the authored assets and their intended replacements, not a captured packaged V6 scene.

## Images actually inspected

- Reference close-up: `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_00.png`.
- Reference map: `E:/HeroHour/Art/Concepts/HH Art/CITYHERO_SAMPLE_1.png`.
- Current own capture: `Art/Previews/PresentationV5-Aegis.png`.
- Older district capture: `Art/Previews/ConceptV3-District.png` (historical, not proof of current V6).
- Newly rendered geometry: `Art/Models/ConceptCityV6/SM_CC6_Service-Preview.png` and `SM_CC6_Tower-Preview.png`.

## Why the raster still looks empty

The concept groups streets around distinctive districts, with uneven rocky banks and dense buildings. HeroHour's source map instead lays long orthogonal asphalt slabs across flat terrain, including the starting bases. The V5 base capture consequently shows several huge uninterrupted diagonal dark bands from the camera angle, while the adjacent sand has few large readable features. Changing downtown meshes alone cannot fix that starting view: downtown is elsewhere, and the bases retain the inherited road slabs.

`Scripts/art/create_cityhero_map.py` authors north/south roads at X `-8000,-5000,0,7500`, west/east roads at Y `-6000,0,6000`, and full-width bridge arteries at Y `±3200`. Asphalt slabs originally have no separately authored junction geometry or irregular edge treatment. `import_district_v3.py` removes the X=-8000 north/south road and adds an HQ approach curve, sidewalks and urban modules; the other inherited slabs remain. V5 intentionally preserves prior actors, so it cannot remove that visual structure. Do not confuse the historical slab definition with a fresh inventory of every current transform.

### Reproducible measurements from source

Unreal positions and shared navigation constants are centimetres. Blender modelling is in metres and import checks compare dimensions multiplied by 100. `BattlefieldLayout.h` gives playable extents ±12000 by ±9000 cm: **240 × 180 m**. This is a compressed RTS board, not a geographically scaled city.

| Authored element | Evidence | Width / size |
| --- | --- | --- |
| North/south original slabs | `create_cityhero_map.py`, `North south road` | 550 cm = 5.5 m |
| West/east original slabs | same, `West road` / `East road` | 450 cm = 4.5 m |
| Bridge artery | originally 1250 cm, then `import_district_v3.py` sets cube Y scale 9 | 900 cm = 9 m after that pass |
| Urban asphalt | `build_district_streets.py`, `SM_DV3_UrbanStreet` | 380 cm = 3.8 m asphalt; 652 cm overall with footpaths |
| Southern detailed road | `SM_DV3_RoadStraight` | 9 m asphalt plus footpaths |
| City parcel pitch | shared `CityColumns` / `CityRows` and district importer | 1500 cm = 15 m |
| Existing gameplay city obstacle | `BattlefieldLayout::CityObstacleRadius` | radius 600 cm = 6 m |
| Vehicle clearance | `BattlefieldLayout::VehicleRadius` | radius 80 cm; only 140 cm of centre-line gap remains between adjacent inflated 680-cm city obstacles |
| V6 building plan | JSON bounds × existing map scale `(2,2,1)` | approximately 7.8 × 7.9–8.3 m; actual vertex radius is independently checked below 600 cm |

The widths are not all excessively wide in physical metres. Their visual dominance comes from uninterrupted length, flat colour, vacant setbacks, repeating right-angle crossings and the camera's broad field. Globally narrowing roads would also risk a mismatch with bridges, sidewalks and existing RTS clearance. A more varied roadside/ground composition is the safer first improvement.

Time is not stretched to compensate for the board scale: `TickEconomy` increments `MatchTime` by positive unpaused `DeltaSeconds`; units move by `MoveSpeed * DeltaSeconds`, capped by remaining distance. The DataV4 authoring script specifies base vehicle speed 220 cm/s (2.2 m/s), infantry 250 and siege 180, then faction/combat modifiers apply. A clear 15-m parcel pitch therefore takes roughly 6.8 seconds at base vehicle speed before detours. These are authored HeroHour tuning values, not measured Tempest Rising values. Camera movement is 4500 cm/s and zoom height spans 1600–28000 cm in `APrototypeController::PlayerTick`.

## What V6 city geometry addresses

The four original buildings add floor rhythm, actual window reveals, balconies, service landings, parapets, roof enclosures and HVAC. Their renders show a stronger building silhouette and surface depth than plain extruded blocks. They retain a very regular bay pattern, and they do not create the concept's varied street frontage or the strong rocky riverbank composition. They also do not improve housing districts, industrial landmarks or the starting base's open ground.

## Prioritized feasible follow-ups

1. **Capture and inspect the actual V6 base plus district views at the same camera positions.** Verify all 28 replacements, persistent NoCollision after reload, unchanged city blockers and visible street passages. Use equal framing when comparing images; a studio model render is not scene acceptance.
2. **Finish junctions and road edges before adding more buildings.** Reuse measured asphalt footprints and shared materials for painted junction corners, drain strips, low flush curb transitions, occasional faded crossings and shallow irregular dust shoulders. Keep overlays at millimetre/centimetre height to avoid covering unit feet. Add no tall objects in the 140-cm city centre corridors. Check overlapping old slabs/modules for visible edges or z-fighting; do not infer a z-fighting defect without a fresh capture.
3. **Give the base starting camera a deliberate composition.** More varied ground roughness, dust trails between resource/depot/HQ, low sparse grass, and worn service markings can break flat empty sand while remaining traversable. Preserve placement freedom around the HQ, real harvester pickup positions and FIFO staging lanes. Avoid decorative traffic barriers on traversable ground: NoCollision alone would let units visibly clip through them.
4. **Dress already blocked parcels with compact, plausible details.** Use original crates/pipe bundles only where they fit between the mesh and existing 600-cm city obstacle boundary; the V6 facade can overlap old V5 prop placements, so audit those combinations. Small bins, utility cabinets, shallow planters and facade-attached signs can add frontage variety within that existing blocker. Prefer batched instances and shared materials instead of hundreds of unique actors/materials.
5. **Improve river silhouette within currently impassable water/bank space.** The reference has irregular turquoise shallows, broken banks, boulders and riparian vegetation. Keep new rocks wholly inside impassable river/border areas, and leave the two ±3200-cm bridge corridors and approaches clear. Geometric water/shore changes must be checked against `IsLand`; art should not imply a crossing where navigation rejects it.
6. **Replace the most visible repeated houses and industrial silhouettes.** A small original West Heights family with varied roof orientation, porch/wall details and shrubs inside existing house footprints would reduce the stamp pattern. Refinery tanks, pipe racks and stacks should fit the existing landmark circles or require an explicit coordinated layout change. These require their own source assets and acceptance captures, not renaming existing meshes.
7. **Then change district layout deliberately if needed.** Multiple buildings per parcel, organic peripheral roads and larger district transitions are needed for stronger reference fidelity. This is beyond a decoration pass: layout constants, static obstacles, placement restrictions, minimap, AI routes and navigation tests must change together. Do not simply enlarge building geometry into open streets.

## Acceptance that should accompany the next pass

Record fixed-camera base/district/river screenshots, actual actor and material counts, before/after performance on the same single running build, and navigation/logistics checks on unchanged or deliberately revised routes. Compare visible art against actual blocking: decorative assets must either sit within existing obstacles/impassable land or be visually traversable. The current 28-building kit is useful progress; it is not a completed concept-faithful world.

## Confirmed rendering defect discovered during V6 review

The coordinating agent's fresh probe in `Saved/Logs/ConceptV6-RotationAudit.log` demonstrates that **this UE 5.8 Python binding interprets `unreal.Rotator(0,90,0)` as pitch=90, yaw=0, roll=0**. Existing authoring scripts used that positional form intending yaw. House actors intended to face 180 degrees were upside down below the ground; multiple city actors intended to face 90/270 degrees lay on their sides and appeared as large white walls. Mesh bounds alone hid the problem because the first component inventory recorded local bounds and scales but omitted world rotation.

`Scripts/art/fix_authored_rotations_v6.py` repairs only affected original `/Game/HERO/` StaticMeshActors, recovers intended yaw from their forward vector's X/Z angle, preserves position/scale, checks all 64 house world-height ranges, and records before/after evidence. It is idempotent and does not alter navigation constants. The five original city/environment scripts now use named `pitch`, `yaw`, `roll` arguments, including their intended sun/viewpoint angles. The coordinator executes the repair serially, checks saved-map reload and captures actual results; this paragraph documents the observed cause and authored repair, not a claim of completed downstream acceptance.
