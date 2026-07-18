# HeroHour - AAA RTS Game

> **Command & Conquer: Generals Zero Hour** inspired Real-Time Strategy game built with **Unreal Engine 5.8**, **F# (Fable 5)**, and **.NET 8** — **Pure Multiplayer Competitive Architecture**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![UE5.8](https://img.shields.io/badge/Unreal%20Engine-5.8-0E1128?logo=unrealengine)]()
[![F# 8](https://img.shields.io/badge/F%23-8.0-378BBA?logo=fsharp)]()
[![.NET 8](https://img.shields.io/badge/.NET-8.0-512BD4?logo=dotnet)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

---

## 🎮 Project Overview

**HeroHour** is a pure multiplayer competitive RTS — no singleplayer campaigns, no PvE content. All development resources flow into **E-Sports balance**, **deterministic lockstep netcode**, and **community-driven features**. Inspired by the unmatched tactical dynamics and asymmetric warfare of *Command & Conquer: Generals - Zero Hour*, HeroHour sets new AAA production standards for modern competitive RTS.

### Three Pillars of Design

| Pillar | Description |
|--------|-------------|
| **Absolute Asymmetry** | No faction shares base units or economic structures. Every army has unique resource efficiencies, movement properties, tactical disadvantages, and victory conditions. |
| **Deterministic Simulation Authenticity** | Every projectile, pathfinding node, and economic transaction is calculated at millisecond precision (60 ticks/sec). Desync and lag compensation are eliminated via the functional F# core. |
| **Interactive Battlefield** | The world is not a static backdrop — buildings, debris, civilian infrastructure, and weather actively modulate warfare. |

### Key Differentiators

- **Massive Scale**: 1000+ units simultaneously
- **E-Sports Balance**: Every unit, upgrade, and ability designed for competitive play
- **Deterministic Lockstep Netcode**: Zero desync, client-side prediction for responsiveness
- **Data-Driven**: All gameplay balanced through data assets, no recompilation needed
- **Modular Design**: Features independently developed and updated
- **Community-First**: In-game Map Editor, AoD infrastructure, full mod support
- **10 Asymmetric World Powers**: Each with unique tech trees, economies, superweapons, and super-agents

---

## 🏗 Technical Architecture: Dual-Core Engine

The technological breakthrough lies in strict separation between deterministic simulation and high-end visual presentation — the **Dual-Core Engine** solves the classic RTS problem of performance drops at massive unit counts.

```
┌─────────────────────────────────────────────────────────────────┐
│                    Unreal Engine 5.8                           │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │              Presentation Layer                            │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Mass Entity System (1000+ units)                     │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Nanite Geometry + Lumen Lighting                     │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Niagara VFX + MetaSounds Audio                       │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ World Partition / Streaming                          │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            ↓ (UFSharpBridge - Async C++ Bridge) │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │              F# Simulation Layer (Fable 5 / .NET 8)      │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Core Logic Engine (60 TPS Deterministic)            │ │ │
│  │  │ - Economy Simulation                                 │ │ │
│  │  │ - Combat Resolution (Projectile Physics)            │ │ │
│  │  │ - AI Decision Making (State Trees)                  │ │ │
│  │  │ - Resource Management                               │ │ │
│  │  │ - AoD Wave Manager (Deterministic Spawning)         │ │ │
│  │  │ - Superweapon / Super-Agent Manager                  │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Data Management                                     │ │ │
│  │  │ - Data Asset Loader                                 │ │ │
│  │  │ - Configuration Manager                             │ │ │
│  │  │ - Save/Replay System (Seed + Input History)         │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            ↓                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │              .NET 8 Runtime                                │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### F# Simulation Layer (Fable 5 / .NET 8)
The entire game state — exact coordinates of all units, HP values, projectile trajectories, macro-economic transactions, Commander AI decision trees, **AoD wave states**, **superweapon cooldowns**, **super-agent instances** — lives in F#. Immutability guarantees each tick produces an exact successor state. This ensures:
- **Absolute cheat-proofing**
- **Perfect replays** (store only initial seed + input history)
- **Deterministic multiplayer** without desync

### Unreal Engine 5.8 Presentation Layer
UE5 acts purely as visualization/audio interface. It receives position/state data via the async C++ bridge `UFSharpBridge`. Mass Entity transforms data into optimized render instances — 1000+ units at 60 FPS with Nanite, Lumen, Niagara.

### Technology Stack

| Layer | Technology | Purpose |
|-------|------------|---------|
| **Rendering** | Unreal Engine 5.8 | Nanite, Lumen, Niagara VFX, World Partition, Mass Entity |
| **Simulation** | F# 8 / Fable 5 | Core game logic, economy, combat, AI, AoD waves, superweapons |
| **Runtime** | .NET 8 | High-performance managed runtime |
| **Scripting** | C++ / Blueprints | UE5 gameplay framework, GAS, UI, UFSharpBridge |
| **Networking** | Deterministic Lockstep + Prediction | Zero desync, client-side prediction for responsiveness |
| **AI** | UE5 State Trees, Behavior Trees, EQS | Commander AI, unit AI, tactical planning |
| **Data** | UE5 Data Assets + JSON Config | Data-driven design, modding support |
| **Build** | Unreal Build Tool + dotnet CLI + Paket | Unified build pipeline |

---

## 🌍 The 10 Asymmetric World Powers — Superweapons & Super-Agents

Each faction has **exactly ONE superweapon** and **exactly ONE super-agent** (hero unit, max 1 per player on field).

| Faction | Superweapon | Super-Agent |
|---------|-------------|-------------|
| 🇺🇸 **USA** | **Chrono-Laser-Phalanx** — Orbital laser array with temporal targeting; strikes anywhere on map with zero travel time, deletes structures/units in 300m radius | **Specter Prime** — Stealth spectral gunship; phases through terrain, deploys temporal decoys, executes surgical strikes |
| 🇨🇳 **China** | **Nuke-Silicon-Kollaps** — Tactical nuke + EMP pulse hybrid; vaporizes center, EMP rings disable all electronics in 2km for 45s | **Lotus X** — Cyber-commando; hacks enemy production globally, converts units to Chinese control temporarily |
| 🏴‍☠️ **HERO** | **Singularitäts-Schrottkanone** — Fires compressed scrap singularity; pulls all units/structures in 500m, crushes them, converts mass to credits | **Nexus Core** — Shapeshifter; copies any enemy unit's abilities, hijacks tech tree access dynamically |
| 🇷🇺 **Russia** | **Tesla-Giga-Spule** — Massive Tesla coil tower; chain-lightning arcs across entire map, prioritizes high-value targets | **Omega Boris** — Tesla-exoskeleton titan; projects shielding aura, overloads enemy vehicles on proximity |
| 🇮🇷 **Iran** | **Sejjil-III Giga-Silo** — Ballistic missile with kinetic penetrators + delayed radiation cloud; denies area for 3 minutes | **Commander Shahin** — Fortress commander; deploys instant bunkers, calls precision strikes, regenerates structures |
| 🇹🇷 **Türkiye** | **TAF-Anka Global Strike** — Swarm of 500 micro-drones from orbit; surgical strikes on all enemy production simultaneously | **Kaan** — Drone carrier hero; commands personal swarm, marks targets for artillery multiplication |
| 🇵🇰 **Pakistan** | **Giga-EMP-Kaskade** — Atmospheric EMP burst; global electronics shutdown for 60s, friendly units hardened | **Guerilla Shadow** — Phasing infiltrator; places EMP traps, converts enemy structures to neutral |
| 🇮🇳 **India** | **Agni-V Plasmabrenner** — Railgun-fired plasma slug from orbit; penetrates crust, creates volcanic vent denying region | **Akash** — Mecha-avatar; railgun arm, quantum shield, calls orbital strikes |
| 🇯🇵 **Japan** | **Mecha-Giga-Wave** — Resonance pulse from Quantum Cores; shatters all mechanical units in hemisphere, heals friendly mechas | **Shinobi 2.0** — Quantum ninja; teleports, phases, executes multi-target assassinations in single tick |
| 🇰🇷 **South Korea** | **K-Sat Cyber-Strahl** — Satellite beam; rewrites enemy unit AI to friendly for 30s, reveals all stealth | **Viper** — Cyber-assassin; hacks superweapons, redirects strikes, disables enemy super-agents |

---

### Faction Visual Overview

| Faction | Showcase |
|---------|----------|
| 🇺🇸 **USA** | ![USA](Art/Concepts/Factions/USA.png) |
| 🇨🇳 **China** | ![China](Art/Concepts/Factions/China.png) |
| 🏴‍☠️ **HERO** | ![HERO](Art/Concepts/Factions/HERO.png) |
| 🇹🇷 **Türkiye** | ![Türkiye](Art/Concepts/Factions/Türkiye.png) |
| 🇮🇷 **Iran** | ![Iran](Art/Concepts/Factions/Iran.png) |
| 🇵🇰 **Pakistan** | ![Pakistan](Art/Concepts/Factions/Pakistan.png) |
| 🇮🇳 **India** | ![Indien](Art/Concepts/Factions/Indien.png) |
| 🇯🇵 **Japan** | ![Japan](Art/Concepts/Factions/Japan.png) |
| 🇰🇷 **South Korea** | ![Südkorea](Art/Concepts/Factions/Südkorea.png) |
| 🇷🇺 **Russia** | ![Russland](Art/Concepts/Factions/Russland.png) |

---

### Core Concept Art

| Concept | Image |
|---------|-------|
| **Main Game Concept** | ![HERO HOUR Concept](Art/Concepts/Factions/HERO%20HOUR_Concept.png) |
| **All Factions Overview** | ![Concept Army](Art/Concepts/Factions/Concept_Army.png) |
| **Superweapons Matrix** | ![Superweapons](Art/Concepts/Factions/SUPERWEAPONS.png) |
| **Super-Agents Matrix** | ![SP Agents](Art/Concepts/Factions/SP_Agent_Concept.png) |
| **Superweapons Detail** | ![SP Weapons](Art/Concepts/Factions/SP_Weapon_Concept.png) |
| **UI / HUD Concept** | ![UI Concept](Art/Concepts/Factions/UI_Concept.png) |

---

### Unit Archetype Concept Art

| Archetype | Image |
|-----------|-------|
| **Infantry Units** | ![Infantry](Art/Concepts/Factions/Infantry_Unit_Concept.png) |
| **Land Vehicles** | ![Land](Art/Concepts/Factions/Land_Unit_Concept.png) |
| **Air Units** | ![Air](Art/Concepts/Factions/Air_Unit_Concept.png) |
| **Mixed Formations** | ![Mix](Art/Concepts/Factions/Mix_Unit_Concept.png) |

---

### Faction Tactical Units Concept Art

![HeroHour Tactical Units](Art/Concepts/Factions/Factions_Tactical_Units_ConceptArt.png)

*All 10 factions' tactical combat units — infantry, vehicles, aircraft, structures with silhouettes for instant recognition*

---

### Faction Concept Art

![HeroHour Faction Concepts](Art/Concepts/Factions/Factions_Core_ConceptArt.png)

*All 10 factions with their Superweapons & Super-Agents visualized*

---

### Map Concept Art

![Map Concept](Art/Concepts/Factions/Map_Concept.png)

*Strategic map layout concepts for competitive and AoD modes*

---

### Concept Video

<video controls width="800">
  <source src="Art/Concepts/Factions/HeroHour_Concept_Video.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

*HeroHour concept video showcasing gameplay, factions, superweapons, and super-agents in action*

---

## 🗺️ In-Game Map Editor & AoD Infrastructure

### Runtime Map Editor (UE5.8)
Integrated directly into the game client — no external tools required.

**Features:**
- **Terrain Sculpting**: Heightmap painting, erosion simulation, biome blending
- **Civilian Infrastructure Placement**: Drag-drop cities, roads, capturable structures
- **AoD Wave Designer**: Visual node-graph for wave composition, scaling rules, attack routes
- **Playtest Instant**: One-click "Test Wave" spawns simulation in-editor
- **Publishing**: One-click workshop upload with versioning, tags, ratings
- **Multiplayer Co-Editing**: Real-time collaborative editing via lockstep sync

**Technical Implementation:**
- UE5 Editor Utility Widgets + Slate for UI
- Runtime landscape editing via `ULandscapeSubsystem`
- Data Assets for all map objects (versioned, diffable)
- Blueprint-exposable editor functions for community extensions

### AoD (Art of Defense) Wave System — F# Deterministic Core

The **mathematical infrastructure** for cooperative survival maps. Fully deterministic, runs in F# simulation layer.

**Core Concepts:**
- **Wave Definition**: `SpawnWave` — wave ID, unit type list, scaling factor (player count), spawn interval, formation
- **Wave Manager State**: Current wave, timer to next wave, remaining AI units on field, difficulty scaling curve
- **Intelligent Routing**: A* + flow-field hybrid computes dynamic attack paths avoiding player defenses
- **Scaling Formula**: `UnitCount = BaseCount * (1 + (PlayerCount - 1) * 0.65) * WaveDifficultyMultiplier`
- **Adaptive AI**: Analyzes player build patterns, targets weakest economic sector, flanks static defenses

**F# Types (Source/HeroHourSimulation/src/AoD/):**
```fsharp
type SpawnWave = {
    WaveId: int
    UnitTypes: UnitTypeId list
    BaseCount: int
    ScalingFactor: float
    SpawnInterval: float
    Formation: FormationType
    AttackRoute: RouteId
}

type WaveManagerState = {
    CurrentWave: int
    TimerToNextWave: float
    RemainingAIUnits: int
    DifficultyCurve: DifficultySettings
    ActiveWaves: SpawnWave list
}
```

**Network Sync**: Wave timer and spawn events replicated via deterministic lockstep — zero bandwidth overhead, perfect sync.

---

## ⚡ Core Multiplayer Features

### Deterministic Lockstep Netcode
- **60 TPS Fixed Timestep**: All clients simulate identical state
- **Input Delay Masking**: Client-side prediction for movement/abilities
- **Rollback**: Frame-perfect reconciliation on desync (theoretical — F# immutability prevents)
- **Bandwidth**: ~2 KB/s per player (inputs only)

### Combat School (Post-Match Analytics)
Deep F# simulation-layer analytics. Logs every action. Post-match UI in UE5:
- Geographic loss heatmap
- APM curve with percentile comparison
- Build-order efficiency vs optimal
- AI-generated suggestions: *"2nd War Factory 45s late → 20% tank delay"*

### Community Features
- **In-Game Tournament Bracket**: Automated Swiss/DE brackets, streaming integration
- **Replay Exchange**: Deterministic replays = instant sharing, frame-perfect seeking
- **Map Workshop**: Editor publishing, ratings, curated map pools per season
- **Clan System**: Shared resources, clan wars, custom lobbies

---

## 📁 Project Structure

```
HeroHour/
├── .github/                    # GitHub Actions CI/CD
│   ├── workflows/
│   │   ├── ci.yml              # Continuous Integration
│   │   ├── cd.yml              # Continuous Deployment
│   │   ├── nightly.yml         # Nightly builds
│   │   └── release.yml         # Release automation
│   └── ISSUE_TEMPLATE/         # Issue templates (Bug, Feature, Tech Debt, Epic)
├── .vscode/                    # VS Code configuration
│   ├── settings.json
│   ├── launch.json
│   ├── tasks.json
│   └── extensions.json
├── docs/                       # Documentation
│   ├── gdd/                    # Game Design Documents
│   │   ├── HERO_HOUR_GDD.md           # Full GDD
│   │   ├── FRACTIONS_OVERVIEW.md      # 10 Factions + Superweapons/Agents
│   │   ├── SUPERWEAPONS.md            # Superweapon design specs
│   │   ├── SUPER_AGENTS.md            # Super-agent design specs
│   │   ├── MAP_EDITOR.md              # In-game editor design
│   │   ├── AOD_INFRASTRUCTURE.md      # AoD wave system design
│   │   ├── COMBAT_SCHOOL.md           # Analytics system design
│   │   ├── WEATHER_SYSTEM.md          # Dynamic weather mechanics
│   │   └── URBAN_WARFARE.md           # Civilian infrastructure design
│   ├── architecture/           # Technical Architecture
│   ├── technical/              # Technical Specifications
│   ├── production/             # Production Pipeline
│   └── api/                    # API Documentation
├── Source/                     # UE5 C++ Source
│   ├── HeroHour/               # Main game module
│   │   ├── HeroHour.Build.cs
│   │   ├── Public/
│   │   │   ├── Simulation/     # F# bridge interfaces
│   │   │   ├── GameplayAbility/ # GAS extensions
│   │   │   ├── AI/             # Commander AI, State Trees
│   │   │   ├── Network/        # Deterministic networking
│   │   │   ├── Data/           # Faction data assets
│   │   │   ├── UI/             # Combat School, Map Editor UI
│   │   │   └── AoD/            # AoD wave UI components
│   │   └── Private/
│   ├── HeroHourSimulation/     # F# Simulation Layer (.NET 8)
│   │   ├── HeroHourSimulation.fsproj
│   │   ├── src/
│   │   │   ├── Core/           # Types, GameState, GameCommand, SimulationResult
│   │   │   ├── Economy/        # ResourceManager, EconomySimulator
│   │   │   ├── Combat/         # CombatResolver, ProjectilePhysics
│   │   │   ├── AI/             # CommanderAI, StateTreeIntegration
│   │   │   ├── Units/          # Faction unit logic
│   │   │   ├── Buildings/      # Production, Construction
│   │   │   ├── Data/           # DataLoader, FactionDataAssets
│   │   │   ├── Weather/        # WeatherSimulation, WeatherEffects
│   │   │   ├── Urban/          # CivilianInfrastructure, CaptureMechanics
│   │   │   ├── AoD/            # AoDTypes, WaveManager, SpawnSystem
│   │   │   ├── Superweapons/   # SuperWeaponTypes, SuperWeaponManager
│   │   │   ├── SuperAgents/    # HeroTypes, SuperAgentManager
│   │   │   └── Events/         # EventBus, CombatSchoolLogger
│   │   └── tests/              # Expecto unit tests
│   ├── HeroHourEditor/         # Editor extensions (Map Editor, Faction editors)
│   └── HeroHourGame/           # Game-specific code
│       ├── Config/
│       │   ├── DefaultEngine.ini
│       │   ├── DefaultGame.ini
│       │   └── DefaultInput.ini
│       └── Content/
├── Scripts/                    # Build & automation scripts
│   ├── build/                  # Build.ps1, BuildUE5.ps1, BuildFable.ps1, BuildDotNet.ps1
│   ├── ci/                     # Setup scripts
│   └── tools/                  # DataAssetGenerator, ModTooling, PerformanceProfiler
├── Content/                    # UE5 Content (LFS)
│   ├── Blueprints/
│   ├── DataAssets/             # 10 Faction DataAssets, Superweapon DataAssets, Hero DataAssets
│   ├── Maps/                   # Competitive maps, AoD maps
│   ├── Materials/
│   ├── Niagara/                # Faction VFX, Weather VFX, Superweapon VFX
│   └── UI/                     # Combat School, Map Editor, Tournament UI
├── Art/                        # Source art assets
├── Audio/                      # Audio assets
├── Localization/               # 7 Languages
├── Tests/                      # Test projects
├── HeroHour.uproject           # UE5 Project file
├── HeroHour.sln                # Solution file
├── Directory.Build.props       # MSBuild properties
├── global.json                 # .NET SDK version
├── nuget.config                # NuGet configuration
├── paket.dependencies          # Paket dependencies
├── README.md                   # This file
├── CONTRIBUTING.md             # Contribution guide
├── CHANGELOG.md                # Version history
├── LICENSE                     # MIT License
├── .gitignore
├── .gitattributes
├── .editorconfig
└── GITHUB_COMMIT_STRATEGY.md   # Git strategy
```

---

## 🚀 Getting Started

### Prerequisites

| Tool | Version | Purpose |
|------|---------|---------|
| **Unreal Engine** | 5.8+ | Game engine |
| **.NET SDK** | 8.0+ | F# simulation layer |
| **F# / Fable** | 8.0 / 5.0 | Functional simulation |
| **Visual Studio** | 2022 17.9+ | C++ development |
| **VS Code** | Latest | F# development (Ionide) |
| **Git LFS** | 3.4+ | Large file storage |
| **Paket** | 8.0+ | .NET dependency management |

### Installation

```bash
# Clone with LFS
git lfs install
git clone https://github.com/OrhanHero/HeroHour.git
cd HeroHour
git lfs pull

# Install .NET dependencies
dotnet restore

# Install Paket dependencies
paket restore

# Generate UE5 project files
./Scripts/build/GenerateProjectFiles.sh

# Open in UE5 Editor
./HeroHour.uproject
```

### Development Workflow

```bash
# 1. Create feature branch
git checkout -b feature/HH-123-hero-ability-system

# 2. Make changes (follow conventional commits)
git add .
git commit -m "feat(simulation): add hero ability cooldown system

- Implement cooldown tracking in simulation layer
- Add data asset for ability cooldowns
- Integrate with GAS cooldown system

Closes HH-123"

# 3. Push and create PR
git push origin feature/HH-123-hero-ability-system
gh pr create --title "feat(simulation): Hero ability cooldown system" --body "..."

# 4. After review & CI pass: squash merge to main
```

---

## 🏗 Build System

### Build Commands

```bash
# Development build
./Scripts/build/BuildDevelopment.sh

# Shipping build
./Scripts/build/BuildShipping.sh

# Run tests
./Scripts/build/RunTests.sh

# Package for distribution
./Scripts/build/PackageGame.sh
```

### CI/CD Pipeline

| Stage | Trigger | Duration | Artifacts |
|-------|---------|----------|-----------|
| **PR Validation** | Pull Request | ~15 min | Test results, coverage |
| **Nightly Build** | Schedule (02:00 UTC) | ~45 min | Development build |
| **Release Build** | Tag push (v*) | ~60 min | Shipping build, installer |
| **Deploy** | Release publish | ~10 min | Store upload |

---

## 🧪 Testing Strategy

| Test Type | Framework | Coverage Target |
|-----------|-----------|-----------------|
| **Unit (F#)** | Expecto | > 85% |
| **Unit (C++)** | GoogleTest | > 70% |
| **Integration** | Custom + Gauntlet | Critical paths |
| **E2E** | Unreal Automation | Gameplay loops |
| **Performance** | UE Insights + custom | Regression < 5% |
| **Network** | Custom replication test | Determinism |
| **AoD Wave Tests** | Expecto | Wave scaling, routing, sync |

---

## 📚 Documentation

| Document | Location | Status |
|----------|----------|--------|
| **Game Design Document** | `docs/gdd/HERO_HOUR_GDD.md` | ✅ Complete |
| **Factions Overview + Superweapons/Agents** | `docs/gdd/FRACTIONS_OVERVIEW.md` | ✅ Complete |
| **Superweapons Design** | `docs/gdd/SUPERWEAPONS.md` | ✅ Complete |
| **Super-Agents Design** | `docs/gdd/SUPER_AGENTS.md` | ✅ Complete |
| **Map Editor Design** | `docs/gdd/MAP_EDITOR.md` | ✅ Complete |
| **AoD Infrastructure** | `docs/gdd/AOD_INFRASTRUCTURE.md` | ✅ Complete |
| **Combat School Design** | `docs/gdd/COMBAT_SCHOOL.md` | ✅ Complete |
| **Weather System** | `docs/gdd/WEATHER_SYSTEM.md` | ✅ Complete |
| **Urban Warfare** | `docs/gdd/URBAN_WARFARE.md` | ✅ Complete |
| **Technical Architecture** | `docs/architecture/TECHNICAL_ARCHITECTURE.md` | 📝 In Progress |
| **API Reference** | `docs/api/` | 📝 Planned |
| **Production Pipeline** | `docs/production/PIPELINE.md` | 📝 Planned |
| **Modding Guide** | `docs/modding/GUIDE.md` | 📝 Planned |

---

## 🤝 Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code of Conduct
- Development setup
- Coding standards
- Commit message convention (Conventional Commits 1.0)
- Pull request process
- Review guidelines

### Quick Links
- [Issue Templates](.github/ISSUE_TEMPLATE/)
- [PR Template](.github/PULL_REQUEST_TEMPLATE.md)
- [Git Strategy](GITHUB_COMMIT_STRATEGY.md)
- [Architecture Decisions](docs/architecture/ADR/)

---

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

- **Command & Conquer: Generals - Zero Hour** — Inspiration
- **Unreal Engine Team** — Epic Games
- **F# Software Foundation** — F# language & tooling
- **Fable Team** — Fable compiler
- **Open Source Community** — Countless libraries & tools

---

## 📞 Contact

- **Project Lead**: [Hero](mailto:hero@herohour.dev)
- **Technical Director**: [Tech Lead](mailto:tech@herohour.dev)
- **Discord**: [HeroHour Community](https://discord.gg/herohour)
- **Website**: [herohour.dev](https://herohour.dev)

---

*HeroHour — Pure Multiplayer Competitive RTS. Where Strategy Meets Spectacle.* ⚔️