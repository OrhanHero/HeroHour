# HeroHour — Roadmap

> **Purpose of this document.** This is the *honest* build plan for HeroHour. It
> describes what actually exists today, and a realistic, milestone-by-milestone
> path from here to a playable game. It deliberately favors **one small thing
> that works** over a large thing that only exists on paper.
>
> If you read only one section, read **"Reality Check"** below.

---

## Reality Check (read this first)

HeroHour today is a **very ambitious vision plus a small runnable simulation core** — not a game in progress. Being clear about that is the single most important thing for finishing it.

**What exists and works today:**
- A deterministic F# simulation *core* that **compiles and runs** (`Source/HeroHourSimulation`).
- A headless console runner that creates a 2-player match, advances 300 fixed ticks, accumulates economy, and verifies determinism (same seed → identical end state).
- 3 passing unit tests (Expecto).
- Extensive design documentation (`docs/gdd`, `docs/technical`) and concept art (`Art/`).
- An EOS (Epic Online Services) C++ subsystem skeleton.

**What does NOT exist yet (despite older docs implying otherwise):**
- ❌ No Unreal Engine project (`.uproject`), no `.sln`, no `paket.dependencies`, no `global.json`.
- ❌ No units, no combat, no production, no pathfinding, no rendering — nothing visual.
- ❌ No networking / lockstep implementation (only the design).
- ❌ No map editor, no superweapons, no super-agents in code.
- ❌ Only 3 factions exist as a type (`USA | China | GLA | Neutral`); the "10 asymmetric world powers" are design, not code.

### The scope problem, stated plainly

The originally described project — AAA RTS, 1000+ units, deterministic lockstep netcode, F#/UE5 dual-core engine, 10 fully asymmetric factions each with a unique superweapon + hero, in-game map editor, EOS, e-sports balance — is realistically a **multi-year effort for a funded team of dozens**. *Command & Conquer: Generals Zero Hour*, the inspiration, was built by a large professional studio.

For a solo or small developer starting **with no assets**, pursuing that full scope directly is the most common way passion projects die: enormous effort, nothing playable. This roadmap exists to avoid that outcome.

### Guiding principles

1. **Always have something that runs.** Every milestone ends with a runnable, testable artifact.
2. **Vertical slice before breadth.** Get *one* complete match loop working before adding factions, superweapons, or netcode.
3. **Placeholders are fine.** Cubes and capsules now; art later. Free asset packs (Kenney, UE Marketplace free, Mixamo) are legitimate.
4. **Determinism is a feature you protect from day one** — it's cheap to keep, expensive to retrofit.
5. **Cut ruthlessly.** Anything not needed for the current milestone is deferred, not deleted from the vision.

---

## Milestone 0 — Runnable Core ✅ (DONE)

*Goal: prove the simulation actually builds and runs.*

- [x] Fix F# compile errors in the core (`Types`, `GameState`, `SimulationResult`).
- [x] Reduce `.fsproj` to only files that exist.
- [x] Add a minimal deterministic tick engine (`src/Simulation.fs`).
- [x] Add a headless console runner (`runner/`).
- [x] Add passing unit tests (`tests/`).
- [x] Verify determinism (same seed → identical end state).

**Artifact:** `dotnet run` in `Source/HeroHourSimulation/runner` prints a 2-player economy simulation and a determinism PASS.

---

## Milestone 1 — Simulation Vertical Slice (headless)

*Goal: a full match loop in F#, no engine, no graphics. This is the heart of the game.*

Scope: **2 factions, ~5 unit types, 2–3 building types, 1 resource, 1 abstract map.**

- [ ] **Units:** spawn, store, and query units per player (`UnitManager`).
- [ ] **Movement:** simple grid or flow-field movement toward a target (deterministic).
- [ ] **Production:** buildings with a production queue that spends resources and spawns units over time.
- [ ] **Combat:** target acquisition + damage resolution using existing `DamageType`/`ArmorType`.
- [ ] **Win condition:** annihilation already wired; verify end-to-end.
- [ ] **Commands:** implement effects for `MoveUnits`, `TrainUnit`, `AttackTarget`.
- [ ] **Tests:** combat math, production timing, movement determinism.

