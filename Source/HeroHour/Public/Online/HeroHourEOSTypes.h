// HeroHourEOSTypes.h
// EOS-specific data structures for HeroHour

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "HeroHourEOSTypes.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType, Category = "HeroHour|EOS")
enum class EHeroHourPresenceState : uint8
{
    Offline = 0,
    Online = 1,
    InMenu = 2,
    InLobby = 3,
    InGame = 4,
    InMatch = 5,
    Away = 6,
    ExtendedAway = 7
};

UENUM(BlueprintType, Category = "HeroHour|EOS")
enum class EHeroHourSessionType : uint8
{
    PublicMatch,
    PrivateMatch,
    RankedMatch,
    CustomGame,
    Campaign,
    Tutorial
};

UENUM(BlueprintType, Category = "HeroHour|EOS")
enum class EHeroHourVictoryCondition : uint8
{
    Annihilation,
    Domination,
    Economic,
    Technological,
    Objective,
    TimeLimit
};

// ============================================================================
// Session Settings
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|Session")
struct FHeroHourSessionSettings
{
    GENERATED_BODY()

public:
    FHeroHourSessionSettings()
    {
        NumPublicConnections = 4;
        NumPrivateConnections = 0;
        bShouldAdvertise = true;
        bAllowJoinInProgress = true;
        bAllowJoinViaPresence = true;
        bAllowInvites = true;
        bUsesPresence = true;
        bUseLobbiesIfAvailable = true;
        bIsDedicated = false;
        bIsLobby = false;

        BuildUniqueId = 1;
        MatchDuration = 3600;
        PopulationCap = 200;
        FogOfWarEnabled = true;
        StartingCredits = 5000;
        StartingEnergy = 100;
        TechPointsEnabled = true;
        SessionType = EHeroHourSessionType::PublicMatch;
    }

    // Standard session settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    FName SessionName = NAME_GameSession;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    int32 NumPublicConnections;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    int32 NumPrivateConnections;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bShouldAdvertise;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bAllowJoinInProgress;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bAllowJoinViaPresence;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bAllowInvites;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bUsesPresence;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bUseLobbiesIfAvailable;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bIsDedicated;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bIsLobby;

    // HeroHour specific settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    int32 BuildUniqueId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    int32 MatchDuration; // seconds, 0 = unlimited

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    int32 PopulationCap;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    bool FogOfWarEnabled;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    int32 StartingCredits;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    int32 StartingEnergy;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    bool TechPointsEnabled;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    EHeroHourSessionType SessionType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    FString MapName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    TArray<FString> AllowedFactions;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroHour|Session")
    TArray<EHeroHourVictoryCondition> VictoryConditions;

    // Convert to FOnlineSessionSettings
    void ToOnlineSessionSettings(FOnlineSessionSettings& OutSettings) const
    {
        OutSettings.NumPublicConnections = NumPublicConnections;
        OutSettings.NumPrivateConnections = NumPrivateConnections;
        OutSettings.bShouldAdvertise = bShouldAdvertise;
        OutSettings.bAllowJoinInProgress = bAllowJoinInProgress;
        OutSettings.bAllowJoinViaPresence = bAllowJoinViaPresence;
        OutSettings.bAllowInvites = bAllowInvites;
        OutSettings.bUsesPresence = bUsesPresence;
        OutSettings.bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
        OutSettings.bIsDedicated = bIsDedicated;
        OutSettings.bIsLobby = bIsLobby;

        // HeroHour settings
        SETTING_MAP(OutSettings, BuildUniqueId);
        SETTING_MAP(OutSettings, MatchDuration);
        SETTING_MAP(OutSettings, PopulationCap);
        SETTING_MAP(OutSettings, FogOfWarEnabled);
        SETTING_MAP(OutSettings, StartingCredits);
        SETTING_MAP(OutSettings, StartingEnergy);
        SETTING_MAP(OutSettings, TechPointsEnabled);
        SETTING_MAP(OutSettings, SessionType);
        SETTING_MAP(OutSettings, MapName);
        SETTING_MAP(OutSettings, AllowedFactions);
        SETTING_MAP(OutSettings, VictoryConditions);
    }

    // Convert from FOnlineSessionSettings
    void FromOnlineSessionSettings(const FOnlineSessionSettings& InSettings)
    {
        NumPublicConnections = InSettings.NumPublicConnections;
        NumPrivateConnections = InSettings.NumPrivateConnections;
        bShouldAdvertise = InSettings.bShouldAdvertise;
        bAllowJoinInProgress = InSettings.bAllowJoinInProgress;
        bAllowJoinViaPresence = InSettings.bAllowJoinViaPresence;
        bAllowInvites = InSettings.bAllowInvites;
        bUsesPresence = InSettings.bUsesPresence;
        bUseLobbiesIfAvailable = InSettings.bUseLobbiesIfAvailable;
        bIsDedicated = InSettings.bIsDedicated;
        bIsLobby = InSettings.bIsLobby;

        GETTING_MAP(InSettings, BuildUniqueId, int32);
        GETTING_MAP(InSettings, MatchDuration, int32);
        GETTING_MAP(InSettings, PopulationCap, int32);
        GETTING_MAP(InSettings, FogOfWarEnabled, bool);
        GETTING_MAP(InSettings, StartingCredits, int32);
        GETTING_MAP(InSettings, StartingEnergy, int32);
        GETTING_MAP(InSettings, TechPointsEnabled, bool);
        GETTING_MAP(InSettings, SessionType, EHeroHourSessionType);
        GETTING_MAP(InSettings, MapName, FString);
        GETTING_MAP(InSettings, AllowedFactions, TArray<FString>);
        GETTING_MAP(InSettings, VictoryConditions, TArray<EHeroHourVictoryCondition>);
    }
};

