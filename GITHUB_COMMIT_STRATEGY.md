# HeroHour - GitHub Commit & Push Strategy

## Project Overview
**HeroHour** - AAA RTS Game (C&C Generals: Zero Hour inspired)
- **Engine**: Unreal Engine 5.8 + F# (Fable 5) + .NET 8
- **Platform**: Windows PC (future: consoles)
- **Timeline**: 18-24 months | **Team**: 40-60 developers
- **Architecture**: UE5 Rendering/Presentation + F# Simulation Layer + .NET 8 Runtime

---

## Git Strategy Overview

### Branch Strategy: **Trunk-Based Development with Feature Branches**

```
main (protected) ←── release/v1.0 ←── feature/* ←── developer forks
                     ↑
               hotfix/v1.0.x
```

| Branch Type | Naming Convention | Protection | Merge Strategy |
|-------------|-------------------|------------|----------------|
| **main** | `main` | Required PR, 2 approvals, CI pass | Squash merge |
| **Release** | `release/v{major}.{minor}` | Required PR, 1 approval | Merge commit |
| **Feature** | `feature/{JIRA-ID}-{short-desc}` | CI pass required | Squash merge |
| **Bugfix** | `bugfix/{JIRA-ID}-{short-desc}` | CI pass required | Squash merge |
| **Hotfix** | `hotfix/v{version}-{short-desc}` | Required PR, 1 approval | Merge commit |
| **Experiment** | `experiment/{topic}` | No protection | Squash/Rebase |

### Commit Message Convention: **Conventional Commits 1.0**

```
<type>(<scope>): <subject>

<body>

<footer>
```

| Type | Description | Scope Examples |
|------|-------------|----------------|
| `feat` | New feature | `simulation`, `rendering`, `ai`, `ui`, `network` |
| `fix` | Bug fix | `pathfinding`, `economy`, `multiplayer`, `ui` |
| `docs` | Documentation | `architecture`, `api`, `gdd`, `setup` |
| `style` | Formatting, linting | `format`, `lint`, `whitespace` |
| `refactor` | Code restructuring | `simulation`, `ai`, `gamplay-ability` |
| `perf` | Performance improvements | `rendering`, `simulation`, `network` |
| `test` | Tests | `unit`, `integration`, `e2e` |
| `chore` | Maintenance | `deps`, `ci`, `build`, `config` |
| `build` | Build system | `unreal`, `fable`, `dotnet`, `ci` |
| `ci` | CI/CD changes | `github-actions`, `gitlab-ci`, `jenkins` |

### Scopes (Project-Specific)
- `simulation` - F# simulation layer (economy, combat, AI decisions)
- `rendering` - UE5 rendering, Nanite, Lumen, Niagara
- `gameplay-ability` - GAS abilities, effects, attributes
- `ai` - Commander AI, behavior trees, state trees, EQS
- `network` - Multiplayer, replication, prediction, dedicated server
- `data` - Data assets, data tables, data assets, config
- `ui` - UI/UX, UMG, HUD, menus
- `audio` - Audio, MetaSounds, voice lines
- `build` - Build system, UE5 build, Fable compile
- `ci` - CI/CD pipelines
- `docs` - Documentation, GDD, architecture docs
- `tooling` - Custom tools, editors, automation

---

## Initial Repository Structure (First Commit)

