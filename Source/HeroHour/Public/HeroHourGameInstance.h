// HeroHourGameInstance.h
// GameInstance for HeroHour with EOS Integration

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Online/HeroHourEOSSubsystem.h"
#include "HeroHourGameInstance.generated.h"

/**
 * HeroHour Game Instance
 * Manages game-wide state and provides access to EOS subsystem
 */
UCLASS(Blueprintable, BlueprintType)
class HEROHOUR_API UHeroHourGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UHeroHourGameInstance();

    // ~ UGameInstance interface
    virtual void Init() override;
    virtual void Shutdown() override;
    // ~ End UGameInstance interface

    // =========================================================================
    // EOS Subsystem Access
    // =========================================================================

    /** Get the EOS subsystem */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS")
    UHeroHourEOSSubsystem* GetEOSSubsystem() const;

    /** Check if EOS is available and initialized */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS")
    bool IsEOSReady() const;

    /** Initialize EOS platform (called automatically on Init) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS")
    bool InitializeEOS();

    /** Shutdown EOS platform */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS")
    void ShutdownEOS();

    // =========================================================================
    // Game Settings (Persisted)
    // =========================================================================

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

    // =========================================================================
    // Game Instance Functions
    // =========================================================================

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

    // Online
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Online")
    void HostGame(const FString& MapName, const FHeroHourGameSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Online")
    void JoinGame(const FString& Address);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Online")
    void LeaveGame();

protected:
    // EOS Subsystem reference
    UPROPERTY()
    TObjectPtr<UHeroHourEOSSubsystem> EOSSubsystem;

    // Cached settings
    UPROPERTY()
    FHeroHourGameSettings CachedSettings;
};