# HeroHour EOS Initialization & Setup Guide

This document describes the complete EOS initialization flow for HeroHour, from engine startup to ready-to-use online services.

## Initialization Sequence

```
GameInstance.Init()
    │
    ▼
GetSubsystem<UHeroHourEOSSubsystem>()
    │
    ▼
UHeroHourEOSSubsystem.Initialize(FSubsystemCollectionBase&)
    │
    ├── IOnlineSubsystem::Get("EOS")
    │
    ├── StaticCast<IOnlineSubsystemEOS>()
    │
    ├── Cache Interfaces:
    │   ├── IOnlineIdentity
    │   ├── IOnlineFriends
    │   ├── IOnlinePresence
    │   ├── IOnlineSession
    │   ├── IOnlineLeaderboards
    │   ├── IOnlineAchievements
    │   ├── IOnlineStats
    │   └── IOnlineUserCloud
    │
    ├── Bind Delegates (Login, Session, Friends, etc.)
    │
    ├── LoadConfiguration()  ← Reads DefaultEngine.ini + Environment Variables
    │
    └── bIsInitialized = true
    │
    ▼
GameInstance.Init() continues...
    │
    ▼
UHeroHourGameInstance::InitializeEOS()
    │
    ├── Verify EOS subsystem exists
    │
    └── InitializeEOSPlatform()  ← Calls EOS_Platform_Create internally
    │
    ▼
READY FOR USE
```

## Required Configuration

### 1. DefaultEngine.ini (Committed)

```ini
[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemEOS.EOSENetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[OnlineSubsystem]
DefaultPlatformService=EOS
bHasVoiceEnabled=false

[OnlineSubsystemEOS]
bEnabled=true
ProductId=9069f4f0eda147c9a341ea081b5e9803
SandboxId=hero-hour-dev
DeploymentId=hero-hour-dev
ClientId=xyza7891HEROGameClient
bUseEAS=false
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
bUseEOSTitleStorage=false
bUseEOSSanctions=false
bUseEOSAntiCheat=false
bEnableOverlay=true
bEnableSocialOverlay=true
CacheDir=EOSCache
TickBudget=0
LogLevel=Log
bEnableLogging=true
```

### 2. DefaultEngine.Local.ini (NOT Committed)

```ini
[OnlineSubsystemEOS]
; Set via environment variables:
; EOS_CLIENT_SECRET=xxx
; EOS_ENCRYPTION_KEY=xxx

; Or manually (for local dev only):
ClientSecret=your_client_secret
EncryptionKey=your_encryption_key

LogLevel=Verbose
bEnableLogging=true
```

### 3. Environment Variables (Required for CI/CD & Production)

```powershell
# Windows
$env:EOS_CLIENT_SECRET = "your_client_secret_from_epic_portal"
$env:EOS_ENCRYPTION_KEY = "your_encryption_key_from_epic_portal"

# Linux/macOS
export EOS_CLIENT_SECRET="your_client_secret_from_epic_portal"
export EOS_ENCRYPTION_KEY="your_encryption_key_from_epic_portal"
```

## Code Entry Points

### C++ Module Startup

```cpp
// Source/HeroHour/Private/HeroHourModule.cpp
void FHeroHourModule::StartupModule()
{
    // Ensure OnlineSubsystemEOS is loaded
    if (!FModuleManager::Get().IsModuleLoaded("OnlineSubsystemEOS"))
    {
        FModuleManager::Get().LoadModule("OnlineSubsystemEOS");
    }
}
```

### GameInstance Initialization

```cpp
// Source/HeroHour/Private/HeroHourGameInstance.cpp
void UHeroHourGameInstance::Init()
{
    Super::Init();
    
    // Get EOS subsystem (auto-created by GameInstanceSubsystem system)
    EOSSubsystem = GetSubsystem<UHeroHourEOSSubsystem>();
    
    if (EOSSubsystem)
    {
        // Initialize EOS platform
        bool bSuccess = EOSSubsystem->InitializeEOSPlatform();
        
        if (!bSuccess)
        {
            UE_LOG(LogHeroHourEOS, Error, TEXT("Failed to initialize EOS! Online features disabled."));
        }
    }
}
```

### Subsystem Initialization

