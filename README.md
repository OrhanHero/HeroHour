# HeroHour - AAA RTS Game

> **Command & Conquer: Generals Zero Hour** inspired Real-Time Strategy game built with **Unreal Engine 5.8**, **F# (Fable 5)**, and **.NET 8**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![UE5.8](https://img.shields.io/badge/Unreal%20Engine-5.8-0E1128?logo=unrealengine)]()
[![F# 8](https://img.shields.io/badge/F%23-8.0-378BBA?logo=fsharp)]()
[![.NET 8](https://img.shields.io/badge/.NET-8.0-512BD4?logo=dotnet)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

---

## 🎮 Project Overview

**HeroHour** combines the strategic depth of classic RTS games with modern AAA production values. The game leverages Unreal Engine 5.8's cutting-edge technology (Nanite, Lumen, Niagara) while maintaining the responsive gameplay and tactical depth that defined the golden age of RTS.

### Core Pillars

| Pillar | Description |
|--------|-------------|
| **Strategic Depth** | Multiple victory conditions, dynamic tech trees, counter-based unit system, economy management, territory control |
| **Tactical Combat** | Cover system, flanking bonuses, unit abilities, formation controls, terrain advantages |
| **Hero System** | Unique commanders with special abilities, hero progression, unit buffs/debuffs, signature moves |
| **AI Intelligence** | Adaptive difficulty, strategic planning, scouting/reconnaissance, economic management, tactical decision making |

### Key Differentiators

- **Massive Scale**: 1000+ units simultaneously
- **Smart AI**: Commander AI with strategic planning and adaptation
- **Data-Driven**: All gameplay balanced through data assets, no recompilation needed
- **Modular Design**: Features independently developed and updated
- **Cross-Platform Ready**: Built with scalability in mind
- **Deep Modding**: Full mod support through data assets

---

## 🏗 Technical Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    Unreal Engine 5.8                           │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │              Gameplay Framework                            │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Gameplay Ability System (GAS)                        │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Mass Entity System                                   │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ State Trees / Behavior Trees                         │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ World Partition / Streaming                          │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            ↓                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │              F# Simulation Layer                          │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Core Logic Engine                                    │ │ │
│  │  │ - Economy Simulation                                 │ │ │
│  │  │ - Combat Resolution                                  │ │ │
│  │  │ - AI Decision Making                                 │ │ │
│  │  │ - Resource Management                                │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  │  ┌──────────────────────────────────────────────────────┐ │ │
│  │  │ Data Management                                      │ │ │
│  │  │ - Data Asset Loader                                  │ │ │
│  │  │ - Configuration Manager                              │ │ │
│  │  │ - Save System                                        │ │ │
│  │  └──────────────────────────────────────────────────────┘ │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            ↓                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │              .NET 8 Runtime                                │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### Technology Stack

| Layer | Technology | Purpose |
|-------|------------|---------|
| **Rendering** | Unreal Engine 5.8 | Nanite, Lumen, Niagara VFX, World Partition |
| **Simulation** | F# 8 / Fable 5 | Core game logic, economy, combat, AI decisions |
| **Runtime** | .NET 8 | High-performance managed runtime |
| **Scripting** | C++ / Blueprints | UE5 gameplay framework, GAS, UI |
| **Networking** | UE5 Netcode + Custom Prediction | Deterministic lockstep + client prediction |
| **AI** | State Trees, Behavior Trees, EQS | Commander AI, unit AI, tactical planning |
| **Data** | UE5 Data Assets + JSON Config | Data-driven design, modding support |
| **Build** | Unreal Build Tool + dotnet CLI | Unified build pipeline |

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
│   └── ISSUE_TEMPLATE/         # Issue templates
├── .vscode/                    # VS Code configuration
│   ├── settings.json
│   ├── launch.json
│   ├── tasks.json
│   └── extensions.json
├── docs/                       # Documentation
│   ├── gdd/                    # Game Design Documents
│   ├── architecture/           # Technical Architecture
│   ├── technical/              # Technical Specifications
│   ├── production/             # Production Pipeline
│   └── api/                    # API Documentation
├── Source/                     # UE5 C++ Source
│   ├── HeroHour/               # Main game module
│   │   ├── Public/             # Public headers
│   │   │   ├── Simulation/     # F# bridge interfaces
│   │   │   ├── GameplayAbility/ # GAS extensions
│   │   │   ├── AI/             # AI systems
│   │   │   ├── Network/        # Networking
│   │   │   ├── Data/           # Data assets
│   │   │   └── UI/             # UI systems
│   │   └── Private/            # Private implementation
│   ├── HeroHourSimulation/     # F# Simulation Layer
│   │   ├── src/
│   │   │   ├── Core/           # Core simulation types
│   │   │   ├── Economy/        # Economy simulation
│   │   │   ├── Combat/         # Combat resolution
│   │   │   ├── AI/             # AI decision making
│   │   │   ├── Units/          # Unit logic
│   │   │   ├── Buildings/      # Building logic
│   │   │   ├── Data/           # Data loading
│   │   │   └── Events/         # Event system
│   │   └── tests/              # Unit tests
│   ├── HeroHourEditor/         # Editor extensions
│   └── HeroHourGame/           # Game-specific code
├── Scripts/                    # Build & automation scripts
│   ├── build/                  # Build scripts
│   ├── ci/                     # CI helpers
│   └── tools/                  # Development tools
├── Content/                    # UE5 Content (tracked via LFS)
│   ├── Blueprints/             # Blueprint classes
│   ├── DataAssets/             # Data-driven assets
│   ├── Maps/                   # Levels & maps
│   ├── Materials/              # Materials
│   ├── Niagara/                # VFX
│   └── UI/                     # UMG widgets
├── Art/                        # Source art assets
│   ├── Concepts/               # Concept art
│   ├── Characters/             # Character models
│   ├── Buildings/              # Building models
│   ├── Environments/           # Environment assets
│   ├── VFX/                    # VFX source
│   └── UI/                     # UI designs
├── Audio/                      # Audio assets
│   ├── Music/                  # Music tracks
│   ├── SFX/                    # Sound effects
│   ├── Voice/                  # Voice lines
│   └── MetaSounds/             # MetaSound patches
├── Localization/               # Localization files
│   ├── en/                     # English
│   ├── de/                     # German
│   ├── fr/                     # French
│   ├── es/                     # Spanish
│   ├── ja/                     # Japanese
│   └── ko/                     # Korean
├── Tests/                      # Test projects
│   ├── Unit/                   # Unit tests
│   ├── Integration/            # Integration tests
│   ├── E2E/                    # End-to-end tests
│   └── Performance/            # Performance tests
├── HeroHour.uproject           # UE5 Project file
├── HeroHour.sln                # Solution file
├── Directory.Build.props       # MSBuild properties
├── global.json                 # .NET SDK version
├── nuget.config                # NuGet configuration
├── paket.dependencies          # Paket dependencies
├── README.md                   # This file
├── CONTRIBUTING.md             # Contribution guide
├── CHANGELOG.md                # Version history
├── LICENSE                     # License
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
git clone https://github.com/your-org/HeroHour.git
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
- Commit message convention
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

- **Command & Conquer: Generals - Zero Hour** - Inspiration
- **Unreal Engine Team** - Epic Games
- **F# Software Foundation** - F# language & tooling
- **Fable Team** - Fable compiler
- **Open Source Community** - Countless libraries & tools

---

## 📞 Contact

- **Project Lead**: [Hero](mailto:hero@herohour.dev)
- **Technical Director**: [Tech Lead](mailto:tech@herohour.dev)
- **Discord**: [HeroHour Community](https://discord.gg/herohour)
- **Website**: [herohour.dev](https://herohour.dev)

---

*HeroHour - Where Strategy Meets Spectacle* ⚔️