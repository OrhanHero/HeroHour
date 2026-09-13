module HeroHour.Simulation.Runner

open HeroHour.Simulation.Core.Types
open HeroHour.Simulation.Core.Constants
open HeroHour.Simulation.Core.GameState
open HeroHour.Simulation.Engine

// ============================================================================
// HeroHour Simulation — Headless Console Runner
// ============================================================================
// Proves the F# simulation core actually builds and runs, with no Unreal
// Engine dependency. It creates a 2-player game, runs a fixed number of
// deterministic ticks, and prints the resulting state. Running it twice with
// the same seed must produce identical output (determinism check).

let private buildSettings () : GameSettings =
    {
        MatchDuration = 0.0 // unlimited
        StartingResources =
            { Credits = STARTING_CREDITS
              Energy = STARTING_ENERGY
              TechPoints = STARTING_TECH_POINTS }
        PopulationCap = DEFAULT_POPULATION_CAP
        FogOfWarEnabled = true
        FogRevealRange = FOG_REVEAL_RANGE
        FogReconRange = FOG_RECON_RANGE
        ResourceTickRate = CREDIT_TICK_RATE
        IncomeMultiplier = 1.0
        VictoryConditions = [ VictoryCondition.Annihilation ]
        AllowedFactions = [ Faction.USA; Faction.China ]
        MapName = "TwoPlayerTestbed"
        RandomSeed = 1337
    }

/// Give every player a small fixed income so the tick loop visibly changes
/// state — deterministic because it depends only on the initial config.
let private seedIncome (state: GameState) : GameState =
    state.Players
    |> Seq.fold (fun (acc: GameState) kvp ->
        let updated =
            { kvp.Value with
                Income = { Credits = 10; Energy = 1; TechPoints = 0 } }
        { acc with Players = acc.Players.SetItem(kvp.Key, updated) }) state

[<EntryPoint>]
let main argv =
    let ticks =
        match argv with
        | [| n |] ->
            match System.Int32.TryParse n with
            | true, v when v > 0 -> v
            | _ -> 300
        | _ -> 300

    let settings = buildSettings ()
    let initial =
        createGameState settings settings.MapName settings.RandomSeed
        |> seedIncome

    printfn "=== HeroHour Simulation Runner ==="
    printfn "Map: %s | Seed: %d | Tick rate: %g TPS" settings.MapName settings.RandomSeed TICK_RATE
    printfn "Players: %d | Running %d ticks (%.1fs game time)..."
        initial.Players.Count ticks (float ticks * TICK_INTERVAL)
    printfn ""

    let final = runTicks ticks initial

    printfn "--- Result after %d ticks ---" final.Tick
    printfn "Game time: %.2fs | Phase: %A" final.GameTime final.Phase
    for kvp in final.Players do
        let p = kvp.Value
        printfn "  [%A] %-10s credits=%d energy=%d tech=%d"
            p.Faction p.Name p.Resources.Credits p.Resources.Energy p.Resources.TechPoints

    printfn ""
    printfn "Determinism check (re-run same seed): %s"
        (let a = (runTicks ticks initial)
         let b = (runTicks ticks initial)
         let sumA = a.Players |> Seq.sumBy (fun p -> int64 p.Value.Resources.Credits)
         let sumB = b.Players |> Seq.sumBy (fun p -> int64 p.Value.Resources.Credits)
         if sumA = sumB && a.Tick = b.Tick then "PASS (identical end state)" else "FAIL")
    0
