# UI priority visual audit — 16 October 2026

## Superseding reference: Tempest runtime UI first

The user's latest instruction prioritizes the complete Tempest-style UI over the earlier concept's five-column lower-right composition. Personally viewed `Saved/Research/TempestRising/reference-live-base.png` (actual 2560 × 1440 runtime screenshot). Observable arrangement: radar upper right; resource strip to its left along the top; match timer top middle; tall right sidebar with BAUEN/DOKTRINEN primary tabs, five vertically written category tabs, three production columns and queue strip; ten numbered control groups bottom left. Multiple thin bevels and steel highlights enclose black interiors; gold selected tabs and orange accents carry state. The previous table below describes the V9-versus-concept gap and must not be read as an instruction to preserve five columns or the lower-left radar now.

Prepared original code-only drawing helpers in `Source/HeroHourPrototype/HUD/PrototypeHUDChrome.h/.cpp`: `DrawSteelFrame`, `DrawTabFill`, and `DrawVerticalLabel`. Helpers take shared HUD rectangles and viewport scale, leaving layout and input ownership with the root integrator. Local UE 5.8 declarations verified: `CanvasTypes.h` exposes `FCanvas::PushRelativeTransform(const FMatrix&)` and `PopTransform`; `GameFramework/HUD.h` exposes public `DrawText`, `DrawLine`, and `DrawRect`. `AHUD::Canvas` is protected, so the vertical-label helper accepts `UCanvas*` explicitly. It measures real font metrics, keeps minimum 12 physical pixels, elides overly long labels, and uses a balanced push/pop transform for bottom-to-top text. No source game textures are copied. These new helpers still require the root's compile and screenshot validation; writing code does not establish a finished visual result.

Scope: read-only visual comparison. No Unreal process, rendering, map edits, or source edits performed for this audit. The inspected source is `Source/HeroHourPrototype/HeroHUD.cpp` and `PrototypeHUDLayout.h`; screenshots are the actual V9 package, not a new V11 verification.

## Evidence personally viewed

- `Art/Concepts/CityHero/Codex-Bild 1. Sept. 2026, 01_22_00.png` (1672 × 941 concept).
- `Art/Previews/ReferenceV9-Package-Aegis-1600.png` (1600 × 900 packaged game).
- `Art/Previews/ReferenceV9-Package-HQ-CloseUp-1280.png` (1280 × 720 packaged game, close camera).

The panel arrangement already follows the concept: resources upper left, army upper middle, powers upper right, events and minimap lower left, selection lower middle, production lower right. That broad arrangement is not the outstanding problem. Surface treatment, icon scale, information hierarchy, and action presentation remain conspicuously different.

## Prioritized discrepancies and concrete corrections

| Priority | Observed V9 versus concept | Correction that preserves real project mechanics |
| --- | --- | --- |
| P0 | Flat charcoal rectangles with broad gray header bars dominate every panel. The concept uses near-black glass, inset areas, several thin steel highlights, stronger lower-left minimap framing, and restrained cyan glow. | Replace the shared `Frame`/`NineSlice` drawing treatment with layered dark interiors and two-level chamfered metal edges. Keep production cells quieter than outer frames. Avoid increasing panel footprint. |
| P0 | Commander area contains two wide flat cards with tiny generic glyphs. The concept's most recognizable upper-right feature is large circular colored power emblems. | Render two large ring emblems for the two actual faction powers, with clear readiness/cooldown text and existing points/target validation. Do not invent a third usable power to fill the concept. Keep draw and click rectangles shared. |
| P0 | Production models occupy a small fraction of the cell on large empty transparent backgrounds. The reference has close, readable unit art filling its card, a narrow price strip, and strong availability states. | Enlarge/crop the existing original portraits within a bounded art viewport, retain five columns, and separate price/hotkey from art. Show real buildability, progress and queue count. No new decorative production choices pretending to be functional. |
| P0 | Selection panel is dominated by long text labels and broad blue buttons (`K / AUS`, `DEL / VERK.`, focus). The concept has a compact stat block and a dense matrix of square cyan action icons. | Use square icon action cells with short hotkeys and full hover descriptions; expose only implemented actions. Keep building power/sell distinct from combat abilities, and preserve disabled-state reasons. |
| P1 | All headers have the same weight and a tall bar. The original differentiates title, section names, subdued field labels, bright values, and colored status. | Establish three consistent text levels. Reserve bright cyan/gold for actionable states and values. Keep the existing minimum physical text size at 1280 × 720. |
| P1 | The minimap is visually a large striped offline placeholder in both inspected package captures. The concept shows the world map, a visible camera frustum, and colored contacts. | Preserve the real radar-off mechanic, but verify the powered-radar state in an actual capture. Improve offline framing, and ensure active radar has terrain, contacts, and camera indicator. Do not infer the active state is broken from these offline screenshots. |
| P1 | Events show a paragraph-like instruction in a large area. Concept events are four compact rows, each with a colored semantic icon. | Render real notices as compact icon-led rows, respecting notice history and duration. Keep initial tutorial instructions separately readable if required. |
| P2 | Army uses three real categories and simple bars; concept has four including aircraft and losses. | Improve silhouettes and separators for the three implemented categories. Add aircraft/loss counters only once those real systems exist. |

## Source-level observations

`NineSlice` currently discards its texture argument with `(void)T` and draws only eight thin edge segments. `Frame` fills a 31-reference-pixel header stripe. Consequently, the presence of `T_UI_Frame_V3` and other art files is not evidence that the material treatment in those files is actually displayed.

The current portrait helper draws the entire 0..1 UV range into each target rectangle. This preserves transparent margins in each source image; card composition can be improved independently of generating more 3D meshes.

`FPrototypeHUDLayout` already centralizes production cards, doctrine cards, tabs, queue slots, and ability hit rectangles. Any replacement action layout must update both rendering and input through that shared geometry. Required check: each visible icon clicks its own action at 1600 × 900 and 1280 × 720; empty cells do nothing; disabled abilities do not spend points or issue commands.

## Acceptance evidence still needed

Fresh game captures of both factions at both resolutions, with radar online, a combat unit selected, a building selected, a genuine busy queue, and an unavailable power. Run UI input checks against the same fresh executable. Assess the frames and cell art at normal screen size, not only enlarged crops. This audit alone does not certify completion or visual parity.

## Paused residence work: bounded read-only finding

Personally viewed `Original699k-Front-PBR.png` and `LOD60k-{Front,Back,Side}-PBR.png` in `Art/Models/TrellisResidenceV11-LOD`. The 60k model retains the overall silhouette, doorway, windows and roof; its parapet and window lintels have visibly wavier edges than the original. It may suit distant RTS placement but is not approved for close-up replacement. `Reduction.json` records 699,605 original triangles and 60,000 reduced triangles, preserved source SHA and finite UVs. Its boundary-edge counts are not by themselves proof of open physical holes because UV-split vertices contribute boundaries. The 120k candidate has no personally inspected comparison render, so there is no evidence-based 120k visual recommendation yet. No authoring or Unreal import was performed; UI takes priority.
