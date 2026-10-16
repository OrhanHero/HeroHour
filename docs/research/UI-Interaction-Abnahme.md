# UI: Interaction acceptance and remaining work

## Final packaged acceptance, 16 October 2026

The final package (`CreatedUtc=2026-10-16T19:36:08.6233213Z`) passes all four faction/viewport combinations: Aegis and Ember at 1600×900 and 1280×720, **73 checks each**. This adds the interrupted world-drag pause regression to the historical 72-check editor baseline below. All 19 required packaged suites pass with 911 assertion checks and 46 natural Match/LogisticsStress rounds; extra 1280 UI runs and 14 FieldVisual checks are counted separately. Seven diagnostic packaged UI captures were visually inspected. [Final suite evidence](../verification/2026-10-16/tempest-ui-package/Transformation-Checks.json), [1280 evidence](../verification/2026-10-16/tempest-ui-package/UI-1280-Checks.json), [capture evidence](../verification/2026-10-16/tempest-ui-package/Captures.json). Remaining coverage notes below remain limitations, not claims of exhaustive physical input playback or pixel parity.

Date: 2026-10-16. Latest verified implementation checkpoint: **Editor Build 5, 21:10 Berlin**. The Tempest-style UI redesign is implemented: top-right radar, tall right three-column production sidebar, vertical categories, BAUEN / DOKTRINEN primary tabs, left commander shortcuts, numbered control groups, membership portraits, producer-selecting queue slots and real pause/result modals. Existing portraits are original HeroHour content; this is a reconstruction of observed interface structure, not imported Tempest UI art.

The actual Editor game runs are verified at **72 PASS / zero FAIL each** for Aegis 1600x900 and Ember 1280x720. Exact logs and coverage limits are below. Root reports a new Windows package built at 21:16 and is running its required test set; package acceptance is **not yet concluded by this audit**. No extra Unreal processes are started while root owns capture/package verification.

The user's latest instruction prioritizes the **Tempest Rising UI layout** over the earlier concept HUD. The actual capture `Saved/Research/TempestRising/reference-live-base.png` directly shows radar at top right, three portrait columns, five vertical tabs, BAUEN / DOKTRINEN and numbered control groups. Its disabled aircraft tab does not prove complete aircraft interaction rules. HeroHour keeps unavailable aircraft visibly disabled and preserves all five implemented building roles. No rule here is claimed to be recovered Tempest source code.

## Historical baseline and changes already implemented

The earlier inspected `Art/Previews/ReferenceV9-Package-Aegis-1600.png` is the **pre-redesign baseline**: bottom-right five-column cards, flat V3 panels and no member portraits. It must not be mistaken for the latest UI or reused as acceptance evidence for this redesign.

The original audit recommended three changes. They are now implemented to the following extent:

1. **Reference-derived hierarchy and skin:** the new radar/sidebar/primary-tab layout and state treatments replace the old arrangement. Final visual likeness, fine typography and all dense states still need image acceptance rather than a geometry-only claim.
2. **Membership selection:** up to nine living own selected members have individual health/portrait cards. Click chooses one; Shift-click removes it. Larger selections retain all gameplay members but show a remaining-count label.
3. **Queue ownership:** clicking an aggregate queue slot selects its real producer without changing orders or credits. Cancellation keeps the explicitly documented last-order-per-selected-producer rule. All orders remain in the backend; the aggregate strip displays at most five. These are no longer merely proposed features.

## Behavioral acceptance matrix

Run both factions at real 1280×720 and 1600×900 viewports. Keep test fixtures explicitly labelled. Input should travel through the controller/HUD public click path. Verify changes to game state, camera, wallet and selection; do not merely assert that draw and input share the same rectangle.

