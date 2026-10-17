# Original terrain dressing V9 — source candidate

**Current status:** Root's final Unreal import passed and saved the map. One actual Unreal district art-review capture was directly inspected. Normal gameplay camera review and updated packaged presentation checks remain separate acceptance steps.

This work follows the rocky, irregular desert edges in the supplied faction-base concept and the large visual gap observed in actual V8 Unreal base captures. It creates original HeroHour geometry, reusing only the project's existing original DistrictV3 PBR textures. No Tempest Rising geometry or texture is extracted. REA evidence informs separate gameplay work; it does not establish provenance or visual quality of this kit.

## Ownership and present status

The art task owns `Scripts/art/build_reference_terrain_v9.py`, `preview_reference_terrain_v9.py`, `import_reference_terrain_v9.py`, and `Art/Models/ReferenceTerrainV9/`. Root alone executes the importer. No Unreal import, map mutation or game-source change has been performed by this task. Blender CPU source generation passed with `REFERENCE_TERRAIN_V9_BUILD_PASS` in `Saved/Logs/ReferenceTerrainV9-Build.log`. The initial two PBR previews were rendered and inspected, then rejected: overly regular cap silhouettes and steep-wall bases appeared as protruding cylinders. The corrected source follows sampled terrain for upper rings as well as its contact ring, uses 24-point eroded profiles and muted sandstone tints. Corrected PBR previews completed and were directly viewed: the river ledges are now irregular and above water; the border clusters retain overly regular spacing and faceted low-detail surfaces. This is a runtime integration candidate, not accepted concept-level finished art.

Four merged meshes contain **45,312 triangles total**, 11,328 each. Each combines twelve stratified ledges and 36 small talus boulders. Layered stepped geometry supplies actual silhouette variation rather than texture-only grooves. Six shared PBR material slots use sand and dirt texture inputs with sandstone tints. Slots retain the audited helper's internal names; `Metal`, `Track` and `Paint` are historical names here and all have zero metallic value. The source carries dominant-plane physical metre UVs. Real FBX roundtrips preserve bounds; closed geometry has zero non-manifold edges and degenerate faces. Metadata records exact FBX hashes, all piece dimensions and positions, terrain-input hashes, and authoritative layout hash.

## Placement and gameplay safety

`BattlefieldLayout.h` defines the walkable rectangle as ±120m X and ±90m Y, with a river exclusion around X=-30m, half-width 7m, and bridge corridors centered on Y=±32m. Connected buildings can extend construction over open land. Tall rocks placed there with NoCollision would cause units and buildings to pass through visible obstacles. This candidate therefore **does not scatter significant rocks across buildable base sand**.

RiverNorth and RiverSouth dress the existing excluded river margins near X=-36m/-24m, Y=±8m/±14m. They retain conservative margins from dry land and both bridge corridors. WestTalus and EastTalus dress the foot of existing canyon walls beyond X=±120.6m, near Y=-18m to +6m. Every generated vertex is checked against these authoritative excluded regions. The prepared importer independently checks complete actor bounds and refuses a changed layout hash. Existing terrain surfaces are sampled from read-only mesh BVHs, keeping source originals untouched. An initial deeper river placement was rejected after sampled heights showed it would remain under water; the regenerated candidate moves ledges toward the existing banks. Some base vertices remain submerged, while final rock tops reach approximately Z=+0.42m/+0.59m against the existing water contact height of -1.45m. Preview review must confirm contact and visibility.

All four actors use identity rotation/scale, explicit measured positions, NoCollision, disabled navigation influence and disabled shadow/distance-field contribution. The importer checks all nine reflected collision-shape arrays, preserves every actor outside its own `TRV9` prefix / `HERO_TerrainV9` tag, and saves only after comparisons pass. It modifies no C++ footprint, resource route, road, bridge or placement rule.

## Reproduce and inspect

