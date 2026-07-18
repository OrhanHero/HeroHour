// HeroHourEOSSubsystem.h
// Epic Online Services (EOS) Integration for HeroHour
// Organization: HERO (hero-c31fffa8) | Product: HERO HOUR (9069f4f0eda147c9a341ea081b5e9803)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "HeroHourEOSSubsystem.generated.h"

// Forward declarations
class IOnlineSubsystemEOS;

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSLoginCompleteDelegate, bool, bWasSuccessful, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSLogoutCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSAuthLoginChangedDelegate, TSharedPtr<const FUniqueNetId>, UserId, ELoginStatus::Type, LoginStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSFriendsListUpdatedDelegate, const TArray<TSharedRef<FOnlineFriend>>&, Friends);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSPresenceUpdatedDelegate, TSharedPtr<const FUniqueNetId>, UserId, const FOnlineUserPresence&, Presence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionCreateCompleteDelegate, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionJoinCompleteDelegate, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionJoinCompleteWithConnectStringDelegate, FName, SessionName, bool, bWasSuccessful, const FString&, ConnectString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionDestroyCompleteDelegate, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSSessionFindCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionStartCompleteDelegate, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionEndCompleteDelegate, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSMatchmakingCompleteDelegate, bool, bWasSuccessful, const FString&, TicketId, const FOnlineSessionSearchResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSLeaderboardReadCompleteDelegate, const TArray<FHeroHourLeaderboardEntry>&, Entries, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSLeaderboardWriteCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSAchievementUnlockedDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSQueryAchievementsCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSStatsUpdateCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSQueryStatsCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSCloudStorageWriteCompleteDelegate, const FString&, FileName, bool, bWasSuccessful, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSCloudStorageReadCompleteDelegate, const FString&, FileName, const TArray<uint8>&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSCloudStorageDeleteCompleteDelegate, const FString&, FileName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSCloudStorageListCompleteDelegate, const TArray<FHeroHourCloudSaveSlot>&, Slots, bool, bWasSuccessful);

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

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType, Category = "HeroHour|EOS")
struct FHeroHourSessionSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    FName SessionName = NAME_GameSession;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    int32 NumPublicConnections = 4;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    int32 NumPrivateConnections = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bShouldAdvertise = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bAllowJoinInProgress = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bAllowInvites = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bUseLobbiesIfAvailable = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bUseLobbiesVoiceChatIfAvailable = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bIsDedicated = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bIsLobby = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    bool bAntiCheatProtected = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    EHeroHourSessionType SessionType = EHeroHourSessionType::PublicMatch;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    FString MapName = TEXT("MainMenu");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    FString GameMode = TEXT("Standard");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Session")
    FString BucketId = TEXT("default");

    // Game-specific settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    float MatchDuration = 3600.0f; // seconds

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    int32 PopulationCap = 200;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    bool bFogOfWarEnabled = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    float FogRevealRange = 1000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    float ResourceTickRate = 0.5f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    float IncomeMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSettings")
    TArray<int32> VictoryConditions; // 0=Annihilation, 1=Domination, 2=Economic, etc.

    void ToOnlineSessionSettings(FOnlineSessionSettings& OutSettings) const;
};

USTRUCT(BlueprintType, Category = "HeroHour|EOS")
struct FHeroHourSessionSearchParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    int32 MaxResults = 50;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    bool bAllowEmptyServers = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    bool bAllowFullServers = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    bool bUseLobbiesIfAvailable = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    int32 PingBucketSize = 50;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    FString MapFilter;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    FString GameModeFilter;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    EHeroHourSessionType SessionTypeFilter = EHeroHourSessionType::PublicMatch;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    int32 MinPlayers = 2;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Search")
    int32 MaxPlayers = 4;

    // Skill-based filters
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill")
    bool bFilterBySkill = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill")
    int32 MinSkillRating = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill")
    int32 MaxSkillRating = 10000;
};

