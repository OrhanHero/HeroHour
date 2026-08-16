// HeroHourFSharpSimulationInterface.h
// Bridge between UE5 C++ and F# Simulation Layer

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeroHourFSharpSimulationInterface.generated.h"

class FHeroHourSimulationState;
class FHeroHourSimulationResult;
struct FHeroHourGameCommand;

UINTERFACE(MinimalAPI, BlueprintType, Category="HeroHour|Simulation")
class UHeroHourFSharpSimulationInterface : public UInterface
{
    GENERATED_BODY()
};

class IHeroHourFSharpSimulationInterface
{
    GENERATED_BODY()

public:
    // Initialize simulation with settings
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    bool InitializeSimulation(const FString& SettingsJson);

    // Run simulation tick
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    FHeroHourSimulationResult TickSimulation(float DeltaTime, const TArray<FHeroHourGameCommand>& Commands);

    // Get current simulation state
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    FString GetSimulationStateJson() const;

    // Apply command immediately (for client prediction)
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    FHeroHourCommandResult ApplyCommand(const FHeroHourGameCommand& Command);

    // Serialize/deserialize for save/load
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    FString SerializeState() const;

    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    bool DeserializeState(const FString& StateJson);

    // Reset simulation
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    void ResetSimulation();

    // Get performance metrics
    UFUNCTION(BlueprintNativeEvent, Category="HeroHour|Simulation")
    FHeroHourPerformanceMetrics GetPerformanceMetrics() const;
};

// Game command struct (mirrors F# type)
USTRUCT(BlueprintType)
struct FHeroHourGameCommand
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourCommandType Type;

    UPROPERTY(BlueprintReadWrite)
    int32 PlayerID;

    UPROPERTY(BlueprintReadWrite)
    TArray<int32> UnitIDs;

    UPROPERTY(BlueprintReadWrite)
    int32 BuildingID;

    UPROPERTY(BlueprintReadWrite)
    FVector Position;

    UPROPERTY(BlueprintReadWrite)
    FVector TargetPosition;

    UPROPERTY(BlueprintReadWrite)
    int32 TargetUnitID;

    UPROPERTY(BlueprintReadWrite)
    int32 TargetBuildingID;

    UPROPERTY(BlueprintReadWrite)
    FString UnitTypeID;

    UPROPERTY(BlueprintReadWrite)
    FString BuildingTypeID;

    UPROPERTY(BlueprintReadWrite)
    FString TechID;

    UPROPERTY(BlueprintReadWrite)
    FString AbilityID;

    UPROPERTY(BlueprintReadWrite)
    int32 ControlGroupIndex;

    UPROPERTY(BlueprintReadWrite)
    FString JsonData; // For extensibility
};

UENUM(BlueprintType)
enum class EHeroHourCommandType : uint8
{
    MoveUnits,
    AttackTarget,
    BuildStructure,
    TrainUnit,
    CancelProduction,
    ResearchTech,
    UseAbility,
    SetRallyPoint,
    SelectUnits,
    SetControlGroup,
    RecallControlGroup,
    ChatMessage,
    Surrender,
    PauseGame,
    ResumeGame,
    RequestSave,
    RequestLoad
};

// Simulation result struct
USTRUCT(BlueprintType)
struct FHeroHourSimulationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    bool bSuccess;

    UPROPERTY(BlueprintReadWrite)
    FString NewStateJson;

    UPROPERTY(BlueprintReadWrite)
    TArray<FHeroHourGameEvent> Events;

    UPROPERTY(BlueprintReadWrite)
    TArray<FHeroHourVisualEffect> VisualEffects;

    UPROPERTY(BlueprintReadWrite)
    TArray<FHeroHourAudioEvent> AudioEvents;

    UPROPERTY(BlueprintReadWrite)
    TArray<FHeroHourNetworkUpdate> NetworkUpdates;

    UPROPERTY(BlueprintReadWrite)
    FHeroHourPerformanceMetrics Metrics;
};