| Scenario | Input and setup | Required independent outcome |
|---|---|---|
| Tabs and selection | Click all five tabs; deselect; select HQ, factory and barracks; return to prior tab | Correct category and visible roster; no spending or placement merely on tab switch; actual card action routes to a compatible own producer; unavailable card never charges |
| Queue owner and completion | Two own factories with distinct queued roles and an HQ collector order; click production in each producer context | Only intended lane changes, correct paid prices, independent active progress and completion; production does not silently transfer to first selected building |
| Cancel | Queue at least two differently priced orders, advance first order, cancel once then repeatedly through empty | Defined order removed; refund equals its stored paid cost exactly once; unrelated producer and completed units remain unchanged; rendered queue updates |
| Low-power transition | Operational radar, defense and active factory queue; disable enough power supply through actual power UI; restore | Supply/demand warning changes; radar background/contacts disappear and minimap cannot pan; defense stops and queue advances at documented reduced rate; restoration resumes without lost orders or duplicate charges |
| Radar navigation | Radar online, click known interior minimap points, map frame/title/footer and then radar off | Actual camera center maps to intended world point within clamp tolerance; frame/title/footer do not pan; offline map consumes clicks without moving world selection/camera or leaking enemy information |
| Multi-selection | Select infantry + vehicles including damaged members; remove one, kill one, change to a single member | Membership/count/health agree with surviving selected actors; deterministic portraits; available group commands only affect supported members; no stale destroyed references |
| Abilities and cooldown | Real visible target for grenade; trigger once, immediately repeat, lose target, wait cooldown | One successful activation and effect; repeated disabled click cannot bypass cooldown; reason distinguishes cooldown from unsupported/no target; countdown and ready state agree with state |
| Commander power | Insufficient points, sufficient points, valid/invalid target, cancel targeting, active global cooldown | No charge for arming/cancelling/invalid target; one exact charge for valid effect; shared cooldown blocks both entry points and keyboard; no pending aim after pause/outcome/placement conflicts |
| Pause and match end | Attempt queue, sale, power switch, ability and doctrine while paused/after outcome | State and wallet unchanged; disabled visuals match actual rejection; no action leaks through into world commands |
| Tooltip and overflow | Hover costly/locked card, full queue, damaged unit, cooldown and top-right commander at both resolutions | Full useful name, cost/time and specific reason; panel entirely in viewport; no clipped last line, illegible text, tooltip covering the hovered target, or font reduction below accepted minimum |
| World input isolation | Click opaque panel gaps, borders, inactive cards and central terrain; drag selection from terrain | UI consumes its intended hit region; terrain remains interactive; no accidental movement/selection/placement behind UI; panel geometry matches visible outlines |

## Evidence required before calling the UI complete

Archive real packaged captures for: empty selection; single infantry with suppression; damaged vehicle; mixed selection; selected producer with active and pending queue; radar online; low power; tooltip near an edge; ability cooldown; commander targeting. At least the dense queue/mixed-selection/tooltip states must be captured at both target resolutions, for both faction styles where different. Keep fixture captures separate from natural match captures.

Pair those images with fresh runtime test logs from the same packaged build and the exact executable/package identity. Review the images against the user's HUD concept and REA reference, including proportions, portrait treatment, spacing, contrast and state hierarchy. Passing existing 43 assertions or loading a frame texture alone is insufficient to claim visual completion. No broad game-complete claim follows from UI acceptance.

## Test implementation handoff

### Verified Editor results, 21:10–21:11 Berlin

Inspected the actual logs after Editor Build 5. Both complete with `UI_TEST_RESULT=PASS`, **72 PASS assertions and zero failed assertions each**:

| Faction / actual viewport | Log | Status |
|---|---|---|
| Aegis 1600×900 | `Saved/Logs/UITest-Aegis-20261016-211041-473-20f883c7.log` | PASS, 72 checks |
| Ember 1280×720 | `Saved/Logs/UITest-Ember-20261016-211051-796-4323b386.log` | PASS, 72 checks |

The command lines independently show `-ResX=1600 -ResY=900` and `-ResX=1280 -ResY=720`, respectively. This is Editor game execution, **not a newly verified Windows package**. The remaining real cross-resolution runs (Aegis 1280, Ember 1600) and fresh package results are pending. No additional Unreal jobs were started by this audit while capture/package work owns the runtime slot.

These results supersede the earlier pending-runtime statements below for this exact source/build checkpoint. Covered behavior includes the real modal pause/resume, sidebar rejection during pause/victory, restart through the result modal, the shared control groups, multiple producer queue inspection/refund, and selection member clicks. Historical 43-check V7 results remain historical.