USTRUCT(BlueprintType, Category = "HeroHour|EOS")
struct FHeroHourMatchmakingParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString BucketId = TEXT("default");

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    int32 MinPlayers = 2;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    int32 MaxPlayers = 4;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bUseSkillBasedMatchmaking = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    int32 SkillRating = 1000;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    int32 SkillDeviation = 200;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    float MaxWaitTime = 120.0f; // seconds

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    TMap<FString, FString> AdditionalAttributes;
};

USTRUCT(BlueprintType, Category = "HeroHour|EOS")
struct FHeroHourStatUpdate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int64 Value = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIncrement = false; // true = add to current, false = set absolute

    FHeroHourStatUpdate() = default;
    FHeroHourStatUpdate(const FString& InName, int64 InValue, bool bInIncrement = false)
        : Name(InName), Value(InValue), bIncrement(bInIncrement) {}
};

USTRUCT(BlueprintType, Category = "HeroHour|EOS")
struct FHeroHourLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Rank = 0;

    UPROPERTY(BlueprintReadWrite)
    FString UserId;

    UPROPERTY(BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite)
    int64 Score = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Metadata;

    UPROPERTY(BlueprintReadWrite)
    FDateTime Timestamp;
};

USTRUCT(BlueprintType, Category = "HeroHour|EOS")
struct FHeroHourCloudSaveSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString FileName;

    UPROPERTY(BlueprintReadWrite)
    FDateTime LastModified;

    UPROPERTY(BlueprintReadWrite)
    int64 FileSize = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Metadata;
};

// ============================================================================
// HeroHourEOSSubsystem
// ============================================================================

/**
 * HeroHour EOS Subsystem
 * Manages all Epic Online Services integration for the game
 * Provides a clean Blueprint-friendly interface for:
 * - Authentication (EOS Auth / EOS Connect)
 * - Friends & Presence
 * - Sessions & Lobbies
 * - Matchmaking
 * - Leaderboards & Achievements
 * - Player Stats
 * - Cloud Storage (Player Data)
 */
