# HeroHour - Epic Online Services (EOS) Integration Guide

## Overview

This document describes the Epic Online Services (EOS) integration for **HERO HOUR**, registered under the **HERO** organization.

### Organization ID
**Organization ID:** `hero-c31fffa8`
**Product ID:** `9069f4f0eda147c9a341ea081b5e9803`
**Product Name:** HERO HOUR

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        HeroHour GameInstance                     │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │              UHeroHourEOSSubsystem (GameInstanceSubsystem) │  │
│  │  ┌──────────┐ ┌────────┐ ┌──────────┐ ┌────────────────┐  │  │
│  │  │  Auth    │ │ Friends│ │ Presence │ │   Sessions     │  │  │
│  │  │          │ │        │ │          │ │  & Lobbies     │  │  │
│  │  └──────────┘ └────────┘ └──────────┘ └────────────────┘  │  │
│  │  ┌──────────┐ ┌────────┐ ┌──────────┐ ┌────────────────┐  │  │
│  │  │Matchmaking│ │Leader- │ │Achieve-  │ │  Cloud Storage │  │  │
│  │  │           │ │boards  │ │ments     │ │  (Player Data) │  │  │
│  │  └──────────┘ └────────┘ └──────────┘ └────────────────┘  │  │
│  │  ┌──────────────────────────────────────────────────────┐  │  │
│  │  │              IOnlineSubsystemEOS (UE5 Plugin)         │  │  │
│  │  └──────────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Epic Online Services SDK                   │
│  ┌──────────┐ ┌────────┐ ┌──────────┐ ┌────────────────────┐   │
│  │  Auth    │ │ Connect│ │  Lobby   │ │  Player Data       │   │
│  │  Service │ │        │ │  Service │ │  Storage           │   │
│  └──────────┘ └────────┘ └──────────┘ └────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## Configuration

### 1. DefaultEngine.ini (Committed to Git)

```ini
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true
ProductId=9069f4f0eda147c9a341ea081b5e9803
SandboxId=hero-hour-dev
DeploymentId=hero-hour-dev
ClientId=xyza7891HEROGameClient
bUseEOSConnect=true
bUseEOSFriends=true
bUseEOSPresence=true
bUseEOSSessions=true
bUseEOSLobbies=true
bUseEOSMatchmaking=true
bUseEOSLeaderboards=true
bUseEOSAchievements=true
bUseEOSStats=true
bUseEOSPlayerDataStorage=true
```

### 2. DefaultEngine.Local.ini (NEVER COMMIT - Template Provided)

```ini
[OnlineSubsystemEOS]
; SET THESE VIA ENVIRONMENT VARIABLES:
; EOS_CLIENT_SECRET=your_client_secret
; EOS_ENCRYPTION_KEY=your_encryption_key

; Or uncomment and set locally (NOT for commit):
; ClientSecret=your_client_secret_here
; EncryptionKey=your_encryption_key_here

LogLevel=Verbose
bEnableLogging=true
```

### 3. Environment Variables (Required for Production)

Set these in your CI/CD pipeline and local development environment:

```bash
# Windows PowerShell
$env:EOS_CLIENT_SECRET = "your_client_secret_from_epic_portal"
$env:EOS_ENCRYPTION_KEY = "your_encryption_key_from_epic_portal"

# Linux/macOS
export EOS_CLIENT_SECRET="your_client_secret_from_epic_portal"
export EOS_ENCRYPTION_KEY="your_encryption_key_from_epic_portal"
```

