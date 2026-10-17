# Suppression threshold precision

16 October 2026. This is a targeted HeroHour numerical fix, unrelated to Tempest Rising recovered code or UI design.

## Observed failure

The first new package run logged two failures in `Saved/Logs/TacticalTest-Aegis-20261016-212100-007-e4d4bec9.log`:

- `20 percent health damage reaches infantry suppression threshold`
- `suppression does not recover during two second hit grace period`

The other 44 tactical assertions passed. Both failures occur on a local `FHRCombatState` instance before world actors/cover fixtures are created. Consequently the new factory footprint, terrain and cover detection cannot cause these two failures. The exact stored failing float was not printed in that log, so its bit pattern is unknown.

The original expression was `Suppression + ActualDamage / MaxHealth * 200.f`. The existing threshold is `Suppression >= 40.f`, and the two failed assertions require exactly `40.f`. A reciprocal-multiply float32 calculation of `20 / 100 * 200` produces `39.999996185302734`; a rounded division-first float32 calculation produces `40.0`. This is a bounded arithmetic explanation consistent with the failing threshold, not a captured bit pattern from the failing package.

## Actual compiler evidence and implementation

The package response file `Intermediate/Build/Win64/x64/UnrealGame/Development/HeroHourPrototype/HeroHourPrototype.Shared.rsp` contains `/fp:fast` and `/Ob2`. A standalone CPU-only reproduction used the same installed MSVC 14.50.35717 compiler with `/O2 /fp:fast`. Explicit double casts alone were optimized back to `divss`/`mulss`, so merely adding double casts does not guarantee double arithmetic under these flags. That first standalone program did **not** reproduce the package's below-threshold result; its assembly only proves the narrowing optimization.

With a function-local `#pragma float_control(precise,on,push)` / `#pragma float_control(pop)`, the standalone assembly instead contains `cvtss2sd`, `divsd` and `mulsd`. The bounded reproduction returned exit 0 for the new function returning exactly float `40.f`. Source/assembly remain under `Saved/Research/SuppressionPrecision/`. This is not an Unreal runtime pass or proof that every target compiler behaves identically.

`FHRCombatState::AddSuppression` now calculates the damage fraction, accumulation and clamp in double precision, then stores a float. An `_MSC_VER`-guarded local precise-FP pragma prevents the verified narrowing optimization while preserving the module's existing compilation settings. Non-MSVC builds retain the portable double implementation. Recovery timing, threshold, movement penalty, damage balance and invalid-input checks are unchanged.

## Regression checks

The two originally failing checks remain unchanged: exactly 20% damage must yield suppressed state and exactly 40 points; advancing two one-second ticks must retain exactly 40 during the grace period. The existing next-half-second check still requires 31 points and recovery out of suppression.

Four additional assertions cover:

1. 19.99% damage remains below the threshold, approximately 39.98 points.
2. 20.01% damage exceeds it, approximately 40.02 points.
3. Maximum finite damage divided by minimum positive normal float health clamps to finite 100 points.
4. Two 10% hits reach exactly the same 40-point threshold.

No assertion was relaxed or removed. Expected tactical assertion count increases from 46 to 50. `git diff --check` passed; **fresh Unreal build/package tactical acceptance is pending** at this documentation checkpoint. Root owns the rebuild and verification; the standalone compiler evidence is not substituted for those tests.
