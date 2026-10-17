# Concept UI V6

## Reference and scope

Compared the actual supplied `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_00.png` with `Art/Previews/PresentationV5-Aegis.png` by visual inspection. The concept uses dark neutral steel, narrow chamfered edges, restrained separators, bright actionable content, separate infantry/vehicle army counters and unit art rather than large chrome highlights.

## Implemented

- Replaced the five nested bright bevel outlines, stretched frame atlas and corner rivets with a single fine chamfered steel outline, dark title strip and small status marker.
- Reused the same thin border for cards, portraits, abilities and queue cells; no new bitmap asset or content import is required.
- Removed decorative scan lines behind selection portraits and production cards. Production portraits are slightly larger while labels, cost and duration remain in their existing bounds.
- Army panel now reports infantry, combat vehicles and collectors separately, using the actual living player army. Colored occupancy bars do not imply a health percentage.
- Combat selection now presents four individually labeled damage, range, armor multiplier and speed values from the actual effective getters in two readable columns, with rank/XP and cover/suppression state below.
- Lowered inactive chrome contrast and reserved cyan, gold, green and red for gameplay state.

## Interaction and compatibility

`PrototypeHUDLayout.h` is unchanged. Every panel, production tile, tab, queue slot, ability, power toggle, sell and focus click rectangle stays identical. Existing font scaling, wrapping, tooltips, German labels, faction-specific portraits, radar gating, pause and placement actions are preserved. No unavailable air units or fictional superweapon states were added to imitate the drawing.

## Validation and limitations

Static diff check passed. Source inspected for removed army-health accumulator references and independent infantry/vehicle/collector counts. Root integration must compile the project, run both faction UI and presentation suites and visually inspect a fresh packaged capture before calling this runtime verified.

This iteration improves chrome and information hierarchy. The concept's denser 5-column production gallery and shorter selection panel require a coordinated shared-layout redesign with input and acceptance checks and are intentionally not represented as complete here.

## V6 scene acceptance contracts

Presentation tests retain the existing 76 checks and add 17 checks from the exact ConceptCityV6 and ConceptTerrainV6 asset manifests: ten authored meshes with own materials, precisely 28 replacement city actors and zero legacy civic meshes, precisely six uniquely mapped terrain tags, and saved NoCollision profile/mode plus disabled navigation influence for every city/terrain component. These checks have not yet run in Editor or a packaged build; no runtime validation is claimed by this agent. MilitaryDetailV6 checks are deferred until those assets and imports exist.

## Military portraits refresh

`Scripts/art/refresh_military_portraits_v6.py` renders the three new MilitaryDetailV6 blends for both factions into the six existing HQ/Factory/Depot portrait paths. Cycles 24 samples, transparent 512x512 RGBA, V5 studio and literal linear faction palettes remain identical. All six renders were inspected: full subjects, readable antennae, intake ramps, pipework and faction markings, no clipped alpha edges. Alpha bounds range from x=43 to x=471 and y=97 to y=460. Metadata keeps all 22 records; the six replacements record their new source blend SHA and PNG SHA. Assertions verified all source blends remain unchanged and the other 16 image hashes and metadata records remain unchanged. Unreal portrait reimport and packaged UI validation remain root integration work.