USTRUCT(BlueprintType)
struct FHeroHourCommandResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourCommandResultType ResultType;

    UPROPERTY(BlueprintReadWrite)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite)
    FString ResultDataJson;
};

UENUM(BlueprintType)
enum class EHeroHourCommandResultType : uint8
{
    Success,
    Failure,
    InvalidState,
    InsufficientResources,
    UnitNotFound,
    BuildingNotFound,
    PlayerNotFound,
    TechNotAvailable,
    AbilityOnCooldown
};

// Game events
USTRUCT(BlueprintType)
struct FHeroHourGameEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourGameEventType Type;

    UPROPERTY(BlueprintReadWrite)
    float Time;

    UPROPERTY(BlueprintReadWrite)
    FString DataJson;
};

UENUM(BlueprintType)
enum class EHeroHourGameEventType : uint8
{
    UnitCreated,
    UnitDied,
    UnitDamaged,
    UnitHealed,
    UnitMoved,
    UnitAttacked,
    UnitAbilityUsed,
    BuildingCreated,
    BuildingDied,
    BuildingCompleted,
    BuildingProductionStarted,
    BuildingProductionCompleted,
    ResourceChanged,
    TechResearched,
    UpgradeCompleted,
    PlayerDefeated,
    PlayerVictorious,
    ObjectiveCompleted,
    FogRevealed,
    ChatMessage,
    GamePaused,
    GameResumed
};

// Visual effects
USTRUCT(BlueprintType)
struct FHeroHourVisualEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourVisualEffectType Type;

    UPROPERTY(BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(BlueprintReadWrite)
    FVector Position;

    UPROPERTY(BlueprintReadWrite)
    float Scale;

    UPROPERTY(BlueprintReadWrite)
    float Radius;

    UPROPERTY(BlueprintReadWrite)
    FString DataJson;
};

UENUM(BlueprintType)
enum class EHeroHourVisualEffectType : uint8
{
    SpawnEffect,
    DeathEffect,
    AttackEffect,
    AbilityEffect,
    BuildingEffect,
    AmbientEffect
};

// Audio events
USTRUCT(BlueprintType)
struct FHeroHourAudioEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourAudioEventType Type;

    UPROPERTY(BlueprintReadWrite)
    FString SoundName;

    UPROPERTY(BlueprintReadWrite)
    FVector Position;

    UPROPERTY(BlueprintReadWrite)
    float Volume;

    UPROPERTY(BlueprintReadWrite)
    float Pitch;

    UPROPERTY(BlueprintReadWrite)
    bool bIs3D;

    UPROPERTY(BlueprintReadWrite)
    FString ParameterName;

    UPROPERTY(BlueprintReadWrite)
    float ParameterValue;
};

UENUM(BlueprintType)
enum class EHeroHourAudioEventType : uint8
{
    PlaySound,
    StopSound,
    SetParameter
};

// Network updates
USTRUCT(BlueprintType)
struct FHeroHourNetworkUpdate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EHeroHourNetworkUpdateType Type;

    UPROPERTY(BlueprintReadWrite)
    int32 EntityID;

    UPROPERTY(BlueprintReadWrite)
    FString DeltaJson;
};

UENUM(BlueprintType)
enum class EHeroHourNetworkUpdateType : uint8
{
    UnitStateUpdate,
    BuildingStateUpdate,
    PlayerStateUpdate,
    FogOfWarUpdate,
    ObjectiveUpdate,
    GameStateUpdate
};

// Performance metrics
USTRUCT(BlueprintType)
struct FHeroHourPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float TickDurationMs;

    UPROPERTY(BlueprintReadWrite)
    float SimulationTimeMs;

    UPROPERTY(BlueprintReadWrite)
    int32 EventCount;

    UPROPERTY(BlueprintReadWrite)
    int32 UnitCount;

    UPROPERTY(BlueprintReadWrite)
    int32 BuildingCount;

    UPROPERTY(BlueprintReadWrite)
    int64 MemoryUsageMB;

    UPROPERTY(BlueprintReadWrite)
    int32 GCCollections;
};