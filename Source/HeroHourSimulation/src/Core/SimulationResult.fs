module HeroHour.Simulation.Core.SimulationResult

open System
open HeroHour.Simulation.Core.Types

/// Result of simulation tick
type SimulationResult = {
    NewState: GameState
    Events: GameEvent list
    VisualEffects: VisualEffect list
    AudioEvents: AudioEvent list
    NetworkUpdates: NetworkUpdate list
    PerformanceMetrics: PerformanceMetrics
}

and GameEvent =
    | UnitCreated of UnitCreatedData
    | UnitDied of UnitDiedData
    | UnitDamaged of UnitDamagedData
    | UnitHealed of UnitHealedData
    | UnitMoved of UnitMovedData
    | UnitAttacked of UnitAttackedData
    | UnitAbilityUsed of UnitAbilityUsedData
    | BuildingCreated of BuildingCreatedData
    | BuildingDied of BuildingDiedData
    | BuildingCompleted of BuildingCompletedData
    | BuildingProductionStarted of BuildingProductionData
    | BuildingProductionCompleted of BuildingProductionData
    | ResourceChanged of ResourceChangedData
    | TechResearched of TechResearchedData
    | UpgradeCompleted of UpgradeCompletedData
    | PlayerDefeated of PlayerDefeatedData
    | PlayerVictorious of PlayerVictoriousData
    | ObjectiveCompleted of ObjectiveCompletedData
    | FogRevealed of FogRevealedData
    | ChatMessage of ChatMessageData
    | GamePaused of GamePausedData
    | GameResumed of GameResumedData

// Event data types
and UnitCreatedData = { UnitID: UnitID; TypeID: UnitTypeID; Owner: PlayerID; Position: Position; Time: float }
and UnitDiedData = { UnitID: UnitID; KillerID: UnitID option; Position: Position; Time: float }
and UnitDamagedData = { UnitID: UnitID; Damage: float; DamageType: DamageType; SourceID: UnitID option; Time: float }
and UnitHealedData = { UnitID: UnitID; Amount: float; SourceID: UnitID option; Time: float }
and UnitMovedData = { UnitID: UnitID; From: Position; To: Position; Time: float }
and UnitAttackedData = { AttackerID: UnitID; TargetID: UnitID; Damage: float; Time: float }
and UnitAbilityUsedData = { UnitID: UnitID; AbilityID: AbilityID; Target: Target option; Time: float }

and BuildingCreatedData = { BuildingID: BuildingID; TypeID: BuildingTypeID; Owner: PlayerID; Position: Position; Time: float }
and BuildingDiedData = { BuildingID: BuildingID; KillerID: UnitID option; Position: Position; Time: float }
and BuildingCompletedData = { BuildingID: BuildingID; Time: float }
and BuildingProductionData = { BuildingID: BuildingID; UnitTypeID: UnitTypeID; Count: int; Time: float }

and ResourceChangedData = { PlayerID: PlayerID; ResourceType: ResourceType; OldAmount: int; NewAmount: int; Time: float }
and TechResearchedData = { PlayerID: PlayerID; TechID: TechID; Time: float }
and UpgradeCompletedData = { PlayerID: PlayerID; BuildingID: BuildingID; UpgradeID: TechID; Time: float }

and PlayerDefeatedData = { PlayerID: PlayerID; VictorID: PlayerID option; Time: float }
and PlayerVictoriousData = { PlayerID: PlayerID; Time: float }

and ObjectiveCompletedData = { ObjectiveID: string; PlayerID: PlayerID; Time: float }
and FogRevealedData = { PlayerID: PlayerID; Region: Bounds; Time: float }
and ChatMessageData = { PlayerID: PlayerID; Message: string; Time: float }
and GamePausedData = { PlayerID: PlayerID; Time: float }
and GameResumedData = { PlayerID: PlayerID; Time: float }

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