// ============================================================================
// Session Search Parameters
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|SessionSearch")
struct FHeroHourSessionSearchParams
{
    GENERATED_BODY()

public:
    FHeroHourSessionSearchParams()
    {
        MaxResults = 50;
        bAllowEmptyServers = true;
        bAllowFullServers = false;
        bUseLobbiesIfAvailable = true;
        PingBucketSize = 50;
        MinPing = 0;
        MaxPing = 200;
        SearchLobbies = true;

        MinPopulationCap = 0;
        MaxPopulationCap = 200;
        RequireFogOfWar = false;
        RequireTechPoints = false;
        GameModeFilter = TEXT("Standard");
    }

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    int32 MaxResults;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    bool bAllowEmptyServers;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    bool bAllowFullServers;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    bool bUseLobbiesIfAvailable;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    int32 PingBucketSize;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    int32 MinPing;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    int32 MaxPing;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    bool SearchLobbies;

    // Game-specific filters
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    int32 MinPopulationCap;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    int32 MaxPopulationCap;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    bool RequireFogOfWar;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    bool RequireTechPoints;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    FString GameModeFilter;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    FString MapFilter;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    TArray<EHeroHourVictoryCondition> RequiredVictoryConditions;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|SessionSearch")
    FString RegionFilter;

    void ToOnlineSessionSearch(FOnlineSessionSearch& OutSearch) const
    {
        OutSearch.MaxSearchResults = MaxResults;
        OutSearch.bAllowEmptyServers = bAllowEmptyServers;
        OutSearch.bAllowFullServers = bAllowFullServers;
        OutSearch.bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
        OutSearch.PingBucketSize = PingBucketSize;

        // Add query settings
        OutSearch.QuerySettings.Set(SEARCH_PRESENCE, bUseLobbiesIfAvailable, EOnlineComparisonOp::Equals);

        if (!MapFilter.IsEmpty())
        {
            OutSearch.QuerySettings.Set(SEARCH_KEYWORDS, MapFilter, EOnlineComparisonOp::Equals);
        }

        if (!GameModeFilter.IsEmpty())
        {
            OutSearch.QuerySettings.Set(TEXT("GameMode"), GameModeFilter, EOnlineComparisonOp::Equals);
        }
    }
};

// ============================================================================
// Matchmaking Parameters
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|Matchmaking")
struct FHeroHourMatchmakingParams
{
    GENERATED_BODY()

public:
    FHeroHourMatchmakingParams()
    {
        BucketId = TEXT("Default");
        MinPlayers = 2;
        MaxPlayers = 4;
        TargetPlayerCount = 4;
        TimeoutSeconds = 120;
        bUseSkillBasedMatchmaking = true;
        SkillRating = 1500;
        SkillDeviation = 350;
        bAllowCrossRegion = false;
        PreferredRegion = TEXT("auto");
    }

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    FString BucketId;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    int32 MinPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    int32 MaxPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    int32 TargetPlayerCount;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    int32 TimeoutSeconds;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    bool bUseSkillBasedMatchmaking;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    int32 SkillRating;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    int32 SkillDeviation;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    bool bAllowCrossRegion;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    FString PreferredRegion;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    TArray<FString> RequiredMaps;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    TArray<FString> ExcludedMaps;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Matchmaking")
    FString GameMode;
};

// ============================================================================
// Stat Updates
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|Stats")
struct FHeroHourStatUpdate
{
    GENERATED_BODY()

public:
    FHeroHourStatUpdate() : Value(0), bIncrement(false) {}

    FHeroHourStatUpdate(const FString& InName, int64 InValue, bool bInIncrement = false)
        : Name(InName), Value(InValue), bIncrement(bInIncrement) {}

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Stats")
    FString Name;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Stats")
    int64 Value;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Stats")
    bool bIncrement;
};

// ============================================================================
// Leaderboard
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|Leaderboards")
struct FHeroHourLeaderboardEntry
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Leaderboards")
    int32 Rank = 0;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Leaderboards")
    FString UserId;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Leaderboards")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Leaderboards")
    int64 Score = 0;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Leaderboards")
    FString Metadata;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Leaderboards")
    FDateTime Timestamp;
};

// ============================================================================
// Achievements
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|Achievements")
struct FHeroHourAchievementDefinition
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    FString AchievementId;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    FString IconUrl;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    bool bIsHidden = false;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    int32 ProgressMax = 1;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|Achievements")
    FString StatName;
};

// ============================================================================
// Cloud Storage
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS|CloudStorage")
struct FHeroHourCloudSaveSlot
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|CloudStorage")
    FString FileName;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|CloudStorage")
    FDateTime LastModified;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|CloudStorage")
    int64 FileSize = 0;

    UPROPERTY(BlueprintReadWrite, Category = "HeroHour|CloudStorage")
    FString Metadata;
};

// ============================================================================
// Cloud Storage Event Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSCloudStorageWriteComplete, const FString&, FileName, bool, bWasSuccessful, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSCloudStorageReadComplete, const FString&, FileName, const TArray<uint8>&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSCloudStorageDeleteComplete, const FString&, FileName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSCloudStorageListComplete, const TArray<FHeroHourCloudSaveSlot>&, Files, bool, bWasSuccessful);