# Build grid and navigation integration review

Status as of 2026-10-15, 13:40 Europe/Berlin: initial geometry corrections passed the short packaged suites, but the 09:27 package failed the Aegis 20-minute soak. A further analytic-segment and waypoint correction is implemented and being rebuilt; final post-correction acceptance remains pending.

## Original mismatch

Placement reserved rotated grid rectangles, while navigation used smaller role circles. Those circles did not contain the grid corners or every rendered building. Increasing every circle to the grid diagonal would have blocked otherwise usable streets and invalidated refinery approaches.

| Role | Mesh XY at runtime (cm) | Reserved grid XY (cm) | Mesh corner radius (cm) | Former nav circle (cm) |
|---|---:|---:|---:|---:|
| Headquarters | 1033.3 × 1033.3 | 1200 × 1200 | 730.7 | 600 |
| Refinery | 535.3 × 573.7 | 800 × 800 | 392.3 | 400 |
| Factory | 748.0 × 688.7 | 1200 × 1200 | 508.4 | 540 |
| Power plant | 480 × 440 | 600 × 600 | 325.6 | 300 |
| Barracks | 500 × 400 | 600 × 600 | 320.2 | 300 |
| Radar | 400 × 400 | 600 × 600 | 282.8 | 300 |
| Defense | 300 × 300 | 400 × 400 | 212.1 | 180 |

Measurements use ConceptV2 source manifests multiplied by runtime structure scales (HQ 1.5, refinery 1.5, factory 1.7), and FrontlinesV4 manifests at scale 1. The authored models fit their grid rectangles. Runtime presentation tests separately validate imported assets and infantry bounds; this table is source-geometry evidence, not a blanket runtime bounds assertion for every building.

## Implemented geometry

[BattlefieldLayout.h](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/BattlefieldLayout.h) defines rectangle half extents on navigation obstacles. Zero extents preserve radius-only fixtures. [PrototypeNavigation.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeNavigation.cpp) builds these extents from each existing building's role, faction and stored quarter-turn rotation. Alive construction sites block immediately.

Navigation expands the rectangle by the 80 cm vehicle radius using closest-point distance, producing rounded corners. Segment validation, diagonal checks and path smoothing all use this predicate. Water, bridge and static city constraints remain active. The finite 120×90-cell A* graph and finite candidate searches prevent unbounded retry loops.

`StructureApproachRadius` intersects a ray with the rectangle expanded by the requested clearance. `ProjectOutsideStructure` uses the same dimensions. Refinery docking, waiting points, production exits and movement projection consume this common geometry through [PrototypeEconomy.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeEconomy.cpp) and [PrototypeGameMode.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeGameMode.cpp). A manual command to an actually blocked structure center still rejects without replacing the existing order; invalid orders do not discard collector claims.

Placement rejects both building overlap and mobile/resource intersections at rectangle corners. It no longer relies on a smaller center circle to protect the opposite corner. A formerly invalid field layout was corrected: the east field uses anchorX 6880 and X offsets 0/−200/+200/+400, keeping all 16 cells between the city and HQ. All 48 actual pickups have reachable routes in the economy acceptance test.

## Neighbor-blocked HQ attack regression

Real Ember match testing exposed another defect after the shape correction. An attack point projected outside the HQ could lie inside a later-built neighboring radar or power plant. Units remained on legal ground but repeatedly requested an illegal destination, producing 600-second stalemates. The first packaged observation had one completed win and two stalled rounds.

`FindStructureApproach` now checks up to 32 perimeter candidates, beginning on the facing side and alternating angular directions. For every angle it computes the valid target rectangle standoff and actual firing-distance allowance, then requires a legal candidate and a real route with **all** obstacles retained. It does not ignore the target or neighboring buildings. Failure leaves the output point unchanged.

`APrototypeUnit::UpdateStructureApproach` caches the accepted point against a weak target reference, checks current legality and retries after route failure with bounded cooldown. This avoids running 32 path searches every frame while allowing a later building to invalidate the cached goal. Both strategic HQ movement and immediate structure attacks use the correction.

The first post-fix Editor Ember run (`MatchTest-20261015-091736-523.log`) ended all three rounds naturally:134.4s win,153.5s loss and 137.7s win. No test kills, wallet grants or forced outcomes were introduced. Final packaged verification is recorded in [Transformation-Acceptance-2026-10-15.md](C:/Users/Hero/HeroHour/docs/research/Transformation-Acceptance-2026-10-15.md).

## Verified gates and limits

Navigation passes 15 checks, including rectangle corners, rotated long/short sides, rounded clearance and legacy circle compatibility. Systems passes 54 checks, including mobile and resource placement at rectangle corners. Economy passes 102 checks, including actual movement, all 48 pickups, concurrent reservations, exact delivery, paused active unloading and manual refinery power recovery. Match simulations additionally check every actual movement segment.

These short-suite results cover the listed fixtures; they did not exclude the later frame-step-sensitive collector stall. They do not establish commercial-game parity, lockstep multiplayer determinism or final long-run acceptance. The completed 09:27-package soaks are recorded below.

## Completed soak regression and frame-step reproduction

The two concurrent real-RHI, capped-60-FPS, 1600×900 runs each completed 1,200 wall-clock seconds. Aegis completed 7 rounds but returned **FAIL** after a collector exceeded the 120-simulation-second no-progress threshold. Ember completed 8 rounds and returned **PASS**. Evidence: [Aegis report](../verification/2026-10-15/Soak-Aegis-0927-regression.json), [Ember report](../verification/2026-10-15/Soak-Ember-0927.json). The package is not finally accepted on the strength of Ember alone.

The accelerated 10-Hz test completed 20/20 rounds before the correction (Saved/Logs/LogisticsStress-BeforeFix-20261015-0953.log). Repeating with 60-Hz simulation steps failed in round 1 at match time 131.579 seconds, last progress 11.567 seconds (Saved/Logs/LogisticsStress-60Hz-BeforeFix-20261015-0955.log). Coarser simulation steps had hidden the defect.

For this reproduced stall, the enclave hypothesis is contradicted by runtime diagnostics: the collector and destination were walkable; routes to the destination, claimed resource cell and an operational refinery existed; all 43 positive cells were reachable, including 42 unclaimed reachable cells. This evidence does not prove every future construction layout connected, but it rules out a disconnected enclave as the explanation of this captured failure.

## Current correction awaiting final acceptance

PrototypeNavigation.cpp now computes analytic segment-to-rectangle distance, retaining the rounded 80 cm vehicle clearance, and analytic segment-to-circle distance. This closes narrow grazing intersections missed by discrete sampling. Skipping a nearby intermediate waypoint additionally requires the entire replacement chord to be legal, rather than relying on endpoint or waypoint proximity alone.

Three added regression fixtures cover a grazing rounded corner missed by 40 cm samples, a genuinely clear tangent that must remain usable, and a nearby bend whose replacement chord cuts building clearance. These are implemented fixtures, not a claim that a final packaged rerun already passed.

At 13:40 Berlin, Root is rebuilding and testing this correction. Post-correction packaged suites, 60-Hz multi-round stress and fresh long-run acceptance must be recorded from actual results; they are still pending in this review.
