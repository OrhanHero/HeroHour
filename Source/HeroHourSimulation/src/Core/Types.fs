module HeroHour.Simulation.Core.Types

open System
open System.Collections.Immutable

// ============================================================================
// Identifiers
// ============================================================================

[<Struct>]
type PlayerID = PlayerID of Guid with
    static member New() = PlayerID(Guid.NewGuid())
    static member Empty = PlayerID(Guid.Empty)
    override this.ToString() = match this with PlayerID g -> g.ToString("N")

[<Struct>]
type UnitID = UnitID of Guid with
    static member New() = UnitID(Guid.NewGuid())
    static member Empty = UnitID(Guid.Empty)
    override this.ToString() = match this with UnitID g -> g.ToString("N")

[<Struct>]
type BuildingID = BuildingID of Guid with
    static member New() = BuildingID(Guid.NewGuid())
    static member Empty = BuildingID(Guid.Empty)
    override this.ToString() = match this with BuildingID g -> g.ToString("N")

[<Struct>]
type TechID = TechID of string with
    override this.ToString() = match this with TechID s -> s

[<Struct>]
type AbilityID = AbilityID of string with
    override this.ToString() = match this with AbilityID s -> s

[<Struct>]
type UnitTypeID = UnitTypeID of string with
    override this.ToString() = match this with UnitTypeID s -> s

[<Struct>]
type BuildingTypeID = BuildingTypeID of string with
    override this.ToString() = match this with BuildingTypeID s -> s

// ============================================================================
// Core Types
// ============================================================================

type Faction =
    | USA
    | China
    | GLA
    | Neutral

type PlayerColor =
    | Red | Blue | Green | Yellow | Orange | Purple | Cyan | Pink
    | Brown | Lime | Magenta | Navy | Olive | Maroon | Teal | Silver

type GamePhase =
    | PreGame
    | Starting
    | Playing
    | Paused
    | Ended
    | PostGame

type VictoryCondition =
    | Annihilation
    | Domination
    | Economic
    | TimeLimit
    | Objective

type UnitCategory =
    | Infantry
    | Vehicle
    | Aircraft
    | Naval
    | Structure
    | Hero

type UnitRole =
    | Scout
    | Assault
    | Support
    | Sniper
    | Commander
    | Siege
    | AntiAir
    | Transport

type DamageType =
    | Kinetic
    | Energy
    | Explosive
    | Chemical
    | Fire
    | EMP

type ArmorType =
    | Light
    | Medium
    | Heavy
    | Structure
    | Shielded

type MovementType =
    | Foot
    | Tracked
    | Wheeled
    | Hover
    | Air
    | Naval
    | Teleport

// ============================================================================
// Position & Geometry
// ============================================================================

[<Struct>]
type Position = {
    X: float
    Y: float
    Z: float
} with
    static member Zero = { X = 0.0; Y = 0.0; Z = 0.0 }
    static member Distance (a: Position) (b: Position) =
        let dx = a.X - b.X
        let dy = a.Y - b.Y
        let dz = a.Z - b.Z
        sqrt (dx*dx + dy*dy + dz*dz)
    static member Distance2D (a: Position) (b: Position) =
        let dx = a.X - b.X
        let dy = a.Y - b.Y
        sqrt (dx*dx + dy*dy)

[<Struct>]
type Rotation = {
    Pitch: float
    Yaw: float
    Roll: float
}

[<Struct>]
type Bounds = {
    Min: Position
    Max: Position
} with
    member this.Contains (pos: Position) =
        pos.X >= this.Min.X && pos.X <= this.Max.X &&
        pos.Y >= this.Min.Y && pos.Y <= this.Max.Y &&
        pos.Z >= this.Min.Z && pos.Z <= this.Max.Z

// ============================================================================
// Resources
// ============================================================================

type ResourceType =
    | Credits
    | Energy
    | TechPoints

type ResourceAmount = {
    Credits: int
    Energy: int
    TechPoints: int
} with
    static member Zero = { Credits = 0; Energy = 0; TechPoints = 0 }
    static member (+) (a: ResourceAmount, b: ResourceAmount) =
        { Credits = a.Credits + b.Credits
          Energy = a.Energy + b.Energy
          TechPoints = a.TechPoints + b.TechPoints }
    static member (-) (a: ResourceAmount, b: ResourceAmount) =
        { Credits = a.Credits - b.Credits
          Energy = a.Energy - b.Energy
          TechPoints = a.TechPoints - b.TechPoints }
    member this.Get (t: ResourceType) =
        match t with
        | Credits -> this.Credits
        | Energy -> this.Energy
        | TechPoints -> this.TechPoints