`PrototypeUITest.cpp` was subsequently updated under assigned ownership for the new Tempest layout contract: `PrimaryTab(0/1)` toggles build/doctrine view; category 0–3 remains functional; category 4 is unavailable aircraft and preserves context and wallet. All five real building-role placement checks remain. The obsolete single-row requirement was removed while panel containment and card overlap checks remain. Added live radar power-button off/on checks, offline minimap camera rejection, and repeated commander activation rejection during active global cooldown through both HUD entry points. These source changes have passed `git diff --check` only; compilation/runtime execution is the root integrator's next step. Do not label them passed until fresh logs exist.

`PrototypeControlGroups.cpp` supplies the shared keyboard/HUD group command. Slots display 1–9,0 and map to group numbers accordingly. Ctrl stores only living own unique members; normal recall replaces selection; Shift recall extends it; rapid repeated recall focuses the actual camera. Weak/dead/foreign members are excluded from counts and recall. Invalid numbers, pause and completed matches reject mutation. New UI checks exercise the actual seventh HUD slot and double-click focus, plus public-command extend, filtering, death and pause behavior. The root integrator owns header declarations, HUD routing and keyboard routing; no duplicate implementation was added to those files by this audit agent.

The next source pass adds two independent paid factory orders, clicks the second aggregate queue slot to select its actual producer without wallet/order-count mutation, and cancels through the HUD to verify only that producer is refunded. Queue ownership remains inspect/select plus existing last-order cancellation, not arbitrary order removal. A real mixed-selection portrait click chooses the actual second selected member without spending credits. Geometry checks now include all vertical tabs, both primary tabs and every queue slot against every building card. These additions are still pending integration runtime results.

### Remaining coverage and feature limitations after Build 5

- The suite validates manual radar shutdown/restoration through the UI. It does not yet validate a complete **supply-shortfall** transition, actual production progression across that transition and defense suspension in the same UI test. Separate systems coverage must not be relabelled as a full UI playback.
- Actual commander target cancellation/invalid targets and alternate keyboard input are not covered by every new scenario. Successful target/payment and immediate cooldown-repeat rejection are covered.
- Group extension uses the shared public command directly. The seventh slot and double-click focus use actual HUD hit routing. Ctrl/Shift physical input injection and Shift-click removal from the membership grid are not exercised by this suite.
- Membership display exposes nine members; larger selections show a remaining-count label without paging or access to every member through portraits. Queue inspection exposes five orders; aggregate queues beyond five have no paging. These are real completeness limits, not failed assertions hidden by the new tests.
- The queue remains last-order cancellation on the selected producer. Inspecting an aggregate slot selects the producer, after which a cancel removes that producer's latest order, which may differ from the originally inspected order. The tooltip must state this rule; arbitrary order cancellation was deliberately not introduced.
- Tooltip edge clipping, minimum text size, hover/disabled styling, full roster image quality and final likeness still require actual image review. A geometry PASS cannot establish those qualities. Empty selection, damaged unit, cooldown, commander aim and dense state captures remain important acceptance evidence.
- Result restart is checked, but the paused menu's separate restart action is not independently tested. Final real-time pause-freeze coverage remains intact.

### Are the display caps gameplay blockers?

**More than nine selected members:** this is a portrait inspection/subselection limit, not a group-command cap. `Controller->Selected` still holds all members; commands and ability dispatch iterate the full selection. Units outside the first nine can still be chosen in the world, by selection rectangle or control-group recall. The smallest useful improvement is portrait paging with previous/next and a page count, preserving the full underlying selection and mapping every page's click to its actual member. No change to combat or selection storage is required. For typical larger RTS groups this is worthwhile, but it does not stop movement/combat today.

**More than five aggregate queue orders:** every selected producer is limited to five backend orders, so selecting that building makes its whole lane inspectable. With no selected producer, the first five aggregate orders can hide other producers' pending work; those orders continue progressing and their producer can still be selected in the world. Thus production/cancellation remains possible, while an all-bases overview is incomplete. The smallest improvement is aggregate-only paging with a count such as `1-5 / 12`, retaining producer-filtered display and the same last-order cancellation rule. A more ambitious per-type sidebar lane is a separate economy/UX decision and is not required to solve this display gap.

Neither cap should be advertised as complete large-army/multi-base UI coverage. Both can be improved without silently changing command semantics. No source changes were made for these suggestions during package verification.