UCLASS(BlueprintType, Blueprintable, Category = "HeroHour|Online|EOS")
class HEROHOUR_API UHeroHourEOSSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ~ UGameInstanceSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // ~ End UGameInstanceSubsystem interface

    // =========================================================================
    // Initialization & Configuration
    // =========================================================================

    /** Initialize EOS platform with configuration from DefaultEngine.ini */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Init")
    bool InitializeEOSPlatform();

    /** Shutdown EOS platform */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Init")
    void ShutdownEOSPlatform();

    /** Check if EOS is initialized and ready */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Init")
    bool IsEOSInitialized() const { return bIsInitialized; }

    /** Get the EOS platform interface */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Init")
    IOnlineSubsystemEOS* GetEOSSubsystem() const;

    /** Get the online subsystem */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Init")
    IOnlineSubsystem* GetOnlineSubsystem() const;

    // =========================================================================
    // Authentication (EOS Auth / EOS Connect)
    // =========================================================================

    /** Login with EOS Auth (account portal) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Auth")
    void LoginWithEOSAuth(int32 LocalUserNum, const FString& Id, const FString& Token, const FString& Type);

    /** Login with EOS Connect (device ID, Epic, Steam, etc.) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Auth")
    void LoginWithEOSConnect(int32 LocalUserNum, const FString& Id, const FString& Token, const FString& Type);

    /** Login with Developer Auth Tool (for testing) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Auth")
    void LoginWithDeveloperAuth(int32 LocalUserNum, const FString& AccountId);

    /** Login with Device ID (anonymous) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Auth")
    void LoginWithDeviceId(int32 LocalUserNum);

    /** Logout current user */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Auth")
    void Logout(int32 LocalUserNum);

    /** Get current login status */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Auth")
    ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const;

    /** Get logged in user's unique net ID */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Auth")
    TSharedPtr<const FUniqueNetId> GetUserId(int32 LocalUserNum) const;

    /** Get logged in user's display name */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Auth")
    FString GetUserDisplayName(int32 LocalUserNum) const;

    /** Get logged in user's Epic Account ID */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Auth")
    FString GetUserEpicAccountId(int32 LocalUserNum) const;

    /** Check if user is logged in */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Auth")
    bool IsUserLoggedIn(int32 LocalUserNum) const;

    /** Get auth token for backend validation */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Auth")
    void GetAuthToken(int32 LocalUserNum, FString& OutToken) const;

    // =========================================================================
    // Friends & Presence
    // =========================================================================

    /** Query friends list for local user */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Friends")
    void QueryFriends(int32 LocalUserNum);

    /** Get friends list */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Friends")
    void GetFriendsList(int32 LocalUserNum, TArray<FOnlineFriend>& OutFriends) const;

    /** Get friend count */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Friends")
    int32 GetFriendsCount(int32 LocalUserNum) const;

    /** Send friend invite */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Friends")
    void SendFriendInvite(int32 LocalUserNum, const FString& TargetUserId);

    /** Accept friend invite */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Friends")
    void AcceptFriendInvite(int32 LocalUserNum, const FString& TargetUserId);

    /** Reject friend invite */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Friends")
    void RejectFriendInvite(int32 LocalUserNum, const FString& TargetUserId);

    /** Remove friend */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Friends")
    void RemoveFriend(int32 LocalUserNum, const FString& TargetUserId);

    /** Set user presence status */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Presence")
    void SetPresence(int32 LocalUserNum, EHeroHourPresenceState State, const FString& StatusStr);

    /** Get user presence */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Presence")
    void GetPresence(int32 LocalUserNum, const TSharedPtr<const FUniqueNetId>& UserId, FOnlineUserPresence& OutPresence) const;

    /** Query presence for multiple users */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Presence")
    void QueryPresence(int32 LocalUserNum, const TArray<TSharedPtr<const FUniqueNetId>>& UserIds);

    // =========================================================================
    // Sessions & Lobbies
    // =========================================================================

    /** Create a new session (host game) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void CreateSession(int32 LocalUserNum, FName SessionName, const FHeroHourSessionSettings& Settings);

    /** Join an existing session */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

    /** Find sessions */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void FindSessions(int32 LocalUserNum, const FHeroHourSessionSearchParams& SearchParams);

    /** Destroy current session */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void DestroySession(FName SessionName);

    /** Start session (begin match) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void StartSession(FName SessionName);

    /** End session (end match) */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void EndSession(FName SessionName);

    /** Get current session connect string for travel */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    FString GetSessionConnectString(FName SessionName) const;

    /** Register player to session */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void RegisterPlayer(FName SessionName, const FString& PlayerName, bool& bWasSuccessful);

    /** Unregister player from session */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Sessions")
    void UnregisterPlayer(FName SessionName, const FString& PlayerName, bool& bWasSuccessful);

    // =========================================================================
    // Matchmaking
    // =========================================================================

    /** Start matchmaking search */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Matchmaking")
    void StartMatchmaking(int32 LocalUserNum, const FHeroHourMatchmakingParams& Params);

    /** Cancel matchmaking */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Matchmaking")
    void CancelMatchmaking(int32 LocalUserNum);

    // =========================================================================
    // Leaderboards
    // =========================================================================

    /** Write leaderboard score */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Leaderboards")
    void WriteLeaderboardScore(int32 LocalUserNum, const FString& LeaderboardName, int64 Score, const FString& Metadata);

    /** Read leaderboard scores */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Leaderboards")
    void ReadLeaderboardScores(int32 LocalUserNum, const FString& LeaderboardName, int32 StartRank, int32 MaxResults);

    /** Read leaderboard scores around user */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Leaderboards")
    void ReadLeaderboardAroundUser(int32 LocalUserNum, const FString& LeaderboardName, int32 Range);

    // =========================================================================
    // Achievements
    // =========================================================================

    /** Unlock achievement */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Achievements")
    void UnlockAchievement(int32 LocalUserNum, const FString& AchievementId);

    /** Query achievements */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Achievements")
    void QueryAchievements(int32 LocalUserNum);

    /** Get achievement progress */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Achievements")
    void GetAchievementProgress(int32 LocalUserNum, const FString& AchievementId, float& OutProgress, bool& OutUnlocked) const;

    // =========================================================================
    // Player Stats
    // =========================================================================

    /** Update player stats */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Stats")
    void UpdateStats(int32 LocalUserNum, const TArray<FHeroHourStatUpdate>& StatUpdates);

    /** Query player stats */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|Stats")
    void QueryStats(int32 LocalUserNum, const TArray<FString>& StatNames);

    // =========================================================================
    // Player Data Storage (Cloud Save)
    // =========================================================================

    /** Write player data to cloud storage */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|CloudStorage")
    void WritePlayerData(int32 LocalUserNum, const FString& FileName, const TArray<uint8>& Data);

    /** Read player data from cloud storage */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|CloudStorage")
    void ReadPlayerData(int32 LocalUserNum, const FString& FileName);

    /** Delete player data from cloud storage */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|CloudStorage")
    void DeletePlayerData(int32 LocalUserNum, const FString& FileName);

    /** List files in cloud storage */
    UFUNCTION(BlueprintCallable, Category = "HeroHour|EOS|CloudStorage")
    void ListPlayerDataFiles(int32 LocalUserNum);

    // =========================================================================
    // Events (Blueprint Assignable)
    // =========================================================================

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSLoginCompleteDelegate OnLoginComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSLogoutCompleteDelegate OnLogoutComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSAuthLoginChangedDelegate OnAuthLoginChanged;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSFriendsListUpdatedDelegate OnFriendsListUpdated;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSPresenceUpdatedDelegate OnPresenceUpdated;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionCreateCompleteDelegate OnSessionCreateComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionJoinCompleteDelegate OnSessionJoinComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionJoinCompleteWithConnectStringDelegate OnSessionJoinCompleteWithConnectString;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionDestroyCompleteDelegate OnSessionDestroyComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionFindCompleteDelegate OnSessionFindComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionStartCompleteDelegate OnSessionStartComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSSessionEndCompleteDelegate OnSessionEndComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSMatchmakingCompleteDelegate OnMatchmakingComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSLeaderboardReadCompleteDelegate OnLeaderboardReadComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSLeaderboardWriteCompleteDelegate OnLeaderboardWriteComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSAchievementUnlockedDelegate OnAchievementUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSQueryAchievementsCompleteDelegate OnQueryAchievementsComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSStatsUpdateCompleteDelegate OnStatsUpdateComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSQueryStatsCompleteDelegate OnQueryStatsComplete;

    // Cloud storage events
    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSCloudStorageWriteCompleteDelegate OnCloudStorageWriteComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSCloudStorageReadCompleteDelegate OnCloudStorageReadComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSCloudStorageDeleteCompleteDelegate OnCloudStorageDeleteComplete;

    UPROPERTY(BlueprintAssignable, Category = "HeroHour|EOS|Events")
    FOnEOSCloudStorageListCompleteDelegate OnCloudStorageListComplete;

    // =========================================================================
    // Utility
    // =========================================================================

    /** Check if EOS is the active online subsystem */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Utils")
    bool IsEOSAvailable() const;

    /** Get EOS SDK version */
    UFUNCTION(BlueprintPure, Category = "HeroHour|EOS|Utils")
    FString GetEOSVersion() const;