```
HeroHour/
├── .github/
│   ├── workflows/
│   │   ├── ci.yml              # CI pipeline (UE5 + Fable + .NET)
│   │   ├── cd.yml              # CD pipeline (staging/production)
│   │   ├── codeql.yml          # Security scanning
│   │   └── dependabot.yml      # Dependency updates
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.yml
│   │   ├── feature_request.yml
│   │   ├── tech_debt.yml
│   │   └── epic.yml
│   ├── PULL_REQUEST_TEMPLATE.md
│   ├── CODEOWNERS
│   └── dependabot.yml
├── .vscode/
│   ├── settings.json
│   ├── extensions.json
│   └── launch.json
├── .editorconfig
├── .gitignore
├── .gitattributes
├── README.md
├── LICENSE
├── CHANGELOG.md
├── CONTRIBUTING.md
├── docs/
│   ├── architecture/
│   │   ├── HIGH_LEVEL_ARCHITECTURE.md
│   │   ├── FSHARP_INTEGRATION.md
│   │   ├── GAS_INTEGRATION.md
│   │   ├── NETWORK_ARCHITECTURE.md
│   │   └── AI_ARCHITECTURE.md
│   ├── gdd/
│   │   ├── HERO_HOUR_GDD.md          # Full GDD from .docx
│   │   ├── GAMEPLAY_PILLARS.md
│   │   ├── HERO_SYSTEM.md
│   │   ├── AI_SYSTEM.md
│   │   └── MULTIPLAYER.md
│   ├── technical/
│   │   ├── UE5_INTEGRATION.md
│   │   ├── FSHARP_SIMULATION.md
│   │   ├── DATA_DRIVEN_DESIGN.md
│   │   ├── MODDING_API.md
│   │   └── PERFORMANCE_TARGETS.md
│   └── production/
│       ├── ASSET_PIPELINE.md
│       ├── BUILD_PIPELINE.md
│       ├── TESTING_STRATEGY.md
│       └── RELEASE_CHECKLIST.md
├── Source/
│   ├── HeroHour/                    # UE5 C++ Module
│   │   ├── HeroHour.Build.cs
│   │   ├── Public/
│   │   │   ├── HeroHour.h
│   │   │   ├── Simulation/
│   │   │   │   ├── IFSharpSimulationBridge.h
│   │   │   │   ├── GameState.h
│   │   │   │   ├── GameCommand.h
│   │   │   │   └── SimulationResult.h
│   │   │   ├── GameplayAbility/
│   │   │   │   ├── HeroHourGameplayAbility.h
│   │   │   │   ├── HeroHourGameplayEffect.h
│   │   │   │   └── HeroHourAttributeSet.h
│   │   │   ├── AI/
│   │   │   │   ├── CommanderAI.h
│   │   │   │   ├── AIBehaviorTree.h
│   │   │   │   └── AIStateTree.h
│   │   │   ├── Network/
│   │   │   │   ├── HeroHourPlayerState.h
│   │   │   │   ├── PredictionManager.h
│   │   │   │   └── ReplicationGraph.h
│   │   │   ├── Data/
│   │   │   │   ├── DataAssetRegistry.h
│   │   │   │   ├── UnitDataAsset.h
│   │   │   │   ├── BuildingDataAsset.h
│   │   │   │   └── HeroDataAsset.h
│   │   │   └── UI/
│   │   │       ├── HUDWidget.h
│   │   │       └── CommandMenuWidget.h
│   │   └── Private/
│   │       ├── HeroHourModule.cpp
│   │       ├── Simulation/
│   │       ├── GameplayAbility/
│   │       ├── AI/
│   │       ├── Network/
│   │       ├── Data/
│   │       └── UI/
│   ├── HeroHourSimulation/          # F# Simulation Layer (.NET 8)
│   │   ├── HeroHourSimulation.fsproj
│   │   ├── src/
│   │   │   ├── Core/
│   │   │   │   ├── GameState.fs
│   │   │   │   ├── GameCommand.fs
│   │   │   │   ├── SimulationResult.fs
│   │   │   │   ├── Types.fs
│   │   │   │   └── Constants.fs
│   │   │   ├── Economy/
│   │   │   │   ├── ResourceManager.fs
│   │   │   │   ├── EconomySimulator.fs
│   │   │   │   └── ResourceTypes.fs
│   │   │   ├── Combat/
│   │   │   │   ├── CombatResolver.fs
│   │   │   │   ├── DamageCalculator.fs
│   │   │   │   └── UnitStats.fs
│   │   │   ├── AI/
│   │   │   │   ├── CommanderAI.fs
│   │   │   │   ├── StrategicPlanner.fs
│   │   │   │   ├── TacticalPlanner.fs
│   │   │   │   ├── EconomyPlanner.fs
│   │   │   │   └── DifficultyScaler.fs
│   │   │   ├── Units/
│   │   │   │   ├── UnitManager.fs
│   │   │   │   ├── MovementSystem.fs
│   │   │   │   └── FormationSystem.fs
│   │   │   ├── Buildings/
│   │   │   │   ├── BuildingManager.fs
│   │   │   │   ├── ProductionQueue.fs
│   │   │   │   └── ConstructionSystem.fs
│   │   │   ├── Data/
│   │   │   │   ├── DataLoader.fs
│   │   │   │   ├── DataAssets.fs
│   │   │   │   └── ConfigManager.fs
│   │   │   └── Events/
│   │   │       ├── EventBus.fs
│   │   │       ├── GameEvent.fs
│   │   │       └── EventHandlers.fs
│   │   ├── tests/
│   │   │   ├── HeroHourSimulation.Tests.fsproj
│   │   │   ├── Core/
│   │   │   ├── Economy/
│   │   │   ├── Combat/
│   │   │   └── AI/
│   │   └── paket.dependencies
│   ├── HeroHourEditor/              # UE5 Editor Extensions
│   │   ├── HeroHourEditor.Build.cs
│   │   └── Private/
│   └── HeroHourGame/                # UE5 Game Module (Blueprints + C++)
│       ├── HeroHourGame.Build.cs
│       ├── Config/
│       │   ├── DefaultEngine.ini
│       │   ├── DefaultGame.ini
│       │   ├── DefaultInput.ini
│       │   └── DefaultScalability.ini
│       ├── Content/
│       │   ├── Blueprints/
│       │   ├── DataAssets/
│       │   ├── Maps/
│       │   ├── Materials/
│       │   ├── Niagara/
│       │   └── UI/
│       └── Source/
├── Scripts/
│   ├── build/
│   │   ├── Build.ps1
│   │   ├── Build.sh
│   │   ├── BuildUE5.ps1
│   │   ├── BuildFable.ps1
│   │   └── BuildDotNet.ps1
│   ├── ci/
│   │   ├── setup-ue5.ps1
│   │   ├── setup-fable.ps1
│   │   └── setup-dotnet.ps1
│   └── tools/
│       ├── DataAssetGenerator/
│       ├── ModTooling/
│       └── PerformanceProfiler/
├── Tests/
│   ├── Unit/
│   ├── Integration/
│   ├── E2E/
│   └── Performance/
├── Art/
│   ├── Concepts/
│   ├── Characters/
│   ├── Buildings/
│   ├── Environments/
│   ├── VFX/
│   └── UI/
├── Audio/
│   ├── Music/
│   ├── SFX/
│   ├── Voice/
│   └── MetaSounds/
└── Localization/
    ├── en/
    ├── de/
    ├── fr/
    ├── es/
    ├── ja/
    └── ko/
```