**Exit criteria:** a scripted headless match where two AIs build, produce units, fight, and one wins — reproducibly from a seed.

---

## Milestone 2 — Minimal Engine Front-End

*Goal: see the simulation. Pick the pragmatic path.*

Decision to make first (see "Open Decisions"): **UE5 vs. a lighter engine (Godot/Unity) vs. a 2D debug renderer.** Recommendation for fastest feedback: start with a **2D top-down debug renderer** (even in the browser or a small UE5/Godot scene) that just draws unit positions from the F# state.

- [ ] Create the actual engine project (whichever engine is chosen) — this is the first time `.uproject`/project files should exist.
- [ ] Bridge: feed F# state out each tick (start with in-process/JSON, not the full async C++ bridge).
- [ ] Render units as primitives (cubes/sprites), colored by player.
- [ ] Basic camera + unit selection + issue Move/Attack orders via mouse.

**Exit criteria:** you can watch and control the Milestone 1 match on screen.

---

## Milestone 3 — Playable Single-Machine Match

*Goal: it's a game you can play against an AI.*

- [ ] Fog of war (types already exist; implement reveal logic).
- [ ] Resource gathering/economy loop that the player interacts with.
- [ ] A real (if simple) map with terrain and buildable areas.
- [ ] HUD: resources, selection, production buttons.
- [ ] One AI opponent good enough to be fun to beat.
- [ ] Placeholder art pass (free assets).

**Exit criteria:** a stranger can start the game, play a full 1v1 vs AI, and win or lose.

---

## Milestone 4 — Content & Identity

*Goal: make it feel like HeroHour, not a tech demo.*

- [ ] Expand from 2 to more factions **one at a time**, each fully playable before the next.
- [ ] First superweapon and first super-agent as real mechanics (start with ONE faction).
- [ ] Real art/audio replacing placeholders, incrementally.
- [ ] Balance pass on the existing factions.

---

## Milestone 5 — Multiplayer

*Goal: the deterministic-lockstep dream, now that there's a deterministic sim to carry it.*

- [ ] Input-only lockstep over LAN first (deterministic sim makes this tractable).
- [ ] Desync detection (hash the state each tick, compare).
- [ ] Online sessions via the existing EOS skeleton.
- [ ] Replays (seed + input history — the architecture already supports this).

---

## Milestone 6+ — The Full Vision

Everything else in the original design becomes a *backlog*, pulled in only after Milestone 5 is stable:
map editor, AoD wave system, remaining factions/superweapons/super-agents, combat-school analytics,
tournaments, clans, localization breadth, e-sports balancing.

---

## Open Decisions (resolve before Milestone 2)

1. **Engine choice.** UE5 is the stated target and matches the AAA visual goal, but the F#↔C++ bridge is a large solo risk. Options:
   - *Pragmatic:* prototype rendering in a lighter engine or a debug renderer; keep F# as the sim.
   - *Committed-to-UE5:* keep F# sim standalone and tested first; add the UE5 bridge last.
2. **Team & time budget.** Solo? Part-time? This dictates how aggressively to cut scope. Be honest here — the roadmap scales to it.
3. **Determinism boundary.** Confirm all gameplay-affecting math stays in F# and uses no floating-point nondeterminism across platforms (consider fixed-point for positions if cross-platform MP matters).

---

## How to run what exists today

```bash
cd Source/HeroHourSimulation/runner
dotnet run -c Release           # runs 300 ticks
dotnet run -c Release -- 600    # runs 600 ticks

# tests
cd ../tests
dotnet run -c Release
```

---

*This roadmap is a living document. Update it as milestones complete or scope changes. Honesty here is what keeps the project finishable.*
