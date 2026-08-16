module HeroHour.Simulation.Core.GameState

open System
open System.Collections.Immutable
open HeroHour.Simulation.Core.Types
open HeroHour.Simulation.Core.Constants

/// Create a new game state with default settings
let createGameState (settings: GameSettings) (mapName: string) (seed: int) : GameState =
    let random = System.Random(seed)
    let localPlayerID = PlayerID.New()

    let initialPlayers =
        settings.AllowedFactions
        |> List.mapi (fun i faction ->
            let playerID = PlayerID.New()
            let color = getPlayerColor i
            let team = if i < settings.AllowedFactions.Length / 2 then 0 else 1

            let playerState = {
                ID = playerID
                Name = sprintf "Player %d" (i + 1)
                Faction = faction
                Color = color
                Team = team
                Resources = settings.StartingResources
                Income = { Credits = 0; Energy = 0; TechPoints = 0 }
                Population = 0
                PopulationCap = settings.PopulationCap
                Units = ImmutableDictionary<UnitID, UnitState>.Empty
                Buildings = ImmutableDictionary<BuildingID, BuildingState>.Empty
                TechTree = []
                ResearchedTechs = []
                IsDefeated = false
                IsVictorious = false
                Score = 0L
                APM = 0.0
                CustomData = Map.empty
            }
            playerID, playerState
        )
        |> ImmutableDictionary<PlayerID, PlayerState>.ToImmutableDictionary

    let fogOfWar = if settings.FogOfWarEnabled then
        createFogOfWar initialPlayers.Keys
    else
        { VisibleRegions = ImmutableDictionary<PlayerID, Bounds list>.Empty
          ExploredRegions = ImmutableDictionary<PlayerID, Bounds list>.Empty
          RevealedUnits = ImmutableDictionary<PlayerID, UnitID list>.Empty }

    {
        GameTime = 0.0
        RealTime = 0.0
        DeltaTime = TICK_INTERVAL
        Tick = 0L
        Phase = GamePhase.PreGame
        Settings = settings
        Players = initialPlayers
        LocalPlayerID = Some localPlayerID
        MapName = mapName
        RandomSeed = seed
        FogOfWar = fogOfWar
        Objectives = []
        EventLog = []
        Winner = None
        CustomData = Map.empty
    }

and getPlayerColor (index: int) : PlayerColor =
    match index % 16 with
    | 0 -> PlayerColor.Red
    | 1 -> PlayerColor.Blue
    | 2 -> PlayerColor.Green
    | 3 -> PlayerColor.Yellow
    | 4 -> PlayerColor.Orange
    | 5 -> PlayerColor.Purple
    | 6 -> PlayerColor.Cyan
    | 7 -> PlayerColor.Pink
    | 8 -> PlayerColor.Brown
    | 9 -> PlayerColor.Lime
    | 10 -> PlayerColor.Magenta
    | 11 -> PlayerColor.Navy
    | 12 -> PlayerColor.Olive
    | 13 -> PlayerColor.Maroon
    | 14 -> PlayerColor.Teal
    | _ -> PlayerColor.Silver

and createFogOfWar (playerIDs: System.Collections.Generic.IEnumerable<PlayerID>) : FogOfWarState =
    let emptyBounds = ImmutableDictionary<PlayerID, Bounds list>.Empty
    let emptyUnits = ImmutableDictionary<PlayerID, UnitID list>.Empty
    { VisibleRegions = emptyBounds
      ExploredRegions = emptyBounds
      RevealedUnits = emptyUnits }

/// Update game state with delta time
let updateGameTime (state: GameState) (deltaTime: float) : GameState =
    let clampedDelta = min deltaTime MAX_DELTA_TIME
    { state with
        GameTime = state.GameTime + clampedDelta
        RealTime = state.RealTime + clampedDelta
        DeltaTime = clampedDelta
        Tick = state.Tick + 1L }

