// HeroHourGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeroHourGameMode.generated.h"

class AHeroHourPlayerController;
class AHeroHourGameState;
class AHeroHourPlayerState;
class UHeroHourFSharpBridge;

UCLASS(Blueprintable)
class HEROHOUR_API AHeroHourGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHeroHourGameMode();

    // Game initialization
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void StartMatch() override;
    virtual void EndMatch() override;

    // Game settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Game")
    FHeroHourGameSettings GameSettings;

    // F# Simulation Bridge
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Simulation")
    TObjectPtr<UHeroHourFSharpBridge> FSharpBridge;

    // Classes
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Classes")
    TSubclassOf<AHeroHourPlayerController> HeroHourPlayerControllerClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Classes")
    TSubclassOf<AHeroHourGameState> HeroHourGameStateClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Classes")
    TSubclassOf<AHeroHourPlayerState> HeroHourPlayerStateClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Classes")
    TSubclassOf<APawn> DefaultPawnClass;

    // Map & Factions
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HeroHour|Map")
    TMap<FString, FHeroHourFactionData> FactionDataMap;

    // Game state
    UPROPERTY(BlueprintReadOnly, Category = "HeroHour|Game")
    EHeroHourGamePhase CurrentPhase = EHeroHourGamePhase::PreGame;

    // Timer
    UPROPERTY()
    FTimerHandle GameTimerHandle;

    UPROPERTY()
    FTimerHandle ResourceTickHandle;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void SetGamePhase(EHeroHourGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    bool CheckVictoryConditions();

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void DeclareVictory(AHeroHourPlayerState* Winner);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void DeclareDefeat(AHeroHourPlayerState* Loser);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void HandlePlayerDefeated(AHeroHourPlayerState* Player);

    // Resource tick
    UFUNCTION()
    void OnResourceTick();

    // Player management
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void AddPlayer(AHeroHourPlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Player")
    void RemovePlayer(AHeroHourPlayerState* Player);

    // Faction
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Faction")
    FHeroHourFactionData GetFactionData(EHeroHourFaction Faction) const;

    // Pause
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void PauseGame();

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void ResumeGame();

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void RequestSaveGame(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void RequestLoadGame(const FString& SlotName);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Timer callbacks
    void OnGameTimerFinished();
    void UpdateGamePhase();

    // Victory checking
    void CheckAnnihilationVictory();
    void CheckDominationVictory();
    void CheckEconomicVictory();
    void CheckObjectiveVictory();
    void CheckTimeLimitVictory();

    // Player state
    void OnPlayerStateChanged(AHeroHourPlayerState* Player);

    // F# Integration
    void InitializeFSharpSimulation();
    void ProcessFSharpCommands();
    void ApplyFSharpResults();
};

// Faction data
USTRUCT(BlueprintType)
struct FHeroHourFactionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHeroHourFaction Faction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> StartingUnitTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> StartingBuildingTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UniqueTechs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UniqueUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UniqueBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HeroClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSlateColor FactionColor;
};