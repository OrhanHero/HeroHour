# Reference HUD V7

## Visual reference actually inspected

- `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_00.png`.
- Compared with `Art/Previews/ConceptV6-Combat.png`.

The concept uses a narrow top-left resource line, compact top-center army, commander powers top-right, minimap lower-left, selected object lower-center, and dense five-column portrait production lower-right. V6 retained the locations but its tall resource panel and two wide production cards per row changed the composition substantially.

## Implemented changes

- Resource block reduced from 94 to 60 reference pixels: title plus one actual economy line. Current/required power, credits, actual mobile army count and live radar status remain visible.
- Army panel reduced to 320 by 80 pixels. Only implemented infantry, combat vehicles and collectors are shown; no fictional air roster.
- Commander panel contains two actual faction doctrine cards, real points, shared cooldown, readiness and match time. These clickable cards use `DoctrineCard()` for both rendering and input. They invoke the existing target-confirmation flow; points are consumed only by the authoritative valid target command.
- Five portrait columns replace two elongated cards. `ProductionColumns` and `ProductionCard()` are shared by draw and click routing. Production panel height is reduced from 500 to 340 pixels. Actual costs, availability, exact-role tooltips, queue slots, refund action, all five functional categories remain.
- Existing lower-left minimap and lower-center selection/active abilities retain their truthful functionality.
- The legibility pass compares `Art/Previews/ConceptV3-Battle.png` directly with the reference. That capture predates the final compact card labels: its tiny full building names are not the final source text. Cards now use `RAFFIN.`, `FABRIK`, `KRAFTWERK`, `KASERNE`, `RADAR` and equivalent short unit labels; the hover title retains each full data-asset name. Doctrine cards retain their full names in hover text and in the wider top-right cards.
- `FitText()` measures the actual Canvas font at the current viewport scale and limits text to its owning width, including the shadow offset. It bounds the 64 reference-pixel card interior, large resource values, selection title/status, production context/help and refund label. This guards against adjacent-card overflow rather than relying on a fixed character count.
- Production portraits are square 62 by 62 reference pixels, centered within each 74-pixel card. The earlier 78 by 62 drawing crossed the card boundary and stretched square renders.
- German text uses actual UTF-8 umlauts, including `PRÄZISION`, `RÜCKFAHRT`, `UNTERDRÜCKT` and `STATIONÄR ENTFALTET`.

## Inspection and verification boundary

At the 1672 by 941 reference canvas: production occupies x1234..1660, y579..919; the five cards occupy x1250..1644 and y685..779. Queue occupies y817..857, cancellation y865..893, help y900. Selection ends x1121, so it does not overlap production. All values scale through the same layout at 1600 by 900.

The same 16:9 layout scales a 74-pixel reference card to approximately 56.6 pixels at 1280 by 720, 70.8 pixels at 1600 by 900 and 84.9 pixels at 1920 by 1080. The card's measured text interior is 64 reference pixels, or approximately 49.0 / 61.2 / 73.5 physical pixels respectively. At 1280 by 720 this remains a dense desktop interface; small explanatory details belong in the hover tooltip. Automatic width fitting prevents overlap but does not prove comfortable reading at that resolution. Root must inspect a fresh 1280 capture before claiming that threshold visually accepted.

The category order and shared draw/input rectangles remain unchanged by the legibility pass. The root integrates the separate UI click-test work; this pass edits no tests and runs no Unreal processes. Root must build, run both faction UI tests, and inspect actual package captures before claiming visual acceptance. This is a composition improvement, not a claim that the complete concept art has been reproduced. The current flat panels, generic glyphs and procedural portraits still lack the reference's bespoke icon art and worn metal finish.