**To get credentials:**
1. Go to [Epic Developer Portal](https://dev.epicgames.com/portal/)
2. Select Organization: **HERO** (`hero-c31fffa8`)
3. Select Product: **HERO HOUR** (`9069f4f0eda147c9a341ea081b5e9803`)
4. Navigate to **Product Settings → Client Credentials**
5. Copy **Client Secret** and **Encryption Key**

## Module Structure

```
Source/HeroHour/
├── Public/
│   ├── HeroHour.h                    # Module interface
│   ├── HeroHourGameInstance.h        # GameInstance with EOS
│   └── Online/
│       ├── HeroHourEOSSubsystem.h    # Main EOS subsystem (Blueprintable)
│       └── HeroHourEOSTypes.h        # Data structures
├── Private/
│   ├── HeroHourModule.cpp            # Module startup/shutdown
│   ├── HeroHourGameInstance.cpp      # GameInstance implementation
│   └── Online/
│       └── HeroHourEOSSubsystem.cpp  # EOS implementation
└── Config/
    ├── DefaultEngine.ini             # EOS config (committed)
    └── DefaultEngine.Local.ini.template  # Template for secrets
```

## Key Classes

### UHeroHourEOSSubsystem (GameInstanceSubsystem)

Main entry point for all EOS functionality. Automatically initialized with GameInstance.

**Blueprint Accessible Functions:**

```cpp
// Initialization
InitializeEOSPlatform() → bool
ShutdownEOSPlatform()
IsEOSInitialized() → bool

// Authentication
LoginWithEOSAuth(LocalUserNum, Id, Token, Type)
LoginWithEOSConnect(LocalUserNum, Id, Token, Type)  // Epic, Steam, etc.
LoginWithDeviceId(LocalUserNum)                     // Anonymous
Logout(LocalUserNum)
GetLoginStatus(LocalUserNum) → ELoginStatus
GetUserId(LocalUserNum) → UniqueNetId
GetUserDisplayName(LocalUserNum) → String
IsUserLoggedIn(LocalUserNum) → bool

// Friends
QueryFriends(LocalUserNum)
GetFriendsList(LocalUserNum, OutFriends)
GetFriendsCount(LocalUserNum) → int32
SendFriendInvite(LocalUserNum, TargetUserId)
AcceptFriendInvite(LocalUserNum, TargetUserId)
RemoveFriend(LocalUserNum, TargetUserId)

// Presence
SetPresence(LocalUserNum, EHeroHourPresenceState, StatusStr)
GetPresence(LocalUserNum, UserId, OutPresence)
QueryPresence(LocalUserNum, UserIds)

// Sessions & Lobbies
CreateSession(LocalUserNum, SessionName, FHeroHourSessionSettings)
JoinSession(LocalUserNum, SessionName, SearchResult)
FindSessions(LocalUserNum, FHeroHourSessionSearchParams)
DestroySession(SessionName)
StartSession(SessionName)
EndSession(SessionName)
GetSessionConnectString(SessionName) → String

// Matchmaking
StartMatchmaking(LocalUserNum, FHeroHourMatchmakingParams)
CancelMatchmaking(LocalUserNum)

// Leaderboards
WriteLeaderboardScore(LocalUserNum, LeaderboardName, Score, Metadata)
ReadLeaderboardScores(LocalUserNum, LeaderboardName, StartRank, MaxResults)
ReadLeaderboardAroundUser(LocalUserNum, LeaderboardName, Range)

// Achievements
UnlockAchievement(LocalUserNum, AchievementId)
QueryAchievements(LocalUserNum)
GetAchievementProgress(LocalUserNum, AchievementId, OutProgress, OutUnlocked)

// Stats
UpdateStats(LocalUserNum, StatUpdates[])
QueryStats(LocalUserNum, StatNames[])

// Cloud Storage (Player Data)
WritePlayerData(LocalUserNum, FileName, Data[])
ReadPlayerData(LocalUserNum, FileName)
DeletePlayerData(LocalUserNum, FileName)
ListPlayerDataFiles(LocalUserNum)
```

**Blueprint Events (Assignable):**

```cpp
OnLoginComplete(bWasSuccessful, Error)
OnLogoutComplete(bWasSuccessful)
OnAuthLoginChanged(UserId, LoginStatus)
OnFriendsListUpdated(Friends[])
OnPresenceUpdated(UserId, Presence)
OnSessionCreateComplete(SessionName, bWasSuccessful)
OnSessionJoinComplete(SessionName, bWasSuccessful)
OnSessionJoinCompleteWithConnectString(SessionName, bWasSuccessful, ConnectString)
OnSessionDestroyComplete(SessionName, bWasSuccessful)
OnSessionFindComplete(bWasSuccessful)
OnSessionStartComplete(SessionName, bWasSuccessful)
OnSessionEndComplete(SessionName, bWasSuccessful)
OnMatchmakingComplete(bWasSuccessful, TicketId, Result)
OnLeaderboardReadComplete(Entries[], bWasSuccessful)
OnLeaderboardWriteComplete(bWasSuccessful)
OnAchievementUnlocked(bWasSuccessful)
OnQueryAchievementsComplete(bWasSuccessful)
OnStatsUpdateComplete(bWasSuccessful)
OnQueryStatsComplete(bWasSuccessful)
OnCloudStorageWriteComplete(FileName, bWasSuccessful, Error)
OnCloudStorageReadComplete(FileName, Data[], bWasSuccessful)
OnCloudStorageDeleteComplete(FileName, bWasSuccessful)
OnCloudStorageListComplete(Files[], bWasSuccessful)
```

## Usage Examples

### 1. Basic Login Flow (Blueprint)

```
Event BeginPlay
    → Get Game Instance → Cast to HeroHourGameInstance
    → Get EOS Subsystem
    → Is EOS Ready?
        ├─ Yes → LoginWithDeviceId(0)
        └─ No  → Show Error "Online services unavailable"

Event OnLoginComplete (from EOS Subsystem)
    → Branch bWasSuccessful
        ├─ True → Set Presence(Online, "In Main Menu")
        │       → Query Friends
        │       → Enable Multiplayer Menu
        └─ False → Show Error "Login Failed: " + Error
```

### 2. Host Multiplayer Game

```
Player Clicks "Host Game"
    → Get EOS Subsystem
    → Create FHeroHourSessionSettings
        MapName = "Map_Desert"
        MatchDuration = 3600
        PopulationCap = 200
        bFogOfWarEnabled = true
    → CreateSession(0, "GameSession", Settings)

Event OnSessionCreateComplete
    → Branch bWasSuccessful
        ├─ True → GetSessionConnectString("GameSession")
        │       → ServerTravel(ConnectString)
        └─ False → Show Error "Failed to create session"
```

### 3. Join Multiplayer Game

```
Player Clicks "Find Games"
    → Get EOS Subsystem
    → Create FHeroHourSessionSearchParams
        MaxResults = 20
        GameModeFilter = "Standard"
    → FindSessions(0, SearchParams)

Event OnSessionFindComplete
    → Get Search Results
    → Populate Session List Widget

Player Selects Session → Clicks "Join"
    → Get EOS Subsystem
    → JoinSession(0, "GameSession", SelectedSearchResult)

Event OnSessionJoinCompleteWithConnectString
    → Branch bWasSuccessful
        ├─ True → ClientTravel(ConnectString)
        └─ False → Show Error "Failed to join session"
```

### 4. Cloud Save (Player Data)

```
Player Completes Mission
    → Serialize Save Data to JSON/Bytes
    → Get EOS Subsystem
    → WritePlayerData(0, "save_mission_01.sav", SaveData)

Event OnCloudStorageWriteComplete
    → Branch bWasSuccessful
        ├─ True → Show "Game Saved to Cloud"
        └─ False → Show Error "Cloud Save Failed"

Player Loads Game
    → Get EOS Subsystem
    → ReadPlayerData(0, "save_mission_01.sav")

Event OnCloudStorageReadComplete
    → Branch bWasSuccessful
        ├─ True → Deserialize Data → Load Game State
        └─ False → Show Error / Use Local Save
```

### 5. Leaderboards & Achievements

```
Player Wins Match
    → Get EOS Subsystem
    → UpdateStats(0, [
        {Name: "Wins", Value: 1, bIncrement: true},
        {Name: "TotalPlayTime", Value: MatchDuration, bIncrement: true}
    ])
    → WriteLeaderboardScore(0, "GlobalWins", NewWinCount, "{}")

Event OnStatsUpdateComplete / OnLeaderboardWriteComplete
    → Check for Achievement Unlocks
    → If Wins >= 10: UnlockAchievement(0, "ACH_WIN_10")
    → If Wins >= 100: UnlockAchievement(0, "ACH_WIN_100")
```

## Testing

### Local Development

1. **Set Environment Variables:**
   ```powershell
   $env:EOS_CLIENT_SECRET = "your_dev_client_secret"
   $env:EOS_ENCRYPTION_KEY = "your_dev_encryption_key"
   ```

2. **Run in Editor (PIE):**
   - Launch 2+ PIE instances
   - Test login, session creation, joining

3. **EOS Developer Auth Tool:**
   - Use for testing without real accounts
   - `LoginWithDeveloperAuth(0, "TestPlayer1")`

### Dedicated Server Testing

```bash
# Build server
./Scripts/build/BuildUE5.ps1 -Target HeroHourServer -Configuration Shipping

# Run server
HeroHourServer.exe -log -Map=Map_Desert -Port=7777
```

### CI/CD Pipeline

```yaml
# .github/workflows/ci.yml
env:
  EOS_CLIENT_SECRET: ${{ secrets.EOS_CLIENT_SECRET }}
  EOS_ENCRYPTION_KEY: ${{ secrets.EOS_ENCRYPTION_KEY }}

steps:
  - name: Build HeroHour
    run: ./Scripts/build/Build.ps1 -Configuration Shipping
```

## Deployment Checklist

### Pre-Launch
- [ ] Production Sandbox ID configured in Epic Portal
- [ ] Production Deployment ID configured in Epic Portal
- [ ] Client Secret & Encryption Key stored in secure vault
- [ ] Environment variables set in all deployment environments
- [ ] Achievement IDs match Epic Portal configuration
- [ ] Leaderboard names match Epic Portal configuration
- [ ] Statistics names match Epic Portal configuration
- [ ] Cloud storage quotas configured
- [ ] Cross-play tested (Steam, Epic, etc. if applicable)

### Launch
- [ ] Switch to Production Sandbox/Deployment
- [ ] Monitor EOS Dashboard for errors
- [ ] Verify matchmaking queues populate
- [ ] Verify leaderboards update
- [ ] Verify cloud saves sync

## Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| `OnlineSubsystemEOS not found` | Ensure plugin enabled in `.uproject` |
| `Login fails: Invalid credentials` | Check Client Secret & Encryption Key env vars |
| `Session not found` | Verify SandboxId/DeploymentId match portal |
| `Matchmaking never finds match` | Check BucketId configuration in portal |
| `Cloud save fails` | Verify Player Data Storage enabled in portal |
| `Friends list empty` | Check EOS Friends service enabled |

### Debug Logging

Enable verbose logging in `DefaultEngine.Local.ini`:

```ini
[OnlineSubsystemEOS]
LogLevel=Verbose
bEnableLogging=true
```

Or via console:
```
Log OnlineSubsystemEOS Verbose
Log Online Verbose
```

## Version Compatibility

| UE Version | EOS SDK Version | Plugin Version |
|------------|-----------------|----------------|
| 5.8        | 1.16+           | 5.8            |

## References

- [EOS Documentation](https://dev.epicgames.com/docs/epic-online-services/)
- [EOS SDK Reference](https://dev.epicgames.com/docs/epic-online-services/eos-sdk-reference)
- [UE5 OnlineSubsystemEOS Plugin](https://dev.epicgames.com/docs/unreal-engine/online-subsystem-epic-online-services-in-unreal-engine)
- [Player Data Storage](https://dev.epicgames.com/docs/epic-online-services/player-data-storage)
- [HeroHour Epic Portal](https://dev.epicgames.com/portal/org/hero-c31fffa8/product/9069f4f0eda147c9a341ea081b5e9803)

---

*Document Version: 1.0*
*Last Updated: 2026-07-19*
*Project: HERO HOUR*