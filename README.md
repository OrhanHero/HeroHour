# HeroHour - AAA RTS Game

> **Command & Conquer: Generals Zero Hour** inspired Real-Time Strategy game built with **Unreal Engine 5.8**, **F# (Fable 5)**, and **.NET 8**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![UE5.8](https://img.shields.io/badge/Unreal%20Engine-5.8-0E1128?logo=unrealengine)]()
[![F# 8](https://img.shields.io/badge/F%23-8.0-378BBA?logo=fsharp)]()
[![.NET 8](https://img.shields.io/badge/.NET-8.0-512BD4?logo=dotnet)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

---

## 🎮 Project Overview

**HeroHour** is not merely a nostalgic tribute to the golden age of RTS — it is a deliberate technological and gameplay evolution of the genre. Inspired by the unmatched tactical dynamics and asymmetric warfare of *Command & Conquer: Generals - Zero Hour*, HeroHour sets new AAA production standards for modern PC gamers.

The project rejects post-launch feature additions — all architectural depth, all **10 geopolitical factions**, and modern simulation tools are core from Day 1.

### Three Pillars of Design

| Pillar | Description |
|--------|-------------|
| **Absolute Asymmetry** | No faction shares base units or economic structures. Every army has unique resource efficiencies, movement properties, tactical disadvantages, and victory conditions. |
| **Deterministic Simulation Authenticity** | Every projectile, pathfinding node, and economic transaction is calculated at millisecond precision (60 ticks/sec). Desync and lag compensation are eliminated via the functional F# core. |
| **Interactive Battlefield** | The world is not a static backdrop — buildings, debris, civilian infrastructure, and weather actively modulate warfare. |

### Key Differentiators

- **Massive Scale**: 1000+ units simultaneously
- **Smart AI**: Commander AI with strategic planning and adaptation (UE5 State Trees)
- **Data-Driven**: All gameplay balanced through data assets, no recompilation needed
- **Modular Design**: Features independently developed and updated
- **Cross-Platform Ready**: Built with scalability in mind
- **Deep Modding**: Full mod support through data assets
- **10 Asymmetric World Powers**: Each with unique tech trees, economies, and playstyles

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
The entire game state — exact coordinates of all units, HP values, projectile trajectories, macro-economic transactions, and Commander AI decision trees — lives in F#. Immutability guarantees each tick produces an exact successor state. This ensures:
- **Absolute cheat-proofing**
- **Perfect replays** (store only initial seed + input history)
- **Deterministic multiplayer** without desync

### Unreal Engine 5.8 Presentation Layer
UE5 acts purely as visualization/audio interface. It receives position/state data via the async C++ bridge `UFSharpBridge`. Mass Entity transforms data into optimized render instances — 1000+ units at 60 FPS with Nanite, Lumen, Niagara.

### Technology Stack

| Layer | Technology | Purpose |
|-------|------------|---------|
| **Rendering** | Unreal Engine 5.8 | Nanite, Lumen, Niagara VFX, World Partition, Mass Entity |
| **Simulation** | F# 8 / Fable 5 | Core game logic, economy, combat, AI decisions |
| **Runtime** | .NET 8 | High-performance managed runtime |
| **Scripting** | C++ / Blueprints | UE5 gameplay framework, GAS, UI, UFSharpBridge |
| **Networking** | Deterministic Lockstep + Prediction | Zero desync, client-side prediction for responsiveness |
| **AI** | UE5 State Trees, Behavior Trees, EQS | Commander AI, unit AI, tactical planning |
| **Data** | UE5 Data Assets + JSON Config | Data-driven design, modding support |
| **Build** | Unreal Build Tool + dotnet CLI + Paket | Unified build pipeline |

---

## 🌍 The 10 Asymmetric World Powers

Each faction has a completely independent technology bundle, individual economic model, and custom Commander sub-classes.

### 1. 🇺🇸 United States — Technological Dominance & Air Superiority
*High-tech precision, stealth operations, absolute air control. Expensive units requiring precise micro, highest survivability.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Specter Gunship II** | Massive air support ship; orbits sector permanently, auto-attacks with 105mm howitzer + 25mm gatling |
| **Special Unit** | **Microwave Stryker** | Heavy wheeled vehicle; emits high-frequency microwave field — disables building power, cooks infantry in cover |
| **Structure** | **Satellite Uplink Command** | Orbital control center; global scan (20s fog reveal), permanent stealth detection radius |
| **Upgrade** | **Hellfire Missiles** | Drones/Jets gain laser-guided Hellfires — armor penetration, bonus vs heavy armor |
| **Upgrade** | **Advanced Chemical Suits** | 100% immunity to bio-toxins & radioactive fallout |

### 2. 🇨🇳 China — Iron Mass & Cyber Warfare
*Sheer production power, massive tank swarms, flamethrower weapons, state-organized cyber espionage. Swarm Bonus: attack/speed bonuses in groups.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Hacker Elite** | Stealth while stationary; hacks enemy vehicles (disable) or supply lines (credit siphon) |
| **Special Unit** | **Overlord Flame-Tank** | Monumental heavy tank; dual napalm projectors — burns defensive lines in seconds, leaves burning ground |
| **Structure** | **China Internet Center** | Massive bunker; garrison up to 8 Hackers for global cyber-mining (massive passive income) |
| **Upgrade** | **Napalm Uplink** | All tank shells & artillery gain napalm component — burning inferno areas on impact |

### 3. 🏴‍☠️ GLA (Global Liberation Army) — Guerrilla & Toxin Networks
*Asymmetric warfare, toxin networks, suicide units, ambushes, scavenging. No conventional power grid.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Toxin Tractor** | Fast technical; sprays persistent toxin clouds, denies area |
| **Special Unit** | **Bomb Truck** | Disguised civilian vehicle; massive suicide explosion on detonation |
| **Structure** | **Arms Dealer** | Produces all ground units; upgradeable to Black Market |
| **Upgrade** | **Anthrax Beta** | Toxin weapons gain persistent damage over time + armor corrosion |

### 4. 🇹🇷 Türkiye — Autonomous Drone Dominance & Mobile Fortresses
*Modern networked unmanned warfare + highly mobile mechanized divisions.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Bayraktar Swarm Drone** | Remote-operated high-end drone; swarm AI, precision missiles, marks targets for artillery (2x range) |
| **Special Unit** | **Altay-M MBT** | Main battle tank; **Fortress Mode** — digs in, +80% armor, switches to heavy kinetic cannon with massive range |
| **Upgrade** | **Active Protection System (APS)** | All heavy vehicles get computer-controlled interception — destroys first incoming explosive projectile, 15s cooldown |

### 5. 🇮🇷 Iran — Ballistic Armor & Fortress Doctrine
*Territory lockdown + long-range ballistic rocket barrages for psychological/physical attrition.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Fateh Mobile Launcher** | Massive tracked rocket vehicle; manual anchoring required; fires heavy SRBMs devastating base areas |
| **Structure** | **Underground Silo** | Deep-earth missile bunker; **immune to all conventional attacks (including superweapons)** while closed; opens only 3s during launch |
| **Upgrade** | **Bunker Reinforcement** | +50% structure HP on all defenses; garrisoned infantry fire ATGMs with increased velocity |

### 6. 🇵🇰 Pakistan — Asymmetric Counterstrike & EMP Ambushes
*Terrain geometry mastery, concealed ambushes, EMP neutralization, lightning needle strikes.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **EMP Commando** | Elite sabotage; invisible in Fog of War; places EMP charges on buildings or throws EMP grenades — disables entire tank columns for 12s |
| **Structure** | **Hidden Training Camp** | Barracks with permanent radar-jamming field; units invisible on enemy minimap until first shot |
| **Upgrade** | **Guerrilla Regeneration** | Units auto-regenerate HP rapidly while in Fog of War outside enemy LOS |

### 7. 🇮🇳 India — Technological Mass & Railgun Prototypes
*Gigantic advanced arms industry + breakthrough lab/software research. Mechanized excellence + kinetic high-energy weapons.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Arjun Mk-III** | Tech showcase tank; **experimental Railgun** — projectile penetrates multiple vehicles in line, ignores physical armor |
| **Structure** | **SDRF Tech Center** | Advanced IT/research complex; **-25% cost & research time for all global upgrades**, detects enemy cyber attacks instantly |
| **Upgrade** | **Composite Reactive Armor** | Intelligent armor reducing explosive damage (artillery/rockets) by flat 40% |

### 8. 🇯🇵 Japan — Precise Mecha Robotics & Quantum Networks
*Almost no tracked vehicles — bipedal mechas, sci-fi energy weapons, autonomous shields. Extreme mobility.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **Tetsujin Walker** | Agile bipedal combat mech; hydraulically optimized joints — **jumps cliffs, rivers, rough terrain** for devastating flanking |
| **Structure** | **Quantum Core** | Revolutionary power source; **5x output of standard plant** on minimal footprint; explodes in massive plasma shockwave when destroyed |
| **Upgrade** | **Plasma Shield Harmonizer** | All mechs gain blue plasma hull; absorbs fixed damage, fully regenerates after 6s without damage |

### 9. 🇰🇷 South Korea — K-Cyber Network & Subsonic Stealth Air Force
*Information warfare mastery + surgical air strikes with extreme-speed jets. Enemy fights blind; SK Commander sees everything.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **KF-X Black Shark** | Futuristic supersonic stealth bomber; **radar-invisible** until bomb release; then visible briefly, accelerates to Mach 2 to escape AA |
| **Structure** | **K-Sat Relay Center** | High-res satellite center; enables **Cyber Blackout** — blinds enemy radar & minimap completely for 30s |
| **Upgrade** | **Smart-Predictive Munition** | All projectiles use pre-calculation algorithm — **guaranteed 100% hit rate on moving targets** (especially fast aircraft/bikes) |

### 10. 🇷🇺 Russian Federation — Heavy Armor Industry & Thermobaric Devastation
*Raw violence, massive steel tracks, Tesla energy, ruthless thermobaric weapons. Highest structure HP in game.*

| Type | Unit / Structure | Description |
|------|------------------|-------------|
| **Special Unit** | **T-14 Armata Overlord** | Colossal dual-main-gun tank; **auto AA flak on turret** — autonomous defense vs air & ground |
| **Special Unit** | **Tesla Trooper** | Heavy infantry in exoskeleton; chain lightning jumps between vehicles, slows mechanical units |
| **Structure** | **Giga-Fabrik** | Massive arms forge; **only building producing two vehicles simultaneously in same queue** |
| **Upgrade** | **Thermobaric Warheads** | Rocket artillery (TOS-2) gets thermobaric ammo — massive heatwave eliminates infantry instantly, burns ground 10s |

---

## ⚡ Core Gameplay Features

### Combat School (Tactical Post-Mission Analysis)
Deep F# simulation-layer analytics tool. Logs every action (clicks, hotkeys, build-order timings, resource efficiency). Post-match visualization in UE5 UI:
- **Geographic loss heatmap**
- **APM curve**
- **AI-generated suggestions**: *"2nd depot 45s late → 20% tank production delay"*

### Generals Challenge Mode (Ultimate Campaign)
Singleplayer centerpiece. Choose 1 of 10 factions → face 9 hyper-specialized AI Generals (UE5 State Trees). Each General uses extreme faction alignment (e.g., US Laser General, GLA Toxin General). AI dynamically analyzes player counters and adapts routes. Defeating a General unlocks them for Multiplayer + unique profile medals.

### Dynamic Weather Simulation (Direct Gameplay Impact)
Full UE5 physical particle simulation — weather decisively affects tactics:

| Weather | Effect |
|---------|--------|
| **Heavy Rain** | Cools air; **-30% laser damage/range** (USA, Japan) — light scatters in water droplets |
| **Massive Sandstorms** | **-70% sight range** for all units/towers; radar systems fail completely — perfect for GLA ambushes / Pakistan EMP |
| **Heavy Snowfall** | Ground freezes; **tracked vehicles (Russia, China) -25% speed/grip**; light mechas (Japan) unaffected |

### Neutral Elements & Urban Warfare
Maps are living, tactically usable civilian environments, not empty arenas.

#### Civilian Infrastructure & Capture Mechanics
- **Civilian vehicles** (cars, buses, tractors) in urban areas — **GLA/HERO** can garrison infantry to instantly convert into **improvised SVBIEDs** or **mobile militia gun platforms**
- **Large urban buildings** (skyscrapers, malls) — garrison up to 20 infantry → **hard-to-crack fortresses at strategic chokepoints**

#### Capturable High-Tech Structures (via Engineers)
| Structure | Benefit |
|-----------|---------|
| **Civilian Oil Refinery** | Continuous reliable Credit stream in F# economy |
| **Civilian Major Hospital** | Map-wide passive healing field; all injured infantry auto-heal over time when not in combat |
| **Intelligence Outpost** | Hacks enemy frequencies; every 90s reveals exact build/production queue of all enemies |

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
│   │   ├── HERO_HOUR_GDD.md           # Full GDD from Ultimate Spec
│   │   ├── FRACTIONS_OVERVIEW.md      # 10 Factions deep-dive
│   │   ├── COMBAT_SCHOOL.md           # Analytics system design
│   │   ├── GENERALS_CHALLENGE.md      # Campaign mode design
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
│   │   │   └── UI/             # Combat School, Generals Challenge UI
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
│   │   │   └── Events/         # EventBus, CombatSchoolLogger
│   │   └── tests/              # Expecto unit tests
│   ├── HeroHourEditor/         # Editor extensions (Faction editors, Weather tool)
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
│   ├── DataAssets/             # 10 Faction DataAssets, Weather DataAssets
│   ├── Maps/                   # Urban maps with civilian infrastructure
│   ├── Materials/
│   ├── Niagara/                # Faction VFX, Weather VFX
│   └── UI/                     # Combat School, Generals Challenge widgets
├── Art/                        # Source art assets
│   ├── Concepts/
│   ├── Characters/
│   ├── Buildings/
│   ├── Environments/
│   ├── VFX/
│   └── UI/
├── Audio/                      # Audio assets
│   ├── Music/
│   ├── SFX/
│   ├── Voice/
│   └── MetaSounds/
├── Localization/               # 7 Languages
│   ├── en/                     # English (source)
│   ├── de/                     # German
│   ├── fr/                     # French
│   ├── es/                     # Spanish
│   ├── tr/                     # Turkish
│   ├── ja/                     # Japanese
│   └── ko/                     # Korean
├── Tests/                      # Test projects
│   ├── Unit/
│   ├── Integration/
│   ├── E2E/
│   └── Performance/
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

---

## 📚 Documentation

| Document | Location | Status |
|----------|----------|--------|
| **Game Design Document** | `docs/gdd/HERO_HOUR_GDD.md` | ✅ Complete |
| **Factions Overview** | `docs/gdd/FRACTIONS_OVERVIEW.md` | ✅ Complete |
| **Combat School Design** | `docs/gdd/COMBAT_SCHOOL.md` | ✅ Complete |
| **Generals Challenge** | `docs/gdd/GENERALS_CHALLENGE.md` | ✅ Complete |
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

*HeroHour — Where Strategy Meets Spectacle* ⚔️