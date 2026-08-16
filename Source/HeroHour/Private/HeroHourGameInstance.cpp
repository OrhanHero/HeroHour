// HeroHourGameInstance.cpp
// GameInstance Implementation for HeroHour

#include "HeroHourGameInstance.h"
#include "Online/HeroHourEOSSubsystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UHeroHourGameInstance::UHeroHourGameInstance()
{
    // Initialize default settings
    DefaultGameSettings.MatchDuration = 3600.0f;
    DefaultGameSettings.StartingResources = FHeroHourResourceAmount(5000, 100, 0);
    DefaultGameSettings.PopulationCap = 200;
    DefaultGameSettings.bFogOfWarEnabled = true;
    DefaultGameSettings.FogRevealRange = 1000.0f;
    DefaultGameSettings.FogReconRange = 2000.0f;
    DefaultGameSettings.ResourceTickRate = 0.5f;
    DefaultGameSettings.IncomeMultiplier = 1.0f;
    DefaultGameSettings.VictoryConditions = { EHeroHourVictoryCondition::Annihilation };

    CachedSettings = DefaultGameSettings;
}

void UHeroHourGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour GameInstance Initializing..."));

    // Get EOS subsystem
    EOSSubsystem = GetSubsystem<UHeroHourEOSSubsystem>();

    if (EOSSubsystem)
    {
        // Initialize EOS
        bool bEOSInit = InitializeEOS();
        if (bEOSInit)
        {
            UE_LOG(LogHeroHourEOS, Log, TEXT("EOS Initialized Successfully"));
        }
        else
        {
            UE_LOG(LogHeroHourEOS, Warning, TEXT("EOS Initialization Failed - Online features unavailable"));
        }
    }
    else
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Failed to get EOS Subsystem!"));
    }

    // Load saved settings
    LoadSettingsFromConfig();

    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour GameInstance Initialized"));
}

void UHeroHourGameInstance::Shutdown()
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour GameInstance Shutting Down..."));

    // Shutdown EOS
    ShutdownEOS();

    // Save settings
    SaveSettingsToConfig();

    Super::Shutdown();

    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour GameInstance Shutdown Complete"));
}

UHeroHourEOSSubsystem* UHeroHourGameInstance::GetEOSSubsystem() const
{
    return EOSSubsystem;
}

bool UHeroHourGameInstance::IsEOSReady() const
{
    return EOSSubsystem != nullptr && EOSSubsystem->IsEOSInitialized();
}

bool UHeroHourGameInstance::InitializeEOS()
{
    if (EOSSubsystem)
    {
        return EOSSubsystem->InitializeEOSPlatform();
    }
    return false;
}

void UHeroHourGameInstance::ShutdownEOS()
{
    if (EOSSubsystem)
    {
        EOSSubsystem->ShutdownEOSPlatform();
    }
}

void UHeroHourGameInstance::SetDefaultGameSettings(const FHeroHourGameSettings& Settings)
{
    DefaultGameSettings = Settings;
    CachedSettings = Settings;
    SaveSettingsToConfig();
}

FHeroHourGameSettings UHeroHourGameInstance::GetDefaultGameSettings() const
{
    return CachedSettings;
}

void UHeroHourGameInstance::SetLastServer(const FString& Address, int32 Port)
{
    LastServerAddress = Address;
    LastServerPort = Port;
    SaveSettingsToConfig();
}

void UHeroHourGameInstance::GetLastServer(FString& OutAddress, int32& OutPort) const
{
    OutAddress = LastServerAddress;
    OutPort = LastServerPort;
}

void UHeroHourGameInstance::SaveSettingsToConfig()
{
    // Config values are automatically saved via UPROPERTY(Config)
    // But we can force flush here
    FlushConfig();
}

void UHeroHourGameInstance::LoadSettingsFromConfig()
{
    // Config values are automatically loaded via UPROPERTY(Config)
    CachedSettings = DefaultGameSettings;
}

void UHeroHourGameInstance::HostGame(const FString& MapName, const FHeroHourGameSettings& Settings)
{
    if (!EOSSubsystem || !EOSSubsystem->IsEOSInitialized())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Cannot host game: EOS not initialized"));
        return;
    }

    LastSelectedMap = MapName;
    CachedSettings = Settings;
    SaveSettingsToConfig();

    // Create session settings
    FHeroHourSessionSettings SessionSettings;
    SessionSettings.SessionName = NAME_GameSession;
    SessionSettings.NumPublicConnections = 4;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bUseLobbiesIfAvailable = true;
    SessionSettings.MapName = MapName;
    SessionSettings.MatchDuration = Settings.MatchDuration;
    SessionSettings.PopulationCap = Settings.PopulationCap;
    SessionSettings.bFogOfWarEnabled = Settings.bFogOfWarEnabled;

    EOSSubsystem->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UHeroHourGameInstance::JoinGame(const FString& Address)
{
    LastServerAddress = Address;
    SaveSettingsToConfig();

    // For EOS, we join via session search results, not direct IP
    // This would typically come from a session search result
    UE_LOG(LogHeroHourEOS, Log, TEXT("Join game requested: %s"), *Address);
}

void UHeroHourGameInstance::LeaveGame()
{
    if (EOSSubsystem && EOSSubsystem->IsEOSInitialized())
    {
        EOSSubsystem->DestroySession(NAME_GameSession);
    }

    // Travel back to main menu
    if (GetWorld())
    {
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
    }
}