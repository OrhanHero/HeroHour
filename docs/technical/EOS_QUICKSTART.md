# HeroHour EOS Integration - Quick Start Guide

## For New Team Members

### 1. Prerequisites

- Unreal Engine 5.8 installed via Epic Games Launcher
- Visual Studio 2022 with Game Development with C++ workload
- .NET 8 SDK (for F# simulation layer)
- Git with LFS (`git lfs install`)

### 2. Initial Setup

```powershell
# 1. Clone repository
git clone https://github.com/HeroHour/HeroHour.git
cd HeroHour

# 2. Install Git LFS
git lfs install
git lfs pull

# 3. Set up EOS credentials (required for online features)
# Copy template and fill in your values
Copy-Item Source/HeroHour/Config/DefaultEngine.Local.ini.template Source/HeroHour/Config/DefaultEngine.Local.ini

# Edit DefaultEngine.Local.ini and add your credentials:
# ClientSecret=your_client_secret
# EncryptionKey=your_encryption_key

# OR set environment variables (recommended):
$env:EOS_CLIENT_SECRET = "your_client_secret_from_epic_portal"
$env:EOS_ENCRYPTION_KEY = "your_encryption_key_from_epic_portal"

# 4. Run setup script
./Scripts/ci/setup-dev-environment.ps1

# 5. Open in VS Code
code .
```

### 3. Get EOS Credentials

1. Ask Tech Lead for access to **HERO** organization on Epic Developer Portal
2. Go to: https://dev.epicgames.com/portal/
3. Select Organization: **HERO** (`hero-c31fffa8`)
4. Select Product: **HERO HOUR** (`9069f4f0eda147c9a341ea081b5e9803`)
5. Navigate to **Product Settings → Client Credentials**
6. Copy **Client Secret** and **Encryption Key**
7. Set as environment variables (NEVER commit to git!)

### 4. Verify Setup

```powershell
# Build project
./Scripts/build/Build.ps1 -Configuration Development

# Run tests
dotnet test Source/HeroHourSimulation/tests/HeroHourSimulation.Tests.fsproj

# Launch editor
./Scripts/tools/LaunchEditor.ps1
```

### 5. Test EOS in Editor

1. Open HeroHour.uproject in UE5 Editor
2. Play In Editor (PIE) with 2+ players
3. Open **Multiplayer Menu** (created by UI team)
4. Test:
   - Login (Device ID or Developer Auth)
   - Create Session
   - Find Sessions
   - Join Session
   - Cloud Save/Load

## Common Commands

```powershell
# Full rebuild
./Scripts/build/Build.ps1 -Clean -Configuration Development

# Build only F# simulation
./Scripts/build/BuildFable.ps1 -Configuration Release

# Run F# tests
dotnet test Source/HeroHourSimulation/tests/HeroHourSimulation.Tests.fsproj --logger "console;verbosity=detailed"

# Package for shipping
./Scripts/build/Build.ps1 -Configuration Shipping -Package

# Run dedicated server
./Scripts/tools/RunServer.ps1 -Map "Map_Desert" -Players 4
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "OnlineSubsystemEOS not found" | Check `.uproject` has OnlineSubsystemEOS plugin enabled |
| Login fails | Verify EOS_CLIENT_SECRET and EOS_ENCRYPTION_KEY env vars |
| Can't find sessions | Verify SandboxId/DeploymentId match Epic Portal |
| Cloud save fails | Enable Player Data Storage in Epic Portal product settings |

## Key Files to Know

| File | Purpose |
|------|---------|
| `Source/HeroHour/Public/Online/HeroHourEOSSubsystem.h` | Main EOS API (Blueprint) |
| `Source/HeroHour/Config/DefaultEngine.ini` | EOS config (committed) |
| `Source/HeroHour/Config/DefaultEngine.Local.ini` | Your local secrets (gitignored) |
| `docs/technical/EOS_INTEGRATION.md` | Full documentation |

## Need Help?

- **Discord:** #heroHour-dev (ask for EOS help)
- **Docs:** `docs/technical/EOS_INTEGRATION.md`
- **Epic Portal:** https://dev.epicgames.com/portal/org/hero-c31fffa8

---

*Welcome to the team! 🎮*