---

## Initial Commit Plan (v0.1.0 - Project Initialization)

### Commit 1: `chore(init): initialize HeroHour repository structure`
```
chore(init): initialize HeroHour repository structure

- Add .gitignore for UE5, F#, .NET, VS Code, Rider
- Add .gitattributes for LFS (uassets, umaps, textures, audio)
- Add .editorconfig for consistent formatting
- Add LICENSE (MIT/Proprietary)
- Add README.md with project overview
- Add CONTRIBUTING.md with contribution guidelines
- Add CHANGELOG.md with Keep a Changelog format

Scope: build, config
```

### Commit 2: `docs(gdd): add HeroHour Game Design Document v2.0`
```
docs(gdd): add HeroHour Game Design Document v2.0

- Executive Summary & Project Vision
- Technical Architecture (UE5.8 + F# + .NET 8)
- Gameplay Framework (GAS, Mass Entity, State Trees)
- AI & Behavior Systems (Commander AI, EQS)
- Asset Production Pipeline
- Multiplayer & Networking Architecture
- UI/UX Design
- Audio & VFX Pipeline
- Modular Gameplay Features
- Production Pipeline & Roadmap
- Studio Structure & Technical Requirements
- Quality Assurance & Live Operations

Source: HERO HOUR_Projekt.docx (AAA Production Framework)

Scope: docs, gdd
```