```cpp
// Source/HeroHour/Private/Online/HeroHourEOSSubsystem.cpp
void UHeroHourEOSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // 1. Get Online Subsystem
    OnlineSubsystem = IOnlineSubsystem::Get(TEXT("EOS"));
    
    // 2. Cast to EOS-specific subsystem
    EOSSubsystem = StaticCast<IOnlineSubsystemEOS*>(OnlineSubsystem.Get());
    
    // 3. Cache all interfaces
    CacheInterfaces();
    
    // 4. Bind all delegates
    BindDelegates();
    
    // 5. Load configuration
    LoadConfiguration();
    
    bIsInitialized = true;
}
```

## Verification Checklist

### At Editor Startup (PIE)

- [ ] `LogHeroHourEOS: HeroHour Module Starting Up...`
- [ ] `LogHeroHourEOS: OnlineSubsystemEOS module is loaded`
- [ ] `LogHeroHourEOS: EOS Config Loaded - ProductId: 9069f4f0eda147c9a341ea081b5e9803`
- [ ] `LogHeroHourEOS: HeroHour EOS Subsystem initialized successfully`
- [ ] `LogHeroHourEOS: HeroHour GameInstance Initialized`
- [ ] `LogHeroHourEOS: EOS Initialized Successfully`

### At Runtime (Blueprint)

- [ ] `Get Game Instance → Cast to HeroHourGameInstance → Get EOS Subsystem → Is EOS Initialized` returns `true`
- [ ] `LoginWithDeviceId(0)` → `OnLoginComplete` fires with `bWasSuccessful = true`
- [ ] `SetPresence(Online, "In Main Menu")` works
- [ ] `QueryFriends(0)` populates friends list
- [ ] `CreateSession(...)` creates lobby
- [ ] `FindSessions(...)` returns results

## Common Initialization Failures

| Error | Cause | Fix |
|-------|-------|-----|
| `OnlineSubsystemEOS not found` | Plugin not enabled | Check `.uproject` has OnlineSubsystemEOS plugin enabled |
| `Failed to get IOnlineSubsystemEOS` | Module load order | Ensure `OnlineSubsystemEOS` loads before `HeroHour` |
| `ClientSecret not set` | Env vars missing | Set `EOS_CLIENT_SECRET` environment variable |
| `Login failed: Invalid credentials` | Wrong sandbox/deployment | Verify SandboxId/DeploymentId match Epic Portal |
| `Session creation failed` | Lobby service disabled | Enable `bUseEOSLobbies=true` in config |

## Shutdown Sequence

```
GameInstance.Shutdown()
    │
    ▼
UHeroHourEOSSubsystem.Deinitialize()
    │
    ├── If logged in: Logout()
    │
    ├── Unbind all delegates
    │
    ├── Clear all interface references
    │
    ├── ShutdownEOSPlatform()  ← Calls EOS_Platform_Release
    │
    └── bIsInitialized = false
    │
    ▼
Super::Shutdown()
```

## Testing EOS Initialization

### Automated Test (C++)

```cpp
// Tests/HeroHourEOS/InitializationTest.cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeroHourEOSInitializationTest, "HeroHour.EOS.Initialization", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FHeroHourEOSInitializationTest::RunTest(const FString& Parameters)
{
    // Create GameInstance
    UHeroHourGameInstance* GameInstance = NewObject<UHeroHourGameInstance>();
    GameInstance->Init();
    
    // Verify EOS subsystem exists
    UHeroHourEOSSubsystem* Subsystem = GameInstance->GetEOSSubsystem();
    TestNotNull("EOS Subsystem exists", Subsystem);
    
    // Verify initialized
    TestTrue("EOS Initialized", Subsystem && Subsystem->IsEOSInitialized());
    
    // Cleanup
    GameInstance->Shutdown();
    
    return true;
}
```

### Manual Test (Blueprint)

1. Open any level
2. Play In Editor (PIE)
3. In Level Blueprint or Widget:
   - Get Game Instance → Cast to HeroHourGameInstance
   - Call `IsEOSReady()` → Print String (should be `true`)
   - Call `GetEOSSubsystem()` → Call `LoginWithDeviceId(0)`
   - Bind to `OnLoginComplete` → Print `bWasSuccessful`

---

*Version: 1.0*
*Last Updated: 2026-07-19*
*Project: HERO HOUR*