// HeroHourEOSSubsystem.cpp
// Epic Online Services (EOS) Integration Implementation for HeroHour

#include "Online/HeroHourEOSSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopeLock.h"

UHeroHourEOSSubsystem::UHeroHourEOSSubsystem()
    : bIsInitialized(false)
    , bIsLoggedIn(false)
    , CurrentSessionName(NAME_None)
    , LoginRetryCount(0)
    , MaxLoginRetries(3)
{
}

void UHeroHourEOSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogHeroHourEOS, Log, TEXT("Initializing HeroHour EOS Subsystem"));

    // Get the online subsystem
    OnlineSubsystem = IOnlineSubsystem::Get(TEXT("EOS"));
    if (!OnlineSubsystem)
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Failed to get EOS Online Subsystem!"));
        return;
    }

    // Get EOS-specific subsystem
    EOSSubsystem = StaticCast<IOnlineSubsystemEOS*>(OnlineSubsystem.Get());
    if (!EOSSubsystem)
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Failed to cast to IOnlineSubsystemEOS!"));
        return;
    }

    // Get interfaces
    IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    FriendsInterface = OnlineSubsystem->GetFriendsInterface();
    PresenceInterface = OnlineSubsystem->GetPresenceInterface();
    SessionInterface = OnlineSubsystem->GetSessionInterface();
    LeaderboardsInterface = OnlineSubsystem->GetLeaderboardsInterface();
    AchievementsInterface = OnlineSubsystem->GetAchievementsInterface();
    StatsInterface = OnlineSubsystem->GetStatsInterface();
    UserCloudInterface = OnlineSubsystem->GetUserCloudInterface();

    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Identity interface not available"));
    }

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Session interface not available"));
    }

    // Bind delegates
    BindDelegates();

    // Load configuration
    LoadConfiguration();

    bIsInitialized = true;
    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour EOS Subsystem initialized successfully"));
}

void UHeroHourEOSSubsystem::Deinitialize()
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Deinitializing HeroHour EOS Subsystem"));

    // Logout if logged in
    if (bIsLoggedIn)
    {
        Logout();
    }

    // Unbind delegates
    UnbindDelegates();

    // Clear interfaces
    IdentityInterface.Reset();
    FriendsInterface.Reset();
    PresenceInterface.Reset();
    SessionInterface.Reset();
    LeaderboardsInterface.Reset();
    AchievementsInterface.Reset();
    StatsInterface.Reset();
    UserCloudInterface.Reset();

    OnlineSubsystem.Reset();
    EOSSubsystem.Reset();

    bIsInitialized = false;

    Super::Deinitialize();
}

void UHeroHourEOSSubsystem::BindDelegates()
{
    if (IdentityInterface.IsValid())
    {
        OnLoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
            FOnLoginCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnLoginComplete));

        OnLogoutCompleteDelegateHandle = IdentityInterface->AddOnLogoutCompleteDelegate_Handle(
            FOnLogoutCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnLogoutComplete));
    }

    if (SessionInterface.IsValid())
    {
        OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
            FOnCreateSessionCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnCreateSessionComplete));

        OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
            FOnFindSessionsCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnFindSessionsComplete));

        OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnJoinSessionComplete));

        OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnDestroySessionComplete));

        OnStartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
            FOnStartSessionCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnStartSessionComplete));

        OnEndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(
            FOnEndSessionCompleteDelegate::CreateUObject(this, &UHeroHourEOSSubsystem::OnEndSessionComplete));
    }

    if (UserCloudInterface.IsValid())
    {
        // Cloud storage delegates
    }
}

void UHeroHourEOSSubsystem::UnbindDelegates()
{
    if (IdentityInterface.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(OnLoginCompleteDelegateHandle);
        IdentityInterface->ClearOnLogoutCompleteDelegate_Handle(OnLogoutCompleteDelegateHandle);
    }

    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
        SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
    }
}

