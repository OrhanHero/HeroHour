# Contributing to HeroHour

Thank you for your interest in contributing to HeroHour! This document provides guidelines and standards for contributing to the project.

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Setup](#development-setup)
4. [Branch Strategy](#branch-strategy)
5. [Commit Convention](#commit-convention)
6. [Pull Request Process](#pull-request-process)
7. [Code Review Guidelines](#code-review-guidelines)
8. [Testing Requirements](#testing-requirements)
9. [Documentation Standards](#documentation-standards)
10. [Reporting Issues](#reporting-issues)

---

## Code of Conduct

This project adheres to a Code of Conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

### Our Standards

- **Be respectful** - Value each other's ideas, styles, and viewpoints
- **Be inclusive** - Welcome newcomers and help them get started
- **Be collaborative** - Work together towards common goals
- **Be professional** - Keep discussions constructive and on-topic

---

## Getting Started

### Prerequisites

- **Windows 10/11** (primary development platform)
- **Unreal Engine 5.8** (installed via Epic Games Launcher)
- **Visual Studio 2022** (Community/Professional/Enterprise) with:
  - Game development with C++ workload
  - .NET desktop development workload
- **.NET 8 SDK** (for F# simulation layer)
- **Fable 5** (via .NET tools)
- **Git** with LFS support (`git lfs install`)
- **VS Code** (recommended) or **Rider** / **Visual Studio**

### First-Time Setup

```bash
# 1. Clone the repository
git clone https://github.com/HeroHour/HeroHour.git
cd HeroHour

# 2. Install Git LFS
git lfs install
git lfs pull

# 3. Run setup script
./Scripts/ci/setup-dev-environment.ps1

# 4. Open in VS Code
code .
```

---

## Development Setup

### IDE Configuration

**VS Code (Recommended)**
```bash
# Install recommended extensions
code --install-extension ms-dotnettools.csdevkit
code --install-extension Ionide.Ionide-fsharp
code --install-extension unreal.unreal-engine
code --install-extension ms-vscode.cpptools
code --install-extension eamodio.gitlens
```

**JetBrains Rider**
- Install Unity/Unreal plugin
- Configure F# plugin

### Build System

```powershell
# Full build (UE5 + F# + .NET)
./Scripts/build/Build.ps1 -Configuration Development

# Build only UE5
./Scripts/build/BuildUE5.ps1 -Target HeroHourEditor -Configuration Development

# Build only F# simulation
./Scripts/build/BuildFable.ps1 -Configuration Release

# Build only .NET tests
./Scripts/build/BuildDotNet.ps1 -RunTests
```

### Running the Game

```powershell
# Launch UE5 Editor
./Scripts/tools/LaunchEditor.ps1

# Run dedicated server
./Scripts/tools/RunServer.ps1 -Map "TestMap" -Players 4

# Run F# simulation tests
dotnet test Source/HeroHourSimulation/tests/HeroHourSimulation.Tests.fsproj
```

---

## Branch Strategy

We use **Trunk-Based Development** with short-lived feature branches.

### Branch Types

| Branch | Naming | Protection | Merge |
|--------|--------|------------|-------|
| `main` | `main` | Required PR, 2 approvals, CI pass | Squash |
| Release | `release/v{X}.{Y}` | Required PR, 1 approval | Merge commit |
| Feature | `feature/{JIRA-ID}-{short-desc}` | CI pass required | Squash |
| Bugfix | `bugfix/{JIRA-ID}-{short-desc}` | CI pass required | Squash |
| Hotfix | `hotfix/v{X}.{Y}.{Z}-{short-desc}` | Required PR, 1 approval | Merge commit |
| Experiment | `experiment/{topic}` | None | Rebase/Squash |

### Branch Lifecycle

```
main ←──────────────────────────────────────────
  ↑                                              │
  │ release/v1.0 ────────────────────────────────┤
  │    ↑                                         │
  │    │ feature/JIRA-123-new-hero ──────────────┤
  │    │    ↑                                    │
  │    │    │ bugfix/JIRA-124-fix-crash ─────────┤
  │    │    │    ↑                               │
  │    │    │    │ hotfix/v1.0.1-crash-fix ──────┤
  │    │    │    │    ↑                          │
  └────┴────┴────┴────┴──────────────────────────┘
```

---

## Commit Convention

We follow **Conventional Commits 1.0** specification.

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

| Type | Description | Example |
|------|-------------|---------|
| `feat` | New feature | `feat(simulation): add resource income system` |
| `fix` | Bug fix | `fix(ai): resolve commander stuck state` |
| `docs` | Documentation | `docs(arch): update GAS integration guide` |
| `style` | Formatting, linting | `style: format F# code with fantomas` |
| `refactor` | Code restructuring | `refactor(network): simplify replication graph` |
| `perf` | Performance | `perf(rendering): optimize Niagara particle spawn` |
| `test` | Tests | `test(simulation): add combat resolver property tests` |
| `chore` | Maintenance | `chore(deps): update UE5 to 5.8.1` |
| `build` | Build system | `build: add Fable 5 compilation step` |
| `ci` | CI/CD | `ci: add CodeQL analysis workflow` |

### Scopes (Project-Specific)

- `simulation` - F# simulation layer (economy, combat, AI decisions)
- `rendering` - UE5 rendering (Nanite, Lumen, Niagara, materials)
- `gameplay-ability` - GAS (abilities, effects, attributes)
- `ai` - Commander AI, behavior trees, state trees, EQS
- `network` - Multiplayer, replication, prediction, dedicated server
- `data` - Data assets, data tables, config, modding API
- `ui` - UMG, HUD, menus, command panel
- `audio` - MetaSounds, voice, music, SFX
- `build` - Build scripts, UE5 build, Fable, .NET
- `ci` - GitHub Actions, pipelines
- `docs` - Documentation, GDD, architecture
- `tooling` - Custom tools, editors, automation
- `config` - Configuration files, .ini, .json

### Examples

```bash
# Good commits
feat(simulation): implement resource tick system with configurable rates
fix(ai): commander now retreats when army strength < 30%
docs(arch): add F# to UE5 communication protocol diagram
perf(rendering): reduce draw calls by 40% via mesh instancing
test(combat): add property-based tests for damage calculation
refactor(network): extract prediction logic into separate module
chore(deps): upgrade .NET 8.0.100 -> 8.0.200

# Bad commits
fixed bug
update
WIP
more changes
```

### Commit Body

- Explain **what** and **why**, not **how** (code shows how)
- Reference issues: `Fixes #123`, `Closes #456`, `Relates to #789`
- List breaking changes: `BREAKING CHANGE: ...`

---

## Pull Request Process

### PR Requirements

Before opening a PR, ensure:

- [ ] Branch is up-to-date with `main` (`git rebase main`)
- [ ] All CI checks pass
- [ ] Code follows style guidelines (fantomas, clang-format)
- [ ] Tests added/updated for new functionality
- [ ] Documentation updated (code comments, README, CHANGELOG)
- [ ] No merge conflicts
- [ ] Commit messages follow convention

### PR Template

```markdown
## Description
Brief description of changes

## Type
- [ ] Feature
- [ ] Bug Fix
- [ ] Documentation
- [ ] Refactor
- [ ] Performance
- [ ] Test
- [ ] Chore

## Related Issues
Fixes #123
Closes #456

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing performed
- [ ] Performance impact assessed

## Checklist
- [ ] Code formatted
- [ ] Linting passes
- [ ] No console errors/warnings
- [ ] CHANGELOG.md updated
- [ ] Documentation updated
```

### Review Process

1. **Automated checks** run (build, test, lint, security)
2. **Code owner review** (required per CODEOWNERS)
3. **Domain expert review** (simulation, rendering, AI, network)
4. **Approval** from 1+ code owner + 1+ domain expert
5. **Merge** (squash for features, merge commit for releases/hotfixes)

### Review Guidelines

**Reviewers should:**
- Be constructive and specific
- Focus on correctness, maintainability, performance
- Ask questions rather than assume
- Approve when confident, request changes when needed

**Authors should:**
- Respond to all comments
- Push fixes as new commits (don't rewrite history after review starts)
- Mark conversations as resolved

---

## Code Review Guidelines

### What We Look For

#### Correctness
- [ ] Logic is correct and handles edge cases
- [ ] No race conditions or deadlocks
- [ ] Memory management is correct (UE5 GC, F# disposal)
- [ ] Network replication is safe

#### Maintainability
- [ ] Code is self-documenting (clear names, types)
- [ ] Complex logic has comments explaining *why*
- [ ] No duplicated code (DRY)
- [ ] Appropriate abstraction levels

#### Performance
- [ ] No obvious allocations in hot paths
- [ ] UE5: No tick-heavy operations, use timers/events
- [ ] F#: Tail recursion, avoid excessive boxing
- [ ] Network: Minimize replicated data

#### Safety
- [ ] No hardcoded secrets or keys
- [ ] Input validation on all external data
- [ ] Proper error handling with Result types (F#)

### Language-Specific Guidelines

#### F# (Simulation Layer)
```fsharp
// Good: Explicit types, Result for errors
let calculateDamage (attacker: Unit) (defender: Unit) : Result<float, string> =
    if attacker.isAlive && defender.isAlive then
        Ok (attacker.damage * defender.armorMultiplier)
    else
        Error "Unit not alive"

// Good: Discriminated unions for domain modeling
type GameCommand =
    | MoveUnits of UnitID list * Position
    | AttackTarget of UnitID list * TargetID
    | BuildStructure of BuildingType * Position

// Avoid: Excessive mutation, null, exceptions for control flow
```

#### C++ (UE5 Layer)
```cpp
// Good: Const correctness, UPROPERTY for reflection
UCLASS()
class AHeroHourUnit : public APawn
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Combat")
    void ApplyDamage(float Damage, const FDamageEvent& Event);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
    float Health;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
    float MaxHealth;
};

// Good: Use TArray, TMap, TSet instead of std containers
// Good: Check pointers before dereferencing
if (Unit && Unit->IsAlive())
{
    Unit->TakeDamage(Damage);
}
```

#### Blueprint/C++ Interface
- Expose minimal C++ API to Blueprints
- Use `BlueprintCallable` sparingly
- Prefer C++ for performance-critical code
- Document Blueprint interfaces in header comments

---

## Testing Requirements

### Test Pyramid

```
           /\
          /  \     E2E Tests (Few)
         /----\    - Full match simulation
        /      \   - Multiplayer scenarios
       /--------\  - User journey tests
      /          \
     /            \  Integration Tests (Some)
    /--------------\ - Simulation + UE5 integration
   /                \ - Network replication tests
  /                  \ - AI vs AI matches
 /--------------------\
/                      \ Unit Tests (Many)
------------------------  - F# simulation logic (80%+ coverage)
                        - UE5: 60%+)
                        - Pure functions, property-based testing
```

### Coverage Targets

| Layer | Target | Tool |
|-------|--------|------|
| F# Simulation | 80%+ | `dotnet test /p:CollectCoverage=true` |
| UE5 Gameplay | 60%+ | `UnrealTestRunner`, `Gauntlet` |
| Network | 70%+ | Custom replication tests |
| AI | 70%+ | Behavior tree validation |

### Running Tests

```powershell
# F# Unit Tests
dotnet test Source/HeroHourSimulation/tests/HeroHourSimulation.Tests.fsproj \
  --configuration Release \
  --collect:"XPlat Code Coverage"

# UE5 Tests (Editor)
./Scripts/tools/RunTests.ps1 -TestFilter "HeroHour*"

# Network Tests
./Scripts/tools/RunNetworkTests.ps1 -Players 4 -Duration 300

# Performance Tests
./Scripts/tools/RunPerfTests.ps1 -Scene "LargeBattle" -Frames 1000
```

### Test Categories

| Category | Command | When |
|----------|---------|------|
| Fast (unit) | `dotnet test --filter Category=Fast` | Pre-commit, CI |
| Medium (integration) | `dotnet test --filter Category=Integration` | PR, Nightly |
| Slow (E2E) | `./RunE2ETests.ps1` | Release, Weekly |
| Performance | `./RunPerfTests.ps1` | Release, On-demand |

---

## Documentation Standards

### Code Documentation

**F#** - XML docs for public APIs
```fsharp
/// Calculates the combat result between two units
/// <param name="attacker">The attacking unit</param>
/// <param name="defender">The defending unit</param>
/// <returns>Combat result with damage and effects</returns>
let resolveCombat (attacker: Unit) (defender: Unit) : CombatResult = ...
```

**C++** - Doxygen-style for public APIs
```cpp
/**
 * Applies damage to this unit
 * @param Damage Amount of damage to apply
 * @param Event Damage event details (type, source, etc.)
 * @return Actual damage applied after mitigation
 */
UFUNCTION(BlueprintCallable, Category="Combat")
float ApplyDamage(float Damage, const FDamageEvent& Event);
```

### Architecture Documentation

- Update `docs/architecture/` for structural changes
- Include diagrams (Mermaid preferred)
- Link to related issues/PRs

### Changelog

Update `CHANGELOG.md` for every PR (Keep a Changelog format):

```markdown
## [Unreleased]
### Added
- New hero commander system with unique abilities (#123)

### Fixed
- Commander AI getting stuck in retreat loop (#456)

### Changed
- Resource tick rate now configurable via data asset (#789)
```

---

## Reporting Issues

### Bug Reports

Use the **Bug Report** template. Include:

1. **Clear title**: `[Bug] Commander AI freezes when resources depleted`
2. **Steps to reproduce**: Numbered, minimal steps
3. **Expected behavior**: What should happen
4. **Actual behavior**: What happens (with logs/screenshots)
5. **Environment**: OS, UE5 version, .NET version, branch
6. **Logs**: `Saved/Logs/`, `dotnet` output, network traces

### Feature Requests

Use the **Feature Request** template. Include:

1. **Problem statement**: What user need does this solve?
2. **Proposed solution**: High-level approach
3. **Alternatives considered**: Why this approach?
4. **Scope estimate**: Small/Medium/Large
5. **Dependencies**: Other teams, systems affected

### Technical Debt

Use the **Tech Debt** template. Include:

1. **Location**: File(s), system(s)
2. **Impact**: Performance, maintainability, risk
3. **Proposed fix**: Refactor approach
4. **Effort estimate**: Hours/days

---

## Release Process

### Versioning

We follow **Semantic Versioning 2.0**:

```
MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD]

Examples:
1.0.0          - First stable release
1.1.0          - Backward-compatible features
1.1.1          - Bug fixes
2.0.0          - Breaking changes
1.0.0-alpha.1  - Alpha pre-release
1.0.0-beta.3   - Beta pre-release
1.0.0-rc.1     - Release candidate
```

### Release Branches

1. Create `release/v{X}.{Y}` from `main`
2. Only bugfixes allowed (cherry-picked from `main`)
3. Version bump in `CHANGELOG.md` and `DefaultGame.ini`
4. Tag `v{X}.{Y}.{Z}` on merge to `main`
5. Deploy to staging → production

### Hotfixes

1. Create `hotfix/v{X}.{Y}.{Z}-{issue}` from `main`
2. Fix with `fix(scope):` commit
3. Fast-track PR (1 approval + CI)
4. Tag `v{X}.{Y}.{Z+1}-hotfix.{N}`
5. Cherry-pick to active release branch

---

## Getting Help

- **Discord**: `#heroHour-dev` (internal)
- **GitHub Discussions**: For design questions
- **GitHub Issues**: For bugs, features, tech debt
- **Wiki**: `docs/` for architecture guides

---

## Recognition

Contributors are recognized in:
- `CONTRIBUTORS.md` (all contributors)
- Release notes (significant contributions)
- Hall of Fame (long-term contributors)

---

*Thank you for contributing to HeroHour! 🎮*

*Last updated: 2025-07-18 | Version 1.0*