### Commit 3: `docs(arch): add technical architecture documentation`
```
docs(arch): add technical architecture documentation

- HIGH_LEVEL_ARCHITECTURE.md - UE5 + F# integration diagram
- FSHARP_INTEGRATION.md - F# simulation layer architecture
- GAS_INTEGRATION.md - Gameplay Ability System design
- NETWORK_ARCHITECTURE.md - Multiplayer replication strategy
- AI_ARCHITECTURE.md - Commander AI hierarchy & behavior trees

Scope: docs, architecture
```

### Commit 4: `docs(tech): add technical specifications`
```
docs(tech): add technical specifications

- UE5_INTEGRATION.md - UE5.8 features (Nanite, Lumen, Niagara, World Partition)
- FSHARP_SIMULATION.md - Fable 5 + .NET 8 simulation layer
- DATA_DRIVEN_DESIGN.md - Data asset architecture
- MODDING_API.md - Modding support through data assets
- PERFORMANCE_TARGETS.md - 1000+ units @ 60fps targets

Scope: docs, technical
```

### Commit 5: `docs(prod): add production pipeline documentation`
```
docs(prod): add production pipeline documentation

- ASSET_PIPELINE.md - Character, building, VFX pipelines
- BUILD_PIPELINE.md - UE5 + Fable + .NET build process
- TESTING_STRATEGY.md - Unit, integration, E2E, performance testing
- RELEASE_CHECKLIST.md - Release validation checklist

Scope: docs, production
```

### Commit 6: `chore(config): add GitHub Actions CI/CD configuration`
```
chore(config): add GitHub Actions CI/CD configuration

- ci.yml: UE5 build, Fable compile, .NET test, linting
- cd.yml: Staging/production deployment
- codeql.yml: Security scanning
- dependabot.yml: Automated dependency updates
- Issue templates: bug, feature, tech debt, epic
- PR template with checklist
- CODEOWNERS for code review routing

Scope: ci, build, config
```

### Commit 7: `chore(config): add development environment configuration`
```
chore(config): add development environment configuration

- .vscode/settings.json - Editor settings for UE5 + F#
- .vscode/extensions.json - Recommended extensions
- .vscode/launch.json - Debug configurations
- paket.dependencies - F# package management

Scope: config, tooling
```

### Commit 8: `build(init): add build scripts and tooling`
```
build(init): add build scripts and tooling

- Scripts/build/Build.ps1 - Unified build script
- Scripts/build/BuildUE5.ps1 - UE5 build automation
- Scripts/build/BuildFable.ps1 - Fable 5 compilation
- Scripts/build/BuildDotNet.ps1 - .NET 8 build
- Scripts/ci/setup-*.ps1 - CI environment setup
- Scripts/tools/ - Custom tooling skeleton

Scope: build, tooling
```

---

## Commit Schedule (First 2 Weeks)

### Week 1: Foundation
| Day | Commits | Focus |
|-----|---------|-------|
| 1 | 1-3 | Repo init, GDD, Architecture docs |
| 2 | 4-5 | Technical & Production docs |
| 3 | 6-7 | CI/CD, Dev environment |
| 4 | 8 | Build scripts |
| 5 | - | Code review, merge to main |