void UHeroHourEOSSubsystem::LoadConfiguration()
{
    GConfig->GetString(TEXT("OnlineSubsystemEOS"), TEXT("ProductId"), ProductId, GEngineIni);
    GConfig->GetString(TEXT("OnlineSubsystemEOS"), TEXT("SandboxId"), SandboxId, GEngineIni);
    GConfig->GetString(TEXT("OnlineSubsystemEOS"), TEXT("DeploymentId"), DeploymentId, GEngineIni);
    GConfig->GetString(TEXT("OnlineSubsystemEOS"), TEXT("ClientId"), ClientId, GEngineIni);

    // Client secret and encryption key should be set via environment variables
    ClientSecret = FPlatformProcess::GetEnv(TEXT("EOS_CLIENT_SECRET"));
    EncryptionKey = FPlatformProcess::GetEnv(TEXT("EOS_ENCRYPTION_KEY"));

    if (ClientSecret.IsEmpty())
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("EOS_CLIENT_SECRET environment variable not set!"));
    }
    if (EncryptionKey.IsEmpty())
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("EOS_ENCRYPTION_KEY environment variable not set!"));
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("EOS Config Loaded - ProductId: %s, SandboxId: %s, DeploymentId: %s"),
        *ProductId, *SandboxId, *DeploymentId);
}

bool UHeroHourEOSSubsystem::Login(FString PlayerName, FString Password, FString AuthType)
{
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Identity interface not available for login"));
        return false;
    }

    if (bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Already logged in, logging out first"));
        Logout();
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Attempting login for player: %s"), *PlayerName);

    FOnlineAccountCredentials Credentials;
    Credentials.Id = PlayerName;
    Credentials.Token = Password;
    Credentials.Type = AuthType.IsEmpty() ? TEXT("accountportal") : AuthType;

    // For EOS, we typically use "accountportal" or "external" auth types
    // With EOS Connect, you can use "epicgames", "steam", etc.

    IdentityInterface->Login(0, Credentials);
    return true;
}

bool UHeroHourEOSSubsystem::LoginWithEOSConnect(const FString& IdToken, const FString& AuthType)
{
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Identity interface not available for EOS Connect login"));
        return false;
    }

    if (bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Already logged in, logging out first"));
        Logout();
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Attempting EOS Connect login with auth type: %s"), *AuthType);

    FOnlineAccountCredentials Credentials;
    Credentials.Token = IdToken;
    Credentials.Type = AuthType.IsEmpty() ? TEXT("epicgames") : AuthType;

    IdentityInterface->Login(0, Credentials);
    return true;
}

bool UHeroHourEOSSubsystem::LoginWithDeviceId()
{
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Identity interface not available for device login"));
        return false;
    }

    if (bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Already logged in, logging out first"));
        Logout();
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Attempting device ID login"));

    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("deviceid");
    // Device ID is typically generated automatically by EOS SDK

    IdentityInterface->Login(0, Credentials);
    return true;
}

void UHeroHourEOSSubsystem::Logout()
{
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Identity interface not available for logout"));
        return;
    }

    if (!bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Not logged in, ignoring logout request"));
        return;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Logging out user: %s"), *LoggedInUserId.ToString());

    IdentityInterface->Logout(0);
}

TSharedPtr<const FUniqueNetId> UHeroHourEOSSubsystem::GetLoggedInUserId() const
{
    if (IdentityInterface.IsValid() && bIsLoggedIn)
    {
        return IdentityInterface->GetUniquePlayerId(0);
    }
    return nullptr;
}

FString UHeroHourEOSSubsystem::GetLoggedInUserIdString() const
{
    auto UserId = GetLoggedInUserId();
    return UserId.IsValid() ? UserId->ToString() : TEXT("");
}

bool UHeroHourEOSSubsystem::IsLoggedIn() const
{
    return bIsLoggedIn && IdentityInterface.IsValid() &&
           IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

void UHeroHourEOSSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    bIsLoggedIn = bWasSuccessful;
    LoginRetryCount = 0;

    if (bWasSuccessful)
    {
        LoggedInUserId = UserId.AsShared();
        UE_LOG(LogHeroHourEOS, Log, TEXT("Login successful for user: %s"), *UserId.ToString());

        // Set default presence
        SetPresence(EHeroHourPresenceState::Online, TEXT("In Main Menu"));

        // Query friends list
        QueryFriends();
    }
    else
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Login failed: %s"), *Error);
        LoggedInUserId.Reset();
    }

    OnLoginCompleteDelegate.Broadcast(bWasSuccessful, Error);
}

void UHeroHourEOSSubsystem::OnLogoutComplete(int32 LocalUserNum, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Logout %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));

    bIsLoggedIn = false;
    LoggedInUserId.Reset();
    CurrentSessionName = NAME_None;

    OnLogoutCompleteDelegate.Broadcast(bWasSuccessful);
}

