# Concept UI V6 visual QA

## Evidence inspected

Read-only inspection of `Art/Previews/ConceptV6-Aegis.png` and `Art/Previews/ConceptV6-Ember.png`, both 1600x900 Editor captures provided by root, compared directly with the supplied `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_00.png`. These captures predate the subsequent rotation fix. They are not packaged-build proof, an interaction test, or evidence that the entire game is complete.

## Visible result

- Thin chamfered neutral borders are substantially closer to the restrained concept chrome than the V5 nested bright rails. Title strips, content and colored statuses have a clearer hierarchy.
- Credits, power and army counters fit. Infantry, vehicle and collector counts are separated, and Aegis versus Ember labels are consistent with their visible units.
- Both HQ portraits are complete and sufficiently large to identify the building; their sand/blue and graphite/orange palettes agree with the corresponding world HQ. Antennae and the base do not clip.
- Production portraits, prices, duration, shortcut badges and labels remain inside their cards in both captures. No obvious overlap or clipped text is visible. Card art is adequate for role recognition but too small to communicate most new mesh details.
- Queue slots, power/sell/focus actions and footer hints are visible within their panels. This inspection does not prove that their clicks route correctly.
- Radar is explicitly offline. The empty tactical display is consistent with that state; these captures do not demonstrate the operational minimap.

## What these images cannot verify

Both captures select the HQ. Damage, range, armor and speed labels appear only for a single combat unit, so their new two-column layout and suppression bar are **not visually validated by these images**. A separate combat/infantry selection capture is required. No battle-state tooltip, long unit name, multi-selection, low-resolution layout or disabled/hover interaction is demonstrated here.

## Remaining concept gaps

- The concept uses a dense five-column production gallery; the runtime still uses two wide cards per row and a large queue area. This is a structural difference, not solved by changing frame colors.
- The concept selection panel is shorter and pairs larger character art with compact square ability controls. Runtime controls remain wide labeled buttons in the established input layout.
- Subtitles, card labels and keyboard hints are small at 1600x900 (roughly 7-9 physical pixels in these captures). Main counters are readable; secondary text deserves a dedicated typography/resolution pass.
- Commander UI shows real points/status rather than the three large superweapon dials in the concept. Adding decorative ready weapons without corresponding mechanics would misrepresent the game.
- The world remains much sparser and more geometric than the illustration. UI improvements do not close that scene-density, terrain or faction-silhouette gap.

No blocking UI defect is established by these two images. The visible results are an incremental fidelity improvement, with the unverified states and larger layout differences explicitly outstanding. No source edits or builds were performed during this QA task.

## Follow-up: packaged combat selection capture

Visually inspected `Art/Previews/ConceptV3-Combat.png`, supplied by root from the 19:35 package on 2026-10-15. This closes the earlier missing evidence for the normal Aegis single-combat-unit statistics layout. A Striker is selected with its complete vehicle portrait; the panel visibly shows health 100/100, damage 20, range 4.8 m, armor multiplier x1.15, speed 2.2 m/s, rank 0 / XP 0 and no-cover state. Each label and value occupies a separate readable position in the two-column layout. No overlap with the health bar, focus button, portrait or bottom ability row is visible. The selection portrait is identifiable at its rendered size and fits without clipping. The source of the displayed values remains the actual effective gameplay getters; this screenshot alone is not a numerical gameplay test.

The three bottom ability controls and keyboard hints fit their bounds. No click, activation or disabled-state behavior is asserted from the image. Infantry suppression, long/localized unit names, hover tooltips, Ember combat selection, multi-selection and alternative resolutions remain outside this visual sample. The earlier concept-gap observations remain valid. This follow-up performed no code edits or builds and makes no claim that all broad acceptance suites have completed.
