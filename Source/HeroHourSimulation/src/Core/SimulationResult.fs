module HeroHour.Simulation.Core.SimulationResult

open System
open HeroHour.Simulation.Core.Types

// NOTE: GameEvent and its data records now live in Types.fs, because
// GameState.EventLog references them and Types.fs is compiled first.

/// Result of simulation tick
type SimulationResult = {
    NewState: GameState
    Events: GameEvent list
    VisualEffects: VisualEffect list
    AudioEvents: AudioEvent list
    NetworkUpdates: NetworkUpdate list
    PerformanceMetrics: PerformanceMetrics
}

/// Visual effects for UE5
and VisualEffect =
    | SpawnEffect of SpawnEffectData
    | DeathEffect of DeathEffectData
    | AttackEffect of AttackEffectData
    | AbilityEffect of AbilityEffectData
    | BuildingEffect of BuildingEffectData
    | AmbientEffect of AmbientEffectData

and SpawnEffectData = { EffectName: string; Position: Position; Scale: float32; Owner: PlayerID }
and DeathEffectData = { EffectName: string; Position: Position; Scale: float32; UnitType: UnitCategory }
and AttackEffectData = { EffectName: string; From: Position; To: Position; DamageType: DamageType }
and AbilityEffectData = { EffectName: string; Position: Position; Radius: float32; AbilityID: AbilityID }
and BuildingEffectData = { EffectName: string; BuildingID: BuildingID; Position: Position }
and AmbientEffectData = { EffectName: string; Position: Position; Duration: float32 }

/// Audio events for UE5
and AudioEvent =
    | PlaySound of PlaySoundData
    | StopSound of StopSoundData
    | SetParameter of SetParameterData

and PlaySoundData = { SoundName: string; Position: Position; Volume: float32; Pitch: float32; Is3D: bool }
and StopSoundData = { SoundName: string; FadeOutTime: float32 }
and SetParameterData = { ParameterName: string; Value: float32; Position: Position option }

/// Network updates for replication
and NetworkUpdate =
    | UnitStateUpdate of UnitID * UnitStateDelta
    | BuildingStateUpdate of BuildingID * BuildingStateDelta
    | PlayerStateUpdate of PlayerID * PlayerStateDelta
    | FogOfWarUpdate of PlayerID * Bounds list
    | ObjectiveUpdate of string * Objective
    | GameStateUpdate of GamePhase * float

and UnitStateDelta = {
    Position: Position option
    Rotation: Rotation option
    Health: float option
    Shield: float option
    IsAlive: bool option
    CurrentOrder: Order option
    AbilityCooldowns: Map<AbilityID, float> option
}

and BuildingStateDelta = {
    Health: float option
    IsConstructing: bool option
    ConstructionProgress: float option
    IsProducing: bool option
    ProductionProgress: float option
    RallyPoint: Position option
}

and PlayerStateDelta = {
    Resources: ResourceAmount option
    Population: int option
    TechTree: TechID list option
}

/// Performance metrics
and PerformanceMetrics = {
    TickDurationMs: float
    SimulationTimeMs: float
    EventCount: int
    UnitCount: int
    BuildingCount: int
    MemoryUsageMB: int64
    GCCollections: int
}