### Week 2: Core Module Scaffolding
| Day | Commits | Focus |
|-----|---------|-------|
| 6 | 9-10 | UE5 Module structure (HeroHour, HeroHourGame) |
| 7 | 11-12 | F# Simulation project structure |
| 8 | 13-14 | Editor module, Config files |
| 9 | 15-16 | Data asset base classes |
| 10 | - | Integration test, merge to main |

---

## Subsequent Major Milestones

### Milestone 1: Core Simulation (v0.2.0) - Sprint 1-3
```
feat(simulation): implement core game state management
feat(simulation): add economy simulation (resources, income)
feat(simulation): implement combat resolution system
feat(simulation): add unit movement & formation system
feat(simulation): implement building construction/production
test(simulation): unit tests for core simulation (80% coverage)
```

### Milestone 2: UE5 Integration (v0.3.0) - Sprint 4-6
```
feat(rendering): integrate F# simulation with UE5
feat(rendering): implement Mass Entity for 1000+ units
feat(rendering): setup Nanite for buildings, Lumen for lighting
feat(gameplay-ability): GAS base classes for abilities/effects
feat(data): data asset registry & loader
perf(rendering): optimize for 60fps @ 1000 units
```

### Milestone 3: AI Systems (v0.4.0) - Sprint 7-9
```
feat(ai): implement Commander AI strategic planner
feat(ai): add economy planner & build order manager
feat(ai): implement military planner (attack/defense)
feat(ai): add behavior trees & state trees for units
feat(ai): integrate EQS for tactical decisions
test(ai): AI vs AI test matches
```

### Milestone 4: Multiplayer (v0.5.0) - Sprint 10-12
```
feat(network): dedicated server architecture
feat(network): replication graph for 1000+ actors
feat(network): client prediction & reconciliation
feat(network): matchmaking & lobby system
feat(network): replay system
test(network): 4-player stress test
```

### Milestone 5: Gameplay Features (v0.6.0) - Sprint 13-16
```
feat(gameplay): Hero system with unique abilities
feat(gameplay): Dynamic campaign mission generation
feat(gameplay): Tech tree & research system
feat(gameplay): Territory control & victory conditions
feat(ui): Command & control UI, HUD, menus
```

### Milestone 6: Polish & Ship (v1.0.0) - Sprint 17-24
```
perf(*): optimization pass (CPU, GPU, memory)
polish(ui): UX polish, accessibility, localization
polish(audio): Full audio implementation
polish(vfx): Niagara effects for all units/abilities
test(*): Full QA pass, certification
build(*): Shipping build configuration
```

---

## PR Review Checklist

### Required for All PRs
- [ ] Conventional commit message format
- [ ] CI passes (build, test, lint)
- [ ] Code review by 1+ domain owner (per CODEOWNERS)
- [ ] No merge conflicts with main
- [ ] CHANGELOG.md updated (for feat/fix)

### Domain-Specific
| Domain | Additional Checks |
|--------|-------------------|
| `simulation` | F# unit tests pass, property-based tests |
| `rendering` | GPU profiler capture, no regressions |
| `gameplay-ability` | GAS tests, ability logic verified |
| `ai` | AI vs AI test runs, behavior verified |
| `network` | 4-player test, bandwidth check |
| `data` | Data asset validation, schema check |
| `build` | Clean build on Windows/Linux |

---

## Release Tagging Convention

```
v{major}.{minor}.{patch}-{stage}.{build}

Examples:
v0.1.0-alpha.1      # Alpha release
v0.2.0-beta.3       # Beta release
v0.5.0-rc.1         # Release candidate
v1.0.0              # GA release
v1.0.1-hotfix.1     # Hotfix
```

### Release Branches
- `release/v0.x` - Stabilization branch
- Only bugfixes allowed (cherry-picked from main)
- Version bump in `CHANGELOG.md` and `DefaultGame.ini`

---

## Git LFS Configuration

