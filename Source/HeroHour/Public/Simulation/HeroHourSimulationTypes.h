// HeroHourSimulationTypes.h
// UE5 compatible types matching F# simulation

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HeroHourSimulationTypes.generated.h"

// Forward declarations
class UHeroHourUnitDataAsset;
class UHeroHourBuildingDataAsset;
class UHeroHourTechDataAsset;
class UHeroHourAbilityDataAsset;

UENUM(BlueprintType)
enum class EHeroHourResourceType : uint8
{
    Credits,
    Energy,
    TechPoints,
    Max
};

UENUM(BlueprintType)
enum class EHeroHourUnitCategory : uint8
{
    Infantry,
    Vehicle,
    Aircraft,
    Naval,
    Structure,
    Hero,
    Special
};

UENUM(BlueprintType)
enum class EHeroHourDamageType : uint8
{
    Kinetic,
    Explosive,
    Energy,
    Chemical,
    Fire,
    Sniper,
    Crushing,
    Healing
};

UENUM(BlueprintType)
enum class EHeroHourMovementType : uint8
{
    Foot,
    Tracked,
    Wheeled,
    Hover,
    Air,
    Naval,
    Teleport
};

UENUM(BlueprintType)
enum class EHeroHourGamePhase : uint8
{
    PreGame,
    Starting,
    Playing,
    Paused,
    Victory,
    Defeat,
    Draw,
    PostGame
};

UENUM(BlueprintType)
enum class EHeroHourVictoryCondition : uint8
{
    Annihilation,
    Domination,
    Economic,
    Technological,
    Objective,
    TimeLimit
};

UENUM(BlueprintType)
enum class EHeroHourFaction : uint8
{
    USA,
    China,
    GLA,
    Neutral,
    Custom
};

// Resource amount
USTRUCT(BlueprintType)
struct FHeroHourResourceAmount
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Credits = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Energy = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 TechPoints = 0;

    FHeroHourResourceAmount() = default;
    FHeroHourResourceAmount(int32 InCredits, int32 InEnergy, int32 InTechPoints)
        : Credits(InCredits), Energy(InEnergy), TechPoints(InTechPoints) {}

    FHeroHourResourceAmount operator+(const FHeroHourResourceAmount& Other) const
    {
        return FHeroHourResourceAmount(Credits + Other.Credits, Energy + Other.Energy, TechPoints + Other.TechPoints);
    }

    FHeroHourResourceAmount operator-(const FHeroHourResourceAmount& Other) const
    {
        return FHeroHourResourceAmount(Credits - Other.Credits, Energy - Other.Energy, TechPoints - Other.TechPoints);
    }

    bool operator>=(const FHeroHourResourceAmount& Other) const
    {
        return Credits >= Other.Credits && Energy >= Other.Energy && TechPoints >= Other.TechPoints;
    }

    static FHeroHourResourceAmount Zero() { return FHeroHourResourceAmount(0, 0, 0); }
};

// Resource cost
USTRUCT(BlueprintType)
struct FHeroHourResourceCost
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FHeroHourResourceAmount Cost;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float BuildTime = 0.0f;

    FHeroHourResourceCost() = default;
    FHeroHourResourceCost(const FHeroHourResourceAmount& InCost, float InBuildTime)
        : Cost(InCost), BuildTime(InBuildTime) {}
};

// Position (matches F# Position type)
USTRUCT(BlueprintType)
struct FHeroHourPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float X = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Y = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Z = 0.0f;

    FHeroHourPosition() = default;
    FHeroHourPosition(float InX, float InY, float InZ = 0.0f)
        : X(InX), Y(InY), Z(InZ) {}

    FVector ToFVector() const { return FVector(X, Y, Z); }
    static FHeroHourPosition FromFVector(const FVector& Vec) { return FHeroHourPosition(Vec.X, Vec.Y, Vec.Z); }
};

// Rotation
USTRUCT(BlueprintType)
struct FHeroHourRotation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Yaw = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Pitch = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Roll = 0.0f;

    FHeroHourRotation() = default;
    FHeroHourRotation(float InYaw, float InPitch = 0.0f, float InRoll = 0.0f)
        : Yaw(InYaw), Pitch(InPitch), Roll(InRoll) {}

    FRotator ToFRotator() const { return FRotator(Pitch, Yaw, Roll); }
    static FHeroHourRotation FromFRotator(const FRotator& Rot) { return FHeroHourRotation(Rot.Yaw, Rot.Pitch, Rot.Roll); }
};

// Bounds for fog of war
USTRUCT(BlueprintType)
struct FHeroHourBounds
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPosition Min;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPosition Max;

    FHeroHourBounds() = default;
    FHeroHourBounds(const FHeroHourPosition& InMin, const FHeroHourPosition& InMax)
        : Min(InMin), Max(InMax) {}
};