void UHeroHourEOSSubsystem::SetPresence(EHeroHourPresenceState State, const FString& StatusStr)
{
    if (!PresenceInterface.IsValid() || !bIsLoggedIn)
    {
        return;
    }

    FOnlineUserPresence Presence;
    Presence.Status.State = static_cast<EOnlinePresenceState::Type>(State);
    Presence.Status.StatusStr = StatusStr;

    // Add game-specific data
    Presence.Properties.Add(TEXT("game"), TEXT("HeroHour"));
    Presence.Properties.Add(TEXT("version"), TEXT("1.0.0"));

    PresenceInterface->SetPresence(0, *GetLoggedInUserId(), Presence);
}

void UHeroHourEOSSubsystem::QueryFriends()
{
    if (!FriendsInterface.IsValid() || !bIsLoggedIn)
    {
        return;
    }

    FriendsInterface->ReadFriendsList(0, TEXT(""), FOnReadFriendsListComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnFriendsListReadComplete));
}

void UHeroHourEOSSubsystem::OnFriendsListReadComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Failed to read friends list: %s"), *ErrorStr);
        return;
    }

    TArray<TSharedRef<FOnlineFriend>> Friends;
    FriendsInterface->GetFriendsList(0, TEXT(""), Friends);

    UE_LOG(LogHeroHourEOS, Log, TEXT("Friends list loaded: %d friends"), Friends.Num());

    OnFriendsListUpdatedDelegate.Broadcast(Friends);
}

// Session Management
bool UHeroHourEOSSubsystem::CreateSession(const FHeroHourSessionSettings& Settings, FName SessionName)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available"));
        return false;
    }

    if (!bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Cannot create session: not logged in"));
        return false;
    }

    CurrentSessionName = SessionName;

    FOnlineSessionSettings SessionSettings;
    Settings.ToOnlineSessionSettings(SessionSettings);

    UE_LOG(LogHeroHourEOS, Log, TEXT("Creating session: %s"), *SessionName.ToString());

    return SessionInterface->CreateSession(0, SessionName, SessionSettings);
}

bool UHeroHourEOSSubsystem::FindSessions(const FHeroHourSessionSearchParams& SearchParams)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available"));
        return false;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = SearchParams.MaxResults;
    SessionSearch->bAllowEmptyServers = SearchParams.bAllowEmptyServers;
    SessionSearch->bAllowFullServers = SearchParams.bAllowFullServers;
    SessionSearch->bUseLobbiesIfAvailable = SearchParams.bUseLobbiesIfAvailable;
    SessionSearch->PingBucketSize = SearchParams.PingBucketSize;

    // Add search filters
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, SearchParams.bUseLobbiesIfAvailable, EOnlineComparisonOp::Equals);

    if (!SearchParams.MapFilter.IsEmpty())
    {
        SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, SearchParams.MapFilter, EOnlineComparisonOp::Equals);
    }

    if (!SearchParams.GameModeFilter.IsEmpty())
    {
        SessionSearch->QuerySettings.Set(TEXT("GameMode"), SearchParams.GameModeFilter, EOnlineComparisonOp::Equals);
    }

    // Skill-based filters would go here

    UE_LOG(LogHeroHourEOS, Log, TEXT("Searching for sessions..."));

    return SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

bool UHeroHourEOSSubsystem::JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available"));
        return false;
    }

    if (!bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Cannot join session: not logged in"));
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Joining session: %s"), *SessionName.ToString());

    return SessionInterface->JoinSession(0, SessionName, SearchResult);
}

bool UHeroHourEOSSubsystem::DestroySession(FName SessionName)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available"));
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Destroying session: %s"), *SessionName.ToString());

    return SessionInterface->DestroySession(SessionName);
}

bool UHeroHourEOSSubsystem::StartSession(FName SessionName)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available"));
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Starting session: %s"), *SessionName.ToString());

    return SessionInterface->StartSession(SessionName);
}

bool UHeroHourEOSSubsystem::EndSession(FName SessionName)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available"));
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Ending session: %s"), *SessionName.ToString());

    return SessionInterface->EndSession(SessionName);
}

FString UHeroHourEOSSubsystem::GetSessionConnectString(FName SessionName)
{
    if (!SessionInterface.IsValid())
    {
        return TEXT("");
    }

    FString ConnectString;
    if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        return ConnectString;
    }

    return TEXT("");
}

