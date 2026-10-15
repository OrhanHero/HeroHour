# Concept City V6 — original downtown architecture

The close-up `Codex-Bild 1. Sept. 2026, 01_22_00.png` and overhead `CITYHERO_SAMPLE_1.png` were actually viewed. The close-up establishes dense charcoal/concrete blocks, repeated inset window bays, roof equipment and parapets. The overhead establishes a compact central skyline bounded by drivable streets.

## Delivered assets

`Art/Models/ConceptCityV6` contains four original Blender sources, matching FBXs, JSON geometry reports, transparent previews and a manifest. There is no extracted Tempest Rising or Generals content.

| Mesh | Floors | Triangles | Purpose |
| --- | ---: | ---: | --- |
| SM_CC6_Courtyard | 4 | 6,180 | Balcony apartment/storefront block |
| SM_CC6_Office | 6 | 8,700 | Regular commercial midrise |
| SM_CC6_Tower | 9 plus penthouse | 13,656 | Central stepped skyline |
| SM_CC6_Service | 5 | 7,704 | Apartment block with exposed service landings |

These are one mesh each, with five shared existing DistrictV3 material slots. Window glass is recessed behind projecting jambs, lintels and sills. Roofs have solid parapets, louvered HVAC units, ducts and service enclosures. All solids are closed, UVs are physically measured in metres, pivots are at ground origin, and every FBX was imported back into Blender to verify dimensions. Per-vertex XY radius at the existing map scale is asserted below 600 cm.

## Reproduction / integration

Run Blender 5.2 background with `Scripts/art/build_concept_city_v6.py`. Expected final marker: `CONCEPT_CITY_V6_BUILD_PASS 4`; check for Python tracebacks even if Blender exits zero. The builder also renders actual FBX-roundtrip previews at the map's `(2,2,1)` scale.

Then run `Scripts/art/import_concept_city_v6.py` using Unreal Editor Python, **serially with other map importers**. It imports to `/Game/HERO/ConceptCityV6/Meshes`, binds the existing five `/Game/HERO/DistrictV3/Materials/M_DV3_*` materials, loads CityHero, replaces precisely the 28 existing `SM_DV3_Civic_*` mesh components, and clears obsolete overrides. Re-running also recognizes its own `SM_CC6_*` meshes. It preserves actor labels, coordinates, rotations, scales, roads, pavements and all gameplay blockers. It sets the persistent `NoCollision` profile and disables navigation influence. Central parcels get towers; other parcels alternate the other profiles.

Expected final marker: `CONCEPT_CITY_V6_IMPORT_PASS 28`; actual imports/replacements are recorded in `Saved/Research/ConceptCityV6-Import.json`. The agent authored and validated the source geometry; the coordinating agent executes Unreal import, captures and acceptance tests. This document does not claim those downstream checks have already run.

## Limits / next art work

This improves the existing 28 downtown parcels; West Heights dwellings, refinery landmarks and terrain are separate tasks. Shared original procedural DistrictV3 PBR surfaces provide continuity, but this is still an original prototype kit rather than final unique baked building art. Buildings have deliberately regular architectural bays; damage states, interiors, signage, bespoke decal weathering and additional facade families remain future work. No collision or RTS navigation obstacle is added.
