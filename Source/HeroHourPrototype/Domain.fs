namespace HeroHour.Prototype

type Faction =
    | Hero
    | USA

type UnitRole =
    | Infantry
    | Vehicle
    | Air

type UnitDefinition = {
    Id: string
    DisplayName: string
    Faction: Faction
    Role: UnitRole
    Cost: int
    BuildTimeSeconds: int
    HitPoints: int
    Damage: int
    MoveSpeed: float
}

type PrototypeMap = {
    Id: string
    DisplayName: string
    RecommendedPlayers: int
    StartingCredits: int
}

type PrototypeScope = {
    Map: PrototypeMap
    Factions: Faction list
    Units: UnitDefinition list
}
