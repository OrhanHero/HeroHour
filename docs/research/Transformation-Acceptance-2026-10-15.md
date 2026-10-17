# HeroHour transformation acceptance — 2026-10-15

Evidence checkpoint: 2026-10-15, Europe/Berlin. This records the implemented HeroHour prototype and its actual final packaged tests. It does not claim byte parity or graphical equivalence with Tempest Rising.

## Packaged acceptance

All 19 invocations of the **13:45 Berlin package** passed: 14 Aegis suites with 446 assertion gates and 5 Ember suites with 73 gates, totaling **519 assertions and 46 natural rounds**. Each faction completed 3 Match rounds plus 20 LogisticsStress rounds at 60-Hz simulation steps. Round results are separate from assertion counts. Exact local logs under Saved/Logs are listed below; the versioned machine-readable record is [Transformation-Checks.json](../verification/2026-10-15/Transformation-Checks.json).

Package CreatedUtc: `2026-10-15T11:45:50.6583192Z`. Build log: `Packaging-20261015-134515-393.log`. Executable SHA-256: `236b68194a924009475c9c45ced1cb73534ffa22167bd16b026bbfef40d2a6f2`.

| Suite | Fraktion | Ergebnis | Log unter Saved/Logs |
|---|---|---|---|
| Combat | Aegis | 17 Prüfpunkte PASS | CombatTest-20261015-134615-941.log |
| Economy | Aegis | 102 Prüfpunkte PASS | EconomyTest-20261015-134616-073.log |
| Navigation | Aegis | 20 Prüfpunkte PASS | NavigationTest-20261015-134616-227.log |
| Match | Aegis | 3 natürliche Partien PASS | MatchTest-20261015-134616-388.log |
| UI | Aegis | 22 Prüfpunkte PASS | UITest-20261015-134616-569.log |
| Architecture | Aegis | 63 Prüfpunkte PASS | ArchitectureTest-20261015-134616-742.log |
| Resource | Aegis | 18 Prüfpunkte PASS | ResourceTest-20261015-134616-921.log |
| Docking | Aegis | 28 Prüfpunkte PASS | DockingTest-20261015-134617-130.log |
| Tactical | Aegis | 46 Prüfpunkte PASS | TacticalTest-20261015-134617-315.log |
| Visibility | Aegis | 25 Prüfpunkte PASS | VisibilityTest-20261015-134617-519.log |
| Systems | Aegis | 54 Prüfpunkte PASS | SystemsTest-20261015-134617-733.log |
| FullMatch | Aegis | 17 Prüfpunkte PASS | FullMatchTest-20261015-134617-955.log |
| Presentation | Aegis | 34 Prüfpunkte PASS | PresentationTest-20261015-134618-177.log |
| LogisticsStress | Aegis | 20 natürliche Partien PASS | LogisticsStressTest-20261015-134618-395.log |
| FullMatch | Ember | 17 Prüfpunkte PASS | FullMatchTest-20261015-134618-601.log |
| Match | Ember | 3 natürliche Partien PASS | MatchTest-20261015-134618-843.log |
| UI | Ember | 22 Prüfpunkte PASS | UITest-20261015-134619-154.log |
| Presentation | Ember | 34 Prüfpunkte PASS | PresentationTest-20261015-134619-419.log |
| LogisticsStress | Ember | 20 natürliche Partien PASS | LogisticsStressTest-20261015-134619-685.log |
Natural Aegis Match outcomes were win/win/win; Ember outcomes were win/loss/loss. LogisticsStress completed 20/20 rounds per faction; its 60-Hz steps specifically exercise the frame-step-sensitive collector regression. These simulation tests do not replace real elapsed-time RHI soaks. The FullMatch suite separately verifies victory, defeat and restart cleanup through actual gameplay fixtures.
## Economy and power recovery

[PrototypeEconomy.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeEconomy.cpp) runs three 16-cell regrowing fields with one stock authority, independent cell reservations and real collector movement to exclusive FIFO refinery docks. Regrowth keeps the intended whole-field budget rather than multiplying it by 16. Acceptance verifies every pickup route, four simultaneous collectors, per-tick stock+cargo+wallet conservation and interrupted delivery recovery.

The previously missing active-transfer pause case is now covered. A collector reaches a genuine dock, unloads 20 credits, then real world pause freezes cargo, wallet, operation identity, exclusive dock ownership and simulation time. Resume transfers at 40 credits/second without catch-up or duplicate payment. Manually disabling the same refinery blocks transfer; restoring power resumes the retained load and pays it once. These checks are included in Economy 102 and close the earlier D5 coverage gap.

[PrototypeSystems.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeSystems.cpp) connects authored power supply/demand to production, radar and defenses. A complete offline grid can recover through a **paid replacement power plant in its own construction lane**, even when an earlier base-building job is frozen. Its completion restores supply and resumes the blocked lane. Systems 54 also checks that paused public commands cannot spend money, apply damage, start abilities, change power state or sell structures.

## Grid, combat and match progression regressions