1. Run Blender 5.2 background with `Scripts/art/build_reference_terrain_v9.py`. Require its pass marker and inspect errors even if Blender exits zero.
2. Verify the manifest, source hashes, 4-actor / 50k-triangle limits, collision flags and FBX roundtrip records.
3. When the shared GPU is free, run `Scripts/art/preview_reference_terrain_v9.py`. Review both actual PBR geometry renders. The water plane used in the river preview is only a known-height visual contact reference. Blender context renders are not Unreal screenshots.
4. Root may execute the prepared importer serially in Unreal and require `REFERENCE_TERRAIN_V9_IMPORT_PASS`, then capture actual approach cameras and review contacts/material blending. Run appropriate navigation/presentation checks after integration.

## Limits

The work adds readable irregular border and river shapes. It cannot solve the large smooth buildable sand areas safely with tall geometry. Those areas still need original ground texture variation, shallow dust/debris treatment, and designed base activity. Existing V6 canyon meshes remain intact. Six simple tints on existing textures are not equivalent to purpose-authored sandstone atlases. No concept parity, runtime frame-time result or package acceptance is claimed.

The corrected `SM_TRV9_RiverNorth-Preview.png` and `SM_TRV9_WestTalus-Preview.png` show actual candidate geometry with existing source terrain loaded read-only. Existing canyon/riverbank source materials appear gray in Blender because their final Unreal surfaces are assigned during import; this does not prove runtime material mismatch. Conversely, it cannot prove good runtime blending either. Actual game-camera review remains required. The preview GPU process exited before the slot was returned to Root.

## Final source comparison and proposed runtime gate

All four final source records were checked again against the manifest: RiverNorth/South each span roughly 13.2m across the two banks and 12m along the river; West/EastTalus each span about 4m across the wall foot and 30.4m along it. All contain 11,328 triangles. Two actual previews cover RiverNorth and WestTalus only; South/East geometry was checked through source metadata and FBX roundtrip, not independently rendered. The original concept has continuous, varied weathered rocks and convincing fine surface breakup. The candidate provides irregular silhouettes but remains much more faceted, repetitive and simply textured. No inference from the two preview images establishes unseen-side visual quality.

A small cooked runtime presentation gate should add four aggregate checks, owned by the existing presentation-test agent:

1. Exactly four `HERO_TerrainV9` actors exist, each with a distinct expected `SM_TRV9_RiverNorth`, `RiverSouth`, `WestTalus` or `EastTalus` mesh under `/Game/HERO/ReferenceV9/Terrain/Meshes/`. Reject a missing actor, duplicate mesh, Engine fallback or unexpected fifth actor.
2. Actual component world bounds stay outside the board (X entirely below -12060cm or above +12060cm), or wholly within the safe central river rectangle (X -3680..-2320cm, Y -2140..+2140cm). Use transformed world bounds, not intended metadata alone. This catches scale, axis and placement errors affecting gameplay land or bridges.
3. Every component has NoCollision, cannot affect navigation, has disabled cast-shadow/distance-field contribution and identity actor scale. This verifies the promised nonblocking/cost behavior in the actual loaded map.
4. Every component has five nonnull material slots, exactly Concrete, ConcreteDark, Metal, Track and Paint, resolving to the project-owned `/Game/HERO/ReferenceV9/Terrain/Materials/MI_TRV9_` material family with a valid original parent. The sixth source slot, Dust, has no polygons and is removed by Unreal import. This catches missing cooked dependencies and accidental gray Engine fallbacks without embedding source-image pixel checks in gameplay tests.

These checks supplement visual review; they do not turn a safe asset into concept-quality art. No presentation-test source was edited by this art task. Terrain source and importer ownership was explicitly released to Root before integration.

## First Unreal import failure and repair

Root's first import attempt failed before map saving: `Saved/Logs/V9-TerrainImport.log` reports `KeyError: Concrete` while constructing the parent material. This was a copied V8 assumption: the terrain manifest's `Concrete` slot uses the Sand texture source, so no `textures['Concrete']` inventory entry exists. The importer now uses the explicitly preflighted Sand texture set for parent defaults; each material instance still receives its exact manifest textures and parameters. This fixes the concrete error without adding an unnecessary texture dependency. Any partially created parent material is rebuilt on retry. The failed attempt is not a passed import or runtime acceptance.

