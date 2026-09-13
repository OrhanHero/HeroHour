module HeroHour.Simulation.Engine

open HeroHour.Simulation.Core.Types
open HeroHour.Simulation.Core.Constants
open HeroHour.Simulation.Core.GameState
open HeroHour.Simulation.Core.GameCommand
open HeroHour.Simulation.Core.SimulationResult

// ============================================================================
// Deterministic Tick Engine (minimal, working core)
// ============================================================================
// This is intentionally small. It is the first end-to-end runnable piece of
// the simulation: advance time, apply per-tick economy income, and report a
// SimulationResult. Combat/AI/production live in future milestones (see
// ROADMAP.md). Everything here is pure given the same inputs -> deterministic.

/// Apply passive resource income to every non-defeated player for one tick.
let private applyIncome (state: GameState) : GameState =
    state.Players
    |> Seq.fold (fun (acc: GameState) kvp ->
        if kvp.Value.IsDefeated then acc
        else addResources acc kvp.Key kvp.Value.Income) state

/// Advance the simulation by exactly one deterministic tick.
let tick (commands: GameCommand list) (state: GameState) : SimulationResult =
    // 1. Validate + (for now) acknowledge commands. Command *effects* on the
    //    world are implemented incrementally in later milestones; here we just
    //    keep the pipeline honest by validating them.
    let _validated =
        commands |> List.map validateCommand

    // 2. Advance time deterministically.
    let advanced = updateGameTime state TICK_INTERVAL

    // 3. Apply economy income.
    let withIncome = applyIncome advanced

    // 4. Re-evaluate victory.
    let winner = checkVictory withIncome
    let finalState =
        match winner with
        | Some w -> { withIncome with Winner = Some w; Phase = GamePhase.Ended }
        | None -> withIncome

    {
        NewState = finalState
        Events = []
        VisualEffects = []
        AudioEvents = []
        NetworkUpdates = []
        PerformanceMetrics = {
            TickDurationMs = 0.0
            SimulationTimeMs = 0.0
            EventCount = 0
            UnitCount = finalState.Players |> Seq.sumBy (fun p -> p.Value.Units.Count)
            BuildingCount = finalState.Players |> Seq.sumBy (fun p -> p.Value.Buildings.Count)
            MemoryUsageMB = 0L
            GCCollections = 0
        }
    }

/// Run N ticks with no commands, returning the final state. Useful for tests
/// and for demonstrating determinism (same seed -> identical end state).
let runTicks (n: int) (state: GameState) : GameState =
    let mutable current = state
    for _ in 1 .. n do
        current <- (tick [] current).NewState
    current
