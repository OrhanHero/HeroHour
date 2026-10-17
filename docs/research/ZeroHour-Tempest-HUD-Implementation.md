# Zero Hour layout / Tempest presentation — 2026-10-17

## Implemented source change

The supplied `Art/Concepts/CityHero/zh_widescreen.jpg` was viewed directly. The composition now follows its functional ordering rather than the previous Tempest right sidebar:

- Radar lower left (300 reference units).
- Wide shared production / command panel lower middle, five command-card columns.
- Selected object portrait, health, statistics and real actions lower right (348 reference units).
- Economy status and credits above the middle command panel.
- Existing doctrine buttons above the lower-right object panel, actual numbered groups above the middle panel.

This uses HeroHour's existing original steel-frame drawing, faction colors, cropped portraits and glyph atlas. No Zero Hour image/frame assets were copied. The Tempest-inspired treatment remains in the frame/material/color rendering. This is an implementation milestone; final visual fidelity requires an actual Unreal screenshot review by the root agent.

## Functional preservation

Rendering and click handling use the same `FPrototypeHUDLayout` geometry. Production role dispatch, actual producer queues, paid-cost refunds, doctrines, ability commands, manual power, selling, minimap power gating, group membership and pause/end menus keep their authoritative existing paths. Radar contents and selection statistics were reflowed to fit their new panels; the old wide-panel text positions and vertical category labels were removed. Tooltip position is clamped around its actual mouse position above the lower panel.

The existing interaction tests remain; additional geometry assertions cover radar/commands/selection ordering, bottom alignment, credits-above relationship, action containment and control-group/credit separation at 1280x720, 1600x900 and 1920x1080.

## Verification

Editor build passed. All four actual editor-game UI runs passed with **76 assertions each**: Aegis and Ember at both 1600x900 and 1280x720. Exact log names, commands and hashes are recorded in `docs/verification/2026-10-17/zero-hour-tempest-ui-package/Editor-UI-Checks.json`.

The first editor run failed six new geometry assertions because the credits panel overlapped the command panel by six reference pixels. The actual layout was corrected (resource Y H-332 to H-344), retaining the same strict tests. The subsequent four passes are separate runs, not a relabelled failure.

A fresh Windows package was produced by `Packaging-20261017-020857-747.log`, with Build-Info CreatedUtc `2026-10-17T00:09:58.9298367Z`. The fresh packaged regression run passed **19 required suites with 917 actual assertion checks** and **46 natural Match/LogisticsStress rounds**. These counts were computed from the completed logs by `Scripts/research/record_zero_hour_ui_package.py`, which also verified executable/base directory, timestamps, suite/faction/viewport command lines, PASS/FAIL markers and package hashes. Both packaged UI suites passed 76 checks each at 1600x900. Extra 1280 packaged checks and screenshots are recorded separately when complete; they are not included in the 917 count.

Exact evidence: `docs/verification/2026-10-17/zero-hour-tempest-ui-package/Transformation-Checks.json` and `Package-Integrity.json`.

Root personally reviewed fresh packaged captures: `Art/Previews/ZeroHour-Production-1600.png` (Aegis, production fixture) and `Art/Previews/ZeroHour-Radar-1280.png` (Ember, radar fixture). Both visibly retain radar left, command cards middle and selection right without the former right sidebar. These are labelled test fixtures, not evidence of a naturally played match. The world is still HeroHour's existing map, not the transferred Tempest sector. Frame richness, landscape fidelity and several unit silhouettes remain below the supplied concepts; layout acceptance does not establish perfect art parity or completion of the game.