A separate CPU-only Blender FBX audit actually reimported all four exported meshes. Each has six source material slots named `BY8_Concrete`, `BY8_ConcreteDark`, `BY8_Metal`, `BY8_Dust`, `BY8_Track`, `BY8_Paint` (Blender may add numeric duplicate suffixes). This prefix is intentional because the source builder reuses the audited V8 material helper. The prepared importer strips that proven source prefix and binds the new `MI_TRV9_*` destination material family; it now explicitly rejects unexpected source prefixes. `Saved/Logs/ReferenceTerrainV9-SlotAudit.log` contains the audit. Importer syntax passed. Root must still retry Unreal and require its pass marker; this repair alone does not establish acceptance.

The second Root attempt also failed before map saving, at the assertion requiring six Unreal mesh material slots (`Saved/Logs/V9-TerrainImport-Retry.log`). An additional CPU FBX audit counted actual polygon assignments, rather than only the source slot inventory. All four meshes have **zero polygons assigned to BY8_Dust**; the other five slots are used. Unreal discards that unused source slot. Exact per-mesh polygon counts and expected five used names are now recorded in each mesh JSON and the manifest. The importer validates the actual Unreal slot count against this independently measured value and compares the complete used-name set, rejecting missing, duplicate or unexpected bindings. It does not simply remove material validation. Six shared MI assets may still exist, but only five are bound on each terrain mesh. The compact measured audit is preserved under `docs/verification/2026-10-16/reference-v9/terrain/UsedMaterialSlots.json`.

The post-export audit metadata is additional to the generator output. Regenerating source geometry must repeat the FBX polygon assignment audit and update `source_material_polygon_counts`, `used_source_material_slots` and `expected_section_count` before import; the importer fails closed if these fields are absent. Neither failed attempt establishes a passed map import; the successful subsequent run is recorded below.

## Successful final Unreal import

The subsequent Root run completed with `REFERENCE_TERRAIN_V9_IMPORT_PASS` in `Saved/Logs/V9-TerrainImport-Final.log`. `Saved/Research/ReferenceTerrainV9-Import.json`, copied to `docs/verification/2026-10-16/reference-v9/terrain/Import.json`, reports four actors, six shared MI assets, 45,312 triangles, and unchanged snapshots of all 1,300 pre-existing actors. All nine collision aggregate arrays are empty for each mesh; actor collision is disabled, navigation unchanged and shadows disabled. Source FBX hashes match the generated files. This is successful map integration evidence, distinct from the two preceding failed attempts. No geometry sources were changed during these importer repairs.

The import report alone does not establish natural terrain contact, camera visibility, visual quality or packaged-build acceptance.

## Actual Unreal district visual review

`Art/Previews/ReferenceTerrainV9-Editor-ArtReview-District.png` was directly viewed after successful integration. It shows the district, river, both bridges and the Aegis start base in Unreal. This is an Editor **art-review** view with the HUD and fog removed, not a normal player view or packaged-build screenshot.

Small irregular protrusions are visible along the central river margins. They add limited silhouette variation while leaving both broad bridge corridors visually unobstructed. At this district camera they are subtle and do not change the overall impression substantially. The four new actor identities cannot be established from pixels alone; their presence is established separately by the successful import report. The exact new border groups are not clearly identifiable in this frame, so their surface contact or color matching is not visually accepted here.

Compared with the original concept, the dominant shortcomings remain large uniformly empty sand parcels, very broad clean roads, repeating city/window grids and abrupt straight riverbanks. The concept has far denser small-scale natural debris and transitions, varied compact building masses and richer terrain shading. This limited safe dressing pass improves some riverbank silhouettes; it does not resolve the wider environment mismatch. A normal gameplay approach capture and a future river close-up are better suited to checking individual rock contacts. No additional map or source changes were made during this review.
