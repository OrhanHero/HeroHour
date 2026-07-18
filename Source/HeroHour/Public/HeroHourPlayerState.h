// HeroHourPlayerState.h
// Replicated player state with resources, tech tree, etc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HeroHourSimulationTypes.h"
#include "HeroHourPlayerState.generated.h"

UENUM(BlueprintType, Category = "HeroHour|Player")
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
    Spectator
};

UCLASS(Blueprintable)
class HEROHOUR_API AHeroHourPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AHeroHourPlayerState();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Resources
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 Credits = 5000;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 Energy = 100;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 TechPoints = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    float CreditsIncome = 0.0f;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    float EnergyIncome = 0.0f;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    float TechPointsIncome = 0.0f;

    // Population
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 CurrentPopulation = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 MaxPopulation = 200;

    // Faction & Team
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    EHeroHourFaction Faction = EHeroHourFaction::USA;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    EHeroHourPlayerTeam Team = EHeroHourPlayerTeam::Team1;

    // Tech tree
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    TArray<FString> ResearchedTechs;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    TArray<FString> AvailableTechs;

    // Hero
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    FString SelectedHeroID;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 HeroLevel = 1;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 HeroExperience = 0;

    // Game state
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    bool bIsDefeated = false;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    bool bIsReady = false;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    int32 Score = 0;

    // Control groups
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Player")
    TMap<int32, TArray<int32>> ControlGroups; // Unit IDs per control group

    // Selection
    UPROPERTY(BlueprintReadOnly, Category = "HeroHour|Player")
    TArray<int32> SelectedUnitIDs;

    // Resource modification
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool AddCredits(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool SpendCredits(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool AddEnergy(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool SpendEnergy(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool AddTechPoints(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool SpendTechPoints(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool CanAfford(int32 CreditCost, int32 EnergyCost, int32 TechCost) const;

    // Population
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool AddPopulation(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool RemovePopulation(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    int32 GetAvailablePopulation() const;

    // Tech tree
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool HasTech(FString TechID) const;

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool ResearchTech(FString TechID);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    bool IsTechAvailable(FString TechID) const;

    // Hero
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void SetSelectedHero(FString HeroID);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void AddHeroExperience(int32 Amount);

    // Control groups
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void SetControlGroup(int32 GroupIndex, const TArray<int32>& UnitIDs);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    TArray<int32> GetControlGroup(int32 GroupIndex) const;

    // Selection
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void SetSelectedUnits(const TArray<int32>& UnitIDs);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void AddToSelection(int32 UnitID);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void RemoveFromSelection(int32 UnitID);

    // Replication callbacks
    UFUNCTION()
    void OnRep_Credits();

    UFUNCTION()
    void OnRep_Energy();

    UFUNCTION()
    void OnRep_TechPoints();

    UFUNCTION()
    void OnRep_CurrentPopulation();

    UFUNCTION()
    void OnRep_Faction();

    UFUNCTION()
    void OnRep_Team();

    UFUNCTION()
    void OnRep_ResearchedTechs();

    UFUNCTION()
    void OnRep_IsDefeated();

    UFUNCTION()
    void OnRep_SelectedHeroID();

    UFUNCTION()
    void OnRep_HeroLevel();

    UFUNCTION()
    void OnRep_ControlGroups();

    // Score
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void AddScore(int32 Amount);

    UFUNCTION(BlueprintPure, Category = "HeroHour|Player")
    FHeroHourResourceAmount GetResourceAmount() const;

    // Team utilities
    UFUNCTION(BlueprintPure, Category = "HeroHour|Player")
    bool IsTeammate(AHeroHourPlayerState* Other) const;

    UFUNCTION(BlueprintPure, Category = "HeroHour|Player")
    bool IsEnemy(AHeroHourPlayerState* Other) const;

protected:
    // Internal resource change events
    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnCreditsChanged OnCreditsChanged;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnEnergyChanged OnEnergyChanged;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnTechPointsChanged OnTechPointsChanged;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnPopulationChanged OnPopulationChanged;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnTechResearched OnTechResearched;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnHeroChanged OnHeroChanged;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|Player")
    FOnDefeated OnDefeated;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreditsChanged, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnergyChanged, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechPointsChanged, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPopulationChanged, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechResearched, FString, TechID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, FString, HeroID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDefeated);

// Resource amount struct
USTRUCT(BlueprintType)
struct FHeroHourResourceAmount
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Credits = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Energy = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TechPoints = 0;

    FHeroHourResourceAmount() = default;
    FHeroHourResourceAmount(int32 InCredits, int32 InEnergy, int32 InTechPoints)
        : Credits(InCredits), Energy(InEnergy), TechPoints(InTechPoints) {}
};