### Tracked Extensions
```
*.uasset filter=lfs diff=lfs merge=lfs -text
*.umap filter=lfs diff=lfs merge=lfs -text
*.png filter=lfs diff=lfs merge=lfs -text
*.jpg filter=lfs diff=lfs merge=lfs -text
*.exr filter=lfs diff=lfs merge=lfs -text
*.wav filter=lfs diff=lfs merge=lfs -text
*.ogg filter=lfs diff=lfs merge=lfs -text
*.uasset filter=lfs diff=lfs merge=lfs -text
*.umap filter=lfs diff=lfs merge=lfs -text
```

---

## Security & Compliance

- **CodeQL** scanning on every PR
- **Dependabot** for NuGet, npm, UE5 dependencies
- **Secret scanning** enabled
- **Signed commits** required for main branch
- **SBOM** generation on release (CycloneDX)

---

## Automation Hooks

### Pre-commit (via Husky/pre-commit)
- `dotnet format` for F#/C#
- `clang-format` for C++
- `markdownlint` for docs
- `yamlfmt` for YAML

### Pre-push
- Run unit tests (fast subset)
- Check for merge conflicts
- Validate commit message format

---

## Team Roles & Code Ownership

| Role | GitHub Team | Code Ownership |
|------|-------------|----------------|
| Tech Lead | @heroHour/tech-leads | All (approval required) |
| Simulation Engineers | @heroHour/simulation | `Source/HeroHourSimulation/**` |
| Rendering Engineers | @heroHour/rendering | `Source/HeroHour/Rendering/**` |
| Gameplay Engineers | @heroHour/gameplay | `Source/HeroHourGame/**`, GAS |
| AI Engineers | @heroHour/ai | `Source/HeroHour/AI/**`, F# AI |
| Network Engineers | @heroHour/network | `Source/HeroHour/Network/**` |
| Tools Engineers | @heroHour/tools | `Scripts/**`, Editor extensions |
| QA Engineers | @heroHour/qa | `Tests/**`, test infrastructure |
| DevOps | @heroHour/devops | `.github/**`, build scripts |

---

## Emergency Procedures

### Hotfix Process
1. Create `hotfix/v{version}-{issue}` from `main`
2. Fix with `fix(scope):` commit
3. Fast-track PR (1 approval, CI pass)
4. Tag `v{version}.{patch}-hotfix.{n}`
5. Cherry-pick to `release/v{version}` if exists

### Rollback Procedure
1. Tag current `main` as `v{version}-rollback-{timestamp}`
2. Revert merge commit on `main`
3. Force push with lease (requires admin)
4. Trigger hotfix process

---

## Metrics & Monitoring

### Tracked Metrics
- **Commit frequency** - Target: 50+/day across team
- **PR cycle time** - Target: < 4 hours
- **Build success rate** - Target: > 95%
- **Test coverage** - Target: > 80% (simulation), > 60% (UE5)
- **LFS bandwidth** - Monitor for cost control

### Dashboards
- GitHub Insights (commits, PRs, contributors)
- Custom Grafana for build/test metrics
- UE5 Insights for performance tracking

---

## Appendix: Useful Git Aliases

```bash
# Add to ~/.gitconfig
[alias]
    co = checkout
    cob = checkout -b
    br = branch
    st = status
    cm = commit -m
    cam = commit -am
    ps = push
    pl = pull --rebase
    lg = log --oneline --graph --decorate -20
    lgf = log --oneline --graph --decorate --all
    undo = reset --soft HEAD~1
    amend = commit --amend --no-edit
    wip = commit -am "WIP"
    unwip = reset HEAD~1
    prune = fetch --prune
    cleanup = !git branch --merged | grep -v '\\*\\|main\\|release' | xargs -n 1 git branch -d
```

---

*Document Version: 1.0*  
*Created: 2025-07-18*  
*Project: HeroHour - AAA RTS Game*  
*Next Review: Sprint 1 Planning*