protected:
    // Internal EOS interfaces
    TSharedPtr<IOnlineSubsystem> OnlineSubsystem;
    TSharedPtr<IOnlineSubsystemEOS> EOSSubsystem;
    TSharedPtr<IOnlineIdentity> IdentityInterface;
    TSharedPtr<IOnlineFriends> FriendsInterface;
    TSharedPtr<IOnlinePresence> PresenceInterface;
    TSharedPtr<IOnlineSession> SessionInterface;
    TSharedPtr<IOnlineLeaderboards> LeaderboardsInterface;
    TSharedPtr<IOnlineAchievements> AchievementsInterface;
    TSharedPtr<IOnlineStats> StatsInterface;
    TSharedPtr<IOnlineUserCloud> UserCloudInterface;

    // Delegate handles
    FDelegateHandle LoginCompleteDelegateHandle;
    FDelegateHandle LogoutCompleteDelegateHandle;
    FDelegateHandle LoginStatusChangedDelegateHandle;
    FDelegateHandle FriendsListUpdatedDelegateHandle;
    FDelegateHandle PresenceUpdatedDelegateHandle;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FDelegateHandle StartSessionCompleteDelegateHandle;
    FDelegateHandle EndSessionCompleteDelegateHandle;
    FDelegateHandle LeaderboardReadCompleteDelegateHandle;
    FDelegateHandle LeaderboardWriteCompleteDelegateHandle;
    FDelegateHandle AchievementWrittenCompleteDelegateHandle;
    FDelegateHandle AchievementQueryCompleteDelegateHandle;
    FDelegateHandle StatsUpdateCompleteDelegateHandle;
    FDelegateHandle StatsQueryCompleteDelegateHandle;
    FDelegateHandle CloudWriteCompleteDelegateHandle;
    FDelegateHandle CloudReadCompleteDelegateHandle;
    FDelegateHandle CloudDeleteCompleteDelegateHandle;
    FDelegateHandle CloudListCompleteDelegateHandle;

    // Configuration
    UPROPERTY()
    FString ProductId;

    UPROPERTY()
    FString SandboxId;

    UPROPERTY()
    FString DeploymentId;

    UPROPERTY()
    FString ClientId;

    UPROPERTY()
    FString ClientSecret;

    UPROPERTY()
    FString EncryptionKey;

    // State
    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    bool bIsLoggedIn = false;

    UPROPERTY()
    TSharedPtr<const FUniqueNetId> LoggedInUserId;

    UPROPERTY()
    FName CurrentSessionName = NAME_None;

    UPROPERTY()
    int32 CurrentLocalUserNum = 0;

    UPROPERTY()
    int32 LoginRetryCount = 0;

    // Session search
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    // Helper functions
    void CacheInterfaces();
    void BindDelegates();
    void UnbindDelegates();
    void LoadConfiguration();

    // Auth helpers
    bool Login(FString PlayerName, FString Password, FString AuthType);
    bool LoginWithEOSConnect(const FString& IdToken, const FString& AuthType);
    bool LoginWithDeviceId();

    // Delegate callbacks
    void OnLoginCompleteCallback(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
    void OnLogoutCompleteCallback(int32 LocalUserNum, bool bWasSuccessful);
    void OnLoginStatusChangedCallback(int32 LocalUserNum, ELoginStatus::Type PreviousStatus, ELoginStatus::Type CurrentStatus, const FUniqueNetId& UserId);
    void OnFriendsListReadComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
    void OnFriendsListUpdatedCallback(int32 LocalUserNum);
    void OnPresenceUpdatedCallback(const TSharedPtr<const FUniqueNetId>& UserId, const FOnlineUserPresence& Presence);
    void OnCreateSessionCompleteCallback(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsCompleteCallback(bool bWasSuccessful);
    void OnJoinSessionCompleteCallback(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionCompleteCallback(FName SessionName, bool bWasSuccessful);
    void OnStartSessionCompleteCallback(FName SessionName, bool bWasSuccessful);
    void OnEndSessionCompleteCallback(FName SessionName, bool bWasSuccessful);
    void OnLeaderboardReadCompleteCallback(bool bWasSuccessful);
    void OnLeaderboardWriteCompleteCallback(bool bWasSuccessful);
    void OnAchievementsWrittenCompleteCallback(const FUniqueNetId& UserId, bool bWasSuccessful);
    void OnQueryAchievementsCompleteCallback(const FUniqueNetId& UserId, bool bWasSuccessful);
    void OnStatsUpdateCompleteCallback(const FUniqueNetId& LocalUserId, bool bWasSuccessful);
    void OnQueryStatsCompleteCallback(const FUniqueNetId& LocalUserId, bool bWasSuccessful);
    void OnCloudWriteCompleteCallback(const FUniqueNetId& UserId, const FString& FileName, bool bWasSuccessful);
    void OnCloudReadCompleteCallback(const FUniqueNetId& UserId, const FString& FileName, const TArray<uint8>& Data, bool bWasSuccessful);
    void OnCloudDeleteCompleteCallback(const FUniqueNetId& UserId, const FString& FileName, bool bWasSuccessful);
    void OnCloudListCompleteCallback(const FUniqueNetId& UserId, const TArray<FCloudFileHeader>& Files, bool bWasSuccessful);
};