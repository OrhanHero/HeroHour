// HeroHourGameState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HeroHourGameState.generated.h"

class AHeroHourGameMode;
class UHeroHourFSharpBridge;

UCLASS(Blueprintable)
class HEROHOUR_API AHeroHourGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AHeroHourGameState();

    // Game time
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    float GameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    int64 CurrentTick = 0;

    // Population
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    int32 TotalPopulation = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    int32 MaxPopulation = 200;

    // Resources
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    FHeroHourResourceAmount GlobalResources;

    // Players
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    TArray<TObjectPtr<AHeroHourPlayerState>> PlayerStates;

    // Game settings
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    FHeroHourGameSettings GameSettings;

    // Phase
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    EHeroHourGamePhase CurrentPhase = EHeroHourGamePhase::PreGame;

    // Winner
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Game")
    TObjectPtr<AHeroHourPlayerState> Winner = nullptr;

    // Fog of War
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|FogOfWar")
    bool bFogOfWarEnabled = true;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|FogOfWar")
    float FogRevealRange = 1000.0f;

    // Objectives
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Objectives")
    TArray<FHeroHourObjective> ActiveObjectives;

    // Map info
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "HeroHour|Map")
    FString CurrentMapName;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void UpdateFromSimulation(const FString& SimulationStateJson);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void AddPlayerState(AHeroHourPlayerState* PlayerState);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void RemovePlayerState(AHeroHourPlayerState* PlayerState);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    AHeroHourPlayerState* GetPlayerState(int32 PlayerIndex) const;

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    AHeroHourPlayerState* GetPlayerStateByID(int32 PlayerID) const;

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    int32 GetAlivePlayerCount() const;

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    TArray<AHeroHourPlayerState*> GetAlivePlayers() const;

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void SetGamePhase(EHeroHourGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void AddObjective(const FHeroHourObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void UpdateObjective(const FString& ObjectiveID, int32 CurrentCount, bool bComplete);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void RemoveObjective(const FString& ObjectiveID);

    // F# Integration
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Simulation")
    FString GetSimulationStateForSync() const;

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Replication callbacks
    UFUNCTION()
    void OnRep_GameTime();

    UFUNCTION()
    void OnRep_CurrentPhase();

    UFUNCTION()
    void OnRep_Winner();

    UFUNCTION()
    void OnRep_ActiveObjectives();
};

// HeroHourGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HeroHourGameInstance.generated.h"

UCLASS(Blueprintable)
class HEROHOUR_API UHeroHourGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UHeroHourGameInstance();

    // Initialization
    virtual void Init() override;
    virtual void Shutdown() override;

    // Settings
    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Settings")
    FHeroHourGameSettings DefaultGameSettings;

    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Settings")
    FString LastSelectedMap;

    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Settings")
    EHeroHourFaction LastSelectedFaction = EHeroHourFaction::USA;

    // Multiplayer
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Multiplayer")
    FString LastServerAddress;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Multiplayer")
    int32 LastServerPort = 7777;

    // UI
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|UI")
    bool bShowFPS = false;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|UI")
    bool bShowDebugInfo = false;

    // Audio
    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Audio")
    float MusicVolume = 0.8f;

    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Audio")
    float SFXVolume = 1.0f;

    UPROPERTY(Config, BlueprintReadWrite, Category = "HeroHour|Audio")
    float VoiceVolume = 1.0f;

    // F# Bridge
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Simulation")
    TObjectPtr<UHeroHourFSharpBridge> FSharpBridge;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    void SetDefaultGameSettings(const FHeroHourGameSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Game")
    FHeroHourGameSettings GetDefaultGameSettings() const;

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Multiplayer")
    void SetLastServer(const FString& Address, int32 Port);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Multiplayer")
    void GetLastServer(FString& OutAddress, int32& OutPort) const;

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "HeroHour|SaveGame")
    void SaveSettingsToConfig();

    UFUNCTION(BlueprintCallable, Category = "HeroHour|SaveGame")
    void LoadSettingsFromConfig();

    // F# Bridge access
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Simulation")
    UHeroHourFSharpBridge* GetFSharpBridge() const;

    // Online subsystem
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Online")
    void HostGame(const FString& MapName, const FHeroHourGameSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Online")
    void JoinGame(const FString& Address);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Online")
    void LeaveGame();
};