type ResourceCost = {
    Credits: int
    Energy: int
    TechPoints: int
    BuildTime: float
}

// ============================================================================
// Unit & Building State
// ============================================================================

type UnitState = {
    ID: UnitID
    TypeID: UnitTypeID
    Owner: PlayerID
    Position: Position
    Rotation: Rotation
    Health: float
    MaxHealth: float
    Shield: float
    MaxShield: float
    IsAlive: bool
    IsSelected: bool
    CurrentOrder: Order option
    OrderQueue: Order list
    Abilities: AbilityID list
    AbilityCooldowns: Map<AbilityID, float>
    Experience: float
    Level: int
    Veterancy: VeterancyLevel
    LastAttackTime: float
    LastMoveTime: float
    CustomData: Map<string, obj>
}

and VeterancyLevel =
    | NoVeterancy
    | Veteran
    | Elite
    | Heroic

and BuildingState = {
    ID: BuildingID
    TypeID: BuildingTypeID
    Owner: PlayerID
    Position: Position
    Rotation: Rotation
    Health: float
    MaxHealth: float
    IsAlive: bool
    IsConstructing: bool
    ConstructionProgress: float
    ConstructionTime: float
    IsProducing: bool
    ProductionQueue: ProductionItem list
    CurrentProduction: ProductionItem option
    ProductionProgress: float
    RallyPoint: Position option
    Abilities: AbilityID list
    Upgrades: TechID list
    CustomData: Map<string, obj>
}

and ProductionItem = {
    UnitTypeID: UnitTypeID
    Count: int
    StartTime: float
}

// ============================================================================
// Orders & Commands
// ============================================================================

and Order =
    | Move of Position
    | Attack of Target
    | Patrol of Position list
    | Guard of UnitID
    | Build of BuildingTypeID * Position
    | Repair of BuildingID
    | Capture of BuildingID
    | Load of BuildingID
    | Unload of Position
    | UseAbility of AbilityID * Target option
    | Stop
    | HoldPosition

and Target =
    | TargetUnit of UnitID
    | TargetBuilding of BuildingID
    | TargetPosition of Position
    | NoTarget

// ============================================================================
// Player State
// ============================================================================

type PlayerState = {
    ID: PlayerID
    Name: string
    Faction: Faction
    Color: PlayerColor
    Team: int
    Resources: ResourceAmount
    Income: ResourceAmount
    Population: int
    PopulationCap: int
    Units: ImmutableDictionary<UnitID, UnitState>
    Buildings: ImmutableDictionary<BuildingID, BuildingState>
    TechTree: TechID list
    ResearchedTechs: TechID list
    IsDefeated: bool
    IsVictorious: bool
    Score: int64
    APM: float
    CustomData: Map<string, obj>
}

// ============================================================================
// Game State
// ============================================================================

type GameSettings = {
    MatchDuration: float // Seconds, 0 = unlimited
    StartingResources: ResourceAmount
    PopulationCap: int
    FogOfWarEnabled: bool
    FogRevealRange: float
    FogReconRange: float
    ResourceTickRate: float
    IncomeMultiplier: float
    VictoryConditions: VictoryCondition list
    AllowedFactions: Faction list
    MapName: string
    RandomSeed: int
}

type GameState = {
    GameTime: float
    RealTime: float
    DeltaTime: float
    Tick: int64
    Phase: GamePhase
    Settings: GameSettings
    Players: ImmutableDictionary<PlayerID, PlayerState>
    LocalPlayerID: PlayerID option
    MapName: string
    RandomSeed: int
    FogOfWar: FogOfWarState
    Objectives: Objective list
    EventLog: GameEvent list
    Winner: PlayerID option
    CustomData: Map<string, obj>
}

and FogOfWarState = {
    VisibleRegions: ImmutableDictionary<PlayerID, Bounds list>
    ExploredRegions: ImmutableDictionary<PlayerID, Bounds list>
    RevealedUnits: ImmutableDictionary<PlayerID, UnitID list>
}

and Objective = {
    ID: string
    Description: string
    Type: ObjectiveType
    TargetPlayer: PlayerID option
    Progress: float
    IsComplete: bool
    Reward: ResourceAmount option
}

and ObjectiveType =
    | DestroyUnits of int
    | CaptureBuilding of BuildingTypeID
    | AccumulateResources of ResourceType * int
    | SurviveTime of float
    | EliminatePlayer of PlayerID

// ============================================================================
// Game Events
// ============================================================================
// Defined here (rather than in SimulationResult) because GameState.EventLog
// references GameEvent, and Types.fs is compiled first.

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