The shared building rectangle now drives placement, navigation clearance, refinery approaches and producer exits. Systems tests cover a mobile actor and a resource cell at a building rectangle's opposite corner; neither can be engulfed through the former smaller circle gate. Navigation 20 verifies rotated rectangle edges, rounded corners, circle-fixture compatibility, three analytic corner/waypoint regressions and two bridge-bank cases. [Grid-Nav-Integration-Review.md](C:/Users/Hero/HeroHour/docs/research/Grid-Nav-Integration-Review.md) records dimensions and the exact correction.

A genuine match stall was traced to a legal HQ perimeter point being inside a neighboring radar/power plant. Units were walkable but their destinations were not. [PrototypeNavigation.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeNavigation.cpp) now searches up to 32 facing-first perimeter candidates and validates a real path with every obstacle retained. [PrototypeGameMode.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeGameMode.cpp) caches the accepted point per weak target and revalidates it after obstruction or route failure. The first observed Ember package had one win and two 600-second stalls; the first post-fix Editor run ended all three games in 134–154 seconds. The final packaged runs above also all ended normally.

[HRCombat.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/RTS/HRCombat.cpp) and its runtime integration implement authored typed damage, veterancy, infantry cover/suppression and active grenade/smoke/deployment abilities. Tactical 46 and Systems 54 exercise actual gameplay effects and authority guards. These are HeroHour-authored values; they are not asserted to reproduce extracted Tempest balance.

## Presentation evidence

[HRPresentationTests.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/RTS/HRPresentationTests.cpp) has 34 gates in both faction packages. A real spawned infantry actor displays its skeletal component; the static fallback is hidden. Imported bounds are near 90×70×180 cm with the authored−35 cm origin. Idle 2s, Walk 0.8s and Fire 0.3s clips use the actual skeleton. Evaluated left/right feet change between 0 and 0.2s, and the spine rotates at the 0.045s recoil peak; actor origin remains stable. The 0.2s fire pose is intentionally already recovered, so it is not used as false evidence of missing animation.

The six original FrontlinesV4 meshes, three effect materials and three original positive-duration sound waves load from packaged content. Every one of the 11 runtime roles is configured after the default vehicle configuration and then checked for the **identity of each authored material slot**, preventing inherited vehicle MID overrides from silently corrupting building/infantry materials. Checking only the material namespace would not have caught that defect. The source clears previous overrides before role reconfiguration.

The final UI 22 gates cover the actual interface; these are functional checks, while rendered concept similarity also requires the independently captured screenshots. [HeroHUD.cpp](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/HeroHUD.cpp) and [PrototypeHUDLayout.h](C:/Users/Hero/HeroHour/Source/HeroHourPrototype/PrototypeHUDLayout.h) are the active implementation, not a detached mockup.

## Completed long-running verification

Both real-RHI soaks of the 13:45 package completed and returned PASS at approximately 14:06:45 Berlin. Aegis: 1200.009 wall seconds, 7 natural rounds, mean 59.647 FPS, P95 16.7355 ms, worst 285.8052 ms. Ember: 1200.005 seconds, 8 natural rounds, mean 59.620 FPS, P95 16.7517 ms, worst 156.0525 ms. Two instances ran concurrently at 1600x900 with a 60-FPS cap; packaged acceptance workers also ran during the opening minutes. These are not isolated maximum-performance measurements. No collector-stall error was observed in either completed run.

Versioned evidence: [summary](../verification/2026-10-15/Soak-Acceptance.json), [Aegis](../verification/2026-10-15/Soak-Aegis.json), [Ember](../verification/2026-10-15/Soak-Ember.json). Raw local logs: Soak-Aegis-20261015-134643-285.log and Soak-Ember-20261015-134643-406.log.

A successful prototype acceptance does not mean commercial-quality art, campaign content, multiplayer determinism or full Tempest Rising parity. Reverse-engineering observations and original HeroHour design choices remain distinguished in the research documents. No original game models, textures or protected payloads are incorporated by this change.

## Historical 09:27 failure and post-correction scope

The earlier package completed both 1,200-second real-RHI runs, but Aegis returned FAIL after 7 completed rounds while Ember returned PASS after 8. Their retained reports are [Aegis regression](../verification/2026-10-15/Soak-Aegis-0927-regression.json) and [Ember](../verification/2026-10-15/Soak-Ember-0927.json). They are historical evidence and must not be presented as acceptance of the current package.

A pre-fix 10-Hz simulation passed 20 rounds; the 60-Hz reproduction failed in round 1 at 131.579 simulation seconds (Saved/Logs/LogisticsStress-60Hz-BeforeFix-20261015-0955.log). The captured diagnostics showed 43 reachable positive resource cells and a reachable operational refinery, contradicting the disconnected-enclave explanation for this failure.

The correction uses analytic whole-segment distance against rounded rectangle and circle obstacles and permits skipping a nearby intermediate waypoint only when the entire replacement chord is legal. Current Navigation 20 and both 20-round 60-Hz LogisticsStress suites passed in the 13:45 package. That is verified regression coverage; the independently completed 20-minute RHI results above also pass.