// Target for commands
USTRUCT(BlueprintType)
struct FHeroHourTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourTargetType Type;

    UPROPERTY(BlueprintReadWrite)
    int32 UnitID = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 BuildingID = -1;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPosition Position;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourBounds Area;

    FHeroHourTarget() = default;

    static FHeroHourTarget Unit(int32 InUnitID)
    {
        FHeroHourTarget T;
        T.Type = EHeroHourTargetType::Unit;
        T.UnitID = InUnitID;
        return T;
    }

    static FHeroHourTarget Building(int32 InBuildingID)
    {
        FHeroHourTarget T;
        T.Type = EHeroHourTargetType::Building;
        T.BuildingID = InBuildingID;
        return T;
    }

    static FHeroHourTarget Position(const FHeroHourPosition& InPosition)
    {
        FHeroHourTarget T;
        T.Type = EHeroHourTargetType::Position;
        T.Position = InPosition;
        return T;
    }

    static FHeroHourTarget Area(const FHeroHourBounds& InArea)
    {
        FHeroHourTarget T;
        T.Type = EHeroHourTargetType::Area;
        T.Area = InArea;
        return T;
    }
};

UENUM(BlueprintType)
enum class EHeroHourTargetType : uint8
{
    None,
    Unit,
    Building,
    Position,
    Area
};

// Unit state
USTRUCT(BlueprintType)
struct FHeroHourUnitState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 UnitID = -1;

    UPROPERTY(BlueprintReadWrite)
    FString TypeID;

    UPROPERTY(BlueprintReadWrite)
    int32 OwnerID = -1;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPosition Position;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourRotation Rotation;

    UPROPERTY(BlueprintReadWrite)
    float Health = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float MaxHealth = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Shield = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadWrite)
    EHeroHourUnitStatus Status;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ActiveAbilities;

    UPROPERTY(BlueprintReadWrite)
    TMap<FString, float> AbilityCooldowns;

    UPROPERTY(BlueprintReadWrite)
    FString CurrentOrder;
};

// Building state
USTRUCT(BlueprintType)
struct FHeroHourBuildingState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 BuildingID = -1;

    UPROPERTY(BlueprintReadWrite)
    FString TypeID;

    UPROPERTY(BlueprintReadWrite)
    int32 OwnerID = -1;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPosition Position;

    UPROPERTY(BlueprintReadWrite)
    float Health = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float MaxHealth = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsConstructing = false;

    UPROPERTY(BlueprintReadWrite)
    float ConstructionProgress = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsProducing = false;

    UPROPERTY(BlueprintReadWrite)
    FString ProductionTypeID;

    UPROPERTY(BlueprintReadWrite)
    float ProductionProgress = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPosition RallyPoint;

    UPROPERTY(BlueprintReadWrite)
    TArray<int32> ProducedUnitIDs;
};

// Player state
USTRUCT(BlueprintType)
struct FHeroHourPlayerState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 PlayerID = -1;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    EHeroHourFaction Faction = EHeroHourFaction::USA;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourResourceAmount Resources;

    UPROPERTY(BlueprintReadWrite)
    int32 Population = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 PopulationCap = 200;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ResearchedTechs;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> AvailableUnits;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> AvailableBuildings;

    UPROPERTY(BlueprintReadWrite)
    bool bIsDefeated = false;

    UPROPERTY(BlueprintReadWrite)
    int32 Score = 0;

    UPROPERTY(BlueprintReadWrite)
    EHeroHourPlayerTeam Team = EHeroHourPlayerTeam::Team1;
};

UENUM(BlueprintType)
enum class EHeroHourPlayerTeam : uint8
{
    Team1,
    Team2,
    Team3,
    Team4,
    Team5,
    Team6,
    Team7,
    Team8,
    FFA,
    Spectator
};

UENUM(BlueprintType)
enum class EHeroHourUnitStatus : uint8
{
    Idle,
    Moving,
    Attacking,
    Building,
    Repairing,
    Harvesting,
    Garrisoned,
    Stunned,
    Dead
};

// Game settings
USTRUCT(BlueprintType)
struct FHeroHourGameSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float MatchDuration = 3600.0f; // seconds

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FHeroHourResourceAmount StartingResources;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 PopulationCap = 200;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bFogOfWarEnabled = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float FogRevealRange = 1000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float FogReconRange = 2000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float ResourceTickRate = 0.5f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float IncomeMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<EHeroHourVictoryCondition> VictoryConditions;

    FHeroHourGameSettings()
    {
        StartingResources = FHeroHourResourceAmount(5000, 100, 0);
        VictoryConditions = { EHeroHourVictoryCondition::Annihilation };
    }
};

// Objective
USTRUCT(BlueprintType)
struct FHeroHourObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ID;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    EHeroHourObjectiveType Type;

    UPROPERTY(BlueprintReadWrite)
    int32 RequiredCount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bIsComplete = false;

    UPROPERTY(BlueprintReadWrite)
    TArray<int32> OwnerPlayerIDs;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourBounds Area;
};

UENUM(BlueprintType)
enum class EHeroHourObjectiveType : uint8
{
    CapturePoint,
    DestroyTarget,
    ProtectTarget,
    CollectResources,
    ResearchTech,
    BuildStructure,
    SurviveTime,
    EliminatePlayer
};