/// Check victory conditions
let checkVictory (state: GameState) : PlayerID option =
    let alivePlayers =
        state.Players
        |> Seq.filter (fun kvp -> not kvp.Value.IsDefeated)
        |> Seq.toList

    match alivePlayers.Length with
    | 0 -> None // Draw
    | 1 -> Some winner ->
        if state.Settings.VictoryConditions |> List.contains VictoryCondition.Annihilation then
            Some winner
        else
            // Check other conditions
            checkOtherVictoryConditions state winner
    | _ -> None

and checkOtherVictoryConditions (state: GameState) (winner: PlayerID) : PlayerID option =
    // Check time limit
    if state.Settings.MatchDuration > 0.0 && state.GameTime >= state.Settings.MatchDuration then
        // Find player with highest score
        state.Players
        |> Seq.maxBy (fun kvp -> kvp.Value.Score)
        |> (fun kvp -> Some kvp.Key)
    else
        // Check objectives
        let completedObjectives =
            state.Objectives
            |> List.filter (fun obj -> obj.IsComplete)

        if completedObjectives.Length = state.Objectives.Length && state.Objectives.Length > 0 then
            Some winner
        else
            None

/// Apply damage to unit
let applyDamageToUnit (state: GameState) (unitID: UnitID) (damage: float) : GameState =
    let playerKVP =
        state.Players
        |> Seq.tryFind (fun kvp -> kvp.Value.Units.ContainsKey(unitID))

    match playerKVP with
    | Some (playerID, playerState) ->
        let unit = playerState.Units.[unitID]
        let newHealth = max 0.0 (unit.Health - damage)
        let isAlive = newHealth > 0.0

        let updatedUnit = { unit with Health = newHealth; IsAlive = isAlive }
        let updatedUnits = playerState.Units.SetItem(unitID, updatedUnit)
        let updatedPlayer = { playerState with Units = updatedUnits }
        let updatedPlayers = state.Players.SetItem(playerID, updatedPlayer)

        let deathEvent = if not isAlive then
            Some (GameEvent.UnitDied { UnitID = unitID; KillerID = None; Position = unit.Position; Time = state.GameTime })
        else
            None

        { state with Players = updatedPlayers; EventLog = state.EventLog @ deathEvent }
    | None -> state

/// Add resource to player
let addResources (state: GameState) (playerID: PlayerID) (amount: ResourceAmount) : GameState =
    match state.Players.TryGetValue(playerID) with
    | true, playerState ->
        let newResources = playerState.Resources + amount
        let updatedPlayer = { playerState with Resources = newResources }
        { state with Players = state.Players.SetItem(playerID, updatedPlayer) }
    | _ -> state

/// Spend resources
let spendResources (state: GameState) (playerID: PlayerID) (cost: ResourceCost) : Result<GameState, string> =
    match state.Players.TryGetValue(playerID) with
    | true, playerState ->
        let resources = playerState.Resources
        if resources.Credits >= cost.Credits &&
           resources.Energy >= cost.Energy &&
           resources.TechPoints >= cost.TechPoints then
            let newResources = {
                Credits = resources.Credits - cost.Credits
                Energy = resources.Energy - cost.Energy
                TechPoints = resources.TechPoints - cost.TechPoints
            }
            let updatedPlayer = { playerState with Resources = newResources }
            Ok { state with Players = state.Players.SetItem(playerID, updatedPlayer) }
        else
            Error "Insufficient resources"
    | _ -> Error "Player not found"

/// Population management
let canAffordPopulation (state: GameState) (playerID: PlayerID) (popCost: int) : bool =
    match state.Players.TryGetValue(playerID) with
    | true, playerState -> playerState.Population + popCost <= playerState.PopulationCap
    | _ -> false

let addPopulation (state: GameState) (playerID: PlayerID) (amount: int) : GameState =
    match state.Players.TryGetValue(playerID) with
    | true, playerState ->
        let updatedPlayer = { playerState with Population = playerState.Population + amount }
        { state with Players = state.Players.SetItem(playerID, updatedPlayer) }
    | _ -> state