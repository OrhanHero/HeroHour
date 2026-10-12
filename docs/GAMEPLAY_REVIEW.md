# Gameplay review: HERO prototype

Reviewed `Source/HeroHourPrototype/PrototypeGameMode.cpp` and `.h` and the urban scene import on 11 October 2026. This is a source review, not confirmation of an interactive playtest.

## Implemented in this pass

- Camera input follows the camera component's world yaw.
- Explicit move commands suppress automatic acquisition until arrival.
- HQ attack orders preserve a strategic target and allow nearby vehicle combat before resuming the HQ attack.
- Destinations and moving vehicles stay within the road's y = +/-500 corridor.
- Vehicles steer around headquarters and are projected outside a 400-unit HQ footprint; move destinations inside a headquarters are redirected outside it.
- Local vehicle separation increased to 160 units.
- Mouse input over the top and bottom HUD panels is consumed instead of issuing hidden world commands.

These source edits have not been compiled or interactively verified by the review agent; the primary agent owns build and runtime validation.

## High-impact issues

1. **Move orders are cancelled by automatic pursuit.** `OrderMove` clears the target but scanning resumes after 0.6 seconds. A vehicle near an enemy therefore reacquires it and abandons retreat. A move-order flag should suppress acquisition until the destination is reached. Explicit attack orders should clear that flag.
2. **Attackers ignore defending vehicles.** Enemy waves receive an explicit HQ target. Since scanning only runs when `AttackTarget` is null, defenders are ignored even while firing into the wave. Allow HQ-targeting vehicles to engage a nearby hostile vehicle, then resume their HQ objective once that local opponent is destroyed.
3. **Movement does not respect physical obstacles.** Vehicles move using `SetActorLocation` without sweep; the root is a scene component, so adding sweep alone will not introduce useful collision. Full pathfinding is absent. For this road-based vertical slice, constrain orders and movement to the clear road corridor, and provide local HQ avoidance. A later free-form map needs actual obstacle-aware navigation.
4. **Camera controls are world-relative.** With the new 45-degree view, W and D no longer correspond to screen-up and screen-right. Use the camera component's world yaw (not just the pawn rotation) to rotate the input axes.
5. **Command traces can hit scenery.** Right-click uses visibility traces. With imported props, a hit on a roof can produce a destination inside a building. Constrain movement destinations to playable space and keep scenery outside it.

## Existing test scope

`RunCombatTest` verifies ranged damage, automatic enemy acquisition, insufficient funds, tank cost, outcome freeze, reset state, army replacement, and loss. It does not verify cursor selection, drag selection, camera framing, rendered art, moving to a clicked location, obstacle avoidance, retreat, or an enemy's response to defenders. Those claims need separate evidence.

## Interactive acceptance pass

- Start the game and confirm the player's base and vehicles are visible without camera adjustment.
- W moves toward screen-up; D moves toward screen-right at all zoom levels.
- Single-click and drag-select vehicles; hold Shift to add another selection.
- Order a formation along the road. Vehicles remain in traversable space and stay distinguishable.
- Order retreat while enemies are within 650 units. The vehicles obey the move command until arrival.
- Attack the opposing HQ. Defending vehicles engage attackers; attackers can respond to defenders.
- Purchase reinforcements; inspect the spawn area for overlapping units.
- Finish the match and restart. Camera, credit balance, unit counts, selection, and outcome reset correctly.

## Minimal camera-relative movement patch

```cpp
const UCameraComponent* View = Camera->FindComponentByClass<UCameraComponent>();
const FRotator Yaw(0.f, View ? View->GetComponentRotation().Yaw : Camera->GetActorRotation().Yaw, 0.f);
const FVector Forward = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
const FVector Right = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
const FVector Direction = Forward * (IsInputKeyDown(EKeys::W) - IsInputKeyDown(EKeys::S))
    + Right * (IsInputKeyDown(EKeys::D) - IsInputKeyDown(EKeys::A));
```

The new road scene places buildings at y = +/-1330 and most decorative obstacles at |y| >= 760. A corridor near |y| <= 500 can be used for this prototype's road combat, provided base geometry is separately handled. This is a deliberate level constraint, not general navigation.