TArray<FOnlineSessionSearchResult> UHeroHourEOSSubsystem::GetSearchResults() const
{
    if (SessionSearch.IsValid())
    {
        return SessionSearch->SearchResults;
    }
    return TArray<FOnlineSessionSearchResult>();
}

void UHeroHourEOSSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Create session %s: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

    if (bWasSuccessful)
    {
        CurrentSessionName = SessionName;
    }

    OnCreateSessionCompleteDelegate.Broadcast(SessionName, bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Find sessions: %s (%d results)"),
        bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"),
        SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0);

    OnFindSessionsCompleteDelegate.Broadcast(bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    const bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);

    UE_LOG(LogHeroHourEOS, Log, TEXT("Join session %s: %s"), *SessionName.ToString(),
        bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

    if (bWasSuccessful)
    {
        CurrentSessionName = SessionName;

        // Get connect string for travel
        FString ConnectString = GetSessionConnectString(SessionName);
        if (!ConnectString.IsEmpty())
        {
            OnJoinSessionCompleteWithConnectStringDelegate.Broadcast(SessionName, true, ConnectString);
        }
    }

    OnJoinSessionCompleteDelegate.Broadcast(SessionName, bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Destroy session %s: %s"), *SessionName.ToString(),
        bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

    if (bWasSuccessful && CurrentSessionName == SessionName)
    {
        CurrentSessionName = NAME_None;
    }

    OnDestroySessionCompleteDelegate.Broadcast(SessionName, bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Start session %s: %s"), *SessionName.ToString(),
        bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

    OnStartSessionCompleteDelegate.Broadcast(SessionName, bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("End session %s: %s"), *SessionName.ToString(),
        bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

    OnEndSessionCompleteDelegate.Broadcast(SessionName, bWasSuccessful);
}

// Matchmaking
bool UHeroHourEOSSubsystem::StartMatchmaking(const FHeroHourMatchmakingParams& Params)
{
    // EOS Matchmaking uses the Sessions interface with specific bucket IDs
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Error, TEXT("Session interface not available for matchmaking"));
        return false;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = Params.MaxPlayers;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUseLobbiesIfAvailable = true;
    SessionSettings.bIsDedicated = false;
    SessionSettings.bIsLobby = true;

    // Set matchmaking bucket
    SessionSettings.Set(TEXT("BUCKET_ID"), Params.BucketId, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    SessionSettings.Set(TEXT("MIN_PLAYERS"), Params.MinPlayers, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings.Set(TEXT("MAX_PLAYERS"), Params.MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);

    // Skill-based settings
    if (Params.bUseSkillBasedMatchmaking)
    {
        SessionSettings.Set(TEXT("SKILL_RATING"), Params.SkillRating, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings.Set(TEXT("SKILL_DEVIATION"), Params.SkillDeviation, EOnlineDataAdvertisementType::ViaOnlineService);
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Starting matchmaking with bucket: %s"), *Params.BucketId);

    return SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

bool UHeroHourEOSSubsystem::CancelMatchmaking()
{
    if (!SessionInterface.IsValid())
    {
        return false;
    }

    return SessionInterface->DestroySession(NAME_GameSession);
}

// Stats & Leaderboards
bool UHeroHourEOSSubsystem::UpdateStats(const TArray<FHeroHourStatUpdate>& StatUpdates)
{
    if (!StatsInterface.IsValid() || !bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Stats interface not available or not logged in"));
        return false;
    }

    TArray<FOnlineStatUpdate> OnlineUpdates;
    for (const auto& Update : StatUpdates)
    {
        FOnlineStatUpdate OnlineUpdate;
        OnlineUpdate.StatName = *Update.Name;
        OnlineUpdate.Value = Update.Value;
        OnlineUpdate.UpdateMethod = Update.bIncrement ? EOnlineStatUpdateMethod::Add : EOnlineStatUpdateMethod::Set;
        OnlineUpdates.Add(OnlineUpdate);
    }

    return StatsInterface->UpdateStats(0, *GetLoggedInUserId(), OnlineUpdates, FOnStatsUpdateComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnStatsUpdateComplete));
}

bool UHeroHourEOSSubsystem::QueryStats(const TArray<FString>& StatNames)
{
    if (!StatsInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    TArray<FString> Names;
    for (const auto& Name : StatNames)
    {
        Names.Add(Name);
    }

    return StatsInterface->QueryStats(0, *GetLoggedInUserId(), Names, FOnQueryStatsComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnQueryStatsComplete));
}

void UHeroHourEOSSubsystem::OnStatsUpdateComplete(const FUniqueNetId& LocalUserId, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Stats update: %s"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnStatsUpdateCompleteDelegate.Broadcast(bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnQueryStatsComplete(const FUniqueNetId& LocalUserId, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Query stats: %s"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnQueryStatsCompleteDelegate.Broadcast(bWasSuccessful);
}

bool UHeroHourEOSSubsystem::ReadLeaderboard(const FString& LeaderboardName, int32 StartRank, int32 MaxResults)
{
    if (!LeaderboardsInterface.IsValid())
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Leaderboards interface not available"));
        return false;
    }

    FOnlineLeaderboardReadRef ReadObject = MakeShareable(new FOnlineLeaderboardRead());
    ReadObject->LeaderboardName = LeaderboardName;
    ReadObject->StartRank = StartRank;
    ReadObject->MaxResults = MaxResults;
    ReadObject->ReadObjectType = ELeaderboardReadType::Global;
    ReadObject->PlayerIds.Add(*GetLoggedInUserId());

    return LeaderboardsInterface->ReadLeaderboards({ReadObject}, FOnReadLeaderboardComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnLeaderboardReadComplete));
}

void UHeroHourEOSSubsystem::OnLeaderboardReadComplete(bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Leaderboard read: %s"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

    TArray<FHeroHourLeaderboardEntry> Entries;

    if (bWasSuccessful && LeaderboardsInterface.IsValid())
    {
        TArray<FOnlineLeaderboardReadRef> Reads;
        LeaderboardsInterface->GetCachedLeaderboards(Reads);

        for (const auto& Read : Reads)
        {
            if (Read->Rows.Num() > 0)
            {
                for (const auto& Row : Read->Rows)
                {
                    FHeroHourLeaderboardEntry Entry;
                    Entry.Rank = Row.Rank;
                    Entry.UserId = Row.PlayerId->ToString();
                    Entry.DisplayName = Row.DisplayName;
                    Entry.Score = Row.Score;
                    Entry.Metadata = Row.Metadata;
                    Entry.Timestamp = Row.TimeStamp;
                    Entries.Add(Entry);
                }
            }
        }
    }

    OnLeaderboardReadCompleteDelegate.Broadcast(Entries, bWasSuccessful);
}

bool UHeroHourEOSSubsystem::WriteLeaderboard(const FString& LeaderboardName, int64 Score, const FString& Metadata)
{
    if (!LeaderboardsInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    FOnlineLeaderboardWrite WriteObject;
    WriteObject.LeaderboardName = LeaderboardName;
    WriteObject.RatedStat = TEXT("Score");
    WriteObject.PlayerId = GetLoggedInUserId();
    WriteObject.Score = Score;
    WriteObject.Metadata = Metadata;

    return LeaderboardsInterface->WriteLeaderboards({WriteObject}, FOnWriteLeaderboardComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnLeaderboardWriteComplete));
}

void UHeroHourEOSSubsystem::OnLeaderboardWriteComplete(bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Leaderboard write: %s"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnLeaderboardWriteCompleteDelegate.Broadcast(bWasSuccessful);
}

// Achievements
bool UHeroHourEOSSubsystem::UnlockAchievement(const FString& AchievementId)
{
    if (!AchievementsInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    FOnlineAchievement Achievement;
    Achievement.Id = AchievementId;
    Achievement.bWasAchievedOnline = true;
    Achievement.UnlockTime = FDateTime::UtcNow();

    TArray<FOnlineAchievement> Achievements;
    Achievements.Add(Achievement);

    return AchievementsInterface->WriteAchievements(0, *GetLoggedInUserId(), Achievements, FOnAchievementsWrittenComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnAchievementsWrittenComplete));
}

bool UHeroHourEOSSubsystem::QueryAchievements()
{
    if (!AchievementsInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    return AchievementsInterface->QueryAchievements(0, *GetLoggedInUserId(), FOnQueryAchievementsComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnQueryAchievementsComplete));
}

void UHeroHourEOSSubsystem::OnAchievementsWrittenComplete(const FUniqueNetId& UserId, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Achievement unlock: %s"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnAchievementUnlockedDelegate.Broadcast(bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnQueryAchievementsComplete(const FUniqueNetId& UserId, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Achievement query: %s"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnQueryAchievementsCompleteDelegate.Broadcast(bWasSuccessful);
}

// Cloud Storage (Player Data)
bool UHeroHourEOSSubsystem::WriteCloudSave(const FString& FileName, const TArray<uint8>& Data, const FString& Metadata)
{
    if (!UserCloudInterface.IsValid() || !bIsLoggedIn)
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("Cloud storage not available or not logged in"));
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Writing cloud save: %s (%d bytes)"), *FileName, Data.Num());

    return UserCloudInterface->WriteFile(*GetLoggedInUserId(), FileName, Data, FOnUserCloudWriteFileComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnCloudWriteComplete));
}

bool UHeroHourEOSSubsystem::ReadCloudSave(const FString& FileName)
{
    if (!UserCloudInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Reading cloud save: %s"), *FileName);

    return UserCloudInterface->ReadFile(*GetLoggedInUserId(), FileName, FOnUserCloudReadFileComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnCloudReadComplete));
}

bool UHeroHourEOSSubsystem::DeleteCloudSave(const FString& FileName)
{
    if (!UserCloudInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    return UserCloudInterface->DeleteFile(*GetLoggedInUserId(), FileName, FOnUserCloudDeleteFileComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnCloudDeleteComplete));
}

bool UHeroHourEOSSubsystem::ListCloudSaves()
{
    if (!UserCloudInterface.IsValid() || !bIsLoggedIn)
    {
        return false;
    }

    return UserCloudInterface->ListFiles(*GetLoggedInUserId(), FOnUserCloudListFilesComplete::CreateUObject(this, &UHeroHourEOSSubsystem::OnCloudListComplete));
}

void UHeroHourEOSSubsystem::OnCloudWriteComplete(const FUniqueNetId& UserId, const FString& FileName, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Cloud write %s: %s"), *FileName, bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnCloudStorageWriteCompleteDelegate.Broadcast(FileName, bWasSuccessful, bWasSuccessful ? TEXT("") : TEXT("Write failed"));
}

void UHeroHourEOSSubsystem::OnCloudReadComplete(const FUniqueNetId& UserId, const FString& FileName, const TArray<uint8>& Data, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Cloud read %s: %s (%d bytes)"), *FileName, bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"), Data.Num());
    OnCloudStorageReadCompleteDelegate.Broadcast(FileName, Data, bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnCloudDeleteComplete(const FUniqueNetId& UserId, const FString& FileName, bool bWasSuccessful)
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("Cloud delete %s: %s"), *FileName, bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    OnCloudStorageDeleteCompleteDelegate.Broadcast(FileName, bWasSuccessful);
}

void UHeroHourEOSSubsystem::OnCloudListComplete(const FUniqueNetId& UserId, const TArray<FCloudFileHeader>& Files, bool bWasSuccessful)
{
    TArray<FHeroHourCloudSaveSlot> Slots;

    if (bWasSuccessful)
    {
        for (const auto& File : Files)
        {
            FHeroHourCloudSaveSlot Slot;
            Slot.FileName = File.Filename;
            Slot.LastModified = File.LastModified;
            Slot.FileSize = File.FileSize;
            Slots.Add(Slot);
        }
    }

    UE_LOG(LogHeroHourEOS, Log, TEXT("Cloud list: %s (%d files)"), bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"), Slots.Num());
    OnCloudStorageListCompleteDelegate.Broadcast(Slots, bWasSuccessful);
}

// Utility
bool UHeroHourEOSSubsystem::IsEOSAvailable() const
{
    return OnlineSubsystem.IsValid() && OnlineSubsystem->GetSubsystemName() == TEXT("EOS");
}

FString UHeroHourEOSSubsystem::GetEOSVersion() const
{
    if (EOSSubsystem)
    {
        // Get EOS SDK version
        return TEXT("1.16");
    }
    return TEXT("Unknown");
}

void UHeroHourEOSSubsystem::Tick(float DeltaTime)
{
    // Tick EOS SDK if needed
    if (EOSSubsystem)
    {
        // EOS tick is handled internally by the subsystem
    }
}