module HeroHour.Simulation.Tests.SimulationTests

open Expecto
open HeroHour.Simulation.Core.Types
open HeroHour.Simulation.Core.Constants
open HeroHour.Simulation.Core.GameState
open HeroHour.Simulation.Engine

// Minimal, real tests for the current simulation core. As modules are added
// back (Combat, Economy, AI — see ROADMAP.md), add matching test files here.

let private baseSettings () : GameSettings =
    { MatchDuration = 0.0
      StartingResources = { Credits = STARTING_CREDITS; Energy = STARTING_ENERGY; TechPoints = STARTING_TECH_POINTS }
      PopulationCap = DEFAULT_POPULATION_CAP
      FogOfWarEnabled = true
      FogRevealRange = FOG_REVEAL_RANGE
      FogReconRange = FOG_RECON_RANGE
      ResourceTickRate = CREDIT_TICK_RATE
      IncomeMultiplier = 1.0
      VictoryConditions = [ VictoryCondition.Annihilation ]
      AllowedFactions = [ Faction.USA; Faction.China ]
      MapName = "TestMap"
      RandomSeed = 42 }

[<Tests>]
let tests =
    testList "Simulation Core" [
        test "createGameState produces one player per allowed faction" {
            let s = baseSettings ()
            let g = createGameState s s.MapName s.RandomSeed
            Expect.equal g.Players.Count 2 "Two factions => two players"
        }

        test "each tick advances the tick counter by exactly one" {
            let s = baseSettings ()
            let g = createGameState s s.MapName s.RandomSeed
            let after = tick [] g
            Expect.equal after.NewState.Tick (g.Tick + 1L) "Tick increments by 1"
        }

        test "simulation is deterministic for the same seed" {
            let s = baseSettings ()
            let g = createGameState s s.MapName s.RandomSeed
            let a = runTicks 200 g
            let b = runTicks 200 g
            Expect.equal a.Tick b.Tick "Same tick count"
            let creditsOf (st: GameState) =
                st.Players |> Seq.sumBy (fun p -> int64 p.Value.Resources.Credits)
            Expect.equal (creditsOf a) (creditsOf b) "Same resource totals => deterministic"
        }
    ]
