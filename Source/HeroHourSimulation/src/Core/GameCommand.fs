module HeroHour.Simulation.Core.GameCommand

open System
open HeroHour.Simulation.Core.Types

/// Commands sent from UE5 to F# simulation
type GameCommand =
    | MoveUnits of UnitID list * Position
    | AttackTarget of UnitID list * Target
    | BuildStructure of PlayerID * BuildingTypeID * Position
    | TrainUnit of PlayerID * BuildingID * UnitTypeID
    | CancelProduction of PlayerID * BuildingID * int // queue index
    | ResearchTech of PlayerID * TechID
    | UseAbility of UnitID * AbilityID * Target option
    | SetRallyPoint of BuildingID * Position
    | SelectUnits of PlayerID * UnitID list
    | SetControlGroup of PlayerID * int * UnitID list
    | RecallControlGroup of PlayerID * int
    | ChatMessage of PlayerID * string
    | Surrender of PlayerID
    | PauseGame of PlayerID
    | ResumeGame of PlayerID
    | RequestSave of PlayerID * string
    | RequestLoad of PlayerID * string

/// Result of command processing
type CommandResult =
    | Success of obj option
    | Failure of string
    | InvalidState of string
    | InsufficientResources of ResourceCost * ResourceAmount
    | UnitNotFound of UnitID
    | BuildingNotFound of BuildingID
    | PlayerNotFound of PlayerID
    | TechNotAvailable of TechID
    | AbilityOnCooldown of AbilityID * float

/// Batch command for network efficiency
type CommandBatch = {
    PlayerID: PlayerID
    Commands: GameCommand list
    Tick: int64
    Sequence: int
    Timestamp: DateTime
}

/// Validation
let validateCommand (cmd: GameCommand) : Result<unit, string> =
    match cmd with
    | MoveUnits (units, pos) ->
        if units.IsEmpty then Error "No units specified" else Ok ()
    | AttackTarget (units, target) ->
        if units.IsEmpty then Error "No units specified" else Ok ()
    | BuildStructure (_, _, pos) -> Ok ()
    | TrainUnit (_, _, unitType) -> Ok ()
    | ResearchTech (_, tech) -> Ok ()
    | UseAbility (_, ability, _) -> Ok ()
    | _ -> Ok ()

/// Serialize for network
let serializeCommand (cmd: GameCommand) : byte[] =
    // Use MessagePack or similar
    System.Text.Json.JsonSerializer.SerializeToUtf8Bytes(cmd)

/// Deserialize from network
let deserializeCommand (data: byte[]) : Result<GameCommand, string> =
    try
        Ok (System.Text.Json.JsonSerializer.Deserialize<GameCommand>(data))
    with ex -> Error ex.Message